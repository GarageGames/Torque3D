#
# cMake configuration file for generating TinyXML library file for Torque3D engine
#

SET( SQUISH_SOURCES
    alpha.cpp
    clusterfit.cpp
    colourblock.cpp
    colourfit.cpp
    colourset.cpp
    maths.cpp
    rangefit.cpp
    singlecolourfit.cpp
    squish.cpp
)

SET( SQUISH_HEADERS
    alpha.h
    clusterfit.h
    colourblock.h
    colourfit.h
    colourset.h
    config.h
    maths.h
    rangefit.h
    simd.h
    simd_float.h
    simd_sse.h
    simd_ve.h
    singlecolourfit.h
    squish.h
)

SET( LIB_DIR ${CMAKE_SOURCE_DIR}/Engine/lib )
ADD_DEFINITIONS( -DPCRE_STATIC -DHAVE_CONFIG_H -DUNICODE -D_CRT_SECURE_NO_DEPRECATE -D_CRT_SECURE_NO_WARNINGS )
INCLUDE_DIRECTORIES( ${LIB_DIR}/squish )
ADD_LIBRARY( squish ${SQUISH_HEADERS} ${SQUISH_SOURCES} )

# Dushan - NOTE to myself 
#        - test this on MacOS and FreeBSD
IF( UNIX )
    SET_TARGET_PROPERTIES( squish PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS} -fPIC" )
ENDIF( UNIX )
