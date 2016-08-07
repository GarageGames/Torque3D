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
#include "T3D/shapeBase.h"

#include "core/dnet.h"
#include "sfx/sfxSystem.h"
#include "sfx/sfxSource.h"
#include "sfx/sfxTrack.h"
#include "sfx/sfxDescription.h"
#include "T3D/sfx/sfx3DWorld.h"
#include "T3D/gameBase/gameConnection.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "core/stream/bitStream.h"
#include "ts/tsPartInstance.h"
#include "ts/tsShapeInstance.h"
#include "ts/tsMaterialList.h"
#include "scene/sceneManager.h"
#include "scene/sceneRenderState.h"
#include "scene/sceneObjectLightingPlugin.h"
#include "T3D/fx/explosion.h"
#include "T3D/fx/cameraFXMgr.h"
#include "environment/waterBlock.h"
#include "T3D/debris.h"
#include "T3D/physicalZone.h"
#include "T3D/containerQuery.h"
#include "math/mathUtils.h"
#include "math/mMatrix.h"
#include "math/mTransform.h"
#include "math/mRandom.h"
#include "platform/profiler.h"
#include "gfx/gfxCubemap.h"
#include "gfx/gfxDrawUtil.h"
#include "gfx/gfxTransformSaver.h"
#include "renderInstance/renderPassManager.h"
#include "collision/earlyOutPolyList.h"
#include "core/resourceManager.h"
#include "scene/reflectionManager.h"
#include "gfx/sim/cubemapData.h"
#include "materials/materialManager.h"
#include "materials/materialFeatureTypes.h"
#include "renderInstance/renderOcclusionMgr.h"
#include "core/stream/fileStream.h"
#include "T3D/accumulationVolume.h"

IMPLEMENT_CO_DATABLOCK_V1(ShapeBaseData);

ConsoleDocClass( ShapeBaseData,
   "@brief Defines properties for a ShapeBase object.\n\n"
   "@see ShapeBase\n"
   "@ingroup gameObjects\n"
);

IMPLEMENT_CALLBACK( ShapeBaseData, onEnabled, void, ( ShapeBase* obj, const char* lastState ), ( obj, lastState ),
   "@brief Called when the object damage state changes to Enabled.\n\n"
   "@param obj The ShapeBase object\n"
   "@param lastState The previous damage state\n" );

IMPLEMENT_CALLBACK( ShapeBaseData, onDisabled, void, ( ShapeBase* obj, const char* lastState ), ( obj, lastState ),
   "@brief Called when the object damage state changes to Disabled.\n\n"
   "@param obj The ShapeBase object\n"
   "@param lastState The previous damage state\n" );

IMPLEMENT_CALLBACK( ShapeBaseData, onDestroyed, void, ( ShapeBase* obj, const char* lastState ), ( obj, lastState ),
   "@brief Called when the object damage state changes to Destroyed.\n\n"
   "@param obj The ShapeBase object\n"
   "@param lastState The previous damage state\n" );

IMPLEMENT_CALLBACK( ShapeBaseData, onImpact, void, ( ShapeBase* obj, SceneObject *collObj, VectorF vec, F32 len ), ( obj, collObj, vec, len ),
   "@brief Called when we collide with another object beyond some impact speed.\n\n"
   "The Player class makes use of this callback when a collision speed is more than PlayerData::minImpactSpeed.\n"
   "@param obj The ShapeBase object\n"
   "@param collObj The object we collided with\n"
   "@param vec Collision impact vector\n"
   "@param len Length of the impact vector\n" );

IMPLEMENT_CALLBACK( ShapeBaseData, onCollision, void, ( ShapeBase* obj, SceneObject *collObj, VectorF vec, F32 len ), ( obj, collObj, vec, len ),
   "@brief Called when we collide with another object.\n\n"
   "@param obj The ShapeBase object\n"
   "@param collObj The object we collided with\n"
   "@param vec Collision impact vector\n"
   "@param len Length of the impact vector\n" );

IMPLEMENT_CALLBACK( ShapeBaseData, onDamage, void, ( ShapeBase* obj, F32 delta ), ( obj, delta ),
   "@brief Called when the object is damaged.\n\n"
   "@param obj The ShapeBase object\n"
   "@param obj The ShapeBase object\n"
   "@param delta The amount of damage received." );

IMPLEMENT_CALLBACK( ShapeBaseData, onTrigger, void, ( ShapeBase* obj, S32 index, bool state ), ( obj, index, state ),
   "@brief Called when a move trigger input changes state.\n\n"
   "@param obj The ShapeBase object\n"
   "@param index Index of the trigger that changed\n"
   "@param state New state of the trigger\n" );

IMPLEMENT_CALLBACK(ShapeBaseData, onEndSequence, void, (ShapeBase* obj, S32 slot, const char* name), (obj, slot, name),
   "@brief Called when a thread playing a non-cyclic sequence reaches the end of the "
   "sequence.\n\n"
   "@param obj The ShapeBase object\n"
   "@param slot Thread slot that finished playing\n"
   "@param name Thread name that finished playing\n");

IMPLEMENT_CALLBACK( ShapeBaseData, onForceUncloak, void, ( ShapeBase* obj, const char* reason ), ( obj, reason ),
   "@brief Called when the object is forced to uncloak.\n\n"
   "@param obj The ShapeBase object\n"
   "@param reason String describing why the object was uncloaked\n" );

//----------------------------------------------------------------------------
// Timeout for non-looping sounds on a channel
static SimTime sAudioTimeout = 500;
F32  ShapeBase::sWhiteoutDec = 0.007f;
F32  ShapeBase::sDamageFlashDec = 0.02f;
F32  ShapeBase::sFullCorrectionDistance = 0.5f;
F32  ShapeBase::sCloakSpeed = 0.5;
U32  ShapeBase::sLastRenderFrame = 0;

static const char *sDamageStateName[] =
{
   // Index by enum ShapeBase::DamageState
   "Enabled",
   "Disabled",
   "Destroyed"
};


//----------------------------------------------------------------------------

ShapeBaseData::ShapeBaseData()
 : shadowEnable( false ),
   shadowSize( 128 ),
   shadowMaxVisibleDistance( 80.0f ),
   shadowProjectionDistance( 10.0f ),
   shadowSphereAdjust( 1.0f ),
   shapeName( StringTable->insert("") ),
   cloakTexName( StringTable->insert("") ),
   cubeDescId( 0 ),
   reflectorDesc( NULL ),
   debris( NULL ),
   debrisID( 0 ),
   debrisShapeName( StringTable->insert("") ),
   explosion( NULL ),
   explosionID( 0 ),
   underwaterExplosion( NULL ),
   underwaterExplosionID( 0 ),
   mass( 1.0f ),
   drag( 0.0f ),
   density( 1.0f ),
   maxEnergy( 0.0f ),
   maxDamage( 1.0f ),
   destroyedLevel( 1.0f ),
   disabledLevel( 1.0f ),
   repairRate( 0.0033f ),
   eyeNode( -1 ),
   earNode( -1 ),
   cameraNode( -1 ),
   cameraMaxDist( 0.0f ),
   cameraMinDist( 0.2f ),
   cameraDefaultFov( 75.0f ),
   cameraMinFov( 5.0f ),
   cameraMaxFov( 120.f ),
   cameraCanBank( false ),
   mountedImagesBank( false ),
   debrisDetail( -1 ),
   damageSequence( -1 ),
   hulkSequence( -1 ),
   observeThroughObject( false ),
   firstPersonOnly( false ),
   useEyePoint( false ),
   isInvincible( false ),
   renderWhenDestroyed( true ),
   computeCRC( false ),
   inheritEnergyFromMount( false ),
   mCRC( 0 )
{      
   dMemset( mountPointNode, -1, sizeof( S32 ) * SceneObject::NumMountPoints );
}

struct ShapeBaseDataProto
{
   F32 mass;
   F32 drag;
   F32 density;
   F32 maxEnergy;
   F32 cameraMaxDist;
   F32 cameraMinDist;
   F32 cameraDefaultFov;
   F32 cameraMinFov;    
   F32 cameraMaxFov;    


   ShapeBaseDataProto()
   {
      mass = 1;
      drag = 0;
      density = 1;
      maxEnergy = 0;
      cameraMaxDist = 0;
      cameraMinDist = 0.2f;
      cameraDefaultFov = 75.f;
      cameraMinFov = 5.0f;
      cameraMaxFov = 120.f;
   }
};

static ShapeBaseDataProto gShapeBaseDataProto;

ShapeBaseData::~ShapeBaseData()
{

}

bool ShapeBaseData::preload(bool server, String &errorStr)
{
   if (!Parent::preload(server, errorStr))
      return false;
   bool shapeError = false;

   // Resolve objects transmitted from server
   if (!server) {

      if( !explosion && explosionID != 0 )
      {
         if( Sim::findObject( explosionID, explosion ) == false)
         {
            Con::errorf( ConsoleLogEntry::General, "ShapeBaseData::preload: Invalid packet, bad datablockId(explosion): 0x%x", explosionID );
         }
         AssertFatal(!(explosion && ((explosionID < DataBlockObjectIdFirst) || (explosionID > DataBlockObjectIdLast))),
            "ShapeBaseData::preload: invalid explosion data");
      }

      if( !underwaterExplosion && underwaterExplosionID != 0 )
      {
         if( Sim::findObject( underwaterExplosionID, underwaterExplosion ) == false)
         {
            Con::errorf( ConsoleLogEntry::General, "ShapeBaseData::preload: Invalid packet, bad datablockId(underwaterExplosion): 0x%x", underwaterExplosionID );
         }
         AssertFatal(!(underwaterExplosion && ((underwaterExplosionID < DataBlockObjectIdFirst) || (underwaterExplosionID > DataBlockObjectIdLast))),
            "ShapeBaseData::preload: invalid underwaterExplosion data");
      }

      if( !debris && debrisID != 0 )
      {
         Sim::findObject( debrisID, debris );
         AssertFatal(!(debris && ((debrisID < DataBlockObjectIdFirst) || (debrisID > DataBlockObjectIdLast))),
            "ShapeBaseData::preload: invalid debris data");
      }


      if( debrisShapeName && debrisShapeName[0] != '\0' && !bool(debrisShape) )
      {
         debrisShape = ResourceManager::get().load(debrisShapeName);
         if( bool(debrisShape) == false )
         {
            errorStr = String::ToString("ShapeBaseData::load: Couldn't load shape \"%s\"", debrisShapeName);
            return false;
         }
         else
         {
            if(!server && !debrisShape->preloadMaterialList(debrisShape.getPath()) && NetConnection::filesWereDownloaded())
               shapeError = true;

            TSShapeInstance* pDummy = new TSShapeInstance(debrisShape, !server);
            delete pDummy;
         }
      }
   }

   //
   if (shapeName && shapeName[0]) {
      S32 i;

      // Resolve shapename
      mShape = ResourceManager::get().load(shapeName);
      if (bool(mShape) == false)
      {
         errorStr = String::ToString("ShapeBaseData: Couldn't load shape \"%s\"",shapeName);
         return false;
      }
      if(!server && !mShape->preloadMaterialList(mShape.getPath()) && NetConnection::filesWereDownloaded())
         shapeError = true;

      if(computeCRC)
      {
         Con::printf("Validation required for shape: %s", shapeName);

         Torque::FS::FileNodeRef    fileRef = Torque::FS::GetFileNode(mShape.getPath());

         if (!fileRef)
         {
            errorStr = String::ToString("ShapeBaseData: Couldn't load shape \"%s\"",shapeName);
            return false;
         }

         if(server)
            mCRC = fileRef->getChecksum();
         else if(mCRC != fileRef->getChecksum())
         {
            errorStr = String::ToString("Shape \"%s\" does not match version on server.",shapeName);
            return false;
         }
      }
      // Resolve details and camera node indexes.
      static const String sCollisionStr( "collision-" );

      for (i = 0; i < mShape->details.size(); i++)
      {
         const String &name = mShape->names[mShape->details[i].nameIndex];

         if (name.compare( sCollisionStr, sCollisionStr.length(), String::NoCase ) == 0)
         {
            collisionDetails.push_back(i);
            collisionBounds.increment();

            mShape->computeBounds(collisionDetails.last(), collisionBounds.last());
            mShape->getAccelerator(collisionDetails.last());

            if (!mShape->bounds.isContained(collisionBounds.last()))
            {
               Con::warnf("Warning: shape %s collision detail %d (Collision-%d) bounds exceed that of shape.", shapeName, collisionDetails.size() - 1, collisionDetails.last());
               collisionBounds.last() = mShape->bounds;
            }
            else if (collisionBounds.last().isValidBox() == false)
            {
               Con::errorf("Error: shape %s-collision detail %d (Collision-%d) bounds box invalid!", shapeName, collisionDetails.size() - 1, collisionDetails.last());
               collisionBounds.last() = mShape->bounds;
            }

            // The way LOS works is that it will check to see if there is a LOS detail that matches
            // the the collision detail + 1 + MaxCollisionShapes (this variable name should change in
            // the future). If it can't find a matching LOS it will simply use the collision instead.
            // We check for any "unmatched" LOS's further down
            LOSDetails.increment();

            String   buff = String::ToString("LOS-%d", i + 1 + MaxCollisionShapes);
            U32 los = mShape->findDetail(buff);
            if (los == -1)
               LOSDetails.last() = i;
            else
               LOSDetails.last() = los;
         }
      }

      // Snag any "unmatched" LOS details
      static const String sLOSStr( "LOS-" );

      for (i = 0; i < mShape->details.size(); i++)
      {
         const String &name = mShape->names[mShape->details[i].nameIndex];

         if (name.compare( sLOSStr, sLOSStr.length(), String::NoCase ) == 0)
         {
            // See if we already have this LOS
            bool found = false;
            for (U32 j = 0; j < LOSDetails.size(); j++)
            {
               if (LOSDetails[j] == i)
               {
                     found = true;
                     break;
               }
            }

            if (!found)
               LOSDetails.push_back(i);
         }
      }

      debrisDetail = mShape->findDetail("Debris-17");
      eyeNode = mShape->findNode("eye");
      earNode = mShape->findNode( "ear" );
      if( earNode == -1 )
         earNode = eyeNode;
      cameraNode = mShape->findNode("cam");
      if (cameraNode == -1)
         cameraNode = eyeNode;

      // Resolve mount point node indexes
      for (i = 0; i < SceneObject::NumMountPoints; i++) {
         char fullName[256];
         dSprintf(fullName,sizeof(fullName),"mount%d",i);
         mountPointNode[i] = mShape->findNode(fullName);
      }

        // find the AIRepairNode - hardcoded to be the last node in the array...
      mountPointNode[AIRepairNode] = mShape->findNode("AIRepairNode");

      //
      hulkSequence = mShape->findSequence("Visibility");
      damageSequence = mShape->findSequence("Damage");

      //
      F32 w = mShape->bounds.len_y() / 2;
      if (cameraMaxDist < w)
         cameraMaxDist = w;
   }

   if(!server)
   {
/*
      // grab all the hud images
      for(U32 i = 0; i < NumHudRenderImages; i++)
      {
         if(hudImageNameFriendly[i] && hudImageNameFriendly[i][0])
            hudImageFriendly[i] = TextureHandle(hudImageNameFriendly[i], BitmapTexture);

         if(hudImageNameEnemy[i] && hudImageNameEnemy[i][0])
            hudImageEnemy[i] = TextureHandle(hudImageNameEnemy[i], BitmapTexture);
      }
*/
   }

   // Resolve CubeReflectorDesc.
   if ( cubeDescName.isNotEmpty() )
   {
      Sim::findObject( cubeDescName, reflectorDesc );
   }
   else if( cubeDescId > 0 )
   {
      Sim::findObject( cubeDescId, reflectorDesc );
   }

   return !shapeError;
}

bool ShapeBaseData::_setMass( void* object, const char* index, const char* data )
{
   ShapeBaseData* shape = reinterpret_cast< ShapeBaseData* >( object );

	F32 mass = dAtof(data);

	if (mass <= 0)
		mass = 0.01f;

	shape->mass = mass;

   return false;
}


void ShapeBaseData::initPersistFields()
{
   addGroup( "Shadows" );

      addField( "shadowEnable", TypeBool, Offset(shadowEnable, ShapeBaseData),
         "Enable shadows for this shape (currently unused, shadows are always enabled)." );
      addField( "shadowSize", TypeS32, Offset(shadowSize, ShapeBaseData),
         "Size of the projected shadow texture (must be power of 2)." );
      addField( "shadowMaxVisibleDistance", TypeF32, Offset(shadowMaxVisibleDistance, ShapeBaseData),
         "Maximum distance at which shadow is visible (currently unused)." );
      addField( "shadowProjectionDistance", TypeF32, Offset(shadowProjectionDistance, ShapeBaseData),
         "Maximum height above ground to project shadow. If the object is higher "
         "than this no shadow will be rendered." );
      addField( "shadowSphereAdjust", TypeF32, Offset(shadowSphereAdjust, ShapeBaseData),
         "Scalar applied to the radius of spot shadows (initial radius is based "
         "on the shape bounds but can be adjusted with this field)." );

   endGroup( "Shadows" );

   addGroup( "Render" );

      addField( "shapeFile", TypeShapeFilename, Offset(shapeName, ShapeBaseData),
         "The DTS or DAE model to use for this object." );

   endGroup( "Render" );

   addGroup( "Destruction", "Parameters related to the destruction effects of this object." );

      addField( "explosion", TYPEID< ExplosionData >(), Offset(explosion, ShapeBaseData),
         "%Explosion to generate when this shape is blown up." );
      addField( "underwaterExplosion", TYPEID< ExplosionData >(), Offset(underwaterExplosion, ShapeBaseData),
         "%Explosion to generate when this shape is blown up underwater." );
      addField( "debris", TYPEID< DebrisData >(), Offset(debris, ShapeBaseData),
         "%Debris to generate when this shape is blown up." );
      addField( "renderWhenDestroyed", TypeBool, Offset(renderWhenDestroyed, ShapeBaseData),
         "Whether to render the shape when it is in the \"Destroyed\" damage state." );
      addField( "debrisShapeName", TypeShapeFilename, Offset(debrisShapeName, ShapeBaseData),
         "The DTS or DAE model to use for auto-generated breakups. @note may not be functional." );

   endGroup( "Destruction" );

   addGroup( "Physics" );
   
      addProtectedField("mass", TypeF32, Offset(mass, ShapeBaseData), &_setMass, &defaultProtectedGetFn, "Shape mass.\nUsed in simulation of moving objects.\n"  );
      addField( "drag", TypeF32, Offset(drag, ShapeBaseData),
         "Drag factor.\nReduces velocity of moving objects." );
      addField( "density", TypeF32, Offset(density, ShapeBaseData),
         "Shape density.\nUsed when computing buoyancy when in water.\n" );

   endGroup( "Physics" );

   addGroup( "Damage/Energy" );

      addField( "maxEnergy", TypeF32, Offset(maxEnergy, ShapeBaseData),
         "Maximum energy level for this object." );
      addField( "maxDamage", TypeF32, Offset(maxDamage, ShapeBaseData),
         "Maximum damage level for this object." );
      addField( "disabledLevel", TypeF32, Offset(disabledLevel, ShapeBaseData),
         "Damage level above which the object is disabled.\n"
         "Currently unused." );
      addField( "destroyedLevel", TypeF32, Offset(destroyedLevel, ShapeBaseData),
         "Damage level above which the object is destroyed.\n"
         "When the damage level increases above this value, the object damage "
         "state is set to \"Destroyed\"." );
      addField( "repairRate", TypeF32, Offset(repairRate, ShapeBaseData),
         "Rate at which damage is repaired in damage units/tick.\n"
         "This value is subtracted from the damage level until it reaches 0." );
      addField( "inheritEnergyFromMount", TypeBool, Offset(inheritEnergyFromMount, ShapeBaseData),
         "Flag controlling whether to manage our own energy level, or to use "
         "the energy level of the object we are mounted to." );
      addField( "isInvincible", TypeBool, Offset(isInvincible, ShapeBaseData),
         "Invincible flag; when invincible, the object cannot be damaged or "
         "repaired." );

   endGroup( "Damage/Energy" );

   addGroup( "Camera", "The settings used by the shape when it is the camera." );

      addField( "cameraMaxDist", TypeF32, Offset(cameraMaxDist, ShapeBaseData),
         "The maximum distance from the camera to the object.\n"
         "Used when computing a custom camera transform for this object.\n\n"
         "@see observeThroughObject" );
      addField( "cameraMinDist", TypeF32, Offset(cameraMinDist, ShapeBaseData),
         "The minimum distance from the camera to the object.\n"
         "Used when computing a custom camera transform for this object.\n\n"
         "@see observeThroughObject" );
      addField( "cameraDefaultFov", TypeF32, Offset(cameraDefaultFov, ShapeBaseData),
         "The default camera vertical FOV in degrees." );
      addField( "cameraMinFov", TypeF32, Offset(cameraMinFov, ShapeBaseData),
         "The minimum camera vertical FOV allowed in degrees." );
      addField( "cameraMaxFov", TypeF32, Offset(cameraMaxFov, ShapeBaseData),
         "The maximum camera vertical FOV allowed in degrees." );
      addField( "cameraCanBank", TypeBool, Offset(cameraCanBank, ShapeBaseData),
         "If the derrived class supports it, allow the camera to bank." );
      addField( "mountedImagesBank", TypeBool, Offset(mountedImagesBank, ShapeBaseData),
         "Do mounted images bank along with the camera?" );
      addField( "firstPersonOnly", TypeBool, Offset(firstPersonOnly, ShapeBaseData),
         "Flag controlling whether the view from this object is first person "
         "only." );
      addField( "useEyePoint", TypeBool, Offset(useEyePoint, ShapeBaseData),
         "Flag controlling whether the client uses this object's eye point to "
         "view from." );
      addField( "observeThroughObject", TypeBool, Offset(observeThroughObject, ShapeBaseData),
         "Observe this object through its camera transform and default fov.\n"
         "If true, when this object is the camera it can provide a custom camera "
         "transform and FOV (instead of the default eye transform)." );

   endGroup("Camera");

   addGroup( "Misc" );

      addField( "computeCRC", TypeBool, Offset(computeCRC, ShapeBaseData),
         "If true, verify that the CRC of the client's shape model matches the "
         "server's CRC for the shape model when loaded by the client." );

   endGroup( "Misc" );

   addGroup( "Reflection" );

      addField( "cubeReflectorDesc", TypeRealString, Offset( cubeDescName, ShapeBaseData ), 
         "References a ReflectorDesc datablock that defines performance and quality properties for dynamic reflections.\n");
      //addField( "reflectMaxRateMs", TypeS32, Offset( reflectMaxRateMs, ShapeBaseData ), "reflection will not be updated more frequently than this" );
      //addField( "reflectMaxDist", TypeF32, Offset( reflectMaxDist, ShapeBaseData ), "distance at which reflection is never updated" );
      //addField( "reflectMinDist", TypeF32, Offset( reflectMinDist, ShapeBaseData ), "distance at which reflection is always updated" );
      //addField( "reflectDetailAdjust", TypeF32, Offset( reflectDetailAdjust, ShapeBaseData ), "scale up or down the detail level for objects rendered in a reflection" );

   endGroup( "Reflection" );

   Parent::initPersistFields();
}

