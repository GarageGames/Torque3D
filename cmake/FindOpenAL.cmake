# Locate OpenAL
# This module defines
# OPENAL_LIBRARY
# OPENAL_FOUND, if false, do not try to link to OpenAL 
# OPENAL_INCLUDE_DIR, where to find the headers
#
# $OPENALDIR is an environment variable that would
# correspond to the ./configure --prefix=$OPENALDIR
# used in building OpenAL.
#
# Created by Eric Wing. This was influenced by the FindSDL.cmake module.
# Changed by Dusan Jocic in order to get it working with OpenAL SDK 1.1 (OpenAL11CoreSDK)

#=============================================================================
# Copyright 2005-2009 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

# For Windows, OpenAL would be included like
# #include <al.h>
# For Apple, you would include it like
# #include <OpenAL/al.h>
# Other systems include it like
# #include <AL/al.h>
# The reason for this is that Creative Labs does not by default put their
# headers in AL/ for Windows, and other systems follow the OpenGL convention
# (OpenAL/ for OSX, and AL/ for Linux and other Unixes).
# 
# For Windows, Creative Labs seems to have added a registry key for their 
# OpenAL 1.1 installer. I have added that key to the list of search paths,
# however, the key looks like it could be a little fragile depending on 
# if they decide to change the 1.00.0000 number for bug fix releases.
# Also, they seem to have laid down groundwork for multiple library platforms
# which puts the library in an extra subdirectory. Currently there is only
# Win32 and I have hardcoded that here. This may need to be adjusted as 
# platforms are introduced.
# The OpenAL 1.0 installer doesn't seem to have a useful key I can use.
# I do not know if the Nvidia OpenAL SDK has a registry key.
# 
# For OSX, remember that OpenAL was added by Apple in 10.4 (Tiger). 
# To support the framework, I originally wrote special framework detection 
# code in this module which I have now removed with CMake's introduction
# of native support for frameworks.
# In addition, OpenAL is open source, and it is possible to compile on Panther. 
# Furthermore, due to bugs in the initial OpenAL release, and the 
# transition to OpenAL 1.1, it is common to need to override the built-in
# framework. 
# Per my request, CMake should search for frameworks first in
# the following order:
# ~/Library/Frameworks/OpenAL.framework/Headers
# /Library/Frameworks/OpenAL.framework/Headers
# /System/Library/Frameworks/OpenAL.framework/Headers
#
# On OSX, this will prefer the Framework version (if found) over others.
# People will have to manually change the cache values of 
# OPENAL_LIBRARY to override this selection or set the CMake environment
# CMAKE_INCLUDE_PATH to modify the search paths.

IF(WIN32)
    SET(AL_HEADER_PREFIX "")
ELSEIF(APPLE)
    SET(AL_HEADER_PREFIX "OpenAL/")
ELSE(WIN32)
    SET(AL_HEADER_PREFIX "AL/")
ENDIF(WIN32)

FIND_PATH(OPENAL_INCLUDE_DIR "${AL_HEADER_PREFIX}al.h"
  HINTS
  $ENV{OPENALDIR}
  PATH_SUFFIXES include include/AL
  PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /sw # Fink
  /opt/local # DarwinPorts
  /opt/csw # Blastwave
  /opt
  #Dushan - moved to Wow6432Node on Windows x64
  #       - maybe we need to change that in order to work on Windows x86 (not sure)  
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\Creative\ Labs\\OpenAL\ 1.1\ Software\ Development\ Kit\\3.05;InstallDir]
  #Dushan - Windows x86
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Creative\ Labs\\OpenAL\ 1.1\ Software\ Development\ Kit\\3.05;InstallDir]
)

FIND_LIBRARY(OPENAL_LIBRARY 
  NAMES OpenAL al openal OpenAL32
  HINTS
  $ENV{OPENALDIR}
  PATH_SUFFIXES lib64 lib libs64 libs libs/Win32 libs/Win64
  PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /sw
  /opt/local
  /opt/csw
  /opt
  #Dushan - moved to Wow6432Node on Windows x64
  #       - maybe we need to change that in order to automatically work on Windows x86 (not sure)
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\Creative\ Labs\\OpenAL\ 1.1\ Software\ Development\ Kit\\3.05;InstallDir]
  #Dushan - Windows x86
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Creative\ Labs\\OpenAL\ 1.1\ Software\ Development\ Kit\\3.05;InstallDir]  
)


# handle the QUIETLY and REQUIRED arguments and set OPENAL_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(OpenAL  DEFAULT_MSG  OPENAL_LIBRARY OPENAL_INCLUDE_DIR)

MARK_AS_ADVANCED(OPENAL_LIBRARY OPENAL_INCLUDE_DIR)
