# Navigation module

addRequiredDefinition( "TORQUE_NAVIGATION_ENABLED" )
addRequiredLibrary( "libraries/library_recast.cmake" )
addRequiredLink( "recast" )

# files
addPathRec( "${srcDir}/navigation" )

# include paths
include_directories( "${libDir}/recast/DebugUtils/Include" )
include_directories( "${libDir}/recast/Recast/Include" )
include_directories( "${libDir}/recast/Detour/Include" )
include_directories( "${libDir}/recast/DetourTileCache/Include" )
include_directories( "${libDir}/recast/DetourCrowd/Include" )

