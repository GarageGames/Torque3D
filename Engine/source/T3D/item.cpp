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
#include "T3D/item.h"

#include "core/stream/bitStream.h"
#include "math/mMath.h"
#include "console/console.h"
#include "console/consoleTypes.h"
#include "sim/netConnection.h"
#include "collision/boxConvex.h"
#include "collision/earlyOutPolyList.h"
#include "collision/extrudedPolyList.h"
#include "math/mPolyhedron.h"
#include "math/mathIO.h"
#include "lighting/lightInfo.h"
#include "lighting/lightManager.h"
#include "T3D/physics/physicsPlugin.h"
#include "T3D/physics/physicsBody.h"
#include "T3D/physics/physicsCollision.h"
#include "ts/tsShapeInstance.h"
#include "console/engineAPI.h"


const F32 sRotationSpeed = 6.0f;        // Secs/Rotation
const F32 sAtRestVelocity = 0.15f;      // Min speed after collision
const S32 sCollisionTimeout = 15;       // Timout value in ticks

// Client prediction
static F32 sMinWarpTicks = 0.5 ;        // Fraction of tick at which instant warp occures
static S32 sMaxWarpTicks = 3;           // Max warp duration in ticks

F32 Item::mGravity = -20.0f;

const U32 sClientCollisionMask = (TerrainObjectType     |
                                  StaticShapeObjectType |
                                  VehicleObjectType     |  
                                  PlayerObjectType);

const U32 sServerCollisionMask = (sClientCollisionMask);

const S32 Item::csmAtRestTimer = 64;

//----------------------------------------------------------------------------

IMPLEMENT_CO_DATABLOCK_V1(ItemData);

ConsoleDocClass( ItemData,
   "@brief Stores properties for an individual Item type.\n\n"   

   "Items represent an object in the world, usually one that the player will interact with.  "
   "One example is a health kit on the group that is automatically picked up when the player "
   "comes into contact with it.\n\n"

   "ItemData provides the common properties for a set of Items.  These properties include a "
   "DTS or DAE model used to render the Item in the world, its physical properties for when the "
   "Item interacts with the world (such as being tossed by the player), and any lights that emit "
   "from the Item.\n\n"

   "@tsexample\n"
	   "datablock ItemData(HealthKitSmall)\n"
      "{\n"
	   "   category =\"Health\";\n"
	   "   className = \"HealthPatch\";\n"
	   "   shapeFile = \"art/shapes/items/kit/healthkit.dts\";\n"
	   "   gravityMod = \"1.0\";\n"
	   "   mass = 2;\n"
	   "   friction = 1;\n"
	   "   elasticity = 0.3;\n"
      "   density = 2;\n"
	   "   drag = 0.5;\n"
	   "   maxVelocity = \"10.0\";\n"
	   "   emap = true;\n"
	   "   sticky = false;\n"
	   "   dynamicType = \"0\"\n;"
	   "   lightOnlyStatic = false;\n"
	   "   lightType = \"NoLight\";\n"
	   "   lightColor = \"1.0 1.0 1.0 1.0\";\n"
	   "   lightTime = 1000;\n"
	   "   lightRadius = 10.0;\n"
      "   simpleServerCollision = true;"
      "   // Dynamic properties used by the scripts\n\n"
      "   pickupName = \"a small health kit\";\n"
	   "   repairAmount = 50;\n"
	   "};\n"
   "@endtsexample\n"

   "@ingroup gameObjects\n"
);


ItemData::ItemData()
{
   shadowEnable = true;


   friction = 0;
   elasticity = 0;

   sticky = false;
   gravityMod = 1.0;
   maxVelocity = 25.0f;

   density = 2;
   drag = 0.5;

   lightOnlyStatic = false;
   lightType = Item::NoLight;
   lightColor.set(1.f,1.f,1.f,1.f);
   lightTime = 1000;
   lightRadius = 10.f; 

   simpleServerCollision = true;
}

ImplementEnumType( ItemLightType,
   "@brief The type of light the Item has\n\n"
   "@ingroup gameObjects\n\n")
   { Item::NoLight,           "NoLight",        "The item has no light attached.\n" },
   { Item::ConstantLight,     "ConstantLight",  "The item has a constantly emitting light attached.\n" },
   { Item::PulsingLight,      "PulsingLight",   "The item has a pulsing light attached.\n" }
EndImplementEnumType;

void ItemData::initPersistFields()
{
   addField("friction",          TypeF32,       Offset(friction,           ItemData), "A floating-point value specifying how much velocity is lost to impact and sliding friction.");
   addField("elasticity",        TypeF32,       Offset(elasticity,         ItemData), "A floating-point value specifying how 'bouncy' this ItemData is.");
   addField("sticky",            TypeBool,      Offset(sticky,             ItemData), 
      "@brief If true, ItemData will 'stick' to any surface it collides with.\n\n"
      "When an item does stick to a surface, the Item::onStickyCollision() callback is called.  The Item has methods to retrieve "
      "the world position and normal the Item is stuck to.\n"
      "@note Valid objects to stick to must be of StaticShapeObjectType.\n");
   addField("gravityMod",        TypeF32,       Offset(gravityMod,         ItemData), "Floating point value to multiply the existing gravity with, just for this ItemData.");
   addField("maxVelocity",       TypeF32,       Offset(maxVelocity,        ItemData), "Maximum velocity that this ItemData is able to move.");

   addField("lightType",         TYPEID< Item::LightType >(),      Offset(lightType, ItemData), "Type of light to apply to this ItemData. Options are NoLight, ConstantLight, PulsingLight. Default is NoLight." );
   addField("lightColor",        TypeColorF,    Offset(lightColor,         ItemData),
      "@brief Color value to make this light. Example: \"1.0,1.0,1.0\"\n\n"
      "@see lightType\n");
   addField("lightTime",         TypeS32,       Offset(lightTime,          ItemData), 
      "@brief Time value for the light of this ItemData, used to control the pulse speed of the PulsingLight LightType.\n\n"
      "@see lightType\n");
   addField("lightRadius",       TypeF32,       Offset(lightRadius,        ItemData), 
      "@brief Distance from the center point of this ItemData for the light to affect\n\n"
      "@see lightType\n");
   addField("lightOnlyStatic",   TypeBool,      Offset(lightOnlyStatic,    ItemData), 
      "@brief If true, this ItemData will only cast a light if the Item for this ItemData has a static value of true.\n\n"
      "@see lightType\n");

   addField("simpleServerCollision",   TypeBool,  Offset(simpleServerCollision,    ItemData), 
      "@brief Determines if only simple server-side collision will be used (for pick ups).\n\n"
      "If set to true then only simple, server-side collision detection will be used.  This is often the case "
      "if the item is used for a pick up object, such as ammo.  If set to false then a full collision volume "
      "will be used as defined by the shape.  The default is true.\n"
      "@note Only applies when using a physics library.\n"
      "@see TurretShape and ProximityMine for examples that should set this to false to allow them to be "
      "shot by projectiles.\n");

   Parent::initPersistFields();
}

