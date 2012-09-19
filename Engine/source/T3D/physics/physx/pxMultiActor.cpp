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

#include "platform/platform.h"
#include "T3D/physics/physX/pxMultiActor.h"

#include "console/consoleTypes.h"
#include "core/stream/fileStream.h"
#include "core/stream/bitStream.h"
#include "core/resourceManager.h"
#include "core/strings/stringUnit.h"
#include "sim/netConnection.h"
#include "math/mathIO.h"
#include "math/mathUtils.h"
#include "gfx/gfxTransformSaver.h"
#include "gfx/gfxDrawUtil.h"
#include "gfx/primBuilder.h"
#include "collision/collision.h"
#include "collision/abstractPolyList.h"
#include "ts/tsShapeInstance.h"
#include "ts/tsPartInstance.h"
#include "lighting/lightManager.h"
#include "scene/sceneManager.h"
#include "scene/sceneRenderState.h"
#include "scene/sceneObjectLightingPlugin.h"
#include "T3D/objectTypes.h"
#include "T3D/containerQuery.h"
#include "T3D/fx/particleEmitter.h"
#include "T3D/debris.h"
#include "renderInstance/renderPassManager.h"
#include "gui/worldEditor/editor.h" // For gEditingMission
#include "T3D/physics/physX/px.h"
#include "T3D/physics/physX/pxWorld.h"
#include "T3D/physics/physX/pxMaterial.h"
#include "T3D/physics/physX/pxCasts.h"
#include "T3D/physics/physx/pxUtils.h"
#include "sfx/sfxSystem.h"

#include <NXU_helper.h>
#include <nxu_schema.h>
#include <NXU_customcopy.h>


class PxMultiActor_Notify : public NXU_userNotify
{
protected:

   Vector<NxActor*> mActors;

   Vector<NxShape*> mShapes;

   Vector<NxJoint*> mJoints;

   const NxMat34 mTransform;

   const Point3F mScale;

   F32 mMassScale;

   NxCompartment *mCompartment;

   PxMaterial *mMaterial;

   Vector<String> *mActorUserProperties;

   Vector<String> *mJointUserProperties;

public:

   void NXU_notifyJoint( NxJoint *joint, const char *userProperties )
   {
      if ( mJointUserProperties )
         mJointUserProperties->push_back( userProperties );
      mJoints.push_back( joint );
   }

   bool NXU_preNotifyJoint( NxJointDesc &joint, const char *userProperties )
   {
      joint.localAnchor[0].x *= mScale.x;
      joint.localAnchor[0].y *= mScale.y;
      joint.localAnchor[0].z *= mScale.z;

      joint.localAnchor[1].x *= mScale.x;
      joint.localAnchor[1].y *= mScale.y;
      joint.localAnchor[1].z *= mScale.z;

      // The PhysX exporter from 3dsMax doesn't allow creation
      // of fixed joints.  It also doesn't seem to export the
      // joint names!  So look for joints which all all the
      // motion axes are locked... make those fixed joints.
      if ( joint.getType() == NX_JOINT_D6 )
      {
         NxD6JointDesc *d6Joint = static_cast<NxD6JointDesc*>( &joint );

         if (  d6Joint->xMotion == NX_D6JOINT_MOTION_LOCKED &&
               d6Joint->yMotion == NX_D6JOINT_MOTION_LOCKED &&
               d6Joint->zMotion == NX_D6JOINT_MOTION_LOCKED &&
               d6Joint->swing1Motion == NX_D6JOINT_MOTION_LOCKED &&
               d6Joint->swing2Motion == NX_D6JOINT_MOTION_LOCKED &&
               d6Joint->twistMotion == NX_D6JOINT_MOTION_LOCKED )
         {
            // Ok... build a new fixed joint.
            NxFixedJointDesc fixed;
            fixed.actor[0] = joint.actor[0];
            fixed.actor[1] = joint.actor[1];
            fixed.localNormal[0] = joint.localNormal[0];
            fixed.localNormal[1] = joint.localNormal[1];
            fixed.localAxis[0] = joint.localAxis[0];
            fixed.localAxis[1] = joint.localAxis[1];
            fixed.localAnchor[0] = joint.localAnchor[0];
            fixed.localAnchor[1] = joint.localAnchor[1];
            fixed.maxForce = joint.maxForce;
            fixed.maxTorque = joint.maxTorque;
            fixed.name = joint.name;
            fixed.userData = joint.userData;
            fixed.jointFlags = joint.jointFlags;

            // What scene are we adding this to?
            NxActor *actor = fixed.actor[0] ? fixed.actor[0] : fixed.actor[1];
            NxScene &scene = actor->getScene();

            NxJoint* theJoint = scene.createJoint( fixed );
            mJoints.push_back( theJoint );
            if ( mJointUserProperties )
               mJointUserProperties->push_back( userProperties );

            // Don't generate this joint.
            return false;
         }
      }

      return true;
   }

   void NXU_notifyActor( NxActor *actor, const char *userProperties )
   {
      mActors.push_back( actor );

      // Save the shapes.
      for ( U32 i=0; i < actor->getNbShapes(); i++ )
         mShapes.push_back( actor->getShapes()[i] );

      mActorUserProperties->push_back( userProperties );
   };

   bool NXU_preNotifyMaterial( NxMaterialDesc &t, const char *userProperties )
   {
      // Don't generate materials if we have one defined!
      return !mMaterial;
   }

   bool NXU_preNotifyActor( NxActorDesc &actor, const char *userProperties )
   {
         // Set the right compartment.
         actor.compartment = mCompartment;

         if ( actor.shapes.size() == 0 )
            Con::warnf( "PxMultiActor_Notify::NXU_preNotifyActor, got an actor (%s) with no shapes, was this intentional?", actor.name );

         // For every shape, cast to its particular type
         // and apply the scale to size, mass and localPosition.
         for( S32 i = 0; i < actor.shapes.size(); i++ )
         {
            // If we have material then set it.
            if ( mMaterial )
               actor.shapes[i]->materialIndex = mMaterial->getMaterialId();

            switch( actor.shapes[i]->getType() )
            {
               case NX_SHAPE_BOX:
               {
                  NxBoxShapeDesc *boxDesc = (NxBoxShapeDesc*)actor.shapes[i];

                  boxDesc->mass *= mMassScale;

                  boxDesc->dimensions.x *= mScale.x;
                  boxDesc->dimensions.y *= mScale.y;
                  boxDesc->dimensions.z *= mScale.z;

                  boxDesc->localPose.t.x *= mScale.x;
                  boxDesc->localPose.t.y *= mScale.y;
                  boxDesc->localPose.t.z *= mScale.z;
                  break;
               }

               case NX_SHAPE_SPHERE:
               {
                  NxSphereShapeDesc *sphereDesc = (NxSphereShapeDesc*)actor.shapes[i];

                  sphereDesc->mass *= mMassScale;

                  // TODO: Spheres do not work with non-uniform
                  // scales very well... how do we fix this?
                  sphereDesc->radius *= mScale.x;

                  sphereDesc->localPose.t.x *= mScale.x;
                  sphereDesc->localPose.t.y *= mScale.y;
                  sphereDesc->localPose.t.z *= mScale.z;
                  break;
               }

               case NX_SHAPE_CAPSULE:
               {
                  NxCapsuleShapeDesc *capsuleDesc = (NxCapsuleShapeDesc*)actor.shapes[i];

                  capsuleDesc->mass *= mMassScale;

                  // TODO: Capsules do not work with non-uniform
                  // scales very well... how do we fix this?
                  capsuleDesc->radius *= mScale.x;
                  capsuleDesc->height *= mScale.y;

                  capsuleDesc->localPose.t.x *= mScale.x;
                  capsuleDesc->localPose.t.y *= mScale.y;
                  capsuleDesc->localPose.t.z *= mScale.z;
                  break;
               }

               default:
               {
                  static String lookup[] =
                  {
                     "PLANE",
                     "SPHERE",
                     "BOX",
                     "CAPSULE",
                     "WHEEL",
                     "CONVEX",
                     "MESH",
                     "HEIGHTFIELD"
                  };

                  Con::warnf( "PxMultiActor_Notify::NXU_preNotifyActor, unsupported shape type (%s), on Actor (%s)", lookup[actor.shapes[i]->getType()].c_str(), actor.name );

                  delete actor.shapes[i];
                  actor.shapes.erase( actor.shapes.begin() + i );
                  --i;
                  break;
               }
            }
         }

         NxBodyDesc *body = const_cast<NxBodyDesc*>( actor.body );
         if ( body )
         {
            // Must scale all of these parameters, else there will be odd results!
            body->mass *= mMassScale;
            body->massLocalPose.t.multiply( mMassScale, body->massLocalPose.t );
            body->massSpaceInertia.multiply( mMassScale, body->massSpaceInertia );

            // Ragdoll damping!
            //body->sleepDamping = 1.7f;
            //body->linearDamping = 0.4f;
            //body->angularDamping = 0.08f;
            //body->wakeUpCounter = 0.3f;
         }

      return   true;
   };

public:

   PxMultiActor_Notify(    NxCompartment *compartment,
                           PxMaterial *material,
                           const NxMat34& mat,
                           const Point3F& scale,
                           Vector<String> *actorProps = NULL,
                           Vector<String> *jointProps = NULL )
      :  mCompartment( compartment ),
         mMaterial( material ),
         mScale( scale ),
         mTransform( mat ),
         mActorUserProperties( actorProps ),
         mJointUserProperties( jointProps )
   {
      const F32 unit = VectorF( 1.0f, 1.0f, 1.0f ).len();
      mMassScale = mScale.len() / unit;
   }

   virtual ~PxMultiActor_Notify()
   {
   }

   const Vector<NxActor*>& getActors() { return mActors; }
   const Vector<NxShape*>& getShapes() { return mShapes; }
   const Vector<NxJoint*>& getJoints() { return mJoints; }
};

ConsoleDocClass( PxMultiActorData,
   
   "@brief Defines the properties of a type of PxMultiActor.\n\n"

   "Usually it is prefered to use PhysicsShape rather than PxMultiActor because "
   "a PhysicsShape is not PhysX specific and can be much easier to setup.\n\n"

   "For more information, refer to Nvidia's PhysX docs.\n\n"
   
   "@ingroup Physics"
);

IMPLEMENT_CO_DATABLOCK_V1(PxMultiActorData);

