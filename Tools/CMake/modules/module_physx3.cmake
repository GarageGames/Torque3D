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

# module Physx 3.4

option(TORQUE_PHYSICS_PHYSX3 "Use PhysX 3.4 physics" OFF)

if( NOT TORQUE_PHYSICS_PHYSX3 )
    return()
endif()

if("${PHYSX3_BASE_PATH}" STREQUAL "")
   set(PHYSX3_BASE_PATH "" CACHE PATH "PhysX 3.4 path" FORCE)
endif()

#still no path we can't go any further
if("${PHYSX3_BASE_PATH}" STREQUAL "")
   message(FATAL_ERROR "No PhysX path selected")
   return()
endif()

#set physx path
set(PHYSX3_PATH "${PHYSX3_BASE_PATH}/PhysX_3.4")

#uncomment for debugging
#message(STATUS "PhysX Base path: ${PHYSX3_BASE_PATH}" )
#message(STATUS "PhysX path: ${PHYSX3_PATH}" )

# Windows/ Visual Studio
if(MSVC)
if(TORQUE_CPU_X32)
   if(MSVC11)
      set(PHYSX3_LIBPATH_PREFIX vc11win32)
   elseif(MSVC12)
      set(PHYSX3_LIBPATH_PREFIX vc12win32)
   elseif(MSVC14)
      set(PHYSX3_LIBPATH_PREFIX vc14win32)
   else()
      message(FATAL_ERROR "This version of VS is not supported")
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
      message(FATAL_ERROR "This version of VS is not supported")
      return()
   endif()
set(PHYSX3_LIBNAME_POSTFIX _x64)

endif()
endif()

# Lets avoid UNIX variable because that could be osx or even cygwin on windows
if(APPLE)
   #we only support x64 on macOS
   set(PHYSX3_LIBPATH_PREFIX osx64)
   elseif(UNIX)
   if(TORQUE_CPU_X32)
      set(PHYSX3_LIBPATH_PREFIX linux32)
      set(PHYSX3_LIBNAME_POSTFIX _x86)
   elseif(TORQUE_CPU_X64)
      set(PHYSX3_LIBPATH_PREFIX linux64)
      set(PHYSX3_LIBNAME_POSTFIX _x64)
   endif()
endif()

MACRO(FIND_PHYSX3_LIBRARY VARNAME LIBNAME WITHPOSTFIX SEARCHDIR)

   set(LIBPOSTFIX "")
   if(${WITHPOSTFIX})
      set(LIBPOSTFIX ${PHYSX3_LIBNAME_POSTFIX})
   endif(${WITHPOSTFIX})
   #release
   find_library(PHYSX3_${VARNAME}_LIBRARY NAMES ${LIBNAME}${LIBPOSTFIX}
               PATHS ${SEARCHDIR}${PHYSX3_LIBPATH_PREFIX})
   #debug
   find_library(PHYSX3_${VARNAME}_LIBRARY_DEBUG NAMES ${LIBNAME}DEBUG${LIBPOSTFIX}
               PATHS ${SEARCHDIR}${PHYSX3_LIBPATH_PREFIX})

ENDMACRO()