DefineEngineMethod( ShapeBaseData, checkDeployPos, bool, ( TransformF txfm ),,
   "@brief Check if there is the space at the given transform is free to spawn into.\n\n"

   "The shape's bounding box volume is used to check for collisions at the given world "
   "transform.  Only interior and static objects are checked for collision.\n"

   "@param txfm Deploy transform to check\n"
   "@return True if the space is free, false if there is already something in "
   "the way.\n"

   "@note This is a server side only check, and is not actually limited to spawning.\n")
{
   if (bool(object->mShape) == false)
      return false;

   MatrixF mat = txfm.getMatrix();

   Box3F objBox = object->mShape->bounds;
   Point3F boxCenter = (objBox.minExtents + objBox.maxExtents) * 0.5f;
   objBox.minExtents = boxCenter + (objBox.minExtents - boxCenter) * 0.9f;
   objBox.maxExtents = boxCenter + (objBox.maxExtents - boxCenter) * 0.9f;

   Box3F wBox = objBox;
   mat.mul(wBox);

   EarlyOutPolyList polyList;
   polyList.mNormal.set(0,0,0);
   polyList.mPlaneList.clear();
   polyList.mPlaneList.setSize(6);
   polyList.mPlaneList[0].set(objBox.minExtents,VectorF(-1,0,0));
   polyList.mPlaneList[1].set(objBox.maxExtents,VectorF(0,1,0));
   polyList.mPlaneList[2].set(objBox.maxExtents,VectorF(1,0,0));
   polyList.mPlaneList[3].set(objBox.minExtents,VectorF(0,-1,0));
   polyList.mPlaneList[4].set(objBox.minExtents,VectorF(0,0,-1));
   polyList.mPlaneList[5].set(objBox.maxExtents,VectorF(0,0,1));

   for (U32 i = 0; i < 6; i++)
   {
      PlaneF temp;
      mTransformPlane(mat, Point3F(1, 1, 1), polyList.mPlaneList[i], &temp);
      polyList.mPlaneList[i] = temp;
   }

   if (gServerContainer.buildPolyList(PLC_Collision, wBox, StaticShapeObjectType, &polyList))
      return false;
   return true;
}


DefineEngineMethod(ShapeBaseData, getDeployTransform, TransformF, ( Point3F pos, Point3F normal ),,
   "@brief Helper method to get a transform from a position and vector (suitable for use with setTransform).\n\n"
   "@param pos Desired transform position\n"
   "@param normal Vector of desired direction\n"
   "@return The deploy transform\n" )
{
   normal.normalize();

   VectorF xAxis;
   if( mFabs(normal.z) > mFabs(normal.x) && mFabs(normal.z) > mFabs(normal.y))
      mCross( VectorF( 0, 1, 0 ), normal, &xAxis );
   else
      mCross( VectorF( 0, 0, 1 ), normal, &xAxis );

   VectorF yAxis;
   mCross( normal, xAxis, &yAxis );

   MatrixF testMat(true);
   testMat.setColumn( 0, xAxis );
   testMat.setColumn( 1, yAxis );
   testMat.setColumn( 2, normal );
   testMat.setPosition( pos );

   return testMat;
}

void ShapeBaseData::packData(BitStream* stream)
{
   Parent::packData(stream);

   if(stream->writeFlag(computeCRC))
      stream->write(mCRC);

   stream->writeFlag(shadowEnable);
   stream->write(shadowSize);
   stream->write(shadowMaxVisibleDistance);
   stream->write(shadowProjectionDistance);
   stream->write(shadowSphereAdjust);


   stream->writeString(shapeName);
   stream->writeString(cloakTexName);
   if(stream->writeFlag(mass != gShapeBaseDataProto.mass))
      stream->write(mass);
   if(stream->writeFlag(drag != gShapeBaseDataProto.drag))
      stream->write(drag);
   if(stream->writeFlag(density != gShapeBaseDataProto.density))
      stream->write(density);
   if(stream->writeFlag(maxEnergy != gShapeBaseDataProto.maxEnergy))
      stream->write(maxEnergy);
   if(stream->writeFlag(cameraMaxDist != gShapeBaseDataProto.cameraMaxDist))
      stream->write(cameraMaxDist);
   if(stream->writeFlag(cameraMinDist != gShapeBaseDataProto.cameraMinDist))
      stream->write(cameraMinDist);
   cameraDefaultFov = mClampF(cameraDefaultFov, cameraMinFov, cameraMaxFov);
   if(stream->writeFlag(cameraDefaultFov != gShapeBaseDataProto.cameraDefaultFov))
      stream->write(cameraDefaultFov);
   if(stream->writeFlag(cameraMinFov != gShapeBaseDataProto.cameraMinFov))
      stream->write(cameraMinFov);
   if(stream->writeFlag(cameraMaxFov != gShapeBaseDataProto.cameraMaxFov))
      stream->write(cameraMaxFov);
   stream->writeFlag(cameraCanBank);
   stream->writeFlag(mountedImagesBank);
   stream->writeString( debrisShapeName );

   stream->writeFlag(observeThroughObject);

   if( stream->writeFlag( debris != NULL ) )
   {
      stream->writeRangedU32(packed? SimObjectId((uintptr_t)debris):
                             debris->getId(),DataBlockObjectIdFirst,DataBlockObjectIdLast);
   }

   stream->writeFlag(isInvincible);
   stream->writeFlag(renderWhenDestroyed);

   if( stream->writeFlag( explosion != NULL ) )
   {
      stream->writeRangedU32( explosion->getId(), DataBlockObjectIdFirst,  DataBlockObjectIdLast );
   }

   if( stream->writeFlag( underwaterExplosion != NULL ) )
   {
      stream->writeRangedU32( underwaterExplosion->getId(), DataBlockObjectIdFirst,  DataBlockObjectIdLast );
   }

   stream->writeFlag(inheritEnergyFromMount);
   stream->writeFlag(firstPersonOnly);
   stream->writeFlag(useEyePoint);
   
   if( stream->writeFlag( reflectorDesc != NULL ) )
   {
      stream->writeRangedU32( reflectorDesc->getId(), DataBlockObjectIdFirst,  DataBlockObjectIdLast );
   }

   //stream->write(reflectPriority);
   //stream->write(reflectMaxRateMs);
   //stream->write(reflectMinDist);
   //stream->write(reflectMaxDist);
   //stream->write(reflectDetailAdjust);
}

