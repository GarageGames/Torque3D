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

#ifndef _DECALINSTANCE_H_
#define _DECALINSTANCE_H_

#ifndef _GFXVERTEXBUFFER_H_
#include "gfx/gfxVertexBuffer.h"
#endif

#ifndef _DECALDATA_H_
#include "T3D/decal/decalData.h"
#endif

struct DecalVertex;
class SceneRenderState;

/// DecalInstance represents a rendering decal in the scene.
/// You should not allocate this yourself, add new decals to the scene
/// via the DecalManager.
/// All data is public, change it if you wish, but be sure to inform
/// the DecalManager.
class DecalInstance
{
   public:

      DecalData *mDataBlock;

      Point3F mPosition;
      Point3F mNormal;
      Point3F mTangent;
      F32 mRotAroundNormal;   
      F32 mSize;

      U32 mCreateTime;
      F32 mVisibility;

      F32 mLastAlpha;

      U32 mTextureRectIdx;      

      DecalVertex *mVerts;
      U16 *mIndices;

      U32 mVertCount;
      U32 mIndxCount;

      U8 mFlags;

      U8 mRenderPriority;

      S32 mId;

      GFXTexHandle *mCustomTex;

      void getWorldMatrix( MatrixF *outMat, bool flip = false );
      
      Box3F getWorldBox() const
      {
         return Box3F( mPosition - Point3F( mSize / 2.f ), mPosition + Point3F( mSize / 2.f ) );
      }

      U8 getRenderPriority() const
      {
         return mRenderPriority == 0 ? mDataBlock->renderPriority : mRenderPriority;
      }

      /// Calculates the size of this decal onscreen in pixels, used for LOD.
      F32 calcPixelSize( U32 viewportHeight, const Point3F &cameraPos, F32 worldToScreenScaleY ) const;
   		
	   DecalInstance() : mId(-1) {}   
};

#endif // _DECALINSTANCE_H_
