project(${TORQUE_APP_NAME})

# TODO: fmod support

###############################################################################
# modules
###############################################################################
option(TORQUE_SFX_VORBIS "Vorbis Sound" ON)
mark_as_advanced(TORQUE_SFX_VORBIS)
option(TORQUE_ADVANCED_LIGHTING "Advanced Lighting" ON)
mark_as_advanced(TORQUE_ADVANCED_LIGHTING)
option(TORQUE_BASIC_LIGHTING "Basic Lighting" ON)
mark_as_advanced(TORQUE_BASIC_LIGHTING)
option(TORQUE_THEORA "Theora Video Support" ON)
mark_as_advanced(TORQUE_THEORA)
option(TORQUE_SFX_DirectX "DirectX Sound" ON)
mark_as_advanced(TORQUE_SFX_DirectX)
option(TORQUE_SFX_OPENAL "OpenAL Sound" ON)
mark_as_advanced(TORQUE_SFX_OPENAL)
option(TORQUE_HIFI "HIFI? support" OFF)
mark_as_advanced(TORQUE_HIFI)
option(TORQUE_EXTENDED_MOVE "Extended move support" OFF)
mark_as_advanced(TORQUE_EXTENDED_MOVE)

###############################################################################
# options
###############################################################################
option(TORQUE_MULTITHREAD "Multi Threading" ON)
mark_as_advanced(TORQUE_MULTITHREAD)

option(TORQUE_DISABLE_MEMORY_MANAGER "Disable memory manager" OFF)
mark_as_advanced(TORQUE_DISABLE_MEMORY_MANAGER)

option(TORQUE_DISABLE_VIRTUAL_MOUNT_SYSTEM "Disable virtual mount system" OFF)
mark_as_advanced(TORQUE_DISABLE_VIRTUAL_MOUNT_SYSTEM)

option(TORQUE_PLAYER "Playback only?" OFF)
mark_as_advanced(TORQUE_PLAYER)

option(TORQUE_TOOLS "Enable or disable the tools" ON)
mark_as_advanced(TORQUE_TOOLS)

option(TORQUE_ENABLE_PROFILER "Enable or disable the profiler" OFF)
mark_as_advanced(TORQUE_ENABLE_PROFILER)

option(TORQUE_DEBUG "T3D Debug mode" OFF)
mark_as_advanced(TORQUE_DEBUG)

option(TORQUE_SHIPPING "T3D Shipping build?" OFF)
mark_as_advanced(TORQUE_SHIPPING)

option(TORQUE_DEBUG_NET "debug network" OFF)
mark_as_advanced(TORQUE_DEBUG_NET)

option(TORQUE_DEBUG_NET_MOVES "debug network moves" OFF)
mark_as_advanced(TORQUE_DEBUG_NET_MOVES)

option(TORQUE_ENABLE_ASSERTS "enables or disable asserts" OFF)
mark_as_advanced(TORQUE_ENABLE_ASSERTS)

option(TORQUE_DEBUG_GFX_MODE "triggers graphics debug mode" OFF)
mark_as_advanced(TORQUE_DEBUG_GFX_MODE)

#option(DEBUG_SPEW "more debug" OFF)
set(TORQUE_NO_DSO_GENERATION ON)

# warning C4800: 'XXX' : forcing value to bool 'true' or 'false' (performance warning)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4800")
# warning C4018: '<' : signed/unsigned mismatch
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4018")
# warning C4244: 'initializing' : conversion from 'XXX' to 'XXX', possible loss of data
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4244")

if(WIN32)
    link_directories($ENV{DXSDK_DIR}/Lib/x86)
endif()