# Find the Libs
if( WIN32 )
   FIND_PHYSX3_LIBRARY(CORE PhysX3 1 ${PHYSX3_PATH}/lib/)
   FIND_PHYSX3_LIBRARY(COMMON PhysX3Common 1 ${PHYSX3_PATH}/lib/)
   FIND_PHYSX3_LIBRARY(COOKING PhysX3Cooking 1 ${PHYSX3_PATH}/lib/)
   FIND_PHYSX3_LIBRARY(CHARACTER PhysX3CharacterKinematic 1 ${PHYSX3_PATH}/lib/)
   FIND_PHYSX3_LIBRARY(EXTENSIONS PhysX3Extensions 0 ${PHYSX3_PATH}/lib/)
   FIND_PHYSX3_LIBRARY(TASK PxTask 1 ${PHYSX3_BASE_PATH}/PxShared/lib/)
   FIND_PHYSX3_LIBRARY(FOUNDATION PxFoundation 1 ${PHYSX3_BASE_PATH}/PxShared/lib/)
   FIND_PHYSX3_LIBRARY(PVD PxPvdSDK 1 ${PHYSX3_BASE_PATH}/PxShared/lib/)
   if(NOT PHYSX3_CORE_LIBRARY)
      return()
   endif()

   #Add the libs
   set(PHYSX_LIBRARIES
      ${PHYSX3_CORE_LIBRARY}
      ${PHYSX3_COMMON_LIBRARY}
      ${PHYSX3_EXTENSIONS_LIBRARY}
      ${PHYSX3_COOKING_LIBRARY}
      ${PHYSX3_CHARACTER_LIBRARY}
      ${PHYSX3_TASK_LIBRARY}
      ${PHYSX3_PVD_LIBRARY}
      ${PHYSX3_FOUNDATION_LIBRARY}
   )

   set(PHYSX_LIBRARIES_DEBUG
      ${PHYSX3_CORE_LIBRARY_DEBUG}
      ${PHYSX3_COMMON_LIBRARY_DEBUG}
      ${PHYSX3_EXTENSIONS_LIBRARY_DEBUG}
      ${PHYSX3_COOKING_LIBRARY_DEBUG}
      ${PHYSX3_CHARACTER_LIBRARY_DEBUG}
      ${PHYSX3_TASK_LIBRARY_DEBUG}
      ${PHYSX3_PVD_LIBRARY_DEBUG}
      ${PHYSX3_FOUNDATION_LIBRARY_DEBUG}
   )
   #macOS & linux use the same lib names
