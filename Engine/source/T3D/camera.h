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

#ifndef _CAMERA_H_
#define _CAMERA_H_

#ifndef _SHAPEBASE_H_
#include "T3D/shapeBase.h"
#endif

#ifndef _DYNAMIC_CONSOLETYPES_H_
#include "console/dynamicTypes.h"
#endif


class CameraData: public ShapeBaseData 
{
   public:

      typedef ShapeBaseData Parent;

      // ShapeBaseData.
      DECLARE_CONOBJECT( CameraData );
      DECLARE_CATEGORY( "Game" );
      DECLARE_DESCRIPTION( "A datablock that describes a camera." );

      static void initPersistFields();
      virtual void packData(BitStream* stream);
      virtual void unpackData(BitStream* stream);
};


/// Implements a basic camera object.
class Camera: public ShapeBase
{
   public:

      typedef ShapeBase Parent;

      /// Movement behavior type for camera.
      enum CameraMotionMode
      {
         StationaryMode  = 0,

         FreeRotateMode,
         FlyMode,
         OrbitObjectMode,
         OrbitPointMode,
         TrackObjectMode,
         OverheadMode,
         EditOrbitMode,       ///< Used by the World Editor

         CameraFirstMode = 0,
         CameraLastMode  = EditOrbitMode
      };

      /// The ExtendedMove position/rotation index used for camera movements
      static S32 smExtendedMovePosRotIndex;

   protected:

      enum MaskBits
      {
         MoveMask          = Parent::NextFreeMask,
         UpdateMask        = Parent::NextFreeMask << 1,
         NewtonCameraMask  = Parent::NextFreeMask << 2,
         EditOrbitMask     = Parent::NextFreeMask << 3,
         NextFreeMask      = Parent::NextFreeMask << 4
      };

      struct StateDelta
      {
         Point3F pos;
         Point3F rot;
         VectorF posVec;
         VectorF rotVec;
      };

      CameraData* mDataBlock;

      Point3F mRot;
      StateDelta mDelta;

      Point3F mOffset;

      static F32 smMovementSpeed;

      SimObjectPtr<GameBase> mOrbitObject;
      F32 mMinOrbitDist;
      F32 mMaxOrbitDist;
      F32 mCurOrbitDist;
      Point3F mPosition;
      bool mObservingClientObject;

      F32 mLastAbsoluteYaw;            ///< Stores that last absolute yaw value as passed in by ExtendedMove
      F32 mLastAbsolutePitch;          ///< Stores that last absolute pitch value as passed in by ExtendedMove
      F32 mLastAbsoluteRoll;           ///< Stores that last absolute roll value as passed in by ExtendedMove

      /// @name NewtonFlyMode
      /// @{

      VectorF  mAngularVelocity;
      F32      mAngularForce;
      F32      mAngularDrag;
      VectorF  mVelocity;
      bool     mNewtonMode;
      bool     mNewtonRotation;
      F32      mMass;
      F32      mDrag;
      F32      mFlyForce;
      F32      mSpeedMultiplier;
      F32      mBrakeMultiplier;

      /// @}

      /// @name EditOrbitMode
      /// @{

      bool     mValidEditOrbitPoint;
      Point3F  mEditOrbitPoint;
      F32      mCurrentEditOrbitDist;

      /// @}

      bool mLocked;

      CameraMotionMode  mMode;

      void _setPosition(const Point3F& pos,const Point3F& viewRot);
      void _setRenderPosition(const Point3F& pos,const Point3F& viewRot);
      void _validateEyePoint( F32 pos, MatrixF* mat );

      void _calcOrbitPoint( MatrixF* mat, const Point3F& rot );
      void _calcEditOrbitPoint( MatrixF *mat, const Point3F& rot );

      static bool _setModeField( void *object, const char *index, const char *data );
      static bool _setNewtonField( void *object, const char *index, const char *data );

      // ShapeBase.
      virtual F32 getCameraFov();
      virtual void setCameraFov( F32 fov );
      virtual F32 getDefaultCameraFov();
      virtual bool isValidCameraFov( F32 fov );
      virtual F32 getDamageFlash() const;
      virtual F32 getWhiteOut() const;
      virtual void setTransform( const MatrixF& mat );
      virtual void setRenderTransform( const MatrixF& mat );