###############################################################################
# Always enabled paths first
###############################################################################
addPath("${srcDir}/") # must come first :)
addPathRec("${srcDir}/app")
addPath("${srcDir}/sfx/media")
addPath("${srcDir}/sfx/null")
addPath("${srcDir}/sfx")
addPath("${srcDir}/component")
addPath("${srcDir}/component/interfaces")
addPath("${srcDir}/console")
addPath("${srcDir}/core")
addPath("${srcDir}/core/stream")
addPath("${srcDir}/core/strings")
addPath("${srcDir}/core/util")
addPath("${srcDir}/core/util/test")
addPath("${srcDir}/core/util/journal")
addPath("${srcDir}/core/util/journal/test")
addPath("${srcDir}/core/util/zip")
addPath("${srcDir}/core/util/zip/unitTests")
addPath("${srcDir}/core/util/zip/compressors")
addPath("${srcDir}/i18n")
addPath("${srcDir}/sim")
#addPath("${srcDir}/unit/tests")
addPath("${srcDir}/unit")
addPath("${srcDir}/util")
addPath("${srcDir}/windowManager")
addPath("${srcDir}/windowManager/torque")
addPath("${srcDir}/windowManager/test")
addPath("${srcDir}/math")
addPath("${srcDir}/math/util")
addPath("${srcDir}/math/test")
addPath("${srcDir}/platform")
addPath("${srcDir}/cinterface")
addPath("${srcDir}/platform/nativeDialogs")
addPath("${srcDir}/platform/menus")
addPath("${srcDir}/platform/test")
addPath("${srcDir}/platform/threads")
addPath("${srcDir}/platform/async")
addPath("${srcDir}/platform/input")
addPath("${srcDir}/platform/output")
addPath("${srcDir}/app")
addPath("${srcDir}/app/net")
addPath("${srcDir}/util/messaging")
addPath("${srcDir}/gfx/Null")
addPath("${srcDir}/gfx/test")
addPath("${srcDir}/gfx/bitmap")
addPath("${srcDir}/gfx/bitmap/loaders")
addPath("${srcDir}/gfx/util")
addPath("${srcDir}/gfx/video")
addPath("${srcDir}/gfx")
addPath("${srcDir}/shaderGen")
addPath("${srcDir}/gfx/sim")
addPath("${srcDir}/gui/buttons")
addPath("${srcDir}/gui/containers")
addPath("${srcDir}/gui/controls")
addPath("${srcDir}/gui/core")
addPath("${srcDir}/gui/game")
addPath("${srcDir}/gui/shiny")
addPath("${srcDir}/gui/utility")
addPath("${srcDir}/gui")
addPath("${srcDir}/collision")
addPath("${srcDir}/materials")
addPath("${srcDir}/lighting")
addPath("${srcDir}/lighting/common")
addPath("${srcDir}/renderInstance")
addPath("${srcDir}/scene")
addPath("${srcDir}/scene/culling")
addPath("${srcDir}/scene/zones")
addPath("${srcDir}/scene/mixin")
addPath("${srcDir}/shaderGen")
addPath("${srcDir}/terrain")
addPath("${srcDir}/environment")
addPath("${srcDir}/forest")
addPath("${srcDir}/forest/ts")
addPath("${srcDir}/ts")
addPath("${srcDir}/ts/arch")
addPath("${srcDir}/physics")
addPath("${srcDir}/gui/3d")
addPath("${srcDir}/postFx")
addPath("${srcDir}/T3D")
addPath("${srcDir}/T3D/examples")
addPath("${srcDir}/T3D/fps")
addPath("${srcDir}/T3D/fx")
addPath("${srcDir}/T3D/vehicles")
addPath("${srcDir}/T3D/physics")
addPath("${srcDir}/T3D/decal")
addPath("${srcDir}/T3D/sfx")
addPath("${srcDir}/T3D/gameBase")
addPath("${srcDir}/T3D/turret")
addPath("${srcDir}/main/")
addPathRec("${srcDir}/ts/collada")
addPathRec("${srcDir}/ts/loader")
addPathRec("${projectSrcDir}")

###############################################################################
# modular paths
###############################################################################
# lighting
if(TORQUE_ADVANCED_LIGHTING)
    addPath("${srcDir}/lighting/advanced")
    addPathRec("${srcDir}/lighting/shadowMap")
    addPathRec("${srcDir}/lighting/advanced/hlsl")
    #addPathRec("${srcDir}/lighting/advanced/glsl")
