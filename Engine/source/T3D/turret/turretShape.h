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

#ifndef _TURRETSHAPE_H_
#define _TURRETSHAPE_H_

#ifndef _ITEM_H_
   #include "T3D/item.h"
#endif

class PhysicsBody;
class TurretShape;

//----------------------------------------------------------------------------

class TurretShapeData: public ItemData {

   typedef ItemData Parent;

public:

   enum FireLinkType {
      FireTogether,              ///< All weapons fire under trigger 0
      GroupedFire,               ///< Weapon mounts 0,2 fire under trigger 0, mounts 1,3 fire under trigger 1
      IndividualFire,            ///< Each weapon mount fires under its own trigger 0-3
      NumFireLinkTypeBits = 2
   };

   FireLinkType   weaponLinkType;   ///< How are the weapons linked together and triggered

   enum {
      NumMirrorDirectionNodes = 4,
   };

   enum Recoil {
      LightRecoil,
      MediumRecoil,
      HeavyRecoil,
      NumRecoilSequences
   };
   S32 recoilSequence[NumRecoilSequences];

   S32   pitchSequence;                ///< Optional sequence played when the turret pitches
   S32   headingSequence;              ///< Optional sequence played when the turret's heading changes

   F32 cameraOffset;          ///< Vertical offset

   F32 maxHeading;            ///< Max degrees to rotate from center. 180 or more indicates full rotation.
   F32 minPitch;              ///< Min degrees to rotate down from straight ahead
   F32 maxPitch;              ///< Max degrees to rotate up from straight ahead

   F32 headingRate;           ///< Degrees per second rotation.  0 means not allowed.  Less than 0 means instantaneous.
   F32 pitchRate;             ///< Degrees per second rotation.  0 means not allowed.  Less than 0 means instantaneous.

   S32 headingNode;                                   ///< Code controlled node for heading changes
   S32 pitchNode;                                     ///< Code controlled node for pitch changes
   S32 pitchNodes[NumMirrorDirectionNodes];           ///< Additional nodes that mirror the movements of the pitch node.
   S32 headingNodes[NumMirrorDirectionNodes];         ///< Additional nodes that mirror the movements of the heading node.
   S32 weaponMountNode[ShapeBase::MaxMountedImages];  ///< Where ShapeBaseImageData weapons are mounted

   bool startLoaded;          ///< Should the turret's mounted weapon(s) start in a loaded state?

   bool zRotOnly;             ///< Should the turret allow only z rotations (like an item)?

public:
   TurretShapeData();

   DECLARE_CONOBJECT(TurretShapeData);

   static void initPersistFields();

   virtual void packData(BitStream* stream);
   virtual void unpackData(BitStream* stream);

   virtual bool preload(bool server, String &errorStr);

   DECLARE_CALLBACK( void, onMountObject, ( SceneObject* turret, SceneObject* obj, S32 node ) );
   DECLARE_CALLBACK( void, onUnmountObject, ( SceneObject* turret, SceneObject* obj ) );
   DECLARE_CALLBACK( void, onStickyCollision, ( TurretShape* obj ) );
};

typedef TurretShapeData::FireLinkType TurretShapeFireLinkType;

DefineEnumType( TurretShapeFireLinkType );

//----------------------------------------------------------------------------

class TurretShape: public Item
{
   typedef Item Parent;
   typedef SceneObject GrandParent;

protected:
   enum MaskBits {
      TurretUpdateMask     = Parent::NextFreeMask << 0,  ///< using one mask because we're running out of maskbits
      NextFreeMask         = Parent::NextFreeMask << 1
   };

   // Client interpolation data for turret heading and pitch
   struct TurretStateDelta {
      Point3F rot;
      VectorF rotVec;
      F32     dt;
   };
   TurretStateDelta mTurretDelta;

   Point3F mRot;           ///< Current heading and pitch
   bool mPitchAllowed;     ///< Are pitch changes allowed
   bool mHeadingAllowed;   ///< Are heading changes allowed
   F32 mPitchUp;           ///< Pitch up limit, in radians
   F32 mPitchDown;         ///< Pitch down limit, in radians
   F32 mHeadingMax;        ///< Maximum heading limit from center, in radians
   F32 mPitchRate;         ///< How fast the turret may pitch, in radians per second
   F32 mHeadingRate;       ///< How fast the turret may yaw, in radians per second

