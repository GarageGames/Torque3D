#
# cMake configuration file for generating nvuCharacter library files for Torque3D engine
# 

SET( PHYSX_CHARACTER_SOURCE_DIR "${PHYSX_SDK_DIR}//SDKs//NxCharacter" )

SET( PHYSX_INCLUDE_DIR "${PHYSX_SDK_DIR}//SDKs//Physics//include" )
SET( PHYSX_LOADER_INCLUDE_DIR "${PHYSX_SDK_DIR}//SDKs//PhysXLoader//include" )
SET( PHYSX_FOUNDATION_INCLUDE_DIR "${PHYSX_SDK_DIR}//SDKs//Foundation//include" )
SET( PHYSX_COOKING_INCLUDE_DIR "${PHYSX_SDK_DIR}//SDKs//Cooking//include" )
SET( PHYSX_CHARACTER_INCLUDE_DIR "${PHYSX_SDK_DIR}//SDKs//NxCharacter//include" )

SET( NXUCHARACTER_SOURCES
    ${PHYSX_CHARACTER_SOURCE_DIR}/include/BoxController.h
	${PHYSX_CHARACTER_SOURCE_DIR}/include/CapsuleController.h
	${PHYSX_CHARACTER_SOURCE_DIR}/include/CCTAllocator.h
	${PHYSX_CHARACTER_SOURCE_DIR}/include/CCTDebugRenderer.h
	${PHYSX_CHARACTER_SOURCE_DIR}/include/CharacterController.h
	${PHYSX_CHARACTER_SOURCE_DIR}/include/CharacterControllerManager.h
	${PHYSX_CHARACTER_SOURCE_DIR}/include/Controller.h
	${PHYSX_CHARACTER_SOURCE_DIR}/include/ControllerManager.h
	${PHYSX_CHARACTER_SOURCE_DIR}/include/NxBoxController.h
	${PHYSX_CHARACTER_SOURCE_DIR}/include/NxCapsuleController.h
	${PHYSX_CHARACTER_SOURCE_DIR}/include/NxCharacter.h
	${PHYSX_CHARACTER_SOURCE_DIR}/include/NxController.h
	${PHYSX_CHARACTER_SOURCE_DIR}/include/NxControllerManager.h
	${PHYSX_CHARACTER_SOURCE_DIR}/include/NxExtended.h
	${PHYSX_CHARACTER_SOURCE_DIR}/include/SweptBox.h
	${PHYSX_CHARACTER_SOURCE_DIR}/include/SweptCapsule.h
	${PHYSX_CHARACTER_SOURCE_DIR}/include/SweptVolume.h
)

SET( NXUCHARACTER_HEADERS
    ${PHYSX_CHARACTER_SOURCE_DIR}/src/BoxController.cpp
	${PHYSX_CHARACTER_SOURCE_DIR}/src/CapsuleController.cpp
	${PHYSX_CHARACTER_SOURCE_DIR}/src/CCTDebugRenderer.cpp
	${PHYSX_CHARACTER_SOURCE_DIR}/src/CharacterController.cpp
	${PHYSX_CHARACTER_SOURCE_DIR}/src/CharacterControllerCallbacks.cpp
	${PHYSX_CHARACTER_SOURCE_DIR}/src/CharacterControllerManager.cpp
	${PHYSX_CHARACTER_SOURCE_DIR}/src/Controller.cpp
	${PHYSX_CHARACTER_SOURCE_DIR}/src/NxCharacter.cpp
	${PHYSX_CHARACTER_SOURCE_DIR}/src/SweptBox.cpp
	${PHYSX_CHARACTER_SOURCE_DIR}/src/SweptCapsule.cpp
	${PHYSX_CHARACTER_SOURCE_DIR}/src/SweptVolume.cpp
)

ADD_DEFINITIONS( -DWIN32 -DNXCHARACTER_STATIC -DUNICODE -D_CRT_SECURE_NO_DEPRECATE -D_CRT_SECURE_NO_WARNINGS )
INCLUDE_DIRECTORIES( ${PHYSX_LOADER_INCLUDE_DIR} ${PHYSX_INCLUDE_DIR} ${PHYSX_FOUNDATION_INCLUDE_DIR} ${PHYSX_COOKING_INCLUDE_DIR} ${PHYSX_CHARACTER_INCLUDE_DIR} )
ADD_LIBRARY( nxuCharacter ${NXUCHARACTER_HEADERS} ${NXUCHARACTER_SOURCES} )

# Dushan - NOTE to myself 
#        - test this on MacOS and FreeBSD
IF( UNIX )
    SET_TARGET_PROPERTIES(nxuCharacter PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS} -fPIC")
ENDIF( UNIX )