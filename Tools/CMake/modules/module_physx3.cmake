# -----------------------------------------------------------------------------
# Copyright (c) 2015 GarageGames, LLC
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to
# deal in the Software without restriction, including without limitation the
# rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.
# -----------------------------------------------------------------------------

# module Physx 3.3

option(TORQUE_PHYSICS_PHYSX3 "Use PhysX 3.3 physics" OFF)

if( NOT TORQUE_PHYSICS_PHYSX3 )
    return()
endif()

if("${PHYSX3_BASE_PATH}" STREQUAL "")
   set(PHYSX3_BASE_PATH "" CACHE PATH "PhysX 3.3 path" FORCE)
endif()

#still no path we can't go any further
if("${PHYSX3_BASE_PATH}" STREQUAL "")
   message(FATAL_ERROR "No PhysX path selected")
   return()
endif()

#set physx path
set(PHYSX3_PATH "${PHYSX3_BASE_PATH}/PhysXSDK")

# TODO linux support
if(MSVC)
if(TORQUE_CPU_X32)
	if(MSVC11)
		set(PHYSX3_LIBPATH_PREFIX vc11win32)
	elseif(MSVC12)
		set(PHYSX3_LIBPATH_PREFIX vc12win32)
	elseif(MSVC14)
		set(PHYSX3_LIBPATH_PREFIX vc14win32)
	else()
		return()
	endif()	
set(PHYSX3_LIBNAME_POSTFIX _x86)

elseif(TORQUE_CPU_X64)
	if(MSVC11)
		set(PHYSX3_LIBPATH_PREFIX vc11win64)
	elseif(MSVC12)
		set(PHYSX3_LIBPATH_PREFIX vc12win64)
	elseif(MSVC14)
		set(PHYSX3_LIBPATH_PREFIX vc14win64)
	else()
		return()
	endif()	
set(PHYSX3_LIBNAME_POSTFIX _x64)

endif()

endif(MSVC)

MACRO(FIND_PHYSX3_LIBRARY VARNAME LIBNAME WITHPOSTFIX)

    set(LIBPOSTFIX "")
    if(${WITHPOSTFIX})
        set(LIBPOSTFIX ${PHYSX3_LIBNAME_POSTFIX})
    endif(${WITHPOSTFIX})
    find_library(PHYSX3_${VARNAME}_LIBRARY NAMES ${LIBNAME}${LIBPOSTFIX}
                 PATHS ${PHYSX3_PATH}/Lib/${PHYSX3_LIBPATH_PREFIX})
    find_library(PHYSX3_${VARNAME}_LIBRARY_DEBUG NAMES ${LIBNAME}DEBUG${LIBPOSTFIX}
                 PATHS ${PHYSX3_PATH}/Lib/${PHYSX3_LIBPATH_PREFIX})

ENDMACRO(FIND_PHYSX3_LIBRARY VARNAME LIBNAME)

# Find the Libs, we just use the full path to save playing around with link_directories
FIND_PHYSX3_LIBRARY(CORE PhysX3 1)
FIND_PHYSX3_LIBRARY(COMMON PhysX3Common 1)
FIND_PHYSX3_LIBRARY(COOKING PhysX3Cooking 1)
FIND_PHYSX3_LIBRARY(CHARACTER PhysX3CharacterKinematic 1)
FIND_PHYSX3_LIBRARY(EXTENSIONS PhysX3Extensions 0)
FIND_PHYSX3_LIBRARY(TASK PxTask 0)
FIND_PHYSX3_LIBRARY(DEBUGGER PhysXVisualDebuggerSDK 0)
FIND_PHYSX3_LIBRARY(PROFILE PhysXProfileSDK 0)

if(NOT PHYSX3_CORE_LIBRARY)
	return()
endif()

# Defines
addDef( "TORQUE_PHYSICS_PHYSX3" )
addDef( "TORQUE_PHYSICS_ENABLED" )

# Source
addPath( "${srcDir}/T3D/physics/physx3" )

# Includes
addInclude( "${PHYSX3_PATH}/Include" )
addInclude( "${PHYSX3_PATH}/Include/extensions" )
addInclude( "${PHYSX3_PATH}/Include/foundation" )
addInclude( "${PHYSX3_PATH}/Include/characterkinematic" )
addInclude( "${PHYSX3_PATH}/Include/common" )

