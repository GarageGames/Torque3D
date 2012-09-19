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

#ifndef _HOVERVEHICLE_H_
#define _HOVERVEHICLE_H_

#ifndef _VEHICLE_H_
#include "T3D/vehicles/vehicle.h"
#endif

class ParticleEmitter;
class ParticleEmitterData;

// -------------------------------------------------------------------------
class HoverVehicleData : public VehicleData
{
   typedef VehicleData Parent;

  protected:
   bool onAdd();

   //-------------------------------------- Console set variables
  public:
   enum Sounds {
      JetSound,
      EngineSound,
      FloatSound,
      MaxSounds
   };
   SFXProfile* sound[MaxSounds];

   enum Jets {
      // These enums index into a static name list.
      ForwardJetEmitter,      // Thrust forward
      BackwardJetEmitter,     // Thrust backward
      DownwardJetEmitter,     // Thrust down
      MaxJetEmitters,
   };
   ParticleEmitterData* jetEmitter[MaxJetEmitters];

   enum JetNodes {
      // These enums index into a static name list.
      ForwardJetNode,
      ForwardJetNode1,
      BackwardJetNode,
      BackwardJetNode1,
      DownwardJetNode,
      DownwardJetNode1,
      //
      MaxJetNodes,
      MaxDirectionJets = 2,
      ThrustJetStart = ForwardJetNode,
      MaxTrails = 4,
   };
   static const char *sJetNode[MaxJetNodes];
   S32 jetNode[MaxJetNodes];


   F32 dragForce;
   F32 vertFactor;
   F32 floatingThrustFactor;

   F32 mainThrustForce;
   F32 reverseThrustForce;
   F32 strafeThrustForce;
   F32 turboFactor;

   F32 stabLenMin;
   F32 stabLenMax;
   F32 stabSpringConstant;
   F32 stabDampingConstant;

   F32 gyroDrag;
   F32 normalForce;
   F32 restorativeForce;
   F32 steeringForce;
   F32 rollForce;
   F32 pitchForce;

   F32 floatingGravMag;

   F32 brakingForce;
   F32 brakingActivationSpeed;

   ParticleEmitterData * dustTrailEmitter;
   S32                   dustTrailID;
   Point3F               dustTrailOffset;
   F32                   triggerTrailHeight;
   F32                   dustTrailFreqMod;

   //-------------------------------------- load set variables
  public:
   F32 maxThrustSpeed;

  public:
   HoverVehicleData();
   ~HoverVehicleData();

   void packData(BitStream*);
   void unpackData(BitStream*);
   bool preload(bool server, String &errorStr);

   DECLARE_CONOBJECT(HoverVehicleData);
   static void initPersistFields();
};


// -------------------------------------------------------------------------
class HoverVehicle : public Vehicle
{
   typedef Vehicle Parent;

  private:
   HoverVehicleData* mDataBlock;
   SimObjectPtr<ParticleEmitter> mDustTrailEmitter;

  protected:
   bool onAdd();
   void onRemove();
   bool onNewDataBlock(GameBaseData *dptr,bool reload);
   void updateDustTrail( F32 dt );

   // Vehicle overrides
  protected:
   void updateMove(const Move *move);

   // Physics
  protected:
   void updateForces(F32);
   F32 getBaseStabilizerLength() const;

   bool mFloating;
   F32  mThrustLevel;

   F32  mForwardThrust;
   F32  mReverseThrust;
   F32  mLeftThrust;
   F32  mRightThrust;

   SFXSource* mJetSound;
   SFXSource* mEngineSound;
   SFXSource* mFloatSound;

   enum ThrustDirection {
      // Enums index into sJetActivationTable
      ThrustForward,
      ThrustBackward,
      ThrustDown,
      NumThrustDirections,
      NumThrustBits = 3
   };
   ThrustDirection mThrustDirection;

   // Jet Threads
   enum Jets {
      // These enums index into a static name list.
      BackActivate,
      BackMaintain,
      JetAnimCount
   };
   static const char* sJetSequence[HoverVehicle::JetAnimCount];
   TSThread* mJetThread[JetAnimCount];
   S32 mJetSeq[JetAnimCount];
   bool mBackMaintainOn;

   // Jet Particles
   struct JetActivation {
      // Convert thrust direction into nodes & emitters
      S32 node;
      S32 emitter;
   };
   static JetActivation sJetActivation[NumThrustDirections];
   SimObjectPtr<ParticleEmitter> mJetEmitter[HoverVehicleData::MaxJetNodes];

   U32 getCollisionMask();
   void updateJet(F32 dt);
   void updateEmitter(bool active,F32 dt,ParticleEmitterData *emitter,S32 idx,S32 count);
  public:
   HoverVehicle();
   ~HoverVehicle();

   // Time/Move Management
  public:
   void advanceTime(F32 dt);

   DECLARE_CONOBJECT(HoverVehicle);
//   static void initPersistFields();

   U32  packUpdate  (NetConnection *conn, U32 mask, BitStream *stream);
   void unpackUpdate(NetConnection *conn,           BitStream *stream);
};

#endif // _H_HOVERVEHICLE

