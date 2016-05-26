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
#include "T3D/components/collision/collisionTrigger.h"

#include "scene/sceneRenderState.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "collision/boxConvex.h"

#include "core/stream/bitStream.h"
#include "math/mathIO.h"
#include "gfx/gfxTransformSaver.h"
#include "renderInstance/renderPassManager.h"
#include "gfx/gfxDrawUtil.h"
#include "T3D/physics/physicsPlugin.h"
#include "T3D/physics/physicsBody.h"
#include "T3D/physics/physicsCollision.h"


bool CollisionTrigger::smRenderCollisionTriggers = false;

//-----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//--------------------------------------------------------------------------

IMPLEMENT_CO_NETOBJECT_V1(CollisionTrigger);

ConsoleDocClass(CollisionTrigger,
   "@brief A CollisionTrigger is a volume of space that initiates script callbacks "
   "when objects pass through the CollisionTrigger.\n\n"

   "CollisionTriggerData provides the callbacks for the CollisionTrigger when an object enters, stays inside "
   "or leaves the CollisionTrigger's volume.\n\n"

   "@see CollisionTriggerData\n"
   "@ingroup gameObjects\n"
   );

IMPLEMENT_CALLBACK(CollisionTrigger, onAdd, void, (U32 objectId), (objectId),
   "@brief Called when the CollisionTrigger is being created.\n\n"
   "@param objectId the object id of the CollisionTrigger being created\n");

IMPLEMENT_CALLBACK(CollisionTrigger, onRemove, void, (U32 objectId), (objectId),
   "@brief Called just before the CollisionTrigger is deleted.\n\n"
   "@param objectId the object id of the CollisionTrigger being deleted\n");

CollisionTrigger::CollisionTrigger()
{
   // Don't ghost by default.
   mNetFlags.set(Ghostable | ScopeAlways);

   mTypeMask |= TriggerObjectType;

   mObjScale.set(1, 1, 1);
   mObjToWorld.identity();
   mWorldToObj.identity();

   mLastThink = 0;
   mCurrTick = 0;

   mConvexList = new Convex;

   mPhysicsRep = NULL;
}

CollisionTrigger::~CollisionTrigger()
{
   delete mConvexList;
   mConvexList = NULL;
   SAFE_DELETE(mPhysicsRep);
}

bool CollisionTrigger::castRay(const Point3F &start, const Point3F &end, RayInfo* info)
{
   // Collide against bounding box
   F32 st, et, fst = 0, fet = 1;
   F32 *bmin = &mObjBox.minExtents.x;
   F32 *bmax = &mObjBox.maxExtents.x;
   F32 const *si = &start.x;
   F32 const *ei = &end.x;

   for (S32 i = 0; i < 3; i++)
   {
      if (*si < *ei)
      {
         if (*si > *bmax || *ei < *bmin)
            return false;
         F32 di = *ei - *si;
         st = (*si < *bmin) ? (*bmin - *si) / di : 0;
         et = (*ei > *bmax) ? (*bmax - *si) / di : 1;
      }
      else
      {
         if (*ei > *bmax || *si < *bmin)
            return false;
         F32 di = *ei - *si;
         st = (*si > *bmax) ? (*bmax - *si) / di : 0;
         et = (*ei < *bmin) ? (*bmin - *si) / di : 1;
      }
      if (st > fst) fst = st;
      if (et < fet) fet = et;
      if (fet < fst)
         return false;
      bmin++; bmax++;
      si++; ei++;
   }

   info->normal = start - end;
   info->normal.normalizeSafe();
   getTransform().mulV(info->normal);

   info->t = fst;
   info->object = this;
   info->point.interpolate(start, end, fst);
   info->material = 0;
   return true;
}

//-----------------------------------------------------------------------------
void CollisionTrigger::consoleInit()
{
   Con::addVariable("$CollisionTrigger::renderCollisionTriggers", TypeBool, &smRenderCollisionTriggers,
      "@brief Forces all CollisionTrigger's to render.\n\n"
      "Used by the Tools and debug render modes.\n"
      "@ingroup gameObjects");
}

