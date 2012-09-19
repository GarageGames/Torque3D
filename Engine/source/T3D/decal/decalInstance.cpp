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
#include "T3D/decal/decalInstance.h"
#include "scene/sceneRenderState.h"

void DecalInstance::getWorldMatrix( MatrixF *outMat, bool flip )
{
   outMat->setPosition( mPosition );

   Point3F fvec;
   mCross( mNormal, mTangent, &fvec );

   outMat->setColumn( 0, mTangent );
   outMat->setColumn( 1, fvec );
   outMat->setColumn( 2, mNormal );
}

F32 DecalInstance::calcPixelSize( U32 viewportHeight, const Point3F &cameraPos, F32 worldToScreenScaleY ) const
{  
   // If fadeStartPixelSize is set less than zero this is interpreted to mean
   // pixelSize based fading is disabled and the decal always renders.
   // Returning a value of F32_MAX should be treated by the caller as
   // meaning "big enough to render at normal quality, dont LOD me.".
   if ( mDataBlock->fadeStartPixelSize < 0.0f )
      return F32_MAX;

   // This is an approximation since mSize is actually the xyz size of the
   // cube we use to clip geometry for the decal. In this case we are assuming
   // it is appropriate to use the radius of the sphere 'inscribed' in that
   // cube as the equivalent of mShape->radius for purposes of calculating
   // the pixelScale ( see TSShapeInstance::setDetailFromDistance ).
   const F32 radius = mSize * 0.5f;

   // Approximate distance to the decal.    
   const F32 distance = ( cameraPos - mPosition ).len() - radius;

   // We are inside the decal's volume. There is no useful pixelSize for us
   // to return, it is essentially the entire screen.
   if ( distance <= 0.0f )
      return F32_MAX;

   // Lod is relative to a viewport with a heigh of 300.  Could prescale this into decalSize...
   const F32 pixelScale = viewportHeight / 300.0f;

   // Inline of SceneState::projectRadius.
   const F32 pixelSize = mSize / distance * worldToScreenScaleY * pixelScale;

   // Optionally scale pixelSize by a detail adjust value here...
   // eg. pixelSize *= TSDetailAdjust...

   return pixelSize;
}
