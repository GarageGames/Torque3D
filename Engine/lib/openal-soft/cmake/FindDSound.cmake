# - Find DirectSound includes and libraries
#
#   DSOUND_FOUND        - True if DSOUND_INCLUDE_DIR & DSOUND_LIBRARY are found
#   DSOUND_LIBRARIES    - Set when DSOUND_LIBRARY is found
#   DSOUND_INCLUDE_DIRS - Set when DSOUND_INCLUDE_DIR is found
#
#   DSOUND_INCLUDE_DIR - where to find dsound.h, etc.
#   DSOUND_LIBRARY     - the dsound library
#

find_path(DSOUND_INCLUDE_DIR
          NAMES dsound.h
          PATHS "${DXSDK_DIR}"
          PATH_SUFFIXES include
          DOC "The DirectSound include directory"
)

find_library(DSOUND_LIBRARY
             NAMES dsound
             PATHS "${DXSDK_DIR}"
             PATH_SUFFIXES lib lib/x86 lib/x64
             DOC "The DirectSound library"
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(DSound
    REQUIRED_VARS DSOUND_LIBRARY DSOUND_INCLUDE_DIR
)

if(DSOUND_FOUND)
    set(DSOUND_LIBRARIES ${DSOUND_LIBRARY})
    set(DSOUND_INCLUDE_DIRS ${DSOUND_INCLUDE_DIR})
endif()

mark_as_advanced(DSOUND_INCLUDE_DIR DSOUND_LIBRARY)
