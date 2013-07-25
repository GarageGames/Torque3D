# - Find vorbis library
# Find the native Vorbis headers and libraries.
#
#  VORBIS_INCLUDE_DIR    - where to find vorbis/vorbis.h, codec.h, etc
#  VORBIS_LIBRARIES      - List of libraries when using libvorbis
#  VORBISFILE_LIBRARY    - Location of vorbisfile library
#  VORBIS_VORBIS_LIBRARY - location of vorbis library
#  VORBIS_FOUND          - True if vorbis is found.


#=============================================================================
#Copyright 2000-2009 Kitware, Inc., Insight Software Consortium
#All rights reserved.
#
#Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
#* Redistributions of source code must retain the above copyright notice, 
#this list of conditions and the following disclaimer.
#
#* Redistributions in binary form must reproduce the above copyright notice, 
#this list of conditions and the following disclaimer in the documentation 
#and/or other materials provided with the distribution.
#
#* Neither the names of Kitware, Inc., the Insight Software Consortium, nor 
#the names of their contributors may be used to endorse or promote products 
#derived from this software without specific prior written  permission.
#
#THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
#AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
#IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
#ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
#LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
#CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
#SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
#INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
#CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
#ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
#POSSIBILITY OF SUCH DAMAGE.
#=============================================================================

# Look for the vorbisfile header file.
FIND_PATH(VORBIS_INCLUDE_DIR NAMES vorbis/vorbisfile.h)
MARK_AS_ADVANCED(VORBIS_INCLUDE_DIR)

# Look for the vorbisfile library.
FIND_LIBRARY(VORBISFILE_LIBRARY NAMES vorbisfile )
MARK_AS_ADVANCED(VORBISFILE_LIBRARY)

# Look for the vorbis library.
FIND_LIBRARY(VORBIS_VORBIS_LIBRARY NAMES vorbis )
MARK_AS_ADVANCED(VORBIS_VORBIS_LIBRARY)



# handle the QUIETLY and REQUIRED arguments and set VORBISFILE_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(${CMAKE_ROOT}/Modules/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Vorbis DEFAULT_MSG VORBIS_VORBIS_LIBRARY VORBIS_INCLUDE_DIR)

SET(VORBIS_LIBRARIES ${VORBISFILE_LIBRARY} ${VORBIS_VORBIS_LIBRARY})
SET(VORBIS_LIBRARY ${VORBIS_LIBRARIES}) #FIXME This should be removed
SET(VORBIS_INCLUDE_DIRS ${VORBIS_INCLUDE_DIR})