PxMultiActorData::PxMultiActorData()
 : material( NULL ),
   collection( NULL ),
   waterDragScale( 1.0f ),
   buoyancyDensity( 1.0f ),
   angularDrag( 0.0f ),
   linearDrag( 0.0f ),
   clientOnly( false ),
   singlePlayerOnly( false ),
   shapeName( StringTable->insert( "" ) ),
   physXStream( StringTable->insert( "" ) ),
   breakForce( 0.0f )
{
   for ( S32 i = 0; i < MaxCorrectionNodes; i++ )
      correctionNodeNames[i] = StringTable->insert( "" );

   for ( S32 i = 0; i < MaxCorrectionNodes; i++ )
      correctionNodes[i] = -1;

   for ( S32 i = 0; i < NumMountPoints; i++ )
   {
      mountNodeNames[i] = StringTable->insert( "" );
      mountPointNode[i] = -1;
   }
}

PxMultiActorData::~PxMultiActorData()
{
   if ( collection )
      NXU::releaseCollection( collection );
}

void PxMultiActorData::initPersistFields()
{
   Parent::initPersistFields();

   addGroup("Media");
      addField( "shapeName", TypeFilename, Offset( shapeName, PxMultiActorData ),
         "@brief Path to the .DAE or .DTS file to render.\n\n");
   endGroup("Media");

   // PhysX collision properties.
   addGroup( "Physics" );

      addField( "physXStream", TypeFilename, Offset( physXStream, PxMultiActorData ),
         "@brief .XML file containing data such as actors, shapes, and joints.\n\n"
         "These files can be created using a free PhysX plugin for 3DS Max.\n\n");
      addField( "material", TYPEID< PxMaterial >(), Offset( material, PxMultiActorData ),
         "@brief An optional PxMaterial to be used for the PxMultiActor.\n\n"
         "Defines properties such as friction and restitution. "
         "Unrelated to the material used for rendering. The physXStream will contain "
         "defined materials that can be customized in 3DS Max. "
         "To override the material for all physics shapes in the physXStream, specify a material here.\n\n");

      addField( "noCorrection", TypeBool, Offset( noCorrection, PxMultiActorData ),
         "@hide" );

      UTF8 buff[256];
      for ( S32 i=0; i < MaxCorrectionNodes; i++ )
      {
         //dSprintf( buff, sizeof(buff), "correctionNode%d", i );
         addField( buff, TypeString, Offset( correctionNodeNames[i], PxMultiActorData ), "@hide" );
      }

      for ( S32 i=0; i < NumMountPoints; i++ )
      {
         //dSprintf( buff, sizeof(buff), "mountNode%d", i );
         addField( buff, TypeString, Offset( mountNodeNames[i], PxMultiActorData ), "@hide" );
      }

      addField( "angularDrag", TypeF32, Offset( angularDrag, PxMultiActorData ),
         "@brief Value used to help calculate rotational drag force while submerged in water.\n\n");
      addField( "linearDrag", TypeF32, Offset( linearDrag, PxMultiActorData ),
         "@brief Value used to help calculate linear drag force while submerged in water.\n\n");
      addField( "waterDragScale", TypeF32, Offset( waterDragScale, PxMultiActorData ),
         "@brief Scale to apply to linear and angular dampening while submerged in water.\n\n ");
      addField( "buoyancyDensity", TypeF32, Offset( buoyancyDensity, PxMultiActorData ),
         "@brief The density used to calculate buoyant forces.\n\n"
         "The result of the calculated buoyancy is relative to the density of the WaterObject the PxMultiActor is within.\n\n"
         "@note This value is necessary because Torque 3D does its own buoyancy simulation. It is not handled by PhysX."
         "@see WaterObject::density");

   endGroup( "Physics" );

   addField( "clientOnly", TypeBool, Offset( clientOnly, PxMultiActorData ),
      "@hide");
   addField( "singlePlayerOnly", TypeBool, Offset( singlePlayerOnly, PxMultiActorData ),
      "@hide");
   addField( "breakForce", TypeF32, Offset( breakForce, PxMultiActorData ),
      "@brief Force required to break an actor.\n\n"
      "This value does not apply to joints. "
      "If an actor is associated with a joint it will break whenever the joint does. "
      "This allows an actor \"not\" associated with a joint to also be breakable.\n\n");
}

void PxMultiActorData::packData(BitStream* stream)
{
   Parent::packData(stream);

   stream->writeString( shapeName );
   stream->writeString( physXStream );

   if( stream->writeFlag( material ) )
      stream->writeRangedU32( packed ? SimObjectId( material ) : material->getId(),
                              DataBlockObjectIdFirst,  DataBlockObjectIdLast );

   if ( !stream->writeFlag( noCorrection ) )
   {
      // Write the correction node indices for the client.
      for ( S32 i = 0; i < MaxCorrectionNodes; i++ )
         stream->write( correctionNodes[i] );
   }

   for ( S32 i = 0; i < NumMountPoints; i++ )
      stream->write( mountPointNode[i] );

   stream->write( waterDragScale );
   stream->write( buoyancyDensity );
   stream->write( angularDrag );
   stream->write( linearDrag );

   stream->writeFlag( clientOnly );
   stream->writeFlag( singlePlayerOnly );
   stream->write( breakForce );
}

void PxMultiActorData::unpackData(BitStream* stream)
{
   Parent::unpackData(stream);

   shapeName = stream->readSTString();
   physXStream = stream->readSTString();

   if( stream->readFlag() )
      material = (PxMaterial*)stream->readRangedU32( DataBlockObjectIdFirst, DataBlockObjectIdLast );

   noCorrection = stream->readFlag();
   if ( !noCorrection )
   {
      for ( S32 i = 0; i < MaxCorrectionNodes; i++ )
         stream->read( &correctionNodes[i] );
   }

   for ( S32 i = 0; i < NumMountPoints; i++ )
      stream->read( &mountPointNode[i] );

   stream->read( &waterDragScale );
   stream->read( &buoyancyDensity );
   stream->read( &angularDrag );
   stream->read( &linearDrag );

   clientOnly = stream->readFlag();
   singlePlayerOnly = stream->readFlag();
   stream->read( &breakForce );
}

bool PxMultiActorData::preload( bool server, String &errorBuffer )
{
   if ( !Parent::preload( server, errorBuffer ) )
      return false;

   // If the stream is null, exit.
   if ( !physXStream || !physXStream[0] )
   {
      errorBuffer = "PxMultiActorData::preload: physXStream is unset!";
      return false;
   }

   // Set up our buffer for the binary stream filename path.
   UTF8 binPhysXStream[260] = { 0 };
   const UTF8* ext = dStrrchr( physXStream, '.' );

   // Copy the xml stream path except for the extension.
   if ( ext )
      dStrncpy( binPhysXStream, physXStream, getMin( 260, ext - physXStream ) );
   else
      dStrncpy( binPhysXStream, physXStream, 260 );

   // Concatenate the binary extension.
   dStrcat( binPhysXStream, ".nxb" );

   // Get the modified times of the two files.
   FileTime xmlTime = {0}, binTime = {0};
   Platform::getFileTimes( physXStream, NULL, &xmlTime );
   Platform::getFileTimes( binPhysXStream, NULL, &binTime );

   // If the binary is newer... load that.
   if ( Platform::compareFileTimes( binTime, xmlTime ) >= 0 )
      _loadCollection( binPhysXStream, true );

   // If the binary failed... then load the xml.
   if ( !collection )
   {
      _loadCollection( physXStream, false );

      // If loaded... resave the xml in binary format
      // for quicker subsequent loads.
      if ( collection )
         NXU::saveCollection( collection, binPhysXStream, NXU::FT_BINARY );
   }

   // If it still isn't loaded then we've failed!
   if ( !collection )
   {
      errorBuffer = String::ToString( "PxMultiActorDatas::preload: could not load '%s'!", physXStream );
      return false;
   }

   if (!shapeName || shapeName == '\0')
   {
      errorBuffer = "PxMultiActorDatas::preload: no shape name!";
      return false;
   }

   shape = ResourceManager::get().load( shapeName );

   if (bool(shape) == false)
   {
      errorBuffer = String::ToString( "PxMultiActorData::preload: unable to load shape: %s", shapeName );
      return false;
   }

   // Find the client side material.
   if ( !server && material )
      Sim::findObject( SimObjectId(material), material );

   // Get the ignore node indexes from the names.
   for ( S32 i = 0; i < MaxCorrectionNodes; i++ )
   {
      if( !correctionNodeNames[i] || !correctionNodeNames[i][0] )
         continue;

      correctionNodes[i] = shape->findNode( correctionNodeNames[i] );
   }

   // Resolve mount point node indexes
   for ( S32 i = 0; i < NumMountPoints; i++) 
   {
      char fullName[256];

      if ( !mountNodeNames[i] || !mountNodeNames[i][0] )
      {
         dSprintf(fullName,sizeof(fullName),"mount%d",i);
         mountPointNode[i] = shape->findNode(fullName);
      }  
      else      
         mountPointNode[i] = shape->findNode(mountNodeNames[i]);            
   }

   // Register for file change notification to reload the collection
   if ( server )
      FS::AddChangeNotification( physXStream, this, &PxMultiActorData::_onFileChanged );

   return true;
}

void PxMultiActorData::_onFileChanged( const Torque::Path &path )
{
   reload();
}

void PxMultiActorData::reload()
{
   bool result = _loadCollection( physXStream, false );

   if ( !result )
      Con::errorf( "PxMultiActorData::reload(), _loadCollection failed..." );

   // Inform MultiActors who use this datablock to reload.
   mReloadSignal.trigger();
}

bool PxMultiActorData::_loadCollection( const UTF8 *path, bool isBinary )
{
   if ( collection )
   {
      NXU::releaseCollection( collection );
      collection = NULL;
   }

   FileStream fs;
   if ( !fs.open( path, Torque::FS::File::Read ) )
      return false;

   // Load the data into memory.
   U32 size = fs.getStreamSize();
   FrameTemp<U8> buff( size );
   fs.read( size, buff );

   // If the stream didn't read anything, there's a problem.
   if ( size <= 0 )
      return false;

   // Ok... try to load it.
   collection = NXU::loadCollection(   path,
                                       isBinary ? NXU::FT_BINARY : NXU::FT_XML,
                                       buff,
                                       size );

   return collection != NULL;
}