endif()
if(TORQUE_BASIC_LIGHTING)
    addPathRec("${srcDir}/lighting/basic")
    addPathRec("${srcDir}/lighting/shadowMap")
endif()

# DirectX Sound
if(TORQUE_SFX_DirectX)
    addPathRec("${srcDir}/sfx/dsound")
    addPathRec("${srcDir}/sfx/xaudio")
endif()

# OpenAL
if(TORQUE_SFX_OPENAL)
    addPath("${srcDir}/sfx/openal")
    #addPath("${srcDir}/sfx/openal/mac")
    addPath("${srcDir}/sfx/openal/win32")
endif()

# Theora
if(TORQUE_THEORA)
    addPath("${srcDir}/core/ogg")
    addPath("${srcDir}/gfx/video")
    addPath("${srcDir}/gui/theora")
endif()

# Include tools for non-tool builds (or define player if a tool build)
if(TORQUE_TOOLS)
    addPath("${srcDir}/gui/worldEditor")
    addPath("${srcDir}/environment/editors")
    addPath("${srcDir}/forest/editor")
    addPath("${srcDir}/gui/editor")
    addPath("${srcDir}/gui/editor/inspector")
endif()

if(TORQUE_HIFI)
    addPath("${srcDir}/T3D/gameBase/hifi")
endif()
    
if(TORQUE_EXTENDED_MOVE)
    addPath("${srcDir}/T3D/gameBase/extended")
else()
    addPath("${srcDir}/T3D/gameBase/std")
endif()

###############################################################################
# platform specific things
###############################################################################
if(WIN32)
    addPath("${srcDir}/platformWin32")
    addPath("${srcDir}/platformWin32/nativeDialogs")
    addPath("${srcDir}/platformWin32/menus")
    addPath("${srcDir}/platformWin32/threads")
    addPath("${srcDir}/platformWin32/videoInfo")
    addPath("${srcDir}/platformWin32/minidump")
    addPath("${srcDir}/windowManager/win32")
    #addPath("${srcDir}/gfx/D3D8")
    addPath("${srcDir}/gfx/D3D")
    addPath("${srcDir}/gfx/D3D9")
    addPath("${srcDir}/gfx/D3D9/pc")
    addPath("${srcDir}/shaderGen/HLSL")    
    addPath("${srcDir}/terrain/hlsl")
    addPath("${srcDir}/forest/hlsl")
    # add windows rc file for the icon
    addFile("${projectSrcDir}/torque.rc")
endif()

if(APPLE)
    addPath("${srcDir}/platformMac")
    addPath("${srcDir}/platformMac/menus")
    addPath("${srcDir}/platformPOSIX")
    addPath("${srcDir}/windowManager/mac")
    addPath("${srcDir}/gfx/gl")
    addPath("${srcDir}/gfx/gl/ggl")
    addPath("${srcDir}/gfx/gl/ggl/mac")
    addPath("${srcDir}/gfx/gl/ggl/generated")
    addPath("${srcDir}/shaderGen/GLSL")
    addPath("${srcDir}/terrain/glsl")
    addPath("${srcDir}/forest/glsl")    
endif()

if(XBOX360)
    addPath("${srcDir}/platformXbox")
    addPath("${srcDir}/platformXbox/threads")
    addPath("${srcDir}/windowManager/360")
    addPath("${srcDir}/gfx/D3D9")
    addPath("${srcDir}/gfx/D3D9/360")
    addPath("${srcDir}/shaderGen/HLSL")
    addPath("${srcDir}/shaderGen/360")
    addPath("${srcDir}/ts/arch/360")
    addPath("${srcDir}/terrain/hlsl")
    addPath("${srcDir}/forest/hlsl")
endif()

if(PS3)
    addPath("${srcDir}/platformPS3")
    addPath("${srcDir}/platformPS3/threads")
    addPath("${srcDir}/windowManager/ps3")
    addPath("${srcDir}/gfx/gl")
    addPath("${srcDir}/gfx/gl/ggl")
    addPath("${srcDir}/gfx/gl/ggl/ps3")
    addPath("${srcDir}/gfx/gl/ggl/generated")
    addPath("${srcDir}/shaderGen/GLSL")
    addPath("${srcDir}/ts/arch/ps3")
    addPath("${srcDir}/terrain/glsl")
    addPath("${srcDir}/forest/glsl")    
