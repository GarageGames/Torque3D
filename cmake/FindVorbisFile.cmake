# Finds Vorbis File library
#
#  VorbisFile_INCLUDE_DIR - where to find vorbisfile.h, etc.
#  VorbisFile_LIBRARIES   - List of libraries when using Vorbis.
#  VorbisFile_FOUND       - True if Vorbis File found.
#
# $VORBISDIR is an environment variable used
# for finding Vorbis.
#

if (VorbisFile_INCLUDE_DIR)
  # Already in cache, be silent
  set(VorbisFile_FIND_QUIETLY TRUE)
endif (VorbisFile_INCLUDE_DIR)

find_path(VorbisFile_INCLUDE_DIR vorbis/vorbisfile.h
  /opt/local/include
  /usr/local/include
  /usr/include
  $ENV{VORBISDIR}
  $ENV{VORBISDIR}/include
)

set(VorbisFile_NAMES vorbisfile)
find_library(VorbisFile_LIBRARY
  NAMES ${VorbisFile_NAMES}
  PATHS /usr/lib /usr/local/lib /opt/local/lib $ENV{VORBISDIR} $ENV{VORBISDIR}/lib
)

if (VorbisFile_INCLUDE_DIR AND VorbisFile_LIBRARY)
   set(VorbisFile_FOUND TRUE)
   set( VorbisFile_LIBRARIES ${VorbisFile_LIBRARY} )
else (VorbisFile_INCLUDE_DIR AND VorbisFile_LIBRARY)
   set(VorbisFile_FOUND FALSE)
   set(VorbisFile_LIBRARIES)
endif (VorbisFile_INCLUDE_DIR AND VorbisFile_LIBRARY)

if (VorbisFile_FOUND)
   if (NOT VorbisFile_FIND_QUIETLY)
      message(STATUS "Found VorbisFile: ${VorbisFile_LIBRARY}")
   endif (NOT VorbisFile_FIND_QUIETLY)
else (VorbisFile_FOUND)
   if (VorbisFile_FIND_REQUIRED)
      message(STATUS "Looked for Vorbis File libraries named ${VorbisFile_NAMES}.")
      message(STATUS "Include file detected: [${VorbisFile_INCLUDE_DIR}].")
      message(STATUS "Lib file detected: [${VorbisFile_LIBRARY}].")
      message(FATAL_ERROR "=========> Could NOT find Vorbis File library")
   endif (VorbisFile_FIND_REQUIRED)
endif (VorbisFile_FOUND)

mark_as_advanced(
  VorbisFile_LIBRARY
  VorbisFile_INCLUDE_DIR
  )
