/*
 * Copyright (c) 2019 TAOS Data, Inc. <jhtao@taosdata.com>
 *
 * This program is free software: you can use, redistribute, and/or modify
 * it under the terms of the GNU Affero General Public License, version 3
 * or later ("AGPL"), as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#define _DEFAULT_SOURCE
#include "sdb.h"

static int32_t sdbCreateDir(SSdb *pSdb);

SSdb *sdbInit(SSdbOpt *pOption) {
  mInfo("start to init sdb in %s", pOption->path);

  SSdb *pSdb = taosMemoryCalloc(1, sizeof(SSdb));
  if (pSdb == NULL) {
    terrno = TSDB_CODE_OUT_OF_MEMORY;
    mError("failed to init sdb since %s", terrstr());
    return NULL;
  }

  char path[PATH_MAX + 100] = {0};
  snprintf(path, sizeof(path), "%s%sdata", pOption->path, TD_DIRSEP);
  pSdb->currDir = taosStrdup(path);
  snprintf(path, sizeof(path), "%s%stmp", pOption->path, TD_DIRSEP);
  pSdb->tmpDir = taosStrdup(path);
  if (pSdb->currDir == NULL || pSdb->tmpDir == NULL) {
    sdbCleanup(pSdb);
    terrno = TSDB_CODE_OUT_OF_MEMORY;
    mError("failed to init sdb since %s", terrstr());
    return NULL;
  }

  if (sdbCreateDir(pSdb) != 0) {
    sdbCleanup(pSdb);
    return NULL;
  }

  for (ESdbType i = 0; i < SDB_MAX; ++i) {
    (void)taosThreadRwlockInit(&pSdb->locks[i], NULL);
    pSdb->maxId[i] = 0;
    pSdb->tableVer[i] = 0;
    pSdb->keyTypes[i] = SDB_KEY_INT32;
  }

  pSdb->pWal = pOption->pWal;
  pSdb->applyIndex = -1;
  pSdb->applyTerm = -1;
  pSdb->applyConfig = -1;
  pSdb->commitIndex = -1;
  pSdb->commitTerm = -1;
  pSdb->commitConfig = -1;
  pSdb->pMnode = pOption->pMnode;
  (void)taosThreadMutexInit(&pSdb->filelock, NULL);
  mInfo("sdb init success");
  return pSdb;
}

void sdbCleanup(SSdb *pSdb) {
  mInfo("start to cleanup sdb");

  int32_t code = 0;

  if ((code = sdbWriteFile(pSdb, 0)) != 0) {
    mError("failed to write sdb file since %s", tstrerror(code));
  }

  if (pSdb->currDir != NULL) {
    taosMemoryFreeClear(pSdb->currDir);
  }

  if (pSdb->tmpDir != NULL) {
    taosRemoveDir(pSdb->tmpDir);
    taosMemoryFreeClear(pSdb->tmpDir);
  }

  for (ESdbType i = 0; i < SDB_MAX; ++i) {
    SHashObj *hash = pSdb->hashObjs[i];
    if (hash == NULL) continue;

    SSdbRow **ppRow = taosHashIterate(hash, NULL);
    while (ppRow != NULL) {
      SSdbRow *pRow = *ppRow;
      if (pRow == NULL) continue;

      sdbFreeRow(pSdb, pRow, true);
      ppRow = taosHashIterate(hash, ppRow);
    }
  }

  for (ESdbType i = 0; i < SDB_MAX; ++i) {
    SHashObj *hash = pSdb->hashObjs[i];
    if (hash == NULL) continue;

    taosHashClear(hash);
    taosHashCleanup(hash);
    (void)taosThreadRwlockDestroy(&pSdb->locks[i]);
    pSdb->hashObjs[i] = NULL;
    memset(&pSdb->locks[i], 0, sizeof(pSdb->locks[i]));

    mInfo("sdb table:%s is cleaned up", sdbTableName(i));
  }

  (void)taosThreadMutexDestroy(&pSdb->filelock);
  taosMemoryFree(pSdb);
  mInfo("sdb is cleaned up");
}

int32_t sdbSetTable(SSdb *pSdb, SSdbTable table) {
  int32_t code = 0;

  ESdbType sdbType = table.sdbType;
  EKeyType keyType = table.keyType;
  pSdb->keyTypes[sdbType] = table.keyType;
  pSdb->insertFps[sdbType] = table.insertFp;
  pSdb->updateFps[sdbType] = table.updateFp;
  pSdb->deleteFps[sdbType] = table.deleteFp;
  pSdb->deployFps[sdbType] = table.deployFp;
  pSdb->encodeFps[sdbType] = table.encodeFp;
  pSdb->decodeFps[sdbType] = table.decodeFp;
  pSdb->afterRestoredFps[sdbType] = table.afterRestoredFp;
  pSdb->validateFps[sdbType] = table.validateFp;

  int32_t hashType = 0;
  if (keyType == SDB_KEY_INT32) {
    hashType = TSDB_DATA_TYPE_INT;
  } else if (keyType == SDB_KEY_INT64) {
    hashType = TSDB_DATA_TYPE_BIGINT;
  } else {
    hashType = TSDB_DATA_TYPE_BINARY;
  }

  SHashObj *hash = taosHashInit(64, taosGetDefaultHashFunction(hashType), true, HASH_ENTRY_LOCK);
  if (hash == NULL) {
    TAOS_RETURN(terrno);
  }

  pSdb->maxId[sdbType] = 0;
  pSdb->hashObjs[sdbType] = hash;
  mInfo("sdb table:%s is initialized", sdbTableName(sdbType));

  TAOS_RETURN(0);
}

static int32_t sdbCreateDir(SSdb *pSdb) {
  int32_t code = 0;
  if (taosMulMkDir(pSdb->currDir) != 0) {
    code = TAOS_SYSTEM_ERROR(ERRNO);
    mError("failed to create dir:%s since %s", pSdb->currDir, tstrerror(code));
    TAOS_RETURN(code);
  }

  if (taosMkDir(pSdb->tmpDir) != 0) {
    code = TAOS_SYSTEM_ERROR(ERRNO);
    mError("failed to create dir:%s since %s", pSdb->tmpDir, tstrerror(code));
    TAOS_RETURN(code);
  }

  return 0;
}

void sdbSetApplyInfo(SSdb *pSdb, int64_t index, int64_t term, int64_t config) {
  mInfo("vgId:1, mnode apply info changed from index:%" PRId64 " term:%" PRId64 " config:%" PRId64 " to index:%" PRId64
        " term:%" PRId64 " config:%" PRId64,
        pSdb->applyIndex, pSdb->applyTerm, pSdb->applyConfig, index, term, config);

  pSdb->applyIndex = index;
  pSdb->applyTerm = term;
  pSdb->applyConfig = config;
}

void sdbGetCommitInfo(SSdb *pSdb, int64_t *index, int64_t *term, int64_t *config) {
  *index = pSdb->commitIndex;
  *term = pSdb->commitTerm;
  *config = pSdb->commitConfig;
#if 1
  mTrace("mnode current info, apply index:%" PRId64 " term:%" PRId64 " config:%" PRId64 ", commit index:%" PRId64
         " term:%" PRId64 " config:%" PRId64,
         pSdb->applyIndex, pSdb->applyTerm, pSdb->applyConfig, *index, *term, *config);
#endif
}

void sdbWriteLock(SSdb *pSdb, int32_t type) {
  TdThreadRwlock *pLock = &pSdb->locks[type];
  // mTrace("sdb table:%d start write lock:%p", type, pLock);
  (void)taosThreadRwlockWrlock(pLock);
  // mTrace("sdb table:%d stop write lock:%p", type, pLock);
}

void sdbReadLock(SSdb *pSdb, int32_t type) {
  TdThreadRwlock *pLock = &pSdb->locks[type];
  // mTrace("sdb table:%d start read lock:%p", type, pLock);
  (void)taosThreadRwlockRdlock(pLock);
  // mTrace("sdb table:%d stop read lock:%p", type, pLock);
}

void sdbUnLock(SSdb *pSdb, int32_t type) {
  TdThreadRwlock *pLock = &pSdb->locks[type];
  // mTrace("sdb table:%d unlock:%p", type, pLock);
  (void)taosThreadRwlockUnlock(pLock);
}