void ItemData::packData(BitStream* stream)
{
   Parent::packData(stream);
   stream->writeFloat(friction, 10);
   stream->writeFloat(elasticity, 10);
   stream->writeFlag(sticky);
   if(stream->writeFlag(gravityMod != 1.0))
      stream->writeFloat(gravityMod, 10);
   if(stream->writeFlag(maxVelocity != -1))
      stream->write(maxVelocity);

   if(stream->writeFlag(lightType != Item::NoLight))
   {
      AssertFatal(Item::NumLightTypes < (1 << 2), "ItemData: light type needs more bits");
      stream->writeInt(lightType, 2);
      stream->writeFloat(lightColor.red, 7);
      stream->writeFloat(lightColor.green, 7);
      stream->writeFloat(lightColor.blue, 7);
      stream->writeFloat(lightColor.alpha, 7);
      stream->write(lightTime);
      stream->write(lightRadius);
      stream->writeFlag(lightOnlyStatic);
   }

   stream->writeFlag(simpleServerCollision);
}

void ItemData::unpackData(BitStream* stream)
{
   Parent::unpackData(stream);
   friction = stream->readFloat(10);
   elasticity = stream->readFloat(10);
   sticky = stream->readFlag();
   if(stream->readFlag())
      gravityMod = stream->readFloat(10);
   else
      gravityMod = 1.0;

   if(stream->readFlag())
      stream->read(&maxVelocity);
   else
      maxVelocity = -1;

   if(stream->readFlag())
   {
      lightType = stream->readInt(2);
      lightColor.red = stream->readFloat(7);
      lightColor.green = stream->readFloat(7);
      lightColor.blue = stream->readFloat(7);
      lightColor.alpha = stream->readFloat(7);
      stream->read(&lightTime);
      stream->read(&lightRadius);
      lightOnlyStatic = stream->readFlag();
   }
   else
      lightType = Item::NoLight;

   simpleServerCollision = stream->readFlag();
}


//----------------------------------------------------------------------------

IMPLEMENT_CO_NETOBJECT_V1(Item);

ConsoleDocClass( Item,
   "@brief Base Item class. Uses the ItemData datablock for common properties.\n\n"   

   "Items represent an object in the world, usually one that the player will interact with.  "
   "One example is a health kit on the group that is automatically picked up when the player "
   "comes into contact with it.\n\n"

   "@tsexample\n"
      "// This is the \"health patch\" dropped by a dying player.\n"
      "datablock ItemData(HealthKitPatch)\n"
      "{\n"
      "   // Mission editor category, this datablock will show up in the\n"
      "   // specified category under the \"shapes\" root category.\n"
      "   category = \"Health\";\n\n"
      "   className = \"HealthPatch\";\n\n"
      "   // Basic Item properties\n"
      "   shapeFile = \"art/shapes/items/patch/healthpatch.dts\";\n"
      "   mass = 2;\n"
      "   friction = 1;\n"
      "   elasticity = 0.3;\n"
      "   emap = true;\n\n"
      "   // Dynamic properties used by the scripts\n"
      "   pickupName = \"a health patch\";\n"
      "   repairAmount = 50;\n"
      "};\n\n"

	   "%obj = new Item()\n"
	   "{\n"
	   "	dataBlock = HealthKitSmall;\n"
	   "	parentGroup = EWCreatorWindow.objectGroup;\n"
	   "	static = true;\n"
	   "	rotate = true;\n"
	   "};\n"
	"@endtsexample\n\n"

   "@see ItemData\n"

   "@ingroup gameObjects\n"
);

IMPLEMENT_CALLBACK( Item, onStickyCollision, void, ( const char* objID ),( objID ),
   "@brief Informs the Item object that it is now sticking to another object.\n\n"
   "This callback is only called if the ItemData::sticky property for this Item is true.\n"
   "@param objID Object ID this Item object.\n"
   "@note Server side only.\n"
   "@see Item, ItemData\n"
);

IMPLEMENT_CALLBACK( Item, onEnterLiquid, void, ( const char* objID, const char* waterCoverage, const char* liquidType ),( objID, waterCoverage, liquidType ),
   "Informs an Item object that it has entered liquid, along with information about the liquid type.\n"
   "@param objID Object ID for this Item object.\n"
   "@param waterCoverage How much coverage of water this Item object has.\n"
   "@param liquidType The type of liquid that this Item object has entered.\n"
   "@note Server side only.\n"
   "@see Item, ItemData, WaterObject\n"
);

IMPLEMENT_CALLBACK( Item, onLeaveLiquid, void, ( const char* objID, const char* liquidType ),( objID, liquidType ),
   "Informs an Item object that it has left a liquid, along with information about the liquid type.\n"
   "@param objID Object ID for this Item object.\n"
   "@param liquidType The type of liquid that this Item object has left.\n"
   "@note Server side only.\n"
   "@see Item, ItemData, WaterObject\n"
);


