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

#ifndef _AITURRETSHAPE_H_
#define _AITURRETSHAPE_H_

#ifndef _TURRETSHAPE_H_
   #include "T3D/turret/turretShape.h"
#endif


//----------------------------------------------------------------------------

class AITurretShapeData: public TurretShapeData {

   typedef TurretShapeData Parent;

public:
   enum Constants {
      MaxStates    = 31,            ///< We get one less than state bits because of
                                    /// the way data is packed.
      NumStateBits = 5,
   };

   struct StateData {
      StateData();
      const char* name;             ///< State name

      /// @name Transition states
      ///
      /// @{

      ///
      struct Transition {
         S32 rest[2];               ///< NotAtRest/AtRest (NotStatic/Static)
         S32 target[2];             ///< NoTarget/Target
         S32 activated[2];          ///< Deactivated/Activated
         S32 timeout;               ///< Transition after delay
      } transition;

      /// @}

      /// @name State attributes
      /// @{

      bool fire;                    ///< Can only have one fire state
      bool scan;                    ///< Perform a continuous scan looking for targets
      bool scaleAnimation;          ///< Scale animation to fit the state timeout
      bool direction;               ///< Animation direction
      bool waitForTimeout;          ///< Require the timeout to pass before advancing to the next
                                    ///  state.
      F32 timeoutValue;             ///< A timeout value; the effect of this value is determined
                                    ///  by the flags scaleAnimation and waitForTimeout
      S32 sequence;                 ///< Main thread sequence ID.
                                    ///
                                    ///
      const char* script;           ///< Function on datablock to call when we enter this state; passed the id of
                                    ///  the imageSlot.

      /// @}
   };

   /// @name State Data
   /// Individual state data used to initialize struct array
   /// @{
   const char*             stateName                  [MaxStates];

   const char*             stateTransitionAtRest      [MaxStates];
   const char*             stateTransitionNotAtRest   [MaxStates];
   const char*             stateTransitionTarget      [MaxStates];
   const char*             stateTransitionNoTarget    [MaxStates];
   const char*             stateTransitionActivated   [MaxStates];
   const char*             stateTransitionDeactivated [MaxStates];
   const char*             stateTransitionTimeout     [MaxStates];
   F32                     stateTimeoutValue          [MaxStates];
   bool                    stateWaitForTimeout        [MaxStates];

   bool                    stateFire                  [MaxStates];
   bool                    stateScan                  [MaxStates];

   bool                    stateScaleAnimation        [MaxStates];
   bool                    stateDirection             [MaxStates];
   const char*             stateSequence              [MaxStates];

   const char*             stateScript                [MaxStates];

   /// @}

   /// @name State Array
   ///
   /// State array is initialized onAdd from the individual state
   /// struct array elements.
   ///
   /// @{
   StateData   state[MaxStates];       ///< Array of states.
   bool        statesLoaded;           ///< Are the states loaded yet?
   S32         fireState;              ///< The ID of the fire state.
   bool        isAnimated;             ///< This image contains at least one animated states
   /// @}

   F32   maxScanHeading;               ///< Maximum heading angle from center to scan, in degrees
   F32   maxScanPitch;                 ///< Maximum pitch angle from center to scan, in degrees
   F32   maxScanDistance;              ///< Maximum distance to scan to

   S32   scanTickFrequency;            ///< How often should we perform a scan
   S32   scanTickFrequencyVariance;    ///< Random amount that should be added to the scan tick frequency

   F32   trackLostTargetTime;          ///< How long after the turret has lost the target should it still track it (in seconds)

   S32 scanNode;                       ///< The node on the shape we will scan from
   S32 aimNode;                        ///< The node on the shape we will aim from

   F32   maxWeaponRange;               ///< Maximum range of the weapons, which may be different than the max scan distance

   F32   weaponLeadVelocity;           ///< Velocity used to lead target (if value <= 0, don't lead target).

public:
   AITurretShapeData();

   DECLARE_CONOBJECT(AITurretShapeData);

   static void initPersistFields();

   virtual bool onAdd();
   virtual bool preload(bool server, String &errorStr);

   virtual void packData(BitStream* stream);
   virtual void unpackData(BitStream* stream);

   S32 lookupState(const char* name);  ///< Get a state by name.
};

//----------------------------------------------------------------------------

class AITurretShape: public TurretShape
{
   typedef TurretShape Parent;

protected:

   enum MaskBits {
      TurretStateMask   = Parent::NextFreeMask,
      NextFreeMask      = Parent::NextFreeMask << 1
   };