void CollisionTrigger::initPersistFields()
{
   addField("polyhedron", TypeTriggerPolyhedron, Offset(mCollisionTriggerPolyhedron, CollisionTrigger),
      "@brief Defines a non-rectangular area for the CollisionTrigger.\n\n"
      "Rather than the standard rectangular bounds, this optional parameter defines a quadrilateral "
      "CollisionTrigger area.  The quadrilateral is defined as a corner point followed by three vectors "
      "representing the edges extending from the corner.\n");

   addProtectedField("enterCommand", TypeCommand, Offset(mEnterCommand, CollisionTrigger), &setEnterCmd, &defaultProtectedGetFn,
      "The command to execute when an object enters this CollisionTrigger. Object id stored in %%obj. Maximum 1023 characters.");
   addProtectedField("leaveCommand", TypeCommand, Offset(mLeaveCommand, CollisionTrigger), &setLeaveCmd, &defaultProtectedGetFn,
      "The command to execute when an object leaves this CollisionTrigger. Object id stored in %%obj. Maximum 1023 characters.");
   addProtectedField("tickCommand", TypeCommand, Offset(mTickCommand, CollisionTrigger), &setTickCmd, &defaultProtectedGetFn,
      "The command to execute while an object is inside this CollisionTrigger. Maximum 1023 characters.");

   Parent::initPersistFields();
}

bool CollisionTrigger::setEnterCmd(void *object, const char *index, const char *data)
{
   static_cast<CollisionTrigger*>(object)->setMaskBits(EnterCmdMask);
   return true; // to update the actual field
}

bool CollisionTrigger::setLeaveCmd(void *object, const char *index, const char *data)
{
   static_cast<CollisionTrigger*>(object)->setMaskBits(LeaveCmdMask);
   return true; // to update the actual field
}

bool CollisionTrigger::setTickCmd(void *object, const char *index, const char *data)
{
   static_cast<CollisionTrigger*>(object)->setMaskBits(TickCmdMask);
   return true; // to update the actual field
}

//--------------------------------------------------------------------------

bool CollisionTrigger::onAdd()
{
   if (!Parent::onAdd())
      return false;

   onAdd_callback(getId());

   Polyhedron temp = mCollisionTriggerPolyhedron;
   setTriggerPolyhedron(temp);

   addToScene();

   if (isServerObject())
      scriptOnAdd();

   return true;
}

void CollisionTrigger::onRemove()
{
   onRemove_callback(getId());

   mConvexList->nukeList();

   removeFromScene();
   Parent::onRemove();
}

bool CollisionTrigger::onNewDataBlock(GameBaseData *dptr, bool reload)
{
   return true;
}

void CollisionTrigger::onDeleteNotify(SimObject *obj)
{
   GameBase* pScene = dynamic_cast<GameBase*>(obj);

   if (pScene != NULL)
   {
      for (U32 i = 0; i < mObjects.size(); i++)
      {
         if (pScene == mObjects[i])
         {
            mObjects.erase(i);
            //onLeaveCollisionTrigger_callback(this, pScene);
            break;
         }
      }
   }

   Parent::onDeleteNotify(obj);
}

void CollisionTrigger::inspectPostApply()
{
   setTriggerPolyhedron(mCollisionTriggerPolyhedron);
   setMaskBits(PolyMask);
   Parent::inspectPostApply();
}

//--------------------------------------------------------------------------

void CollisionTrigger::buildConvex(const Box3F& box, Convex* convex)
{
   // These should really come out of a pool
   mConvexList->collectGarbage();

   Box3F realBox = box;
   mWorldToObj.mul(realBox);
   realBox.minExtents.convolveInverse(mObjScale);
   realBox.maxExtents.convolveInverse(mObjScale);

   if (realBox.isOverlapped(getObjBox()) == false)
      return;

   // Just return a box convex for the entire shape...
   Convex* cc = 0;
   CollisionWorkingList& wl = convex->getWorkingList();
   for (CollisionWorkingList* itr = wl.wLink.mNext; itr != &wl; itr = itr->wLink.mNext) {
      if (itr->mConvex->getType() == BoxConvexType &&
         itr->mConvex->getObject() == this) {
         cc = itr->mConvex;
         break;
      }
   }
   if (cc)
      return;

   // Create a new convex.
   BoxConvex* cp = new BoxConvex;
   mConvexList->registerObject(cp);
   convex->addToWorkingList(cp);
   cp->init(this);

   mObjBox.getCenter(&cp->mCenter);
   cp->mSize.x = mObjBox.len_x() / 2.0f;
   cp->mSize.y = mObjBox.len_y() / 2.0f;
   cp->mSize.z = mObjBox.len_z() / 2.0f;
}