Item::Item()
{
   mTypeMask |= ItemObjectType | DynamicShapeObjectType;
   mDataBlock = 0;
   mStatic = false;
   mRotate = false;
   mVelocity = VectorF(0,0,0);
   mAtRest = true;
   mAtRestCounter = 0;
   mInLiquid = false;
   delta.warpTicks = 0;
   delta.dt = 1;
   mCollisionObject = 0;
   mCollisionTimeout = 0;
   mPhysicsRep = NULL;

   mConvex.init(this);
   mWorkingQueryBox.minExtents.set(-1e9, -1e9, -1e9);
   mWorkingQueryBox.maxExtents.set(-1e9, -1e9, -1e9);

   mLight = NULL;

   mSubclassItemHandlesScene = false;
}

Item::~Item()
{
   SAFE_DELETE(mLight);
}


//----------------------------------------------------------------------------

bool Item::onAdd()
{
   if (!Parent::onAdd() || !mDataBlock)
      return false;

   if (mStatic)
      mAtRest = true;
   mObjToWorld.getColumn(3,&delta.pos);

   // Setup the box for our convex object...
   mObjBox.getCenter(&mConvex.mCenter);
   mConvex.mSize.x = mObjBox.len_x() / 2.0;
   mConvex.mSize.y = mObjBox.len_y() / 2.0;
   mConvex.mSize.z = mObjBox.len_z() / 2.0;
   mWorkingQueryBox.minExtents.set(-1e9, -1e9, -1e9);
   mWorkingQueryBox.maxExtents.set(-1e9, -1e9, -1e9);

   if( !isHidden() && !mSubclassItemHandlesScene )
      addToScene();

   if (isServerObject())
   {
      if (!mSubclassItemHandlesScene)
         scriptOnAdd();
   }
   else if (mDataBlock->lightType != NoLight)
   {
      mDropTime = Sim::getCurrentTime();
   }

   _updatePhysics();

   return true;
}

void Item::_updatePhysics()
{
   SAFE_DELETE( mPhysicsRep );

   if ( !PHYSICSMGR )
      return;

   if (mDataBlock->simpleServerCollision)
   {
      // We only need the trigger on the server.
      if ( isServerObject() )
      {
         PhysicsCollision *colShape = PHYSICSMGR->createCollision();
         colShape->addBox( mObjBox.getExtents() * 0.5f, MatrixF::Identity );

         PhysicsWorld *world = PHYSICSMGR->getWorld( isServerObject() ? "server" : "client" );
         mPhysicsRep = PHYSICSMGR->createBody();
         mPhysicsRep->init( colShape, 0, PhysicsBody::BF_TRIGGER | PhysicsBody::BF_KINEMATIC, this, world );
         mPhysicsRep->setTransform( getTransform() );
      }
   }
   else
   {
      if ( !mShapeInstance )
         return;

      PhysicsCollision* colShape = mShapeInstance->getShape()->buildColShape( false, getScale() );

      if ( colShape )
      {
         PhysicsWorld *world = PHYSICSMGR->getWorld( isServerObject() ? "server" : "client" );
         mPhysicsRep = PHYSICSMGR->createBody();
         mPhysicsRep->init( colShape, 0, PhysicsBody::BF_KINEMATIC, this, world );
         mPhysicsRep->setTransform( getTransform() );
      }
   }
}

bool Item::onNewDataBlock( GameBaseData *dptr, bool reload )
{
   mDataBlock = dynamic_cast<ItemData*>(dptr);
   if (!mDataBlock || !Parent::onNewDataBlock(dptr,reload))
      return false;

   if (!mSubclassItemHandlesScene)
      scriptOnNewDataBlock();

   if ( isProperlyAdded() )
      _updatePhysics();

   return true;
}

void Item::onRemove()
{
   mWorkingQueryBox.minExtents.set(-1e9, -1e9, -1e9);
   mWorkingQueryBox.maxExtents.set(-1e9, -1e9, -1e9);

   SAFE_DELETE( mPhysicsRep );

   if (!mSubclassItemHandlesScene)
   {
      scriptOnRemove();
      removeFromScene();
   }

   Parent::onRemove();
}

void Item::onDeleteNotify( SimObject *obj )
{
   if ( obj == mCollisionObject ) 
   {
      mCollisionObject = NULL;
      mCollisionTimeout = 0;
   }

   Parent::onDeleteNotify( obj );
}

// Lighting: -----------------------------------------------------------------

void Item::registerLights(LightManager * lightManager, bool lightingScene)
{
   if(lightingScene)
      return;

   if(mDataBlock->lightOnlyStatic && !mStatic)
      return;

   F32 intensity;
   switch(mDataBlock->lightType)
   {
      case ConstantLight:
         intensity = mFadeVal;
         break;

      case PulsingLight:
      {
         S32 delta = Sim::getCurrentTime() - mDropTime;
         intensity = 0.5f + 0.5f * mSin(M_PI_F * F32(delta) / F32(mDataBlock->lightTime));
         intensity = 0.15f + intensity * 0.85f;
         intensity *= mFadeVal;  // fade out light on flags
         break;
      }

      default:
         return;
   }

   // Create a light if needed
   if (!mLight)
   {
      mLight = lightManager->createLightInfo();
   }   
   mLight->setColor( mDataBlock->lightColor * intensity );
   mLight->setType( LightInfo::Point );
   mLight->setRange( mDataBlock->lightRadius );
   mLight->setPosition( getBoxCenter() );

   lightManager->registerGlobalLight( mLight, this );
}


//----------------------------------------------------------------------------

Point3F Item::getVelocity() const
{
   return mVelocity;
}