   struct TargetInfo
   {
      SimObjectPtr<ShapeBase> target;           ///< Current target
      Point3F                 lastPos;          ///< The target's last known position
      VectorF                 lastVel;          ///< The target's last known velocity
      SimTime                 lastSightTime;    ///< The last time we saw the target
      bool                    hadValidTarget;   ///< Did we previously have a valid target?

      TargetInfo() {reset();}

      void reset()
      {
         target = NULL;
         lastPos.zero();
         lastVel.zero();
         lastSightTime = 0;
         hadValidTarget = false;
      }

      // Check if we currently have a valid target
      bool isValid() const {return target != NULL;}

      // Check if we used to have a target
      bool hadTarget() const {return hadValidTarget;}
   };

   // Static attributes
   AITurretShapeData* mDataBlock;

   F32      mScanHeading;
   F32      mScanPitch;
   F32      mScanDistance;
   F32      mScanDistanceSquared;
   Box3F    mScanBox;
   Box3F    mTransformedScanBox;

   S32      mScanTickFrequency;
   S32      mScanTickFrequencyVariance;
   S32      mTicksToNextScan;

   F32      mWeaponRangeSquared;
   F32      mWeaponLeadVelocitySquared;

   SimSet   mIgnoreObjects;      ///< Ignore these objects when targeting

   bool        mScanForTargets;
   bool        mTrackTarget;
   TargetInfo  mTarget;                ///< Information on the current target

   SimObjectList  mPotentialTargets;

   AITurretShapeData::StateData *mState;
   F32 mStateDelayTime;                   ///< Time till next state.
   bool mStateActive;                     ///< Is the turret active?
   TSThread *mStateAnimThread;

   void _initState();
   void _updateTurretState(F32 dt);

   /// Utility function to call state script functions on the datablock
   /// @param   function   Function
   void _scriptCallback(const char* function);

   void _setScanBox();

   void _cleanupPotentialTargets();
   void _performScan();
   void _lostTarget();
   void _gainedTarget(ShapeBase* target);
   void _trackTarget(F32 dt);
   void _cleanupTargetAndTurret();
   bool _testTargetLineOfSight(Point3F& aimPoint, ShapeBase* target, Point3F& sightPoint);

   /// ObjectRenderInst delegate hooked up in prepBatchRender 
   /// if GameBase::gShowBoundingBox is true.
   void _renderScanner( ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance *overrideMat );

public:

   MatrixF   mScanWorkspaceScanMat;
   MatrixF   mScanWorkspaceScanWorldMat;

public:

   AITurretShape();
   virtual ~AITurretShape();

   static void initPersistFields();   

   bool onAdd();
   void onRemove();
   bool onNewDataBlock(GameBaseData *dptr, bool reload);

   void addToIgnoreList(ShapeBase* obj);
   void removeFromIgnoreList(ShapeBase* obj);

   void setTurretStateName(const char* newState, bool force=false);
   void setTurretState(U32 newState, bool force=false);

   void activateTurret() {mStateActive = true;}
   void deactivateTurret() {mStateActive = false;}

   void startScanForTargets() {mScanForTargets = true;}
   void stopScanForTargets() {mScanForTargets = false;}

   void startTrackingTarget() {mTrackTarget = true;}
   void stopTrackingTarget() {mTrackTarget = false;}

   ShapeBase* getTarget() {return mTarget.target;}
   bool hasTarget() {return mTarget.target != NULL;}
   void resetTarget() {mTarget.reset();}

   void addPotentialTarget(ShapeBase* shape);

   void setWeaponLeadVelocity(F32 velocity) {mWeaponLeadVelocitySquared = velocity * velocity;}
   F32 getWeaponLeadVelocity() {return mSqrt(mWeaponLeadVelocitySquared);}

   void setAllGunsFiring(bool fire);
   void setGunSlotFiring(S32 slot, bool fire);

   virtual void setTransform(const MatrixF &mat);
   void getScanTransform(MatrixF& mat);
   void getAimTransform(MatrixF& mat);

   F32 getMaxScanHeading() const {return mScanHeading;}
   F32 getMaxScanPitch() const {return mScanPitch;}
   F32 getMaxScanDistance() const {return mScanDistance;}
   F32 getMaxScanDistanceSquared() const {return mScanDistanceSquared;}

   void recenterTurret();

   virtual void processTick(const Move *move);
   virtual void advanceTime(F32 dt);

   virtual U32  packUpdate  (NetConnection *conn, U32 mask, BitStream *stream);
   virtual void unpackUpdate(NetConnection *conn,           BitStream *stream);

   void prepBatchRender( SceneRenderState *state, S32 mountedImageIndex );

   DECLARE_CONOBJECT(AITurretShape);
};

#endif // _AITURRETSHAPE_H_
