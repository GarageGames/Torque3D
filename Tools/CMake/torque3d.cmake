# TODO: fmod support

# Prefs
add_definitions(-DTORQUE_SHADERGEN)
add_definitions(-DINITGUID)
add_definitions(-DNTORQUE_SHARED)

# flags found in the code:
# TORQUE_TGB_ONLY
# TORQUE_PLAYER
# TORQUE_DEMO_PURCHASE
# TORQUE_ENABLE_VFS
# TORQUE_DEBUG
# TORQUE_MINIDUMP
# TORQUE_RELEASE
# TORQUE_DEBUG_GUARD
# _XBOX
# TORQUE_OS_MAC
# TORQUE_TOOLS
# TORQUE_SHIPPING
# TORQUE_DEBUG_NET
# TORQUE_OS_LINUX
# TORQUE_OS_OPENBSD
# TORQUE_OS_XENON
# TORQUE_OS_PS3
# TORQUE_OS_MAC
# TORQUE_OS_WIN32
# TORQUE_ENGINE_PRODUCT
# DEBUG_AST_NODES
# TORQUE_ENABLE_SCRIPTASSERTS
# TORQUE_NO_DSO_GENERATION
# DOXYGENIZING
# TORQUE2D_TOOLS_FIXME
# DEBUG_SPEW
# DISABLE_DEBUG_SPEW
# TORQUE_NET_STATS
# TORQUE_DEBUG_NET
# TORQUE_LOCBUILD
# USE_UNDOCUMENTED_GROUP
# _PACK_BUG_WORKAROUNDS
# PLATFORM_LITTLE_ENDIAN
# TORQUE_BIG_ENDIAN
# NO_BLUR
# FRAMEALLOCATOR_DEBUG_GUARD
# TORQUE_DEBUG_RES_MANAGER
# TORQUE_ENABLE_UTF16_CACHE
# TORQUE_ENABLE_THREAD_STATIC_METRICS
# TORQUE_ENABLE_THREAD_STATICS
# WANT_TO_SIMULATE_UI_ON_360
# TORQUE_GATHER_METRICS
# D3DXSHADER_USE_LEGACY_D3DX9_31_DLL
# TORQUE_ENABLE_CSF_GENERATION
# TORQUE_DEBUG_RENDER
# TORQUE_ENABLE_PROFILER
# TORQUE_ENABLE_PROFILE_PATH
# DOXYGEN
# TORQUE_COLLADA
# TORQUE_COMPILER_GCC
# LOG_INPUT
# TORQUE_SUPPORTS_GCC_INLINE_X86_ASM
# TORQUE_ENABLE_ASSERTS
# DLL_CODE
# TORQUE_USE_WINSOCK
# TORQUE_DISABLE_PC_CONNECTIVITY
# TORQUE_SUPPORTS_VC_INLINE_X86_ASM
# TORQUE_64BITS
# APSTUDIO_INVOKED
# APSTUDIO_READONLY_SYMBOLS
# INITGUID
# LOG_MOUSEMOVE
# TORQUE_COMPILER_MINGW
# TORQUE_LIB
# TORQUE_SHARED
# TORQUE_DEDICATED
# TORQUE_ENGINE
# TORQUE_OGGVORBIS
# TORQUE_EXTENDED_MOVE
# DECALMANAGER_DEBUG
# TORQUE_DEBUG_NET_MOVES
# TORQUE_DEMO_WATERMARK
# TORQUE_CPU_X86
# USE_MEM_VERTEX_BUFFERS
# TORQUE_MAX_LIB
# TORQUE_ENABLE_SAMPLING
#TORQUE_DISABLE_FIND_ROOT_WITHIN_ZIP

