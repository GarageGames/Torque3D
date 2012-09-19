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

#ifndef _PXMULTIACTOR_H
#define _PXMULTIACTOR_H

#ifndef _GAMEBASE_H_
#include "T3D/gameBase/gameBase.h"
#endif
#ifndef __RESOURCE_H__
#include "core/resource.h"
#endif
#ifndef _T3D_PHYSICS_PHYSICSPLUGIN_H_
#include "T3D/physics/physicsPlugin.h"
#endif
#ifndef _PHYSX_H_
#include "T3D/physics/physx/px.h"
#endif
#ifndef _STRINGUNIT_H_
#include "core/strings/stringUnit.h"
#endif
#ifndef _PHYSICS_PHYSICSUSERDATA_H_
#include "T3D/physics/physicsUserData.h"
#endif
#ifndef _TSSHAPE_H_
#include "ts/tsShape.h"
#endif


class TSShapeInstance;
class BaseMatInstance;
class PxMultiActor;
class PxWorld;
class PxMaterial;
class NxScene;
class NxActor;
class NxShape;
class NxCompartment;
class NxJoint;
class NxMat34;
class NxVec3;
class ParticleEmitterData;


namespace NXU
{
   class NxuPhysicsCollection;
}


class PxUserData : public PhysicsUserData
{
public:

   /// The constructor.
   PxUserData()
      :  PhysicsUserData(),
         mIsBroken( false ),
         mParticleEmitterData( NULL )
   {
   }

   static PxUserData* getData( const NxActor &actor )
   {
      PxUserData *result = (PxUserData*)actor.userData;

      AssertFatal( !result || typeid( *result ) == typeid( PxUserData ),
          "PxUserData::getData - The pointer is the wrong type!" );

      return result;
   }

   static PxUserData* getData( const NxJoint &joint )
   {
      PxUserData *result = (PxUserData*)joint.userData;

      AssertFatal( !result || typeid( *result ) == typeid( PxUserData ),
          "PxUserData::getData - The pointer is the wrong type!" );

      return result;
   }

   typedef Signal<void(NxReal, NxJoint&)> JointBreakSignal;

   JointBreakSignal& getOnJointBreakSignal() { return mOnJointBreakSignal; }

   // Breakable stuff...
   Vector<NxActor*> mUnbrokenActors;
   Vector<NxActor*> mBrokenActors;
   Vector<NxMat34> mRelXfm;
   ParticleEmitterData *mParticleEmitterData;  
   bool mIsBroken;
   JointBreakSignal mOnJointBreakSignal;
};


class ParticleEmitterData;

class PxMultiActorData : public GameBaseData
{
   typedef GameBaseData Parent;

public:

   PxMultiActorData();
   virtual ~PxMultiActorData();

   DECLARE_CONOBJECT(PxMultiActorData);
   
   static void initPersistFields();
   
   void packData(BitStream* stream);
   void unpackData(BitStream* stream);
   
   bool preload( bool server, String &errorBuffer );
   //bool onAdd();
   
   void allocPrimBuffer( S32 overrideSize = -1 );

   bool _loadCollection( const UTF8 *path, bool isBinary );

   void _onFileChanged( const Torque::Path &path );

   void reload();

   void dumpModel();

   Signal<void(void)> mReloadSignal;

public:

   // Rendering
   StringTableEntry shapeName;
   Resource<TSShape> shape;

   PxMaterial *material;

   /// Filename to load the physics actor from.
   StringTableEntry physXStream;

   enum
   {      
      NumMountPoints = 32,
      MaxCorrectionNodes = 2
   };

   StringTableEntry correctionNodeNames[MaxCorrectionNodes];
   StringTableEntry mountNodeNames[NumMountPoints];
   S32 correctionNodes[MaxCorrectionNodes];
   S32 mountPointNode[NumMountPoints];  ///< Node index of mountPoint

   /// If true no network corrections will 
   /// be done during gameplay.
   bool noCorrection;

   /// Physics collection that holds the actor
   /// and all associated shapes and data.
   NXU::NxuPhysicsCollection *collection;

   bool createActors(   NxScene *scene,
                        NxCompartment *compartment,
                        const NxMat34 *nxMat, 
                        const Point3F& scale, 
                        Vector<NxActor*> *outActors,
                        Vector<NxShape*> *outShapes,
                        Vector<NxJoint*> *outJoints,
                        Vector<String> *outActorUserProperties,
                        Vector<String> *outJointUserProperties );

   /// Angular and Linear Drag (dampening) is scaled by this when in water.
   F32 waterDragScale;

   /// The density of this object (for purposes of buoyancy calculation only).
   F32 buoyancyDensity;

   F32 angularDrag;
   F32 linearDrag;

   /// If this flag is set to true,
   /// the physics actors will only be
   /// created on the client, and the server
   /// object is only responsible for ghosting.
   /// Objects with this flag set will never stop
   /// the physics player from moving through them.
   bool clientOnly;

   bool singlePlayerOnly;

   /// When applyImpulse is passed a force of this magnitude or greater
   /// any actors hit by the force vector that have broken versions 
   /// will become 'broken'.
   F32 breakForce;
};


class PxMultiActor : public GameBase
{
   typedef GameBase Parent;

