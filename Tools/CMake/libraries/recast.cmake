# -----------------------------------------------------------------------------
# Copyright (c) 2014 GarageGames, LLC
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