option(TORQUE_MULTITHREAD "Multi Threading" ON)
option(TORQUE_DISABLE_MEMORY_MANAGER "Disable memory manager" OFF)
option(TORQUE_DISABLE_VIRTUAL_MOUNT_SYSTEM "Disable virtual mount system" OFF)
option(TORQUE_PLAYER "Playback only?" OFF)
option(TORQUE_TOOLS "Enable or disable the tools" ON)
option(TORQUE_ENABLE_PROFILER "Enable or disable the profiler" OFF)
option(TORQUE_DEBUG "T3D Debug mode" OFF)
option(TORQUE_SHIPPING "T3D Shipping build?" OFF)
option(TORQUE_DEBUG_NET "debug network" OFF)
option(TORQUE_DEBUG_NET_MOVES "debug network moves" OFF)
#option(DEBUG_SPEW "more debug" OFF)

set(TORQUE_APP_NAME "Default" CACHE STRING "the app name")
set(TORQUE_APP_VERSION "1000" CACHE STRING "the app version: major * 1000 + minor * 100 + revision * 10.")
set(TORQUE_APP_VERSION_STRING "1.0" CACHE STRING "the app version string")

# MAXPACKETSIZE
# TORQUE_GATHER_METRICS
# TORQUE_DEBUG_GUARD
# TORQUE_ENABLE_THREAD_STATICS
# TORQUE_ENABLE_THREAD_STATIC_METRICS
# FRAMEALLOCATOR_DEBUG_GUARD

set(TORQUE_NO_DSO_GENERATION ON)


add_definitions(-DUNICODE)
add_definitions(-D_UNICODE) # for VS
add_definitions(-DTORQUE_UNICODE)
#add_definitions(-DTORQUE_SHARED)

# for libTomCrypt
add_definitions(-DLTC_NO_PROTOTYPES)


add_definitions(-D_CRT_SECURE_NO_WARNINGS)
add_definitions(-D_CRT_SECURE_NO_DEPRECATE)

# warning C4800: 'XXX' : forcing value to bool 'true' or 'false' (performance warning)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4800")

# warning C4018: '<' : signed/unsigned mismatch
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4018")

# warning C4244: 'initializing' : conversion from 'XXX' to 'XXX', possible loss of data
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4244")

include_directories(${projectSrcDir})
include_directories(${srcDir}/)
include_directories(${libDir}/lmpg)
include_directories(${libDir}/lpng)
include_directories(${libDir}/ljpeg)
include_directories(${libDir}/lungif)
include_directories(${libDir}/zlib)
include_directories(${libDir}/) # for tinyxml
include_directories(${libDir}/tinyxml)
include_directories(${libDir}/squish)
include_directories(${libDir}/convexDecomp)
include_directories(${libDir}/libogg/include)

set(t3dLibs "")
set(t3dLibDirs "")

# external things
include_directories($ENV{DXSDK_DIR}/Include)
set(t3dLibDirs "${t3dLibs};$ENV{DXSDK_DIR}/Lib/x86")

# module helper

set(t "")
addLibPath(t "${srcDir}/app" GLOB_RECURSE)

# lighting
option(TORQUE_ADVANCED_LIGHTING "Advanced Lighting" ON)
if(TORQUE_ADVANCED_LIGHTING)
    add_definitions(-DTORQUE_ADVANCED_LIGHTING)
    addLibPath(t "${srcDir}/lighting/advanced" GLOB)
    addLibPath(t "${srcDir}/lighting/shadowMap" GLOB_RECURSE)
    addLibPath(t "${srcDir}/lighting/advanced/hlsl" GLOB_RECURSE)
    #addLibPath(t "${srcDir}/lighting/advanced/glsl" GLOB_RECURSE)
endif()
option(TORQUE_BASIC_LIGHTING "Basic Lighting" ON)
if(TORQUE_BASIC_LIGHTING)
    add_definitions(-DTORQUE_BASIC_LIGHTING)
    addLibPath(t "${srcDir}/lighting/basic" GLOB_RECURSE)
    addLibPath(t "${srcDir}/lighting/shadowMap" GLOB_RECURSE)
endif()

# opcode
include_directories(${libDir}/opcode)
add_definitions(-DBAN_OPCODE_AUTOLINK)
add_definitions(-DICE_NO_DLL)
add_definitions(-DTORQUE_OPCODE)


