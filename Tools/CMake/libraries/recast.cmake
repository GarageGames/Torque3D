# Recast library
project(recast)

# Source
addPathRec( "${libDir}/recast/DebugUtils/Source" )
addPathRec( "${libDir}/recast/Recast/Source" )
addPathRec( "${libDir}/recast/Detour/Source" )
addPathRec( "${libDir}/recast/DetourCrowd/Source" )
addPathRec( "${libDir}/recast/DetourTileCache/Source" )

# Additional includes
addInclude( "${libDir}/recast/DebugUtils/Include" )
addInclude( "${libDir}/recast/Recast/Include" )
addInclude( "${libDir}/recast/Detour/Include" )
addInclude( "${libDir}/recast/DetourTileCache/Include" )
addInclude( "${libDir}/recast/DetourCrowd/Include" )

finishLibrary()