void Item::setVelocity(const VectorF& vel)
{
   mVelocity = vel;

   // Clamp against the maximum velocity.
   if ( mDataBlock->maxVelocity > 0 )
   {
      F32 len = mVelocity.magnitudeSafe();
      if ( len > mDataBlock->maxVelocity )
      {
         Point3F excess = mVelocity * ( 1.0f - (mDataBlock->maxVelocity / len ) );
         mVelocity -= excess;
      }
   }

   setMaskBits(PositionMask);
   mAtRest = false;
   mAtRestCounter = 0;
}

void Item::applyImpulse(const Point3F&,const VectorF& vec)
{
   // Items ignore angular velocity
   VectorF vel;
   vel.x = vec.x / mDataBlock->mass;
   vel.y = vec.y / mDataBlock->mass;
   vel.z = vec.z / mDataBlock->mass;
   setVelocity(vel);
}

void Item::setCollisionTimeout(ShapeBase* obj)
{
   if (mCollisionObject)
      clearNotify(mCollisionObject);
   deleteNotify(obj);
   mCollisionObject = obj;
   mCollisionTimeout = sCollisionTimeout;
   setMaskBits(ThrowSrcMask);
}


//----------------------------------------------------------------------------

void Item::processTick(const Move* move)
{
   Parent::processTick(move);

   //
   if (mCollisionObject && !--mCollisionTimeout)
      mCollisionObject = 0;

   // Warp to catch up to server
   if (delta.warpTicks > 0)
   {
      delta.warpTicks--;

      // Set new pos.
      MatrixF mat = mObjToWorld;
      mat.getColumn(3,&delta.pos);
      delta.pos += delta.warpOffset;
      mat.setColumn(3,delta.pos);
      Parent::setTransform(mat);

      // Backstepping
      delta.posVec.x = -delta.warpOffset.x;
      delta.posVec.y = -delta.warpOffset.y;
      delta.posVec.z = -delta.warpOffset.z;
   }
   else
   {
      if (isServerObject() && mAtRest && (mStatic == false && mDataBlock->sticky == false))
      {
         if (++mAtRestCounter > csmAtRestTimer)
         {
            mAtRest = false;
            mAtRestCounter = 0;
            setMaskBits(PositionMask);
         }
      }

      if (!mStatic && !mAtRest && isHidden() == false)
      {
         updateVelocity(TickSec);
         updateWorkingCollisionSet(isGhost() ? sClientCollisionMask : sServerCollisionMask, TickSec);
         updatePos(isGhost() ? sClientCollisionMask : sServerCollisionMask, TickSec);
      }
      else
      {
         // Need to clear out last updatePos or warp interpolation
         delta.posVec.set(0,0,0);
      }
   }
}

void Item::interpolateTick(F32 dt)
{
   Parent::interpolateTick(dt);

   // Client side interpolation
   Point3F pos = delta.pos + delta.posVec * dt;
   MatrixF mat = mRenderObjToWorld;
   mat.setColumn(3,pos);
   setRenderTransform(mat);
   delta.dt = dt;
}


//----------------------------------------------------------------------------

void Item::setTransform(const MatrixF& mat)
{
   Point3F pos;
   mat.getColumn(3,&pos);
   MatrixF tmat;
   if (!mRotate) {
      // Forces all rotation to be around the z axis
      VectorF vec;
      mat.getColumn(1,&vec);
      tmat.set(EulerF(0,0,-mAtan2(-vec.x,vec.y)));
   }
   else
      tmat.identity();
   tmat.setColumn(3,pos);
   Parent::setTransform(tmat);
   if (!mStatic)
   {
      mAtRest = false;
      mAtRestCounter = 0;
   }

   if ( mPhysicsRep )
      mPhysicsRep->setTransform( getTransform() );

   setMaskBits(RotationMask | PositionMask | NoWarpMask);
}


//----------------------------------------------------------------------------
void Item::updateWorkingCollisionSet(const U32 mask, const F32 dt)
{
   // It is assumed that we will never accelerate more than 10 m/s for gravity...
   //
   Point3F scaledVelocity = mVelocity * dt;
   F32 len    = scaledVelocity.len();
   F32 newLen = len + (10 * dt);

   // Check to see if it is actually necessary to construct the new working list,
   //  or if we can use the cached version from the last query.  We use the x
   //  component of the min member of the mWorkingQueryBox, which is lame, but
   //  it works ok.
   bool updateSet = false;

   Box3F convexBox = mConvex.getBoundingBox(getTransform(), getScale());
   F32 l = (newLen * 1.1) + 0.1;  // from Convex::updateWorkingList
   convexBox.minExtents -= Point3F(l, l, l);
   convexBox.maxExtents += Point3F(l, l, l);

   // Check containment
   {
      if (mWorkingQueryBox.minExtents.x != -1e9)
      {
         if (mWorkingQueryBox.isContained(convexBox) == false)
         {
            // Needed region is outside the cached region.  Update it.
            updateSet = true;
         }
         else
         {
            // We can leave it alone, we're still inside the cached region
         }
      }
      else
      {
         // Must update
         updateSet = true;
      }
   }

   // Actually perform the query, if necessary
   if (updateSet == true)
   {
      mWorkingQueryBox = convexBox;
      mWorkingQueryBox.minExtents -= Point3F(2 * l, 2 * l, 2 * l);
      mWorkingQueryBox.maxExtents += Point3F(2 * l, 2 * l, 2 * l);

      disableCollision();
      if (mCollisionObject)
         mCollisionObject->disableCollision();

      mConvex.updateWorkingList(mWorkingQueryBox, mask);

      if (mCollisionObject)
         mCollisionObject->enableCollision();
      enableCollision();
   }
}