#Add the libs
set(PHYSX_LIBRARIES_DEBUG
	${PHYSX3_CORE_LIBRARY_DEBUG}
	${PHYSX3_COMMON_LIBRARY_DEBUG}
	${PHYSX3_COOKING_LIBRARY_DEBUG}
	${PHYSX3_CHARACTER_LIBRARY_DEBUG}
	${PHYSX3_EXTENSIONS_LIBRARY_DEBUG}
	${PHYSX3_TASK_LIBRARY_DEBUG}
	${PHYSX3_DEBUGGER_LIBRARY_DEBUG}
	${PHYSX3_PROFILE_LIBRARY_DEBUG}
)

set(PHYSX_LIBRARIES
	${PHYSX3_CORE_LIBRARY}
	${PHYSX3_COMMON_LIBRARY}
	${PHYSX3_COOKING_LIBRARY}
	${PHYSX3_CHARACTER_LIBRARY}
	${PHYSX3_EXTENSIONS_LIBRARY}
	${PHYSX3_TASK_LIBRARY}
	${PHYSX3_DEBUGGER_LIBRARY}
	${PHYSX3_PROFILE_LIBRARY}
)

addLibRelease("${PHYSX_LIBRARIES}")
addLibDebug("${PHYSX_LIBRARIES_DEBUG}")

#Install dll files
if( WIN32 )
	# File Copy for Release   
	INSTALL(FILES "${PHYSX3_PATH}/Bin/${PHYSX3_LIBPATH_PREFIX}/PhysX3${PHYSX3_LIBNAME_POSTFIX}.dll"             DESTINATION "${projectOutDir}" CONFIGURATIONS "Release")
	INSTALL(FILES "${PHYSX3_PATH}/Bin/${PHYSX3_LIBPATH_PREFIX}/PhysX3CharacterKinematic${PHYSX3_LIBNAME_POSTFIX}.dll"             DESTINATION "${projectOutDir}" CONFIGURATIONS "Release")
	INSTALL(FILES "${PHYSX3_PATH}/Bin/${PHYSX3_LIBPATH_PREFIX}/PhysX3Common${PHYSX3_LIBNAME_POSTFIX}.dll"             DESTINATION "${projectOutDir}" CONFIGURATIONS "Release")
	INSTALL(FILES "${PHYSX3_PATH}/Bin/${PHYSX3_LIBPATH_PREFIX}/PhysX3Cooking${PHYSX3_LIBNAME_POSTFIX}.dll"             DESTINATION "${projectOutDir}" CONFIGURATIONS "Release")

	# File Copy for Debug
	if(TORQUE_CPU_X32)
		INSTALL(FILES "${PHYSX3_PATH}/Bin/${PHYSX3_LIBPATH_PREFIX}/nvToolsExt32_1.dll"             DESTINATION "${projectOutDir}" CONFIGURATIONS "Debug")
	elseif(TORQUE_CPU_X64)
		INSTALL(FILES "${PHYSX3_PATH}/Bin/${PHYSX3_LIBPATH_PREFIX}/nvToolsExt64_1.dll"             DESTINATION "${projectOutDir}" CONFIGURATIONS "Debug")
	endif()
	
	INSTALL(FILES "${PHYSX3_PATH}/Bin/${PHYSX3_LIBPATH_PREFIX}/PhysX3DEBUG${PHYSX3_LIBNAME_POSTFIX}.dll"             DESTINATION "${projectOutDir}" CONFIGURATIONS "Debug")
	INSTALL(FILES "${PHYSX3_PATH}/Bin/${PHYSX3_LIBPATH_PREFIX}/PhysX3CharacterKinematicDEBUG${PHYSX3_LIBNAME_POSTFIX}.dll"             DESTINATION "${projectOutDir}" CONFIGURATIONS "Debug")
	INSTALL(FILES "${PHYSX3_PATH}/Bin/${PHYSX3_LIBPATH_PREFIX}/PhysX3CommonDEBUG${PHYSX3_LIBNAME_POSTFIX}.dll"             DESTINATION "${projectOutDir}" CONFIGURATIONS "Debug")
	INSTALL(FILES "${PHYSX3_PATH}/Bin/${PHYSX3_LIBPATH_PREFIX}/PhysX3CookingDEBUG${PHYSX3_LIBNAME_POSTFIX}.dll"             DESTINATION "${projectOutDir}" CONFIGURATIONS "Debug")
	
endif(WIN32)
