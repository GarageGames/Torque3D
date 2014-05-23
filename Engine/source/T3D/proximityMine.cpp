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

#include "core/stream/bitStream.h"
#include "console/console.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "sim/netConnection.h"
#include "math/mMath.h"
#include "math/mathIO.h"
#include "math/mathUtils.h"
#include "gfx/gfxTransformSaver.h"
#include "gfx/gfxDrawUtil.h"
#include "sfx/sfxTrack.h"
#include "sfx/sfxTypes.h"
#include "sfx/sfxSystem.h"
#include "ts/tsShapeInstance.h"
#include "T3D/fx/explosion.h"
#include "T3D/proximityMine.h"
#include "T3D/physics/physicsBody.h"

const U32 sTriggerCollisionMask = ( VehicleObjectType | PlayerObjectType );

static S32 gAutoDeleteTicks = 16;   // Provides about half a second for all clients to be updated
                                    // before a mine is deleted on the server.

//----------------------------------------------------------------------------

IMPLEMENT_CO_DATABLOCK_V1( ProximityMineData );

ConsoleDocClass( ProximityMineData,
   "@brief Stores common properties for a ProximityMine.\n\n"
   "@see ProximityMine\n"
   "@ingroup gameObjects\n"
);

IMPLEMENT_CALLBACK( ProximityMineData, onTriggered, void, ( ProximityMine* obj, SceneObject *target ),( obj, target ),
   "Callback invoked when an object triggers the ProximityMine.\n\n"
   "@param obj The ProximityMine object\n"
   "@param target The object that triggered the mine\n"
   "@note This callback is only invoked on the server.\n"
   "@see ProximityMine\n"
);

IMPLEMENT_CALLBACK( ProximityMineData, onExplode, void, ( ProximityMine* obj, Point3F pos ),( obj, pos ),
   "Callback invoked when a ProximityMine is about to explode.\n\n"
   "@param obj The ProximityMine object\n"
   "@param pos The position of the mine explosion\n"
   "@note This callback is only invoked on the server.\n"
   "@see ProximityMine\n"
);

ProximityMineData::ProximityMineData()
 : armingDelay( 0 ),
   armingSequence( -1 ),
   armingSound( NULL ),
   triggerRadius( 5.0f ),
   triggerSpeed( 1.0f ),
   autoTriggerDelay( 0 ),
   triggerOnOwner( false ),
   triggerDelay( 0 ),
   triggerSequence( -1 ),
   triggerSound( NULL ),
   explosionOffset( 0.05f )
{
}

void ProximityMineData::initPersistFields()
{
   addGroup( "Arming" );
   addField( "armingDelay", TypeF32, Offset(armingDelay, ProximityMineData), 
      "Delay (in seconds) from when the mine is placed to when it becomes active." );
   addField( "armingSound", TypeSFXTrackName, Offset(armingSound, ProximityMineData),
      "Sound to play when the mine is armed (starts at the same time as "
      "the <i>armed</i> sequence if defined)." );
   endGroup( "Arming" );

   addGroup( "Triggering" );
   addField( "autoTriggerDelay", TypeF32, Offset(autoTriggerDelay, ProximityMineData),
      "@brief Delay (in seconds) from arming until the mine automatically "
      "triggers and explodes, even if no object has entered the trigger area.\n\n"
      "Set to 0 to disable." );
   addField( "triggerOnOwner", TypeBool, Offset(triggerOnOwner, ProximityMineData),
      "@brief Controls whether the mine can be triggered by the object that owns it.\n\n"
      "For example, a player could deploy mines that are only dangerous to other "
      "players and not himself." );
   addField( "triggerRadius", TypeF32, Offset(triggerRadius, ProximityMineData),
      "Distance at which an activated mine will detect other objects and explode." );
   addField( "triggerSpeed", TypeF32, Offset(triggerSpeed, ProximityMineData),
      "Speed above which moving objects within the trigger radius will trigger the mine" );
   addField( "triggerDelay", TypeF32, Offset(triggerDelay, ProximityMineData),
      "Delay (in seconds) from when the mine is triggered until it explodes." );
   addField( "triggerSound", TypeSFXTrackName, Offset(triggerSound, ProximityMineData),
      "Sound to play when the mine is triggered (starts at the same time as "
      "the <i>triggered</i> sequence if defined)." );
   endGroup( "Triggering" );

   addGroup( "Explosion" );
   addField( "explosionOffset", TypeF32, Offset(explosionOffset, ProximityMineData),
      "@brief Offset from the mine's origin where the explosion emanates from."
      "Sometimes a thrown mine may be slightly sunk into the ground.  This can be just "
      "enough to cause the explosion to occur under the ground, especially on flat "
      "ground, which can end up blocking the explosion.  This offset along the mine's "
      "'up' normal allows you to raise the explosion origin to a better height.");
   endGroup( "Explosion" );

   Parent::initPersistFields();
}

