#
# cMake configuration file for generating TinyXML library file for Torque3D engine
#

SET( TINYXML_SOURCES
    tinyxml.cpp
    tinyxmlerror.cpp
    tinyxmlparser.cpp
    tinystr.cpp
)

SET( TINYXML_HEADERS
    tinyxml.h
    tinystr.h
)

ADD_DEFINITIONS( -DUNICODE -D_CRT_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_DEPRECATE )
ADD_LIBRARY( tinyxml ${TINYXML_HEADERS} ${TINYXML_SOURCES} )

# Dushan - NOTE to myself 
#        - test this on MacOS and FreeBSD
IF( UNIX )
    SET_TARGET_PROPERTIES( tinyxml PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS} -fPIC" )
ENDIF( UNIX )
