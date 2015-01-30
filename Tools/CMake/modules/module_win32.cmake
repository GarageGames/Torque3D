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

# Win32 module

if(WIN32)
	option(TORQUE_SFX_DirectX "DirectX Sound" ON)
	mark_as_advanced(TORQUE_SFX_DirectX)
else()
	set(TORQUE_SFX_DirectX OFF)
endif()

if( NOT WIN32 )
    return()
endif()

# warning C4800: 'XXX' : forcing value to bool 'true' or 'false' (performance warning)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4800")
# warning C4018: '<' : signed/unsigned mismatch
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4018")
# warning C4244: 'initializing' : conversion from 'XXX' to 'XXX', possible loss of data
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4244")

if( TORQUE_CPU_X64 )
    link_directories($ENV{DXSDK_DIR}/Lib/x64)
else()
    link_directories($ENV{DXSDK_DIR}/Lib/x86)
endif()

addPath("${srcDir}/platformWin32")
addPath("${srcDir}/platformWin32/nativeDialogs")
addPath("${srcDir}/platformWin32/menus")
addPath("${srcDir}/platformWin32/threads")
addPath("${srcDir}/platformWin32/videoInfo")
addPath("${srcDir}/platformWin32/minidump")
addPath("${srcDir}/windowManager/win32")
addPath("${srcDir}/gfx/D3D")
addPath("${srcDir}/gfx/D3D9")
addPath("${srcDir}/gfx/D3D9/pc")
addPath("${srcDir}/shaderGen/HLSL")    
addPath("${srcDir}/terrain/hlsl")
addPath("${srcDir}/forest/hlsl")
# add windows rc file for the icon
addFile("${projectSrcDir}/torque.rc")

if(NOT EXISTS "${projectSrcDir}/torque.rc")
    CONFIGURE_FILE("${cmakeDir}/torque-win.rc.in" "${projectSrcDir}/torque.rc")
endif()
if(NOT EXISTS "${projectOutDir}/${PROJECT_NAME}-debug.bat")
    CONFIGURE_FILE("${cmakeDir}/app-debug-win.bat.in" "${projectOutDir}/${PROJECT_NAME}-debug.bat")
endif()
if(NOT EXISTS "${projectOutDir}/cleanup.bat")
    CONFIGURE_FILE("${cmakeDir}/cleanup-win.bat.in" "${projectOutDir}/cleanup.bat")
endif()

# copy pasted from T3D build system, some might not be needed
set(TORQUE_EXTERNAL_LIBS "COMCTL32.LIB;COMDLG32.LIB;USER32.LIB;ADVAPI32.LIB;GDI32.LIB;WINMM.LIB;WSOCK32.LIB;vfw32.lib;Imm32.lib;d3d9.lib;d3dx9.lib;DxErr.lib;ole32.lib;shell32.lib;oleaut32.lib;version.lib" CACHE STRING "external libs to link against")
mark_as_advanced(TORQUE_EXTERNAL_LIBS)
addLib("${TORQUE_EXTERNAL_LIBS}")

file(TO_CMAKE_PATH $ENV{DXSDK_DIR} CMAKE_DXSDK_DIR)
addInclude( "${CMAKE_DXSDK_DIR}/Include" )

INSTALL(FILES "${CMAKE_SOURCE_DIR}/Templates/${TORQUE_TEMPLATE}/cleanShaders.bat"     DESTINATION "${TORQUE_APP_DIR}")
INSTALL(FILES "${CMAKE_SOURCE_DIR}/Templates/${TORQUE_TEMPLATE}/DeleteCachedDTSs.bat" DESTINATION "${TORQUE_APP_DIR}")
INSTALL(FILES "${CMAKE_SOURCE_DIR}/Templates/${TORQUE_TEMPLATE}/DeleteDSOs.bat"       DESTINATION "${TORQUE_APP_DIR}")
INSTALL(FILES "${CMAKE_SOURCE_DIR}/Templates/${TORQUE_TEMPLATE}/DeletePrefs.bat"      DESTINATION "${TORQUE_APP_DIR}")
