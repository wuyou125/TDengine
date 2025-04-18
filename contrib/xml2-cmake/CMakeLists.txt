set(LIBXML2_SOURCE_DIR "${TD_CONTRIB_DIR}/libxml2")

set(SRCS
    "${LIBXML2_SOURCE_DIR}/SAX.c"
    "${LIBXML2_SOURCE_DIR}/entities.c"
    "${LIBXML2_SOURCE_DIR}/encoding.c"
    "${LIBXML2_SOURCE_DIR}/error.c"
    "${LIBXML2_SOURCE_DIR}/parserInternals.c"
    "${LIBXML2_SOURCE_DIR}/parser.c"
    "${LIBXML2_SOURCE_DIR}/tree.c"
    "${LIBXML2_SOURCE_DIR}/hash.c"
    "${LIBXML2_SOURCE_DIR}/list.c"
    "${LIBXML2_SOURCE_DIR}/xmlIO.c"
    "${LIBXML2_SOURCE_DIR}/xmlmemory.c"
    "${LIBXML2_SOURCE_DIR}/uri.c"
    "${LIBXML2_SOURCE_DIR}/valid.c"
    "${LIBXML2_SOURCE_DIR}/xlink.c"
    "${LIBXML2_SOURCE_DIR}/HTMLparser.c"
    "${LIBXML2_SOURCE_DIR}/HTMLtree.c"
    "${LIBXML2_SOURCE_DIR}/debugXML.c"
    "${LIBXML2_SOURCE_DIR}/xpath.c"
    "${LIBXML2_SOURCE_DIR}/xpointer.c"
    "${LIBXML2_SOURCE_DIR}/xinclude.c"
    "${LIBXML2_SOURCE_DIR}/nanohttp.c"
    "${LIBXML2_SOURCE_DIR}/nanoftp.c"
    "${LIBXML2_SOURCE_DIR}/catalog.c"
    "${LIBXML2_SOURCE_DIR}/globals.c"
    "${LIBXML2_SOURCE_DIR}/threads.c"
    "${LIBXML2_SOURCE_DIR}/c14n.c"
    "${LIBXML2_SOURCE_DIR}/xmlstring.c"
    "${LIBXML2_SOURCE_DIR}/buf.c"
    "${LIBXML2_SOURCE_DIR}/xmlregexp.c"
    "${LIBXML2_SOURCE_DIR}/xmlschemas.c"
    "${LIBXML2_SOURCE_DIR}/xmlschemastypes.c"
    "${LIBXML2_SOURCE_DIR}/xmlunicode.c"
    "${LIBXML2_SOURCE_DIR}/triostr.c"
    "${LIBXML2_SOURCE_DIR}/xmlreader.c"
    "${LIBXML2_SOURCE_DIR}/relaxng.c"
    "${LIBXML2_SOURCE_DIR}/dict.c"
    "${LIBXML2_SOURCE_DIR}/SAX2.c"
    "${LIBXML2_SOURCE_DIR}/xmlwriter.c"
    "${LIBXML2_SOURCE_DIR}/legacy.c"
    "${LIBXML2_SOURCE_DIR}/chvalid.c"
    "${LIBXML2_SOURCE_DIR}/pattern.c"
    "${LIBXML2_SOURCE_DIR}/xmlsave.c"
    "${LIBXML2_SOURCE_DIR}/xmlmodule.c"
    "${LIBXML2_SOURCE_DIR}/schematron.c"
    "${LIBXML2_SOURCE_DIR}/xzlib.c"
)
add_library(_libxml2 ${SRCS})

#target_link_libraries(_libxml2 PRIVATE td_contrib::zlib)
target_link_libraries(_libxml2 PRIVATE zlib)

target_include_directories(_libxml2 BEFORE PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/linux_x86_64/include")
target_include_directories(_libxml2 BEFORE PUBLIC "${LIBXML2_SOURCE_DIR}/include")

add_library(td_contrib::libxml2 ALIAS _libxml2)
