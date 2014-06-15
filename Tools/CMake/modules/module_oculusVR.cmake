# module OculusVR
 
# Source
addPathRec( "${srcDir}/platform/input/oculusVR" )

# Includes
addInclude( "${TORQUE_OCULUSVR_SDK_PATH}/LibOVR/Include" )
addInclude( "${TORQUE_OCULUSVR_SDK_PATH}/LibOVR/Src" )
 
# Libs
if( WIN32 ) 
    link_directories( "${TORQUE_OCULUSVR_SDK_PATH}/LibOVR/Lib/Win32" )
    addLib( "libovr" )
    addLib( "libovrd" )
endif()