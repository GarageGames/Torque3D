#
# cMake configuration file for generating Zlib library file for Torque3D engine
#

SET( ZLIB_SOURCES
    adler32.c
    compress.c
    crc32.c
    deflate.c
	gzclose.c
	gzread.c
	gzwrite.c
    infback.c
    inffast.c
    inflate.c
    inftrees.c
    trees.c
    uncompr.c
    zutil.c
)

SET( ZLIB_HEADERS
    crc32.h
    deflate.h
	gzguts.h
    inffast.h
    inffixed.h
    inflate.h
    inftrees.h
    trees.h
    zconf.h
    zlib.h
    zutil.h
)

ADD_DEFINITIONS( -DUNICODE -D_CRT_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_DEPRECATE )
ADD_LIBRARY( zlib ${ZLIB_HEADERS} ${ZLIB_SOURCES} )

# Dushan - NOTE to myself 
#        - test this on MacOS and FreeBSD
IF( UNIX )
    SET_TARGET_PROPERTIES( zlib PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS} -fPIC" )
ENDIF( UNIX )