# collada
add_definitions(-DTORQUE_COLLADA)
add_definitions(-DDOM_INCLUDE_TINYXML)
add_definitions(-DPCRE_STATIC)
addLibPath(t "${srcDir}/ts/collada" GLOB_RECURSE)
addLibPath(t "${srcDir}/ts/loader" GLOB_RECURSE)
include_directories(${libDir}/collada/include)
include_directories(${libDir}/collada/include/1.4)

# DirectX Sound
option(TORQUE_SFX_DirectX "DirectX Sound" ON)
if(TORQUE_SFX_DirectX)
    addLibPath(t "${srcDir}/sfx/dsound" GLOB_RECURSE)
    addLibPath(t "${srcDir}/sfx/xaudio" GLOB_RECURSE)
    set(t3dLibs "${t3dLibs};x3daudio.lib")
endif()

# OpenAL
option(TORQUE_SFX_OPENAL "OpenAL Sound" ON)
if(TORQUE_SFX_OPENAL)
    addLibPath(t "${srcDir}/sfx/openal" GLOB)
    #addLibPath(t "${srcDir}/sfx/openal/mac" GLOB)
    addLibPath(t "${srcDir}/sfx/openal/win32" GLOB)
    include_directories(${libDir}/openal/win32)
endif()

# vorbis
option(TORQUE_SFX_VORBIS "Vorbis Sound" ON)
if(TORQUE_SFX_VORBIS)
    add_definitions(-DTORQUE_OGGVORBIS)
    include_directories(${libDir}/libvorbis/include)
    set(t3dLibs "${t3dLibs};libvorbis;libogg")
endif()

# Theora
option(TORQUE_THEORA "Theora Video Support" ON)
if(TORQUE_THEORA)
    add_definitions(-DTORQUE_OGGTHEORA)
    add_definitions(-DTORQUE_OGGVORIBS)
    addLibPath(t "${srcDir}/core/ogg" GLOB)
    addLibPath(t "${srcDir}/gfx/video" GLOB)
    addLibPath(t "${srcDir}/gui/theora" GLOB)
    include_directories(${libDir}/libtheora/include)
    set(t3dLibs "${t3dLibs};libtheora")
endif()

# Always on things
addLibPath(t "${srcDir}/sfx/media" GLOB)
addLibPath(t "${srcDir}/sfx/null" GLOB)
addLibPath(t "${srcDir}/sfx" GLOB)

# Components
addLibPath(t "${srcDir}/component" GLOB)
addLibPath(t "${srcDir}/component/interfaces" GLOB)

   
addLibPath(t "${srcDir}/console" GLOB)
addLibPath(t "${srcDir}/core" GLOB)
addLibPath(t "${srcDir}/core/stream" GLOB)
addLibPath(t "${srcDir}/core/strings" GLOB)
addLibPath(t "${srcDir}/core/util" GLOB)
addLibPath(t "${srcDir}/core/util/test" GLOB)
addLibPath(t "${srcDir}/core/util/journal" GLOB)
addLibPath(t "${srcDir}/core/util/journal/test" GLOB)
addLibPath(t "${srcDir}/core/util/zip" GLOB)
addLibPath(t "${srcDir}/core/util/zip/unitTests" GLOB)
addLibPath(t "${srcDir}/core/util/zip/compressors" GLOB)
addLibPath(t "${srcDir}/i18n" GLOB)
addLibPath(t "${srcDir}/sim" GLOB)
#addLibPath(t "${srcDir}/unit/tests" GLOB)
addLibPath(t "${srcDir}/unit" GLOB)
addLibPath(t "${srcDir}/util" GLOB)
addLibPath(t "${srcDir}/windowManager" GLOB)
addLibPath(t "${srcDir}/windowManager/torque" GLOB)
addLibPath(t "${srcDir}/windowManager/test" GLOB)
addLibPath(t "${srcDir}/math" GLOB)
addLibPath(t "${srcDir}/math/util" GLOB)
addLibPath(t "${srcDir}/math/test" GLOB)
addLibPath(t "${srcDir}/platform" GLOB)
addLibPath(t "${srcDir}/cinterface" GLOB)

