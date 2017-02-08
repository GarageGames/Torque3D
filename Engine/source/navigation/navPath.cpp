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

#include "torqueRecast.h"
#include "navPath.h"
#include "duDebugDrawTorque.h"

#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "console/typeValidators.h"
#include "math/mathTypes.h"

#include "scene/sceneRenderState.h"
#include "gfx/gfxDrawUtil.h"
#include "renderInstance/renderPassManager.h"
#include "gfx/primBuilder.h"
#include "core/stream/bitStream.h"
#include "math/mathIO.h"

#include <DetourDebugDraw.h>
#include <climits>

extern bool gEditingMission;

IMPLEMENT_CO_NETOBJECT_V1(NavPath);

NavPath::NavPath() :
   mFrom(0.0f, 0.0f, 0.0f),
   mTo(0.0f, 0.0f, 0.0f)
{
   mTypeMask |= MarkerObjectType;

   mMesh = NULL;
   mWaypoints = NULL;

   mFrom.set(0, 0, 0);
   mFromSet = false;
   mTo.set(0, 0, 0);
   mToSet = false;
   mLength = 0.0f;

   mCurIndex = -1;
   mIsLooping = false;
   mAutoUpdate = false;
   mIsSliced = false;

   mMaxIterations = 1;

   mAlwaysRender = false;
   mXray = false;
   mRenderSearch = false;

   mQuery = NULL;
}

NavPath::~NavPath()
{
   dtFreeNavMeshQuery(mQuery);
   mQuery = NULL;
}

void NavPath::checkAutoUpdate()
{
   EventManager *em = NavMesh::getEventManager();
   em->removeAll(this);
   if(mMesh)
   {
      if(mAutoUpdate)
      {
         em->subscribe(this, "NavMeshRemoved");
         em->subscribe(this, "NavMeshUpdate");
         em->subscribe(this, "NavMeshUpdateBox");
         em->subscribe(this, "NavMeshObstacleAdded");
         em->subscribe(this, "NavMeshObstacleRemoved");
      }
   }
}