endif()

if(UNIX)
    # linux_dedicated
    addPath("${srcDir}/windowManager/dedicated")
    # linux
    addPath("${srcDir}/platformX86UNIX")
    addPath("${srcDir}/platformX86UNIX/threads")
    addPath("${srcDir}/platformPOSIX")
    addPath("${srcDir}/gfx/gl")
    addPath("${srcDir}/gfx/gl/ggl")
    addPath("${srcDir}/gfx/gl/ggl/x11") # This one is not yet implemented!
    addPath("${srcDir}/gfx/gl/ggl/generated")
    addPath("${srcDir}/shaderGen/GLSL")
    addPath("${srcDir}/terrain/glsl")
    addPath("${srcDir}/forest/glsl")    
endif()

###############################################################################
###############################################################################
addExecutable()
###############################################################################
###############################################################################

# configure the relevant files only once
if(NOT EXISTS "${projectSrcDir}/torqueConfig.h")
    message(STATUS "writing ${projectSrcDir}/torqueConfig.h")
    CONFIGURE_FILE("${cmakeDir}/torqueConfig.h.in" "${projectSrcDir}/torqueConfig.h")
endif()
if(NOT EXISTS "${projectSrcDir}/torque.ico")
    CONFIGURE_FILE("${cmakeDir}/torque.ico" "${projectSrcDir}/torque.ico" COPYONLY)
endif()
if(NOT EXISTS "${projectOutDir}/${PROJECT_NAME}.torsion")
    CONFIGURE_FILE("${cmakeDir}/template.torsion.in" "${projectOutDir}/${PROJECT_NAME}.torsion")
endif()
if(EXISTS "${CMAKE_SOURCE_DIR}/Templates/${TORQUE_TEMPLATE}/game/main.cs.in" AND NOT EXISTS "${projectOutDir}/main.cs")
    CONFIGURE_FILE("${CMAKE_SOURCE_DIR}/Templates/${TORQUE_TEMPLATE}/game/main.cs.in" "${projectOutDir}/main.cs")
endif()
if(WIN32)
	if(NOT EXISTS "${projectSrcDir}/torque.rc")
		CONFIGURE_FILE("${cmakeDir}/torque-win.rc.in" "${projectSrcDir}/torque.rc")
	endif()
	if(NOT EXISTS "${projectOutDir}/${PROJECT_NAME}-debug.bat")
		CONFIGURE_FILE("${cmakeDir}/app-debug-win.bat.in" "${projectOutDir}/${PROJECT_NAME}-debug.bat")
	endif()
	if(NOT EXISTS "${projectOutDir}/cleanup.bat")
		CONFIGURE_FILE("${cmakeDir}/cleanup-win.bat.in" "${projectOutDir}/cleanup.bat")
	endif()
endif()

###############################################################################
# Common Libraries
###############################################################################
addLib(lmng)
addLib(lpng)
addLib(lungif)
addLib(ljpeg)
addLib(zlib)
addLib(tinyxml)
addLib(opcode)
addLib(squish)
addLib(collada)
addLib(pcre)
addLib(convexDecomp)

if(WIN32)
    # copy pasted from T3D build system, some might not be needed
	set(TORQUE_EXTERNAL_LIBS "COMCTL32.LIB;COMDLG32.LIB;USER32.LIB;ADVAPI32.LIB;GDI32.LIB;WINMM.LIB;WSOCK32.LIB;vfw32.lib;Imm32.lib;d3d9.lib;d3dx9.lib;DxErr.lib;ole32.lib;shell32.lib;oleaut32.lib;version.lib" CACHE STRING "external libs to link against")
	mark_as_advanced(TORQUE_EXTERNAL_LIBS)
    addLib("${TORQUE_EXTERNAL_LIBS}")
endif()