addLibPath(t "${srcDir}/platform/nativeDialogs" GLOB)
addLibPath(t "${srcDir}/platform/menus" GLOB)
addLibPath(t "${srcDir}/platform/test" GLOB)

addLibPath(t "${srcDir}/platform/threads" GLOB)
addLibPath(t "${srcDir}/platform/async" GLOB)
addLibPath(t "${srcDir}/platform/input" GLOB)
addLibPath(t "${srcDir}/platform/output" GLOB)
addLibPath(t "${srcDir}/app" GLOB)
addLibPath(t "${srcDir}/app/net" GLOB)

addLibPath(t "${srcDir}/util/messaging" GLOB)

# win32
if(WIN32)
    #add_definitions(-DTORQUE_OS_WIN32)
    addLibPath(t "${srcDir}/platformWin32" GLOB)
    addLibPath(t "${srcDir}/platformWin32/nativeDialogs" GLOB)
    addLibPath(t "${srcDir}/platformWin32/menus" GLOB)
    addLibPath(t "${srcDir}/platformWin32/threads" GLOB)
    addLibPath(t "${srcDir}/platformWin32/videoInfo" GLOB)
    addLibPath(t "${srcDir}/platformWin32/minidump" GLOB)
    addLibPath(t "${srcDir}/windowManager/win32" GLOB)
    #addLibPath(t "${srcDir}/gfx/D3D8" GLOB)
    addLibPath(t "${srcDir}/gfx/D3D" GLOB)
    addLibPath(t "${srcDir}/gfx/D3D9" GLOB)
    addLibPath(t "${srcDir}/gfx/D3D9/pc" GLOB)
    addLibPath(t "${srcDir}/shaderGen/HLSL" GLOB)    
    addLibPath(t "${srcDir}/terrain/hlsl" GLOB)
    addLibPath(t "${srcDir}/forest/hlsl" GLOB)
endif()

# mac
if(APPLE)
    #add_definitions(-DTORQUE_OS_MAC)
    addLibPath(t "${srcDir}/platformMac" GLOB)
    addLibPath(t "${srcDir}/platformMac/menus" GLOB)
    addLibPath(t "${srcDir}/platformPOSIX" GLOB)
    addLibPath(t "${srcDir}/windowManager/mac" GLOB)
    addLibPath(t "${srcDir}/gfx/gl" GLOB)
    addLibPath(t "${srcDir}/gfx/gl/ggl" GLOB)
    addLibPath(t "${srcDir}/gfx/gl/ggl/mac" GLOB)
    addLibPath(t "${srcDir}/gfx/gl/ggl/generated" GLOB)
    addLibPath(t "${srcDir}/shaderGen/GLSL" GLOB)
    addLibPath(t "${srcDir}/terrain/glsl" GLOB)
    addLibPath(t "${srcDir}/forest/glsl" GLOB)    
endif()

# xbox360
if(XBOX360)
    #add_definitions(-D_XBOX)
    #add_definitions(-DTORQUE_OS_XENON)
    addLibPath(t "${srcDir}/platformXbox" GLOB)
    addLibPath(t "${srcDir}/platformXbox/threads" GLOB)
    addLibPath(t "${srcDir}/windowManager/360" GLOB)
    addLibPath(t "${srcDir}/gfx/D3D9" GLOB)
    addLibPath(t "${srcDir}/gfx/D3D9/360" GLOB)
    addLibPath(t "${srcDir}/shaderGen/HLSL" GLOB)
    addLibPath(t "${srcDir}/shaderGen/360" GLOB)
    addLibPath(t "${srcDir}/ts/arch/360" GLOB)
    addLibPath(t "${srcDir}/terrain/hlsl" GLOB)
    addLibPath(t "${srcDir}/forest/hlsl" GLOB)
endif()

