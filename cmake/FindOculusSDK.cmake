# - Find OculusSDK
# Find the native OculusSDK headers and libraries.
#
#  OCULUS_SDK_INCLUDE_DIRS - where to find OVR.h, etc.
#  OCULUS_SDK_LIBRARIES    - List of libraries when using OculusSDK.
#  OCULUS_SDK_FOUND        - True if OculusSDK found.

SET(OCULUS_SDK_ROOT_DIR
    "${OCULUS_SDK_ROOT_DIR}"
	${ENGINE_DIR}/lib/OculusSDK/libOVR/Include
	${ENGINE_DIR}/lib/OculusSDK/Include
    CACHE
    PATH
    "Root directory to search for OculusSDK")

# Look for the header file.
FIND_PATH(OCULUS_SDK_INCLUDE_DIRS NAMES OVR.h HINTS 
	${OCULUS_SDK_ROOT_DIR}/LibOVR/Include )

# Determine architecture
IF(CMAKE_SIZEOF_VOID_P MATCHES "8")
    SET(OCULUS_SDK_LIB_ARCH "x86_64" CACHE STRING "library location")
ELSE()
    SET(OCULUS_SDK_LIB_ARCH "i386" CACHE STRING "library location")
ENDIF()
MARK_AS_ADVANCED(OCULUS_SDK_LIB_ARCH)

# Look for the library.
FIND_LIBRARY(OCULUS_SDK_LIBRARY NAMES libovr ovr HINTS ${OCULUS_SDK_ROOT_DIR} 
                                                      ${OCULUS_SDK_ROOT_DIR}/LibOVR/Lib/Win32
                                                      ${OCULUS_SDK_ROOT_DIR}/LibOVR/Lib/Linux/Release/${OCULUS_SDK_LIB_ARCH}
													  #Dushan - check for OculusSDK inside Torque 3D library folder
	                                                  ${ENGINE_DIR}/lib/OculusSDK/libOVR/lib/Win32 INTERNAL
                                                    )

# This will find release lib on Linux if no debug is available - on Linux this is no problem and avoids 
# having to compile in debug when not needed
FIND_LIBRARY(OCULUS_SDK_LIBRARY_DEBUG NAMES libovr${CMAKE_DEBUG_POSTFIX} ovr${CMAKE_DEBUG_POSTFIX} ovr HINTS 
                                                      ${OCULUS_SDK_ROOT_DIR}/LibOVR/Lib/Win32
                                                      ${OCULUS_SDK_ROOT_DIR}/LibOVR/Lib/Linux/Debug/${OCULUS_SDK_LIB_ARCH}
                                                      ${OCULUS_SDK_ROOT_DIR}/LibOVR/Lib/Linux/Release/${OCULUS_SDK_LIB_ARCH}
													  #Dushan - check for OculusSDK inside Torque 3D library folder
	                                                  ${ENGINE_DIR}/lib/OculusSDK/libOVR/lib/Win32 INTERNAL
                                                    )
    
MARK_AS_ADVANCED(OCULUS_SDK_LIBRARY)
MARK_AS_ADVANCED(OCULUS_SDK_LIBRARY_DEBUG)

SET(OCULUS_SDK_LIBRARIES optimized ${OCULUS_SDK_LIBRARY} debug ${OCULUS_SDK_LIBRARY_DEBUG})

# handle the QUIETLY and REQUIRED arguments and set OCULUS_SDK_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(OculusSDK DEFAULT_MSG OCULUS_SDK_LIBRARIES OCULUS_SDK_INCLUDE_DIRS)

MARK_AS_ADVANCED(OCULUS_SDK_LIBRARIES OCULUS_SDK_INCLUDE_DIRS)
