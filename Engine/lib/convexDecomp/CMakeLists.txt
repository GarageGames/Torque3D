#
# cMake configuration file for generating Convex Decomposition library fils for Torque3D engine
#

SET( CONVEXDECOMP_SOURCES
    NvConcavityVolume.cpp
    NvConvexDecomposition.cpp
    NvFloatMath.cpp
    NvMeshIslandGeneration.cpp
    NvRayCast.cpp
    NvRemoveTjunctions.cpp
    NvSplitMesh.cpp
    NvStanHull.cpp
    NvThreadConfig.cpp
    wavefront.cpp
)
SET( CONVEXDECOMP_HEADERS
    NvConcavityVolume.h
    NvConvexDecomposition.h
    NvFloatMath.h
    NvHashMap.h
    NvMeshIslandGeneration.h
    NvRayCast.h
    NvRemoveTjunctions.h
    NvSimpleTypes.h
    NvSplitMesh.h
    NvStanHull.h
    NvThreadConfig.h
    NvUserMemAlloc.h
    wavefront.h
)

ADD_DEFINITIONS( -DWIN32 -DUNICODE -D_CRT_SECURE_NO_DEPRECATE -D_CRT_SECURE_NO_WARNINGS )
ADD_LIBRARY( convexDecomp ${CONVEXDECOMP_HEADERS} ${CONVEXDECOMP_SOURCES} )

# Dushan - NOTE to myself 
#        - test this on MacOS and FreeBSD
IF( UNIX )
    SET_TARGET_PROPERTIES(convexDecomp PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS} -fPIC")
ENDIF( UNIX )