###############################################################################
# Always enabled Definitions
###############################################################################
addDebugDef(TORQUE_DEBUG)
addDebugDef(TORQUE_ENABLE_ASSERTS)
addDebugDef(TORQUE_DEBUG_GFX_MODE)

addDef(TORQUE_SHADERGEN)
addDef(INITGUID)
addDef(NTORQUE_SHARED)
addDef(UNICODE)
addDef(_UNICODE) # for VS
addDef(TORQUE_UNICODE)
#addDef(TORQUE_SHARED) # not used anymore as the game is the executable directly
addDef(LTC_NO_PROTOTYPES) # for libTomCrypt
addDef(BAN_OPCODE_AUTOLINK)
addDef(ICE_NO_DLL)
addDef(TORQUE_OPCODE)
addDef(TORQUE_COLLADA)
addDef(DOM_INCLUDE_TINYXML)
addDef(PCRE_STATIC)
addDef(_CRT_SECURE_NO_WARNINGS)
addDef(_CRT_SECURE_NO_DEPRECATE)


###############################################################################
# Modules
###############################################################################
if(TORQUE_SFX_DirectX)
    addLib(x3daudio.lib)
endif()

if(TORQUE_ADVANCED_LIGHTING)
    addDef(TORQUE_ADVANCED_LIGHTING)
endif()
if(TORQUE_BASIC_LIGHTING)
    addDef(TORQUE_BASIC_LIGHTING)
endif()

if(TORQUE_SFX_OPENAL)
    addInclude("${libDir}/openal/win32")
endif()

if(TORQUE_SFX_VORBIS)
    addInclude(${libDir}/libvorbis/include)
    addDef(TORQUE_OGGVORBIS)
    addLib(libvorbis)
    addLib(libogg)
endif()

if(TORQUE_THEORA)
    addDef(TORQUE_OGGTHEORA)
    addDef(TORQUE_OGGVORIBS)
    addInclude(${libDir}/libtheora/include)
    addLib(libtheora)
endif()

if(TORQUE_HIFI)
    addDef(TORQUE_HIFI_NET)
endif()
if(TORQUE_EXTENDED_MOVE)
    addDef(TORQUE_EXTENDED_MOVE)
endif()

###############################################################################
# Include Paths
###############################################################################
addInclude("${projectSrcDir}")
addInclude("${srcDir}/")
addInclude("${libDir}/lmpg")
addInclude("${libDir}/lpng")
addInclude("${libDir}/ljpeg")
addInclude("${libDir}/lungif")
addInclude("${libDir}/zlib")
addInclude("${libDir}/") # for tinyxml
addInclude("${libDir}/tinyxml")
addInclude("${libDir}/squish")
addInclude("${libDir}/convexDecomp")
addInclude("${libDir}/libogg/include")
addInclude("${libDir}/opcode")
addInclude("${libDir}/collada/include")
addInclude("${libDir}/collada/include/1.4")

# external things
if(WIN32)
    set_property(TARGET ${PROJECT_NAME} APPEND PROPERTY INCLUDE_DIRECTORIES $ENV{DXSDK_DIR}/Include)
endif()

###############################################################################
# Installation
###############################################################################
INSTALL(DIRECTORY "${CMAKE_SOURCE_DIR}/Templates/${TORQUE_TEMPLATE}/game"                 DESTINATION "${projectDir}")
if(WIN32)
	INSTALL(FILES "${CMAKE_SOURCE_DIR}/Templates/${TORQUE_TEMPLATE}/cleanShaders.bat"     DESTINATION "${projectDir}")
	INSTALL(FILES "${CMAKE_SOURCE_DIR}/Templates/${TORQUE_TEMPLATE}/DeleteCachedDTSs.bat" DESTINATION "${projectDir}")
	INSTALL(FILES "${CMAKE_SOURCE_DIR}/Templates/${TORQUE_TEMPLATE}/DeleteDSOs.bat"       DESTINATION "${projectDir}")
	INSTALL(FILES "${CMAKE_SOURCE_DIR}/Templates/${TORQUE_TEMPLATE}/DeletePrefs.bat"      DESTINATION "${projectDir}")
endif()