# ps3
if(PS3)
    #add_definitions(-DTORQUE_OS_PS3)
    addLibPath(t "${srcDir}/platformPS3" GLOB)
    addLibPath(t "${srcDir}/platformPS3/threads" GLOB)
    addLibPath(t "${srcDir}/windowManager/ps3" GLOB)
    # GFX - GGL
    addLibPath(t "${srcDir}/gfx/gl" GLOB)
    addLibPath(t "${srcDir}/gfx/gl/ggl" GLOB)
    addLibPath(t "${srcDir}/gfx/gl/ggl/ps3" GLOB)
    addLibPath(t "${srcDir}/gfx/gl/ggl/generated" GLOB)
    addLibPath(t "${srcDir}/shaderGen/GLSL" GLOB)
    addLibPath(t "${srcDir}/ts/arch/ps3" GLOB)
    addLibPath(t "${srcDir}/terrain/glsl" GLOB)
    addLibPath(t "${srcDir}/forest/glsl" GLOB)    
endif()

if(UNIX)
    #add_definitions(-DTORQUE_OS_LINUX)
    # linux_dedicated
    addLibPath(t "${srcDir}/windowManager/dedicated" GLOB)
    # linux
    addLibPath(t "${srcDir}/platformX86UNIX" GLOB)
    addLibPath(t "${srcDir}/platformX86UNIX/threads" GLOB)
    addLibPath(t "${srcDir}/platformPOSIX" GLOB)
    # GFX - GGL
    addLibPath(t "${srcDir}/gfx/gl" GLOB)
    addLibPath(t "${srcDir}/gfx/gl/ggl" GLOB)
    addLibPath(t "${srcDir}/gfx/gl/ggl/x11" GLOB) # This one is not yet implemented!
    addLibPath(t "${srcDir}/gfx/gl/ggl/generated" GLOB)
    addLibPath(t "${srcDir}/shaderGen/GLSL" GLOB)
    addLibPath(t "${srcDir}/terrain/glsl" GLOB)
    addLibPath(t "${srcDir}/forest/glsl" GLOB)    
endif()

# GFX
addLibPath(t "${srcDir}/gfx/Null" GLOB)
addLibPath(t "${srcDir}/gfx/test" GLOB)
addLibPath(t "${srcDir}/gfx/bitmap" GLOB)
addLibPath(t "${srcDir}/gfx/bitmap/loaders" GLOB)
addLibPath(t "${srcDir}/gfx/util" GLOB)
addLibPath(t "${srcDir}/gfx/video" GLOB)
addLibPath(t "${srcDir}/gfx" GLOB)
addLibPath(t "${srcDir}/shaderGen" GLOB)

# GFX - Sim dependent 
addLibPath(t "${srcDir}/gfx/sim" GLOB)

# GUI
addLibPath(t "${srcDir}/gui/buttons" GLOB)
addLibPath(t "${srcDir}/gui/containers" GLOB)
addLibPath(t "${srcDir}/gui/controls" GLOB)

addLibPath(t "${srcDir}/gui/core" GLOB)
addLibPath(t "${srcDir}/gui/game" GLOB)
addLibPath(t "${srcDir}/gui/shiny" GLOB)
addLibPath(t "${srcDir}/gui/utility" GLOB)
addLibPath(t "${srcDir}/gui" GLOB)

# Include tools for non-tool builds (or define player if a tool build)
addLibPath(t "${srcDir}/gui/worldEditor" GLOB)
addLibPath(t "${srcDir}/environment/editors" GLOB)
addLibPath(t "${srcDir}/forest/editor" GLOB)
addLibPath(t "${srcDir}/gui/editor" GLOB)
addLibPath(t "${srcDir}/gui/editor/inspector" GLOB)

# T3D
# 3D
addLibPath(t "${srcDir}/collision" GLOB)
addLibPath(t "${srcDir}/materials" GLOB)
addLibPath(t "${srcDir}/lighting" GLOB)
addLibPath(t "${srcDir}/lighting/common" GLOB)
addLibPath(t "${srcDir}/renderInstance" GLOB)
addLibPath(t "${srcDir}/scene" GLOB)
addLibPath(t "${srcDir}/scene/culling" GLOB)
addLibPath(t "${srcDir}/scene/zones" GLOB)
addLibPath(t "${srcDir}/scene/mixin" GLOB)
addLibPath(t "${srcDir}/shaderGen" GLOB)
addLibPath(t "${srcDir}/terrain" GLOB)
addLibPath(t "${srcDir}/environment" GLOB)