bool PxMultiActorData::createActors(   NxScene *scene,
                                          NxCompartment *compartment,
                                          const NxMat34 *nxMat,
                                          const Point3F& scale,
                                          Vector<NxActor*> *outActors,
                                          Vector<NxShape*> *outShapes,
                                          Vector<NxJoint*> *outJoints,
                                          Vector<String> *outActorUserProperties,
                                          Vector<String> *outJointUserProperties )
{
   if ( !scene )
   {
      Con::errorf( "PxMultiActorData::createActor() - returned null NxScene" );
      return NULL;
   }

   PxMultiActor_Notify pxNotify( compartment, material, *nxMat, scale, outActorUserProperties, outJointUserProperties );

   NXU::instantiateCollection( collection, *gPhysicsSDK, scene, nxMat, &pxNotify );

   *outActors = pxNotify.getActors();
   *outJoints = pxNotify.getJoints();
   if ( outShapes )
      *outShapes = pxNotify.getShapes();

   if ( outActors->empty() )
   {
      Con::errorf( "PxMultiActorData::createActors() - NXUStream notifier returned empty actors or joints!" );
      return false;
   }

   return true;
}

ConsoleDocClass( PxMultiActor,
   
   "@brief Represents a destructible physical object simulated using PhysX.\n\n"

   "Usually it is prefered to use PhysicsShape and not PxMultiActor because "
   "it is not PhysX specific and much easier to setup.\n"
   
   "@see PxMultiActorData.\n"   
   "@ingroup Physics"
);

IMPLEMENT_CO_NETOBJECT_V1(PxMultiActor);

PxMultiActor::PxMultiActor()
 : mShapeInstance( NULL ),
   mRootActor( NULL ),
   mWorld( NULL ),
   mStartImpulse( 0, 0, 0 ),
   mResetXfm( true ),
   mActorScale( 0, 0, 0 ),
   mDebugRender( false ),
   mIsDummy( false ),
   mBroken( false ),
   mDataBlock( NULL )
{
   mNetFlags.set( Ghostable | ScopeAlways );

   mTypeMask |= StaticObjectType | StaticShapeObjectType;

   //mUserData.setObject( this );
}

void PxMultiActor::initPersistFields()
{
   Parent::initPersistFields();

   /*
   // We're overloading these fields from SceneObject
   // in order to force it to go thru setTransform!
   removeField( "position" );
   removeField( "rotation" );
   removeField( "scale" );

   addGroup( "Transform" );

      addProtectedField( "position", TypeMatrixPosition, 0,
         &PxMultiActor::_setPositionField,
         &PxMultiActor::_getPositionField,
         "" );

      addProtectedField( "rotation", TypeMatrixRotation, 0,
         &PxMultiActor::_setRotationField,
         &PxMultiActor::_getRotationField,
         "" );

      addField( "scale", TypePoint3F, Offset( mObjScale, PxMultiActor ) );

   endGroup( "Transform" );
   */

   //addGroup("Physics");
   //   addField( "AngularDrag", TypeF32, )
   //endGroup("Physics");

   addGroup( "Debug" );
      addField( "debugRender", TypeBool, Offset( mDebugRender, PxMultiActor ), "@hide");
      addField( "broken", TypeBool, Offset( mBroken, PxMultiActor ), "@hide");
   endGroup( "Debug" );

   //addGroup("Collision");
   //endGroup("Collision");
}

bool PxMultiActor::onAdd()
{
   PROFILE_SCOPE( PxMultiActor_OnAdd );

   if (!Parent::onAdd() || !mDataBlock )
      return false;

   mIsDummy = isClientObject() && PHYSICSMGR->isSinglePlayer(); //&& mDataBlock->singlePlayerOnly;

   mShapeInstance = new TSShapeInstance( mDataBlock->shape, isClientObject() );

   mObjBox = mDataBlock->shape->bounds;
   resetWorldBox();

   addToScene();

   String worldName = isServerObject() ? "server" : "client";

   // SinglePlayer objects only have server-side physics representations.
   if ( mIsDummy )
      worldName = "server";

   mWorld = dynamic_cast<PxWorld*>( PHYSICSMGR->getWorld( worldName ) );
   if ( !mWorld || !mWorld->getScene() )
   {
      Con::errorf( "PxMultiActor::onAdd() - PhysXWorld not initialized!" );
      return false;
   }

   applyWarp( getTransform(), true, false );
   mResetXfm = getTransform();

   if ( !_createActors( getTransform() ) )
   {
      Con::errorf( "PxMultiActor::onAdd(), _createActors failed" );
      return false;
   }

   if ( !mIsDummy )
      mDataBlock->mReloadSignal.notify( this, &PxMultiActor::onFileNotify );

   // If the editor is on... let it know!
   //if ( gEditingMission )
      //onEditorEnable(); // TODO: Fix this up.

   PhysicsPlugin::getPhysicsResetSignal().notify( this, &PxMultiActor::onPhysicsReset, 1050.0f );

   setAllBroken( false );

   if ( isServerObject() )
      scriptOnAdd();

   return true;
}


void PxMultiActor::onRemove()
{
   removeFromScene();

   _destroyActors();
   mWorld = NULL;

   SAFE_DELETE( mShapeInstance );

   PhysicsPlugin::getPhysicsResetSignal().remove( this, &PxMultiActor::onPhysicsReset );

   if ( !mIsDummy && mDataBlock )
      mDataBlock->mReloadSignal.remove( this, &PxMultiActor::onFileNotify );

   Parent::onRemove();
}

void PxMultiActor::_destroyActors()
{
   // Dummies don't have physics objects.
   if ( mIsDummy || !mWorld )
      return;

   mWorld->releaseWriteLock();

   // Clear the root actor.
   mRootActor = NULL;

   // Clear the relative transforms.
   //mRelXfms.clear();

   // The shapes are owned by the actors, so
   // we just need to clear them.
   mShapes.clear();

   // Release the joints first.
   for( S32 i = 0; i < mJoints.size(); i++ )
   {
      NxJoint *joint = mJoints[i];

      if ( !joint )
         continue;

      // We allocate per joint userData and we must free it.
      PxUserData *jointData = PxUserData::getData( *joint );
      if ( jointData )
         delete jointData;

      mWorld->releaseJoint( *joint );
   }
   mJoints.clear();

   // Now release the actors.
   for( S32 i = 0; i < mActors.size(); i++ )
   {
      NxActor *actor = mActors[i];

      PxUserData *actorData = PxUserData::getData( *actor );
      if ( actorData )
         delete actorData;

      if ( actor )
         mWorld->releaseActor( *actor );
   }
   mActors.clear();
}

bool PxMultiActor::_createActors( const MatrixF &xfm )
{
   if ( mIsDummy )
   {
      // Dummies don't have physics objects, but
      // they do handle actor deltas.

      PxMultiActor *serverObj = static_cast<PxMultiActor*>( mServerObject.getObject() );
      mActorDeltas.setSize( serverObj->mActors.size() );
      dMemset( mActorDeltas.address(), 0, mActorDeltas.memSize() );

      return true;
   }

   NxMat34 nxMat;
   nxMat.setRowMajor44( xfm );

   // Store the scale for comparison in setScale().
   mActorScale = getScale();

   // Release the write lock so we can create actors.
   mWorld->releaseWriteLock();

   Vector<String> actorUserProperties;
   Vector<String> jointUserProperties;
   bool created = mDataBlock->createActors(  mWorld->getScene(),
                                             NULL,
                                             &nxMat,
                                             mActorScale,
                                             &mActors,
                                             &mShapes,
                                             &mJoints,
                                             &actorUserProperties,
                                             &jointUserProperties  );

   // Debug output...
   //for ( U32 i = 0; i < mJoints.size(); i++ )
   //   Con::printf( "Joint0 name: '%s'", mJoints[i]->getName() );
   //for ( U32 i = 0; i < actorUserProperties.size(); i++ )
      //Con::printf( "actor%i UserProperties: '%s'", i, actorUserProperties[i].c_str() );
   //for ( U32 i = 0; i < jointUserProperties.size(); i++ )
     // Con::printf( "joint%i UserProperties: '%s'", i, jointUserProperties[i].c_str() );

   if ( !created )
   {
      Con::errorf( "PxMultiActor::_createActors() - failed!" );
      return false;
   }

   // Make the first actor the root actor by default, but
   // if we have a kinematic actor then use that.
   mRootActor = mActors[0];
   for ( S32 i = 0; i < mActors.size(); i++ )
   {
      if ( mActors[i]->readBodyFlag( NX_BF_KINEMATIC ) )
      {
         mRootActor = mActors[i];
         break;
      }
   }

   mDelta.pos = mDelta.lastPos = getPosition();
   mDelta.rot = mDelta.lastRot = getTransform();

   bool *usedActors = new bool[mActors.size()];
   dMemset( usedActors, 0, sizeof(bool) * mActors.size() );

   TSShape *shape = mShapeInstance->getShape();

   // Should already be done when actors are destroyed.
   mMappedActors.clear();
   Vector<String> mappedActorProperties;

   // Remap the actors to the shape instance's bone indices.
   for( S32 i = 0; i < mShapeInstance->mNodeTransforms.size(); i++ )
   {
      if ( !shape )
         break;

      UTF8 comparisonName[260] = { 0 };
      NxActor *actor = NULL;
      NxActor *pushActor = NULL;
      String actorProperties;

      S32 nodeNameIdx = shape->nodes[i].nameIndex;
      const UTF8 *nodeName = shape->getName( nodeNameIdx );

      S32 dl = -1;
      dStrcpy( comparisonName, String::GetTrailingNumber( nodeName, dl ) ); //, ext - nodeName );
      dSprintf( comparisonName, sizeof( comparisonName ), "%s_pxactor", comparisonName );

      //String test( nodeName );
      //AssertFatal( test.find("gableone",0,String::NoCase) == String::NPos, "found it" );

      // If we find an actor that corresponds to this node we will
      // push it back into the remappedActors vector, otherwise
      // we will push back NULL.
      for ( S32 j = 0; j < mActors.size(); j++ )
      {
         actor = mActors[j];
         const UTF8 *actorName = actor->getName();

         if ( dStricmp( comparisonName, actorName ) == 0 )
         {
            pushActor = actor;
            actorProperties = actorUserProperties[j];
            usedActors[j] = true;
            break;
         }
      }

      mMappedActors.push_back( pushActor );
      mappedActorProperties.push_back( actorProperties );
      if ( !pushActor )
         dl = -1;
      mMappedActorDL.push_back( dl );

      // Increase the sleep tolerance.
      if ( pushActor )
      {
         //pushActor->raiseBodyFlag( NX_BF_ENERGY_SLEEP_TEST );
         //pushActor->setSleepEnergyThreshold( 2 );
         //pushActor->userData = NULL;
      }
   }

   // Delete any unused/orphaned actors.
   for ( S32 i = 0; i < mActors.size(); i++ )
   {
      if ( usedActors[i] )
         continue;

      NxActor *actor = mActors[i];

      Con::errorf( "PxMultiActor::_createActors() - Orphan NxActor - '%s'!", actor->getName() );

      if ( actor == mRootActor )
      {
         Con::errorf( "PxMultiActor::_createActors() - root actor (%s) was orphan, cannot continue.", actor->getName() );
         return false;
      }

      // Remove references to shapes of the deleted actor.
      for ( S32 i = 0; i < mShapes.size(); i++ )
      {
         if ( &(mShapes[i]->getActor()) == actor )
         {
            mShapes.erase_fast(i);
            i--;
         }
      }

      mWorld->releaseActor( *actor );
   }

   // Done with this helper.
   delete [] usedActors;

   // Repopulate mActors with one entry per real actor we own.
   mActors.clear();
   mMappedToActorIndex.clear();
   actorUserProperties.clear();
   for ( S32 i = 0; i < mMappedActors.size(); i++ )
   {
      S32 index = -1;
      if ( mMappedActors[i] )
      {
         index = mActors.push_back_unique( mMappedActors[i] );
         while ( index >= actorUserProperties.size() )
            actorUserProperties.push_back( String::EmptyString );
         actorUserProperties[index] = mappedActorProperties[i];
      }
      mMappedToActorIndex.push_back( index );
   }

   if ( mActors.size() == 0 )
   {
      Con::errorf( "PxMultiActor::_createActors, got zero actors! Were all actors orphans?" );
      return false;
   }

   // Initialize the actor deltas.
   mActorDeltas.setSize( mActors.size() );
   dMemset( mActorDeltas.address(), 0, mActorDeltas.memSize() );

   // Assign user data for actors.
   for ( U32 i = 0; i < mActors.size(); i++ )
   {
      NxActor *actor = mActors[i];
      if ( !actor )
         continue;

      actor->userData = _createActorUserData( actor, actorUserProperties[i] );
   }

   //NxActor *actor1;
   //NxActor *actor2;
   //PxUserData *pUserData;

   // Allocate user data for joints.
   for ( U32 i = 0; i < mJoints.size(); i++ )
   {
      NxJoint *joint = mJoints[i];
      if ( !joint )
         continue;

      joint->userData = _createJointUserData( joint, jointUserProperties[i] );

      /*
      // Set actors attached to joints as not-pushable (by the player).
      joint->getActors( &actor1, &actor2 );
      if ( actor1 )
      {
         pUserData = PxUserData::getData( *actor1 );
         if ( pUserData )
            pUserData->mCanPush = false;
      }
      if ( actor2 )
      {
         pUserData = PxUserData::getData( *actor2 );
         if ( pUserData )
            pUserData->mCanPush = false;
      }
      */
   }

   // Set actors and meshes to the unbroken state.
   setAllBroken( false );

   return true;
}

