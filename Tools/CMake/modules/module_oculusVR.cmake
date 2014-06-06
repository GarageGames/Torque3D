# module OculusVR
 
# Source
addPathRec( "${srcDir}/platform/input/oculusVR" )

# Includes
include_directories( "${TORQUE_OCULUSVR_SDK_PATH}/LibOVR/Include" )
include_directories( "${TORQUE_OCULUSVR_SDK_PATH}/LibOVR/Src" )
 
# Libs
if( WIN32 ) 
    link_directories( "${TORQUE_OCULUSVR_SDK_PATH}/LibOVR/Lib/Win32" )
    addRequiredLink( "libovr.lib" )
    addRequiredLink( "libovrd.lib" )
endif()