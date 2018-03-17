# -----------------------------------------------------------------------------
# Copyright (c) 2014 GarageGames, LLC
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

# module PhysX

option(TORQUE_PHYSX "Enable PhysX module" OFF)

if(TORQUE_PHYSX)
	if(TORQUE_PHYSX_SDK_PATH STREQUAL "")
		set(TORQUE_PHYSX_SDK_PATH "" CACHE PATH "PhysX library path" FORCE)
	endif()
else() # hide variable
    set(TORQUE_PHYSX_SDK_PATH "" CACHE INTERNAL "" FORCE) 
endif()

if(TORQUE_PHYSX)
	# Try to figure out which version of physx this is
	if(EXISTS "${TORQUE_PHYSX_SDK_PATH}/SDKs/")
		set(ISPHYSX3 "FALSE")
	else()
		set(ISPHYSX3 "TRUE")
	endif()
	
	addDef( "TORQUE_PHYSICS_ENABLED" )
	
	if(${ISPHYSX3})
		#PhysX3
		addDef( "TORQUE_PHYSICS_PHYSX3" )
		
		# Source
		addPath( "${srcDir}/T3D/physics/physx3" )
		addPath( "${srcDir}/T3D/physics/physx3/Cloth" )
		addPath( "${srcDir}/T3D/physics/physx3/Particles" )

		# Includes
		addInclude( "${TORQUE_PHYSX_SDK_PATH}/Include" )
		addInclude( "${TORQUE_PHYSX_SDK_PATH}/Include/extensions" )
		addInclude( "${TORQUE_PHYSX_SDK_PATH}/Include/foundation" )
		addInclude( "${TORQUE_PHYSX_SDK_PATH}/Include/characterkinematic" )
		addInclude( "${TORQUE_PHYSX_SDK_PATH}/Include/common" )
		
		# Libraries
		if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
			addLibPath( "${TORQUE_PHYSX_SDK_PATH}/Lib/win64" )
			addLib( "PhysX3_x64.lib" )
			addLib( "PhysX3Common_x64.lib" )
			addLib( "PhysX3Extensions.lib" )
			addLib( "PhysX3Cooking_x64.lib" )
			addLib( "PxTask.lib" )
			addLib( "PhysX3CharacterKinematic_x64.lib" )
			addLib( "PhysXVisualDebuggerSDK.lib" )
			addLib( "PhysXProfileSDK.lib" )
		else()
			addLibPath( "${TORQUE_PHYSX_SDK_PATH}/Lib/win32" )
			addLib( "PhysX3_x86.lib" )
			addLib( "PhysX3Common_x86.lib" )
			addLib( "PhysX3Extensions.lib" )
			addLib( "PhysX3Cooking_x86.lib" )
			addLib( "PxTask.lib" )
			addLib( "PhysX3CharacterKinematic_x86.lib" )
			addLib( "PhysXVisualDebuggerSDK.lib" )
			addLib( "PhysXProfileSDK.lib" )
		endif()
		
		# Install
		if( WIN32 ) 
			# File Copy for Release
			if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
				INSTALL(FILES "${TORQUE_PHYSX_SDK_PATH}/Bin/win64/nvToolsExt64_1.dll"             DESTINATION "${projectOutDir}")
				INSTALL(FILES "${TORQUE_PHYSX_SDK_PATH}/Bin/win64/PhysX3_x64.dll"             DESTINATION "${projectOutDir}")
				INSTALL(FILES "${TORQUE_PHYSX_SDK_PATH}/Bin/win64/PhysX3CharacterKinematic_x64.dll"             DESTINATION "${projectOutDir}")
				INSTALL(FILES "${TORQUE_PHYSX_SDK_PATH}/Bin/win64/PhysX3Common_x64.dll"             DESTINATION "${projectOutDir}")
				INSTALL(FILES "${TORQUE_PHYSX_SDK_PATH}/Bin/win64/PhysX3Cooking_x64.dll"             DESTINATION "${projectOutDir}")
			else()
				INSTALL(FILES "${TORQUE_PHYSX_SDK_PATH}/Bin/win32/nvToolsExt32_1.dll"             DESTINATION "${projectOutDir}")
				INSTALL(FILES "${TORQUE_PHYSX_SDK_PATH}/Bin/win32/PhysX3_x86.dll"             DESTINATION "${projectOutDir}")
				INSTALL(FILES "${TORQUE_PHYSX_SDK_PATH}/Bin/win32/PhysX3CharacterKinematic_x86.dll"             DESTINATION "${projectOutDir}")
				INSTALL(FILES "${TORQUE_PHYSX_SDK_PATH}/Bin/win32/PhysX3Common_x86.dll"             DESTINATION "${projectOutDir}")
				INSTALL(FILES "${TORQUE_PHYSX_SDK_PATH}/Bin/win32/PhysX3Cooking_x86.dll"             DESTINATION "${projectOutDir}")
			endif()
		endif()
	else()
		#PhysX2
		addDef( "TORQUE_PHYSICS_PHYSX" )
		
		# Source
		addPath( "${srcDir}/T3D/physics/physx" )

		# Includes
		addInclude( "${TORQUE_PHYSX_SDK_PATH}/SDKs/Physics/include" )
		addInclude( "${TORQUE_PHYSX_SDK_PATH}/SDKs/Foundation/include" )
		addInclude( "${TORQUE_PHYSX_SDK_PATH}/SDKs/PhysXLoader/include" )
		addInclude( "${TORQUE_PHYSX_SDK_PATH}/SDKs/Cooking/include" )
		addInclude( "${TORQUE_PHYSX_SDK_PATH}/SDKs/NxCharacter/include" )
		addInclude( "${TORQUE_PHYSX_SDK_PATH}/Tools/NxuStream2" )
		
		# Libraries
		if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
			addLibPath( "${TORQUE_PHYSX_SDK_PATH}/SDKs/lib/Win64" )
			addLib( "PhysXCooking64.lib" )
			addLib( "PhysXLoader64.lib" )
		else()
			addLibPath( "${TORQUE_PHYSX_SDK_PATH}/SDKs/lib/Win32" )
			addLib( "PhysXCooking.lib" )
			addLib( "PhysXLoader.lib" )
		endif()
		
		addLib( "nxCharacter" )
		addLib( "nxuStream" )
		
		# Install
		if( WIN32 ) 
			# File Copy for Release
			if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
				INSTALL(FILES "${TORQUE_PHYSX_SDK_PATH}/Bin/win64/cudart64_30_9.dll"             DESTINATION "${projectOutDir}")
				INSTALL(FILES "${TORQUE_PHYSX_SDK_PATH}/Bin/win64/PhysXCooking64.dll"             DESTINATION "${projectOutDir}")
				INSTALL(FILES "${TORQUE_PHYSX_SDK_PATH}/Bin/win64/PhysXCore64.dll"             DESTINATION "${projectOutDir}")
				INSTALL(FILES "${TORQUE_PHYSX_SDK_PATH}/Bin/win64/PhysXDevice64.dll"             DESTINATION "${projectOutDir}")
				INSTALL(FILES "${TORQUE_PHYSX_SDK_PATH}/Bin/win64/PhysXLoader64.dll"             DESTINATION "${projectOutDir}")
			else()
				INSTALL(FILES "${TORQUE_PHYSX_SDK_PATH}/Bin/win32/cudart32_30_9.dll"             DESTINATION "${projectOutDir}")
				INSTALL(FILES "${TORQUE_PHYSX_SDK_PATH}/Bin/win32/PhysXCooking.dll"             DESTINATION "${projectOutDir}")
				INSTALL(FILES "${TORQUE_PHYSX_SDK_PATH}/Bin/win32/PhysXCore.dll"             DESTINATION "${projectOutDir}")
				INSTALL(FILES "${TORQUE_PHYSX_SDK_PATH}/Bin/win32/PhysXDevice.dll"             DESTINATION "${projectOutDir}")
				INSTALL(FILES "${TORQUE_PHYSX_SDK_PATH}/Bin/win32/PhysXLoader.dll"             DESTINATION "${projectOutDir}")
			endif()
		endif()
	endif()
endif()