void Item::updateVelocity(const F32 dt)
{
   // Acceleration due to gravity
   mVelocity.z += (mGravity * mDataBlock->gravityMod) * dt;
   F32 len;
   if (mDataBlock->maxVelocity > 0 && (len = mVelocity.len()) > (mDataBlock->maxVelocity * 1.05)) {
      Point3F excess = mVelocity * (1.0 - (mDataBlock->maxVelocity / len ));
      excess *= 0.1f;
      mVelocity -= excess;
   }

   // Container buoyancy & drag
   mVelocity.z -= mBuoyancy * (mGravity * mDataBlock->gravityMod * mGravityMod) * dt;
   mVelocity   -= mVelocity * mDrag * dt;
}


void Item::updatePos(const U32 /*mask*/, const F32 dt)
{
   // Try and move
   Point3F pos;
   mObjToWorld.getColumn(3,&pos);
   delta.posVec = pos;

   bool contact = false;
   bool nonStatic = false;
   bool stickyNotify = false;
   CollisionList collisionList;
   F32 time = dt;

   static Polyhedron sBoxPolyhedron;
   static ExtrudedPolyList sExtrudedPolyList;
   static EarlyOutPolyList sEarlyOutPolyList;
   MatrixF collisionMatrix(true);
   Point3F end = pos + mVelocity * time;
   U32 mask = isServerObject() ? sServerCollisionMask : sClientCollisionMask;

   // Part of our speed problem here is that we don't track contact surfaces, like we do
   //  with the player.  In order to handle the most common and performance impacting
   //  instance of this problem, we'll use a ray cast to detect any contact surfaces below
   //  us.  This won't be perfect, but it only needs to catch a few of these to make a
   //  big difference.  We'll cast from the top center of the bounding box at the tick's
   //  beginning to the bottom center of the box at the end.
   Point3F startCast((mObjBox.minExtents.x + mObjBox.maxExtents.x) * 0.5,
                     (mObjBox.minExtents.y + mObjBox.maxExtents.y) * 0.5,
                     mObjBox.maxExtents.z);
   Point3F endCast((mObjBox.minExtents.x + mObjBox.maxExtents.x) * 0.5,
                   (mObjBox.minExtents.y + mObjBox.maxExtents.y) * 0.5,
                   mObjBox.minExtents.z);
   collisionMatrix.setColumn(3, pos);
   collisionMatrix.mulP(startCast);
   collisionMatrix.setColumn(3, end);
   collisionMatrix.mulP(endCast);
   RayInfo rinfo;
   bool doToughCollision = true;
   disableCollision();
   if (mCollisionObject)
      mCollisionObject->disableCollision();
   if (getContainer()->castRay(startCast, endCast, mask, &rinfo))
   {
      F32 bd = -mDot(mVelocity, rinfo.normal);

      if (bd >= 0.0)
      {
         // Contact!
         if (mDataBlock->sticky && rinfo.object->getTypeMask() & (STATIC_COLLISION_TYPEMASK)) {
            mVelocity.set(0, 0, 0);
            mAtRest = true;
            mAtRestCounter = 0;
            stickyNotify = true;
            mStickyCollisionPos    = rinfo.point;
            mStickyCollisionNormal = rinfo.normal;
            doToughCollision = false;;
         } else {
            // Subtract out velocity into surface and friction
            VectorF fv = mVelocity + rinfo.normal * bd;
            F32 fvl = fv.len();
            if (fvl) {
               F32 ff = bd * mDataBlock->friction;
               if (ff < fvl) {
                  fv *= ff / fvl;
                  fvl = ff;
               }
            }
            bd *= 1 + mDataBlock->elasticity;
            VectorF dv = rinfo.normal * (bd + 0.002);
            mVelocity += dv;
            mVelocity -= fv;

            // Keep track of what we hit
            contact = true;
            U32 typeMask = rinfo.object->getTypeMask();
            if (!(typeMask & StaticObjectType))
               nonStatic = true;
            if (isServerObject() && (typeMask & ShapeBaseObjectType)) {
               ShapeBase* col = static_cast<ShapeBase*>(rinfo.object);
               queueCollision(col,mVelocity - col->getVelocity());
            }
         }
      }
   }
   enableCollision();
   if (mCollisionObject)
      mCollisionObject->enableCollision();

   if (doToughCollision)
   {
      U32 count;
      for (count = 0; count < 3; count++)
      {
         // Build list from convex states here...
         end = pos + mVelocity * time;


         collisionMatrix.setColumn(3, end);
         Box3F wBox = getObjBox();
         collisionMatrix.mul(wBox);
         Box3F testBox = wBox;
         Point3F oldMin = testBox.minExtents;
         Point3F oldMax = testBox.maxExtents;
         testBox.minExtents.setMin(oldMin + (mVelocity * time));
         testBox.maxExtents.setMin(oldMax + (mVelocity * time));

         sEarlyOutPolyList.clear();
         sEarlyOutPolyList.mNormal.set(0,0,0);
         sEarlyOutPolyList.mPlaneList.setSize(6);
         sEarlyOutPolyList.mPlaneList[0].set(wBox.minExtents,VectorF(-1,0,0));
         sEarlyOutPolyList.mPlaneList[1].set(wBox.maxExtents,VectorF(0,1,0));
         sEarlyOutPolyList.mPlaneList[2].set(wBox.maxExtents,VectorF(1,0,0));
         sEarlyOutPolyList.mPlaneList[3].set(wBox.minExtents,VectorF(0,-1,0));
         sEarlyOutPolyList.mPlaneList[4].set(wBox.minExtents,VectorF(0,0,-1));
         sEarlyOutPolyList.mPlaneList[5].set(wBox.maxExtents,VectorF(0,0,1));

         CollisionWorkingList& eorList = mConvex.getWorkingList();
         CollisionWorkingList* eopList = eorList.wLink.mNext;
         while (eopList != &eorList) {
            if ((eopList->mConvex->getObject()->getTypeMask() & mask) != 0)
            {
               Box3F convexBox = eopList->mConvex->getBoundingBox();
               if (testBox.isOverlapped(convexBox))
               {
                  eopList->mConvex->getPolyList(&sEarlyOutPolyList);
                  if (sEarlyOutPolyList.isEmpty() == false)
                     break;
               }
            }
            eopList = eopList->wLink.mNext;
         }
         if (sEarlyOutPolyList.isEmpty())
         {
            pos = end;
            break;
         }

         collisionMatrix.setColumn(3, pos);
         sBoxPolyhedron.buildBox(collisionMatrix, mObjBox, true);

         // Build extruded polyList...
         VectorF vector = end - pos;
         sExtrudedPolyList.extrude(sBoxPolyhedron, vector);
         sExtrudedPolyList.setVelocity(mVelocity);
         sExtrudedPolyList.setCollisionList(&collisionList);

         CollisionWorkingList& rList = mConvex.getWorkingList();
         CollisionWorkingList* pList = rList.wLink.mNext;
         while (pList != &rList) {
            if ((pList->mConvex->getObject()->getTypeMask() & mask) != 0)
            {
               Box3F convexBox = pList->mConvex->getBoundingBox();
               if (testBox.isOverlapped(convexBox))
               {
                  pList->mConvex->getPolyList(&sExtrudedPolyList);
               }
            }
            pList = pList->wLink.mNext;
         }

         if (collisionList.getTime() < 1.0)
         {
            // Set to collision point
            F32 dt = time * collisionList.getTime();
            pos += mVelocity * dt;
            time -= dt;

            // Pick the most resistant surface
            F32 bd = 0;
            const Collision* collision = 0;
            for (int c = 0; c < collisionList.getCount(); c++) {
               const Collision &cp = collisionList[c];
               F32 dot = -mDot(mVelocity,cp.normal);
               if (dot > bd) {
                  bd = dot;
                  collision = &cp;
               }
            }

            if (collision && mDataBlock->sticky && collision->object->getTypeMask() & (STATIC_COLLISION_TYPEMASK)) {
               mVelocity.set(0, 0, 0);
               mAtRest = true;
               mAtRestCounter = 0;
               stickyNotify = true;
               mStickyCollisionPos    = collision->point;
               mStickyCollisionNormal = collision->normal;
               break;
            } else {
               // Subtract out velocity into surface and friction
               if (collision) {
                  VectorF fv = mVelocity + collision->normal * bd;
                  F32 fvl = fv.len();
                  if (fvl) {
                     F32 ff = bd * mDataBlock->friction;
                     if (ff < fvl) {
                        fv *= ff / fvl;
                        fvl = ff;
                     }
                  }
                  bd *= 1 + mDataBlock->elasticity;
                  VectorF dv = collision->normal * (bd + 0.002);
                  mVelocity += dv;
                  mVelocity -= fv;

                  // Keep track of what we hit
                  contact = true;
                  U32 typeMask = collision->object->getTypeMask();
                  if (!(typeMask & StaticObjectType))
                     nonStatic = true;
                  if (isServerObject() && (typeMask & ShapeBaseObjectType)) {
                     ShapeBase* col = static_cast<ShapeBase*>(collision->object);
                     queueCollision(col,mVelocity - col->getVelocity());
                  }
               }
            }
         }
         else
         {
            pos = end;
            break;
         }
      }
      if (count == 3)
      {
         // Couldn't move...
         mVelocity.set(0, 0, 0);
      }
   }

   // If on the client, calculate delta for backstepping
   if (isGhost()) {
      delta.pos     = pos;
      delta.posVec -= pos;
      delta.dt = 1;
   }

   // Update transform
   MatrixF mat = mObjToWorld;
   mat.setColumn(3,pos);
   Parent::setTransform(mat);
   enableCollision();
   if (mCollisionObject)
      mCollisionObject->enableCollision();
   updateContainer();

   if ( mPhysicsRep )
      mPhysicsRep->setTransform( mat );

   //
   if (contact) {
      // Check for rest condition
      if (!nonStatic && mVelocity.len() < sAtRestVelocity) {
         mVelocity.x = mVelocity.y = mVelocity.z = 0;
         mAtRest = true;
         mAtRestCounter = 0;
      }

      // Only update the client if we hit a non-static shape or
      // if this is our final rest pos.
      if (nonStatic || mAtRest)
         setMaskBits(PositionMask);
   }

   // Collision callbacks. These need to be processed whether we hit
   // anything or not.
   if (!isGhost())
   {
      SimObjectPtr<Item> safePtr(this);
      if (stickyNotify)
      {
         notifyCollision();
         if(bool(safePtr))
			 onStickyCollision_callback( getIdString() );
      }
      else
         notifyCollision();

      // water
      if(bool(safePtr))
      {
         if(!mInLiquid && mWaterCoverage != 0.0f)
         {
			onEnterLiquid_callback( getIdString(), Con::getFloatArg(mWaterCoverage), mLiquidType.c_str() );
            mInLiquid = true;
         }
         else if(mInLiquid && mWaterCoverage == 0.0f)
         {
			 onLeaveLiquid_callback(getIdString(), mLiquidType.c_str());
            mInLiquid = false;
         }
      }
   }
}