PxUserData* PxMultiActor::_createActorUserData( NxActor *actor, String &userProperties )
{
   PxUserData *actorData = new PxUserData();
   actorData->setObject( this );

   // We use this for saving relative xfms for 'broken' actors.
   NxMat34 actorPose = actor->getGlobalPose();
   NxMat34 actorSpaceXfm;
   actorPose.getInverse( actorSpaceXfm );

   const String actorName( actor->getName() );

   static const String showStr( "PxBrokenShow" );
   static const String hideStr( "PxBrokenHide" );

   // 3DSMax saves out double newlines, replace them with one.
   userProperties.replace( "\r\n", "\n" );

   U32 propertyCount = StringUnit::getUnitCount( userProperties, "\n" );
   for ( U32 i = 0; i < propertyCount; i++ )
   {
      String propertyStr = StringUnit::getUnit( userProperties, i, "\n" );
      U32 wordCount = StringUnit::getUnitCount( propertyStr, "=" );

      if ( wordCount == 0 )
      {
         // We sometimes get empty lines between properties,
         // which doesn't break anything.
         continue;
      }

      if ( wordCount != 2 )
      {
         Con::warnf( "PxMultiActor::_createActorUserData, malformed UserProperty string (%s) for actor (%s)", propertyStr.c_str(), actorName.c_str() );
         continue;
      }

      String propertyName = StringUnit::getUnit( propertyStr, 0, "=" );
      String propertyValue = StringUnit::getUnit( propertyStr, 1, "=" );

      Vector<NxActor*> *dstVector = NULL;
      if ( propertyName.equal( showStr, String::NoCase ) )
         dstVector = &actorData->mBrokenActors;
      else if ( propertyName.equal( hideStr, String::NoCase ) )
         dstVector = &actorData->mUnbrokenActors;

      if ( !dstVector )
         continue;

      U32 valueCount = StringUnit::getUnitCount( propertyValue, "," );
      for ( U32 j = 0; j < valueCount; j++ )
      {
         String val = StringUnit::getUnit( propertyValue, j, "," );

         NxActor *pActor = _findActor( val );
         if ( !pActor )
            Con::warnf( "PxMultiActor::_createActorUserData, actor (%s) was not found when parsing UserProperties for actor (%s)", val.c_str(), actorName.c_str() );
         else
         {
            dstVector->push_back( pActor );

            if ( dstVector == &actorData->mBrokenActors )
            {
               NxMat34 relXfm = pActor->getGlobalPose();
               relXfm.multiply( relXfm, actorSpaceXfm );
               actorData->mRelXfm.push_back( relXfm );
            }
         }
      }
   }

   // Only add a contact signal to this actor if
   // we have objects we can break.
   if (  actorData->mBrokenActors.size() > 0 &&
         mDataBlock->breakForce > 0.0f )
   {
      actor->setContactReportFlags( NX_NOTIFY_ON_START_TOUCH_FORCE_THRESHOLD | NX_NOTIFY_FORCES );
      actor->setContactReportThreshold( mDataBlock->breakForce );
      actorData->getContactSignal().notify( this, &PxMultiActor::_onContact );
   }

   return actorData;
}

PxUserData* PxMultiActor::_createJointUserData( NxJoint *joint, String &userProperties )
{
   PxUserData *jointData = new PxUserData();
   jointData->setObject( this );

   // We use this for saving relative xfms for 'broken' actors.
   NxActor *actor0;
   NxActor *actor1;
   joint->getActors( &actor0, &actor1 );
   NxMat34 actorPose = actor0->getGlobalPose();
   NxMat34 actorSpaceXfm;
   actorPose.getInverse( actorSpaceXfm );

   // The PxMultiActor will live longer than the joint
   // so this notify shouldn't ever need to be removed. Although if someone
   // other than this multiactor were to register for this notify and their
   // lifetime could be shorter, then 'they' might have to.
   jointData->getOnJointBreakSignal().notify( this, &PxMultiActor::_onJointBreak );

   // JCFHACK: put this in userProperties too.
   Sim::findObject( "JointBreakEmitter", jointData->mParticleEmitterData );

   String showStr( "PxBrokenShow" );
   String hideStr( "PxBrokenHide" );

   // Max saves out double newlines, replace them with one.
   userProperties.replace( "\r\n", "\n" );

   U32 propertyCount = StringUnit::getUnitCount( userProperties, "\n" );
   for ( U32 i = 0; i < propertyCount; i++ )
   {
      String propertyStr = StringUnit::getUnit( userProperties, i, "\n" );
      U32 wordCount = StringUnit::getUnitCount( propertyStr, "=" );

      if ( wordCount == 0 )
      {
         // We sometimes get empty lines between properties,
         // which doesn't break anything.
         continue;
      }

      if ( wordCount != 2 )
      {
         Con::warnf( "PxMultiActor::_createJointUserData, malformed UserProperty string (%s) for joint (%s)", propertyStr.c_str(), joint->getName() );
         continue;
      }

      String propertyName = StringUnit::getUnit( propertyStr, 0, "=" );
      String propertyValue = StringUnit::getUnit( propertyStr, 1, "=" );

      Vector<NxActor*> *dstVector = NULL;
      if ( propertyName.equal( showStr, String::NoCase ) )
         dstVector = &jointData->mBrokenActors;
      else if ( propertyName.equal( hideStr, String::NoCase ) )
         dstVector = &jointData->mUnbrokenActors;

      if ( !dstVector )
         continue;

      U32 valueCount = StringUnit::getUnitCount( propertyValue, "," );
      for ( U32 j = 0; j < valueCount; j++ )
      {
         String val = StringUnit::getUnit( propertyValue, j, "," );

         NxActor *pActor = _findActor( val );
         if ( !pActor )
            Con::warnf( "PxMultiActor::_createJointUserData, actor (%s) was not found when parsing UserProperties for joint (%s)", val.c_str(), joint->getName() );
         else
         {
            dstVector->push_back( pActor );

            if ( dstVector == &jointData->mBrokenActors )
            {
               NxMat34 relXfm = pActor->getGlobalPose();
               relXfm.multiply( relXfm, actorSpaceXfm );
               jointData->mRelXfm.push_back( relXfm );
            }
         }
      }
   }

   return jointData;
}

NxActor* PxMultiActor::_findActor( const String &actorName ) const
{
   for ( U32 i = 0; i < mActors.size(); i++ )
   {
      NxActor *actor = mActors[i];
      if ( !actor )
         continue;

      if ( dStricmp( actor->getName(), actorName ) == 0 )
         return actor;
   }

   return NULL;
}

String PxMultiActor::_getMeshName( const NxActor *actor ) const
{
   String meshName = actor->getName();
   meshName.replace( "_pxactor", "" );
   //meshName = StringUnit::getUnit( meshName, 0, "_" );
   return meshName;
}

bool PxMultiActor::onNewDataBlock( GameBaseData *dptr, bool reload )
{
   mDataBlock = dynamic_cast<PxMultiActorData*>(dptr);

   if ( !mDataBlock || !Parent::onNewDataBlock( dptr, reload ) )
      return false;

   // JCF: if we supported it, we would recalculate the value of mIsDummy now,
   // but that would really hose everything since an object that was a dummy
   // wouldn't have any actors and would need to create them, etc...

   scriptOnNewDataBlock();

   return true;
}

void PxMultiActor::inspectPostApply()
{
   // Make sure we call the parent... else
   // we won't get transform and scale updates!
   Parent::inspectPostApply();

   //setMaskBits( LightMask );
   setMaskBits( UpdateMask );
}