bool ProximityMineData::preload( bool server, String& errorStr )
{
   if ( Parent::preload( server, errorStr ) == false )
      return false;

   if ( !server )
   {
      // Resolve sounds
      String sfxErrorStr;
      if( !sfxResolve( &armingSound, sfxErrorStr ) )
         Con::errorf( ConsoleLogEntry::General, "ProximityMineData::preload: Invalid packet: %s", sfxErrorStr.c_str() );
      if( !sfxResolve( &triggerSound, sfxErrorStr ) )
         Con::errorf( ConsoleLogEntry::General, "ProximityMineData::preload: Invalid packet: %s", sfxErrorStr.c_str() );
   }

   if ( mShape )
   {
      // Lookup animation sequences
      armingSequence = mShape->findSequence( "armed" );
      triggerSequence = mShape->findSequence( "triggered" );
   }

   return true;
}

void ProximityMineData::packData( BitStream* stream )
{
   Parent::packData( stream );

   stream->write( armingDelay );
   sfxWrite( stream, armingSound );

   stream->write( autoTriggerDelay );
   stream->writeFlag( triggerOnOwner );
   stream->write( triggerRadius );
   stream->write( triggerSpeed );
   stream->write( triggerDelay );
   sfxWrite( stream, triggerSound );
}

void ProximityMineData::unpackData( BitStream* stream )
{
   Parent::unpackData(stream);

   stream->read( &armingDelay );
   sfxRead( stream, &armingSound );

   stream->read( &autoTriggerDelay );
   triggerOnOwner = stream->readFlag();
   stream->read( &triggerRadius );
   stream->read( &triggerSpeed );
   stream->read( &triggerDelay );
   sfxRead( stream, &triggerSound );
}

//----------------------------------------------------------------------------

IMPLEMENT_CO_NETOBJECT_V1( ProximityMine );

ConsoleDocClass( ProximityMine,
   "@brief A simple proximity mine.\n\n"

   "Proximity mines can be deployed using the world editor or thrown by an "
   "in-game object. Once armed, any Player or Vehicle object that moves within "
   "the mine's trigger area will cause it to explode.\n\n"

   "Internally, the ProximityMine object transitions through the following states:\n"
   "<ol>\n"
   "  <li><b>Thrown</b>: Mine has been thrown, but has not yet attached to a surface</li>\n"
   "  <li><b>Deployed</b>: Mine has attached to a surface but is not yet armed. Start "
      "playing the #armingSound and <i>armed</i> sequence.</li>\n"
   "  <li><b>Armed</b>: Mine is armed and will trigger if a Vehicle or Player object moves "
      "within the trigger area.</li>\n"
   "  <li><b>Triggered</b>: Mine has been triggered and will explode soon. Invoke the "
      "onTriggered callback, and start playing the #triggerSound and <i>triggered</i> "
      "sequence.</li>\n"
   "  <li><b>Exploded</b>: Mine has exploded and will be deleted on the server shortly. "
      "Invoke the onExplode callback on the server and generate the explosion effects "
      "on the client.</li>\n"
   "</ol>\n\n"

   "@note Proximity mines with the #static field set to true will start in the "
   "<b>Armed</b> state. Use this for mines placed with the World Editor.\n\n"

   "The shape used for the mine may optionally define the following sequences:\n"
   "<dl>\n"
   "  <dt>armed</dt><dd>Sequence to play when the mine is deployed, but before "
   "it becomes active and triggerable (#armingDelay should be set appropriately).</dd>\n"
   "  <dt>triggered</dt><dd>Sequence to play when the mine is triggered, just "
   "before it explodes (#triggerDelay should be set appropriately).<dd>\n"
   "</dl>\n\n"

   "@tsexample\n"
   "datablock ProximityMineData( SimpleMine )\n"
   "{\n"
   "   // ShapeBaseData fields\n"
   "   category = \"Weapon\";\n"
   "   shapeFile = \"art/shapes/weapons/misc/proximityMine.dts\";\n\n"

   "   // ItemData fields\n"
   "   sticky = true;\n\n"

   "   // ProximityMineData fields\n"
   "   armingDelay = 0.5;\n"
   "   armingSound = MineArmedSound;\n\n"

   "   autoTriggerDelay = 0;\n"
   "   triggerOnOwner = true;\n"
   "   triggerRadius = 5.0;\n"
   "   triggerSpeed = 1.0;\n"
   "   triggerDelay = 0.5;\n"
   "   triggerSound = MineTriggeredSound;\n"
   "   explosion = RocketLauncherExplosion;\n\n"

   "   // dynamic fields\n"
   "   pickUpName = \"Proximity Mines\";\n"
   "   maxInventory = 20;\n\n"

   "   damageType = \"MineDamage\"; // type of damage applied to objects in radius\n"
   "   radiusDamage = 30;           // amount of damage to apply to objects in radius\n"
   "   damageRadius = 8;            // search radius to damage objects when exploding\n"
   "   areaImpulse = 2000;          // magnitude of impulse to apply to objects in radius\n"
   "};\n\n"

   "function ProximityMineData::onTriggered( %this, %obj, %target )\n"
   "{\n"
   "   echo( %this.name SPC \"triggered by \" @ %target.getClassName() );\n"
   "}\n\n"

   "function ProximityMineData::onExplode( %this, %obj, %position )\n"
   "{\n"
   "   // Damage objects within the mine's damage radius\n"
   "   if ( %this.damageRadius > 0 )\n"
   "      radiusDamage( %obj.sourceObject, %position, %this.damageRadius, %this.radiusDamage, %this.damageType, %this.areaImpulse );\n"
   "}\n\n"

   "function ProximityMineData::damage( %this, %obj, %position, %source, %amount, %damageType )\n"
   "{\n"
   "   // Explode if any damage is applied to the mine\n"
   "   %obj.schedule(50 + getRandom(50), explode);\n"
   "}\n\n"

   "%obj = new ProximityMine()\n"
   "{\n"
   "   dataBlock = SimpleMine;\n"
   "};\n"
   "@endtsexample\n\n"

   "@see ProximityMineData\n"

   "@ingroup gameObjects\n"
);

