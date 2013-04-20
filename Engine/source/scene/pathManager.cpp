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

#include "gfx/gfxDevice.h"
#include "scene/pathManager.h"
#include "sim/netConnection.h"
#include "core/stream/bitStream.h"
#include "scene/simPath.h"
#include "math/mathIO.h"
#include "scene/sceneRenderState.h"
#include "scene/sceneManager.h"
#include "platform/profiler.h"
#include "core/module.h"

extern bool gEditingMission;


namespace {

U32 countNumBits(U32 n)
{
   U32 count = 0;
   while (n != 0) {
      n >>= 1;
      count++;
   }

   return count ? count : 1;
}

} // namespace {}


MODULE_BEGIN( PathManager )

   MODULE_INIT
   {
      AssertFatal(gClientPathManager == NULL && gServerPathManager == NULL, "Error, already initialized the path manager!");

      gClientPathManager = new PathManager(false);
      gServerPathManager = new PathManager(true);
   }

   MODULE_SHUTDOWN
   {
      AssertFatal(gClientPathManager != NULL && gServerPathManager != NULL, "Error, path manager not initialized!");

      delete gClientPathManager;
      gClientPathManager = NULL;
      delete gServerPathManager;
      gServerPathManager = NULL;
   }

MODULE_END;


//--------------------------------------------------------------------------
//-------------------------------------- PathManagerEvent
//
class PathManagerEvent : public NetEvent
{
  public:
   U32  modifiedPath;
   bool clearPaths;
   PathManager::PathEntry path;

  public:
   typedef NetEvent Parent;
   PathManagerEvent() { }

   void pack(NetConnection*, BitStream*);
   void write(NetConnection*, BitStream*);
   void unpack(NetConnection*, BitStream*);
   void process(NetConnection*);

   DECLARE_CONOBJECT(PathManagerEvent);
};

void PathManagerEvent::pack(NetConnection*, BitStream* stream)
{
   // Write out the modified path...
   stream->write(modifiedPath);
   stream->writeFlag(clearPaths);
   stream->write(path.totalTime);
   stream->write(path.positions.size());


   // This is here for safety. You can remove it if you want to try your luck at bigger sizes. -- BJG
   AssertWarn(path.positions.size() < 1500/40, "Warning! Path size is pretty big - may cause packet overrun!");

   // Each one of these is about 8 floats and 2 ints
   // so we'll say it's about 40 bytes in size, which is where the 40 in the above calc comes from.
   for (U32 j = 0; j < path.positions.size(); j++)
   {
      mathWrite(*stream, path.positions[j]);
      mathWrite(*stream, path.rotations[j]);
      stream->write(path.msToNext[j]);
      stream->write(path.smoothingType[j]);
   }
}

void PathManagerEvent::write(NetConnection*nc, BitStream *stream)
{
   pack(nc, stream);
}

void PathManagerEvent::unpack(NetConnection*, BitStream* stream)
{
   // Read in the modified path...

   stream->read(&modifiedPath);
   clearPaths = stream->readFlag();
   stream->read(&path.totalTime);

   U32 numPoints;
   stream->read(&numPoints);
   path.positions.setSize(numPoints);
   path.rotations.setSize(numPoints);
   path.msToNext.setSize(numPoints);
   path.smoothingType.setSize(numPoints);
   for (U32 j = 0; j < path.positions.size(); j++)
   {
      mathRead(*stream, &path.positions[j]);
      mathRead(*stream, &path.rotations[j]);
      stream->read(&path.msToNext[j]);
      stream->read(&path.smoothingType[j]);
   }
}

void PathManagerEvent::process(NetConnection*)
{
   if (clearPaths)
   {
      // Clear out all the client's paths...
      gClientPathManager->clearPaths();
   }
   AssertFatal(modifiedPath <= gClientPathManager->mPaths.size(), "Error out of bounds path!");
   if (modifiedPath == gClientPathManager->mPaths.size()) {
      PathManager::PathEntry *pe = new PathManager::PathEntry;
      *pe = path;
      gClientPathManager->mPaths.push_back(pe);
   }
   else
      *(gClientPathManager->mPaths[modifiedPath]) = path;
}

IMPLEMENT_CO_NETEVENT_V1(PathManagerEvent);