void PxMultiActor::onStaticModified( const char *slotName, const char *newValue )
{
   if ( isProperlyAdded() && dStricmp( slotName, "broken" ) == 0 )
      setAllBroken( dAtob(newValue) );
}

void PxMultiActor::onDeleteNotify( SimObject *obj )
{
   Parent::onDeleteNotify(obj);
   if ( obj == mMount.object )
      unmount();
}

void PxMultiActor::onFileNotify()
{
   // Destroy the existing actors and recreate them...

   mWorld->getPhysicsResults();
   _destroyActors();
   _createActors( mResetXfm );
}

void PxMultiActor::onPhysicsReset( PhysicsResetEvent reset )
{
   // Dummies don't create or destroy actors, they just reuse the
   // server object's ones.
   if ( mIsDummy )
      return;

   // Store the reset transform for later use.
   if ( reset == PhysicsResetEvent_Store )
   {
      mRootActor->getGlobalPose().getRowMajor44( mResetXfm );
   }
   else if ( reset == PhysicsResetEvent_Restore )
   {
      // Destroy the existing actors and recreate them to
      // ensure they are in the proper mission startup state.
      mWorld->getPhysicsResults();

      _destroyActors();
      _createActors( mResetXfm );
   }

   for ( U32 i = 0; i < mActors.size(); i++ )
   {
      if ( !mActors[i] )
         continue;

      mActors[i]->wakeUp();
   }
}

void PxMultiActor::prepRenderImage( SceneRenderState *state )
{
   PROFILE_SCOPE( PxMultiActor_PrepRenderImage );

   if ( !mShapeInstance )
      return;

   Point3F cameraOffset;
   getTransform().getColumn(3,&cameraOffset);
   cameraOffset -= state->getDiffuseCameraPosition();
   F32 dist = cameraOffset.len();
   if ( dist < 0.01f )
      dist = 0.01f;

   F32 invScale = (1.0f/getMax(getMax(mObjScale.x,mObjScale.y),mObjScale.z));

   S32 dl = mShapeInstance->setDetailFromDistance( state, dist * invScale );
   if ( dl < 0 )
      return;

   GFXTransformSaver saver;

   // Set up our TS render state here.
   TSRenderState rdata;
   rdata.setSceneState( state );

   // We might have some forward lit materials
   // so pass down a query to gather lights.
   LightQuery query;
   query.init( getWorldSphere() );
   rdata.setLightQuery( &query );

   MatrixF mat = getRenderTransform();
   mat.scale( getScale() );
   GFX->setWorldMatrix( mat );

   if ( mDebugRender || Con::getBoolVariable( "$PxDebug::render", false ) )
   {
      ObjectRenderInst *ri = state->getRenderPass()->allocInst<ObjectRenderInst>();
      ri->renderDelegate.bind( this, &PxMultiActor::_debugRender );
      ri->type = RenderPassManager::RIT_Object;
      state->getRenderPass()->addInst( ri );
   }
   else
      mShapeInstance->render( rdata );
}

void PxMultiActor::_debugRender( ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance *overrideMat )
{
   if ( mShapeInstance )
   {
      GFXTransformSaver saver;

      MatrixF mat = getRenderTransform();
      mat.scale( mObjScale );
      GFX->multWorld( mat );

      //mShapeInstance->renderDebugNodes();
   }

   Vector<NxActor*> *actors = &mActors;
   if ( mIsDummy )
   {
      PxMultiActor *serverObj = static_cast<PxMultiActor*>( mServerObject.getObject() );
      if ( serverObj )
         actors = &serverObj->mActors;
   }

   if ( !actors )
      return;

   for ( U32 i = 0; i < actors->size(); i++ )
   {
      NxActor *pActor = (*actors)[i];
      if ( !pActor )
         continue;

      PxUtils::drawActor( pActor );
   }
}

void PxMultiActor::_onJointBreak( NxReal breakForce, NxJoint &brokenJoint )
{
   // Dummies do not have physics objects
   // and shouldn't receive this callback.
   if ( mIsDummy )
      return;

   NxActor *actor0 = NULL;
   NxActor *actor1 = NULL;
   brokenJoint.getActors( &actor0, &actor1 );
   NxMat34 parentPose = actor0->getGlobalPose();

   Point3F jointPos = pxCast<Point3F>( brokenJoint.getGlobalAnchor() );

   PxUserData *jointData = PxUserData::getData( brokenJoint );
   setBroken( parentPose, NxVec3( 0.0f ), jointData, true );

   // NOTE: We do not NULL the joint in the list,
   // or release it here, as we allow it to be released
   // by the _destroyActors function on a reset or destruction
   // of the PxMultiActor.
}

void PxMultiActor::_onContact(   PhysicsUserData *us,
                                 PhysicsUserData *them,
                                 const Point3F &hitPoint,
                                 const Point3F &hitForce )
{
   PxUserData *data = (PxUserData*)us;
   if (  data &&
         !data->mIsBroken &&
         hitForce.len() > mDataBlock->breakForce )
      setAllBroken( true );
}

U32 PxMultiActor::packUpdate(NetConnection *con, U32 mask, BitStream *stream)
{
   U32 retMask = Parent::packUpdate(con, mask, stream);

   stream->writeFlag( mDebugRender );

   stream->writeFlag( mask & SleepMask );

   if ( stream->writeFlag( mask & WarpMask ) )
   {
      stream->writeAffineTransform( getTransform() );
   }
   else if ( stream->writeFlag( mask & MoveMask ) )
   {
      /*
      stream->writeAffineTransform( getTransform() );

      NxActor *actor = mActors[ mDataBlock->correctionNodes[0] ];

      const NxVec3& linVel = actor->getLinearVelocity();
      stream->write( linVel.x );
      stream->write( linVel.y );
      stream->write( linVel.z );
      */
   }

   // This internally uses the mask passed to it.
   if ( mLightPlugin )
      retMask |= mLightPlugin->packUpdate( this, LightMask, con, mask, stream );

   return retMask;
}


void PxMultiActor::unpackUpdate(NetConnection *con, BitStream *stream)
{
   Parent::unpackUpdate(con, stream);

   mDebugRender = stream->readFlag();

   if ( stream->readFlag() ) // SleepMask
   {
      for ( S32 i = 0; i < mActors.size(); i++ )
      {
         NxActor *actor = mActors[i];

         if ( !actor )
            continue;

         if ( actor )
            actor->putToSleep();
      }
   }

   if ( stream->readFlag() ) // WarpMask
   {
      // If we set a warp mask,
      // we need to instantly move
      // the actor to the new position
      // without applying any corrections.
      MatrixF mat;
      stream->readAffineTransform( &mat );

      applyWarp( mat, true, false );
   }
   else if ( stream->readFlag() ) // MoveMask
   {
      /*
      MatrixF mat;
      stream->readAffineTransform( &mat );

      NxVec3 linVel, angVel;
      stream->read( &linVel.x );
      stream->read( &linVel.y );
      stream->read( &linVel.z );

      applyCorrection( mat, linVel, angVel );
      */
   }
/*
   if ( stream->readFlag() ) // ImpulseMask
   {
      // TODO : Set up correction nodes.

      NxVec3 linVel;
      stream->read( &linVel.x );
      stream->read( &linVel.y );
      stream->read( &linVel.z );

      NxActor *actor = mActors[ mDataBlock->correctionNodes[0] ];

      if ( actor )
      {
         mWorld->releaseWriteLock();
         actor->setLinearVelocity( linVel );
         mStartImpulse.zero();
      }
      else
         mStartImpulse.set( linVel.x, linVel.y, linVel.z );
   }
*/
   if ( mLightPlugin )
      mLightPlugin->unpackUpdate( this, con, stream );
}

void PxMultiActor::setScale( const VectorF& scale )
{
   if ( scale == getScale() )
      return;

   // This is so that the level
   // designer can change the scale
   // of a PhysXSingleActor in the editor
   // and have the PhysX representation updated properly.

   // First we call the parent's setScale
   // so that the ScaleMask can be set.
   Parent::setScale( scale );

   // Check to see if the scale has really changed.
   if ( !isProperlyAdded() || mActorScale.equal( scale ) )
      return;

   // Recreate the physics actors.
   _destroyActors();
   _createActors( getTransform() );
}

void PxMultiActor::applyWarp( const MatrixF& newMat, bool interpRender, bool sweep )
{
   // Do we have actors to move?
   if ( mRootActor )
   {
      // Get ready to change the physics state.
      mWorld->releaseWriteLock();

      /// Convert the new transform to nx.
      NxMat34 destXfm;
      destXfm.setRowMajor44( newMat );

      // Get the inverse of the root actor transform
      // so we can move all the actors relative to it.
      NxMat34 rootInverseXfm;
      mRootActor->getGlobalPose().getInverse( rootInverseXfm );

      // Offset all the actors.
      MatrixF tMat;
      NxMat34 newXfm, relXfm;
      for ( S32 i = 0; i < mActors.size(); i++ )
      {
         NxActor *actor = mActors[i];
         if ( !actor )
            continue;

         const bool isKinematic = actor->readBodyFlag( NX_BF_KINEMATIC );

         // Stop any velocity on it.
         if ( !isKinematic )
         {
            actor->setAngularVelocity( NxVec3( 0.0f ) );
            actor->setLinearVelocity( NxVec3( 0.0f ) );
         }

         // Get the transform relative to the current root.
         relXfm.multiply( actor->getGlobalPose(), rootInverseXfm );

         /*
         if ( sweep )
         {
            actor->getGl obalPose().getRowMajor44( mResetPos[i] );
            sweepTest( &newMat );
         }
         */

         //
         newXfm.multiply( relXfm, destXfm );
         //if ( isKinematic )
            //actor->moveGlobalPose( newXfm );
         //else
            actor->setGlobalPose( newXfm );

         // Reset the delta.
         Delta &delta = mActorDeltas[i];
         delta.pos = pxCast<Point3F>( newXfm.t );
         newXfm.getRowMajor44( tMat );
         delta.rot.set( tMat );

         if ( !interpRender )
         {
            mActorDeltas[i].lastPos = mActorDeltas[i].pos;
            mActorDeltas[i].lastRot = mActorDeltas[i].rot;
         }
      }
   }

   Parent::setTransform( newMat );

   mDelta.pos = newMat.getPosition();
   mDelta.rot = newMat;

   if ( !interpRender )
   {
      mDelta.lastPos = mDelta.pos;
      mDelta.lastRot = mDelta.rot;
   }
}