//------------------------------------------------------------------------------

void CollisionTrigger::setTransform(const MatrixF & mat)
{
   Parent::setTransform(mat);

   if (mPhysicsRep)
      mPhysicsRep->setTransform(mat);

   if (isServerObject()) {
      MatrixF base(true);
      base.scale(Point3F(1.0 / mObjScale.x,
         1.0 / mObjScale.y,
         1.0 / mObjScale.z));
      base.mul(mWorldToObj);
      mClippedList.setBaseTransform(base);

      setMaskBits(TransformMask | ScaleMask);
   }
}

void CollisionTrigger::prepRenderImage(SceneRenderState *state)
{
   // only render if selected or render flag is set
   if (!smRenderCollisionTriggers && !isSelected())
      return;

   ObjectRenderInst *ri = state->getRenderPass()->allocInst<ObjectRenderInst>();
   ri->renderDelegate.bind(this, &CollisionTrigger::renderObject);
   ri->type = RenderPassManager::RIT_Editor;
   ri->translucentSort = true;
   ri->defaultKey = 1;
   state->getRenderPass()->addInst(ri);
}

void CollisionTrigger::renderObject(ObjectRenderInst *ri,
   SceneRenderState *state,
   BaseMatInstance *overrideMat)
{
   if (overrideMat)
      return;

   GFXStateBlockDesc desc;
   desc.setZReadWrite(true, false);
   desc.setBlend(true);

   // CollisionTrigger polyhedrons are set up with outward facing normals and CCW ordering
   // so can't enable backface culling.
   desc.setCullMode(GFXCullNone);

   GFXTransformSaver saver;

   MatrixF mat = getRenderTransform();
   mat.scale(getScale());

   GFX->multWorld(mat);

   GFXDrawUtil *drawer = GFX->getDrawUtil();

   drawer->drawPolyhedron(desc, mCollisionTriggerPolyhedron, ColorI(255, 192, 0, 45));

   // Render wireframe.

   desc.setFillModeWireframe();
   drawer->drawPolyhedron(desc, mCollisionTriggerPolyhedron, ColorI::BLACK);
}

void CollisionTrigger::setTriggerPolyhedron(const Polyhedron& rPolyhedron)
{
   mCollisionTriggerPolyhedron = rPolyhedron;

   if (mCollisionTriggerPolyhedron.pointList.size() != 0) {
      mObjBox.minExtents.set(1e10, 1e10, 1e10);
      mObjBox.maxExtents.set(-1e10, -1e10, -1e10);
      for (U32 i = 0; i < mCollisionTriggerPolyhedron.pointList.size(); i++) {
         mObjBox.minExtents.setMin(mCollisionTriggerPolyhedron.pointList[i]);
         mObjBox.maxExtents.setMax(mCollisionTriggerPolyhedron.pointList[i]);
      }
   }
   else {
      mObjBox.minExtents.set(-0.5, -0.5, -0.5);
      mObjBox.maxExtents.set(0.5, 0.5, 0.5);
   }

   MatrixF xform = getTransform();
   setTransform(xform);

   mClippedList.clear();
   mClippedList.mPlaneList = mCollisionTriggerPolyhedron.planeList;
   //   for (U32 i = 0; i < mClippedList.mPlaneList.size(); i++)
   //      mClippedList.mPlaneList[i].neg();

   MatrixF base(true);
   base.scale(Point3F(1.0 / mObjScale.x,
      1.0 / mObjScale.y,
      1.0 / mObjScale.z));
   base.mul(mWorldToObj);

   mClippedList.setBaseTransform(base);

   SAFE_DELETE(mPhysicsRep);

   if (PHYSICSMGR)
   {
      PhysicsCollision *colShape = PHYSICSMGR->createCollision();

      MatrixF colMat(true);
      colMat.displace(Point3F(0, 0, mObjBox.getExtents().z * 0.5f * mObjScale.z));

      colShape->addBox(mObjBox.getExtents() * 0.5f * mObjScale, colMat);
      //MatrixF colMat( true );
      //colMat.scale( mObjScale );
      //colShape->addConvex( mCollisionTriggerPolyhedron.pointList.address(), mCollisionTriggerPolyhedron.pointList.size(), colMat );

      PhysicsWorld *world = PHYSICSMGR->getWorld(isServerObject() ? "server" : "client");
      mPhysicsRep = PHYSICSMGR->createBody();
      mPhysicsRep->init(colShape, 0, PhysicsBody::BF_TRIGGER | PhysicsBody::BF_KINEMATIC, this, world);
      mPhysicsRep->setTransform(getTransform());
   }
}