ProximityMine::ProximityMine()
{
   mTypeMask |= StaticShapeObjectType;
   mDataBlock = 0;

   mStickyCollisionPos.zero();

   mOwner = NULL;

   mState = Thrown;
   mStateTimeout = 0;

   mAnimThread = NULL;

   // For the Item class
   mSubclassItemHandlesScene = true;
}

ProximityMine::~ProximityMine()
{
}

//----------------------------------------------------------------------------

void ProximityMine::consoleInit()
{
   Parent::consoleInit();

   Con::addVariable("$ProxMine::autoDeleteTicks", TypeS32, &gAutoDeleteTicks,
      "@brief Number of ticks until an exploded mine is deleted on the server.\n\n"

      "After a mine has exploded it remains in the server's scene graph for a time "
      "to allow its exploded state to be passed along to each client.  This variable "
      "controls how long a mine remains before it is deleted.  Any client that has not "
      "received the exploded state by then (perhaps due to lag) will not see any "
      "explosion produced by the mine.\n\n"

      "@ingroup GameObjects");
}

//----------------------------------------------------------------------------

bool ProximityMine::onAdd()
{
   if ( !Parent::onAdd() || !mDataBlock )
      return false;

   addToScene();

   if (isServerObject())
      scriptOnAdd();

   if ( mStatic )
   {
      // static mines are armed immediately
      mState = Deployed;
      mStateTimeout = 0;
   }

   return true;
}

bool ProximityMine::onNewDataBlock( GameBaseData* dptr, bool reload )
{
   mDataBlock = dynamic_cast<ProximityMineData*>( dptr );
   if ( !mDataBlock || !Parent::onNewDataBlock( dptr, reload ) )
      return false;

   scriptOnNewDataBlock();
   return true;
}

void ProximityMine::onRemove()
{
   scriptOnRemove();
   removeFromScene();

   Parent::onRemove();
}

//----------------------------------------------------------------------------

void ProximityMine::setTransform( const MatrixF& mat )
{
   ShapeBase::setTransform( mat );   // Skip Item::setTransform as it restricts rotation to the Z axis

   if ( !mStatic )
   {
      mAtRest = false;
      mAtRestCounter = 0;
   }

   if ( mPhysicsRep )
      mPhysicsRep->setTransform( getTransform() );

   setMaskBits( Item::RotationMask | Item::PositionMask | Item::NoWarpMask );
}