bool NavPath::setProtectedMesh(void *obj, const char *index, const char *data)
{
   NavPath *object = static_cast<NavPath*>(obj);

   if(Sim::findObject(data, object->mMesh))
      object->checkAutoUpdate();

   return true;
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

bool NavPath::setProtectedAutoUpdate(void *obj, const char *index, const char *data)
{
   NavPath *object = static_cast<NavPath*>(obj);

   object->mAutoUpdate = dAtob(data);
   object->checkAutoUpdate();

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
      return StringTable->EmptyString();
}

const char *NavPath::getProtectedTo(void *obj, const char *data)
{
   NavPath *object = static_cast<NavPath*>(obj);

   if(object->mToSet)
      return data;
   else
      return StringTable->EmptyString();
}

IRangeValidator ValidIterations(1, S32_MAX);

void NavPath::initPersistFields()
{
   addGroup("NavPath");

   addProtectedField("from", TypePoint3F, Offset(mFrom, NavPath),
      &setProtectedFrom, &getProtectedFrom,
      "World location this path starts at.");
   addProtectedField("to", TypePoint3F, Offset(mTo, NavPath),
      &setProtectedTo, &getProtectedTo,
      "World location this path should end at.");

   addProtectedField("mesh", TypeRealString, Offset(mMeshName, NavPath),
      &setProtectedMesh, &defaultProtectedGetFn,
      "Name of the NavMesh object this path travels within.");
   addProtectedField("waypoints", TYPEID<SimPath::Path>(), Offset(mWaypoints, NavPath),
      &setProtectedWaypoints, &defaultProtectedGetFn,
      "Path containing waypoints for this NavPath to visit.");

   addField("isLooping", TypeBool, Offset(mIsLooping, NavPath),
      "Does this path loop?");
   addField("isSliced", TypeBool, Offset(mIsSliced, NavPath),
      "Plan this path over multiple updates instead of all at once.");
   addFieldV("maxIterations", TypeS32, Offset(mMaxIterations, NavPath), &ValidIterations,
      "Maximum iterations of path planning this path does per tick.");
   addProtectedField("autoUpdate", TypeBool, Offset(mAutoUpdate, NavPath),
      &setProtectedAutoUpdate, &defaultProtectedGetFn,
      "If set, this path will automatically replan when its navigation mesh changes.");

   endGroup("NavPath");

   addGroup("Flags");

   addField("allowWalk", TypeBool, Offset(mLinkTypes.walk, NavPath),
      "Allow the path to use dry land.");
   addField("allowJump", TypeBool, Offset(mLinkTypes.jump, NavPath),
      "Allow the path to use jump links.");
   addField("allowDrop", TypeBool, Offset(mLinkTypes.drop, NavPath),
      "Allow the path to use drop links.");
   addField("allowSwim", TypeBool, Offset(mLinkTypes.swim, NavPath),
      "Allow the path to move in water.");
   addField("allowLedge", TypeBool, Offset(mLinkTypes.ledge, NavPath),
      "Allow the path to jump ledges.");
   addField("allowClimb", TypeBool, Offset(mLinkTypes.climb, NavPath),
      "Allow the path to use climb links.");
   addField("allowTeleport", TypeBool, Offset(mLinkTypes.teleport, NavPath),
      "Allow the path to use teleporters.");

   endGroup("Flags");

   addGroup("NavPath Render");

   addField("alwaysRender", TypeBool, Offset(mAlwaysRender, NavPath),
      "Render this NavPath even when not selected.");
   addField("xray", TypeBool, Offset(mXray, NavPath),
      "Render this NavPath through other objects.");
   addField("renderSearch", TypeBool, Offset(mRenderSearch, NavPath),
      "Render the closed list of this NavPath's search.");

   endGroup("NavPath Render");

   Parent::initPersistFields();
}

bool NavPath::onAdd()
{
   if(!Parent::onAdd())
      return false;

   if(gEditingMission)
      mNetFlags.set(Ghostable);

   resize();

   addToScene();

   if(isServerObject())
   {
      mQuery = dtAllocNavMeshQuery();
      if(!mQuery)
         return false;
      checkAutoUpdate();
      if(!plan())
         setProcessTick(true);
   }

   return true;
}

void NavPath::onRemove()
{
   Parent::onRemove();

   removeFromScene();
}

bool NavPath::init()
{
   mStatus = DT_FAILURE;

   // Check that all the right data is provided.
   if(!mMesh || !mMesh->getNavMesh())
      return false;
   if(!(mFromSet && mToSet) && !(mWaypoints && mWaypoints->size()))
      return false;

   // Initialise our query.
   if(dtStatusFailed(mQuery->init(mMesh->getNavMesh(), MaxPathLen)))
      return false;

   mPoints.clear();
   mFlags.clear();
   mVisitPoints.clear();
   mLength = 0.0f;

   if(isServerObject())
      setMaskBits(PathMask);

   // Add points we need to visit in reverse order.
   if(mWaypoints && mWaypoints->size())
   {
      if(mIsLooping && mFromSet)
         mVisitPoints.push_back(mFrom);
      if(mToSet)
         mVisitPoints.push_front(mTo);
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
      if(mFromSet)
         mVisitPoints.push_back(mFrom);
   }
   else
   {
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
   // Initialise filter.
   mFilter.setIncludeFlags(mLinkTypes.getFlags());

   // Initialise query and visit locations.
   if(!init())
      return false;

   if(mIsSliced)
      return planSliced();
   else
      return planInstant();
}

bool NavPath::planSliced()
{
   bool visited = visitNext();

   if(visited)
      setProcessTick(true);

   return visited;
}

bool NavPath::planInstant()
{
   setProcessTick(false);
   visitNext();
   S32 store = mMaxIterations;
   mMaxIterations = INT_MAX;
   while(update());
   mMaxIterations = store;
   return finalise();
}

bool NavPath::visitNext()
{
   U32 s = mVisitPoints.size();
   if(s < 2)
      return false;

   // Current leg of journey.
   Point3F &start = mVisitPoints[s-1];
   Point3F &end = mVisitPoints[s-2];

   // Drop to height of statics.
   RayInfo info;
   if(getContainer()->castRay(start, start - Point3F(0, 0, mMesh->mWalkableHeight * 2.0f), StaticObjectType, &info))
      start = info.point;
   if(getContainer()->castRay(end + Point3F(0, 0, 0.1f), end - Point3F(0, 0, mMesh->mWalkableHeight * 2.0f), StaticObjectType, &info))
      end = info.point;

   // Convert to Detour-friendly coordinates and data structures.
   F32 from[] = {start.x, start.z, -start.y};
   F32 to[] =   {end.x,   end.z,   -end.y};
   F32 extx = mMesh->mWalkableRadius * 4.0f;
   F32 extz = mMesh->mWalkableHeight;
   F32 extents[] = {extx, extz, extx};
   dtPolyRef startRef, endRef;

   if(dtStatusFailed(mQuery->findNearestPoly(from, extents, &mFilter, &startRef, NULL)) || !startRef)
   {
      Con::errorf("No NavMesh polygon near visit point (%g, %g, %g) of NavPath %s",
         start.x, start.y, start.z, getIdString());
      return false;
   }

   if(dtStatusFailed(mQuery->findNearestPoly(to, extents, &mFilter, &endRef, NULL)) || !endRef)
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
   if(dtStatusInProgress(mStatus))
      mStatus = mQuery->updateSlicedFindPath(mMaxIterations, NULL);
   if(dtStatusSucceed(mStatus))
   {
      // Add points from this leg.
      dtPolyRef path[MaxPathLen];
      S32 pathLen;
      mStatus = mQuery->finalizeSlicedFindPath(path, &pathLen, MaxPathLen);
      if(dtStatusSucceed(mStatus) && pathLen)
      {
         F32 straightPath[MaxPathLen * 3];
         S32 straightPathLen;
         dtPolyRef straightPathPolys[MaxPathLen];
         U8 straightPathFlags[MaxPathLen];

         U32 s = mVisitPoints.size();
         Point3F start = mVisitPoints[s-1];
         Point3F end = mVisitPoints[s-2];
         F32 from[] = {start.x, start.z, -start.y};
         F32 to[] =   {end.x,   end.z,   -end.y};

         mQuery->findStraightPath(from, to, path, pathLen,
            straightPath, straightPathFlags,
            straightPathPolys, &straightPathLen, MaxPathLen);

         s = mPoints.size();
         mPoints.increment(straightPathLen);
         mFlags.increment(straightPathLen);
         for(U32 i = 0; i < straightPathLen; i++)
         {
            F32 *f = straightPath + i * 3;
            mPoints[s + i] = RCtoDTS(f);
            mMesh->getNavMesh()->getPolyFlags(straightPathPolys[i], &mFlags[s + i]);
            // Add to length
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
   setProcessTick(false);

   resize();

   return success();
}

void NavPath::processTick(const Move *move)
{
   if(!mMesh)
      if(Sim::findObject(mMeshName.c_str(), mMesh))
         plan();
   if(dtStatusInProgress(mStatus))
      update();
}

Point3F NavPath::getNode(S32 idx) const
{
   if(idx < size() && idx >= 0)
      return mPoints[idx];
   return Point3F(0,0,0);
}

U16 NavPath::getFlags(S32 idx) const
{
   if(idx < size() && idx >= 0)
      return mFlags[idx];
   return 0;
}

S32 NavPath::size() const
{
   return mPoints.size();
}

void NavPath::onEditorEnable()
{
   mNetFlags.set(Ghostable);
}

void NavPath::onEditorDisable()
{
   mNetFlags.clear(Ghostable);
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
   ri->type = RenderPassManager::RIT_Editor;      
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

   if(mRenderSearch && getServerObject())
   {
      NavPath *np = static_cast<NavPath*>(getServerObject());
      if(np->mQuery && !dtStatusSucceed(np->mStatus))
      {
         duDebugDrawTorque dd;
         dd.overrideColor(duRGBA(250, 20, 20, 255));
         duDebugDrawNavMeshNodes(&dd, *np->mQuery);
         dd.render();
      }
   }
}

U32 NavPath::packUpdate(NetConnection *conn, U32 mask, BitStream *stream)
{
   U32 retMask = Parent::packUpdate(conn, mask, stream);

   stream->writeFlag(mIsLooping);
   stream->writeFlag(mAlwaysRender);
   stream->writeFlag(mXray);
   stream->writeFlag(mRenderSearch);

   if(stream->writeFlag(mFromSet))
      mathWrite(*stream, mFrom);
   if(stream->writeFlag(mToSet))
      mathWrite(*stream, mTo);

   if(stream->writeFlag(mask & PathMask))
   {
      stream->writeInt(mPoints.size(), 32);
      for(U32 i = 0; i < mPoints.size(); i++)
      {
         mathWrite(*stream, mPoints[i]);
         stream->writeInt(mFlags[i], 16);
      }
   }

   return retMask;
}

void NavPath::unpackUpdate(NetConnection *conn, BitStream *stream)
{
   Parent::unpackUpdate(conn, stream);

   mIsLooping = stream->readFlag();
   mAlwaysRender = stream->readFlag();
   mXray = stream->readFlag();
   mRenderSearch = stream->readFlag();

   if((mFromSet = stream->readFlag()) == true)
      mathRead(*stream, &mFrom);
   if((mToSet = stream->readFlag()) == true)
      mathRead(*stream, &mTo);

   if(stream->readFlag())
   {
      mPoints.clear();
      mFlags.clear();
      mPoints.setSize(stream->readInt(32));
      mFlags.setSize(mPoints.size());
      for(U32 i = 0; i < mPoints.size(); i++)
      {
         Point3F p;
         mathRead(*stream, &p);
         mPoints[i] = p;
         mFlags[i] = stream->readInt(16);
      }
      resize();
   }
}

DefineEngineMethod(NavPath, plan, bool, (),,
   "@brief Find a path using the already-specified path properties.")
{
   return object->plan();
}

DefineEngineMethod(NavPath, onNavMeshUpdate, void, (const char *data),,
   "@brief Callback when this path's NavMesh is loaded or rebuilt.")
{
   if(object->mMesh && !dStrcmp(data, object->mMesh->getIdString()))
      object->plan();
}

DefineEngineMethod(NavPath, onNavMeshUpdateBox, void, (const char *data),,
   "@brief Callback when a particular area in this path's NavMesh is rebuilt.")
{
   String s(data);
   U32 space = s.find(' ');
   if(space != String::NPos)
   {
      String id = s.substr(0, space);
      if(!object->mMesh || id.compare(object->mMesh->getIdString()))
         return;
      String boxstr = s.substr(space + 1);
      Box3F box;
      castConsoleTypeFromString(box, boxstr.c_str());
      if(object->getWorldBox().isOverlapped(box))
         object->plan();
   }
}

DefineEngineMethod(NavPath, size, S32, (),,
   "@brief Return the number of nodes in this path.")
{
   return object->size();
}

DefineEngineMethod(NavPath, getNode, Point3F, (S32 idx),,
   "@brief Get a specified node along the path.")
{
   return object->getNode(idx);
}

DefineEngineMethod(NavPath, getFlags, S32, (S32 idx),,
   "@brief Get a specified node along the path.")
{
   return (S32)object->getFlags(idx);
}

DefineEngineMethod(NavPath, getLength, F32, (),,
   "@brief Get the length of this path.")
{
   return object->getLength();
}