addLibPath(t "${srcDir}/forest" GLOB)
addLibPath(t "${srcDir}/forest/ts" GLOB)

addLibPath(t "${srcDir}/ts" GLOB)
addLibPath(t "${srcDir}/ts/arch" GLOB)
addLibPath(t "${srcDir}/physics" GLOB)
addLibPath(t "${srcDir}/gui/3d" GLOB)
addLibPath(t "${srcDir}/postFx" GLOB)

# 3D game
addLibPath(t "${srcDir}/T3D" GLOB)
addLibPath(t "${srcDir}/T3D/examples" GLOB)
addLibPath(t "${srcDir}/T3D/fps" GLOB)
addLibPath(t "${srcDir}/T3D/fx" GLOB)
addLibPath(t "${srcDir}/T3D/vehicles" GLOB)
addLibPath(t "${srcDir}/T3D/physics" GLOB)
addLibPath(t "${srcDir}/T3D/decal" GLOB)
addLibPath(t "${srcDir}/T3D/sfx" GLOB)
addLibPath(t "${srcDir}/T3D/gameBase" GLOB)
addLibPath(t "${srcDir}/T3D/turret" GLOB)

# HIFI NET
option(TORQUE_HIFI "HIFI? support" OFF)
if(TORQUE_HIFI)
    add_definitions(-DTORQUE_HIFI_NET)
    addLibPath(t "${srcDir}/T3D/gameBase/hifi" GLOB)
endif()

# extended move
option(TORQUE_EXTENDED_MOVE "Extended move support" OFF)
if(TORQUE_EXTENDED_MOVE)
    add_definitions(-DTORQUE_EXTENDED_MOVE)
    addLibPath(t "${srcDir}/T3D/gameBase/extended" GLOB)
else()
    addLibPath(t "${srcDir}/T3D/gameBase/std" GLOB)
endif()

if(WIN32)
    # copy pasted from T3D build system, some might not be needed
    set(t3dLibs "${t3dLibs};COMCTL32.LIB;COMDLG32.LIB;USER32.LIB;ADVAPI32.LIB;GDI32.LIB;WINMM.LIB;WSOCK32.LIB;vfw32.lib;Imm32.lib;d3d9.lib;d3dx9.lib;DxErr.lib;ole32.lib;shell32.lib;oleaut32.lib;version.lib")
    
    # add windows rc file for the icon
    set(t "${t};${projectSrcDir}/torque-win.rc")
endif()

# add main/main.cpp
set(t "${t};${srcDir}/main/main.cpp;")

# configure only once
if(NOT EXISTS ${projectSrcDir}/torqueConfig.h)
    message(STATUS "writing ${projectSrcDir}/torqueConfig.h")
    CONFIGURE_FILE(${cmakeDir}/torqueConfig.h.in ${projectSrcDir}/torqueConfig.h)
    CONFIGURE_FILE(${cmakeDir}/torque-win.rc.in ${projectSrcDir}/torque-win.rc)
    CONFIGURE_FILE(${cmakeDir}/torque.ico ${projectSrcDir}/torque.ico COPYONLY)
endif()


# internal libs
set(t3dLibs "${t3dLibs};lmng;lpng;lungif;ljpeg;zlib;tinyxml;opcode;squish;collada;pcre;convexDecomp")
#set(t3dLibs "${t3dLibs};libtorque3d")
link_directories(${t3dLibDirs})

#addLibraryFinal("${t}" "libtorque3d")
addExecutableFinal("${t}" "torque3d" "${srcDir}")

target_link_libraries(torque3d ${t3dLibs})

INSTALL_TARGETS(/ torque3d)
INSTALL_FILES(/ FILES ${CMAKE_SOURCE_DIR}/Templates/${TORQUE_TEMPLATE}/game/)