//----------------------------------------------------------------------------

static MatrixF IMat(1);

bool Item::buildPolyList(PolyListContext context, AbstractPolyList* polyList, const Box3F&, const SphereF&)
{
   if ( context == PLC_Decal )
      return false;

   // Collision with the item is always against the item's object
   // space bounding box axis aligned in world space.
   Point3F pos;
   mObjToWorld.getColumn(3,&pos);
   IMat.setColumn(3,pos);
   polyList->setTransform(&IMat, mObjScale);
   polyList->setObject(this);
   polyList->addBox(mObjBox);
   return true;
}


//----------------------------------------------------------------------------

U32 Item::packUpdate(NetConnection *connection, U32 mask, BitStream *stream)
{
   U32 retMask = Parent::packUpdate(connection,mask,stream);

   if (stream->writeFlag(mask & InitialUpdateMask)) {
      stream->writeFlag(mRotate);
      stream->writeFlag(mStatic);
      if (stream->writeFlag(getScale() != Point3F(1, 1, 1)))
         mathWrite(*stream, getScale());
   }

   if (mask & ThrowSrcMask && mCollisionObject) {
      S32 gIndex = connection->getGhostIndex(mCollisionObject);
      if (stream->writeFlag(gIndex != -1))
         stream->writeInt(gIndex,NetConnection::GhostIdBitSize);
   }
   else
      stream->writeFlag(false);

   if (stream->writeFlag(mask & RotationMask && !mRotate)) {
      // Assumes rotation is about the Z axis
      AngAxisF aa(mObjToWorld);
      stream->writeFlag(aa.axis.z < 0);
      stream->write(aa.angle);
   }

   if (stream->writeFlag(mask & PositionMask)) {
      Point3F pos;
      mObjToWorld.getColumn(3,&pos);
      mathWrite(*stream, pos);
      if (!stream->writeFlag(mAtRest)) {
         mathWrite(*stream, mVelocity);
      }
      stream->writeFlag(!(mask & NoWarpMask));
   }
   return retMask;
}