/*
bool PxMultiActor::_setPositionField( void *obj, const char *data )
{
   PxMultiActor *object = reinterpret_cast<PxMultiActor*>( obj );

   MatrixF transform( object->getTransform() );
   Con::setData( TypeMatrixPosition, &transform, 0, 1, &data );

   object->setTransform( transform );

   return false;
}

const char* PxMultiActor::_getPositionField( void *obj, const char *data )
{
   PxMultiActor *object = reinterpret_cast<PxMultiActor*>( obj );
   return Con::getData( TypeMatrixPosition,
                        &object->mObjToWorld,
                        0 );
}

bool PxMultiActor::_setRotationField( void *obj, const char *data )
{
   PxMultiActor *object = reinterpret_cast<PxMultiActor*>( obj );

   MatrixF transform( object->getTransform() );
   Con::setData( TypeMatrixRotation, &transform, 0, 1, &data );

   object->setTransform( transform );

   return false;
}

const char* PxMultiActor::_getRotationField( void *obj, const char *data )
{
   PxMultiActor *object = reinterpret_cast<PxMultiActor*>( obj );
   return Con::getData( TypeMatrixRotation,
                        &object->mObjToWorld,
                        0 );
}
*/

void PxMultiActor::setTransform( const MatrixF& mat )
{
   applyWarp( mat, false, true );
   setMaskBits( WarpMask );
}

void PxMultiActor::mountObject( SceneObject *obj, U32 node )
{
   if (obj->getObjectMount())
      obj->unmount();

   obj->mountObject( this, (node >= 0 && node < PxMultiActorData::NumMountPoints) ? node: 0 );
}


void PxMultiActor::unmountObject( SceneObject *obj )
{
   obj->unmountObject( this );
}

bool PxMultiActor::_getNodeTransform( U32 nodeIdx, MatrixF *outXfm )
{   
   if ( !mShapeInstance )
      return false;

   PxMultiActor *actorOwner = this;
   if ( mIsDummy )
   {
      actorOwner = static_cast<PxMultiActor*>( mServerObject.getObject() );
      if ( !actorOwner )
         return false;
   }

   TSShape *shape = mShapeInstance->getShape();
   String nodeName = shape->getNodeName( nodeIdx );

   NxActor *pActor = NULL;
   UTF8 comparisonName[260] = { 0 };
   S32 dummy = -1;

   // Convert the passed node name to a valid actor name.
   dStrcpy( comparisonName, String::GetTrailingNumber( nodeName, dummy ) );
   dSprintf( comparisonName, sizeof( comparisonName ), "%s_pxactor", comparisonName );

   // If we have an actor with that name, we are done.
   pActor = actorOwner->_findActor( comparisonName );
   if ( pActor )
   {
      pActor->getGlobalPose().getRowMajor44( *outXfm );
      return true;
   }

   // Check if the parent node has an actor...

   S32 parentIdx = shape->nodes[nodeIdx].parentIndex;
   if ( parentIdx == -1 )
      return false;

   const String &parentName = shape->getNodeName( parentIdx );
   dStrcpy( comparisonName, String::GetTrailingNumber( parentName, dummy ) );
   dSprintf( comparisonName, sizeof( comparisonName ), "%s_pxactor", comparisonName );

   pActor = actorOwner->_findActor( comparisonName );
   if ( !pActor )
      return false;

   MatrixF actorMat;
   pActor->getGlobalPose().getRowMajor44( actorMat );

   MatrixF nmat;
   QuatF q;
   TSTransform::setMatrix( shape->defaultRotations[nodeIdx].getQuatF(&q),shape->defaultTranslations[nodeIdx],&nmat);
   *outXfm->mul( actorMat, nmat );
   
   return true;
}

void PxMultiActor::getMountTransform(U32 mountPoint,MatrixF* mat)
{
   // Returns mount point to world space transform
   if (mountPoint < PxMultiActorData::NumMountPoints) {
      S32 ni = mDataBlock->mountPointNode[mountPoint];
      if (ni != -1) {
         if ( _getNodeTransform( ni, mat ) )
            return;
      }
   }
   *mat = mObjToWorld;
}

void PxMultiActor::getRenderMountTransform(U32 mountPoint,MatrixF* mat)
{
   // Returns mount point to world space transform
   if (mountPoint < PxMultiActorData::NumMountPoints) {
      S32 ni = mDataBlock->mountPointNode[mountPoint];
      if (ni != -1) {
         if ( _getNodeTransform( ni, mat ) )
            return;
      }
   }
   *mat = getRenderTransform();
}

void PxMultiActor::processTick( const Move *move )
{
   PROFILE_SCOPE( PxMultiActor_ProcessTick );

   // Set the last pos/rot to the
   // values of the previous tick for interpolateTick.
   mDelta.lastPos = mDelta.pos;
   mDelta.lastRot = mDelta.rot;

   /*
   NxActor *corrActor = mActors[ mDataBlock->correctionNodes[0] ];

   if ( corrActor->isSleeping() || corrActor->readBodyFlag( NX_BF_FROZEN ) )
   {
      if ( !mSleepingLastTick )
         setMaskBits( WarpMask | SleepMask );

      mSleepingLastTick = true;

      // HACK!  Refactor sleeping so that we don't
      // sleep when only one correction actor does.
      _updateBounds();

      return;
   }

   mSleepingLastTick = false;
   */

   MatrixF mat;
   Vector<NxActor*> *actors;

   if ( mIsDummy )
   {
      PxMultiActor *serverObj = static_cast<PxMultiActor*>( mServerObject.getObject() );
      if ( !serverObj )
         return;

      mat = serverObj->getTransform();
      actors = &serverObj->mActors;
   }
   else
   {
      // Container buoyancy & drag
      _updateContainerForces();

      // Save the transform from the root actor.
      mRootActor->getGlobalPose().getRowMajor44( mat );
      actors = &mActors;
   }

   // Update the transform and the root delta.
   Parent::setTransform( mat );
   mDelta.pos = mat.getPosition();
   mDelta.rot.set( mat );

   // On the client we update the individual
   // actor deltas as well for interpolation.
   if ( isClientObject() )
   {
      PROFILE_SCOPE( PxMultiActor_ProcessTick_UpdateDeltas );

      for ( U32 i = 0; i < mActorDeltas.size(); i++ )
      {
         if ( !(*actors)[i] )
            continue;

         Delta &delta = mActorDeltas[i];

         // Store the last position.
         delta.lastPos = delta.pos;
         delta.lastRot = delta.rot;

         // Get the new position.
         (*actors)[i]->getGlobalPose().getRowMajor44( (NxF32*)mat );

         // Calculate the delta between the current
         // global pose and the last global pose.
         delta.pos = mat.getPosition();
         delta.rot.set( mat );
      }
   }

   // Update the bounding box to match the physics.
   _updateBounds();

   // Set the MoveMask so this will be updated to the client.
   //setMaskBits( MoveMask );
}

void PxMultiActor::interpolateTick( F32 delta )
{
   PROFILE_SCOPE( PxMultiActor_InterpolateTick );

   Point3F interpPos;
   QuatF interpRot;
   {
       // Interpolate the position based on the delta.
      interpPos.interpolate( mDelta.pos, mDelta.lastPos, delta );

      // Interpolate the rotation based on the delta.
      interpRot.interpolate( mDelta.rot, mDelta.lastRot, delta );

      // Set up the interpolated transform.
      MatrixF interpMat;
      interpRot.setMatrix( &interpMat );
      interpMat.setPosition( interpPos );

      Parent::setRenderTransform( interpMat );
   }

   PxMultiActor *srcObj = NULL;
   if ( mIsDummy )
      srcObj = static_cast<PxMultiActor*>( mServerObject.getObject() );
   else
      srcObj = this;

   // JCF: to disable applying NxActor positions to the renderable mesh
   // you can uncomment this line.
   //srcObj = NULL;
   if ( mShapeInstance && srcObj != NULL )
   {
      mShapeInstance->animate();
      getDynamicXfms( srcObj, delta );
   }
}

/*
void PxMultiActor::sweepTest( MatrixF *mat )
{
   NxVec3 nxCurrPos = getPosition();

   // If the position is zero,
   // the parent hasn't been updated yet
   // and we don't even need to do the sweep test.
   // This is a fix for a problem that was happening
   // where on the add of the PhysXSingleActor, it would
   // set the position to a very small value because it would be getting a hit
   // even though the current position was 0.
   if ( nxCurrPos.isZero() )
      return;

   // Set up the flags and the query structure.
   NxU32 flags = NX_SF_STATICS | NX_SF_DYNAMICS;

   NxSweepQueryHit sweepResult;
   dMemset( &sweepResult, 0, sizeof( sweepResult ) );

   NxVec3 nxNewPos = mat->getPosition();

   // Get the velocity which will be our sweep direction and distance.
   NxVec3 nxDir = nxNewPos - nxCurrPos;
   if ( nxDir.isZero() )
      return;

   NxActor *corrActor = mActors[ mDataBlock->correctionNodes[0] ];

   // Get the scene and do the sweep.
   corrActor->wakeUp();
   corrActor->linearSweep( nxDir, flags, NULL, 1, &sweepResult, NULL );

   if ( sweepResult.hitShape && sweepResult.t < nxDir.magnitude() )
   {
      nxDir.normalize();
      nxDir *= sweepResult.t;
      nxCurrPos += nxDir;

      mat->setPosition( Point3F( nxCurrPos.x, nxCurrPos.y, nxCurrPos.z ) );
   }
}
*/

/*
void PxMultiActor::applyCorrection( const MatrixF& mat, const NxVec3& linVel, const NxVec3& angVel )
{
   // Sometimes the actor hasn't been
   // created yet during the call from unpackUpdate.
   NxActor *corrActor = mActors[ mDataBlock->correctionNodes[0] ];

   if ( !corrActor || mForceSleep )
      return;

   NxVec3 newPos = mat.getPosition();
   NxVec3 currPos = getPosition();

   NxVec3 offset = newPos - currPos;

   // If the difference isn't large enough,
   // just set the new transform, no correction.
   if ( offset.magnitude() > 0.3f )
   {
      // If we're going to set the linear or angular velocity,
      // we do it before we add a corrective force, since it would be overwritten otherwise.
      NxVec3 currLinVel, currAngVel;
      currLinVel = corrActor->getLinearVelocity();
      currAngVel = corrActor->getAngularVelocity();

      // Scale the corrective force by half,
      // otherwise it will over correct and oscillate.
      NxVec3 massCent = corrActor->getCMassGlobalPosition();
      corrActor->addForceAtPos( offset, massCent, NX_SMOOTH_VELOCITY_CHANGE );

      // If the linear velocity is divergent enough, change to server linear velocity.
      if ( (linVel - currLinVel).magnitude() > 0.3f )
         corrActor->setLinearVelocity( linVel );
      // Same for angular.
      if ( (angVel - currAngVel).magnitude() > 0.3f )
         corrActor->setAngularVelocity( angVel );
   }

   Parent::setTransform( mat );
}
*/

