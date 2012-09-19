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

#ifndef _MIRRORSUBOBJECT_H_
#define _MIRRORSUBOBJECT_H_

#ifndef _INTERIORSUBOBJECT_H_
#include "interior/interiorSubObject.h"
#endif

#include "gfx/gfxTextureHandle.h"

class TextureHandle;

class MirrorSubObject : public InteriorSubObject
{
   typedef InteriorSubObject Parent;

  public:
   U32   mDetailLevel;
   U32   mZone;

   F32     mAlphaLevel;
   Point3F mCentroid;

   U32   surfaceCount;
   U32   surfaceStart;

  private:
   bool           mInitialized;
   GFXTexHandle*  mWhite;
   MatrixF        mReflectionMatrix;

   bool  isInitialized() const { return mInitialized; }
   void  setupTransforms();


   // ISO overrides
  protected:
   U32  getSubObjectKey() const;
   bool _readISO(Stream&);
   bool _writeISO(Stream&) const;

   // Render control.  A sub-object should return false from renderDetailDependant if
   //  it exists only at the level-0 detail level, ie, doors, elevators, etc., true
   //  if should only render at the interiors detail, ie, translucencies.
   //SubObjectRenderImage* getRenderImage(SceneRenderState*, const Point3F&);
   bool                  renderDetailDependant() const;
   U32                   getZone() const;
   void                  noteTransformChange();

   InteriorSubObject*  clone(InteriorInstance*) const;

   // Rendering
  protected:
   //void renderObject(SceneRenderState*, SceneRenderImage*);
   void transformModelview(const U32, const MatrixF&, MatrixF*);
   void transformPosition(const U32, Point3F&);
   bool computeNewFrustum(    const U32      portalIndex,
                              const Frustum  &oldFrustum,
                              const F64      nearPlane,
                              const F64      farPlane,
                              const RectI&   oldViewport,
                              F64            *newFrustum,
                              RectI&         newViewport,
                              const bool     flippedMatrix );
   void openPortal(const U32   portalIndex,
                   SceneRenderState* pCurrState,
                   SceneRenderState* pParentState);
   void closePortal(const U32   portalIndex,
                    SceneRenderState* pCurrState,
                    SceneRenderState* pParentState);
   void getWSPortalPlane(const U32 portalIndex, PlaneF*);


  public:
   MirrorSubObject();
   ~MirrorSubObject();

   DECLARE_CONOBJECT(MirrorSubObject);
   static void initPersistFields();
};

#endif // _H_MIRRORSUBOBJECT

