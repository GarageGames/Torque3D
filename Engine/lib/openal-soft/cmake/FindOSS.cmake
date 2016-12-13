# - Find OSS includes
#
#   OSS_FOUND        - True if OSS_INCLUDE_DIR is found
#   OSS_INCLUDE_DIRS - Set when OSS_INCLUDE_DIR is found
#
#   OSS_INCLUDE_DIR - where to find sys/soundcard.h, etc.
#

find_path(OSS_INCLUDE_DIR
          NAMES sys/soundcard.h
          DOC "The OSS include directory"
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OSS  REQUIRED_VARS OSS_INCLUDE_DIR)

if(OSS_FOUND)
    set(OSS_INCLUDE_DIRS ${OSS_INCLUDE_DIR})
endif()

mark_as_advanced(OSS_INCLUDE_DIR)