elseif( UNIX)
   FIND_PHYSX3_LIBRARY(CORE PhysX3 0 ${PHYSX3_PATH}/lib/)
   FIND_PHYSX3_LIBRARY(COMMON PhysX3Common 0 ${PHYSX3_PATH}/lib/)
   FIND_PHYSX3_LIBRARY(EXTENSIONS PhysX3Extensions 0 ${PHYSX3_PATH}/lib/)
   FIND_PHYSX3_LIBRARY(COOKING PhysX3Cooking 0 ${PHYSX3_PATH}/lib/)
   FIND_PHYSX3_LIBRARY(CHARACTER PhysX3CharacterKinematic 0 ${PHYSX3_PATH}/lib/)
   FIND_PHYSX3_LIBRARY(CONTROLLER SimulationController 0 ${PHYSX3_PATH}/lib/)
   FIND_PHYSX3_LIBRARY(SCENEQUERY SceneQuery 0 ${PHYSX3_PATH}/lib/)
   #low level
   FIND_PHYSX3_LIBRARY(LOWLEVEL LowLevel 0 ${PHYSX3_PATH}/lib/)
   FIND_PHYSX3_LIBRARY(LOWLEVEL_DYNAMICS LowLevelDynamics 0 ${PHYSX3_PATH}/lib/)
   FIND_PHYSX3_LIBRARY(LOWLEVEL_AABB LowLevelAABB 0 ${PHYSX3_PATH}/lib/)
   FIND_PHYSX3_LIBRARY(LOWLEVEL_CLOTH LowLevelCloth 0 ${PHYSX3_PATH}/lib/)
   #possible bug, particles should be deprecated in 3.4
   FIND_PHYSX3_LIBRARY(LOWLEVEL_PARTICLES LowLevelParticles 0 ${PHYSX3_PATH}/lib/)
   #shared dir
   FIND_PHYSX3_LIBRARY(TASK PxTask 0 ${PHYSX3_BASE_PATH}/PxShared/lib/)
   FIND_PHYSX3_LIBRARY(FOUNDATION PxFoundation 0 ${PHYSX3_BASE_PATH}/PxShared/lib/)
   FIND_PHYSX3_LIBRARY(PVD PxPvdSDK 0 ${PHYSX3_BASE_PATH}/PxShared/lib/)
   if(NOT PHYSX3_CORE_LIBRARY)
      return()
   endif()

   #Add the libs
   set(PHYSX_LIBRARIES
      ${PHYSX3_COMMON_LIBRARY}
      ${PHYSX3_CORE_LIBRARY}
      ${PHYSX3_COOKING_LIBRARY}
      ${PHYSX3_EXTENSIONS_LIBRARY}
      ${PHYSX3_CHARACTER_LIBRARY}
      ${PHYSX3_FOUNDATION_LIBRARY}
      ${PHYSX3_PVD_LIBRARY}
      ${PHYSX3_CONTROLLER_LIBRARY}
      ${PHYSX3_SCENEQUERY_LIBRARY}
      ${PHYSX3_LOWLEVEL_LIBRARY}
      ${PHYSX3_LOWLEVEL_AABB_LIBRARY}
      ${PHYSX3_LOWLEVEL_DYNAMICS_LIBRARY}
      ${PHYSX3_LOWLEVEL_CLOTH_LIBRARY}
      ${PHYSX3_LOWLEVEL_PARTICLES_LIBRARY}
      ${PHYSX3_TASK_LIBRARY}
   )

   set(PHYSX_LIBRARIES_DEBUG
      ${PHYSX3_COMMON_LIBRARY_DEBUG}
      ${PHYSX3_CORE_LIBRARY_DEBUG}
      ${PHYSX3_COOKING_LIBRARY_DEBUG}
      ${PHYSX3_EXTENSIONS_LIBRARY_DEBUG}
      ${PHYSX3_CHARACTER_LIBRARY_DEBUG}
      ${PHYSX3_FOUNDATION_LIBRARY_DEBUG}
      ${PHYSX3_PVD_LIBRARY_DEBUG}
      ${PHYSX3_CONTROLLER_LIBRARY_DEBUG}
      ${PHYSX3_SCENEQUERY_LIBRARY_DEBUG}
      ${PHYSX3_LOWLEVEL_LIBRARY_DEBUG}
      ${PHYSX3_LOWLEVEL_AABB_LIBRARY_DEBUG}
      ${PHYSX3_LOWLEVEL_DYNAMICS_LIBRARY_DEBUG}
      ${PHYSX3_LOWLEVEL_CLOTH_LIBRARY_DEBUG}
      ${PHYSX3_LOWLEVEL_PARTICLES_LIBRARY_DEBUG}
      ${PHYSX3_TASK_LIBRARY_DEBUG}
   )
endif()

# Defines
addDef( "TORQUE_PHYSICS_PHYSX3" )
addDef( "TORQUE_PHYSICS_ENABLED" )

# Source
addPath( "${srcDir}/T3D/physics/physx3" )

# Includes
addInclude( "${PHYSX3_BASE_PATH}/PxShared/include" )
addInclude( "${PHYSX3_BASE_PATH}/PxShared/src/foundation/include" )
addInclude( "${PHYSX3_BASE_PATH}/PxShared/src/pvd/include" )
addInclude( "${PHYSX3_PATH}/Include" )

# Libs
addLibRelease("${PHYSX_LIBRARIES}")
addLibDebug("${PHYSX_LIBRARIES_DEBUG}")

