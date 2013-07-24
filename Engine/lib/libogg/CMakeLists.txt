#
# cMake configuration file for generating Theora library file for Torque3D engine
#

SET( OGG_SOURCES
    src/bitwise.c
    src/framing.c
)

SET( OGG_HEADERS
    include/ogg/ogg.h
    include/ogg/os_types.h
)

SET( LIB_DIR ${CMAKE_SOURCE_DIR}/Engine/lib )
ADD_DEFINITIONS( -DUNICODE -D_CRT_SECURE_NO_DEPRECATE -D_CRT_SECURE_NO_WARNINGS )
INCLUDE_DIRECTORIES( ${LIB_DIR}/libogg/include )

# Dushan - make this friendly name as in VS configuration provided before by projectGenerator
ADD_LIBRARY( libogg ${OGG_HEADERS} ${OGG_SOURCES} )

# Dushan - NOTE to myself 
#        - test this on MacOS and FreeBSD
#        - Why we need this anyway on UNIX systems?
#        - We can simply use FindOGG.cmake file
IF( UNIX )
    SET_TARGET_PROPERTIES(libogg PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS} -fPIC")
ENDIF( UNIX )
