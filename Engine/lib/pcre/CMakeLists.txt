#
# cMake configuration file for generating Theora library file for Torque3D engine
#

SET( PCRE_SOURCES
    pcrecpp.cc
    pcre_chartables.c
    pcre_compile.c
    pcre_config.c
    pcre_dfa_exec.c
    pcre_exec.c
    pcre_fullinfo.c
    pcre_get.c
    pcre_globals.c
    pcre_info.c
    pcre_maketables.c
    pcre_newline.c
    pcre_ord2utf8.c
    pcre_refcount.c
    pcre_scanner.cc
    pcre_stringpiece.cc
    pcre_study.c
    pcre_tables.c
    pcre_try_flipped.c
    pcre_ucp_searchfuncs.c
    pcre_valid_utf8.c
    pcre_version.c
    pcre_xclass.c
)

SET( PCRE_HEADERS
    config.h
    pcre.h
    pcrecpp.h
    pcrecpparg.h
    pcrecpp_internal.h
    pcreposix.h
    pcre_internal.h
    pcre_scanner.h
    pcre_stringpiece.h
    ucp.h
    ucpinternal.h
    ucptable.h
)

SET( LIB_DIR ${CMAKE_SOURCE_DIR}/Engine/lib )
ADD_DEFINITIONS( -DPCRE_STATIC -DHAVE_CONFIG_H -DUNICODE -D_CRT_SECURE_NO_DEPRECATE -D_CRT_SECURE_NO_WARNINGS )
INCLUDE_DIRECTORIES( ${LIB_DIR}/pcre )
ADD_LIBRARY(pcre ${PCRE_HEADERS} ${PCRE_SOURCES})

# Dushan - NOTE to myself 
#        - test this on MacOS and FreeBSD
IF(UNIX)
    SET_TARGET_PROPERTIES(pcre PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS} -fPIC")
ENDIF(UNIX)