void Item::unpackUpdate(NetConnection *connection, BitStream *stream)
{
   Parent::unpackUpdate(connection,stream);

   // InitialUpdateMask
   if (stream->readFlag()) {
      mRotate = stream->readFlag();
      mStatic = stream->readFlag();
      if (stream->readFlag())
         mathRead(*stream, &mObjScale);
      else
         mObjScale.set(1, 1, 1);
   }

   // ThrowSrcMask && mCollisionObject
   if (stream->readFlag()) {
      S32 gIndex = stream->readInt(NetConnection::GhostIdBitSize);
      setCollisionTimeout(static_cast<ShapeBase*>(connection->resolveGhost(gIndex)));
   }

   MatrixF mat = mObjToWorld;

   // RotationMask && !mRotate
   if (stream->readFlag()) {
      // Assumes rotation is about the Z axis
      AngAxisF aa;
      aa.axis.set(0.0f, 0.0f, stream->readFlag() ? -1.0f : 1.0f);
      stream->read(&aa.angle);
      aa.setMatrix(&mat);
      Point3F pos;
      mObjToWorld.getColumn(3,&pos);
      mat.setColumn(3,pos);
   }

   // PositionMask
   if (stream->readFlag()) {
      Point3F pos;
      mathRead(*stream, &pos);
      F32 speed = mVelocity.len();
      if ((mAtRest = stream->readFlag()) == true)
         mVelocity.set(0.0f, 0.0f, 0.0f);
      else
         mathRead(*stream, &mVelocity);

      if (stream->readFlag() && isProperlyAdded()) {
         // Determin number of ticks to warp based on the average
         // of the client and server velocities.
         delta.warpOffset = pos - delta.pos;
         F32 as = (speed + mVelocity.len()) * 0.5f * TickSec;
         F32 dt = (as > 0.00001f) ? delta.warpOffset.len() / as: sMaxWarpTicks;
         delta.warpTicks = (S32)((dt > sMinWarpTicks)? getMax(mFloor(dt + 0.5f), 1.0f): 0.0f);

         if (delta.warpTicks)
         {
            // Setup the warp to start on the next tick, only the
            // object's position is warped.
            if (delta.warpTicks > sMaxWarpTicks)
               delta.warpTicks = sMaxWarpTicks;
            delta.warpOffset /= (F32)delta.warpTicks;
         }
         else {
            // Going to skip the warp, server and client are real close.
            // Adjust the frame interpolation to move smoothly to the
            // new position within the current tick.
            Point3F cp = delta.pos + delta.posVec * delta.dt;
            VectorF vec = delta.pos - cp;
            F32 vl = vec.len();
            if (vl) {
               F32 s = delta.posVec.len() / vl;
               delta.posVec = (cp - pos) * s;
            }
            delta.pos = pos;
            mat.setColumn(3,pos);
         }
      }
      else {
         // Set the item to the server position
         delta.warpTicks = 0;
         delta.posVec.set(0,0,0);
         delta.pos = pos;
         delta.dt = 0;
         mat.setColumn(3,pos);
      }
   }
   Parent::setTransform(mat);
}

DefineEngineMethod( Item, isStatic, bool, (),, 
   "@brief Is the object static (ie, non-movable)?\n\n"   
   "@return True if the object is static, false if it is not.\n"
   "@tsexample\n"
	   "// Query the item on if it is or is not static.\n"
	   "%isStatic = %itemData.isStatic();\n\n"
   "@endtsexample\n\n"
   "@see static\n"
   )
{
   return object->isStatic();
}

DefineEngineMethod( Item, isAtRest, bool, (),, 
   "@brief Is the object at rest (ie, no longer moving)?\n\n"   
   "@return True if the object is at rest, false if it is not.\n"
   "@tsexample\n"
	   "// Query the item on if it is or is not at rest.\n"
	   "%isAtRest = %item.isAtRest();\n\n"
   "@endtsexample\n\n"
   )
{
   return object->isAtRest();
}

DefineEngineMethod( Item, isRotating, bool, (),, 
   "@brief Is the object still rotating?\n\n"   
   "@return True if the object is still rotating, false if it is not.\n"
   "@tsexample\n"
	   "// Query the item on if it is or is not rotating.\n"
	   "%isRotating = %itemData.isRotating();\n\n"
   "@endtsexample\n\n"
   "@see rotate\n"
   )
{
   return object->isRotating();
}