//--------------------------------------------------------------------------

bool CollisionTrigger::testObject(GameBase* enter)
{
   if (mCollisionTriggerPolyhedron.pointList.size() == 0)
      return false;

   mClippedList.clear();

   SphereF sphere;
   sphere.center = (mWorldBox.minExtents + mWorldBox.maxExtents) * 0.5;
   VectorF bv = mWorldBox.maxExtents - sphere.center;
   sphere.radius = bv.len();

   enter->buildPolyList(PLC_Collision, &mClippedList, mWorldBox, sphere);
   return mClippedList.isEmpty() == false;
}


void CollisionTrigger::potentialEnterObject(GameBase* enter)
{
   for (U32 i = 0; i < mObjects.size(); i++) {
      if (mObjects[i] == enter)
         return;
   }

   if (testObject(enter) == true) {
      mObjects.push_back(enter);
      deleteNotify(enter);

      if (!mEnterCommand.isEmpty())
      {
         String command = String("%obj = ") + enter->getIdString() + ";" + mEnterCommand;
         Con::evaluate(command.c_str());
      }

      //onEnterCollisionTrigger_callback(this, enter);
   }
}


void CollisionTrigger::processTick(const Move* move)
{
   Parent::processTick(move);

   //
   if (mObjects.size() == 0)
      return;

   if (mLastThink + 100 < mCurrTick)
   {
      mCurrTick = 0;
      mLastThink = 0;

      for (S32 i = S32(mObjects.size() - 1); i >= 0; i--)
      {
         if (testObject(mObjects[i]) == false)
         {
            GameBase* remove = mObjects[i];
            mObjects.erase(i);
            clearNotify(remove);

            if (!mLeaveCommand.isEmpty())
            {
               String command = String("%obj = ") + remove->getIdString() + ";" + mLeaveCommand;
               Con::evaluate(command.c_str());
            }

            //onLeaveCollisionTrigger_callback(this, remove);
         }
      }

      if (!mTickCommand.isEmpty())
         Con::evaluate(mTickCommand.c_str());

      //if (mObjects.size() != 0)
      //   onTickCollisionTrigger_callback(this);
   }
   else
   {
      mCurrTick += TickMs;
   }
}

//--------------------------------------------------------------------------

