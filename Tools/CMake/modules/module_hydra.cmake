# module OculusVR
 
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