void PxMultiActor::_updateBounds()
{
   PROFILE_SCOPE( PxMultiActor_UpdateBounds );

   if ( mIsDummy )
   {
      PxMultiActor *serverObj = static_cast<PxMultiActor*>( mServerObject.getObject() );
      if ( !serverObj )
         return;

      mWorldBox = serverObj->getWorldBox();
      mWorldSphere = serverObj->getWorldSphere();
      mObjBox = serverObj->getObjBox();
      mRenderWorldBox = serverObj->getRenderWorldBox();
      mRenderWorldSphere = mWorldSphere;

      return;
   }

   NxBounds3 bounds;
   bounds.setEmpty();

   NxBounds3 shapeBounds;

   for ( U32 i = 0; i < mActors.size(); i++ )
   {
      NxActor *pActor = mActors[i];

      if ( !pActor || pActor->readActorFlag( NX_AF_DISABLE_COLLISION ) )
         continue;

      NxShape *const* pShapeArray = pActor->getShapes();
      U32 shapeCount = pActor->getNbShapes();
      for ( U32 i = 0; i < shapeCount; i++ )
      {
         // Get the shape's bounds.
         pShapeArray[i]->getWorldBounds( shapeBounds );

         // Combine them into the total bounds.
         bounds.combine( shapeBounds );
      }
   }

   mWorldBox = pxCast<Box3F>( bounds );

   mWorldBox.getCenter(&mWorldSphere.center);
   mWorldSphere.radius = (mWorldBox.maxExtents - mWorldSphere.center).len();

   mObjBox = mWorldBox;
   mWorldToObj.mul(mObjBox);

   mRenderWorldBox = mWorldBox;
   mRenderWorldSphere = mWorldSphere;
}

void PxMultiActor::getDynamicXfms( PxMultiActor *srcObj, F32 dt )
{
   PROFILE_SCOPE( PxMultiActor_getDynamicXfms );

   Vector<MatrixF> *torqueXfms = &mShapeInstance->mNodeTransforms;
   const MatrixF &objectXfm = getRenderWorldTransform();

   AssertFatal( torqueXfms->size() == srcObj->mMappedActors.size(), "The two skeletons are different!" );

   TSShape *shape = mShapeInstance->getShape();

   // TODO: We're currently preparing deltas and getting
   // dynamic xforms even if the object isn't visible.
   // we should probably try to delay all this until
   // we're about to render.
   //
   /*
   // TODO: Set up deltas!
   if ( mCurrPos.empty() || mCurrRot.empty() )
      _prepareDeltas();
   */

   MatrixF globalXfm;
   MatrixF mat, tmp;
   QuatF newRot;
   Point3F newPos;

   S32 dl = mShapeInstance->getCurrentDetail();
   if ( dl < 0 )
      return;

   const String &detailName = shape->getName( shape->details[dl].nameIndex );
   S32 detailSize = -1;
   String::GetTrailingNumber( detailName, detailSize );

   for( S32 i = 0; i < srcObj->mMappedActors.size(); i++ )
   {
      NxActor *actor = srcObj->mMappedActors[i];

      if ( !actor || actor->readBodyFlag( NX_BF_KINEMATIC ) )
         continue;

      // see if the node at this index is part of the
      // currently visible detail level.
      if ( srcObj->mMappedActorDL[i] != detailSize )
         continue;

      // Get the right actor delta structure.
      U32 index = srcObj->mMappedToActorIndex[i];
      const Delta &delta = mActorDeltas[index];

      // Do the interpolation.
      newRot.interpolate( delta.rot, delta.lastRot, dt );
      newRot.setMatrix( &globalXfm );
      newPos.interpolate( delta.pos, delta.lastPos, dt );
      globalXfm.setPosition( newPos );

      (*torqueXfms)[i].mul( objectXfm, globalXfm );
   }
}

void PxMultiActor::applyImpulse( const Point3F &pos, const VectorF &vec )
{
   // TODO : Implement this based on correction nodes.
   /*
   if ( !mWorld || !mActor )
      return;

   mWorld->releaseWriteLock();

   NxVec3 linVel = mActor->getLinearVelocity();
   NxVec3 nxVel( vel.x, vel.y, vel.z );

   mActor->setLinearVelocity(linVel + nxVel);
   */

   // JCF: something more complex is required to apply forces / breakage
   // on only individual actors, and we don't have enough data to do that
   // within this method.

   if ( vec.len() > mDataBlock->breakForce )
      setAllBroken( true );

   NxVec3 nxvec = pxCast<NxVec3>( vec );
   NxVec3 nxpos = pxCast<NxVec3>( pos );

   for ( U32 i = 0; i < mActors.size(); i++ )
   {
      NxActor *actor = mActors[i];
      if ( actor->isDynamic() &&
           !actor->readBodyFlag( NX_BF_KINEMATIC ) &&
           !actor->readActorFlag( NX_AF_DISABLE_COLLISION ) )
      {
         actor->addForceAtPos( nxvec, nxpos, NX_IMPULSE );
      }
   }

   //setMaskBits( ImpulseMask );
}

void PxMultiActor::applyRadialImpulse( const Point3F &origin, F32 radius, F32 magnitude )
{
   mWorld->releaseWriteLock();

   // Find all currently enabled actors hit by the impulse radius...
   Vector<NxActor*> hitActors;
   NxVec3 nxorigin = pxCast<NxVec3>(origin);
   NxSphere impulseSphere( nxorigin, radius );

   for ( U32 i = 0; i < mActors.size(); i++ )
   {
      NxActor *pActor = mActors[i];

      if ( pActor->readActorFlag( NX_AF_DISABLE_COLLISION ) ||
           !pActor->isDynamic() ||
           pActor->readBodyFlag( NX_BF_KINEMATIC ) )
         continue;

      U32 numShapes = pActor->getNbShapes();
      NxShape *const* pShapeArray = pActor->getShapes();

      for ( U32 j = 0; j < numShapes; j++ )
      {
         const NxShape *pShape = pShapeArray[j];

         if ( pShape->checkOverlapSphere( impulseSphere ) )
         {
            hitActors.push_back( pActor );
            break;
         }
      }
   }

   // Apply forces to hit actors, but swap out for broken
   // actors first if appropriate...
   for ( U32 i = 0; i < hitActors.size(); i++ )
   {
      NxActor *pActor = hitActors[i];

      PxUserData *pUserData = PxUserData::getData( *pActor );

      // TODO: We should calculate the real force accounting
      // for falloff before we break things with it.

      // If we have enough force, and this is an actor that
      // can be 'broken' by impacts, break it now.
      if ( pUserData &&
           //pUserData->mCanPush &&
           pUserData->mBrokenActors.size() > 0 &&
           magnitude > mDataBlock->breakForce )
      {
         setBroken(  pActor->getGlobalPose(),
                     pActor->getLinearVelocity(),
                     pUserData,
                     true );

         // apply force that would have been applied to this actor
         // to the broken actors we just enabled.

         for ( U32 j = 0; j < pUserData->mBrokenActors.size(); j++ )
         {
            NxActor *pBrokenActor = pUserData->mBrokenActors[j];
            _applyActorRadialForce( pBrokenActor, nxorigin, radius, magnitude );
         }
      }
      else
      {
         // Apply force to the actor.
         _applyActorRadialForce( pActor, nxorigin, radius, magnitude );
      }
   }
}

void PxMultiActor::_applyActorRadialForce( NxActor *inActor, const NxVec3 &origin, F32 radius, F32 magnitude )
{
   // TODO: We're not getting a good torque force
   // out of explosions because we're not picking
   // the nearest point on the actor to the origin
   // of the radial force.

   NxVec3 force = inActor->getCMassGlobalPosition() - origin;
   NxF32 dist = force.magnitude();
   force.normalize();

   if ( dist == 0.0f )
      force *= magnitude;
   else
      force *= mClampF( radius / dist, 0.0f, 1.0f ) * magnitude;      

   // HACK: Make the position we push the force thru between the
   // actor pos and its center of mass.  This gives us some
   // rotational force as well as make the covered structure
   // explode better.
   NxVec3 forcePos = ( inActor->getGlobalPosition() + inActor->getCMassGlobalPosition() ) / 2.0f;
   inActor->addForceAtPos( force, forcePos, NX_VELOCITY_CHANGE );
}