void ProximityMine::setDeployedPos( const Point3F& pos, const Point3F& normal )
{
   // Align to deployed surface normal
   MatrixF mat( true );
   MathUtils::getMatrixFromUpVector( normal, &mat );
   mat.setPosition( pos + normal * mObjBox.minExtents.z );

   delta.pos = pos;
   delta.posVec.set(0, 0, 0);

   ShapeBase::setTransform( mat );
   if ( mPhysicsRep )
      mPhysicsRep->setTransform( getTransform() );

   setMaskBits( DeployedMask );
}

void ProximityMine::processTick( const Move* move )
{
   Parent::processTick( move );

   // Process state machine
   mStateTimeout -= TickSec;

   State lastState = NumStates;;
   while ( mState != lastState )
   {
      lastState = mState;
      switch ( mState )
      {
         case Thrown:
            if ( mAtRest )
            {
               mState = Deployed;
               mStateTimeout = mDataBlock->armingDelay;

               // Get deployed position if mine was not stuck to another surface
               if ( mStickyCollisionPos.isZero() )
               {
                  mObjToWorld.getColumn( 2, &mStickyCollisionNormal );
                  mObjToWorld.getColumn( 3, &mStickyCollisionPos );
               }
               setDeployedPos( mStickyCollisionPos, mStickyCollisionNormal );

               if ( mDataBlock->armingSequence != -1 )
               {
                  mAnimThread = mShapeInstance->addThread();
                  mShapeInstance->setSequence( mAnimThread, mDataBlock->armingSequence, 0.0f );
               }
               if ( mDataBlock->armingSound )
                  SFX->playOnce( mDataBlock->armingSound, &getRenderTransform() );
            }
            break;

         case Deployed:
            // Timeout into Armed state
            if ( mStateTimeout <= 0 )
            {
               mState = Armed;
               mStateTimeout = mDataBlock->autoTriggerDelay ? mDataBlock->autoTriggerDelay : F32_MAX;
            }
            break;

         case Armed:
         {
            // Check for objects within the trigger area
            Box3F triggerBox( mDataBlock->triggerRadius * 2 );
            triggerBox.setCenter( getTransform().getPosition() );

            SimpleQueryList sql;
            getContainer()->findObjects( triggerBox, sTriggerCollisionMask,
               SimpleQueryList::insertionCallback, &sql );
            for ( S32 i = 0; i < sql.mList.size(); i++ )
            {
               // Detect movement in the trigger area
               if ( ( sql.mList[i] == mOwner && !mDataBlock->triggerOnOwner ) ||
                    sql.mList[i]->getVelocity().len() < mDataBlock->triggerSpeed )
                  continue;

               // Mine has been triggered
               mShapeInstance->destroyThread( mAnimThread );
               mAnimThread = NULL;

               mState = Triggered;
               mStateTimeout = mDataBlock->triggerDelay;
               if ( mDataBlock->triggerSequence != -1 )
               {
                  mAnimThread = mShapeInstance->addThread();
                  mShapeInstance->setSequence( mAnimThread, mDataBlock->triggerSequence, 0.0f );
               }
               if ( mDataBlock->triggerSound )
                  SFX->playOnce( mDataBlock->triggerSound, &getRenderTransform() );

               if ( isServerObject() )
                  mDataBlock->onTriggered_callback( this, sql.mList[0] );
            }
            break;
         }

         case Triggered:
            // Timeout into exploded state
            if ( mStateTimeout <= 0 )
            {
               explode();
            }
            break;

         case Exploded:
            // Mine's delete themselves on the server after exploding
            if ( isServerObject() && ( mStateTimeout <= 0 ) )
            {
               deleteObject();
               return;
            }
            break;
      }
   }
}

//----------------------------------------------------------------------------

void ProximityMine::explode()
{
   // Make sure we don't explode twice
   if ( mState == Exploded )
   {
      return;
   }

   mState = Exploded;
   mStateTimeout = TickSec * gAutoDeleteTicks;  // auto-delete on server N ticks after exploding

   // Move the explosion point slightly off the surface to avoid problems with radius damage
   Point3F normal = getTransform().getUpVector();
   Point3F explodePos = getTransform().getPosition() + normal * mDataBlock->explosionOffset;

   if ( isServerObject() )
   {
      // Do what the server needs to do, damage the surrounding objects, etc.
      mDataBlock->onExplode_callback( this, explodePos );
      setMaskBits( ExplosionMask );

      // Wait till the timeout to self delete. This gives the server object time
      // to get ghosted to the client
   } 
   else 
   {
      // Client just plays the explosion effect at the right place
      if ( mDataBlock->explosion )
      {
         Explosion *pExplosion = new Explosion;
         pExplosion->onNewDataBlock( mDataBlock->explosion, false );

         MatrixF xform( true );
         xform.setPosition( explodePos );
         pExplosion->setTransform( xform );
         pExplosion->setInitialState( explodePos, normal );
         pExplosion->setCollideType( sTriggerCollisionMask );
         if ( pExplosion->registerObject() == false )
         {
            Con::errorf( ConsoleLogEntry::General, "ProximityMine(%s)::explode: couldn't register explosion",
                         mDataBlock->getName() );
            delete pExplosion;
         }
      }
   }
}

