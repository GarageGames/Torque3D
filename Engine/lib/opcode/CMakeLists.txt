#
# cMake configuration file for generating Opcode library file for Torque3D engine
#

SET( OPCODE_SOURCES
    Opcode.cpp
    OPC_AABBCollider.cpp
    OPC_AABBTree.cpp
    OPC_BaseModel.cpp
    OPC_BoxPruning.cpp
    OPC_Collider.cpp
    OPC_Common.cpp
    OPC_HybridModel.cpp
    OPC_LSSCollider.cpp
    OPC_MeshInterface.cpp
    OPC_Model.cpp
    OPC_OBBCollider.cpp
    OPC_OptimizedTree.cpp
    OPC_Picking.cpp
    OPC_PlanesCollider.cpp
    OPC_RayCollider.cpp
    OPC_SphereCollider.cpp
    OPC_SweepAndPrune.cpp
    OPC_TreeBuilders.cpp
    OPC_TreeCollider.cpp
    OPC_VolumeCollider.cpp
    Ice/IceAABB.cpp
    Ice/IceContainer.cpp
    Ice/IceHPoint.cpp
    Ice/IceIndexedTriangle.cpp
    Ice/IceMatrix3x3.cpp
    Ice/IceMatrix4x4.cpp
    Ice/IceOBB.cpp
    Ice/IcePlane.cpp
    Ice/IcePoint.cpp
    Ice/IceRandom.cpp
    Ice/IceRay.cpp
    Ice/IceRevisitedRadix.cpp
    Ice/IceSegment.cpp
    Ice/IceTriangle.cpp
    Ice/IceUtils.cpp
)

SET( OPCODE_HEADERS
    Opcode.h
    OPC_AABBCollider.h
    OPC_AABBTree.h
    OPC_BaseModel.h
    OPC_BoxBoxOverlap.h
    OPC_BoxPruning.h
    OPC_Collider.h
    OPC_Common.h
    OPC_HybridModel.h
    OPC_IceHook.h
    OPC_LSSAABBOverlap.h
    OPC_LSSCollider.h
    OPC_LSSTriOverlap.h
    OPC_MeshInterface.h
    OPC_Model.h
    OPC_OBBCollider.h
    OPC_OptimizedTree.h
    OPC_Picking.h
    OPC_PlanesAABBOverlap.h
    OPC_PlanesCollider.h
    OPC_PlanesTriOverlap.h
    OPC_RayAABBOverlap.h
    OPC_RayCollider.h
    OPC_RayTriOverlap.h
    OPC_Settings.h
    OPC_SphereAABBOverlap.h
    OPC_SphereCollider.h
    OPC_SphereTriOverlap.h
    OPC_SweepAndPrune.h
    OPC_TreeBuilders.h
    OPC_TreeCollider.h
    OPC_TriBoxOverlap.h
    OPC_TriTriOverlap.h
    OPC_VolumeCollider.h
    Ice/IceAABB.h
    Ice/IceAxes.h
    Ice/IceBoundingSphere.h
    Ice/IceContainer.h
    Ice/IceFPU.h
    Ice/IceHPoint.h
    Ice/IceIndexedTriangle.h
    Ice/IceLSS.h
    Ice/IceMatrix3x3.h
    Ice/IceMatrix4x4.h
    Ice/IceMemoryMacros.h
    Ice/IceOBB.h
    Ice/IcePairs.h
    Ice/IcePlane.h
    Ice/IcePoint.h
    Ice/IcePreprocessor.h
    Ice/IceRandom.h
    Ice/IceRay.h
    Ice/IceRevisitedRadix.h
    Ice/IceSegment.h
    Ice/IceTriangle.h
    Ice/IceTriList.h
    Ice/IceTypes.h
    Ice/IceUtils.h
)

ADD_DEFINITIONS( -DTORQUE_OPCODE -DICE_NO_DLL -DUNICODE -D_CRT_SECURE_NO_DEPRECATE -D_CRT_SECURE_NO_WARNINGS )
ADD_LIBRARY( opcode ${OPCODE_HEADERS} ${OPCODE_SOURCES} )

# Dushan - NOTE to myself 
#        - test this on MacOS and FreeBSD
IF( UNIX )
    SET_TARGET_PROPERTIES( opcode PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS} -fPIC" )
ENDIF( UNIX )
