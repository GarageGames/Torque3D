# -----------------------------------------------------------------------------
# Copyright (c) 2015 GarageGames, LLC
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

project(libbullet)

addPath( "${libDir}/bullet/src" )
addPath( "${libDir}/bullet/src/BulletCollision" )
addPath( "${libDir}/bullet/src/BulletCollision/BroadphaseCollision" )
addPath( "${libDir}/bullet/src/BulletCollision/CollisionDispatch" )
addPath( "${libDir}/bullet/src/BulletCollision/CollisionShapes" )
addPath( "${libDir}/bullet/src/BulletCollision/Gimpact" )
addPath( "${libDir}/bullet/src/BulletCollision/NarrowPhaseCollision" )
addPath( "${libDir}/bullet/src/BulletDynamics" )
addPath( "${libDir}/bullet/src/BulletDynamics/Character" )
addPath( "${libDir}/bullet/src/BulletDynamics/ConstraintSolver" )
addPath( "${libDir}/bullet/src/BulletDynamics/Dynamics" )
addPath( "${libDir}/bullet/src/BulletDynamics/Vehicle" )
addPath( "${libDir}/bullet/src/LinearMath" )


addInclude( "${libDir}/bullet/src" )

finishLibrary()