#Install files - macOS & Linux use static libs,nothing to install
if( WIN32 )
   # File Copy for Release
   INSTALL(FILES "${PHYSX3_PATH}/Bin/${PHYSX3_LIBPATH_PREFIX}/PhysX3${PHYSX3_LIBNAME_POSTFIX}.dll"             DESTINATION "${projectOutDir}" CONFIGURATIONS Release)
   INSTALL(FILES "${PHYSX3_PATH}/Bin/${PHYSX3_LIBPATH_PREFIX}/PhysX3CharacterKinematic${PHYSX3_LIBNAME_POSTFIX}.dll"             DESTINATION "${projectOutDir}" CONFIGURATIONS Release)
   INSTALL(FILES "${PHYSX3_PATH}/Bin/${PHYSX3_LIBPATH_PREFIX}/PhysX3Common${PHYSX3_LIBNAME_POSTFIX}.dll"             DESTINATION "${projectOutDir}" CONFIGURATIONS Release)
   INSTALL(FILES "${PHYSX3_PATH}/Bin/${PHYSX3_LIBPATH_PREFIX}/PhysX3Cooking${PHYSX3_LIBNAME_POSTFIX}.dll"             DESTINATION "${projectOutDir}" CONFIGURATIONS Release)
   INSTALL(FILES "${PHYSX3_BASE_PATH}/PxShared/Bin/${PHYSX3_LIBPATH_PREFIX}/PxFoundation${PHYSX3_LIBNAME_POSTFIX}.dll"             DESTINATION "${projectOutDir}" CONFIGURATIONS Release)
   INSTALL(FILES "${PHYSX3_BASE_PATH}/PxShared/Bin/${PHYSX3_LIBPATH_PREFIX}/PxPvdSDK${PHYSX3_LIBNAME_POSTFIX}.dll"             DESTINATION "${projectOutDir}" CONFIGURATIONS Release)

   # File Copy for Debug
   if(TORQUE_CPU_X32)
      INSTALL(FILES "${PHYSX3_PATH}/Bin/${PHYSX3_LIBPATH_PREFIX}/nvToolsExt32_1.dll"             DESTINATION "${projectOutDir}" CONFIGURATIONS Debug)
   elseif(TORQUE_CPU_X64)
      INSTALL(FILES "${PHYSX3_PATH}/Bin/${PHYSX3_LIBPATH_PREFIX}/nvToolsExt64_1.dll"             DESTINATION "${projectOutDir}" CONFIGURATIONS Debug)
   endif()

   INSTALL(FILES "${PHYSX3_PATH}/Bin/${PHYSX3_LIBPATH_PREFIX}/PhysX3DEBUG${PHYSX3_LIBNAME_POSTFIX}.dll"             DESTINATION "${projectOutDir}" CONFIGURATIONS Debug)
   INSTALL(FILES "${PHYSX3_PATH}/Bin/${PHYSX3_LIBPATH_PREFIX}/PhysX3CharacterKinematicDEBUG${PHYSX3_LIBNAME_POSTFIX}.dll"             DESTINATION "${projectOutDir}" CONFIGURATIONS Debug)
   INSTALL(FILES "${PHYSX3_PATH}/Bin/${PHYSX3_LIBPATH_PREFIX}/PhysX3CommonDEBUG${PHYSX3_LIBNAME_POSTFIX}.dll"             DESTINATION "${projectOutDir}" CONFIGURATIONS Debug)
   INSTALL(FILES "${PHYSX3_PATH}/Bin/${PHYSX3_LIBPATH_PREFIX}/PhysX3CookingDEBUG${PHYSX3_LIBNAME_POSTFIX}.dll"             DESTINATION "${projectOutDir}" CONFIGURATIONS Debug)
   INSTALL(FILES "${PHYSX3_BASE_PATH}/PxShared/Bin/${PHYSX3_LIBPATH_PREFIX}/PxFoundationDEBUG${PHYSX3_LIBNAME_POSTFIX}.dll"             DESTINATION "${projectOutDir}" CONFIGURATIONS Debug)
   INSTALL(FILES "${PHYSX3_BASE_PATH}/PxShared/Bin/${PHYSX3_LIBPATH_PREFIX}/PxPvdSDKDEBUG${PHYSX3_LIBNAME_POSTFIX}.dll"             DESTINATION "${projectOutDir}" CONFIGURATIONS Debug)

endif()
