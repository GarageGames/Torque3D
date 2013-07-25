# Find Sixense
#
# Finds the libraries and header files for the Sixsence SDK
#
# This module defines
# SIXSENCE_FOUND - Sixsence was found
# SIXSENCE_INCLUDE_DIR - Directory containing Sixsence header files
# SIXSENCE_LIBRARY - Library name of Sixsence library
#
# Based on the FindLeap.cmake

# Don't be verbose if previously run successfully
IF(SIXSENCE_INCLUDE_DIR AND SIXSENCE_LIBRARY)
    SET(SIXSENCE_FIND_QUIETLY TRUE)
ENDIF(SIXSENCE_INCLUDE_DIR AND SIXSENCE_LIBRARY)

# Set locations to search
SET(SIXSENCE_INCLUDE_SEARCH_PATHS
	/usr/include
	/usr/local/include
	/opt/sixense/include
	/opt/include 
	#Dushan - check for Sixsence SDK inside Torque 3D library folder
	${ENGINE_DIR}/lib/Sixense/Include INTERNAL
	)
SET(SIXSENCE_LIBRARY_SEARCH_DIRS
	/usr/lib
	/usr/lib64
	/usr/local/lib
	/usr/local/lib64
	/opt/Sixense/lib
	/opt/Sixense/lib64
	#Dushan - check for Sixsence SDK inside Torque 3D library folder
	${ENGINE_DIR}/lib/Sixense/lib/win32/release_dll INTERNAL
	)
SET(SIXSENCE_INC_DIR_SUFFIXES PATH_SUFFIXES sixense)

# Set name of the Sixsence library to use
IF(APPLE)
    SET(SIXSENCE_LIBRARY_NAME libsixense.dylib)
ELSEIF(UNIX)
    SET(SIXSENCE_LIBRARY_NAME sosixense.so)
ELSE(WIN32)
    SET(SIXSENCE_LIBRARY_NAME sixense.lib)
ENDIF(APPLE)

IF(NOT SIXSENCE_FIND_QUIETLY)
    MESSAGE(STATUS "Checking for Sixsence")
ENDIF(NOT SIXSENCE_FIND_QUIETLY)

# Search for header files
FIND_PATH(SIXSENCE_INCLUDE_DIR Sixense.h
    PATHS ${SIXSENCE_INCLUDE_SEARCH_PATHS}
    PATH_SUFFIXES ${SIXSENCE_INC_DIR_SUFFIXES})

# Search for library
FIND_LIBRARY(SIXSENCE_LIBRARY ${SIXSENCE_LIBRARY_NAME}
    PATHS ${SIXSENCE_LIBRARY_SEARCH_DIRS}
    PATH_SUFFIXES ${SIXSENCE_LIB_DIR_SUFFIXES})

SET(SIXSENCE_INCLUDE_DIR ${SIXSENCE_INCLUDE_DIR} CACHE STRING
    "Directory containing Sixsence header files")
SET(SIXSENCE_LIBRARY ${SIXSENCE_LIBRARY} CACHE STRING "Library name of Sixsence library")

IF(SIXSENCE_INCLUDE_DIR AND SIXSENCE_LIBRARY)
    SET(SIXSENCE_FOUND TRUE)
ENDIF(SIXSENCE_INCLUDE_DIR AND SIXSENCE_LIBRARY)

IF(SIXSENCE_FOUND)
    IF(NOT SIXSENCE_FIND_QUIETLY)
        MESSAGE(STATUS " libraries: ${SIXSENCE_LIBRARY}")
        MESSAGE(STATUS " includes: ${SIXSENCE_INCLUDE_DIR}")
    ENDIF(NOT SIXSENCE_FIND_QUIETLY)
ELSE(SIXSENCE_FOUND)
    IF(NOT SIXSENCE_LIBRARY)
        MESSAGE(SEND_ERROR "Sixsence library could not be found.")
    ENDIF(NOT SIXSENCE_LIBRARY)
    IF(NOT SIXSENCE_INCLUDE_DIR)
        MESSAGE(SEND_ERROR "Sixsence include files could not be found.")
    ENDIF(NOT SIXSENCE_INCLUDE_DIR)
ENDIF(SIXSENCE_FOUND)

MARK_AS_ADVANCED(SIXSENCE_INCLUDE_DIR SIXSENCE_LIBRARY)

