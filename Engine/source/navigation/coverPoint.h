//-----------------------------------------------------------------------------
// Copyright (c) 2014 Daniel Buckmaster
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

#ifndef _COVERPOINT_H_
#define _COVERPOINT_H_

#ifndef _SCENEOBJECT_H_
#include "scene/sceneObject.h"
#endif
#ifndef _GFXSTATEBLOCK_H_
#include "gfx/gfxStateBlock.h"
#endif
#ifndef _GFXVERTEXBUFFER_H_
#include "gfx/gfxVertexBuffer.h"
#endif
#ifndef _GFXPRIMITIVEBUFFER_H_
#include "gfx/gfxPrimitiveBuffer.h"
#endif

class BaseMatInstance;

class CoverPoint : public SceneObject
{
   typedef SceneObject Parent;

   /// Network mask bits.
   enum MaskBits 
   {
      TransformMask = Parent::NextFreeMask << 0,
      NextFreeMask  = Parent::NextFreeMask << 1
   };

public:
   CoverPoint();
   virtual ~CoverPoint();

   DECLARE_CONOBJECT(CoverPoint);

   /// Amount of cover provided at this point.
   enum Size {
      Prone,
      Crouch,
      Stand,
      NumSizes
   };

   void setSize(Size size) { mSize = size; setMaskBits(TransformMask); }
   Size getSize() const { return mSize; }

   /// Is this point occupied?
   bool isOccupied() const { return mOccupied; }
   void setOccupied(bool occ) { mOccupied = occ; setMaskBits(TransformMask); }

   /// Get the normal of this point (i.e., the direction from which it provides cover).
   Point3F getNormal() const;

   F32 getQuality() const { return mQuality; }

   bool peekLeft() const { return mPeekLeft; }
   bool peekRight() const { return mPeekRight; }
   bool peekOver() const { return mPeekOver; }

   void setPeek(bool left, bool right, bool over) { mPeekLeft = left, mPeekRight = right, mPeekOver = over; }

   /// Init static buffers for rendering.
   static void initGFXResources();

   /// @name SceneObject
   /// @{
   static void initPersistFields();

   bool onAdd();
   void onRemove();

   void onEditorEnable();
   void onEditorDisable();
   void inspectPostApply();

   void setTransform(const MatrixF &mat);

   void prepRenderImage(SceneRenderState *state);
   void render(ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance *overrideMat);
   /// @}

   /// @name NetObject
   /// @{
   U32 packUpdate(NetConnection *conn, U32 mask, BitStream *stream);
   void unpackUpdate(NetConnection *conn, BitStream *stream);
   /// @}

protected:

private:
   typedef GFXVertexPCN VertexType;
   static GFXStateBlockRef smNormalSB;
   static GFXVertexBufferHandle<VertexType> smVertexBuffer[NumSizes];

   /// Size of this point.
   Size mSize;
   /// Quality of this point as cover.
   F32 mQuality;

   /// Can we look left around this cover?
   bool mPeekLeft;
   /// Can we look right around this cover?
   bool mPeekRight;
   /// Can we look up over this cover?
   bool mPeekOver;

   /// Is this point currently occupied?
   bool mOccupied;
};

typedef CoverPoint::Size CoverPointSize;
DefineEnumType(CoverPointSize);

#endif
