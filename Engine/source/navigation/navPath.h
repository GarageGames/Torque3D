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

#ifndef _NAVPATH_H_
#define _NAVPATH_H_

#include "scene/sceneObject.h"
#include "scene/simPath.h"
#include "navMesh.h"
#include <DetourNavMeshQuery.h>

class NavPath: public SceneObject {
   typedef SceneObject Parent;
   /// Maximum size of Detour path.
   static const U32 MaxPathLen = 1024;

public:
   /// @name NavPath
   /// Functions for planning and accessing the path.
   /// @{

   SimObjectPtr<NavMesh> mMesh;
   SimObjectPtr<SimPath::Path> mWaypoints;

   /// Location to start at.
   Point3F mFrom;
   /// Has a starting location been set?
   bool mFromSet;
   /// Location to end at.
   Point3F mTo;
   /// Has an end been set?
   bool mToSet;

   /// This path should include a segment from the end to the start.
   bool mIsLooping;

   /// Render even when not selected in the editor.
   bool mAlwaysRender;
   /// Render on top of other objects.
   bool mXray;

   /// Plan the path.
   bool plan();

   /// Updated a sliced plan.
   /// @return True if we need to keep updating, false if we can stop.
   bool update();

   /// Finalise a sliced plan.
   /// @return True if the plan was successful overall.
   bool finalise();

   /// @}

   /// @name Path interface
   /// @{

   /// Return world-space position of a path node.
   /// @param[in] idx Node index to retrieve.
   Point3F getNode(S32 idx);

   /// Return the number of nodes in this path.
   S32 getCount();

   /// Return the length of this path.
   F32 getLength() { return mLength; };

   /// @}

   /// @name SceneObject
   /// @{

   static void initPersistFields();

   bool onAdd();
   void onRemove();

   void onEditorEnable();
   void onEditorDisable();
   void inspectPostApply();

   void onDeleteNotify(SimObject *object);

   U32 packUpdate(NetConnection *conn, U32 mask, BitStream *stream);
   void unpackUpdate(NetConnection *conn, BitStream *stream);

   void prepRenderImage(SceneRenderState *state);
   void renderSimple(ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance *overrideMat);

   DECLARE_CONOBJECT(NavPath);

   /// @}

   /// @name ProcessObject
   /// @{
   void processTick(const Move *move);
   /// @}

   NavPath();
   ~NavPath();

protected:
   enum masks {
      PathMask     = Parent::NextFreeMask << 0,
      NextFreeMask = Parent::NextFreeMask << 1
   };

private:
   /// Create appropriate data structures and stuff.
   bool init();

   /// 'Visit' the most recent two points on our visit list.
   bool visitNext();

   /// Detour path query.
   dtNavMeshQuery *mQuery;
   /// Current status of our Detour query.
   dtStatus mStatus;
   /// Filter that provides the movement costs for paths.
   dtQueryFilter mFilter;
   
   /// List of points the path should visit (waypoints, if you will).
   Vector<Point3F> mVisitPoints;
   /// List of points in the final path.
   Vector<Point3F> mPoints;

   /// Total length of path in world units.
   F32 mLength;

   /// Resets our world transform and bounds to fit our point list.
   void resize();

   /// @name Protected console getters/setters
   /// @{
   static bool setProtectedMesh(void *obj, const char *index, const char *data);
   static const char *getProtectedMesh(void *obj, const char *data);
   static bool setProtectedWaypoints(void *obj, const char *index, const char *data);

   static bool setProtectedFrom(void *obj, const char *index, const char *data);
   static const char *getProtectedFrom(void *obj, const char *data);

   static bool setProtectedTo(void *obj, const char *index, const char *data);
   static const char *getProtectedTo(void *obj, const char *data);
   /// @}
};

#endif
