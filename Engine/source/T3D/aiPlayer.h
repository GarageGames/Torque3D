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

#ifndef _AIPLAYER_H_
#define _AIPLAYER_H_

#ifndef _PLAYER_H_
#include "T3D/player.h"
#endif

#ifdef TORQUE_NAVIGATION_ENABLED
#include "navigation/navPath.h"
#include "navigation/navMesh.h"
#include "navigation/coverPoint.h"
#endif // TORQUE_NAVIGATION_ENABLED

class AIPlayer : public Player {

	typedef Player Parent;

public:
	enum MoveState {
		ModeStop,                       // AI has stopped moving.
		ModeMove,                       // AI is currently moving.
		ModeStuck,                      // AI is stuck, but wants to move.
      ModeSlowing,                    // AI is slowing down as it reaches it's destination.
	};

private:
   MoveState mMoveState;
   F32 mMoveSpeed;
   F32 mMoveTolerance;                 // Distance from destination before we stop
   F32 mAttackRadius;                  // Distance to trigger weaponry calcs
   Point3F mMoveDestination;           // Destination for movement
   Point3F mLastLocation;              // For stuck check
   F32 mMoveStuckTolerance;            // Distance tolerance on stuck check
   S32 mMoveStuckTestDelay;            // The number of ticks to wait before checking if the AI is stuck
   S32 mMoveStuckTestCountdown;        // The current countdown until at AI starts to check if it is stuck
   bool mMoveSlowdown;                 // Slowdown as we near the destination

   SimObjectPtr<GameBase> mAimObject; // Object to point at, overrides location
   bool mAimLocationSet;               // Has an aim location been set?
   Point3F mAimLocation;               // Point to look at
   bool mTargetInLOS;                  // Is target object visible?

   Point3F mAimOffset;

   // move triggers
   bool mMoveTriggers[MaxTriggerKeys];

   // Utility Methods
   void throwCallback( const char *name );

#ifdef TORQUE_NAVIGATION_ENABLED
   /// Should we jump?
   enum JumpStates {
      None,  ///< No, don't jump.
      Now,   ///< Jump immediately.
      Ledge, ///< Jump when we walk off a ledge.
   } mJump;

   /// Stores information about a path.
   struct PathData {
      /// Pointer to path object.
      SimObjectPtr<NavPath> path;
      /// Do we own our path? If so, we will delete it when finished.
      bool owned;
      /// Path node we're at.
      U32 index;
      /// Default constructor.
      PathData() : path(NULL)
      {
         owned = false;
         index = 0;
      }
   };

   /// Path we are currently following.
   PathData mPathData;


   /// Get the current path we're following.
   NavPath *getPath() { return mPathData.path; }

   /// Stores information about our cover.
   struct CoverData {
      /// Pointer to a cover point.
      SimObjectPtr<CoverPoint> cover;
      /// Default constructor.
      CoverData() : cover(NULL) {}
   };

   /// Current cover we're trying to get to.
   CoverData mCoverData;


   /// Information about a target we're following.
   struct FollowData {
      /// Object to follow.
      SimObjectPtr<SceneObject> object;
      /// Distance at whcih to follow.
      F32 radius;
      Point3F lastPos;
      /// Default constructor.
      FollowData() : object(NULL)
      {
         radius = 5.0f;
         lastPos = Point3F::Zero;
      }
   };

   /// Current object we're following.
   FollowData mFollowData;


   /// NavMesh we pathfind on.
   SimObjectPtr<NavMesh> mNavMesh;

   /// Move to the specified node in the current path.
   void moveToNode(S32 node);
#endif // TORQUE_NAVIGATION_ENABLED

protected:
   virtual void onReachDestination();
   virtual void onStuck();

public:
   DECLARE_CONOBJECT( AIPlayer );

   AIPlayer();
   ~AIPlayer();

   static void initPersistFields();

   bool onAdd();
   void onRemove();

   virtual bool getAIMove( Move *move );
   virtual void updateMove(const Move *move);
   /// Clear out the current path.
   void clearPath();
   /// Stop searching for cover.
   void clearCover();
   /// Stop following me!
   void clearFollow();

   // Targeting and aiming sets/gets
   void setAimObject( GameBase *targetObject );
   void setAimObject(GameBase *targetObject, const Point3F& offset);
   GameBase* getAimObject() const  { return mAimObject; }
   void setAimLocation( const Point3F &location );
   Point3F getAimLocation() const { return mAimLocation; }
   void clearAim();
   bool checkInLos(GameBase* target = NULL, bool _useMuzzle = false, bool _checkEnabled = false);
   bool checkInFoV(GameBase* target = NULL, F32 camFov = 45.0f, bool _checkEnabled = false);
   F32 getTargetDistance(GameBase* target, bool _checkEnabled);

   // Movement sets/gets
   void setMoveSpeed( const F32 speed );
   F32 getMoveSpeed() const { return mMoveSpeed; }
   void setMoveTolerance( const F32 tolerance );
   F32 getMoveTolerance() const { return mMoveTolerance; }
   void setMoveDestination( const Point3F &location, bool slowdown );
   Point3F getMoveDestination() const { return mMoveDestination; }
   void stopMove();
   void setAiPose( S32 pose );
   S32  getAiPose();
	
   // Trigger sets/gets
   void setMoveTrigger( U32 slot, const bool isSet = true );
   bool getMoveTrigger( U32 slot ) const;
   void clearMoveTriggers();

#ifdef TORQUE_NAVIGATION_ENABLED
   /// @name Pathfinding
   /// @{

   enum NavSize {
      Small,
      Regular,
      Large
   } mNavSize;
   void setNavSize(NavSize size) { mNavSize = size; updateNavMesh(); }
   NavSize getNavSize() const { return mNavSize; }

   bool setPathDestination(const Point3F &pos);
   Point3F getPathDestination() const;

   void followNavPath(NavPath *path);
   void followObject(SceneObject *obj, F32 radius);

   void repath();

   bool findCover(const Point3F &from, F32 radius);

   NavMesh *findNavMesh() const;
   void updateNavMesh();
   NavMesh *getNavMesh() const { return mNavMesh; }

   /// Get cover we are moving to.
   CoverPoint *getCover() { return mCoverData.cover; }

   /// Types of link we can use.
   LinkData mLinkTypes;

   /// @}
#endif // TORQUE_NAVIGATION_ENABLED
};

#endif