void ProximityMine::advanceTime( F32 dt )
{
   Parent::advanceTime( dt );

   if ( mAnimThread )
      mShapeInstance->advancePos( dt, mAnimThread );
}

//----------------------------------------------------------------------------

U32 ProximityMine::packUpdate( NetConnection* connection, U32 mask, BitStream* stream )
{
   // Handle rotation ourselves (so it is not locked to the Z axis like for Items)
   U32 retMask = Parent::packUpdate( connection, mask & (~Item::RotationMask), stream );

   if ( stream->writeFlag( mask & Item::RotationMask ) )
   {
      QuatF rot( mObjToWorld );
      mathWrite( *stream, rot );
   }

   if ( stream->writeFlag( !mStatic && ( mask & DeployedMask ) && ( mState > Thrown ) ) )
   {
      mathWrite( *stream, mStickyCollisionPos );
      mathWrite( *stream, mStickyCollisionNormal );
   }

   stream->writeFlag( ( mask & ExplosionMask ) && ( mState == Exploded ) );

   return retMask;
}

void ProximityMine::unpackUpdate( NetConnection* connection, BitStream* stream )
{
   Parent::unpackUpdate( connection, stream );

   // Item::RotationMask
   if ( stream->readFlag() )
   {
      QuatF rot;
      mathRead( *stream, &rot );

      Point3F pos = mObjToWorld.getPosition();
      rot.setMatrix( &mObjToWorld );
      mObjToWorld.setPosition( pos );
   }

   // !mStatic && ( mask & DeployedMask ) && ( mState > Thrown )
   if ( stream->readFlag() )
   {
      mathRead( *stream, &mStickyCollisionPos );
      mathRead( *stream, &mStickyCollisionNormal );

      mAtRest = true;

      setDeployedPos( mStickyCollisionPos, mStickyCollisionNormal );
   }

   // ( mask & ExplosionMask ) && ( mState == Exploded )
   if ( stream->readFlag() )
   {
      // start the explosion visuals on the client
      explode();
   }

   if ( mStatic && mState <= Deployed )
   {
      // static mines are armed immediately
      mState = Deployed;
      mStateTimeout = 0;
   }
}

//----------------------------------------------------------------------------

void ProximityMine::prepRenderImage( SceneRenderState* state )
{
   // Don't render the mine if exploded
   if ( mState == Exploded )
      return;

   // Use ShapeBase to render the 3D shape
   Parent::prepRenderImage( state );

   // Add a custom render instance to draw the trigger area
   if ( !state->isShadowPass() )
   {
      ObjectRenderInst* ri = state->getRenderPass()->allocInst<ObjectRenderInst>();
      ri->renderDelegate.bind( this, &ProximityMine::renderObject );
      ri->type = RenderPassManager::RIT_ObjectTranslucent;
      ri->translucentSort = true;
      ri->defaultKey = 1;
      state->getRenderPass()->addInst( ri );
   }
}

void ProximityMine::renderObject( ObjectRenderInst* ri,
                                  SceneRenderState* state,
                                  BaseMatInstance* overrideMat )
{
   if ( overrideMat )
      return;
/*
   // Render the trigger area
   if ( mState == Armed || mState == Triggered )
   {
      const ColorF drawColor(1, 0, 0, 0.05f);
      if ( drawColor.alpha > 0 )
      {
         GFXStateBlockDesc desc;
         desc.setZReadWrite( true, false );
         desc.setBlend( true );

         GFXTransformSaver saver;

         MatrixF mat = getRenderTransform();
         mat.scale( getScale() );

         GFX->getDrawUtil()->drawSphere(  desc, mDataBlock->triggerRadius, mat.getPosition(),
                                          drawColor, true, false, &mat );
      }
   }
*/
}

//----------------------------------------------------------------------------

DefineEngineMethod( ProximityMine, explode, void, (),,
   "@brief Manually cause the mine to explode.\n\n")
{
   object->explode();
}
