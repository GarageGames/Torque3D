//-----------------------------------------------------------------------------
// Copyright (c) 2012 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#ifndef _GAMEFUNCTIONS_H_
#define _GAMEFUNCTIONS_H_

#ifndef _MPOINT3_H_
#include "math/mPoint3.h"
#endif
#ifndef _MMATRIX_H_
#include "math/mMatrix.h"
#endif

struct CameraQuery;


/// Actually renders the world.  This is the function that will render the
/// scene ONLY - new guis, no damage flashes.
void GameRenderWorld();

/// Renders overlays such as damage flashes, white outs, and water masks.  
/// These are usually a color applied over the entire screen.
void GameRenderFilters(const CameraQuery& camq);

/// Does the same thing as GameGetCameraTransform, but fills in other data 
/// including information about the far and near clipping planes.
bool GameProcessCameraQuery(CameraQuery *query);

/// Gets the position, rotation, and velocity of the camera.
bool GameGetCameraTransform(MatrixF *mat, Point3F *velocity);

/// Gets the camera field of view angle.
F32 GameGetCameraFov();

/// Sets the field of view angle of the camera.
void GameSetCameraFov(F32 fov);

/// Sets where the camera fov will be change to.  This is for 
/// non-instantaneous zooms/retractions.
void GameSetCameraTargetFov(F32 fov);

/// Update the camera fov to be closer to the target fov.
void GameUpdateCameraFov();

#endif // _GAMEFUNCTIONS_H_