void PxMultiActor::_updateContainerForces()
{
   if ( !mWorld->getEnabled() )
      return;

   PROFILE_SCOPE( PxMultiActor_updateContainerForces );

   // Update container drag and buoyancy properties ( for each Actor )

   for ( U32 i = 0; i < mActors.size(); i++ )
   {
      NxActor *pActor = mActors[i];

      if (  !pActor ||
            pActor->readBodyFlag(NX_BF_KINEMATIC) ||
            pActor->readActorFlag(NX_AF_DISABLE_COLLISION) )
         continue;

      // Get world bounds of this actor ( the combination of all shape bounds )
      NxShape *const* shapes = pActor->getShapes();
      NxBounds3 bounds;
      bounds.setEmpty();
      NxBounds3 shapeBounds;

      for ( U32 i = 0; i < pActor->getNbShapes(); i++ )
      {
         NxShape *pShape = shapes[i];
         pShape->getWorldBounds(shapeBounds);

         bounds.combine( shapeBounds );
      }

      Box3F boundsBox = pxCast<Box3F>(bounds);

      ContainerQueryInfo info;
      info.box = boundsBox;
      info.mass = pActor->getMass();

      // Find and retreive physics info from intersecting WaterObject(s)
      mContainer->findObjects( boundsBox, WaterObjectType|PhysicalZoneObjectType, findRouter, &info );

      // Calculate buoyancy and drag
      F32 angDrag = mDataBlock->angularDrag;
      F32 linDrag = mDataBlock->linearDrag;
      F32 buoyancy = 0.0f;

      if ( true ) //info.waterCoverage >= 0.1f)
      {
         F32 waterDragScale = info.waterViscosity * mDataBlock->waterDragScale;
         F32 powCoverage = mPow( info.waterCoverage, 0.25f );

         if ( info.waterCoverage > 0.0f )
         {
            //angDrag = mBuildAngDrag * waterDragScale;
            //linDrag = mBuildLinDrag * waterDragScale;
            angDrag = mLerp( angDrag, angDrag * waterDragScale, powCoverage );
            linDrag = mLerp( linDrag, linDrag * waterDragScale, powCoverage );
         }

         buoyancy = ( info.waterDensity / mDataBlock->buoyancyDensity ) * mPow( info.waterCoverage, 2.0f );
      }

      // Apply drag (dampening)
      pActor->setLinearDamping( linDrag );
      pActor->setAngularDamping( angDrag );

      // Apply buoyancy force
      if ( buoyancy != 0 )
      {
         // A little hackery to prevent oscillation
         // Based on this blog post:
         // (http://reinot.blogspot.com/2005/11/oh-yes-they-float-georgie-they-all.html)
         // JCF: disabled!
         NxVec3 gravity;
         mWorld->getScene()->getGravity(gravity);
         //NxVec3 velocity = pActor->getLinearVelocity();

         NxVec3 buoyancyForce = buoyancy * -gravity * TickSec * pActor->getMass();
         //F32 currHeight = getPosition().z;
         //const F32 C = 2.0f;
         //const F32 M = 0.1f;

         //if ( currHeight + velocity.z * TickSec * C > info.waterHeight )
         //   buoyancyForce *= M;

         pActor->addForceAtPos( buoyancyForce, pActor->getCMassGlobalPosition(), NX_IMPULSE );
      }

      // Apply physical zone forces
      if ( info.appliedForce.len() > 0.001f )
         pActor->addForceAtPos( pxCast<NxVec3>(info.appliedForce), pActor->getCMassGlobalPosition(), NX_IMPULSE );
   }
}

/*
ConsoleMethod( PxMultiActor, applyImpulse, void, 3, 3, "applyImpulse - takes a velocity vector to apply")
{
   VectorF vec;
   dSscanf( argv[2],"%g %g %g",
           &vec.x,&vec.y,&vec.z );

   object->applyImpulse( vec );
}
*/

void PxMultiActor::setAllBroken( bool isBroken )
{
   PROFILE_SCOPE( PxMultiActor_SetAllBroken );

   if ( mIsDummy )
   {
      PxMultiActor *serverObj = static_cast<PxMultiActor*>( mServerObject.getObject() );
      serverObj->setAllBroken( isBroken );
      return;
   }

   mWorld->releaseWriteLock();

   NxActor *actor0 = NULL;
   NxActor *actor1 = NULL;
   NxMat34 parentPose;

   for ( U32 i = 0; i < mJoints.size(); i++ )
   {
      NxJoint *joint = mJoints[i];
      if ( !joint )
         continue;

      PxUserData *jointData = PxUserData::getData( *joint );
      if ( !jointData )
         continue;

      joint->getActors( &actor0, &actor1 );
      parentPose = actor0->getGlobalPose();

      setBroken( parentPose, NxVec3(0.0f), jointData, isBroken );
   }

   for ( U32 i = 0; i < mActors.size(); i++ )
   {
      NxActor *actor = mActors[i];
      if ( !actor )
         continue;

      PxUserData *actorData = PxUserData::getData( *actor );
      if ( !actorData )
         continue;

      setBroken(  actor->getGlobalPose(),
                  actor->getLinearVelocity(),
                  actorData,
                  isBroken );
   }
}

void PxMultiActor::setBroken( const NxMat34 &parentPose,
                              const NxVec3 &parentVel,
                              PxUserData *userData,
                              bool isBroken )
{
   PROFILE_SCOPE( PxMultiActor_SetBroken );

   // TODO: This function is highly inefficent and
   // way too complex to follow... the hacked single
   // player mode doesn't help.

   // Be careful not to set something broken twice.
   if (  isBroken &&
         userData->mIsBroken == isBroken )
      return;

   userData->mIsBroken = isBroken;

   Vector<NxActor*> *hideActors = NULL;
   Vector<NxActor*> *showActors = NULL;

   if ( isBroken )
   {
      hideActors = &userData->mUnbrokenActors;
      showActors = &userData->mBrokenActors;
   }
   else
   {
      hideActors = &userData->mBrokenActors;
      showActors = &userData->mUnbrokenActors;
   }

   NxActor *pActor = NULL;
   MatrixF tMat;
   for ( U32 i = 0; i < hideActors->size(); i++ )
   {
      pActor = (*hideActors)[i];

      pActor->raiseActorFlag( NX_AF_DISABLE_COLLISION );
      pActor->raiseBodyFlag( NX_BF_KINEMATIC );
      pActor->putToSleep();

      NxShape *const* pShapeArray = pActor->getShapes();
      U32 shapeCount = pActor->getNbShapes();
      for ( U32 i = 0; i < shapeCount; i++ )
         pShapeArray[i]->setFlag( NX_SF_DISABLE_RAYCASTING, true );

      setMeshHidden( _getMeshName( pActor ), true );
   }

   // Get the client side delta array.
   Vector<Delta> *actorDeltas = NULL;
   if ( isClientObject() )
      actorDeltas = &mActorDeltas;
   else if ( isServerObject() && PHYSICSMGR->isSinglePlayer() )
   {
      PxMultiActor *clientObj = static_cast<PxMultiActor*>( getClientObject() );
      if ( clientObj )
         actorDeltas = &clientObj->mActorDeltas;
   }
   U32 index;

   for ( U32 i = 0; i < showActors->size(); i++ )
   {
      pActor = (*showActors)[i];

      if ( showActors == &userData->mBrokenActors )
      {
         NxMat34 pose;
         pose.multiply( parentPose, userData->mRelXfm[i] );
         pActor->setGlobalPose( pose );

         if ( actorDeltas )
         {
            for ( U32 j=0; j < mMappedActors.size(); j++ )
            {
               if ( mMappedActors[j] == pActor )
               {
                  index = mMappedToActorIndex[j];

                  // Reset the delta.
                  Delta &delta = (*actorDeltas)[index];
                  delta.pos = pxCast<Point3F>( pose.t );
                  pose.getRowMajor44( tMat );
                  delta.rot.set( tMat );
                  delta.lastPos = delta.pos;
                  delta.lastRot = delta.rot;

                  break;
               }
            }
         }
      }

      pActor->clearActorFlag( NX_AF_DISABLE_COLLISION );
      pActor->clearBodyFlag( NX_BF_KINEMATIC );
      pActor->setLinearVelocity( parentVel );
      pActor->wakeUp();

      NxShape *const* pShapeArray = pActor->getShapes();
      U32 shapeCount = pActor->getNbShapes();
      for ( U32 i = 0; i < shapeCount; i++ )
         pShapeArray[i]->setFlag( NX_SF_DISABLE_RAYCASTING, false );

      setMeshHidden( _getMeshName(pActor), false );
   }
}

void PxMultiActor::setAllHidden( bool hide )
{
   for ( U32 i = 0; i < mShapeInstance->mMeshObjects.size(); i++ )
      mShapeInstance->setMeshForceHidden( i, hide );
}

ConsoleMethod( PxMultiActor, setAllHidden, void, 3, 3, "( bool )"
               "@brief Hides or unhides all meshes contained in the PxMultiActor.\n\n"
               "Hidden meshes will not be rendered.")
{
   object->setAllHidden( dAtob(argv[2]) );
}

void PxMultiActor::setMeshHidden( String namePrefix, bool hidden )
{
   if ( isServerObject() && PHYSICSMGR->isSinglePlayer() )
   {
      PxMultiActor *clientObj = static_cast<PxMultiActor*>( getClientObject() );
      if ( clientObj )
         clientObj->setMeshHidden( namePrefix, hidden );
   }

   for ( U32 i = 0; i < mShapeInstance->mMeshObjects.size(); i++ )
   {
      String meshName = mShapeInstance->getShape()->getMeshName( i );

      if ( meshName.find( namePrefix ) != String::NPos )
      {
         mShapeInstance->setMeshForceHidden( i, hidden );
         return;
      }
   }

   Con::warnf( "PxMultiActor::setMeshHidden - could not find mesh containing substring (%s)", namePrefix.c_str() );
}

ConsoleMethod( PxMultiActor, setBroken, void, 3, 3, "( bool )"
             "@brief Sets the PxMultiActor to a broken or unbroken state.\n\n")
{
   object->setAllBroken( dAtob( argv[2] ) );
}

void PxMultiActorData::dumpModel()
{
   TSShapeInstance *inst = new TSShapeInstance( shape, true );

   String path = Platform::getMainDotCsDir();
   path += "/model.dump";

   FileStream *st;
   if((st = FileStream::createAndOpen( path, Torque::FS::File::Write )) != NULL)
   {
      if ( inst )
         inst->dump( *st );
      else
         Con::errorf( "PxMultiActor::dumpModel, no ShapeInstance." );

      delete st;
   }
   else
      Con::errorf( "PxMultiActor::dumpModel, error opening dump file." );
}

ConsoleMethod( PxMultiActorData, dumpModel, void, 2, 2, 
             "@brief Dumps model hierarchy and details to a file.\n\n"
             "The file will be created as \'model.dump\' in the game folder. "
             "If model.dump already exists, it will be overwritten.\n\n")
{
   object->dumpModel();
}

ConsoleMethod( PxMultiActor, setMeshHidden, void, 4, 4, "(string meshName, bool isHidden)"
             "@brief Prevents the provided mesh from being rendered.\n\n")
{
   object->setMeshHidden( argv[2], dAtob( argv[3] ) );
}

void PxMultiActor::listMeshes( const String &state ) const
{
   if ( mShapeInstance )
      mShapeInstance->listMeshes( state );
}

ConsoleMethod( PxMultiActor, listMeshes, void, 3, 3, "(enum Hidden/Shown/All)"
             "@brief Lists all meshes of the provided type in the console window.\n\n"
             "@param All Lists all of the %PxMultiActor's meshes.\n"
             "@param Hidden Lists all of the %PxMultiActor's hidden meshes.\n"
             "@param Shown Lists all of the %PxMultiActor's visible meshes.\n")
{
   object->listMeshes( argv[2] );
};

ConsoleMethod( PxMultiActorData, reload, void, 2, 2, ""
              "@brief Reloads all data used for the PxMultiActorData.\n\n"
              "If the reload sucessfully completes, all PxMultiActor's will be notified.\n\n")
{
   object->reload();
}