// Will be internalized once the @internal tag is working
ConsoleDocClass( PathManagerEvent,
   "@brief Class responsible for the registration, transmission, and management "
   "of paths on client and server.\n\n"

   "For internal use only, not intended for use in TorqueScript or game development\n\n"

   "@internal\n"
);

//--------------------------------------------------------------------------
//-------------------------------------- PathManager Implementation
//
PathManager* gClientPathManager = NULL;
PathManager* gServerPathManager = NULL;

//--------------------------------------------------------------------------
PathManager::PathManager(const bool isServer)
{
   VECTOR_SET_ASSOCIATION(mPaths);

   mIsServer  = isServer;
}

PathManager::~PathManager()
{
   clearPaths();
}

void PathManager::clearPaths()
{
   for (U32 i = 0; i < mPaths.size(); i++)
      delete mPaths[i];
   mPaths.setSize(0);
#ifdef TORQUE_DEBUG
   // This gets rid of the memory used by the vector.
   // Prevents it from showing up in memory leak logs.
   mPaths.compact();
#endif
}

ConsoleFunction(clearServerPaths, void, 1, 1, "")
{
   gServerPathManager->clearPaths();
}

ConsoleFunction(clearClientPaths, void, 1, 1, "")
{
   gClientPathManager->clearPaths();
}

//--------------------------------------------------------------------------
U32 PathManager::allocatePathId()
{
   mPaths.increment();
   mPaths.last() = new PathEntry;

   return (mPaths.size() - 1);
}


void PathManager::updatePath(const U32              id,
                             const Vector<Point3F>& positions,
                             const Vector<QuatF>&   rotations,
                             const Vector<U32>&     times,
                             const Vector<U32>&     smoothingTypes)
{
   AssertFatal(mIsServer == true, "PathManager::updatePath: Error, must be called on the server side");
   AssertFatal(id < mPaths.size(), "PathManager::updatePath: error, id out of range");
   AssertFatal(positions.size() == times.size() && positions.size() == smoothingTypes.size(), "Error, times and positions must match!");

   PathEntry& rEntry = *mPaths[id];

   rEntry.positions = positions;
   rEntry.rotations = rotations;
   rEntry.msToNext  = times;
   rEntry.smoothingType = smoothingTypes;

   rEntry.totalTime = 0;
   for (S32 i = 0; i < S32(rEntry.msToNext.size()); i++)
      rEntry.totalTime += rEntry.msToNext[i];

   transmitPath(id);
}


//--------------------------------------------------------------------------
void PathManager::transmitPaths(NetConnection* nc)
{
   AssertFatal(mIsServer, "Error, cannot call transmitPaths on client path manager!");

   // Send over paths
   for(S32 i = 0; i < mPaths.size(); i++)
   {
      PathManagerEvent* event = new PathManagerEvent;
      event->clearPaths       = (i == 0);
      event->modifiedPath = i;
      event->path = *(mPaths[i]);
      nc->postNetEvent(event);
   }
}

void PathManager::transmitPath(const U32 id)
{
   AssertFatal(mIsServer, "Error, cannot call transmitNewPath on client path manager!");

   // Post to all active clients that have already received their paths...
   //
   SimGroup* pClientGroup = Sim::getClientGroup();
   for (SimGroup::iterator itr = pClientGroup->begin(); itr != pClientGroup->end(); itr++) {
      NetConnection* nc = dynamic_cast<NetConnection*>(*itr);
      if (nc && nc->missionPathsSent())
      {
         // Transmit the updated path...
         PathManagerEvent* event = new PathManagerEvent;
         event->modifiedPath     = id;
         event->clearPaths       = false;
         event->path             = *(mPaths[id]);
         nc->postNetEvent(event);
      }
   }
}

