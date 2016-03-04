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

# module Hydra

option(TORQUE_HYDRA "Enable HYDRA module" OFF)
mark_as_advanced(TORQUE_HYDRA)
if(TORQUE_HYDRA)
	if(TORQUE_HYDRA_SDK_PATH STREQUAL "")
		set(TORQUE_HYDRA_SDK_PATH "" CACHE PATH "HYDRA library path" FORCE)
	endif()
else() # hide variable
    set(TORQUE_HYDRA_SDK_PATH "" CACHE INTERNAL "" FORCE) 
endif()

if(TORQUE_HYDRA)
	# Source
	addPathRec( "${srcDir}/platform/input/razerHydra" )

	# Includes
	addInclude( "${TORQUE_RAZERHYDRA_SDK_PATH}/include" )
	 
	# Install
	if( WIN32 ) 
		# File Copy for Release   
		INSTALL(FILES "${TORQUE_RAZERHYDRA_SDK_PATH}/bin/win32/release_dll/sixense.dll"             DESTINATION "${projectOutDir}")

		# File Copy for Debug
		INSTALL(FILES "${TORQUE_RAZERHYDRA_SDK_PATH}/bin/win32/debug_dll/sixensed.dll"              DESTINATION "${projectOutDir}" CONFIGURATIONS "Debug" )
		# Only needed by the debug sixense library
		INSTALL(FILES "${TORQUE_RAZERHYDRA_SDK_PATH}/samples/win32/sixense_simple3d/DeviceDLL.dll"  DESTINATION "${projectOutDir}" CONFIGURATIONS "Debug" )
	endif()
endif()