   public:

      Camera();
      ~Camera();

      CameraMotionMode getMode() const { return mMode; }

      Point3F getPosition();
      Point3F getRotation() { return mRot; };
      void setRotation( const Point3F& viewRot );

      Point3F getOffset() { return mOffset; };
      void lookAt( const Point3F& pos);
      void setOffset( const Point3F& offset) { if( mOffset != offset ) mOffset = offset; setMaskBits( UpdateMask ); }
      void setFlyMode();
      void setOrbitMode( GameBase *obj, const Point3F& pos, const Point3F& rot, const Point3F& offset,
                         F32 minDist, F32 maxDist, F32 curDist, bool ownClientObject, bool locked = false );
      void setTrackObject( GameBase *obj, const Point3F& offset);
      void onDeleteNotify( SimObject* obj );

      GameBase* getOrbitObject()      { return(mOrbitObject); }
      bool isObservingClientObject()   { return(mObservingClientObject); }

      /// @name NewtonFlyMode
      /// @{

      void setNewtonFlyMode();
      VectorF getVelocity() const { return mVelocity; }
      void setVelocity( const VectorF& vel );
      VectorF getAngularVelocity() const { return mAngularVelocity; }
      void setAngularVelocity( const VectorF& vel );
      bool isRotationDamped() {return mNewtonRotation;}
      void setAngularForce( F32 force ) {mAngularForce = force; setMaskBits(NewtonCameraMask);}
      void setAngularDrag( F32 drag ) {mAngularDrag = drag; setMaskBits(NewtonCameraMask);}
      void setMass( F32 mass ) {mMass = mass; setMaskBits(NewtonCameraMask);}
      void setDrag( F32 drag ) {mDrag = drag; setMaskBits(NewtonCameraMask);}
      void setFlyForce( F32 force ) {mFlyForce = force; setMaskBits(NewtonCameraMask);}
      void setSpeedMultiplier( F32 mul ) {mSpeedMultiplier = mul; setMaskBits(NewtonCameraMask);}
      void setBrakeMultiplier( F32 mul ) {mBrakeMultiplier = mul; setMaskBits(NewtonCameraMask);}

      /// @}

      /// @name EditOrbitMode
      /// @{

      void setEditOrbitMode();
      bool isEditOrbitMode() {return mMode == EditOrbitMode;}
      bool getValidEditOrbitPoint() { return mValidEditOrbitPoint; }
      void setValidEditOrbitPoint( bool state );
      Point3F getEditOrbitPoint() const;
      void setEditOrbitPoint( const Point3F& pnt );

      /// Orient the camera to view the given radius.  Requires that an
      /// edit orbit point has been set.
      void autoFitRadius( F32 radius );

      /// @}

      // ShapeBase.
      static void initPersistFields();
      static void consoleInit();

      virtual void onEditorEnable();
      virtual void onEditorDisable();

      virtual bool onAdd();
      virtual void onRemove();
      virtual bool onNewDataBlock( GameBaseData *dptr, bool reload );
      virtual void processTick( const Move* move );
      virtual void interpolateTick( F32 delta);
      virtual void getCameraTransform( F32* pos,MatrixF* mat );
      virtual void getEyeCameraTransform( IDisplayDevice *display, U32 eyeId, MatrixF *outMat );
      virtual DisplayPose calcCameraDeltaPose(GameConnection *con, const DisplayPose& inPose);

      virtual void writePacketData( GameConnection* conn, BitStream* stream );
      virtual void readPacketData( GameConnection* conn, BitStream* stream );
      virtual U32  packUpdate( NetConnection* conn, U32 mask, BitStream* stream );
      virtual void unpackUpdate( NetConnection* conn, BitStream* stream );

      DECLARE_CONOBJECT( Camera );
      DECLARE_CATEGORY( "Game" );
      DECLARE_DESCRIPTION( "Represents a position, direction and field of view to render a scene from." );
};

typedef Camera::CameraMotionMode CameraMotionMode;
DefineEnumType( CameraMotionMode );

#endif
