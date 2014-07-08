//-----------------------------------------------------------------------------
// Copyright (c) 2013 GarageGames, LLC
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

#include "torqueRecast.h"
#include "navPath.h"

#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "console/typeValidators.h"

#include "scene/sceneRenderState.h"
#include "gfx/gfxDrawUtil.h"
#include "renderInstance/renderPassManager.h"
#include "gfx/primBuilder.h"
#include "core/stream/bitStream.h"
#include "math/mathIO.h"

#include <DetourDebugDraw.h>

extern bool gEditingMission;

IMPLEMENT_CO_NETOBJECT_V1(NavPath);

NavPath::NavPath() :
   mFrom(0.0f, 0.0f, 0.0f),
   mTo(0.0f, 0.0f, 0.0f)
{
   mTypeMask |= StaticShapeObjectType | MarkerObjectType;
   mNetFlags.clear(Ghostable);

   mMesh = NULL;
   mWaypoints = NULL;

   mFrom.set(0, 0, 0);
   mFromSet = false;
   mTo.set(0, 0, 0);
   mToSet = false;
   mLength = 0.0f;

   mIsLooping = false;

   mAlwaysRender = false;
   mXray = false;

   mQuery = dtAllocNavMeshQuery();
}

NavPath::~NavPath()
{
   // Required for Detour.
   dtFreeNavMeshQuery(mQuery);
   mQuery = NULL;
}

bool NavPath::setProtectedMesh(void *obj, const char *index, const char *data)
{
   NavMesh *mesh = NULL;
   NavPath *object = static_cast<NavPath*>(obj);

   if(Sim::findObject(data, mesh))
      object->mMesh = mesh;

   return false;
}

const char *NavPath::getProtectedMesh(void *obj, const char *data)
{
   NavPath *object = static_cast<NavPath*>(obj);

   if(object->mMesh.isNull())
      return "";

   if(object->mMesh->getName())
      return object->mMesh->getName();
   else
      return object->mMesh->getIdString();
}

bool NavPath::setProtectedWaypoints(void *obj, const char *index, const char *data)
{
   SimPath::Path *points = NULL;
   NavPath *object = static_cast<NavPath*>(obj);

   if(Sim::findObject(data, points))
   {
      object->mWaypoints = points;
      object->mIsLooping = points->isLooping();
   }
   else
      object->mWaypoints = NULL;

   return false;
}

bool NavPath::setProtectedFrom(void *obj, const char *index, const char *data)
{
   NavPath *object = static_cast<NavPath*>(obj);

   if(dStrcmp(data, ""))
   {
      object->mFromSet = true;
      return true;
   }
   else
   {
      object->mFromSet = false;
      return false;
   }
}

bool NavPath::setProtectedTo(void *obj, const char *index, const char *data)
{
   NavPath *object = static_cast<NavPath*>(obj);

   if(dStrcmp(data, ""))
   {
      object->mToSet = true;
      return true;
   }
   else
   {
      object->mToSet = false;
      return false;
   }
}

const char *NavPath::getProtectedFrom(void *obj, const char *data)
{
   NavPath *object = static_cast<NavPath*>(obj);

   if(object->mFromSet)
      return data;
   else
      return "";
}

const char *NavPath::getProtectedTo(void *obj, const char *data)
{
   NavPath *object = static_cast<NavPath*>(obj);

   if(object->mToSet)
      return data;
   else
      return "";
}

bool NavPath::setProtectedAlwaysRender(void *obj, const char *index, const char *data)
{
   NavPath *path = static_cast<NavPath*>(obj);
   bool always = dAtob(data);
   if(always)
   {
      if(!gEditingMission)
         path->mNetFlags.set(Ghostable);
   }
   else
   {
      if(!gEditingMission)
         path->mNetFlags.clear(Ghostable);
   }
   path->mAlwaysRender = always;
   path->setMaskBits(PathMask);
   return true;
}

static IRangeValidator NaturalNumber(1, S32_MAX);

