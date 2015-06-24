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

#include "coverPoint.h"

#include "math/mathIO.h"
#include "scene/sceneRenderState.h"
#include "core/stream/bitStream.h"
#include "materials/sceneData.h"
#include "gfx/gfxDebugEvent.h"
#include "gfx/gfxTransformSaver.h"
#include "gfx/gfxDrawUtil.h"
#include "renderInstance/renderPassManager.h"
#include "console/engineAPI.h"

extern bool gEditingMission;

IMPLEMENT_CO_NETOBJECT_V1(CoverPoint);

ConsoleDocClass(CoverPoint,
   "@brief A type of marker that designates a location AI characters can take cover.\n\n"
);

ImplementEnumType(CoverPointSize,
   "The size of a cover point.\n")
   { CoverPoint::Prone,  "Prone",  "Only provides cover when prone.\n" },
   { CoverPoint::Crouch, "Crouch", "Only provides cover when crouching.\n" },
   { CoverPoint::Stand,  "Stand",  "Provides cover when standing.\n" },
EndImplementEnumType;

//-----------------------------------------------------------------------------
// Object setup and teardown
//-----------------------------------------------------------------------------
CoverPoint::CoverPoint()
{
   mNetFlags.clear(Ghostable);
   mTypeMask |= MarkerObjectType;
   mSize = Stand;
   mQuality = 1.0f;
   mOccupied = false;
   mPeekLeft = false;
   mPeekRight = false;
   mPeekOver = false;
}

CoverPoint::~CoverPoint()
{
}

//-----------------------------------------------------------------------------
// Object Editing
//-----------------------------------------------------------------------------
void CoverPoint::initPersistFields()
{
   addGroup("CoverPoint");

   addField("size", TYPEID<CoverPointSize>(), Offset(mSize, CoverPoint),
      "The size of this cover point.");

   addField("quality", TypeF32, Offset(mQuality, CoverPoint),
      "Reliability of this point as solid cover. (0...1)");

   addField("peekLeft", TypeBool, Offset(mPeekLeft, CoverPoint),
      "Can characters look left around this cover point?");
   addField("peekRight", TypeBool, Offset(mPeekRight, CoverPoint),
      "Can characters look right around this cover point?");
   addField("peekOver", TypeBool, Offset(mPeekOver, CoverPoint),
      "Can characters look over the top of this cover point?");

   endGroup("CoverPoint");

   Parent::initPersistFields();
}

bool CoverPoint::onAdd()
{
   if(!Parent::onAdd())
      return false;

   // Set up a 1x1x1 bounding box
   mObjBox.set(Point3F(-0.5f, -0.5f, -0.5f),
               Point3F( 0.5f,  0.5f,  0.5f));
   resetWorldBox();

   if(gEditingMission)
      onEditorEnable();

   addToScene();

   return true;
}

void CoverPoint::onRemove()
{
   if(gEditingMission)
      onEditorDisable();

   removeFromScene();

   Parent::onRemove();

   for(U32 i = 0; i < NumSizes; i++)
      smVertexBuffer[i] = NULL;
}

void CoverPoint::setTransform(const MatrixF & mat)
{
   Parent::setTransform(mat);
   setMaskBits(TransformMask);
}

void CoverPoint::onEditorEnable()
{
   mNetFlags.set(Ghostable);
}

void CoverPoint::onEditorDisable()
{
   mNetFlags.clear(Ghostable);
}

void CoverPoint::inspectPostApply()
{
   setMaskBits(TransformMask);
}

U32 CoverPoint::packUpdate(NetConnection *conn, U32 mask, BitStream *stream)
{
   U32 retMask = Parent::packUpdate(conn, mask, stream);

   stream->writeInt(mSize, 4);

   stream->writeFlag(mOccupied);

   stream->writeFlag(peekLeft());
   stream->writeFlag(peekRight());
   stream->writeFlag(peekOver());

   // Write our transform information
   if(stream->writeFlag(mask & TransformMask))
   {
      mathWrite(*stream, getTransform());
      mathWrite(*stream, getScale());
   }

   return retMask;
}

void CoverPoint::unpackUpdate(NetConnection *conn, BitStream *stream)
{
   Parent::unpackUpdate(conn, stream);

   mSize = (Size)stream->readInt(4);

   setOccupied(stream->readFlag());

   mPeekLeft = stream->readFlag();
   mPeekRight = stream->readFlag();
   mPeekOver = stream->readFlag();

   if(stream->readFlag()) // TransformMask
   {
      mathRead(*stream, &mObjToWorld);
      mathRead(*stream, &mObjScale);

      setTransform(mObjToWorld);
   }
}

//-----------------------------------------------------------------------------
// Functionality
//-----------------------------------------------------------------------------

Point3F CoverPoint::getNormal() const
{
   return getTransform().getForwardVector();
}

DefineEngineMethod(CoverPoint, isOccupied, bool, (),,
   "@brief Returns true if someone is already using this cover point.")
{
   return object->isOccupied();
}

//-----------------------------------------------------------------------------
// Object Rendering
//-----------------------------------------------------------------------------