U32 CollisionTrigger::packUpdate(NetConnection* con, U32 mask, BitStream* stream)
{
   U32 i;
   U32 retMask = Parent::packUpdate(con, mask, stream);

   if (stream->writeFlag(mask & TransformMask))
   {
      stream->writeAffineTransform(mObjToWorld);
   }

   // Write the polyhedron
   if (stream->writeFlag(mask & PolyMask))
   {
      stream->write(mCollisionTriggerPolyhedron.pointList.size());
      for (i = 0; i < mCollisionTriggerPolyhedron.pointList.size(); i++)
         mathWrite(*stream, mCollisionTriggerPolyhedron.pointList[i]);

      stream->write(mCollisionTriggerPolyhedron.planeList.size());
      for (i = 0; i < mCollisionTriggerPolyhedron.planeList.size(); i++)
         mathWrite(*stream, mCollisionTriggerPolyhedron.planeList[i]);

      stream->write(mCollisionTriggerPolyhedron.edgeList.size());
      for (i = 0; i < mCollisionTriggerPolyhedron.edgeList.size(); i++) {
         const Polyhedron::Edge& rEdge = mCollisionTriggerPolyhedron.edgeList[i];

         stream->write(rEdge.face[0]);
         stream->write(rEdge.face[1]);
         stream->write(rEdge.vertex[0]);
         stream->write(rEdge.vertex[1]);
      }
   }

   if (stream->writeFlag(mask & EnterCmdMask))
      stream->writeLongString(CMD_SIZE - 1, mEnterCommand.c_str());
   if (stream->writeFlag(mask & LeaveCmdMask))
      stream->writeLongString(CMD_SIZE - 1, mLeaveCommand.c_str());
   if (stream->writeFlag(mask & TickCmdMask))
      stream->writeLongString(CMD_SIZE - 1, mTickCommand.c_str());

   return retMask;
}

void CollisionTrigger::unpackUpdate(NetConnection* con, BitStream* stream)
{
   Parent::unpackUpdate(con, stream);

   U32 i, size;

   // Transform
   if (stream->readFlag())
   {
      MatrixF temp;
      stream->readAffineTransform(&temp);
      setTransform(temp);
   }

   // Read the polyhedron
   if (stream->readFlag())
   {
      Polyhedron tempPH;
      stream->read(&size);
      tempPH.pointList.setSize(size);
      for (i = 0; i < tempPH.pointList.size(); i++)
         mathRead(*stream, &tempPH.pointList[i]);

      stream->read(&size);
      tempPH.planeList.setSize(size);
      for (i = 0; i < tempPH.planeList.size(); i++)
         mathRead(*stream, &tempPH.planeList[i]);

      stream->read(&size);
      tempPH.edgeList.setSize(size);
      for (i = 0; i < tempPH.edgeList.size(); i++) {
         Polyhedron::Edge& rEdge = tempPH.edgeList[i];

         stream->read(&rEdge.face[0]);
         stream->read(&rEdge.face[1]);
         stream->read(&rEdge.vertex[0]);
         stream->read(&rEdge.vertex[1]);
      }
      setTriggerPolyhedron(tempPH);
   }

   if (stream->readFlag())
   {
      char buf[CMD_SIZE];
      stream->readLongString(CMD_SIZE - 1, buf);
      mEnterCommand = buf;
   }
   if (stream->readFlag())
   {
      char buf[CMD_SIZE];
      stream->readLongString(CMD_SIZE - 1, buf);
      mLeaveCommand = buf;
   }
   if (stream->readFlag())
   {
      char buf[CMD_SIZE];
      stream->readLongString(CMD_SIZE - 1, buf);
      mTickCommand = buf;
   }
}

//ConsoleMethod( CollisionTrigger, getNumObjects, S32, 2, 2, "")
DefineEngineMethod(CollisionTrigger, getNumObjects, S32, (), ,
   "@brief Get the number of objects that are within the CollisionTrigger's bounds.\n\n"
   "@see getObject()\n")
{
   return object->getNumCollisionTriggeringObjects();
}

//ConsoleMethod( CollisionTrigger, getObject, S32, 3, 3, "(int idx)")
DefineEngineMethod(CollisionTrigger, getObject, S32, (S32 index), ,
   "@brief Retrieve the requested object that is within the CollisionTrigger's bounds.\n\n"
   "@param index Index of the object to get (range is 0 to getNumObjects()-1)\n"
   "@returns The SimObjectID of the object, or -1 if the requested index is invalid.\n"
   "@see getNumObjects()\n")
{
   if (index >= object->getNumCollisionTriggeringObjects() || index < 0)
      return -1;
   else
      return object->getObject(U32(index))->getId();
}