   enum MaskBits 
   {
      MoveMask          = Parent::NextFreeMask << 0,
      WarpMask          = Parent::NextFreeMask << 1,
      LightMask         = Parent::NextFreeMask << 2,
      SleepMask         = Parent::NextFreeMask << 3,
      ForceSleepMask    = Parent::NextFreeMask << 4,
      ImpulseMask       = Parent::NextFreeMask << 5,
      UpdateMask        = Parent::NextFreeMask << 6,
      MountedMask       = Parent::NextFreeMask << 7,
      NextFreeMask      = Parent::NextFreeMask << 8
   };  

public:

   PxMultiActor();

   DECLARE_CONOBJECT( PxMultiActor );
   static void initPersistFields();

   // SimObject
   bool onAdd();
   void onRemove();
   void inspectPostApply();
   void onPhysicsReset( PhysicsResetEvent reset );
   void onStaticModified( const char *slotName, const char *newValue );
   void onDeleteNotify( SimObject *obj );

   // NetObject
   U32 packUpdate( NetConnection *conn, U32 mask, BitStream *stream );
   void unpackUpdate( NetConnection *conn, BitStream *stream );

   // SceneObject
   void prepRenderImage( SceneRenderState *state );
   void setScale( const VectorF &scale );
   void setTransform( const MatrixF &mat );
   virtual void mountObject( SceneObject *obj, U32 node );
   virtual void unmountObject( SceneObject *obj );   
   virtual void getMountTransform( U32 mountPoint, MatrixF *mat );
   virtual void getRenderMountTransform( U32 index, MatrixF *mat );

   // GameBase
   virtual bool onNewDataBlock( GameBaseData *dptr, bool reload );
   virtual void processTick( const Move *move );
   virtual void interpolateTick( F32 delta );
   virtual void applyImpulse( const Point3F &pos, const VectorF &vec );
   virtual void applyRadialImpulse( const Point3F &origin, F32 radius, F32 magnitude );

   /// PxMultiActor
   /// @{  

   /// Set visibility of all broken/unbroken meshes to match this state.
   void setAllBroken( bool isBroken );

   /// Sets up actors and meshes associated with the passed joint to reflect
   /// the desired state.
   void setBroken(   const NxMat34 &parentPose, 
                     const NxVec3 &parentVel,
                     PxUserData *userData, 
                     bool isBroken );

   /// 
   void setMeshHidden( String namePrefix, bool hidden );
   
   void setAllHidden( bool hide );

   void listMeshes( const String &state ) const;
   
   void _onJointBreak( NxReal breakForce, NxJoint &brokenJoint );

   void _onContact(  PhysicsUserData *us,
                     PhysicsUserData *them,
                     const Point3F &hitPoint,
                     const Point3F &hitForce );

   void applyWarp( const MatrixF& mat, bool interpRender, bool sweep );

   void getDynamicXfms( PxMultiActor *srcObj, F32 dt );        

   /// @}

protected:

   /// This creates the physics objects.
   bool _createActors( const MatrixF &xfm );

   /// Creates a PxUserData for a joint and parses userProperties into it.
   PxUserData* _createJointUserData( NxJoint *joint, String &userProperties );

   /// Creates a PxUserData and parses userProperties into it.
   PxUserData* _createActorUserData( NxActor *actor, String &userProperties );

   /// Called to cleanup the physics objects.
   void _destroyActors();

   NxActor* _findActor( const String &actorName ) const;

   /// Get the corresponding meshName for a given actor.   
   String _getMeshName( const NxActor *actor ) const;   

   ///
   void _updateBounds();

   void _updateContainerForces();

   void _debugRender( ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance *overrideMat );

   void onFileNotify();

   void _applyActorRadialForce( NxActor *inActor, const NxVec3 &origin, F32 radius, F32 magnitude );

   void _updateDeltas( bool clearDelta );

   bool _getNodeTransform( U32 nodeIdx, MatrixF *outXfm );

protected:

   PxMultiActorData *mDataBlock;

   PxWorld *mWorld;

   Vector<NxActor*> mActors;
   Vector<NxActor*> mMappedActors;
   Vector<S32> mMappedToActorIndex;
   Vector<S32> mMappedActorDL;
   Vector<NxJoint*> mJoints;
   Vector<NxShape*> mShapes;

   /// This is the root actor whose transform is the
   /// transform of this SceneObject.
   NxActor *mRootActor;  

   TSShapeInstance *mShapeInstance;
   Resource<TSShape> mDebrisShape;

   struct Delta
   {
      Point3F pos;
      Point3F lastPos;
      QuatF rot;
      QuatF lastRot;
   };

   Delta mDelta;

   Vector<Delta> mActorDeltas;

   /// The transform of this actor when it was first
   /// created.  It is used to reset the physics state
   /// when the editor is enabled.
   MatrixF mResetXfm;


   /// The userdata object assigned to all actors
   /// and joints of this multi-actor.
   //PxUserData mUserData;

   ///
   //Vector<MatrixF> mRelXfms;

   /// This is the scale the actors were built at and
   /// is used to decide if we need to recreate them.
   VectorF mActorScale;
   //F32 mBuildAngDrag;
   //F32 mBuildLinDrag;

   VectorF mStartImpulse;

   bool mDebugRender;

   /// A helper set to true if is a client object and
   /// is a singlePlayerOnly object.
   bool mIsDummy;

   /// Helper for 
   bool mBroken;
};

#endif // _PXMULTIACTOR_H

