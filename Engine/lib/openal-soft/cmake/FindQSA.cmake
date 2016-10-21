# - Find QSA includes and libraries
#
#   QSA_FOUND        - True if QSA_INCLUDE_DIR & QSA_LIBRARY are found
#   QSA_LIBRARIES    - Set when QSA_LIBRARY is found
#   QSA_INCLUDE_DIRS - Set when QSA_INCLUDE_DIR is found
#
#   QSA_INCLUDE_DIR - where to find sys/asoundlib.h, etc.
#   QSA_LIBRARY     - the asound library
#

# Only check for QSA on QNX, because it conflicts with ALSA.
if("${CMAKE_C_PLATFORM_ID}" STREQUAL "QNX")
    find_path(QSA_INCLUDE_DIR
              NAMES sys/asoundlib.h
              DOC "The QSA include directory"
    )

    find_library(QSA_LIBRARY
                 NAMES asound
                 DOC "The QSA library"
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(QSA
    REQUIRED_VARS QSA_LIBRARY QSA_INCLUDE_DIR
)

if(QSA_FOUND)
    set(QSA_LIBRARIES ${QSA_LIBRARY})
    set(QSA_INCLUDE_DIRS ${QSA_INCLUDE_DIR})
endif()

mark_as_advanced(QSA_INCLUDE_DIR QSA_LIBRARY)
