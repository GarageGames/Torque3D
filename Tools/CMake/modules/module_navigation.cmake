# Navigation module

addDef( "TORQUE_NAVIGATION_ENABLED" )
addLib( "recast" )

# files
addPathRec( "${srcDir}/navigation" )

# include paths
addInclude( "${libDir}/recast/DebugUtils/Include" )
addInclude( "${libDir}/recast/Recast/Include" )
addInclude( "${libDir}/recast/Detour/Include" )
addInclude( "${libDir}/recast/DetourTileCache/Include" )
addInclude( "${libDir}/recast/DetourCrowd/Include" )