void NavPath::initPersistFields()
{
   addGroup("NavPath");

   addProtectedField("from", TypePoint3F, Offset(mFrom, NavPath),
      &setProtectedFrom, &getProtectedFrom,
      "World location this path starts at.");
   addProtectedField("to", TypePoint3F, Offset(mTo, NavPath),
      &setProtectedTo, &getProtectedTo,
      "World location this path should end at.");

   addProtectedField("mesh", TYPEID<NavMesh>(), Offset(mMesh, NavPath),
      &setProtectedMesh, &getProtectedMesh,
      "NavMesh object this path travels within.");
   addProtectedField("waypoints", TYPEID<SimPath::Path>(), Offset(mWaypoints, NavPath),
      &setProtectedWaypoints, &defaultProtectedGetFn,
      "Path containing waypoints for this NavPath to visit.");

   addField("isLooping", TypeBool, Offset(mIsLooping, NavPath),
      "Does this path loop?");

   endGroup("NavPath");

   addGroup("NavPath Render");
   
   addProtectedField("alwaysRender", TypeBool, Offset(mAlwaysRender, NavMesh),
      &setProtectedAlwaysRender, &defaultProtectedGetFn,
      "Display this NavPath even outside the editor.");
   addField("xray", TypeBool, Offset(mXray, NavPath),
      "Render this NavPath through other objects.");

   endGroup("NavPath Render");

   Parent::initPersistFields();
}

bool NavPath::onAdd()
{
   if(!Parent::onAdd())
      return false;

   addToScene();

   // Ghost immediately if the editor's already open.
   if(gEditingMission || mAlwaysRender)
      mNetFlags.set(Ghostable);

   // Automatically find a path if we can.
   if(isServerObject())
      plan();

   // Set initial world bounds and stuff.
   resize();

   return true;
}

void NavPath::onRemove()
{
   // Remove from simulation.
   removeFromScene();

   Parent::onRemove();
}

bool NavPath::init()
{
   // Check that enough data is provided.
   if(mMesh.isNull() || !mMesh->getNavMesh())
      return false;
   if(!(mFromSet && mToSet) && !(!mWaypoints.isNull() && mWaypoints->size()))
      return false;

   // Initialise query in Detour.
   if(dtStatusFailed(mQuery->init(mMesh->getNavMesh(), MaxPathLen)))
      return false;

   mPoints.clear();
   mVisitPoints.clear();
   mLength = 0.0f;

   // Send path data to clients who are ghosting this object.
   if(isServerObject())
      setMaskBits(PathMask);

   // Add points we need to visit in reverse order.
   if(mWaypoints && mWaypoints->size())
   {
      // Add destination. For looping paths, that includes 'from'.
      if(mIsLooping && mFromSet)
         mVisitPoints.push_back(mFrom);
      if(mToSet)
         mVisitPoints.push_front(mTo);
      // Add waypoints.
      for(S32 i = mWaypoints->size() - 1; i >= 0; i--)
      {
         SceneObject *s = dynamic_cast<SceneObject*>(mWaypoints->at(i));
         if(s)
         {
            mVisitPoints.push_back(s->getPosition());
            // This is potentially slow, but safe.
            if(!i && mIsLooping && !mFromSet)
               mVisitPoints.push_front(s->getPosition());
         }
      }
      // Add source (only ever specified by 'from').
      if(mFromSet)
         mVisitPoints.push_back(mFrom);
   }
   else
   {
      // Add (from,) to and from
      if(mIsLooping)
         mVisitPoints.push_back(mFrom);
      mVisitPoints.push_back(mTo);
      mVisitPoints.push_back(mFrom);
   }

   return true;
}

void NavPath::resize()
{
   if(!mPoints.size())
   {
      mObjBox.set(Point3F(-0.5f, -0.5f, -0.5f),
                  Point3F( 0.5f,  0.5f,  0.5f));
      resetWorldBox();
      setTransform(MatrixF(true));
      return;
   }

   // Grow a box to just fit over all our points.
   Point3F max(mPoints[0]), min(mPoints[0]), pos(0.0f);
   for(U32 i = 1; i < mPoints.size(); i++)
   {
      Point3F p = mPoints[i];
      max.x = getMax(max.x, p.x);
      max.y = getMax(max.y, p.y);
      max.z = getMax(max.z, p.z);
      min.x = getMin(min.x, p.x);
      min.y = getMin(min.y, p.y);
      min.z = getMin(min.z, p.z);
      pos += p;
   }
   pos /= mPoints.size();
   min -= Point3F(0.5f, 0.5f, 0.5f);
   max += Point3F(0.5f, 0.5f, 0.5f);

   mObjBox.set(min - pos, max - pos);
   MatrixF mat = Parent::getTransform();
   mat.setPosition(pos);
   Parent::setTransform(mat);
}

bool NavPath::plan()
{
   if(!init())
      return false;

   if(!visitNext())
      return false;

   while(update());

   if(!finalise())
      return false;

   resize();

   return true;
}