void ShapeBaseData::unpackData(BitStream* stream)
{
   Parent::unpackData(stream);
   computeCRC = stream->readFlag();
   if(computeCRC)
      stream->read(&mCRC);

   shadowEnable = stream->readFlag();
   stream->read(&shadowSize);
   stream->read(&shadowMaxVisibleDistance);
   stream->read(&shadowProjectionDistance);
   stream->read(&shadowSphereAdjust);

   shapeName = stream->readSTString();
   cloakTexName = stream->readSTString();
   if(stream->readFlag())
      stream->read(&mass);
   else
      mass = gShapeBaseDataProto.mass;

   if(stream->readFlag())
      stream->read(&drag);
   else
      drag = gShapeBaseDataProto.drag;

   if(stream->readFlag())
      stream->read(&density);
   else
      density = gShapeBaseDataProto.density;

   if(stream->readFlag())
      stream->read(&maxEnergy);
   else
      maxEnergy = gShapeBaseDataProto.maxEnergy;

   if(stream->readFlag())
      stream->read(&cameraMaxDist);
   else
      cameraMaxDist = gShapeBaseDataProto.cameraMaxDist;

   if(stream->readFlag())
      stream->read(&cameraMinDist);
   else
      cameraMinDist = gShapeBaseDataProto.cameraMinDist;

   if(stream->readFlag())
      stream->read(&cameraDefaultFov);
   else
      cameraDefaultFov = gShapeBaseDataProto.cameraDefaultFov;

   if(stream->readFlag())
      stream->read(&cameraMinFov);
   else
      cameraMinFov = gShapeBaseDataProto.cameraMinFov;

   if(stream->readFlag())
      stream->read(&cameraMaxFov);
   else
      cameraMaxFov = gShapeBaseDataProto.cameraMaxFov;

   cameraCanBank = stream->readFlag();
   mountedImagesBank = stream->readFlag();

   debrisShapeName = stream->readSTString();

   observeThroughObject = stream->readFlag();

   if( stream->readFlag() )
   {
      debrisID = stream->readRangedU32( DataBlockObjectIdFirst, DataBlockObjectIdLast );
   }

   isInvincible = stream->readFlag();
   renderWhenDestroyed = stream->readFlag();

   if( stream->readFlag() )
   {
      explosionID = stream->readRangedU32( DataBlockObjectIdFirst, DataBlockObjectIdLast );
   }

   if( stream->readFlag() )
   {
      underwaterExplosionID = stream->readRangedU32( DataBlockObjectIdFirst, DataBlockObjectIdLast );
   }

   inheritEnergyFromMount = stream->readFlag();
   firstPersonOnly = stream->readFlag();
   useEyePoint = stream->readFlag();

   if( stream->readFlag() )
   {
      cubeDescId = stream->readRangedU32( DataBlockObjectIdFirst, DataBlockObjectIdLast );
   }

   //stream->read(&reflectPriority);
   //stream->read(&reflectMaxRateMs);
   //stream->read(&reflectMinDist);
   //stream->read(&reflectMaxDist);
   //stream->read(&reflectDetailAdjust);
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

Chunker<ShapeBase::CollisionTimeout> sTimeoutChunker;
ShapeBase::CollisionTimeout* ShapeBase::sFreeTimeoutList = 0;


//----------------------------------------------------------------------------

IMPLEMENT_CO_NETOBJECT_V1(ShapeBase);

ConsoleDocClass( ShapeBase,
   "@ingroup gameObjects"
);

IMPLEMENT_CALLBACK( ShapeBase, validateCameraFov, F32, (F32 fov), (fov),
   "@brief Called on the server when the client has requested a FOV change.\n\n"

   "When the client requests that its field of view should be changed (because "
   "they want to use a sniper scope, for example) this new FOV needs to be validated "
   "by the server.  This method is called if it exists (it is optional) to validate "
   "the requested FOV, and modify it if necessary.  This could be as simple as checking "
   "that the FOV falls within a correct range, to making sure that the FOV matches the "
   "capabilities of the current weapon.\n\n"

   "Following this method, ShapeBase ensures that the given FOV still falls within "
   "the datablock's cameraMinFov and cameraMaxFov.  If that is good enough for your "
   "purposes, then you do not need to define the validateCameraFov() callback for "
   "your ShapeBase.\n\n"

   "@param fov The FOV that has been requested by the client.\n"
   "@return The FOV as validated by the server.\n\n"

   "@see ShapeBaseData\n\n");

ShapeBase::ShapeBase()
 : mDataBlock( NULL ),
   mIsAiControlled( false ),
   mAiPose( 0 ),
   mControllingObject( NULL ),
   mMoveMotion( false ),
   mShapeBaseMount( NULL ),
   mShapeInstance( NULL ),
   mConvexList( new Convex ),
   mEnergy( 0.0f ),
   mRechargeRate( 0.0f ),
   mMass( 1.0f ),
   mOneOverMass( 1.0f ),
   mDrag( 0.0f ),
   mBuoyancy( 0.0f ),
   mLiquidHeight( 0.0f ),
   mWaterCoverage( 0.0f ),
   mAppliedForce( Point3F::Zero ),
   mGravityMod( 1.0f ),
   mDamageFlash( 0.0f ),
   mWhiteOut( 0.0f ),
   mFlipFadeVal( false ),
   mTimeoutList( NULL ),
   mDamage( 0.0f ),
   mRepairRate( 0.0f ),
   mRepairReserve( 0.0f ),
   mDamageState( Enabled ),
   mDamageThread( NULL ),
   mHulkThread( NULL ),
   damageDir( 0.0f, 0.0f, 1.0f ),
   mCloaked( false ),
   mCloakLevel( 0.0f ),
   mFadeOut( true ),
   mFading( false ),
   mFadeVal( 1.0f ),
   mFadeElapsedTime( 0.0f ),
   mFadeTime( 1.0f ),
   mFadeDelay( 0.0f ),
   mCameraFov( 90.0f ),
   mIsControlled( false ),
   mLastRenderFrame( 0 ),
   mLastRenderDistance( 0.0f )
{
   mTypeMask |= ShapeBaseObjectType | LightObjectType;   

   S32 i;

   for (i = 0; i < MaxSoundThreads; i++) {
      mSoundThread[i].play = false;
      mSoundThread[i].profile = 0;
      mSoundThread[i].sound = 0;
   }

   for (i = 0; i < MaxScriptThreads; i++) {
      mScriptThread[i].sequence = -1;
      mScriptThread[i].thread = 0;
      mScriptThread[i].state = Thread::Stop;
      mScriptThread[i].atEnd = false;
	   mScriptThread[i].timescale = 1.f;
	   mScriptThread[i].position = -1.f;
   }

   for (i = 0; i < MaxTriggerKeys; i++)
      mTrigger[i] = false;
}


ShapeBase::~ShapeBase()
{
   SAFE_DELETE( mConvexList );

   AssertFatal(mMount.link == 0,"ShapeBase::~ShapeBase: An object is still mounted");
   if( mShapeInstance && (mShapeInstance->getDebrisRefCount() == 0) )
   {
      delete mShapeInstance;
      mShapeInstance = NULL;
   }

   CollisionTimeout* ptr = mTimeoutList;
   while (ptr) {
      CollisionTimeout* cur = ptr;
      ptr = ptr->next;
      cur->next = sFreeTimeoutList;
      sFreeTimeoutList = cur;
   }
}

void ShapeBase::initPersistFields()
{
   addProtectedField( "skin", TypeRealString, Offset(mAppliedSkinName, ShapeBase), &_setFieldSkin, &_getFieldSkin,
      "@brief The skin applied to the shape.\n\n"

      "'Skinning' the shape effectively renames the material targets, allowing "
      "different materials to be used on different instances of the same model. "
      "Using getSkinName() and setSkinName() is equivalent to reading and "
      "writing the skin field directly.\n\n"

      "Any material targets that start with the old skin name have that part "
      "of the name replaced with the new skin name. The initial old skin name is "
      "\"base\". For example, if a new skin of \"blue\" was applied to a model "
      "that had material targets <i>base_body</i> and <i>face</i>, the new targets "
      "would be <i>blue_body</i> and <i>face</i>. Note that <i>face</i> was not "
      "renamed since it did not start with the old skin name of \"base\".\n\n"

      "To support models that do not use the default \"base\" naming convention, "
      "you can also specify the part of the name to replace in the skin field "
      "itself. For example, if a model had a material target called <i>shapemat</i>, "
      "we could apply a new skin \"shape=blue\", and the material target would be "
      "renamed to <i>bluemat</i> (note \"shape\" has been replaced with \"blue\").\n\n"

      "Multiple skin updates can also be applied at the same time by separating "
      "them with a semicolon. For example: \"base=blue;face=happy_face\".\n\n"

      "Material targets are only renamed if an existing Material maps to that "
      "name, or if there is a diffuse texture in the model folder with the same "
      "name as the new target.\n\n" );

   addField( "isAiControlled", TypeBool, Offset(mIsAiControlled, ShapeBase),
      "@brief Is this object AI controlled.\n\n"
      "If True then this object is considered AI controlled and not player controlled.\n" );

   Parent::initPersistFields();
}

bool ShapeBase::_setFieldSkin( void *object, const char *index, const char *data )
{
   ShapeBase* shape = static_cast<ShapeBase*>( object );
   if ( shape )
      shape->setSkinName( data );
   return false;
}

const char *ShapeBase::_getFieldSkin( void *object, const char *data )
{
   ShapeBase* shape = static_cast<ShapeBase*>( object );
   return shape ? shape->getSkinName() : "";
}

//----------------------------------------------------------------------------

bool ShapeBase::onAdd()
{
   if( !Parent::onAdd() )
      return false;
      
   if( !mDataBlock )
   {
      Con::errorf( "ShapeBase::onAdd - no datablock on shape %i:%s (%s)",
         getId(), getClassName(), getName() );
      return false;
   }

   // Resolve sounds that arrived in the initial update
   S32 i;
   for (i = 0; i < MaxSoundThreads; i++)
      updateAudioState(mSoundThread[i]);

   for (i = 0; i < MaxScriptThreads; i++)
   {
      Thread& st = mScriptThread[i];
      if(st.thread)
         updateThread(st);
   }   

/*
      if(mDataBlock->cloakTexName != StringTable->insert(""))
        mCloakTexture = TextureHandle(mDataBlock->cloakTexName, MeshTexture, false);
*/         
   // Accumulation and environment mapping
   if (isClientObject() && mShapeInstance)
   {
      AccumulationVolume::addObject(this);
   }
   return true;
}

void ShapeBase::onRemove()
{
   mConvexList->nukeList();

   Parent::onRemove();

   // Stop any running sounds on the client
   if (isGhost())
      for (S32 i = 0; i < MaxSoundThreads; i++)
         stopAudio(i);

   if ( isClientObject() )   
   {
      mCubeReflector.unregisterReflector();
   }
}


void ShapeBase::onSceneRemove()
{
   mConvexList->nukeList();
   Parent::onSceneRemove();
}

bool ShapeBase::onNewDataBlock( GameBaseData *dptr, bool reload )
{
   ShapeBaseData *prevDB = dynamic_cast<ShapeBaseData*>( mDataBlock );

   bool isInitialDataBlock = ( mDataBlock == 0 );

   if ( Parent::onNewDataBlock( dptr, reload ) == false )
      return false;

   mDataBlock = dynamic_cast<ShapeBaseData*>(dptr);
   if (!mDataBlock)
      return false;

   setMaskBits(DamageMask);
   mDamageThread = 0;
   mHulkThread = 0;

   // Even if loadShape succeeds, there may not actually be
   // a shape assigned to this object.
   if (bool(mDataBlock->mShape)) {
      delete mShapeInstance;
      mShapeInstance = new TSShapeInstance(mDataBlock->mShape, isClientObject());
      if (isClientObject())
         mShapeInstance->cloneMaterialList();

      mObjBox = mDataBlock->mShape->bounds;
      resetWorldBox();

      // Set the initial mesh hidden state.
      mMeshHidden.setSize( mDataBlock->mShape->objects.size() );
      mMeshHidden.clear();

      // Initialize the threads
      for (U32 i = 0; i < MaxScriptThreads; i++) {
         Thread& st = mScriptThread[i];
         if (st.sequence != -1) {
            // TG: Need to see about suppressing non-cyclic sounds
            // if the sequences were activated before the object was
            // ghosted.
            // TG: Cyclic animations need to have a random pos if
            // they were started before the object was ghosted.

            // If there was something running on the old shape, the thread
            // needs to be reset. Otherwise we assume that it's been
            // initialized either by the constructor or from the server.
            bool reset = st.thread != 0;
            st.thread = 0;
            
            // New datablock/shape may not actually HAVE this sequence.
            // In that case stop playing it.
            
            AssertFatal( prevDB != NULL, "ShapeBase::onNewDataBlock - how did you have a sequence playing without a prior datablock?" );
   
            const TSShape *prevShape = prevDB->mShape;
            const TSShape::Sequence &prevSeq = prevShape->sequences[st.sequence];
            const String &prevSeqName = prevShape->names[prevSeq.nameIndex];

            st.sequence = mDataBlock->mShape->findSequence( prevSeqName );

            if ( st.sequence != -1 )
            {
               setThreadSequence( i, st.sequence, reset );                              
            }            
         }
      }

      if (mDataBlock->damageSequence != -1) {
         mDamageThread = mShapeInstance->addThread();
         mShapeInstance->setSequence(mDamageThread,
                                     mDataBlock->damageSequence,0);
      }
      if (mDataBlock->hulkSequence != -1) {
         mHulkThread = mShapeInstance->addThread();
         mShapeInstance->setSequence(mHulkThread,
                                     mDataBlock->hulkSequence,0);
      }

      if( isGhost() )
      {
         // Reapply the current skin
         mAppliedSkinName = "";
         reSkin();
      }
   }

   //
   mEnergy = 0;
   mDamage = 0;
   mDamageState = Enabled;
   mRepairReserve = 0;
   updateMass();
   updateDamageLevel();
   updateDamageState();

   mDrag = mDataBlock->drag;
   mCameraFov = mDataBlock->cameraDefaultFov;
   updateMass();

   if( !isInitialDataBlock && mLightPlugin )
      mLightPlugin->reset();
   
   if ( isClientObject() )
   {      
      mCubeReflector.unregisterReflector();

      if ( mDataBlock->reflectorDesc )
         mCubeReflector.registerReflector( this, mDataBlock->reflectorDesc );      
   }

   return true;
}

void ShapeBase::onDeleteNotify( SimObject *obj )
{
   if ( obj == mCurrentWaterObject )
      mCurrentWaterObject = NULL;

   Parent::onDeleteNotify( obj );      
}

void ShapeBase::onImpact(SceneObject* obj, const VectorF& vec)
{
   if (!isGhost())
      mDataBlock->onImpact_callback( this, obj, vec, vec.len() );
}

void ShapeBase::onImpact(const VectorF& vec)
{
   if (!isGhost())
      mDataBlock->onImpact_callback( this, NULL, vec, vec.len() );
}


//----------------------------------------------------------------------------

void ShapeBase::processTick(const Move* move)
{
   PROFILE_SCOPE( ShapeBase_ProcessTick );

   // Energy management
   if (mDamageState == Enabled && mDataBlock->inheritEnergyFromMount == false) {
      F32 store = mEnergy;
      mEnergy += mRechargeRate;
      if (mEnergy > mDataBlock->maxEnergy)
         mEnergy = mDataBlock->maxEnergy;
      else
         if (mEnergy < 0)
            mEnergy = 0;

      // Virtual setEnergyLevel is used here by some derived classes to
      // decide whether they really want to set the energy mask bit.
      if (mEnergy != store)
         setEnergyLevel(mEnergy);
   }

   // Repair management
   if (mDataBlock->isInvincible == false)
   {
      F32 store = mDamage;
      mDamage -= mRepairRate;
      mDamage = mClampF(mDamage, 0.f, mDataBlock->maxDamage);

      if (mRepairReserve > mDamage)
         mRepairReserve = mDamage;
      if (mRepairReserve > 0.0)
      {
         F32 rate = getMin(mDataBlock->repairRate, mRepairReserve);
         mDamage -= rate;
         mRepairReserve -= rate;
      }

      if (store != mDamage)
      {
         updateDamageLevel();
         if (isServerObject()) {
            setMaskBits(DamageMask);
            mDataBlock->onDamage_callback( this, mDamage - store );
         }
      }
   }

   if (isServerObject()) {
      // Server only...
      advanceThreads(TickSec);
      updateServerAudio();

      // update wet state
      setImageWetState(0, mWaterCoverage > 0.4); // more than 40 percent covered

      // update motion state
      mMoveMotion = false;
      if (move && (move->x || move->y || move->z))
      {
         mMoveMotion = true;
      }
      for (S32 i = 0; i < MaxMountedImages; i++)
      {
         setImageMotionState(i, mMoveMotion);
      }

      if(mFading)
      {
         F32 dt = TickMs / 1000.0f;
         F32 newFadeET = mFadeElapsedTime + dt;
         if(mFadeElapsedTime < mFadeDelay && newFadeET >= mFadeDelay)
            setMaskBits(CloakMask);
         mFadeElapsedTime = newFadeET;
         if(mFadeElapsedTime > mFadeTime + mFadeDelay)
         {
            mFadeVal = F32(!mFadeOut);
            mFading = false;
         }
      }
   }

   // Advance images
   if (isServerObject())
   {
      for (S32 i = 0; i < MaxMountedImages; i++)
      {
         if (mMountedImageList[i].dataBlock)
            updateImageState(i, TickSec);
      }
   }

   // Call script on trigger state changes
   if (move && mDataBlock && isServerObject()) 
   {
      for (S32 i = 0; i < MaxTriggerKeys; i++) 
      {
         if (move->trigger[i] != mTrigger[i]) 
         {
            mTrigger[i] = move->trigger[i];
            mDataBlock->onTrigger_callback( this, i, move->trigger[i] );
         }
      }
   }

   // Update the damage flash and the whiteout
   //
   if (mDamageFlash > 0.0)
   {
      mDamageFlash -= sDamageFlashDec;
      if (mDamageFlash <= 0.0)
         mDamageFlash = 0.0;
   }
   if (mWhiteOut > 0.0)
   {
      mWhiteOut -= sWhiteoutDec;
      if (mWhiteOut <= 0.0)
         mWhiteOut = 0.0;
   }

   if (isMounted()) {
      MatrixF mat;
      mMount.object->getMountTransform( mMount.node, mMount.xfm, &mat );
      Parent::setTransform(mat);
   }
}

void ShapeBase::advanceTime(F32 dt)
{
   // On the client, the shape threads and images are
   // advanced at framerate.
   advanceThreads(dt);
   updateAudioPos();
   for (S32 i = 0; i < MaxMountedImages; i++)
      if (mMountedImageList[i].dataBlock)
      {
         updateImageState(i, dt);
         updateImageAnimation(i, dt);
      }

   // Cloaking
   if (mCloaked && mCloakLevel != 1.0) {
      if (sCloakSpeed <= 0.0f)
      {
         // Instantaneous
         mCloakLevel = 1.0;
      }
      else
      {
         // Over time determined by sCloakSpeed
         mCloakLevel += dt / sCloakSpeed;
         if (mCloakLevel >= 1.0)
            mCloakLevel = 1.0;
      }
   } else if (!mCloaked && mCloakLevel != 0.0) {
      if (sCloakSpeed <= 0.0f)
      {
         // Instantaneous
         mCloakLevel = 0.0;
      }
      else
      {
         // Over time determined by sCloakSpeed
         mCloakLevel -= dt / sCloakSpeed;
         if (mCloakLevel <= 0.0)
            mCloakLevel = 0.0;
      }
   }
   if(mFading)
   {
      mFadeElapsedTime += dt;
      if(mFadeElapsedTime > mFadeTime)
      {
         mFadeVal = F32(!mFadeOut);
         mFading = false;
      }
      else
      {
         mFadeVal = mFadeElapsedTime / mFadeTime;
         if(mFadeOut)
            mFadeVal = 1 - mFadeVal;
      }
   }

   if (isMounted()) {
      MatrixF mat;
      mMount.object->getRenderMountTransform( 0.0f, mMount.node, mMount.xfm, &mat );
      Parent::setRenderTransform(mat);
   }
}

void ShapeBase::setControllingClient( GameConnection* client )
{
   if( isGhost() && gSFX3DWorld )
   {
      if( gSFX3DWorld->getListener() == this && !client && getControllingClient() && getControllingClient()->isConnectionToServer() )
      {
         // We are the current listener and are no longer a controller object on the
         // connection, so clear our listener status.
         
         gSFX3DWorld->setListener( NULL );
      }
      else if( client && client->isConnectionToServer() && !getControllingObject() )
      {
         // We're on the local client and not controlled by another object, so make
         // us the current SFX listener.
         
         gSFX3DWorld->setListener( this );
      }
   }

   Parent::setControllingClient( client );

   // Update all of the mounted images' shapes so they may
   // optimize their animation threads.
   for (U32 i=0; i<MaxMountedImages; ++i)
   {
      MountedImage& image = mMountedImageList[i];
      image.updateDoAnimateAllShapes( this );
   }
}

void ShapeBase::setControllingObject(ShapeBase* obj)
{
   if (obj) {
      setProcessTick(false);
      // Even though we don't processTick, we still need to
      // process after the controller in case anyone is mounted
      // on this object.
      processAfter(obj);
   }
   else {
      setProcessTick(true);
      clearProcessAfter();
      // Catch the case of the controlling object actually
      // mounted on this object.
      if (mControllingObject->mMount.object == this)
         mControllingObject->processAfter(this);
   }
   mControllingObject = obj;
}

ShapeBase* ShapeBase::getControlObject()
{
   return 0;
}

void ShapeBase::setControlObject(ShapeBase*)
{
}

bool ShapeBase::isFirstPerson() const
{
   // Always first person as far as the server is concerned.
   if (!isGhost())
      return true;

   if (const GameConnection* con = getControllingClient())
      return con->getControlObject() == this && con->isFirstPerson();
   return false;
}

bool ShapeBase::isValidCameraFov(F32 fov)
{
   return((fov >= mDataBlock->cameraMinFov) && (fov <= mDataBlock->cameraMaxFov));
}

void ShapeBase::setCameraFov(F32 fov)
{
   // On server allow for script side checking
   if ( !isGhost() && isMethod( "validateCameraFov" ) )
   {
      fov = validateCameraFov_callback( fov );
   }

   mCameraFov = mClampF(fov, mDataBlock->cameraMinFov, mDataBlock->cameraMaxFov);
}

void ShapeBase::onCameraScopeQuery(NetConnection *cr, CameraScopeQuery * query)
{
   // update the camera query
   query->camera = this;

   if(GameConnection * con = dynamic_cast<GameConnection*>(cr))
   {
      // get the fov from the connection (in deg)
      F32 fov;
      if (con->getControlCameraFov(&fov))
      {
         query->fov = mDegToRad(fov/2);
         query->sinFov = mSin(query->fov);
         query->cosFov = mCos(query->fov);
      }
   }

   // use eye rather than camera transform (good enough and faster)
   MatrixF eyeTransform;
   getEyeTransform(&eyeTransform);
   eyeTransform.getColumn(3, &query->pos);
   eyeTransform.getColumn(1, &query->orientation);

   // Get the visible distance.
	if (getSceneManager() != NULL)
		query->visibleDistance = getSceneManager()->getVisibleDistance();

   Parent::onCameraScopeQuery( cr, query );
}


//----------------------------------------------------------------------------
F32 ShapeBase::getEnergyLevel()
{
   if ( mDataBlock->inheritEnergyFromMount && mShapeBaseMount )
      return mShapeBaseMount->getEnergyLevel();
   return mEnergy; 
}

F32 ShapeBase::getEnergyValue()
{
   if ( mDataBlock->inheritEnergyFromMount && mShapeBaseMount )
   {
      F32 maxEnergy = mShapeBaseMount->mDataBlock->maxEnergy;
      if ( maxEnergy > 0.f )
         return (mShapeBaseMount->getEnergyLevel() / maxEnergy);
   }
   else
   {
      F32 maxEnergy = mDataBlock->maxEnergy;
      if ( maxEnergy > 0.f )
         return (mEnergy / mDataBlock->maxEnergy);   
   }

   return 0.0f;
}

void ShapeBase::setEnergyLevel(F32 energy)
{
   if (mDataBlock->inheritEnergyFromMount == false || !mShapeBaseMount) {
      if (mDamageState == Enabled) {
         mEnergy = (energy > mDataBlock->maxEnergy)?
            mDataBlock->maxEnergy: (energy < 0)? 0: energy;
      }
   } else {
      // Pass the set onto whatever we're mounted to...
      if ( mShapeBaseMount )
      {
         mShapeBaseMount->setEnergyLevel(energy);
      }
   }
}

void ShapeBase::setDamageLevel(F32 damage)
{
   if (!mDataBlock->isInvincible) {
      F32 store = mDamage;
      mDamage = mClampF(damage, 0.f, mDataBlock->maxDamage);

      if (store != mDamage) {
         updateDamageLevel();
         if (isServerObject()) {
            setMaskBits(DamageMask);
            mDataBlock->onDamage_callback( this, mDamage - store );
         }
      }
   }
}

void ShapeBase::updateContainer()
{
   PROFILE_SCOPE( ShapeBase_updateContainer );

   // Update container drag and buoyancy properties

   // Set default values.
   mDrag = mDataBlock->drag;
   mBuoyancy = 0.0f;      
   mGravityMod = 1.0;
   mAppliedForce.set(0,0,0);
   
   ContainerQueryInfo info;
   info.box = getWorldBox();
   info.mass = getMass();

   mContainer->findObjects(info.box, WaterObjectType|PhysicalZoneObjectType,findRouter,&info);
      
   mWaterCoverage = info.waterCoverage;
   mLiquidType    = info.liquidType;
   mLiquidHeight  = info.waterHeight;   
   setCurrentWaterObject( info.waterObject );
   
   // This value might be useful as a datablock value,
   // This is what allows the player to stand in shallow water (below this coverage)
   // without jiggling from buoyancy
   if (mWaterCoverage >= 0.25f) 
   {      
      // water viscosity is used as drag for in water.
      // ShapeBaseData drag is used for drag outside of water.
      // Combine these two components to calculate this ShapeBase object's 
      // current drag.
      mDrag = ( info.waterCoverage * info.waterViscosity ) + 
              ( 1.0f - info.waterCoverage ) * mDataBlock->drag;
      mBuoyancy = (info.waterDensity / mDataBlock->density) * info.waterCoverage;
   }

   mAppliedForce = info.appliedForce;
   mGravityMod = info.gravityScale;

   //Con::printf( "WaterCoverage: %f", mWaterCoverage );
   //Con::printf( "Drag: %f", mDrag );
}


//----------------------------------------------------------------------------

void ShapeBase::applyRepair(F32 amount)
{
   // Repair increases the repair reserve
   if (amount > 0 && ((mRepairReserve += amount) > mDamage))
      mRepairReserve = mDamage;
}

void ShapeBase::applyDamage(F32 amount)
{
   if (amount > 0)
      setDamageLevel(mDamage + amount);
}

F32 ShapeBase::getDamageValue()
{
   // Return a 0-1 damage value.
   return mDamage / mDataBlock->maxDamage;
}

F32 ShapeBase::getMaxDamage()
{
   return mDataBlock->maxDamage;
}

void ShapeBase::updateDamageLevel()
{
   if (mDamageThread) {
      // mDamage is already 0-1 on the client
      if (mDamage >= mDataBlock->destroyedLevel) {
         if (getDamageState() == Destroyed)
            mShapeInstance->setPos(mDamageThread, 0);
         else
            mShapeInstance->setPos(mDamageThread, 1);
      } else {
         mShapeInstance->setPos(mDamageThread, mDamage / mDataBlock->destroyedLevel);
      }
   }      
}


//----------------------------------------------------------------------------

void ShapeBase::setDamageState(DamageState state)
{
   if (mDamageState == state)
      return;

   bool invokeCallback = false;
   const char* lastState = 0;

   if (!isGhost()) {
      if (state != getDamageState())
         setMaskBits(DamageMask);

      lastState = getDamageStateName();
      switch (state) {
         case Destroyed: {
            if (mDamageState == Enabled)
            {
               setDamageState(Disabled);

               // It is possible that the setDamageState() call above has already
               // upgraded our state to Destroyed.  If that is the case, no need
               // to continue.
               if (mDamageState == state)
                  return;
            }
            invokeCallback = true;
            break;
         }
         case Disabled:
            if (mDamageState == Enabled)
               invokeCallback = true;
            break;
         case Enabled:
            invokeCallback = true;
            break;
         default:
            AssertFatal(false, "Invalid damage state");
            break;
      }
   }

   mDamageState = state;
   if (mDamageState != Enabled) {
      mRepairReserve = 0;
      mEnergy = 0;
   }
   if (invokeCallback) {
      // Like to call the scripts after the state has been intialize.
      // This should only end up being called on the server.
      switch (state) {
         case Destroyed:
            mDataBlock->onDestroyed_callback( this, lastState );
            break;
         case Disabled:
            mDataBlock->onDisabled_callback( this, lastState );
            break;
         case Enabled:
            mDataBlock->onEnabled_callback( this, lastState );
            break;
         default:
            break;
      }
   }
   updateDamageState();
   updateDamageLevel();
}

bool ShapeBase::setDamageState(const char* state)
{
   for (S32 i = 0; i < NumDamageStates; i++)
      if (!dStricmp(state,sDamageStateName[i])) {
         setDamageState(DamageState(i));
         return true;
      }
   return false;
}

const char* ShapeBase::getDamageStateName()
{
   return sDamageStateName[mDamageState];
}

void ShapeBase::updateDamageState()
{
   if (mHulkThread) {
      F32 pos = (mDamageState == Destroyed) ? 1.0f : 0.0f;
      if (mShapeInstance->getPos(mHulkThread) != pos) {
         mShapeInstance->setPos(mHulkThread,pos);

         if (isClientObject())
            mShapeInstance->animate();
      }
   }
}


//----------------------------------------------------------------------------

void ShapeBase::blowUp()
{
   Point3F center;
   mObjBox.getCenter(&center);
   center += getPosition();
   MatrixF trans = getTransform();
   trans.setPosition( center );

   // explode
   Explosion* pExplosion = NULL;

   if( pointInWater( (Point3F &)center ) && mDataBlock->underwaterExplosion )
   {
      pExplosion = new Explosion;
      pExplosion->onNewDataBlock(mDataBlock->underwaterExplosion, false);
   }
   else
   {
      if (mDataBlock->explosion)
      {
         pExplosion = new Explosion;
         pExplosion->onNewDataBlock(mDataBlock->explosion, false);
      }
   }

   if( pExplosion )
   {
      pExplosion->setTransform(trans);
      pExplosion->setInitialState(center, damageDir);
      if (pExplosion->registerObject() == false)
      {
         Con::errorf(ConsoleLogEntry::General, "ShapeBase(%s)::explode: couldn't register explosion",
                     mDataBlock->getName() );
         delete pExplosion;
         pExplosion = NULL;
      }
   }

   TSShapeInstance *debShape = NULL;

   if( mDataBlock->debrisShape == NULL )
   {
      return;
   }
   else
   {
      debShape = new TSShapeInstance( mDataBlock->debrisShape, true);
   }


   Vector< TSPartInstance * > partList;
   TSPartInstance::breakShape( debShape, 0, partList, NULL, NULL, 0 );

   if( !mDataBlock->debris )
   {
      mDataBlock->debris = new DebrisData;
   }

   // cycle through partlist and create debris pieces
   for( U32 i=0; i<partList.size(); i++ )
   {
      //Point3F axis( 0.0, 0.0, 1.0 );
      Point3F randomDir = MathUtils::randomDir( damageDir, 0, 50 );

      Debris *debris = new Debris;
      debris->setPartInstance( partList[i] );
      debris->init( center, randomDir );
      debris->onNewDataBlock( mDataBlock->debris, false );
      debris->setTransform( trans );

      if( !debris->registerObject() )
      {
         Con::warnf( ConsoleLogEntry::General, "Could not register debris for class: %s", mDataBlock->getName() );
         delete debris;
         debris = NULL;
      }
      else
      {
         debShape->incDebrisRefCount();
      }
   }

   damageDir.set(0, 0, 1);
}

//----------------------------------------------------------------------------

void ShapeBase::onMount( SceneObject *obj, S32 node )
{   
   mConvexList->nukeList();   

   // Are we mounting to a ShapeBase object?
   mShapeBaseMount = dynamic_cast<ShapeBase*>( obj );

   Parent::onMount( obj, node );
}

void ShapeBase::onUnmount( SceneObject *obj, S32 node )
{
   Parent::onUnmount( obj, node );

   mShapeBaseMount = NULL;
}

Point3F ShapeBase::getAIRepairPoint()
{
   if (mDataBlock->mountPointNode[ShapeBaseData::AIRepairNode] < 0)
        return Point3F(0, 0, 0);
   MatrixF xf(true);
   getMountTransform( ShapeBaseData::AIRepairNode, MatrixF::Identity, &xf );
   Point3F pos(0, 0, 0);
   xf.getColumn(3,&pos);
   return pos;
}

//----------------------------------------------------------------------------

void ShapeBase::getEyeTransform(MatrixF* mat)
{
   getEyeBaseTransform(mat, true);
}

void ShapeBase::getEyeBaseTransform(MatrixF* mat, bool includeBank)
{
   // Returns eye to world space transform
   S32 eyeNode = mDataBlock->eyeNode;
   if (eyeNode != -1)
      mat->mul(getTransform(), mShapeInstance->mNodeTransforms[eyeNode]);
   else
      *mat = getTransform();
}

void ShapeBase::getRenderEyeTransform(MatrixF* mat)
{
   getRenderEyeBaseTransform(mat, true);
}

void ShapeBase::getRenderEyeBaseTransform(MatrixF* mat, bool includeBank)
{
   // Returns eye to world space transform
   S32 eyeNode = mDataBlock->eyeNode;
   if (eyeNode != -1)
      mat->mul(getRenderTransform(), mShapeInstance->mNodeTransforms[eyeNode]);
   else
      *mat = getRenderTransform();
}

void ShapeBase::getCameraTransform(F32* pos,MatrixF* mat)
{
   // Returns camera to world space transform
   // Handles first person / third person camera position

   if (isServerObject() && mShapeInstance)
      mShapeInstance->animateNodeSubtrees(true);

   if (*pos != 0)
   {
      F32 min,max;
      Point3F offset;
      MatrixF eye,rot;
      getCameraParameters(&min,&max,&offset,&rot);
      getRenderEyeTransform(&eye);
      mat->mul(eye,rot);

      // Use the eye transform to orient the camera
      VectorF vp,vec;
      vp.x = vp.z = 0;
      vp.y = -(max - min) * *pos;
      eye.mulV(vp,&vec);
      
      VectorF minVec;
      vp.y = -min;
      eye.mulV( vp, &minVec );

      // Use the camera node's pos.
      Point3F osp,sp;
      if (mDataBlock->cameraNode != -1) {
         mShapeInstance->mNodeTransforms[mDataBlock->cameraNode].getColumn(3,&osp);

         // Scale the camera position before applying the transform
         const Point3F& scale = getScale();
         osp.convolve( scale );

         getRenderTransform().mulP(osp,&sp);
      }
      else
         getRenderTransform().getColumn(3,&sp);

      // Make sure we don't extend the camera into anything solid
      Point3F ep = sp + minVec + vec + offset;
      disableCollision();
      if (isMounted())
         getObjectMount()->disableCollision();
      RayInfo collision;
      if( mContainer && mContainer->castRay(sp, ep,
                              (0xFFFFFFFF & ~(WaterObjectType      |
                                              GameBaseObjectType   |
                                              TriggerObjectType    |
                                              DefaultObjectType)),
                              &collision) == true) {
         F32 vecLenSq = vec.lenSquared();
         F32 adj = (-mDot(vec, collision.normal) / vecLenSq) * 0.1;
         F32 newPos = getMax(0.0f, collision.t - adj);
         if (newPos == 0.0f)
            eye.getColumn(3,&ep);
         else
            ep = sp + offset + (vec * newPos);
      }
      mat->setColumn(3,ep);
      if (isMounted())
         getObjectMount()->enableCollision();
      enableCollision();
   }
   else
   {
      getRenderEyeTransform(mat);
   }

   // Apply Camera FX.
   mat->mul( gCamFXMgr.getTrans() );
}

void ShapeBase::getEyeCameraTransform(IDisplayDevice *displayDevice, U32 eyeId, MatrixF *outMat)
{
   MatrixF temp(1);
   Point3F eyePos;
   Point3F rotEyePos;

   DisplayPose inPose;
   displayDevice->getFrameEyePose(&inPose, eyeId);
   DisplayPose newPose = calcCameraDeltaPose(displayDevice->getCurrentConnection(), inPose);

   // Ok, basically we just need to add on newPose to the camera transform
   // NOTE: currently we dont support third-person camera in this mode
   MatrixF cameraTransform(1);
   F32 fakePos = 0;
   getCameraTransform(&fakePos, &cameraTransform);

   QuatF baserot = cameraTransform;
   QuatF qrot = QuatF(newPose.orientation);
   QuatF concatRot;
   concatRot.mul(baserot, qrot);
   concatRot.setMatrix(&temp);
   temp.setPosition(cameraTransform.getPosition() + concatRot.mulP(newPose.position, &rotEyePos));

   *outMat = temp;
}

DisplayPose ShapeBase::calcCameraDeltaPose(GameConnection *con, const DisplayPose& inPose)
{
   // NOTE: this is intended to be similar to updateMove
   // WARNING: does not take into account any move values

   DisplayPose outPose;
   outPose.orientation = getRenderTransform().toEuler();
   outPose.position = inPose.position;

   if (con && con->getControlSchemeAbsoluteRotation())
   {
      // Pitch
      outPose.orientation.x = inPose.orientation.x;

      // Constrain the range of mRot.x
      while (outPose.orientation.x < -M_PI_F) 
         outPose.orientation.x += M_2PI_F;
      while (outPose.orientation.x > M_PI_F) 
         outPose.orientation.x -= M_2PI_F;

      // Yaw
      outPose.orientation.z = inPose.orientation.z;

      // Constrain the range of mRot.z
      while (outPose.orientation.z < -M_PI_F) 
         outPose.orientation.z += M_2PI_F;
      while (outPose.orientation.z > M_PI_F) 
         outPose.orientation.z -= M_2PI_F;

      // Bank
      if (mDataBlock->cameraCanBank)
      {
         outPose.orientation.y = inPose.orientation.y;
      }

      // Constrain the range of mRot.y
      while (outPose.orientation.y > M_PI_F) 
         outPose.orientation.y -= M_2PI_F;
   }

   return outPose;
}

void ShapeBase::getCameraParameters(F32 *min,F32* max,Point3F* off,MatrixF* rot)
{
   *min = mDataBlock->cameraMinDist;
   *max = mDataBlock->cameraMaxDist;
   off->set(0,0,0);
   rot->identity();
}

//----------------------------------------------------------------------------
F32 ShapeBase::getDamageFlash() const
{
   return mDamageFlash;
}

void ShapeBase::setDamageFlash(const F32 flash)
{
   mDamageFlash = flash;
   if (mDamageFlash < 0.0)
      mDamageFlash = 0;
   else if (mDamageFlash > 1.0)
      mDamageFlash = 1.0;
}


//----------------------------------------------------------------------------
F32 ShapeBase::getWhiteOut() const
{
   return mWhiteOut;
}

void ShapeBase::setWhiteOut(const F32 flash)
{
   mWhiteOut = flash;
   if (mWhiteOut < 0.0)
      mWhiteOut = 0;
   else if (mWhiteOut > 1.5)
      mWhiteOut = 1.5;
}


//----------------------------------------------------------------------------

bool ShapeBase::onlyFirstPerson() const
{
   return mDataBlock->firstPersonOnly;
}

bool ShapeBase::useObjsEyePoint() const
{
   return mDataBlock->useEyePoint;
}

//----------------------------------------------------------------------------
void ShapeBase::setVelocity(const VectorF&)
{
}

void ShapeBase::applyImpulse(const Point3F&,const VectorF&)
{
}


//----------------------------------------------------------------------------

void ShapeBase::playAudio(U32 slot,SFXTrack* profile)
{
   AssertFatal( slot < MaxSoundThreads, "ShapeBase::playAudio() bad slot index" );
   Sound& st = mSoundThread[slot];
   if( profile && ( !st.play || st.profile != profile ) ) 
   {
      setMaskBits(SoundMaskN << slot);
      st.play = true;
      st.profile = profile;
      updateAudioState(st);
   }
}

void ShapeBase::stopAudio(U32 slot)
{
   AssertFatal( slot < MaxSoundThreads, "ShapeBase::stopAudio() bad slot index" );

   Sound& st = mSoundThread[slot];
   if ( st.play ) 
   {
      st.play = false;
      setMaskBits(SoundMaskN << slot);
      updateAudioState(st);
   }
}

void ShapeBase::updateServerAudio()
{
   // Timeout non-looping sounds
   for (S32 i = 0; i < MaxSoundThreads; i++) {
      Sound& st = mSoundThread[i];
      if (st.play && st.timeout && st.timeout < Sim::getCurrentTime()) {
         clearMaskBits(SoundMaskN << i);
         st.play = false;
      }
   }
}

void ShapeBase::updateAudioState(Sound& st)
{
   SFX_DELETE( st.sound );

   if ( st.play && st.profile ) 
   {
      if ( isGhost() ) 
      {
         if ( Sim::findObject( SimObjectId((uintptr_t)st.profile), st.profile ) )
         {
            st.sound = SFX->createSource( st.profile, &getTransform() );
            if ( st.sound )
               st.sound->play();
         }
         else
            st.play = false;
      }
      else 
      {
         // Non-looping sounds timeout on the server
         st.timeout = 0;
         if ( !st.profile->getDescription()->mIsLooping )
            st.timeout = Sim::getCurrentTime() + sAudioTimeout;
      }
   }
   else
      st.play = false;
}

void ShapeBase::updateAudioPos()
{
   for (S32 i = 0; i < MaxSoundThreads; i++)
   {
      SFXSource* source = mSoundThread[i].sound;
      if ( source )
         source->setTransform( getTransform() );
   }
}

//----------------------------------------------------------------------------

const char *ShapeBase::getThreadSequenceName( U32 slot )
{
	Thread& st = mScriptThread[slot];
	if ( st.sequence == -1 )
	{
		// Invalid Animation.
		return "";
	}

	// Name Index
	const U32 nameIndex = getShape()->sequences[st.sequence].nameIndex;

	// Return Name.
	return getShape()->getName( nameIndex );
}

bool ShapeBase::setThreadSequence(U32 slot, S32 seq, bool reset)
{
   Thread& st = mScriptThread[slot];
   if (st.thread && st.sequence == seq && st.state == Thread::Play)
      return true;

   // Handle a -1 sequence, as this may be set when a thread has been destroyed.
   if(seq == -1)
      return true;

   if (seq < MaxSequenceIndex) {
      setMaskBits(ThreadMaskN << slot);
      st.sequence = seq;
      if (reset) {
         st.state = Thread::Play;
         st.atEnd = false;
         st.timescale = 1.f;
         st.position = 0.f;
      }
      if (mShapeInstance) {
         if (!st.thread)
            st.thread = mShapeInstance->addThread();
         mShapeInstance->setSequence(st.thread,seq,st.position);
         updateThread(st);
      }
      return true;
   }
   return false;
}

void ShapeBase::updateThread(Thread& st)
{
	switch (st.state)
	{
		case Thread::Stop:
			{
				mShapeInstance->setTimeScale( st.thread, 1.f );
				mShapeInstance->setPos( st.thread, ( st.timescale > 0.f ) ? 1.0f : 0.0f );
			} // Drop through to pause state

		case Thread::Pause:
			{
				mShapeInstance->setTimeScale( st.thread, 0.f );
			} break;

		case Thread::Play:
			{
				if (st.atEnd)
				{
					mShapeInstance->setTimeScale(st.thread,1);
					mShapeInstance->setPos( st.thread, ( st.timescale > 0.f ) ? 1.0f : 0.0f );
					mShapeInstance->setTimeScale(st.thread,0);
               st.state = Thread::Stop;
				}
				else
				{
					if ( st.position != -1.f )
					{
						mShapeInstance->setTimeScale( st.thread, 1.f );
						mShapeInstance->setPos( st.thread, st.position );
					}

					mShapeInstance->setTimeScale(st.thread, st.timescale );
				}
			} break;

      case Thread::Destroy:
         {
            st.atEnd = true;
            st.sequence = -1;
            if(st.thread)
            {
               mShapeInstance->destroyThread(st.thread);
               st.thread = 0;
            }
         } break;
	}
}

bool ShapeBase::stopThread(U32 slot)
{
   Thread& st = mScriptThread[slot];
   if (st.sequence != -1 && st.state != Thread::Stop) {
      setMaskBits(ThreadMaskN << slot);
      st.state = Thread::Stop;
      updateThread(st);
      return true;
   }
   return false;
}

bool ShapeBase::destroyThread(U32 slot)
{
   Thread& st = mScriptThread[slot];
   if (st.sequence != -1 && st.state != Thread::Destroy) {
      setMaskBits(ThreadMaskN << slot);
      st.state = Thread::Destroy;
      updateThread(st);
      return true;
   }
   return false;
}

bool ShapeBase::pauseThread(U32 slot)
{
   Thread& st = mScriptThread[slot];
   if (st.sequence != -1 && st.state != Thread::Pause) {
      setMaskBits(ThreadMaskN << slot);
      st.state = Thread::Pause;
      updateThread(st);
      return true;
   }
   return false;
}

bool ShapeBase::playThread(U32 slot)
{
   Thread& st = mScriptThread[slot];
   if (st.sequence != -1 && st.state != Thread::Play) {
      setMaskBits(ThreadMaskN << slot);
      st.state = Thread::Play;
      updateThread(st);
      return true;
   }
   return false;
}

bool ShapeBase::setThreadPosition( U32 slot, F32 pos )
{
	Thread& st = mScriptThread[slot];
	if (st.sequence != -1)
	{
		setMaskBits(ThreadMaskN << slot);
		st.position = pos;
		st.atEnd = false;
		updateThread(st);

		return true;
	}
	return false;
}

bool ShapeBase::setThreadDir(U32 slot,bool forward)
{
	Thread& st = mScriptThread[slot];
	if (st.sequence != -1)
	{
		if ( ( st.timescale >= 0.f ) != forward )
		{
			setMaskBits(ThreadMaskN << slot);
			st.timescale *= -1.f ;
			st.atEnd = false;
			updateThread(st);
		}
		return true;
	}
	return false;
}

bool ShapeBase::setThreadTimeScale( U32 slot, F32 timeScale )
{
	Thread& st = mScriptThread[slot];
	if (st.sequence != -1)
	{
		if (st.timescale != timeScale)
		{
			setMaskBits(ThreadMaskN << slot);
			st.timescale = timeScale;
			updateThread(st);
		}
		return true;
	}
	return false;
}

void ShapeBase::advanceThreads(F32 dt)
{
   for (U32 i = 0; i < MaxScriptThreads; i++) {
      Thread& st = mScriptThread[i];
      if (st.thread) {
         if (!mShapeInstance->getShape()->sequences[st.sequence].isCyclic() && !st.atEnd &&
			 ( ( st.timescale > 0.f )? mShapeInstance->getPos(st.thread) >= 1.0:
              mShapeInstance->getPos(st.thread) <= 0)) {
            st.atEnd = true;
            updateThread(st);
            if (!isGhost()) {
               mDataBlock->onEndSequence_callback(this, i, this->getThreadSequenceName(i));
            }
         }

         // Make sure the thread is still valid after the call to onEndSequence_callback().
         // Someone could have called destroyThread() while in there.
         if(st.thread)
         {
            mShapeInstance->advanceTime(dt,st.thread);
            st.position = mShapeInstance->getPos(st.thread);
         }
      }
   }
}

//----------------------------------------------------------------------------

/// Emit particles on the given emitter, if we're within triggerHeight above some static surface with a
/// material that has 'showDust' set to true.  The particles will have a lifetime of 'numMilliseconds'
/// and be emitted at the given offset from the contact point having a direction of 'axis'.

void ShapeBase::emitDust( ParticleEmitter* emitter, F32 triggerHeight, const Point3F& offset, U32 numMilliseconds, const Point3F& axis )
{
   if( !emitter )
      return;

   Point3F startPos = getPosition();
   Point3F endPos = startPos + Point3F( 0.0, 0.0, - triggerHeight );

   RayInfo rayInfo;
   if( getContainer()->castRay( startPos, endPos, STATIC_COLLISION_TYPEMASK, &rayInfo ) )
   {
      Material* material = ( rayInfo.material ? dynamic_cast< Material* >( rayInfo.material->getMaterial() ) : 0 );
      if( material && material->mShowDust )
      {
         ColorF colorList[ ParticleData::PDC_NUM_KEYS ];

         for( U32 x = 0; x < getMin( Material::NUM_EFFECT_COLOR_STAGES, ParticleData::PDC_NUM_KEYS ); ++ x )
            colorList[ x ] = material->mEffectColor[ x ];
         for( U32 x = Material::NUM_EFFECT_COLOR_STAGES; x < ParticleData::PDC_NUM_KEYS; ++ x )
            colorList[ x ].set( 1.0, 1.0, 1.0, 0.0 );

         emitter->setColors( colorList );

         Point3F contactPoint = rayInfo.point + offset;
         emitter->emitParticles( contactPoint, true, ( axis == Point3F::Zero ? rayInfo.normal : axis ),
                                 getVelocity(), numMilliseconds );
      }
   }
}

//----------------------------------------------------------------------------

TSShape const* ShapeBase::getShape()
{
   return mShapeInstance? mShapeInstance->getShape(): 0;
}

void ShapeBase::prepRenderImage( SceneRenderState *state )
{
   _prepRenderImage( state, true, true );
}

void ShapeBase::_prepRenderImage(   SceneRenderState *state, 
                                    bool renderSelf, 
                                    bool renderMountedImages )
{
   PROFILE_SCOPE( ShapeBase_PrepRenderImage );

   //if ( mIsCubemapUpdate )
   //   return false;

   if( ( getDamageState() == Destroyed ) && ( !mDataBlock->renderWhenDestroyed ) )
      return;

   // We don't need to render if all the meshes are forced hidden.
   if ( mMeshHidden.getSize() > 0 && mMeshHidden.testAll() )   
      return;
      
   // If we're rendering shadows don't render the mounted
   // images unless the shape is also rendered.
   if ( state->isShadowPass() && !renderSelf )
      return;

   // If we're currently rendering our own reflection we
   // don't want to render ourselves into it.
   if ( mCubeReflector.isRendering() )
      return;

   // We force all the shapes to use the highest detail
   // if we're the control object or mounted.
   bool forceHighestDetail = false;
   {
      GameConnection *con = GameConnection::getConnectionToServer();
      ShapeBase *co = NULL;
      if(con && ( (co = dynamic_cast<ShapeBase*>(con->getControlObject())) != NULL) )
      {
         if(co == this || co->getObjectMount() == this)
            forceHighestDetail = true;
      }
   }

   mLastRenderFrame = sLastRenderFrame;

   // get shape detail...we might not even need to be drawn
   Point3F cameraOffset = getWorldBox().getClosestPoint( state->getDiffuseCameraPosition() ) - state->getDiffuseCameraPosition();
   F32 dist = cameraOffset.len();
   if (dist < 0.01f)
      dist = 0.01f;

   F32 invScale = (1.0f/getMax(getMax(mObjScale.x,mObjScale.y),mObjScale.z));

   if (mShapeInstance)
   {
      if ( forceHighestDetail )         
         mShapeInstance->setCurrentDetail( 0 );
      else
         mShapeInstance->setDetailFromDistance( state, dist * invScale );
                              
      mShapeInstance->animate();
   }
   
   if (  ( mShapeInstance && mShapeInstance->getCurrentDetail() < 0 ) ||
         ( !mShapeInstance && !gShowBoundingBox ) ) 
   {
      // no, don't draw anything
      return;
   }

   if( renderMountedImages )
   {
      for (U32 i = 0; i < MaxMountedImages; i++)
      {
         MountedImage& image = mMountedImageList[i];
         U32 imageShapeIndex = getImageShapeIndex(image);
         if (image.dataBlock && image.shapeInstance[imageShapeIndex])
         {
            // Select detail levels on mounted items but... always 
            // draw the control object's mounted images in high detail.
            if ( forceHighestDetail )
               image.shapeInstance[imageShapeIndex]->setCurrentDetail( 0 );
            else
               image.shapeInstance[imageShapeIndex]->setDetailFromDistance( state, dist * invScale );

            if (!mIsZero( (1.0f - mCloakLevel) * mFadeVal))
            {
               prepBatchRender( state, i );

               // Debug rendering of the mounted shape bounds.
               if ( gShowBoundingBox )
               {
                  ObjectRenderInst *ri = state->getRenderPass()->allocInst<ObjectRenderInst>();
                  ri->renderDelegate.bind( this, &ShapeBase::_renderBoundingBox );
                  ri->objectIndex = i;
                  ri->type = RenderPassManager::RIT_Editor;
                  state->getRenderPass()->addInst( ri );
               }
            }
         }
      }
   }

   // Debug rendering of the shape bounding box.
   if ( gShowBoundingBox )
   {
      ObjectRenderInst *ri = state->getRenderPass()->allocInst<ObjectRenderInst>();
      ri->renderDelegate.bind( this, &ShapeBase::_renderBoundingBox );
      ri->objectIndex = -1;
      ri->type = RenderPassManager::RIT_Editor;
      state->getRenderPass()->addInst( ri );
   }

   if ( mShapeInstance && renderSelf )
      prepBatchRender( state, -1 );

   calcClassRenderData();
}

//----------------------------------------------------------------------------
// prepBatchRender
//----------------------------------------------------------------------------
void ShapeBase::prepBatchRender(SceneRenderState* state, S32 mountedImageIndex )
{
   // CHANGES IN HERE SHOULD BE DUPLICATED IN TSSTATIC!
   
   GFXTransformSaver saver;
   
   // Set up our TS render state. 
   TSRenderState rdata;
   rdata.setSceneState( state );
   if ( mCubeReflector.isEnabled() )
      rdata.setCubemap( mCubeReflector.getCubemap() );
   rdata.setFadeOverride( (1.0f - mCloakLevel) * mFadeVal );

   // We might have some forward lit materials
   // so pass down a query to gather lights.
   LightQuery query;
   query.init( getWorldSphere() );
   rdata.setLightQuery( &query );

   if( mountedImageIndex != -1 )
   {
      MountedImage& image = mMountedImageList[mountedImageIndex];

      if( image.dataBlock && image.shapeInstance )
      {
         renderMountedImage( mountedImageIndex, rdata, state );
      }
   }
   else
   {
      MatrixF mat = getRenderTransform();
      mat.scale( mObjScale );
      GFX->setWorldMatrix( mat );

      if ( state->isDiffusePass() && mCubeReflector.isEnabled() && mCubeReflector.getOcclusionQuery() )
      {
         RenderPassManager *pass = state->getRenderPass();

         OccluderRenderInst *ri = pass->allocInst<OccluderRenderInst>();   

         ri->type = RenderPassManager::RIT_Occluder;
         ri->query = mCubeReflector.getOcclusionQuery();   
         mObjToWorld.mulP( mObjBox.getCenter(), &ri->position );
         ri->scale.set( mObjBox.getExtents() );
         ri->orientation = pass->allocUniqueXform( mObjToWorld );        
         ri->isSphere = false;
         state->getRenderPass()->addInst( ri );
      }

      mShapeInstance->animate();
      mShapeInstance->render( rdata );
   }
}

void ShapeBase::renderMountedImage( U32 imageSlot, TSRenderState &rstate, SceneRenderState *state )
{
   GFX->pushWorldMatrix();

   MatrixF mat;
   getRenderImageTransform(imageSlot, &mat, rstate.getSceneState()->isShadowPass());
   GFX->setWorldMatrix( mat );

   MountedImage& image = mMountedImageList[imageSlot];
   U32 imageShapeIndex = getImageShapeIndex(image);
   image.shapeInstance[imageShapeIndex]->animate();
   image.shapeInstance[imageShapeIndex]->render( rstate );

   GFX->popWorldMatrix();
}

void ShapeBase::_renderBoundingBox( ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance *overrideMat )
{
   // If we got an override mat then this some
   // special rendering pass... skip out of it.
   if ( overrideMat )
      return;

   GFXStateBlockDesc desc;
   desc.setZReadWrite( true, false );
   desc.setBlend( true );
   desc.fillMode = GFXFillWireframe;

   GFXDrawUtil *drawer = GFX->getDrawUtil();

   if ( ri->objectIndex != -1 )
   {
      MountedImage &image = mMountedImageList[ ri->objectIndex ];

      if ( image.shapeInstance )
      {
         MatrixF mat;
         getRenderImageTransform( ri->objectIndex, &mat );         

         const Box3F &objBox = image.shapeInstance[getImageShapeIndex(image)]->getShape()->bounds;

         drawer->drawCube( desc, objBox, ColorI( 255, 255, 255 ), &mat );
      }
   }
   else   
      drawer->drawCube( desc, mObjBox, ColorI( 255, 255, 255 ), &mRenderObjToWorld );
}

bool ShapeBase::castRay(const Point3F &start, const Point3F &end, RayInfo* info)
{
   if (mShapeInstance) 
   {
      RayInfo shortest;
      shortest.t = 1e8;

      info->object = NULL;
      for (U32 i = 0; i < mDataBlock->LOSDetails.size(); i++)
      {
         mShapeInstance->animate(mDataBlock->LOSDetails[i]);
         if (mShapeInstance->castRay(start, end, info, mDataBlock->LOSDetails[i]))
         {
            info->object = this;
            if (info->t < shortest.t)
               shortest = *info;
         }
      }

      if (info->object == this) 
      {
         // Copy out the shortest time...
         *info = shortest;
         return true;
      }
   }

   return false;
}

bool ShapeBase::castRayRendered(const Point3F &start, const Point3F &end, RayInfo* info)
{
   if (mShapeInstance) 
   {
      RayInfo localInfo;
      mShapeInstance->animate();
      bool res = mShapeInstance->castRayRendered(start, end, &localInfo, mShapeInstance->getCurrentDetail());
      if (res)
      {
         *info = localInfo;
         info->object = this;
         return true;
      }
   }

   return false;
}


//----------------------------------------------------------------------------

bool ShapeBase::buildPolyList(PolyListContext context, AbstractPolyList* polyList, const Box3F &, const SphereF &)
{
   if ( !mShapeInstance )
      return false;

   polyList->setTransform(&mObjToWorld, mObjScale);
   polyList->setObject(this);

   if ( context == PLC_Selection )
   {
      mShapeInstance->animate();
      mShapeInstance->buildPolyList(polyList,mShapeInstance->getCurrentDetail());
      return true;
   }
   else if ( context == PLC_Export )
   {
      // Try to call on the client so we can export materials
      ShapeBase* exportObj = this;
      if ( isServerObject() && getClientObject() )
         exportObj = dynamic_cast<ShapeBase*>(getClientObject());

      S32 dl = 0;
      exportObj->mShapeInstance->animate();
      exportObj->mShapeInstance->buildPolyList(polyList, dl);
      return true;
   }
   else
   {
      bool ret = false;
      for (U32 i = 0; i < mDataBlock->collisionDetails.size(); i++)
      {
         mShapeInstance->buildPolyList(polyList,mDataBlock->collisionDetails[i]);
         ret = true;
      }

      return ret;
   }
}

void ShapeBase::buildConvex(const Box3F& box, Convex* convex)
{
   if (mShapeInstance == NULL)
      return;

   // These should really come out of a pool
   mConvexList->collectGarbage();

   Box3F realBox = box;
   mWorldToObj.mul(realBox);
   realBox.minExtents.convolveInverse(mObjScale);
   realBox.maxExtents.convolveInverse(mObjScale);

   if (realBox.isOverlapped(getObjBox()) == false)
      return;

   for (U32 i = 0; i < mDataBlock->collisionDetails.size(); i++)
   {
         Box3F newbox = mDataBlock->collisionBounds[i];
         newbox.minExtents.convolve(mObjScale);
         newbox.maxExtents.convolve(mObjScale);
         mObjToWorld.mul(newbox);
         if (box.isOverlapped(newbox) == false)
            continue;

         // See if this hull exists in the working set already...
         Convex* cc = 0;
         CollisionWorkingList& wl = convex->getWorkingList();
         for (CollisionWorkingList* itr = wl.wLink.mNext; itr != &wl; itr = itr->wLink.mNext) {
            if (itr->mConvex->getType() == ShapeBaseConvexType &&
                (static_cast<ShapeBaseConvex*>(itr->mConvex)->pShapeBase == this &&
                 static_cast<ShapeBaseConvex*>(itr->mConvex)->hullId     == i)) {
               cc = itr->mConvex;
               break;
            }
         }
         if (cc)
            continue;

         // Create a new convex.
         ShapeBaseConvex* cp = new ShapeBaseConvex;
         mConvexList->registerObject(cp);
         convex->addToWorkingList(cp);
         cp->mObject    = this;
         cp->pShapeBase = this;
         cp->hullId     = i;
         cp->box        = mDataBlock->collisionBounds[i];
         cp->transform = 0;
         cp->findNodeTransform();
   }
}


//----------------------------------------------------------------------------

void ShapeBase::queueCollision( SceneObject *obj, const VectorF &vec)
{
   // Add object to list of collisions.
   SimTime time = Sim::getCurrentTime();
   S32 num = obj->getId();

   CollisionTimeout** adr = &mTimeoutList;
   CollisionTimeout* ptr = mTimeoutList;
   while (ptr) {
      if (ptr->objectNumber == num) {
         if (ptr->expireTime < time) {
            ptr->expireTime = time + CollisionTimeoutValue;
            ptr->object = obj;
            ptr->vector = vec;
         }
         return;
      }
      // Recover expired entries
      if (ptr->expireTime < time) {
         CollisionTimeout* cur = ptr;
         *adr = ptr->next;
         ptr = ptr->next;
         cur->next = sFreeTimeoutList;
         sFreeTimeoutList = cur;
      }
      else {
         adr = &ptr->next;
         ptr = ptr->next;
      }
   }

   // New entry for the object
   if (sFreeTimeoutList != NULL)
   {
      ptr = sFreeTimeoutList;
      sFreeTimeoutList = ptr->next;
      ptr->next = NULL;
   }
   else
   {
      ptr = sTimeoutChunker.alloc();
   }

   ptr->object = obj;
   ptr->objectNumber = obj->getId();
   ptr->vector = vec;
   ptr->expireTime = time + CollisionTimeoutValue;
   ptr->next = mTimeoutList;

   mTimeoutList = ptr;
}

void ShapeBase::notifyCollision()
{
   // Notify all the objects that were just stamped during the queueing
   // process.
   SimTime expireTime = Sim::getCurrentTime() + CollisionTimeoutValue;
   for (CollisionTimeout* ptr = mTimeoutList; ptr; ptr = ptr->next)
   {
      if (ptr->expireTime == expireTime && ptr->object)
      {
         SimObjectPtr<SceneObject> safePtr(ptr->object);
         SimObjectPtr<ShapeBase> safeThis(this);
         onCollision(ptr->object,ptr->vector);
         ptr->object = 0;

         if(!bool(safeThis))
            return;

         if(bool(safePtr))
            safePtr->onCollision(this,ptr->vector);

         if(!bool(safeThis))
            return;
      }
   }
}

void ShapeBase::onCollision( SceneObject *object, const VectorF &vec )
{
   if (!isGhost())
      mDataBlock->onCollision_callback( this, object, vec, vec.len() );
}

//--------------------------------------------------------------------------
bool ShapeBase::pointInWater( Point3F &point )
{
   if ( mCurrentWaterObject == NULL )
      return false;

   return mCurrentWaterObject->isUnderwater( point );   
}

//----------------------------------------------------------------------------

void ShapeBase::writePacketData(GameConnection *connection, BitStream *stream)
{
   Parent::writePacketData(connection, stream);

   stream->write(getEnergyLevel());
   stream->write(mRechargeRate);
}

void ShapeBase::readPacketData(GameConnection *connection, BitStream *stream)
{
   Parent::readPacketData(connection, stream);

   F32 energy;
   stream->read(&energy);
   setEnergyLevel(energy);

   stream->read(&mRechargeRate);
}

F32 ShapeBase::getUpdatePriority(CameraScopeQuery *camInfo, U32 updateMask, S32 updateSkips)
{
   // If it's the scope object, must be high priority
   if (camInfo->camera == this) {
      // Most priorities are between 0 and 1, so this
      // should be something larger.
      return 10.0f;
   }
   if( camInfo->camera )
   {
      ShapeBase* camera = dynamic_cast< ShapeBase* >( camInfo->camera );
      // see if the camera is mounted to this...
      // if it is, this should have a high priority
      if( camera && camera->getObjectMount() == this)
         return 10.0f;
   }
   return Parent::getUpdatePriority(camInfo, updateMask, updateSkips);
}

U32 ShapeBase::packUpdate(NetConnection *con, U32 mask, BitStream *stream)
{
   U32 retMask = Parent::packUpdate(con, mask, stream);

   if (mask & InitialUpdateMask) {
      // mask off sounds that aren't playing
      S32 i;
      for (i = 0; i < MaxSoundThreads; i++)
         if (!mSoundThread[i].play)
            mask &= ~(SoundMaskN << i);

      // mask off threads that aren't running
      for (i = 0; i < MaxScriptThreads; i++)
         if (mScriptThread[i].sequence == -1)
            mask &= ~(ThreadMaskN << i);

      // mask off images that aren't updated
      for(i = 0; i < MaxMountedImages; i++)
         if(!mMountedImageList[i].dataBlock)
            mask &= ~(ImageMaskN << i);
   }

   if(!stream->writeFlag(mask & (NameMask | DamageMask | SoundMask | MeshHiddenMask |
         ThreadMask | ImageMask | CloakMask | SkinMask)))
      return retMask;

   if (stream->writeFlag(mask & DamageMask)) {
      stream->writeFloat(mClampF(mDamage / mDataBlock->maxDamage, 0.f, 1.f), DamageLevelBits);
      stream->writeInt(mDamageState,NumDamageStateBits);
      stream->writeNormalVector( damageDir, 8 );
   }

   if (stream->writeFlag(mask & ThreadMask)) {
      for (S32 i = 0; i < MaxScriptThreads; i++) {
         Thread& st = mScriptThread[i];
         if (stream->writeFlag( (st.sequence != -1 || st.state == Thread::Destroy) && (mask & (ThreadMaskN << i)) ) ) {
            stream->writeInt(st.sequence,ThreadSequenceBits);
            stream->writeInt(st.state,2);
            stream->write(st.timescale);
            stream->write(st.position);
            stream->writeFlag(st.atEnd);
         }
      }
   }

   if (stream->writeFlag(mask & SoundMask)) {
      for (S32 i = 0; i < MaxSoundThreads; i++) {
         Sound& st = mSoundThread[i];
         if (stream->writeFlag(mask & (SoundMaskN << i)))
            if (stream->writeFlag(st.play))
               stream->writeRangedU32(st.profile->getId(),DataBlockObjectIdFirst,
                                      DataBlockObjectIdLast);
      }
   }

   if (stream->writeFlag(mask & ImageMask)) {
      for (S32 i = 0; i < MaxMountedImages; i++)
         if (stream->writeFlag(mask & (ImageMaskN << i))) {
            MountedImage& image = mMountedImageList[i];
            if (stream->writeFlag(image.dataBlock))
               stream->writeInt(image.dataBlock->getId() - DataBlockObjectIdFirst,
                                DataBlockObjectIdBitSize);
            con->packNetStringHandleU(stream, image.skinNameHandle);
            con->packNetStringHandleU(stream, image.scriptAnimPrefix);

            // Used to force the 1st person rendering on the client.  This is required
            // as this object could be ghosted to the client prior to its controlling client
            // being set.  Therefore there is a network tick when the object is in limbo...
            stream->writeFlag(image.dataBlock && image.dataBlock->animateAllShapes && getControllingClient() == con);

            stream->writeFlag(image.wet);
            stream->writeFlag(image.motion);
            stream->writeFlag(image.ammo);
            stream->writeFlag(image.loaded);
            stream->writeFlag(image.target);
            stream->writeFlag(image.triggerDown);
            stream->writeFlag(image.altTriggerDown);

            for (U32 j=0; j<ShapeBaseImageData::MaxGenericTriggers; ++j)
            {
               stream->writeFlag(image.genericTrigger[j]);
            }

            stream->writeInt(image.fireCount,3);            
            stream->writeInt(image.altFireCount,3);
            stream->writeInt(image.reloadCount,3);
            stream->writeFlag(isImageFiring(i));
            stream->writeFlag(isImageAltFiring(i));
            stream->writeFlag(isImageReloading(i));
         }
   }

   // Group some of the uncommon stuff together.
   if (stream->writeFlag(mask & (NameMask | CloakMask | SkinMask | MeshHiddenMask ))) {
         
      if (stream->writeFlag(mask & CloakMask))
      {
         // cloaking
         stream->writeFlag( mCloaked );

         // piggyback control update
         stream->writeFlag(bool(getControllingClient()));

         // fading
         if(stream->writeFlag(mFading && mFadeElapsedTime >= mFadeDelay)) {
            stream->writeFlag(mFadeOut);
            stream->write(mFadeTime);
         }
         else
            stream->writeFlag(mFadeVal == 1.0f);
      }
      if (stream->writeFlag(mask & NameMask)) {
         con->packNetStringHandleU(stream, mShapeNameHandle);
      }

      if ( stream->writeFlag( mask & MeshHiddenMask ) )
         stream->writeBits( mMeshHidden );

      if (stream->writeFlag(mask & SkinMask))
         con->packNetStringHandleU(stream, mSkinNameHandle);
   }

   return retMask;
}

void ShapeBase::unpackUpdate(NetConnection *con, BitStream *stream)
{
   Parent::unpackUpdate(con, stream);
   mLastRenderFrame = sLastRenderFrame; // make sure we get a process after the event...

   if(!stream->readFlag())
      return;

   if (stream->readFlag()) {
      mDamage = mClampF(stream->readFloat(DamageLevelBits) * mDataBlock->maxDamage, 0.f, mDataBlock->maxDamage);
      DamageState prevState = mDamageState;
      mDamageState = DamageState(stream->readInt(NumDamageStateBits));
      stream->readNormalVector( &damageDir, 8 );
      if (prevState != Destroyed && mDamageState == Destroyed && isProperlyAdded())
         blowUp();
      updateDamageLevel();
      updateDamageState();
   }

   if (stream->readFlag()) {
      for (S32 i = 0; i < MaxScriptThreads; i++) {
         if (stream->readFlag()) {
            Thread& st = mScriptThread[i];
            U32 seq = stream->readInt(ThreadSequenceBits);
            st.state = Thread::State(stream->readInt(2));
            stream->read( &st.timescale );
            stream->read( &st.position );
            st.atEnd = stream->readFlag();
            if (st.sequence != seq && st.state != Thread::Destroy)
               setThreadSequence(i,seq,false);
            else
               updateThread(st);
         }
      }
   }

   if ( stream->readFlag() ) 
   {
      for ( S32 i = 0; i < MaxSoundThreads; i++ ) 
      {
         if ( stream->readFlag() ) 
         {
            Sound& st = mSoundThread[i];
            st.play = stream->readFlag();
            if ( st.play ) 
            {
               st.profile = (SFXTrack*) stream->readRangedU32(  DataBlockObjectIdFirst,
                                                                DataBlockObjectIdLast );
            }

            if ( isProperlyAdded() )
               updateAudioState( st );
         }
      }
   }

   // Mounted Images
   if (stream->readFlag()) {
      for (S32 i = 0; i < MaxMountedImages; i++) {
         if (stream->readFlag()) {
            MountedImage& image = mMountedImageList[i];
            ShapeBaseImageData* imageData = 0;
            if (stream->readFlag()) {
               SimObjectId id = stream->readInt(DataBlockObjectIdBitSize) +
                  DataBlockObjectIdFirst;
               if (!Sim::findObject(id,imageData)) {
                  con->setLastError("Invalid packet (mounted images).");
                  return;
               }
            }

            NetStringHandle skinDesiredNameHandle = con->unpackNetStringHandleU(stream);

            NetStringHandle scriptDesiredAnimPrefix = con->unpackNetStringHandleU(stream);

            image.forceAnimateAllShapes = stream->readFlag();

            image.wet = stream->readFlag();

            image.motion = stream->readFlag();

            image.ammo = stream->readFlag();

            image.loaded = stream->readFlag();

            image.target = stream->readFlag();

            image.triggerDown = stream->readFlag();
            image.altTriggerDown = stream->readFlag();

            for (U32 j=0; j<ShapeBaseImageData::MaxGenericTriggers; ++j)
            {
               image.genericTrigger[j] = stream->readFlag();
            }

            S32 count = stream->readInt(3);
            S32 altCount = stream->readInt(3);
            S32 reloadCount = stream->readInt(3);

            bool datablockChange = image.dataBlock != imageData;
            if (datablockChange || (image.skinNameHandle != skinDesiredNameHandle))
            {
               MountedImage& image = mMountedImageList[i];
               image.scriptAnimPrefix = scriptDesiredAnimPrefix;

               setImage(   i, imageData, 
                           skinDesiredNameHandle, image.loaded, 
                           image.ammo, image.triggerDown, image.altTriggerDown,
                           image.motion, image.genericTrigger[0], image.genericTrigger[1], image.genericTrigger[2], image.genericTrigger[3],
                           image.target);
            }
            
            if (!datablockChange && image.scriptAnimPrefix != scriptDesiredAnimPrefix)
            {
               // We don't have a new image, but we do have a new script anim prefix to work with.
               // Notify the image of this change.
               MountedImage& image = mMountedImageList[i];
               image.scriptAnimPrefix = scriptDesiredAnimPrefix;
               updateAnimThread(i, getImageShapeIndex(image));
            }

            bool isFiring = stream->readFlag();
            bool isAltFiring = stream->readFlag();
            bool isReloading = stream->readFlag();

            if (isProperlyAdded()) {
               // Normal processing
               bool processFiring = false;
               if (count != image.fireCount)
               {
                  image.fireCount = count;
                  setImageState(i,getImageFireState(i),true);
                  processFiring = true;
               }
               else if (altCount != image.altFireCount)
               {
                  image.altFireCount = altCount;
                  setImageState(i,getImageAltFireState(i),true);
                  processFiring = true;
               }
               else if (reloadCount != image.reloadCount)
               {
                  image.reloadCount = reloadCount;
                  setImageState(i,getImageReloadState(i),true);
               }

               if (processFiring && imageData)
               {
                  if ( imageData->lightType == ShapeBaseImageData::WeaponFireLight )                     
                     image.lightStart = Sim::getCurrentTime();                     
               }
               
               updateImageState(i,0);
            }
            else
            {               
               if(imageData)
               {
                  // Initial state
                  image.fireCount = count;
                  image.altFireCount = altCount;
                  image.reloadCount = reloadCount;
                  if (isFiring)
                     setImageState(i,getImageFireState(i),true);
                  else if (isAltFiring)
                     setImageState(i,getImageAltFireState(i),true);
                  else if (isReloading)
                     setImageState(i,getImageReloadState(i),true);
               }
            }
         }
      }
   }

   if (stream->readFlag())
   {
      if(stream->readFlag())     // CloakMask and control
      {
         // Read cloaking state.
         
         setCloakedState(stream->readFlag());
         mIsControlled = stream->readFlag();

         if (( mFading = stream->readFlag()) == true) {
            mFadeOut = stream->readFlag();
            if(mFadeOut)
               mFadeVal = 1.0f;
            else
               mFadeVal = 0;
            stream->read(&mFadeTime);
            mFadeDelay = 0;
            mFadeElapsedTime = 0;
         }
         else
            mFadeVal = F32(stream->readFlag());
      }
      if (stream->readFlag())  { // NameMask
         mShapeNameHandle = con->unpackNetStringHandleU(stream);
      }
      
      if ( stream->readFlag() ) // MeshHiddenMask
      {
         stream->readBits( &mMeshHidden );
         _updateHiddenMeshes();
      }

      if (stream->readFlag())    // SkinMask
      {
         NetStringHandle skinDesiredNameHandle = con->unpackNetStringHandleU(stream);;
         if (mSkinNameHandle != skinDesiredNameHandle)
         {
            mSkinNameHandle = skinDesiredNameHandle;
            reSkin();
         }
      }
   }
}


//--------------------------------------------------------------------------

void ShapeBase::forceUncloak(const char * reason)
{
   AssertFatal(isServerObject(), "ShapeBase::forceUncloak: server only call");
   if(!mCloaked)
      return;

   mDataBlock->onForceUncloak_callback( this, reason ? reason : "" );
}

void ShapeBase::setCloakedState(bool cloaked)
{
   if (cloaked == mCloaked)
      return;

   if (isServerObject())
      setMaskBits(CloakMask);

   // Have to do this for the client, if we are ghosted over in the initial
   //  packet as cloaked, we set the state immediately to the extreme
   if (isProperlyAdded() == false) {
      mCloaked = cloaked;
      if (mCloaked)
         mCloakLevel = 1.0;
      else
         mCloakLevel = 0.0;
   } else {
      mCloaked = cloaked;
   }
}


//--------------------------------------------------------------------------

void ShapeBase::setHidden( bool hidden )
{
   if( hidden != isHidden() )
   {
      Parent::setHidden( hidden );

      if( hidden )
         setProcessTick( false );
      else
         setProcessTick( true );
   }
}

//--------------------------------------------------------------------------

void ShapeBaseConvex::findNodeTransform()
{
   S32 dl = pShapeBase->mDataBlock->collisionDetails[hullId];

   TSShapeInstance* si = pShapeBase->getShapeInstance();
   TSShape* shape = si->getShape();

   const TSShape::Detail* detail = &shape->details[dl];
   const S32 subs = detail->subShapeNum;
   const S32 start = shape->subShapeFirstObject[subs];
   const S32 end = start + shape->subShapeNumObjects[subs];

   // Find the first object that contains a mesh for this
   // detail level. There should only be one mesh per
   // collision detail level.
   for (S32 i = start; i < end; i++) 
   {
      const TSShape::Object* obj = &shape->objects[i];
      if (obj->numMeshes && detail->objectDetailNum < obj->numMeshes) 
      {
         nodeTransform = &si->mNodeTransforms[obj->nodeIndex];
         return;
      }
   }
   return;
}

const MatrixF& ShapeBaseConvex::getTransform() const
{
   // If the transform isn't specified, it's assumed to be the
   // origin of the shape.
   const MatrixF& omat = (transform != 0)? *transform: mObject->getTransform();

   // Multiply on the mesh shape offset
   // tg: Returning this static here is not really a good idea, but
   // all this Convex code needs to be re-organized.
   if (nodeTransform) {
      static MatrixF mat;
      mat.mul(omat,*nodeTransform);
      return mat;
   }
   return omat;
}

Box3F ShapeBaseConvex::getBoundingBox() const
{
   const MatrixF& omat = (transform != 0)? *transform: mObject->getTransform();
   return getBoundingBox(omat, mObject->getScale());
}

Box3F ShapeBaseConvex::getBoundingBox(const MatrixF& mat, const Point3F& scale) const
{
   Box3F newBox = box;
   newBox.minExtents.convolve(scale);
   newBox.maxExtents.convolve(scale);
   mat.mul(newBox);
   return newBox;
}

Point3F ShapeBaseConvex::support(const VectorF& v) const
{
   TSShape::ConvexHullAccelerator* pAccel =
      pShapeBase->mShapeInstance->getShape()->getAccelerator(pShapeBase->mDataBlock->collisionDetails[hullId]);
   AssertFatal(pAccel != NULL, "Error, no accel!");

   F32 currMaxDP = mDot(pAccel->vertexList[0], v);
   U32 index = 0;
   for (U32 i = 1; i < pAccel->numVerts; i++) {
      F32 dp = mDot(pAccel->vertexList[i], v);
      if (dp > currMaxDP) {
         currMaxDP = dp;
         index = i;
      }
   }

   return pAccel->vertexList[index];
}


void ShapeBaseConvex::getFeatures(const MatrixF& mat, const VectorF& n, ConvexFeature* cf)
{
   cf->material = 0;
   cf->object = mObject;

   TSShape::ConvexHullAccelerator* pAccel =
      pShapeBase->mShapeInstance->getShape()->getAccelerator(pShapeBase->mDataBlock->collisionDetails[hullId]);
   AssertFatal(pAccel != NULL, "Error, no accel!");

   F32 currMaxDP = mDot(pAccel->vertexList[0], n);
   U32 index = 0;
   U32 i;
   for (i = 1; i < pAccel->numVerts; i++) {
      F32 dp = mDot(pAccel->vertexList[i], n);
      if (dp > currMaxDP) {
         currMaxDP = dp;
         index = i;
      }
   }

   const U8* emitString = pAccel->emitStrings[index];
   U32 currPos = 0;
   U32 numVerts = emitString[currPos++];
   for (i = 0; i < numVerts; i++) {
      cf->mVertexList.increment();
      U32 index = emitString[currPos++];
      mat.mulP(pAccel->vertexList[index], &cf->mVertexList.last());
   }

   U32 numEdges = emitString[currPos++];
   for (i = 0; i < numEdges; i++) {
      U32 ev0 = emitString[currPos++];
      U32 ev1 = emitString[currPos++];
      cf->mEdgeList.increment();
      cf->mEdgeList.last().vertex[0] = ev0;
      cf->mEdgeList.last().vertex[1] = ev1;
   }

   U32 numFaces = emitString[currPos++];
   for (i = 0; i < numFaces; i++) {
      cf->mFaceList.increment();
      U32 plane = emitString[currPos++];
      mat.mulV(pAccel->normalList[plane], &cf->mFaceList.last().normal);
      for (U32 j = 0; j < 3; j++)
         cf->mFaceList.last().vertex[j] = emitString[currPos++];
   }
}


void ShapeBaseConvex::getPolyList(AbstractPolyList* list)
{
   list->setTransform(&pShapeBase->getTransform(), pShapeBase->getScale());
   list->setObject(pShapeBase);

   pShapeBase->mShapeInstance->animate(pShapeBase->mDataBlock->collisionDetails[hullId]);
   pShapeBase->mShapeInstance->buildPolyList(list,pShapeBase->mDataBlock->collisionDetails[hullId]);
}


//--------------------------------------------------------------------------

bool ShapeBase::isInvincible()
{
   if( mDataBlock )
   {
      return mDataBlock->isInvincible;
   }
   return false;
}

void ShapeBase::startFade( F32 fadeTime, F32 fadeDelay, bool fadeOut )
{
   setMaskBits(CloakMask);
   mFadeElapsedTime = 0;
   mFading = true;
   if(fadeDelay < 0)
      fadeDelay = 0;
   if(fadeTime < 0)
      fadeTime = 0;
   mFadeTime = fadeTime;
   mFadeDelay = fadeDelay;
   mFadeOut = fadeOut;
   mFadeVal = F32(mFadeOut);
}

//--------------------------------------------------------------------------

void ShapeBase::setShapeName(const char* name)
{
   if (!isGhost()) {
      if (name[0] != '\0') {
         // Use tags for better network performance
         // Should be a tag, but we'll convert to one if it isn't.
         if (name[0] == StringTagPrefixByte)
            mShapeNameHandle = NetStringHandle(U32(dAtoi(name + 1)));
         else
            mShapeNameHandle = NetStringHandle(name);
      }
      else {
         mShapeNameHandle = NetStringHandle();
      }
      setMaskBits(NameMask);
   }
}

void ShapeBase::setSkinName(const char* name)
{
   if (!isGhost()) {
      if (name[0] != '\0') {
         // Use tags for better network performance
         // Should be a tag, but we'll convert to one if it isn't.
         if (name[0] == StringTagPrefixByte)
            mSkinNameHandle = NetStringHandle(U32(dAtoi(name + 1)));
         else
            mSkinNameHandle = NetStringHandle(name);
      }
      else
         mSkinNameHandle = NetStringHandle();
      setMaskBits(SkinMask);
   }
}

//----------------------------------------------------------------------------

void ShapeBase::reSkin()
{
   if ( isGhost() && mShapeInstance && mSkinNameHandle.isValidString() )
   {
      Vector<String> skins;
      String(mSkinNameHandle.getString()).split( ";", skins );

      for (S32 i = 0; i < skins.size(); i++)
      {
         String oldSkin( mAppliedSkinName.c_str() );
         String newSkin( skins[i] );

         // Check if the skin handle contains an explicit "old" base string. This
         // allows all models to support skinning, even if they don't follow the 
         // "base_xxx" material naming convention.
         S32 split = newSkin.find( '=' );    // "old=new" format skin?
         if ( split != String::NPos )
         {
            oldSkin = newSkin.substr( 0, split );
            newSkin = newSkin.erase( 0, split+1 );
         }

         mShapeInstance->reSkin( newSkin, oldSkin );
         mAppliedSkinName = newSkin;
      }
   }
}

void ShapeBase::setCurrentWaterObject( WaterObject *obj )
{
   if ( obj )
      deleteNotify( obj );
   if ( mCurrentWaterObject )
      clearNotify( mCurrentWaterObject );

   mCurrentWaterObject = obj;
}

//--------------------------------------------------------------------------
//----------------------------------------------------------------------------
DefineEngineMethod( ShapeBase, setHidden, void, ( bool show ),,
   "@brief Add or remove this object from the scene.\n\n"
   "When removed from the scene, the object will not be processed or rendered.\n"
   "@param show False to hide the object, true to re-show it\n\n" )
{
   object->setHidden( show );
}

DefineEngineMethod( ShapeBase, isHidden, bool, (),,
   "Check if the object is hidden.\n"
   "@return true if the object is hidden, false if visible.\n\n" )
{
   return object->isHidden();
}

//----------------------------------------------------------------------------
DefineEngineMethod( ShapeBase, playAudio, bool, ( S32 slot, SFXTrack* track ),,
   "@brief Attach a sound to this shape and start playing it.\n\n"

   "@param slot Audio slot index for the sound (valid range is 0 - 3)\n" // 3 = ShapeBase::MaxSoundThreads-1
   "@param track SFXTrack to play\n"
   "@return true if the sound was attached successfully, false if failed\n\n"
   
   "@see stopAudio()\n")
{
   if (track && slot >= 0 && slot < ShapeBase::MaxSoundThreads) {
      object->playAudio(slot,track);
      return true;
   }
   return false;
}

DefineEngineMethod( ShapeBase, stopAudio, bool, ( S32 slot ),,
   "@brief Stop a sound started with playAudio.\n\n"

   "@param slot audio slot index (started with playAudio)\n"
   "@return true if the sound was stopped successfully, false if failed\n\n"
   
   "@see playAudio()\n")
{
   if (slot >= 0 && slot < ShapeBase::MaxSoundThreads) {
      object->stopAudio(slot);
      return true;
   }
   return false;
}


//----------------------------------------------------------------------------
DefineEngineMethod( ShapeBase, playThread, bool, ( S32 slot, const char* name ), ( "" ),
   "@brief Start a new animation thread, or restart one that has been paused or "
   "stopped.\n\n"

   "@param slot thread slot to play. Valid range is 0 - 3)\n"  // 3 = ShapeBase::MaxScriptThreads-1
   "@param name name of the animation sequence to play in this slot. If not "
   "specified, the paused or stopped thread in this slot will be resumed.\n"
   "@return true if successful, false if failed\n\n"

   "@tsexample\n"
   "%obj.playThread( 0, \"ambient\" );      // Play the ambient sequence in slot 0\n"
   "%obj.setThreadTimeScale( 0, 0.5 );    // Play at half-speed\n"
   "%obj.pauseThread( 0 );                // Pause the sequence\n"
   "%obj.playThread( 0 );                 // Resume playback\n"
   "%obj.playThread( 0, \"spin\" );         // Replace the sequence in slot 0\n"
   "@endtsexample\n"
   
   "@see pauseThread()\n"
   "@see stopThread()\n"
   "@see setThreadDir()\n"
   "@see setThreadTimeScale()\n"
   "@see destroyThread()\n")
{
   if (slot >= 0 && slot < ShapeBase::MaxScriptThreads) {
      if (!dStrEqual(name, "")) {
         if (object->getShape()) {
            S32 seq = object->getShape()->findSequence(name);
            if (seq != -1 && object->setThreadSequence(slot,seq))
               return true;
         }
      }
      else
         if (object->playThread(slot))
            return true;
   }
   return false;
}

DefineEngineMethod( ShapeBase, setThreadDir, bool, ( S32 slot, bool fwd ),,
   "@brief Set the playback direction of an animation thread.\n\n"

   "@param slot thread slot to modify\n"
   "@param fwd true to play the animation forwards, false to play backwards\n"
   "@return true if successful, false if failed\n\n"
   
   "@see playThread()\n" )
{
   if (slot >= 0 && slot < ShapeBase::MaxScriptThreads) {
      if (object->setThreadDir(slot,fwd))
         return true;
   }
   return false;
}

DefineEngineMethod( ShapeBase, setThreadTimeScale, bool, ( S32 slot, F32 scale ),,
   "@brief Set the playback time scale of an animation thread.\n\n"

   "@param slot thread slot to modify\n"
   "@param scale new thread time scale (1=normal speed, 0.5=half speed etc)\n"
   "@return true if successful, false if failed\n\n"
   
   "@see playThread\n" )
{
   if (slot >= 0 && slot < ShapeBase::MaxScriptThreads) {
      if (object->setThreadTimeScale(slot,scale))
         return true;
   }
   return false;
}

DefineEngineMethod( ShapeBase, setThreadPosition, bool, ( S32 slot, F32 pos ),,
   "@brief Set the position within an animation thread.\n\n"

   "@param slot thread slot to modify\n"
   "@param pos position within thread\n"
   "@return true if successful, false if failed\n\n"
   
   "@see playThread\n" )
{
   if (slot >= 0 && slot < ShapeBase::MaxScriptThreads) {
      if (object->setThreadPosition(slot,pos))
         return true;
   }
   return false;
}

DefineEngineMethod( ShapeBase, stopThread, bool, ( S32 slot ),,
   "@brief Stop an animation thread.\n\n"

   "If restarted using playThread, the animation "
   "will start from the beginning again.\n"
   "@param slot thread slot to stop\n"
   "@return true if successful, false if failed\n\n"
   
   "@see playThread\n" )
{
   if (slot >= 0 && slot < ShapeBase::MaxScriptThreads) {
      if (object->stopThread(slot))
         return true;
   }
   return false;
}

DefineEngineMethod( ShapeBase, destroyThread, bool, ( S32 slot ),,
   "@brief Destroy an animation thread, which prevents it from playing.\n\n"

   "@param slot thread slot to destroy\n"
   "@return true if successful, false if failed\n\n"
   
   "@see playThread\n" )
{
   if (slot >= 0 && slot < ShapeBase::MaxScriptThreads) {
      if (object->destroyThread(slot))
         return true;
   }
   return false;
}

DefineEngineMethod( ShapeBase, pauseThread, bool, ( S32 slot ),,
   "@brief Pause an animation thread.\n\n"
   
   "If restarted using playThread, the animation "
   "will resume from the paused position.\n"
   "@param slot thread slot to stop\n"
   "@return true if successful, false if failed\n\n"
   
   "@see playThread\n" )
{
   if (slot >= 0 && slot < ShapeBase::MaxScriptThreads) {
      if (object->pauseThread(slot))
         return true;
   }
   return false;
}

//----------------------------------------------------------------------------
DefineEngineMethod( ShapeBase, mountImage, bool,
   ( ShapeBaseImageData* image, S32 slot, bool loaded, const char* skinTag ), ( true, "" ),
   "@brief Mount a new Image.\n\n"

   "@param image the Image to mount\n"
   "@param slot Image slot to mount into (valid range is 0 - 3)\n"
   "@param loaded initial loaded state for the Image\n"
   "@param skinTag tagged string to reskin the mounted Image\n"
   "@return true if successful, false if failed\n\n"

   "@tsexample\n"
   "%player.mountImage( PistolImage, 1 );\n"
   "%player.mountImage( CrossbowImage, 0, false );\n"
   "%player.mountImage( RocketLauncherImage, 0, true, 'blue' );\n"
   "@endtsexample\n"
   
   "@see unmountImage()\n"
   "@see getMountedImage()\n"
   "@see getPendingImage()\n"
   "@see isImageMounted()\n")
{
   if (image && slot >= 0 && slot < ShapeBase::MaxMountedImages) {

      NetStringHandle team;
      if (skinTag[0] == StringTagPrefixByte)
         team = NetStringHandle(U32(dAtoi(skinTag+1)));

      return object->mountImage( image, slot, loaded, team );
   }
   return false;
}

DefineEngineMethod( ShapeBase, unmountImage, bool, ( S32 slot ),,
   "@brief Unmount the mounted Image in the specified slot.\n\n"

   "@param slot Image slot to unmount\n"
   "@return true if successful, false if failed\n\n"
   
   "@see mountImage()\n")
{
   if (slot >= 0 && slot < ShapeBase::MaxMountedImages)
      return object->unmountImage(slot);
   return false;
}

DefineEngineMethod( ShapeBase, getMountedImage, S32, ( S32 slot ),,
   "@brief Get the Image mounted in the specified slot.\n\n"

   "@param slot Image slot to query\n"
   "@return ID of the ShapeBaseImageData datablock mounted in the slot, or 0 "
   "if no Image is mounted there.\n\n" )
{
   if (slot >= 0 && slot < ShapeBase::MaxMountedImages)
      if (ShapeBaseImageData* data = object->getMountedImage(slot))
         return data->getId();
   return 0;
}

DefineEngineMethod( ShapeBase, getPendingImage, S32, ( S32 slot ),,
   "@brief Get the Image that will be mounted next in the specified slot.\n\n"

   "Calling mountImage when an Image is already mounted does one of two things: "
   "<ol><li>Mount the new Image immediately, the old Image is discarded and "
   "whatever state it was in is ignored.</li>"
   "<li>If the current Image state does not allow Image changes, the new "
   "Image is marked as pending, and will not be mounted until the current "
   "state completes. eg. if the user changes weapons, you may wish to ensure "
   "that the current weapon firing state plays to completion first.</li></ol>\n"
   "This command retrieves the ID of the pending Image (2nd case above).\n"
   
   "@param slot Image slot to query\n"
   "@return ID of the pending ShapeBaseImageData datablock, or 0 if none.\n\n" )
{
   if (slot >= 0 && slot < ShapeBase::MaxMountedImages)
      if (ShapeBaseImageData* data = object->getPendingImage(slot))
         return data->getId();
   return 0;
}

DefineEngineMethod( ShapeBase, isImageFiring, bool, ( S32 slot ),,
   "@brief Check if the current Image state is firing.\n\n"

   "@param slot Image slot to query\n"
   "@return true if the current Image state in this slot has the 'stateFire' flag set.\n" )
{
   if (slot >= 0 && slot < ShapeBase::MaxMountedImages)
      return object->isImageFiring(slot);
   return false;
}

DefineEngineMethod( ShapeBase, isImageMounted, bool, ( ShapeBaseImageData* image ),,
   "@brief Check if the given datablock is mounted to any slot on this object.\n\n"

   "@param image ShapeBaseImageData datablock to query\n"
   "@return true if the Image is mounted to any slot, false otherwise.\n\n" )
{
   return (image && object->isImageMounted(image));
}

DefineEngineMethod( ShapeBase, getMountSlot, S32, ( ShapeBaseImageData* image ),,
   "@brief Get the first slot the given datablock is mounted to on this object.\n\n"

   "@param image ShapeBaseImageData datablock to query\n"
   "@return index of the first slot the Image is mounted in, or -1 if the Image "
   "is not mounted in any slot on this object.\n\n" )

{
   return image ? object->getMountSlot(image) : -1;
}

DefineEngineMethod( ShapeBase, getImageSkinTag, S32, ( S32 slot ),,
   "@brief Get the skin tag ID for the Image mounted in the specified slot.\n\n"

   "@param slot Image slot to query\n"
   "@return the skinTag value passed to mountImage when the image was "
   "mounted\n\n" )
{
   if (slot >= 0 && slot < ShapeBase::MaxMountedImages)
      return object->getImageSkinTag(slot).getIndex();
   return -1;
}

DefineEngineMethod( ShapeBase, getImageState, const char*, ( S32 slot ),,
   "@brief Get the name of the current state of the Image in the specified slot.\n\n"

   "@param slot Image slot to query\n"
   "@return name of the current Image state, or \"Error\" if slot is invalid\n\n" )
{
   if (slot >= 0 && slot < ShapeBase::MaxMountedImages)
      return object->getImageState(slot);
   return "Error";
}

DefineEngineMethod( ShapeBase, hasImageState, bool, ( S32 slot, const char* state ),,
   "@brief Check if the given state exists on the mounted Image.\n\n"

   "@param slot Image slot to query\n"
   "@param state Image state to check for\n"
   "@return true if the Image has the requested state defined.\n\n" )
{
   if (slot >= 0 && slot < ShapeBase::MaxMountedImages)
      return object->hasImageState(slot, state);
   return false;
}

DefineEngineMethod( ShapeBase, getImageTrigger, bool, ( S32 slot ),,
   "@brief Get the trigger state of the Image mounted in the specified slot.\n\n"

   "@param slot Image slot to query\n"
   "@return the Image's current trigger state\n\n" )
{
   if (slot >= 0 && slot < ShapeBase::MaxMountedImages)
      return object->getImageTriggerState(slot);
   return false;
}

DefineEngineMethod( ShapeBase, setImageTrigger, bool, ( S32 slot, bool state ),,
   "@brief Set the trigger state of the Image mounted in the specified slot.\n\n"

   "@param slot Image slot to modify\n"
   "@param state new trigger state for the Image\n"
   "@return the Image's new trigger state\n\n" )
{
   if (slot >= 0 && slot < ShapeBase::MaxMountedImages) {
      object->setImageTriggerState(slot,state);
      return object->getImageTriggerState(slot);
   }
   return false;
}

DefineEngineMethod( ShapeBase, getImageGenericTrigger, bool, ( S32 slot, S32 trigger ),,
   "@brief Get the generic trigger state of the Image mounted in the specified slot.\n\n"

   "@param slot Image slot to query\n"
   "@param trigger Generic trigger number\n"
   "@return the Image's current generic trigger state\n\n" )
{
   if (slot >= 0 && slot < ShapeBase::MaxMountedImages && trigger >= 0 && trigger < ShapeBaseImageData::MaxGenericTriggers)
      return object->getImageGenericTriggerState(slot, trigger);
   return false;
}

DefineEngineMethod( ShapeBase, setImageGenericTrigger, S32, ( S32 slot, S32 trigger, bool state ),,
   "@brief Set the generic trigger state of the Image mounted in the specified slot.\n\n"

   "@param slot Image slot to modify\n"
   "@param trigger Generic trigger number\n"
   "@param state new generic trigger state for the Image\n"
   "@return the Image's new generic trigger state or -1 if there was a problem.\n\n" )
{
   if (slot >= 0 && slot < ShapeBase::MaxMountedImages && trigger >= 0 && trigger < ShapeBaseImageData::MaxGenericTriggers) {
      object->setImageGenericTriggerState(slot,trigger,state);
      return object->getImageGenericTriggerState(slot,trigger);
   }
   return -1;
}

DefineEngineMethod( ShapeBase, getImageAltTrigger, bool, ( S32 slot ),,
   "@brief Get the alt trigger state of the Image mounted in the specified slot.\n\n"

   "@param slot Image slot to query\n"
   "@return the Image's current alt trigger state\n\n" )
{
   if (slot >= 0 && slot < ShapeBase::MaxMountedImages)
      return object->getImageAltTriggerState(slot);
   return false;
}

DefineEngineMethod( ShapeBase, setImageAltTrigger, bool, ( S32 slot, bool state ),,
   "@brief Set the alt trigger state of the Image mounted in the specified slot.\n\n"

   "@param slot Image slot to modify\n"
   "@param state new alt trigger state for the Image\n"
   "@return the Image's new alt trigger state\n\n" )
{
   if (slot >= 0 && slot < ShapeBase::MaxMountedImages) {
      object->setImageAltTriggerState(slot,state);
      return object->getImageAltTriggerState(slot);
   }
   return false;
}

DefineEngineMethod( ShapeBase, getImageAmmo, bool, ( S32 slot ),,
   "@brief Get the ammo state of the Image mounted in the specified slot.\n\n"

   "@param slot Image slot to query\n"
   "@return the Image's current ammo state\n\n" )
{
   if (slot >= 0 && slot < ShapeBase::MaxMountedImages)
      return object->getImageAmmoState(slot);
   return false;
}

DefineEngineMethod( ShapeBase, setImageAmmo, bool, ( S32 slot, bool state ),,
   "@brief Set the ammo state of the Image mounted in the specified slot.\n\n"

   "@param slot Image slot to modify\n"
   "@param state new ammo state for the Image\n"
   "@return the Image's new ammo state\n\n" )
{
   if (slot >= 0 && slot < ShapeBase::MaxMountedImages) {
      object->setImageAmmoState(slot,state);
      return state;
   }
   return false;
}

DefineEngineMethod( ShapeBase, getImageLoaded, bool, ( S32 slot ),,
   "@brief Get the loaded state of the Image mounted in the specified slot.\n\n"

   "@param slot Image slot to query\n"
   "@return the Image's current loaded state\n\n" )
{
   if (slot >= 0 && slot < ShapeBase::MaxMountedImages)
      return object->getImageLoadedState(slot);
   return false;
}

DefineEngineMethod( ShapeBase, setImageLoaded, bool, ( S32 slot, bool state ),,
   "@brief Set the loaded state of the Image mounted in the specified slot.\n\n"

   "@param slot Image slot to modify\n"
   "@param state new loaded state for the Image\n"
   "@return the Image's new loaded state\n\n" )
{
   if (slot >= 0 && slot < ShapeBase::MaxMountedImages) {
      object->setImageLoadedState(slot, state);
      return state;
   }
   return false;
}

DefineEngineMethod( ShapeBase, getImageTarget, bool, ( S32 slot ),,
   "@brief Get the target state of the Image mounted in the specified slot.\n\n"

   "@param slot Image slot to query\n"
   "@return the Image's current target state\n\n" )
{
   if (slot >= 0 && slot < ShapeBase::MaxMountedImages)
      return object->getImageTargetState(slot);
   return false;
}

DefineEngineMethod( ShapeBase, setImageTarget, bool, ( S32 slot, bool state ),,
   "@brief Set the target state of the Image mounted in the specified slot.\n\n"

   "@param slot Image slot to modify\n"
   "@param state new target state for the Image\n"
   "@return the Image's new target state\n\n" )
{
   if (slot >= 0 && slot < ShapeBase::MaxMountedImages) {
      object->setImageTargetState(slot,state);
      return state;
   }
   return false;
}

DefineEngineMethod( ShapeBase, getImageScriptAnimPrefix, const char*, ( S32 slot ),,
   "@brief Get the script animation prefix of the Image mounted in the specified slot.\n\n"

   "@param slot Image slot to query\n"
   "@return the Image's current script animation prefix\n\n" )
{
   if (slot >= 0 && slot < ShapeBase::MaxMountedImages)
      return object->getImageScriptAnimPrefix(slot).getString();
   return "";
}

DefineEngineMethod( ShapeBase, setImageScriptAnimPrefix, void, ( S32 slot, const char* prefix ),,
   "@brief Set the script animation prefix for the Image mounted in the specified slot.\n\n"
   "This is used to further modify the prefix used when deciding which animation sequence to "
   "play while this image is mounted.\n"

   "@param slot Image slot to modify\n"
   "@param prefix The prefix applied to the image\n" )
{
   if (slot >= 0 && slot < ShapeBase::MaxMountedImages) {

      NetStringHandle prefixHandle;
      if (prefix[0] == StringTagPrefixByte)
         prefixHandle = NetStringHandle(U32(dAtoi(prefix+1)));

      object->setImageScriptAnimPrefix(slot, prefixHandle);
   }
}

DefineEngineMethod( ShapeBase, getMuzzleVector, VectorF, ( S32 slot ),,
   "@brief Get the muzzle vector of the Image mounted in the specified slot.\n\n"

   "If the Image shape contains a node called 'muzzlePoint', then the muzzle "
   "vector is the forward direction vector of that node's transform in world "
   "space. If no such node is specified, the slot's mount node is used "
   "instead.\n\n"

   "If the correctMuzzleVector flag (correctMuzzleVectorTP in 3rd person) "
   "is set in the Image, the muzzle vector is computed to point at whatever "
   "object is right in front of the object's 'eye' node.\n"

   "@param slot Image slot to query\n"
   "@return the muzzle vector, or \"0 1 0\" if the slot is invalid\n\n" )
{
   VectorF vec(0, 1, 0);
   if (slot >= 0 && slot < ShapeBase::MaxMountedImages)
      object->getMuzzleVector(slot, &vec);

   return vec;
}

DefineEngineMethod( ShapeBase, getMuzzlePoint, Point3F, ( S32 slot ),,
   "@brief Get the muzzle position of the Image mounted in the specified slot.\n\n"

   "If the Image shape contains a node called 'muzzlePoint', then the muzzle "
   "position is the position of that node in world space. If no such node "
   "is specified, the slot's mount node is used instead.\n"

   "@param slot Image slot to query\n"
   "@return the muzzle position, or \"0 0 0\" if the slot is invalid\n\n" )
{
   Point3F pos(0, 0, 0);
   if (slot >= 0 && slot < ShapeBase::MaxMountedImages)
      object->getMuzzlePoint(slot, &pos);

   return pos;
}

DefineEngineMethod( ShapeBase, getSlotTransform, TransformF, ( S32 slot ),,
   "@brief Get the world transform of the specified mount slot.\n\n"

   "@param slot Image slot to query\n"
   "@return the mount transform\n\n" )
{
   MatrixF xf(true);
   if (slot >= 0 && slot < ShapeBase::MaxMountedImages)
      object->getMountTransform( slot, MatrixF::Identity, &xf );

   return xf;
}

//----------------------------------------------------------------------------

DefineEngineMethod( ShapeBase, getAIRepairPoint, Point3F, (),,
   "@brief Get the position at which the AI should stand to repair things.\n\n"

   "If the shape defines a node called \"AIRepairNode\", this method will "
   "return the current world position of that node, otherwise \"0 0 0\".\n"
   "@return the AI repair position\n\n" )
{
   return object->getAIRepairPoint();
}

DefineEngineMethod( ShapeBase, getVelocity, VectorF, (),,
   "@brief Get the object's current velocity.\n\n"

   "@return the current velocity\n\n" )
{
   return object->getVelocity();
}

DefineEngineMethod( ShapeBase, setVelocity, bool, ( Point3F vel ),,
   "@brief Set the object's velocity.\n\n"

   "@param vel new velocity for the object\n"
   "@return true\n\n" )
{
   object->setVelocity( vel );
   return true;
}

DefineEngineMethod( ShapeBase, applyImpulse, bool, ( Point3F pos, Point3F vec ),,
   "@brief Apply an impulse to the object.\n\n"

   "@param pos world position of the impulse\n"
   "@param vec impulse momentum (velocity * mass)\n"
   "@return true\n\n" )
{
   object->applyImpulse( pos, vec );
   return true;
}

DefineEngineMethod( ShapeBase, getEyeVector, VectorF, (),,
   "@brief Get the forward direction of the 'eye' for this object.\n\n"

   "If the object model has a node called 'eye', this method will return that "
   "node's current forward direction vector, otherwise it will return the "
   "object's current forward direction vector.\n"

   "@return the eye vector for this object\n"

   "@see getEyePoint\n"
   "@see getEyeTransform\n" )
{
   MatrixF mat;
   object->getEyeTransform(&mat);
   return mat.getForwardVector();
}

DefineEngineMethod( ShapeBase, getEyePoint, Point3F, (),,
   "@brief Get the position of the 'eye' for this object.\n\n"

   "If the object model has a node called 'eye', this method will return that "
   "node's current world position, otherwise it will return the object's current "
   "world position.\n"

   "@return the eye position for this object\n"

   "@see getEyeVector\n"
   "@see getEyeTransform\n" )
{
   MatrixF mat;
   object->getEyeTransform(&mat);
   return mat.getPosition();
}

DefineEngineMethod( ShapeBase, getEyeTransform, TransformF, (),,
   "@brief Get the 'eye' transform for this object.\n\n"

   "If the object model has a node called 'eye', this method will return that "
   "node's current transform, otherwise it will return the object's current "
   "transform.\n"

   "@return the eye transform for this object\n"

   "@see getEyeVector\n"
   "@see getEyePoint\n" )
{
   MatrixF mat;
   object->getEyeTransform(&mat);
   return mat;
}

DefineEngineMethod( ShapeBase, getLookAtPoint, const char*, ( F32 distance, S32 typeMask ), ( 2000, 0xFFFFFFFF ),
   "@brief Get the world position this object is looking at.\n\n"

   "Casts a ray from the eye and returns information about what the ray hits.\n"

   "@param distance maximum distance of the raycast\n"
   "@param typeMask typeMask of objects to include for raycast collision testing\n"
   "@return look-at information as \"Object HitX HitY HitZ [Material]\" or empty string for no hit\n\n"

   "@tsexample\n"
   "%lookat = %obj.getLookAtPoint();\n"
   "echo( \"Looking at: \" @ getWords( %lookat, 1, 3 ) );\n"
   "@endtsexample\n" )
{
   MatrixF mat;
   object->getEyeTransform( &mat );
   
   // Get eye vector.
   
   VectorF eyeVector;
   mat.getColumn( 1, &eyeVector );
   
   // Get eye position.
   
   VectorF eyePos;
   mat.getColumn( 3, &eyePos );
   
   // Make sure the eye vector covers the distance.
   
   eyeVector *= distance;
   
   // Do a container search.
   
   VectorF start = eyePos;
   VectorF end = eyePos + eyeVector;
   
   RayInfo ri;
   if( !gServerContainer.castRay( start, end, typeMask, &ri ) || !ri.object )
      return ""; // No hit.
   
   // Gather hit info.
      
   enum { BUFFER_SIZE = 256 };
   char* buffer = Con::getReturnBuffer( BUFFER_SIZE );
   if( ri.material )
      dSprintf( buffer, BUFFER_SIZE, "%u %f %f %f %u",
         ri.object->getId(),
         ri.point.x,
         ri.point.y,
         ri.point.z,
         ri.material->getMaterial()->getId() );
   else
      dSprintf( buffer, BUFFER_SIZE, "%u %f %f %f",
         ri.object->getId(),
         ri.point.x,
         ri.point.y,
         ri.point.z );

   return buffer;
}

DefineEngineMethod( ShapeBase, setEnergyLevel, void, ( F32 level ),,
   "@brief Set this object's current energy level.\n\n"

   "@param level new energy level\n"
   
   "@see getEnergyLevel()\n"
   "@see getEnergyPercent()\n")
{
   object->setEnergyLevel( level );
}

DefineEngineMethod( ShapeBase, getEnergyLevel, F32, (),,
   "@brief Get the object's current energy level.\n\n"

   "@return energy level\n"
   
   "@see setEnergyLevel()\n")
{
   return object->getEnergyLevel();
}

DefineEngineMethod( ShapeBase, getEnergyPercent, F32, (),,
   "@brief Get the object's current energy level as a percentage of maxEnergy.\n\n"
   "@return energyLevel / datablock.maxEnergy\n"

   "@see setEnergyLevel()\n")
{
   return object->getEnergyValue();
}

DefineEngineMethod( ShapeBase, setDamageLevel, void, ( F32 level ),,
   "@brief Set the object's current damage level.\n\n"

   "@param level new damage level\n"
   
   "@see getDamageLevel()\n"
   "@see getDamagePercent()\n")
{
   object->setDamageLevel( level );
}

DefineEngineMethod( ShapeBase, getDamageLevel, F32, (),,
   "@brief Get the object's current damage level.\n\n"

   "@return damage level\n"
   
   "@see setDamageLevel()\n")
{
   return object->getDamageLevel();
}

DefineEngineMethod( ShapeBase, getDamagePercent, F32, (),,
   "@brief Get the object's current damage level as a percentage of maxDamage.\n\n"

   "@return damageLevel / datablock.maxDamage\n"
   
   "@see setDamageLevel()\n")
{
   return object->getDamageValue();
}
  
DefineEngineMethod(ShapeBase, getMaxDamage, F32, (),,   
   "Get the object's maxDamage level.\n"  
   "@return datablock.maxDamage\n")    
{    
   return object->getMaxDamage();    
}  

DefineEngineMethod( ShapeBase, setDamageState, bool, ( const char* state ),,
   "@brief Set the object's damage state.\n\n"

   "@param state should be one of \"Enabled\", \"Disabled\", \"Destroyed\"\n"
   "@return true if successful, false if failed\n"
   
   "@see getDamageState()\n")
{
   return object->setDamageState( state );
}

DefineEngineMethod( ShapeBase, getDamageState, const char*, (),,
   "@brief Get the object's damage state.\n\n"

   "@return the damage state; one of \"Enabled\", \"Disabled\", \"Destroyed\"\n"
   
   "@see setDamageState()\n")
{
   return object->getDamageStateName();
}

DefineEngineMethod( ShapeBase, isDestroyed, bool, (),,
   "@brief Check if the object is in the Destroyed damage state.\n\n"

   "@return true if damage state is \"Destroyed\", false if not\n" 
   
   "@see isDisabled()\n"
   "@see isEnabled()\n")
{
   return object->isDestroyed();
}

DefineEngineMethod( ShapeBase, isDisabled, bool, (),,
   "@brief Check if the object is in the Disabled or Destroyed damage state.\n\n"

   "@return true if damage state is not \"Enabled\", false if it is\n"
   
   "@see isDestroyed()\n"
   "@see isEnabled()\n")
{
   return object->getDamageState() != ShapeBase::Enabled;
}

DefineEngineMethod( ShapeBase, isEnabled, bool, (),,
   "@brief Check if the object is in the Enabled damage state.\n\n"

   "@return true if damage state is \"Enabled\", false if not\n"
   
   "@see isDestroyed()\n"
   "@see isDisabled()\n")
{
   return object->getDamageState() == ShapeBase::Enabled;
}

DefineEngineMethod(ShapeBase, blowUp, void, (),, "@brief Explodes an object into pieces.")
{
	object->blowUp();
}

DefineEngineMethod( ShapeBase, applyDamage, void, ( F32 amount ),,
   "@brief Increment the current damage level by the specified amount.\n\n"

   "@param amount value to add to current damage level\n" )
{
   object->applyDamage( amount );
}

DefineEngineMethod( ShapeBase, applyRepair, void, ( F32 amount ),,
   "@brief Repair damage by the specified amount.\n\n"

   "Note that the damage level is only reduced by repairRate per tick, so it may "
   "take several ticks for the total repair to complete.\n"

   "@param amount total repair value (subtracted from damage level over time)\n" )
{
   object->applyRepair( amount );
}

DefineEngineMethod( ShapeBase, setRepairRate, void, ( F32 rate ),,
   "@brief Set amount to repair damage by each tick.\n\n"

   "Note that this value is separate to the repairRate field in ShapeBaseData. "
   "This value will be subtracted from the damage level each tick, whereas the "
   "ShapeBaseData field limits how much of the applyRepair value is subtracted "
   "each tick. Both repair types can be active at the same time.\n"
   
   "@param rate value to subtract from damage level each tick (must be > 0)\n"
   
   "@see getRepairRate()\n")
{
   if(rate < 0)
      rate = 0;
   object->setRepairRate( rate );
}

DefineEngineMethod( ShapeBase, getRepairRate, F32, (),,
   "@brief Get the per-tick repair amount.\n\n"

   "@return the current value to be subtracted from damage level each tick\n"

   "@see setRepairRate\n" )
{
   return object->getRepairRate();
}

DefineEngineMethod( ShapeBase, setRechargeRate, void, ( F32 rate ),,
   "@brief Set the recharge rate.\n\n"

   "The recharge rate is added to the object's current energy level each tick, "
   "up to the maxEnergy level set in the ShapeBaseData datablock.\n"
   
   "@param rate the recharge rate (per tick)\n"
   
   "@see getRechargeRate()\n")
{
   object->setRechargeRate( rate );
}

DefineEngineMethod( ShapeBase, getRechargeRate, F32, (),,
   "@brief Get the current recharge rate.\n\n"

   "@return the recharge rate (per tick)\n"
   
   "@see setRechargeRate()\n")
{
   return object->getRechargeRate();
}

DefineEngineMethod( ShapeBase, getControllingClient, S32, (),,
   "@brief Get the client (if any) that controls this object.\n\n"

   "The controlling client is the one that will send moves to us to act on.\n"

   "@return the ID of the controlling GameConnection, or 0 if this object is not "
   "controlled by any client.\n"
   
   "@see GameConnection\n")
{
   if (GameConnection* con = object->getControllingClient())
      return con->getId();
   return 0;
}

DefineEngineMethod( ShapeBase, getControllingObject, S32, (),,
   "@brief Get the object (if any) that controls this object.\n\n"

   "@return the ID of the controlling ShapeBase object, or 0 if this object is "
   "not controlled by another object.\n" )
{
   if (ShapeBase* con = object->getControllingObject())
      return con->getId();
   return 0;
}

DefineEngineMethod( ShapeBase, canCloak, bool, (),,
   "@brief Check if this object can cloak.\n\n"
   "@return true\n"
   
   "@note Not implemented as it always returns true.")
{
   return true;
}

DefineEngineMethod( ShapeBase, setCloaked, void, ( bool cloak ),,
   "@brief Set the cloaked state of this object.\n\n"

   "When an object is cloaked it is not rendered.\n"

   "@param cloak true to cloak the object, false to uncloak\n"
   
   "@see isCloaked()\n")
{
   if (object->isServerObject())
      object->setCloakedState( cloak );
}

DefineEngineMethod( ShapeBase, isCloaked, bool, (),,
   "@brief Check if this object is cloaked.\n\n"

   "@return true if cloaked, false if not\n" 
   
   "@see setCloaked()\n")
{
   return object->getCloakedState();
}

DefineEngineMethod( ShapeBase, setDamageFlash, void, ( F32 level ),,
   "@brief Set the damage flash level.\n\n"

   "Damage flash may be used as a postfx effect to flash the screen when the "
   "client is damaged.\n"

   "@note Relies on the flash postFx.\n"

   "@param level flash level (0-1)\n"
   
   "@see getDamageFlash()\n")
{
   if (object->isServerObject())
      object->setDamageFlash( level );
}

DefineEngineMethod( ShapeBase, getDamageFlash, F32, (),,
   "@brief Get the damage flash level.\n\n"

   "@return flash level\n"

   "@see setDamageFlash\n" )
{
   return object->getDamageFlash();
}

DefineEngineMethod( ShapeBase, setWhiteOut, void, ( F32 level ),,
   "@brief Set the white-out level.\n\n"

   "White-out may be used as a postfx effect to brighten the screen in response "
   "to a game event.\n"

   "@note Relies on the flash postFx.\n"

   "@param level flash level (0-1)\n"
   
   "@see getWhiteOut()\n")
{
   if (object->isServerObject())
      object->setWhiteOut( level );
}

DefineEngineMethod( ShapeBase, getWhiteOut, F32, (),,
   "@brief Get the white-out level.\n\n"

   "@return white-out level\n"

   "@see setWhiteOut\n" )
{
   return object->getWhiteOut();
}

DefineEngineMethod( ShapeBase, getDefaultCameraFov, F32, (),,
   "@brief Returns the default vertical field of view in degrees for this object if used as a camera.\n\n"

   "@return Default FOV\n" )
{
   if (object->isServerObject())
      return object->getDefaultCameraFov();
   return 0.0;
}

DefineEngineMethod( ShapeBase, getCameraFov, F32, (),,
   "@brief Returns the vertical field of view in degrees for this object if used as a camera.\n\n"

   "@return current FOV as defined in ShapeBaseData::cameraDefaultFov\n" )
{
   if (object->isServerObject())
      return object->getCameraFov();
   return 0.0;
}

DefineEngineMethod( ShapeBase, setCameraFov, void, ( F32 fov ),,
   "@brief Set the vertical field of view in degrees for this object if used as a camera.\n\n"

   "@param fov new FOV value\n" )
{
   if (object->isServerObject())
      object->setCameraFov( fov );
}

DefineEngineMethod( ShapeBase, startFade, void, ( S32 time, S32 delay, bool fadeOut ),,
   "@brief Fade the object in or out without removing it from the scene.\n\n"

   "A faded out object is still in the scene and can still be collided with, "
   "so if you want to disable collisions for this shape after it fades out "
   "use setHidden to temporarily remove this shape from the scene.\n"
  
   "@note Items have the ability to light their surroundings. When an Item with "
   "an active light is fading out, the light it emits is correspondingly "
   "reduced until it goes out. Likewise, when the item fades in, the light is "
   "turned-up till it reaches it's normal brightntess.\n"

   "@param time duration of the fade effect in ms\n"
   "@param delay delay in ms before the fade effect begins\n"
   "@param fadeOut true to fade-out to invisible, false to fade-in to full visibility\n" )
{
   object->startFade( (F32)time / (F32)1000.0, delay / 1000.0, fadeOut );
}

DefineEngineMethod( ShapeBase, setDamageVector, void, ( Point3F vec ),,
   "@brief Set the damage direction vector.\n\n"

   "Currently this is only used to initialise the explosion if this object "
   "is blown up.\n"

   "@param vec damage direction vector\n\n"

   "@tsexample\n"
   "%obj.setDamageVector( \"0 0 1\" );\n"
   "@endtsexample\n" )
{
   vec.normalize();
   object->setDamageDir( vec );
}

DefineEngineMethod( ShapeBase, setShapeName, void, ( const char* name ),,
   "@brief Set the name of this shape.\n\n"

   "@note This is the name of the shape object that is sent to the client, "
   "not the DTS or DAE model filename.\n"

   "@param name new name for the shape\n\n"
   
   "@see getShapeName()\n")
{
   object->setShapeName( name );
}

DefineEngineMethod( ShapeBase, getShapeName, const char*, (),,
   "@brief Get the name of the shape.\n\n"

   "@note This is the name of the shape object that is sent to the client, "
   "not the DTS or DAE model filename.\n"

   "@return the name of the shape\n\n" 
   
   "@see setShapeName()\n")
{
   return object->getShapeName();
}

DefineEngineMethod( ShapeBase, setSkinName, void, ( const char* name ),,
   "@brief Apply a new skin to this shape.\n\n"

   "'Skinning' the shape effectively renames the material targets, allowing "
   "different materials to be used on different instances of the same model.\n\n"

   "@param name name of the skin to apply\n\n"

   "@see skin\n"
   "@see getSkinName()\n")
{
   object->setSkinName( name );
}

DefineEngineMethod( ShapeBase, getSkinName, const char*, (),,
   "@brief Get the name of the skin applied to this shape.\n\n"

   "@return the name of the skin\n\n" 

   "@see skin\n"
   "@see setSkinName()\n")
{
   return object->getSkinName();
}

//----------------------------------------------------------------------------
void ShapeBase::consoleInit()
{
   Con::addVariable("SB::DFDec", TypeF32, &sDamageFlashDec, "Speed to reduce the damage flash effect per tick.\n\n"
      "@see ShapeBase::setDamageFlash()\n"
      "@see ShapeBase::getDamageFlash()\n"
      "@note Relies on the flash postFx.\n"
	   "@ingroup gameObjects\n");
   Con::addVariable("SB::WODec", TypeF32, &sWhiteoutDec, "Speed to reduce the whiteout effect per tick.\n\n"
      "@see ShapeBase::setWhiteOut()\n"
      "@see ShapeBase::getWhiteOut"
      "@note Relies on the flash postFx.\n"
	   "@ingroup gameObjects\n");
   Con::addVariable("SB::FullCorrectionDistance", TypeF32, &sFullCorrectionDistance, 
      "@brief Distance at which a weapon's muzzle vector is fully corrected to match where the player is looking.\n\n"
      "When a weapon image has correctMuzzleVector set and the Player is in 1st person, the muzzle vector from the "
      "weapon is modified to match where the player is looking.  Beyond the FullCorrectionDistance the muzzle vector "
      "is always corrected.  Between FullCorrectionDistance and the player, the weapon's muzzle vector is adjusted so that "
      "the closer the aim point is to the player, the closer the muzzle vector is to the true (non-corrected) one.\n"
	   "@ingroup gameObjects\n");
   Con::addVariable("SB::CloakSpeed", TypeF32, &sCloakSpeed, 
      "@brief Time to cloak, in seconds.\n\n"
	   "@ingroup gameObjects\n");
}

void ShapeBase::_updateHiddenMeshes()
{
   if ( !mShapeInstance )
      return;

   // This may happen at some point in the future... lets
   // detect it so that it can be fixed at that time.
   AssertFatal( mMeshHidden.getSize() == mShapeInstance->mMeshObjects.size(),
      "ShapeBase::_updateMeshVisibility() - Mesh visibility size mismatch!" );

   for ( U32 i = 0; i < mMeshHidden.getSize(); i++ )
      setMeshHidden( i, mMeshHidden.test( i ) );
}

void ShapeBase::setMeshHidden( const char *meshName, bool forceHidden )
{
   setMeshHidden( mDataBlock->mShape->findObject( meshName ), forceHidden );
}

void ShapeBase::setMeshHidden( S32 meshIndex, bool forceHidden )
{
   if ( meshIndex == -1 || meshIndex >= mMeshHidden.getSize() )
      return;

   if ( forceHidden )
      mMeshHidden.set( meshIndex );   
   else
      mMeshHidden.clear( meshIndex );   

   if ( mShapeInstance )
      mShapeInstance->setMeshForceHidden( meshIndex, forceHidden );

   setMaskBits( MeshHiddenMask );
}

void ShapeBase::setAllMeshesHidden( bool forceHidden )
{
   if ( forceHidden )
      mMeshHidden.set();
   else
      mMeshHidden.clear();

   if ( mShapeInstance )
   {
      for ( U32 i = 0; i < mMeshHidden.getSize(); i++ )
         mShapeInstance->setMeshForceHidden( i, forceHidden );
   }

   setMaskBits( MeshHiddenMask );
}

DefineEngineMethod( ShapeBase, setAllMeshesHidden, void, ( bool hide ),,
   "@brief Set the hidden state on all the shape meshes.\n\n"

   "This allows you to hide all meshes in the shape, for example, and then only "
   "enable a few.\n"

   "@param hide new hidden state for all meshes\n\n" )
{
   object->setAllMeshesHidden( hide );
}

DefineEngineMethod( ShapeBase, setMeshHidden, void, ( const char* name, bool hide ),,
   "@brief Set the hidden state on the named shape mesh.\n\n"

   "@param name name of the mesh to hide/show\n"
   "@param hide new hidden state for the mesh\n\n" )
{
   object->setMeshHidden( name, hide );
}

// Some development-handy functions
#ifndef TORQUE_SHIPPING

void ShapeBase::dumpMeshVisibility()
{
   if ( !mShapeInstance )
      return;

   const Vector<TSShapeInstance::MeshObjectInstance> &meshes = mShapeInstance->mMeshObjects;

   for ( U32 i = 0; i < meshes.size(); i++)
   {
      const TSShapeInstance::MeshObjectInstance &mesh = meshes[i];

      const String &meshName = mDataBlock->mShape->getMeshName( i );

      Con::printf( "%d - %s - forceHidden = %s, visibility = %f", 
         i,
         meshName.c_str(),
         mesh.forceHidden ? "true" : "false",
         mesh.visible );
   }
}

DefineEngineMethod( ShapeBase, dumpMeshVisibility, void, (),,
   "@brief Print a list of visible and hidden meshes in the shape to the console "
   "for debugging purposes.\n\n"
   "@note Only in a SHIPPING build.\n")
{
   object->dumpMeshVisibility();
}

#endif // #ifndef TORQUE_SHIPPING

//------------------------------------------------------------------------
//These functions are duplicated in tsStatic and shapeBase.
//They each function a little differently; but achieve the same purpose of gathering
//target names/counts without polluting simObject.

DefineEngineMethod( ShapeBase, getTargetName, const char*, ( S32 index ),,
   "@brief Get the name of the indexed shape material.\n\n"

   "@param index index of the material to get (valid range is 0 - getTargetCount()-1).\n"
   "@return the name of the indexed material.\n\n"
   
   "@see getTargetCount()\n")
{
	ShapeBase *obj = dynamic_cast< ShapeBase* > ( object );
	if(obj)
	{
		// Try to use the client object (so we get the reskinned targets in the Material Editor)
		if ((ShapeBase*)obj->getClientObject())
			obj = (ShapeBase*)obj->getClientObject();

		return obj->getShapeInstance()->getTargetName(index);
	}

	return "";
}

DefineEngineMethod( ShapeBase, getTargetCount, S32, (),,
   "@brief Get the number of materials in the shape.\n\n"

   "@return the number of materials in the shape.\n\n" 
   
   "@see getTargetName()\n")
{
   ShapeBase *obj = dynamic_cast< ShapeBase* > ( object );
   if(obj)
   {
      // Try to use the client object (so we get the reskinned targets in the Material Editor)
      if ((ShapeBase*)obj->getClientObject())
         obj = (ShapeBase*)obj->getClientObject();

      if (obj->getShapeInstance() != NULL)
         return obj->getShapeInstance()->getTargetCount();
	}
   
   return -1;
}

DefineEngineMethod( ShapeBase, changeMaterial, void, ( const char* mapTo, Material* oldMat, Material* newMat ),,
   "@brief Change one of the materials on the shape.\n\n"

   "This method changes materials per mapTo with others. The material that "
   "is being replaced is mapped to unmapped_mat as a part of this transition.\n"

   "@note Warning, right now this only sort of works. It doesn't do a live "
   "update like it should.\n"

   "@param mapTo the name of the material target to remap (from getTargetName)\n"
   "@param oldMat the old Material that was mapped \n"
   "@param newMat the new Material to map\n\n"

   "@tsexample\n"
   "// remap the first material in the shape\n"
   "%mapTo = %obj.getTargetName( 0 );\n"
   "%obj.changeMaterial( %mapTo, 0, MyMaterial );\n"
   "@endtsexample\n" )
{
   // if no valid new material, theres no reason for doing this
   if( !newMat )
   {
      Con::errorf("ShapeBase::changeMaterial failed: New material does not exist!");
      return;
   }

   // initilize server/client versions
   ShapeBase *serverObj = object;
   ShapeBase *clientObj = dynamic_cast< ShapeBase* > ( object->getClientObject() );

   // Check the mapTo name exists for this shape
   S32 matIndex = serverObj->getShape()->materialList->getMaterialNameList().find_next(String(mapTo));
   if (matIndex < 0)
   {
      Con::errorf("ShapeBase::changeMaterial failed: Invalid mapTo name '%s'", mapTo);
      return;
   }

   // Lets remap the old material off, so as to let room for our current material room to claim its spot
   if( oldMat )
      oldMat->mMapTo = String("unmapped_mat");

   newMat->mMapTo = mapTo;

   // Map the material by name in the matmgr
   MATMGR->mapMaterial( mapTo, newMat->getName() );

   // Replace instances with the new material being traded in. For ShapeBase
   // class we have to update the server/client objects separately so both
   // represent our changes
   delete serverObj->getShape()->materialList->mMatInstList[matIndex];
   serverObj->getShape()->materialList->mMatInstList[matIndex] = newMat->createMatInstance();
   if (clientObj)
   {
      delete clientObj->getShape()->materialList->mMatInstList[matIndex];
      clientObj->getShape()->materialList->mMatInstList[matIndex] = newMat->createMatInstance();
   }

   // Finish up preparing the material instances for rendering
   const GFXVertexFormat *flags = getGFXVertexFormat<GFXVertexPNTTB>();
   FeatureSet features = MATMGR->getDefaultFeatures();

   serverObj->getShape()->materialList->getMaterialInst(matIndex)->init( features, flags );
   if (clientObj)
      clientObj->getShapeInstance()->mMaterialList->getMaterialInst(matIndex)->init( features, flags );
}

DefineEngineMethod( ShapeBase, getModelFile, const char *, (),,
   "@brief Get the model filename used by this shape.\n\n"

   "@return the shape filename\n\n" )
{
	GameBaseData * datablock = object->getDataBlock();
	if( !datablock )
		return String::EmptyString;

	const char *fieldName = StringTable->insert( String("shapeFile") );
   return datablock->getDataField( fieldName, NULL );
}
