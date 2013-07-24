#
# cMake configuration file for generating Opcode library file for Torque3D engine
#

SET( LIBUNGIF_SOURCES
    dgif_lib.c
    gifalloc.c
    gif_err.c
)
SET( LIBUNGIF_HEADERS
    gif_lib.h
    gif_lib_private.h
)

ADD_DEFINITIONS( -D_GBA_NO_FILEIO -DUNICODE -D_CRT_SECURE_NO_DEPRECATE -D_CRT_SECURE_NO_WARNINGS )
# Dushan - make this friendly name as in VS configuration provided before by projectGenerator
ADD_LIBRARY( lungif ${LIBUNGIF_HEADERS} ${LIBUNGIF_SOURCES} )

# Dushan - NOTE to myself 
#        - test this on MacOS and FreeBSD
IF( UNIX )
    SET_TARGET_PROPERTIES( lungif PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS} -fPIC" )
ENDIF( UNIX )