bool NavPath::visitNext()
{
   U32 s = mVisitPoints.size();
   if(s < 2)
      return false;

   // Current leg of journey.
   Point3F start = mVisitPoints[s-1];
   Point3F end = mVisitPoints[s-2];

   // Convert to Detour-friendly coordinates and data structures.
   F32 from[] = {start.x, start.z, -start.y};
   F32 to[] =   {end.x,   end.z,   -end.y};
   F32 extents[] = {1.0f, 1.0f, 1.0f};
   dtPolyRef startRef, endRef;

   if(dtStatusFailed(mQuery->findNearestPoly(from, extents, &mFilter, &startRef, from)) || !startRef)
   {
      Con::errorf("No NavMesh polygon near visit point (%g, %g, %g) of NavPath %s",
         start.x, start.y, start.z, getIdString());
      return false;
   }

   if(dtStatusFailed(mQuery->findNearestPoly(to, extents, &mFilter, &endRef, to)) || !startRef)
   {
      Con::errorf("No NavMesh polygon near visit point (%g, %g, %g) of NavPath %s",
         end.x, end.y, end.z, getIdString());
      return false;
   }

   // Init sliced pathfind.
   mStatus = mQuery->initSlicedFindPath(startRef, endRef, from, to, &mFilter);
   if(dtStatusFailed(mStatus))
      return false;

   return true;
}

bool NavPath::update()
{
   // StatusInProgress means a query is underway.
   if(dtStatusInProgress(mStatus))
      mStatus = mQuery->updateSlicedFindPath(INT_MAX, NULL);
   // StatusSucceeded means the query found its destination.
   if(dtStatusSucceed(mStatus))
   {
      // Finalize the path. Need to use the static path length cap again.
      dtPolyRef path[MaxPathLen];
      S32 pathLen;
      mStatus = mQuery->finalizeSlicedFindPath(path, &pathLen, MaxPathLen);
      // Apparently stuff can go wrong during finalizing, so check the status again.
      if(dtStatusSucceed(mStatus) && pathLen)
      {
         // These next few blocks are straight from Detour example code.
         F32 straightPath[MaxPathLen * 3];
         S32 straightPathLen;
         dtPolyRef straightPathPolys[MaxPathLen];
         U8 straightPathFlags[MaxPathLen];

         U32 s = mVisitPoints.size();
         Point3F start = mVisitPoints[s-1];
         Point3F end = mVisitPoints[s-2];
         F32 from[] = {start.x, start.z, -start.y};
         F32 to[] =   {end.x,   end.z,   -end.y};

         // Straightens out the path.
         mQuery->findStraightPath(from, to, path, pathLen,
            straightPath, straightPathFlags,
            straightPathPolys, &straightPathLen, MaxPathLen);

         // Convert Detour point path to list of Torque points.
         s = mPoints.size();
         mPoints.increment(straightPathLen);
         for(U32 i = 0; i < straightPathLen; i++)
         {
            F32 *f = straightPath + i * 3;
            mPoints[s + i] = RCtoDTS(f);
            // Accumulate length if we're not the first vertex.
            if(s > 0 || i > 0)
               mLength += (mPoints[s+i] - mPoints[s+i-1]).len();
         }

         if(isServerObject())
            setMaskBits(PathMask);
      }
      else
         return false;
      // Check to see where we still need to visit.
      if(mVisitPoints.size() > 1)
      {
         //Next leg of the journey.
         mVisitPoints.pop_back();
         return visitNext();
      }
      else
      {
         // Finished!
         return false;
      }
   }
   else if(dtStatusFailed(mStatus))
   {
      // Something went wrong in planning.
      return false;
   }
   return true;
}

bool NavPath::finalise()
{
   // Stop ticking.
   setProcessTick(false);

   // Reset world bounds and stuff.
   resize();

   return dtStatusSucceed(mStatus);
}

void NavPath::processTick(const Move *move)
{
   if(dtStatusInProgress(mStatus))
      update();
}

Point3F NavPath::getNode(S32 idx)
{
   if(idx < getCount() && idx >= 0)
      return mPoints[idx];
   Con::errorf("Trying to access out-of-bounds path index %d (path length: %d)!", idx, getCount());
   return Point3F(0,0,0);
}

S32 NavPath::getCount()
{
   return mPoints.size();
}

void NavPath::onEditorEnable()
{
   mNetFlags.set(Ghostable);
   if(isClientObject() && !mAlwaysRender)
      addToScene();
}

