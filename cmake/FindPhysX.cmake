#Dushan - find nVidia PhysX SDK version 2.8.4

SET( PHYSX_FOUND FALSE )
SET( PHYSX_INCLUDE_DIR "NOTFOUND" )
SET( PHYSX_LIBRARIES "NOTFOUND" )

IF( WIN32 )
		FIND_PATH( PHYSX_SDK_DIR SDKs//Physics//include//NxPhysics.h PATHS
		    "$ENV{PROGRAMFILES}//NVIDIA Corporation//NVIDIA PhysX SDK//v2.8.4_win"
		    "$ENV{PROGRAMW6432}//NVIDIA Corporation//NVIDIA PhysX SDK//v2.8.4_win"
		    "$ENV{PROGRAMFILES}//NVIDIA Corporation//NVIDIA PhysX SDK//v2.8.3"
		    "$ENV{PROGRAMW6432}//NVIDIA Corporation//NVIDIA PhysX SDK//v2.8.3"
	    )

	IF(PHYSX_SDK_DIR)
		SET( PHYSX_FOUND TRUE )
		SET( PHYSX_BINARY_DIR "${PHYSX_SDK_DIR}//Bin//win32" )
		SET( PHYSX_LIBRARY_DIR "${PHYSX_SDK_DIR}//SDKs//lib//Win32" )
		SET( PHYSX_INCLUDE_DIR "${PHYSX_SDK_DIR}//SDKs//Physics//include" )
		SET( PHYSX_LOADER_INCLUDE_DIR "${PHYSX_SDK_DIR}//SDKs//PhysXLoader//include" )
		SET( PHYSX_FOUNDATION_INCLUDE_DIR "${PHYSX_SDK_DIR}//SDKs//Foundation//include" )
		SET( PHYSX_COOKING_INCLUDE_DIR "${PHYSX_SDK_DIR}//SDKs//Cooking//include" )
		SET( PHYSX_CHARACTER_INCLUDE_DIR "${PHYSX_SDK_DIR}//SDKs//NxCharacter//include" )
		#Dushan
		#Torque3D need NxuStream from PhysX Tools folder
		SET( PHYSX_NXUSTREAM2_INCLUDE_DIR "${PHYSX_SDK_DIR}//Tools//NxuStream2" )
		
		FIND_LIBRARY( PHYSX_LOADER_LIBRARY PhysXLoader.lib PATHS "${PHYSX_SDK_DIR}//SDKs//lib//Win32" )
		FIND_LIBRARY( PHYSX_COOKING_LIBRARY NAMES PhysXCooking.lib PATHS "${PHYSX_SDK_DIR}//SDKs//lib//Win32" )
		FIND_LIBRARY( PHYSX_CHARACTER_LIBRARY NAMES NxCharacter.lib PATHS "${PHYSX_SDK_DIR}//SDKs//lib//Win32" )
		SET( PHYSX_LIBRARIES ${PHYSX_LOADER_LIBRARY} ${PHYSX_COOKING_LIBRARY} ${PHYSX_CHARACTER_LIBRARY} )
		SET( PHYSX_INCLUDE_DIRS ${PHYSX_LOADER_INCLUDE_DIR} ${PHYSX_INCLUDE_DIR} ${PHYSX_FOUNDATION_INCLUDE_DIR} ${PHYSX_COOKING_INCLUDE_DIR} ${PHYSX_CHARACTER_INCLUDE_DIR} ${PHYSX_NXUSTREAM2_INCLUDE_DIR} )
	ELSE( PHYSX_SDK_DIR )
		SET( PHYSX_FOUND FALSE )
	ENDIF( PHYSX_SDK_DIR )

ENDIF( WIN32 )

IF( PHYSX_FOUND )
   MESSAGE( STATUS "Found nVidia PhysX" )
ELSE(PHYSX_FOUND )
   MESSAGE( FATAL_ERROR "Could NOT find nVidia PhysX. Please make sure the PhysX SDK v2.8.3 or higher is installed in it's default location." )
ENDIF( PHYSX_FOUND )