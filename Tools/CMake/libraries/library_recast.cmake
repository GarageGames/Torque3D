# Recast library
project(recast)

# Source
addPathRec( "${libDir}/recast/DebugUtils/Source" )
addPathRec( "${libDir}/recast/Recast/Source" )
addPathRec( "${libDir}/recast/Detour/Source" )
addPathRec( "${libDir}/recast/DetourCrowd/Source" )
addPathRec( "${libDir}/recast/DetourTileCache/Source" )

# Additional includes
include_directories( "${libDir}/recast/DebugUtils/Include" )
include_directories( "${libDir}/recast/Recast/Include" )
include_directories( "${libDir}/recast/Detour/Include" )
include_directories( "${libDir}/recast/DetourTileCache/Include" )
include_directories( "${libDir}/recast/DetourCrowd/Include" )

addStaticLib()