void NavPath::onEditorDisable()
{
   if(!mAlwaysRender)
   {
      mNetFlags.clear(Ghostable);
      if(isClientObject())
         removeFromScene();
   }
}

void NavPath::inspectPostApply()
{
   plan();
}

void NavPath::onDeleteNotify(SimObject *obj)
{
   if(obj == (SimObject*)mMesh)
   {
      mMesh = NULL;
      plan();
   }
}

void NavPath::prepRenderImage(SceneRenderState *state)
{
   ObjectRenderInst *ri = state->getRenderPass()->allocInst<ObjectRenderInst>();
   ri->renderDelegate.bind(this, &NavPath::renderSimple);
   ri->type = RenderPassManager::RIT_Object;
   ri->translucentSort = true;
   ri->defaultKey = 1;
   state->getRenderPass()->addInst(ri);
}

void NavPath::renderSimple(ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance *overrideMat)
{
   if(overrideMat)
      return;

   if(state->isReflectPass() || !(isSelected() || mAlwaysRender))
      return;

   GFXDrawUtil *drawer = GFX->getDrawUtil();
   GFXStateBlockDesc desc;
   desc.setZReadWrite(true, false);
   desc.setBlend(true);
   desc.setCullMode(GFXCullNone);

   if(isSelected())
   {
      drawer->drawCube(desc, getWorldBox(), ColorI(136, 255, 228, 5));
      desc.setFillModeWireframe();
      drawer->drawCube(desc, getWorldBox(), ColorI::BLACK);
   }

   desc.setZReadWrite(!mXray, false);

   ColorI pathColour(255, 0, 255);

   if(!mIsLooping)
   {
      desc.setFillModeSolid();
      if(mFromSet) drawer->drawCube(desc, Point3F(0.2f, 0.2f, 0.2f), mFrom, pathColour);
      if(mToSet)   drawer->drawCube(desc, Point3F(0.2f, 0.2f, 0.2f), mTo, pathColour);
   }

   GFXStateBlockRef sb = GFX->createStateBlock(desc);
   GFX->setStateBlock(sb);

   PrimBuild::color3i(pathColour.red, pathColour.green, pathColour.blue);

   PrimBuild::begin(GFXLineStrip, mPoints.size());
   for (U32 i = 0; i < mPoints.size(); i++)
      PrimBuild::vertex3fv(mPoints[i]);
   PrimBuild::end();
}

U32 NavPath::packUpdate(NetConnection *conn, U32 mask, BitStream *stream)
{
   U32 retMask = Parent::packUpdate(conn, mask, stream);

   stream->writeFlag(mIsLooping);
   stream->writeFlag(mAlwaysRender);
   stream->writeFlag(mXray);

   if(stream->writeFlag(mFromSet))
      mathWrite(*stream, mFrom);
   if(stream->writeFlag(mToSet))
      mathWrite(*stream, mTo);

   if(stream->writeFlag(mask & PathMask))
   {
      stream->writeInt(mPoints.size(), 32);
      for(U32 i = 0; i < mPoints.size(); i++)
         mathWrite(*stream, mPoints[i]);
   }

   return retMask;
}

void NavPath::unpackUpdate(NetConnection *conn, BitStream *stream)
{
   Parent::unpackUpdate(conn, stream);

   mIsLooping = stream->readFlag();
   mAlwaysRender = stream->readFlag();
   mXray = stream->readFlag();

   if((mFromSet = stream->readFlag()) == true)
      mathRead(*stream, &mFrom);
   if((mToSet = stream->readFlag()) == true)
      mathRead(*stream, &mTo);

   if(stream->readFlag())
   {
      mPoints.clear();
      mPoints.setSize(stream->readInt(32));
      for(U32 i = 0; i < mPoints.size(); i++)
      {
         Point3F p;
         mathRead(*stream, &p);
         mPoints[i] = p;
      }
      resize();
   }
}

DefineEngineMethod(NavPath, replan, bool, (),,
   "@brief Find a path using the already-specified path properties.")
{
   return object->plan();
}

DefineEngineMethod(NavPath, getCount, S32, (),,
   "@brief Return the number of nodes in this path.")
{
   return object->getCount();
}

DefineEngineMethod(NavPath, getNode, Point3F, (S32 idx),,
   "@brief Get a specified node along the path.")
{
   return object->getNode(idx);
}

DefineEngineMethod(NavPath, getLength, F32, (),,
   "@brief Get the length of this path in Torque units (i.e. the total distance it covers).")
{
   return object->getLength();
}