DefineEngineMethod( Item, setCollisionTimeout, bool, (int ignoreColObj),(NULL), 
   "@brief Temporarily disable collisions against a specific ShapeBase object.\n\n"

   "This is useful to prevent a player from immediately picking up an Item they have "
   "just thrown.  Only one object may be on the timeout list at a time.  The timeout is "
   "defined as 15 ticks.\n\n"

   "@param objectID ShapeBase object ID to disable collisions against.\n"
   "@return Returns true if the ShapeBase object requested could be found, false if it could not.\n"

   "@tsexample\n"
	   "// Set the ShapeBase Object ID to disable collisions against\n"
	   "%ignoreColObj = %player.getID();\n\n"
	   "// Inform this Item object to ignore collisions temproarily against the %ignoreColObj.\n"
	   "%item.setCollisionTimeout(%ignoreColObj);\n\n"
   "@endtsexample\n\n"
   )
{
   ShapeBase* source = NULL;
   if (Sim::findObject(ignoreColObj,source)) {
      object->setCollisionTimeout(source);
      return true;
   }
   return false;
}


DefineEngineMethod( Item, getLastStickyPos, const char*, (),, 
   "@brief Get the position on the surface on which this Item is stuck.\n\n"   
   "@return Returns The XYZ position of where this Item is stuck.\n"
   "@tsexample\n"
	   "// Acquire the position where this Item is currently stuck\n"
	   "%stuckPosition = %item.getLastStickPos();\n\n"
   "@endtsexample\n\n"
   "@note Server side only.\n"
   )
{
   char* ret = Con::getReturnBuffer(256);
   if (object->isServerObject())
      dSprintf(ret, 255, "%g %g %g",
               object->mStickyCollisionPos.x,
               object->mStickyCollisionPos.y,
               object->mStickyCollisionPos.z);
   else
      dStrcpy(ret, "0 0 0");

   return ret;
}

DefineEngineMethod( Item, getLastStickyNormal, const char *, (),, 
   "@brief Get the normal of the surface on which the object is stuck.\n\n"   
   "@return Returns The XYZ normal from where this Item is stuck.\n"
   "@tsexample\n"
	   "// Acquire the position where this Item is currently stuck\n"
	   "%stuckPosition = %item.getLastStickPos();\n\n"
   "@endtsexample\n\n"
   "@note Server side only.\n"
   )
{
   char* ret = Con::getReturnBuffer(256);
   if (object->isServerObject())
      dSprintf(ret, 255, "%g %g %g",
               object->mStickyCollisionNormal.x,
               object->mStickyCollisionNormal.y,
               object->mStickyCollisionNormal.z);
   else
      dStrcpy(ret, "0 0 0");

   return ret;
}

//----------------------------------------------------------------------------

bool Item::_setStatic(void *object, const char *index, const char *data)
{
   Item *i = static_cast<Item*>(object);
   i->mAtRest = dAtob(data);
   i->setMaskBits(InitialUpdateMask | PositionMask);
   return true;
}

bool Item::_setRotate(void *object, const char *index, const char *data)
{
   Item *i = static_cast<Item*>(object);
   i->setMaskBits(InitialUpdateMask | RotationMask);
   return true;
}

void Item::initPersistFields()
{
   addGroup("Misc");	
   addProtectedField("static", TypeBool, Offset(mStatic, Item), &_setStatic, &defaultProtectedGetFn, "If true, the object is not moving in the world.\n");
   addProtectedField("rotate", TypeBool, Offset(mRotate, Item), &_setRotate, &defaultProtectedGetFn, "If true, the object will automatically rotate around its Z axis.\n");
   endGroup("Misc");

   Parent::initPersistFields();
}

void Item::consoleInit()
{
   Con::addVariable("Item::minWarpTicks",TypeF32,&sMinWarpTicks,
      "@brief Fraction of tick at which instant warp occures on the client.\n\n"
	   "@ingroup GameObjects");
   Con::addVariable("Item::maxWarpTicks",TypeS32,&sMaxWarpTicks, 
      "@brief When a warp needs to occur due to the client being too far off from the server, this is the "
      "maximum number of ticks we'll allow the client to warp to catch up.\n\n"
	   "@ingroup GameObjects");
}

//----------------------------------------------------------------------------

void Item::prepRenderImage( SceneRenderState* state )
{
   // Items do NOT render if destroyed
   if (getDamageState() == Destroyed)
      return;

   Parent::prepRenderImage( state );
}

void Item::buildConvex(const Box3F& box, Convex* convex)
{
   if (mShapeInstance == NULL)
      return;

   // These should really come out of a pool
   mConvexList->collectGarbage();

   if (box.isOverlapped(getWorldBox()) == false)
      return;

   // Just return a box convex for the entire shape...
   Convex* cc = 0;
   CollisionWorkingList& wl = convex->getWorkingList();
   for (CollisionWorkingList* itr = wl.wLink.mNext; itr != &wl; itr = itr->wLink.mNext) {
      if (itr->mConvex->getType() == BoxConvexType &&
          itr->mConvex->getObject() == this) {
         cc = itr->mConvex;
         break;
      }
   }
   if (cc)
      return;

   // Create a new convex.
   BoxConvex* cp = new BoxConvex;
   mConvexList->registerObject(cp);
   convex->addToWorkingList(cp);
   cp->init(this);

   mObjBox.getCenter(&cp->mCenter);
   cp->mSize.x = mObjBox.len_x() / 2.0f;
   cp->mSize.y = mObjBox.len_y() / 2.0f;
   cp->mSize.z = mObjBox.len_z() / 2.0f;
}

void Item::advanceTime(F32 dt)
{
   Parent::advanceTime(dt);

   if( mRotate )
   {
      F32 r = (dt / sRotationSpeed) * M_2PI;
      Point3F pos = mRenderObjToWorld.getPosition();
      MatrixF rotMatrix;
      if( mRotate )
      {
         rotMatrix.set( EulerF( 0.0, 0.0, r ) );
      }
      else
      {
         rotMatrix.set( EulerF( r * 0.5, 0.0, r ) );
      }
      MatrixF mat = mRenderObjToWorld;
      mat.setPosition( pos );
      mat.mul( rotMatrix );
      setRenderTransform(mat);
   }

}