void PathManager::getPathPosition(const U32 id,
                                  const F64 msPosition,
                                  Point3F&  rPosition,
                                  QuatF &rotation)
{
   AssertFatal(isValidPath(id), "Error, this is not a valid path!");
   PROFILE_START(PathManGetPos);

   // Ok, query holds our path information...
   F64 ms = msPosition;
   if (ms > mPaths[id]->totalTime)
      ms = mPaths[id]->totalTime;

   S32 startNode = 0;
   while (ms > mPaths[id]->msToNext[startNode]) {
      ms -= mPaths[id]->msToNext[startNode];
      startNode++;
   }
   S32 endNode = (startNode + 1) % mPaths[id]->positions.size();

   Point3F& rStart = mPaths[id]->positions[startNode];
   Point3F& rEnd   = mPaths[id]->positions[endNode];

   F64 interp = ms / F32(mPaths[id]->msToNext[startNode]);
   if(mPaths[id]->smoothingType[startNode] == Marker::SmoothingTypeLinear)
   {
      rPosition = (rStart * (1.0 - interp)) + (rEnd * interp);
   }
   else if(mPaths[id]->smoothingType[startNode] == Marker::SmoothingTypeAccelerate)
   {
      interp = mSin(interp * M_PI - (M_PI / 2)) * 0.5 + 0.5;
      rPosition = (rStart * (1.0 - interp)) + (rEnd * interp);
   }
   else if(mPaths[id]->smoothingType[startNode] == Marker::SmoothingTypeSpline)
   {
      S32 preStart = startNode - 1;
      S32 postEnd = endNode + 1;
      if(postEnd >= mPaths[id]->positions.size())
         postEnd = 0;
      if(preStart < 0)
         preStart = mPaths[id]->positions.size() - 1;
      Point3F p0 = mPaths[id]->positions[preStart];
      Point3F p1 = rStart;
      Point3F p2 = rEnd;
      Point3F p3 = mPaths[id]->positions[postEnd];
      rPosition.x = mCatmullrom(interp, p0.x, p1.x, p2.x, p3.x);
      rPosition.y = mCatmullrom(interp, p0.y, p1.y, p2.y, p3.y);
      rPosition.z = mCatmullrom(interp, p0.z, p1.z, p2.z, p3.z);
   }
   rotation.interpolate( mPaths[id]->rotations[startNode], mPaths[id]->rotations[endNode], interp );
   PROFILE_END();
}

U32 PathManager::getPathTotalTime(const U32 id) const
{
   AssertFatal(isValidPath(id), "Error, this is not a valid path!");

   return mPaths[id]->totalTime;
}

U32 PathManager::getPathNumWaypoints(const U32 id) const
{
   AssertFatal(isValidPath(id), "Error, this is not a valid path!");

   return mPaths[id]->positions.size();
}

U32 PathManager::getWaypointTime(const U32 id, const U32 wayPoint) const
{
   AssertFatal(isValidPath(id), "Error, this is not a valid path!");
   AssertFatal(wayPoint < getPathNumWaypoints(id), "Invalid waypoint!");

   U32 time = 0;
   for (U32 i = 0; i < wayPoint; i++)
      time += mPaths[id]->msToNext[i];

   return time;
}

U32 PathManager::getPathTimeBits(const U32 id)
{
   AssertFatal(isValidPath(id), "Error, this is not a valid path!");

   return countNumBits(mPaths[id]->totalTime);
}

U32 PathManager::getPathWaypointBits(const U32 id)
{
   AssertFatal(isValidPath(id), "Error, this is not a valid path!");

   return countNumBits(mPaths[id]->positions.size());
}


bool PathManager::dumpState(BitStream* stream) const
{
   stream->write(mPaths.size());

   for (U32 i = 0; i < mPaths.size(); i++) {
      const PathEntry& rEntry = *mPaths[i];
      stream->write(rEntry.totalTime);

      stream->write(rEntry.positions.size());
      for (U32 j = 0; j < rEntry.positions.size(); j++) {
         mathWrite(*stream, rEntry.positions[j]);
         stream->write(rEntry.msToNext[j]);
      }
   }

   return stream->getStatus() == Stream::Ok;
}

bool PathManager::readState(BitStream* stream)
{
   U32 i;
   for (i = 0; i < mPaths.size(); i++)
      delete mPaths[i];

   U32 numPaths;
   stream->read(&numPaths);
   mPaths.setSize(numPaths);

   for (i = 0; i < mPaths.size(); i++) {
      mPaths[i] = new PathEntry;
      PathEntry& rEntry = *mPaths[i];

      stream->read(&rEntry.totalTime);

      U32 numPositions;
      stream->read(&numPositions);
      rEntry.positions.setSize(numPositions);
      rEntry.msToNext.setSize(numPositions);
      for (U32 j = 0; j < rEntry.positions.size(); j++) {
         mathRead(*stream, &rEntry.positions[j]);
         stream->read(&rEntry.msToNext[j]);
      }
   }

   return stream->getStatus() == Stream::Ok;
}