   bool mRespawn;

   TSThread* mRecoilThread;
   TSThread* mImageStateThread;

   TSThread* mPitchThread;
   TSThread* mHeadingThread;

   // Static attributes
   TurretShapeData* mDataBlock;

   bool mSubclassTurretShapeHandlesScene;    ///< A subclass of TurretShape will handle all of the adding to the scene

   void _setRotation(const Point3F& rot);
   void _updateNodes(const Point3F& rot);
   void _applyLimits(Point3F& rot);
   bool _outsideLimits(Point3F& rot);        ///< Return true if any angle is outside of the limits

   void onUnmount(SceneObject* obj,S32 node);

   // Script level control
   bool allowManualRotation;
   bool allowManualFire;

   void updateAnimation(F32 dt);

   virtual void onImage(U32 imageSlot, bool unmount);
   virtual void onImageRecoil(U32 imageSlot,ShapeBaseImageData::StateData::RecoilState);
   virtual void onImageStateAnimation(U32 imageSlot, const char* seqName, bool direction, bool scaleToState, F32 stateTimeOutValue);

public:

   TurretShape();
   virtual ~TurretShape();

   static void initPersistFields();   

   bool onAdd();
   void onRemove();
   bool onNewDataBlock(GameBaseData *dptr, bool reload);

   const char* getStateName();
   virtual void updateDamageLevel();

   virtual void processTick(const Move *move);
   virtual void interpolateTick(F32 dt);
   virtual void advanceTime(F32 dt);

   virtual void setTransform( const MatrixF &mat );

   virtual bool getAllowManualRotation() { return allowManualRotation; }
   virtual void setAllowManualRotation(bool allow) { setMaskBits(TurretUpdateMask); allowManualRotation = allow; }
   virtual bool getAllowManualFire() { return allowManualFire; }
   virtual void setAllowManualFire(bool allow) { setMaskBits(TurretUpdateMask); allowManualFire = allow; }

   virtual void updateMove(const Move* move);

   bool getNodeTransform(S32 node, MatrixF& mat);
   bool getWorldNodeTransform(S32 node, MatrixF& mat);

   Point3F getTurretRotation() {return mRot;}
   void setTurretRotation(const Point3F& rot) {_setRotation(rot);}

   bool doRespawn() { return mRespawn; };

   virtual void mountObject( SceneObject *obj, S32 node, const MatrixF &xfm = MatrixF::Identity );
   virtual void unmountObject( SceneObject *obj );

   virtual void getCameraParameters(F32 *min,F32* max,Point3F* offset,MatrixF* rot);
   virtual void getCameraTransform(F32* pos,MatrixF* mat);

   virtual void writePacketData( GameConnection* conn, BitStream* stream );
   virtual void readPacketData( GameConnection* conn, BitStream* stream );
   virtual U32  packUpdate  (NetConnection *conn, U32 mask, BitStream *stream);
   virtual void unpackUpdate(NetConnection *conn,           BitStream *stream);

   virtual void getWeaponMountTransform( S32 index, const MatrixF &xfm, MatrixF *outMat );
   virtual void getRenderWeaponMountTransform( F32 delta, S32 index, const MatrixF &xfm, MatrixF *outMat );

   virtual void getImageTransform(U32 imageSlot,MatrixF* mat);
   virtual void getRenderImageTransform(U32 imageSlot,MatrixF* mat,bool noEyeOffset=false);

   virtual void getImageTransform(U32 imageSlot,S32 node, MatrixF* mat);
   virtual void getRenderImageTransform(U32 imageSlot,S32 node, MatrixF* mat);

   virtual void prepRenderImage( SceneRenderState* state );
   virtual void prepBatchRender( SceneRenderState *state, S32 mountedImageIndex );

   DECLARE_CONOBJECT(TurretShape);
};

#endif // _TURRETSHAPE_H_