GFXStateBlockRef CoverPoint::smNormalSB;
GFXVertexBufferHandle<CoverPoint::VertexType> CoverPoint::smVertexBuffer[CoverPoint::NumSizes];

void CoverPoint::initGFXResources()
{
   if(smVertexBuffer[0] != NULL)
      return;

   static const Point3F planePoints[4] = 
   {
      Point3F(-1.0f, 0.0f, 0.0f), Point3F(-1.0f, 0.0f, 2.0f),
      Point3F( 1.0f, 0.0f, 0.0f), Point3F( 1.0f, 0.0f, 2.0f),
   };

   static const U32 planeFaces[6] =
   {
      0, 1, 2,
      1, 2, 3
   };

   static const Point3F scales[NumSizes] =
   {
      Point3F(1.0f, 1.0f, 0.5f), // Prone
      Point3F(1.0f, 1.0f, 1.0f), // Crouch
      Point3F(1.0f, 1.0f, 2.0f)  // Stand
   };

   static const ColorI colours[NumSizes] =
   {
      ColorI(180,   0,  0, 128), // Prone
      ColorI(250, 200, 90, 128), // Crouch
      ColorI( 80, 190, 20, 128)  // Stand
   };

   for(U32 i = 0; i < NumSizes; i++)
   {
      // Fill the vertex buffer
      VertexType *pVert = NULL;
      smVertexBuffer[i].set(GFX, 6, GFXBufferTypeStatic);

      pVert = smVertexBuffer[i].lock();
      for(U32 j = 0; j < 6; j++)
      {
         pVert[j].point  = planePoints[planeFaces[j]] * scales[i] * 0.5f;
         pVert[j].normal = Point3F(0.0f, -1.0f, 0.0f);
         pVert[j].color  = colours[i];
      }
      smVertexBuffer[i].unlock();
   }

   // Set up our StateBlock
   GFXStateBlockDesc desc;
   desc.cullDefined = true;
   desc.cullMode = GFXCullNone;
   desc.setBlend(true);
   smNormalSB = GFX->createStateBlock(desc);
}

void CoverPoint::prepRenderImage(SceneRenderState *state)
{
   // Allocate an ObjectRenderInst so that we can submit it to the RenderPassManager
   ObjectRenderInst *ri = state->getRenderPass()->allocInst<ObjectRenderInst>();

   // Now bind our rendering function so that it will get called
   ri->renderDelegate.bind(this, &CoverPoint::render);

   // Set our RenderInst as a standard object render
   ri->type = RenderPassManager::RIT_Editor;

   // Set our sorting keys to a default value
   ri->defaultKey = 0;
   ri->defaultKey2 = 0;

   // Submit our RenderInst to the RenderPassManager
   state->getRenderPass()->addInst(ri);
}

void CoverPoint::render(ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance *overrideMat)
{
   initGFXResources();

   if(overrideMat)
      return;

   if(smVertexBuffer[mSize].isNull())
      return;

   PROFILE_SCOPE(CoverPoint_Render);

   // Set up a GFX debug event (this helps with debugging rendering events in external tools)
   GFXDEBUGEVENT_SCOPE(CoverPoint_Render, ColorI::RED);

   // GFXTransformSaver is a handy helper class that restores
   // the current GFX matrices to their original values when
   // it goes out of scope at the end of the function
   GFXTransformSaver saver;

   // Calculate our object to world transform matrix
   MatrixF objectToWorld = getRenderTransform();
   objectToWorld.scale(getScale());

   // Apply our object transform
   GFX->multWorld(objectToWorld);

   // Set the state block
   GFX->setStateBlock(smNormalSB);

   // Set up the "generic" shaders
   // These handle rendering on GFX layers that don't support
   // fixed function. Otherwise they disable shaders.
   GFX->setupGenericShaders(GFXDevice::GSModColorTexture);

   // Set the vertex buffer
   GFX->setVertexBuffer(smVertexBuffer[mSize]);

   // Draw our triangles
   GFX->drawPrimitive(GFXTriangleList, 0, 2);

   // Data for decorations.
   GFXStateBlockDesc desc;
   F32 height = (float)(mSize + 1) / NumSizes * 2.0f;

   // Draw an X if we're occupied.
   if(isOccupied())
   {
      GFX->getDrawUtil()->drawArrow(desc, Point3F(-0.5, 0, 0), Point3F(0.5, 0, height), ColorI::RED);
      GFX->getDrawUtil()->drawArrow(desc, Point3F(0.5, 0, 0), Point3F(-0.5, 0, height), ColorI::RED);
   }

   // Draw arrows to represent peek directions.
   if(peekLeft())
      GFX->getDrawUtil()->drawArrow(desc, Point3F(0, 0, height * 0.5), Point3F(-0.5, 0, height * 0.5), ColorI::GREEN);
   if(peekRight())
      GFX->getDrawUtil()->drawArrow(desc, Point3F(0, 0, height * 0.5), Point3F(0.5, 0, height * 0.5), ColorI::GREEN);
   if(peekOver())
      GFX->getDrawUtil()->drawArrow(desc, Point3F(0, 0, height * 0.5), Point3F(0, 0, height), ColorI::GREEN);
}
