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

#include "platform/platform.h"
#include "scene/sceneCameraState.h"

#include "gfx/gfxDevice.h"


//-----------------------------------------------------------------------------

SceneCameraState::SceneCameraState( const RectI& viewport, const Frustum& frustum, const MatrixF& worldView, const MatrixF& projection )
   : mViewport( viewport ),
     mFrustum( frustum ),
     mWorldViewMatrix( worldView ),
     mHeadWorldViewMatrix( worldView ),
     mProjectionMatrix( projection )
{
   mViewDirection = frustum.getTransform().getForwardVector();
}

//-----------------------------------------------------------------------------

SceneCameraState SceneCameraState::fromGFX( )
{
   return fromGFXWithViewport( GFX->getViewport() );
}

//-----------------------------------------------------------------------------

SceneCameraState SceneCameraState::fromGFXWithViewport( const RectI& viewport )
{
   const MatrixF& world = GFX->getWorldMatrix();
   
   MatrixF camera = world;
   camera.inverse();

   Frustum frustum = GFX->getFrustum();
   frustum.setTransform( camera );

   SceneCameraState ret = SceneCameraState(
      viewport,
      frustum,
      world,
      GFX->getProjectionMatrix()
   );

   // If rendering to stereo, make sure we get the head matrix
   S32 stereoTarget = GFX->getCurrentStereoTarget();
   if (stereoTarget != -1)
   {
      ret.mHeadWorldViewMatrix = GFX->getStereoHeadTransform();
      ret.mHeadWorldViewMatrix.inverse();
   }

   return ret;
}
