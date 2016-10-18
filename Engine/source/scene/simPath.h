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

#ifndef _SIMPATH_H_
#define _SIMPATH_H_

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


namespace SimPath
{

//--------------------------------------------------------------------------
/// A path!
class Path : public SimGroup
{
   typedef SimGroup Parent;

  public:
   enum : U32
   {
      NoPathIndex = 0xFFFFFFFF
   };


  private:
   U32 mPathIndex;
   bool mIsLooping;

  protected:
   bool onAdd();
   void onRemove();

  public:
   Path();
   ~Path();

   void addObject(SimObject*);
   void removeObject(SimObject*);

   void sortMarkers();
   void updatePath();
   bool isLooping() { return mIsLooping; }
   U32 getPathIndex() const;

   DECLARE_CONOBJECT(Path);
   static void initPersistFields();
};

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
inline U32 Path::getPathIndex() const
{
   return mPathIndex;
}

} // Namespace


//--------------------------------------------------------------------------
class Marker : public SceneObject
{
   typedef SceneObject Parent;
   friend class Path;

  public:
   enum SmoothingType
   {
      SmoothingTypeLinear,
      SmoothingTypeSpline,
      SmoothingTypeAccelerate,
   };

   enum KnotType
   {
      KnotTypeNormal,
      KnotTypePositionOnly,
      KnotTypeKink,
   };


   U32   mSeqNum;
   U32   mSmoothingType;
   U32   mKnotType;

   U32   mMSToNext;

   // Rendering
  protected:
   void prepRenderImage(SceneRenderState *state);
   void renderObject(ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance* overrideMat);

  protected:
   bool onAdd();
   void onRemove();
   void onGroupAdd();

   void onEditorEnable();
   void onEditorDisable();
   
   static void initGFXResources();

   static GFXStateBlockRef smStateBlock;
   static GFXVertexBufferHandle<GFXVertexPCT> smVertexBuffer;
   static GFXPrimitiveBufferHandle smPrimitiveBuffer;

  public:
   Marker();
   ~Marker();

   DECLARE_CONOBJECT(Marker);
   static void initPersistFields();
   void inspectPostApply();

   U32  packUpdate  (NetConnection *conn, U32 mask, BitStream *stream);
   void unpackUpdate(NetConnection *conn,           BitStream *stream);
};

typedef Marker::SmoothingType MarkerSmoothingType;
typedef Marker::KnotType MarkerKnotType;

DefineEnumType( MarkerSmoothingType );
DefineEnumType( MarkerKnotType );

#endif // _H_PATH

