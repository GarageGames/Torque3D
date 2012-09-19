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
#include "T3D/fx/lightning.h"

#include "scene/sceneRenderState.h"
#include "console/consoleTypes.h"
#include "math/mathIO.h"
#include "core/stream/bitStream.h"
#include "T3D/gameBase/gameConnection.h"
#include "T3D/shapeBase.h"
#include "math/mRandom.h"
#include "math/mathUtils.h"
#include "terrain/terrData.h"
#include "scene/sceneManager.h"
#include "T3D/player.h"
#include "T3D/camera.h"
#include "sfx/sfxSystem.h"
#include "sfx/sfxTrack.h"
#include "sfx/sfxTypes.h"
#include "gfx/primBuilder.h"
#include "console/engineAPI.h"


IMPLEMENT_CO_DATABLOCK_V1(LightningData);
IMPLEMENT_CO_NETOBJECT_V1(Lightning);

ConsoleDocClass( LightningData,
   "@brief Common data for a Lightning emitter object.\n"
   "@see Lightning\n"
   "@ingroup FX\n"
   "@ingroup Atmosphere\n"
   "@ingroup Datablocks\n"
);

ConsoleDocClass( Lightning,
   "@brief An emitter for lightning bolts.\n\n"

   "Lightning strike events are created on the server and transmitted to all "
   "clients to render the bolt. The strike may be followed by a random thunder "
   "sound. Player or Vehicle objects within the Lightning strike range can be "
   "hit and damaged by bolts.\n"

   "@see LightningData\n"
   "@ingroup FX\n"
   "@ingroup Atmosphere\n"
);

IMPLEMENT_CALLBACK( Lightning, applyDamage, void, ( const Point3F& hitPosition, const Point3F& hitNormal, SceneObject* hitObject ),
   ( hitPosition, hitNormal, hitObject ),
   "Informs an object that it was hit by a lightning bolt and needs to take damage.\n"
   "@param hitPosition World position hit by the lightning bolt.\n"
   "@param hitNormal Surface normal at @a hitPosition.\n"
   "@param hitObject Player or Vehicle object that was hit.\n"
   "@tsexample\n"
   "function Lightning::applyDamage( %this, %hitPosition, %hitNormal, %hitObject )\n"
   "{\n"
   "   // apply damage to the player\n"
   "   %hitObject.applyDamage( 25 );\n"
   "}\n"
   "@endtsexample\n"
);


MRandomLCG sgLightningRand;


S32 QSORT_CALLBACK cmpSounds(const void* p1, const void* p2)
{
   U32 i1 = *((const S32*)p1);
   U32 i2 = *((const S32*)p2);

   if (i1 < i2) {
      return 1;
   } else if (i1 > i2) {
      return -1;
   } else {
      return 0;
   }
}

//--------------------------------------------------------------------------
//--------------------------------------
//
class LightningStrikeEvent : public NetEvent
{
  public:
   typedef NetEvent Parent;
   enum EventType {
      WarningFlash   = 0,
      Strike         = 1,
      TargetedStrike = 2,

      TypeMin        = WarningFlash,
      TypeMax        = TargetedStrike
   };
   enum Constants {
      PositionalBits = 10
   };

   Point2F                   mStart;
   SimObjectPtr<SceneObject> mTarget;

   Lightning*                mLightning;

   // Set by unpack...
  public:
   S32                       mClientId;

  public:
   LightningStrikeEvent();
   ~LightningStrikeEvent();

   void pack(NetConnection*, BitStream*);
   void write(NetConnection*, BitStream*){}
   void unpack(NetConnection*, BitStream*);
   void process(NetConnection*);

   DECLARE_CONOBJECT(LightningStrikeEvent);
};

IMPLEMENT_CO_CLIENTEVENT_V1(LightningStrikeEvent);

ConsoleDocClass( LightningStrikeEvent,
   "@brief Network event that triggers a lightning strike on the client when it "
   "is received.\n\n"
   "This event is sent to all clients when the warningFlashes(), "
   "strikeRandomPoint() or strikeObject() methods are invoked on the Lightning "
   "object on the server.\n"
   "@see Lightning, LightningData\n"
   "@ingroup FX\n"
   "@ingroup Atmosphere\n"
);

LightningStrikeEvent::LightningStrikeEvent()
{
   mLightning = NULL;
   mTarget = NULL;
}

LightningStrikeEvent::~LightningStrikeEvent()
{
}

void LightningStrikeEvent::pack(NetConnection* con, BitStream* stream)
{
   if(!mLightning)
   {
      stream->writeFlag(false);
      return;
   }
   S32 id = con->getGhostIndex(mLightning);
   if(id == -1)
   {
      stream->writeFlag(false);
      return;
   }
   stream->writeFlag(true);
   stream->writeRangedU32(U32(id), 0, NetConnection::MaxGhostCount);
   stream->writeFloat(mStart.x, PositionalBits);
   stream->writeFloat(mStart.y, PositionalBits);

   if( mTarget )
   {
      S32 ghostIndex = con->getGhostIndex(mTarget);
      if (ghostIndex == -1)
         stream->writeFlag(false);
      else
      {
         stream->writeFlag(true);
         stream->writeRangedU32(U32(ghostIndex), 0, NetConnection::MaxGhostCount);
      }
   }
   else
      stream->writeFlag( false );
}

void LightningStrikeEvent::unpack(NetConnection* con, BitStream* stream)
{
   if(!stream->readFlag())
      return;
   S32 mClientId = stream->readRangedU32(0, NetConnection::MaxGhostCount);
   mLightning = NULL;
   NetObject* pObject = con->resolveGhost(mClientId);
   if (pObject)
      mLightning = dynamic_cast<Lightning*>(pObject);

   mStart.x = stream->readFloat(PositionalBits);
   mStart.y = stream->readFloat(PositionalBits);

   if( stream->readFlag() )
   {
      // target id
      S32 mTargetID    = stream->readRangedU32(0, NetConnection::MaxGhostCount);

      NetObject* pObject = con->resolveGhost(mTargetID);
      if( pObject != NULL )
      {
         mTarget = dynamic_cast<SceneObject*>(pObject);
      }
      if( bool(mTarget) == false )
      {
         Con::errorf(ConsoleLogEntry::General, "LightningStrikeEvent::unpack: could not resolve target ghost properly");
      }
   }
}

void LightningStrikeEvent::process(NetConnection*)
{
   if (mLightning)
      mLightning->processEvent(this);
}


//--------------------------------------------------------------------------
//--------------------------------------
//
LightningData::LightningData()
{
   strikeSound = NULL;

   dMemset( strikeTextureNames, 0, sizeof( strikeTextureNames ) );
   dMemset( strikeTextures, 0, sizeof( strikeTextures ) );
   dMemset( thunderSounds, 0, sizeof( thunderSounds ) );
}

LightningData::~LightningData()
{

}

//--------------------------------------------------------------------------
void LightningData::initPersistFields()
{
   addField( "strikeSound", TYPEID< SFXTrack >(), Offset(strikeSound, LightningData),
      "Sound profile to play when a lightning strike occurs." );
   addField( "thunderSounds", TYPEID< SFXTrack >(), Offset(thunderSounds, LightningData), MaxThunders,
      "@brief List of thunder sound effects to play.\n\n"
      "A random one of these sounds will be played shortly after each strike "
      "occurs." );
   addField( "strikeTextures", TypeString, Offset(strikeTextureNames, LightningData), MaxTextures,
      "List of textures to use to render lightning strikes." );

   Parent::initPersistFields();
}


//--------------------------------------------------------------------------
bool LightningData::onAdd()
{
   if(!Parent::onAdd())
      return false;

   return true;
}


bool LightningData::preload(bool server, String &errorStr)
{
   if (Parent::preload(server, errorStr) == false)
      return false;

   dQsort(thunderSounds, MaxThunders, sizeof(SFXTrack*), cmpSounds);
   for (numThunders = 0; numThunders < MaxThunders && thunderSounds[numThunders] != NULL; numThunders++) {
      //
   }

   if (server == false) 
   {
      String errorStr;
      for (U32 i = 0; i < MaxThunders; i++) {
         if( !sfxResolve( &thunderSounds[ i ], errorStr ) )
            Con::errorf(ConsoleLogEntry::General, "LightningData::preload: Invalid packet: %s", errorStr.c_str());
      }

      if( !sfxResolve( &strikeSound, errorStr ) )
         Con::errorf(ConsoleLogEntry::General, "LightningData::preload: Invalid packet: %s", errorStr.c_str());

      for (U32 i = 0; i < MaxTextures; i++) 
      {
         if (strikeTextureNames[i][0])
            strikeTextures[i] = GFXTexHandle(strikeTextureNames[i], &GFXDefaultStaticDiffuseProfile, avar("%s() - strikeTextures[%d] (line %d)", __FUNCTION__, i, __LINE__));
      }
   }


   return true;
}


//--------------------------------------------------------------------------
void LightningData::packData(BitStream* stream)
{
   Parent::packData(stream);

   U32 i;
   for (i = 0; i < MaxThunders; i++)
      sfxWrite( stream, thunderSounds[ i ] );
   for (i = 0; i < MaxTextures; i++) {
      stream->writeString(strikeTextureNames[i]);
   }

   sfxWrite( stream, strikeSound );
}

void LightningData::unpackData(BitStream* stream)
{
   Parent::unpackData(stream);

   U32 i;
   for (i = 0; i < MaxThunders; i++)
      sfxRead( stream, &thunderSounds[ i ] );
   for (i = 0; i < MaxTextures; i++) {
      strikeTextureNames[i] = stream->readSTString();
   }

   sfxRead( stream, &strikeSound );
}


//--------------------------------------------------------------------------
//--------------------------------------
//
Lightning::Lightning()
{
   mNetFlags.set(Ghostable|ScopeAlways);
   mTypeMask |= StaticObjectType|EnvironmentObjectType;

   mLastThink = 0;

   mStrikeListHead  = NULL;
   mThunderListHead = NULL;

   strikesPerMinute = 12;
   strikeWidth = 2.5;
   chanceToHitTarget = 0.5f;
   strikeRadius = 20.0f;
   boltStartRadius = 20.0f;
   color.set( 1.0f, 1.0f, 1.0f, 1.0f );
   fadeColor.set( 0.1f, 0.1f, 1.0f, 1.0f );
   useFog = true;

   setScale( VectorF( 512.0f, 512.0f, 300.0f ) );
}

Lightning::~Lightning()
{
   while( mThunderListHead )
   {
      Thunder* next = mThunderListHead->next;
      delete mThunderListHead;
      mThunderListHead = next;
   }

   while( mStrikeListHead )
   {
      Strike* next = mStrikeListHead->next;
      delete mStrikeListHead;
      mStrikeListHead = next;
   }
}

//--------------------------------------------------------------------------
void Lightning::initPersistFields()
{
   addGroup( "Strikes" );
   addField( "strikesPerMinute", TypeS32, Offset(strikesPerMinute, Lightning),
      "@brief Number of lightning strikes to perform per minute.\n\n"
      "Automatically invokes strikeRandomPoint() at regular intervals." );
   addField( "strikeWidth", TypeF32, Offset(strikeWidth, Lightning),
      "Width of a lightning bolt." );
   addField( "strikeRadius", TypeF32, Offset(strikeRadius, Lightning),
      "@brief Horizontal size (XY plane) of the search box used to find and "
      "damage Player or Vehicle objects within range of the strike.\n\n"
      "Only the object at highest altitude with a clear line of sight to the "
      "bolt will be hit." );
   endGroup( "Strikes" );

   addGroup( "Colors" );
   addField( "color", TypeColorF, Offset(color, Lightning),
      "Color to blend the strike texture with." );
   addField( "fadeColor", TypeColorF, Offset(fadeColor, Lightning),
      "@brief Color to blend the strike texture with when the bolt is fading away.\n\n"
      "Bolts fade away automatically shortly after the strike occurs." );
   endGroup( "Colors" );

   addGroup( "Bolts" );
   addField( "chanceToHitTarget", TypeF32, Offset(chanceToHitTarget, Lightning),
      "Percentage chance (0-1) that a given lightning bolt will hit something." );
   addField( "boltStartRadius", TypeF32, Offset(boltStartRadius, Lightning),
      "@brief Radial distance from the center of the Lightning object for the "
      "start point of the bolt.\n\n"
      "The actual start point will be a random point within this radius." );
   addField( "useFog", TypeBool, Offset(useFog, Lightning),
      "Controls whether lightning bolts are affected by fog when they are rendered." );
   endGroup( "Bolts" );

   Parent::initPersistFields();
}

//--------------------------------------------------------------------------
bool Lightning::onAdd()
{
   if(!Parent::onAdd())
      return false;

   mObjBox.minExtents.set( -0.5f, -0.5f, -0.5f );
   mObjBox.maxExtents.set(  0.5f,  0.5f,  0.5f );

   resetWorldBox();
   addToScene();

   return true;
}


void Lightning::onRemove()
{
   removeFromScene();

   Parent::onRemove();
}


bool Lightning::onNewDataBlock( GameBaseData *dptr, bool reload )
{
   mDataBlock = dynamic_cast<LightningData*>( dptr );
   if ( !mDataBlock || !Parent::onNewDataBlock( dptr, reload ) )
      return false;

   scriptOnNewDataBlock();
   return true;
}


//--------------------------------------------------------------------------
void Lightning::prepRenderImage( SceneRenderState* state )
{
   ObjectRenderInst *ri = state->getRenderPass()->allocInst<ObjectRenderInst>();
   ri->renderDelegate.bind(this, &Lightning::renderObject);
   // The Lightning isn't technically foliage but our debug
   // effect seems to render best as a Foliage type (translucent,
   // renders itself, no sorting)
   ri->type = RenderPassManager::RIT_Foliage;
   state->getRenderPass()->addInst( ri );
}


void Lightning::renderObject(ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance* overrideMat)
{
   if (overrideMat)
      return;

   if (mLightningSB.isNull())
   {
      GFXStateBlockDesc desc;
      desc.setBlend( true, GFXBlendSrcAlpha, GFXBlendOne);
      desc.setCullMode(GFXCullNone);
      desc.zWriteEnable = false;
      desc.samplersDefined = true;
      desc.samplers[0].magFilter = GFXTextureFilterLinear;
      desc.samplers[0].minFilter = GFXTextureFilterLinear;
      desc.samplers[0].addressModeU = GFXAddressWrap;
      desc.samplers[0].addressModeV = GFXAddressWrap;

      mLightningSB = GFX->createStateBlock(desc);

   }

   GFX->setStateBlock(mLightningSB);


   Strike* walk = mStrikeListHead;
   while (walk != NULL)
   {
      GFX->setTexture(0, mDataBlock->strikeTextures[0]);

      for( U32 i=0; i<3; i++ )
      {
         if( walk->bolt[i].isFading )
         {
            F32 alpha = 1.0f - walk->bolt[i].percentFade;
            if( alpha < 0.0f ) alpha = 0.0f;
            PrimBuild::color4f( fadeColor.red, fadeColor.green, fadeColor.blue, alpha );
         }
         else
         {
            PrimBuild::color4f( color.red, color.green, color.blue, color.alpha );
         }
         walk->bolt[i].render( state->getCameraPosition() );
      }

      walk = walk->next;
   }

   //GFX->setZWriteEnable(true);
	//GFX->setAlphaTestEnable(false);
	//GFX->setAlphaBlendEnable(false);
}

void Lightning::scheduleThunder(Strike* newStrike)
{
   AssertFatal(isClientObject(), "Lightning::scheduleThunder: server objects should not enter this version of the function");

   // If no thunder sounds, don't schedule anything!
   if (mDataBlock->numThunders == 0)
      return;

   GameConnection* connection = GameConnection::getConnectionToServer();
   if (connection) {
      MatrixF cameraMatrix;

      if (connection->getControlCameraTransform(0, &cameraMatrix)) {
         Point3F worldPos;
         cameraMatrix.getColumn(3, &worldPos);

         worldPos.x -= newStrike->xVal;
         worldPos.y -= newStrike->yVal;
         worldPos.z  = 0.0f;

         F32 dist = worldPos.len();
         F32 t    = dist / 330.0f;

         // Ok, we need to schedule a random strike sound t secs in the future...
         //
         if (t <= 0.03f) {
            // If it's really close, just play it...
            U32 thunder = sgLightningRand.randI(0, mDataBlock->numThunders - 1);
            SFX->playOnce(mDataBlock->thunderSounds[thunder]);
         } else {
            Thunder* pThunder = new Thunder;
            pThunder->tRemaining = t;
            pThunder->next       = mThunderListHead;
            mThunderListHead     = pThunder;
         }
      }
   }
}


//--------------------------------------------------------------------------
void Lightning::processTick(const Move* move)
{
   Parent::processTick(move);

   if (isServerObject() && !isHidden()) {
      S32 msBetweenStrikes = (S32)(60.0 / strikesPerMinute * 1000.0);

      mLastThink += TickMs;
      if( mLastThink > msBetweenStrikes )
      {
         strikeRandomPoint();
         mLastThink -= msBetweenStrikes;
      }
   }
}

void Lightning::interpolateTick(F32 dt)
{
   Parent::interpolateTick(dt);
}

void Lightning::advanceTime(F32 dt)
{
   Parent::advanceTime(dt);

   Strike** pWalker = &mStrikeListHead;
   while (*pWalker != NULL) {
      Strike* pStrike = *pWalker;

      for( U32 i=0; i<3; i++ )
      {
         pStrike->bolt[i].update( dt );
      }

      pStrike->currentAge += dt;
      if (pStrike->currentAge > pStrike->deathAge) {
         *pWalker = pStrike->next;
         delete pStrike;
      } else {
         pWalker = &((*pWalker)->next);
      }
   }

   Thunder** pThunderWalker = &mThunderListHead;
   while (*pThunderWalker != NULL) {
      Thunder* pThunder = *pThunderWalker;

      pThunder->tRemaining -= dt;
      if (pThunder->tRemaining <= 0.0f) {
         *pThunderWalker = pThunder->next;
         delete pThunder;

         // Play the sound...
         U32 thunder = sgLightningRand.randI(0, mDataBlock->numThunders - 1);
         SFX->playOnce(mDataBlock->thunderSounds[thunder]);
      } else {
         pThunderWalker = &((*pThunderWalker)->next);
      }
   }
}


//--------------------------------------------------------------------------
void Lightning::processEvent(LightningStrikeEvent* pEvent)
{
      AssertFatal(pEvent->mStart.x >= 0.0f && pEvent->mStart.x <= 1.0f, "Out of bounds coord!");

      Strike* pStrike = new Strike;

      Point3F strikePoint;
      strikePoint.zero();

      if( pEvent->mTarget )
      {
         Point3F objectCenter;
         pEvent->mTarget->getObjBox().getCenter( &objectCenter );
         objectCenter.convolve( pEvent->mTarget->getScale() );
         pEvent->mTarget->getTransform().mulP( objectCenter );

         strikePoint = objectCenter;
      }
      else
      {
         strikePoint.x = pEvent->mStart.x;
         strikePoint.y = pEvent->mStart.y;
         strikePoint *= mObjScale;
         strikePoint += getPosition();
         strikePoint += Point3F( -mObjScale.x * 0.5f, -mObjScale.y * 0.5f, 0.0f );

         RayInfo rayInfo;
         Point3F start = strikePoint;
         start.z = mObjScale.z * 0.5f + getPosition().z;
         strikePoint.z += -mObjScale.z * 0.5f;
         bool rayHit = gClientContainer.castRay( start, strikePoint,
                                      (STATIC_COLLISION_TYPEMASK | WaterObjectType),
                                      &rayInfo);
         if( rayHit )
         {
            strikePoint.z = rayInfo.point.z;
         }
         else
         {
            strikePoint.z = pStrike->bolt[0].findHeight( strikePoint, getSceneManager() );
         }
      }

      pStrike->xVal       = strikePoint.x;
      pStrike->yVal       = strikePoint.y;

      pStrike->deathAge   = 1.6f;
      pStrike->currentAge = 0.0f;
      pStrike->next       = mStrikeListHead;

      for( U32 i=0; i<3; i++ )
      {
         F32 randStart = boltStartRadius;
         F32 height = mObjScale.z * 0.5f + getPosition().z;
         pStrike->bolt[i].startPoint.set( pStrike->xVal + gRandGen.randF( -randStart, randStart ), pStrike->yVal + gRandGen.randF( -randStart, randStart ), height );
         pStrike->bolt[i].endPoint = strikePoint;
         pStrike->bolt[i].width = strikeWidth;
         pStrike->bolt[i].numMajorNodes = 10;
         pStrike->bolt[i].maxMajorAngle = 30.0f;
         pStrike->bolt[i].numMinorNodes = 4;
         pStrike->bolt[i].maxMinorAngle = 15.0f;
         pStrike->bolt[i].generate();
         pStrike->bolt[i].startSplits();
         pStrike->bolt[i].lifetime = 1.0f;
         pStrike->bolt[i].fadeTime = 0.2f;
         pStrike->bolt[i].renderTime = gRandGen.randF(0.0f, 0.25f);
      }

      mStrikeListHead     = pStrike;

      scheduleThunder(pStrike);

      MatrixF trans(true);
      trans.setPosition( strikePoint );

      if (mDataBlock->strikeSound)
      {
         SFX->playOnce(mDataBlock->strikeSound, &trans );
      }

}

void Lightning::warningFlashes()
{
   AssertFatal(isServerObject(), "Error, client objects may not initiate lightning!");


   SimGroup* pClientGroup = Sim::getClientGroup();
   for (SimGroup::iterator itr = pClientGroup->begin(); itr != pClientGroup->end(); itr++) {
      NetConnection* nc = static_cast<NetConnection*>(*itr);
      if (nc != NULL)
      {
         LightningStrikeEvent* pEvent = new LightningStrikeEvent;
         pEvent->mLightning = this;

         nc->postNetEvent(pEvent);
      }
   }
}

void Lightning::strikeRandomPoint()
{
   AssertFatal(isServerObject(), "Error, client objects may not initiate lightning!");


   Point3F strikePoint( gRandGen.randF( 0.0f, 1.0f ), gRandGen.randF( 0.0f, 1.0f ), 0.0f );

   // check if an object is within target range

   strikePoint *= mObjScale;
   strikePoint += getPosition();
   strikePoint += Point3F( -mObjScale.x * 0.5f, -mObjScale.y * 0.5f, 0.0f );

   Box3F queryBox;
   F32 boxWidth = strikeRadius * 2.0f;

   queryBox.minExtents.set( -boxWidth * 0.5f, -boxWidth * 0.5f, -mObjScale.z * 0.5f );
   queryBox.maxExtents.set(  boxWidth * 0.5f,  boxWidth * 0.5f,  mObjScale.z * 0.5f );
   queryBox.minExtents += strikePoint;
   queryBox.maxExtents += strikePoint;

   SimpleQueryList sql;
   getContainer()->findObjects(queryBox, DAMAGEABLE_TYPEMASK,
                               SimpleQueryList::insertionCallback, &sql);

   SceneObject *highestObj = NULL;
   F32 highestPnt = 0.0f;

   for( U32 i = 0; i < sql.mList.size(); i++ )
   {
      Point3F objectCenter;
      sql.mList[i]->getObjBox().getCenter(&objectCenter);
      objectCenter.convolve(sql.mList[i]->getScale());
      sql.mList[i]->getTransform().mulP(objectCenter);

      // check if object can be struck

      RayInfo rayInfo;
      Point3F start = objectCenter;
      start.z = mObjScale.z * 0.5f + getPosition().z;
      Point3F end = objectCenter;
      end.z = -mObjScale.z * 0.5f + getPosition().z;
      bool rayHit = gServerContainer.castRay( start, end,
                                   (0xFFFFFFFF),
                                   &rayInfo);

      if( rayHit && rayInfo.object == sql.mList[i] )
      {
         if( !highestObj )
         {
            highestObj = sql.mList[i];
            highestPnt = objectCenter.z;
            continue;
         }

         if( objectCenter.z > highestPnt )
         {
            highestObj = sql.mList[i];
            highestPnt = objectCenter.z;
         }
      }


   }

   // hah haaaaa, we have a target!
   SceneObject *targetObj = NULL;
   if( highestObj )
   {
      F32 chance = gRandGen.randF();
      if( chance <= chanceToHitTarget )
      {
         Point3F objectCenter;
         highestObj->getObjBox().getCenter(&objectCenter);
         objectCenter.convolve(highestObj->getScale());
         highestObj->getTransform().mulP(objectCenter);

         bool playerInWarmup = false;
         Player *playerObj = dynamic_cast< Player * >(highestObj);
         if( playerObj )
         {
            if( !playerObj->getControllingClient() )
            {
               playerInWarmup = true;
            }
         }

         if( !playerInWarmup )
         {
            applyDamage_callback( objectCenter, VectorF( 0.0f, 0.0f, 1.0f ), highestObj );
            targetObj = highestObj;
         }
      }
   }

   SimGroup* pClientGroup = Sim::getClientGroup();
   for (SimGroup::iterator itr = pClientGroup->begin(); itr != pClientGroup->end(); itr++)
   {
      NetConnection* nc = static_cast<NetConnection*>(*itr);

      LightningStrikeEvent* pEvent = new LightningStrikeEvent;
      pEvent->mLightning = this;

      pEvent->mStart.x = strikePoint.x;
      pEvent->mStart.y = strikePoint.y;
      pEvent->mTarget = targetObj;

      nc->postNetEvent(pEvent);
   }


}

//--------------------------------------------------------------------------
void Lightning::strikeObject(ShapeBase*)
{
   AssertFatal(isServerObject(), "Error, client objects may not initiate lightning!");

   AssertFatal(false, "Lightning::strikeObject is not implemented.");
}


//--------------------------------------------------------------------------
U32 Lightning::packUpdate(NetConnection* con, U32 mask, BitStream* stream)
{
   U32 retMask = Parent::packUpdate(con, mask, stream);

   // Only write data if this is the initial packet or we've been inspected.
   if (stream->writeFlag(mask & (InitialUpdateMask | ExtendedInfoMask)))
   {
      // Initial update
      mathWrite(*stream, getPosition());
      mathWrite(*stream, mObjScale);

      stream->write(strikeWidth);
      stream->write(chanceToHitTarget);
      stream->write(strikeRadius);
      stream->write(boltStartRadius);
      stream->write(color.red);
      stream->write(color.green);
      stream->write(color.blue);
      stream->write(fadeColor.red);
      stream->write(fadeColor.green);
      stream->write(fadeColor.blue);
      stream->write(useFog);
      stream->write(strikesPerMinute);
   }

   return retMask;
}

//--------------------------------------------------------------------------
void Lightning::unpackUpdate(NetConnection* con, BitStream* stream)
{
   Parent::unpackUpdate(con, stream);

   if (stream->readFlag())
   {
      // Initial update
      Point3F pos;
      mathRead(*stream, &pos);
      setPosition( pos );

      mathRead(*stream, &mObjScale);

      stream->read(&strikeWidth);
      stream->read(&chanceToHitTarget);
      stream->read(&strikeRadius);
      stream->read(&boltStartRadius);
      stream->read(&color.red);
      stream->read(&color.green);
      stream->read(&color.blue);
      stream->read(&fadeColor.red);
      stream->read(&fadeColor.green);
      stream->read(&fadeColor.blue);
      stream->read(&useFog);
      stream->read(&strikesPerMinute);
   }
}

//--------------------------------------------------------------------------

DefineEngineMethod(Lightning, warningFlashes, void, (),,
   "@brief Creates a LightningStrikeEvent that triggers harmless lightning "
   "bolts on all clients.\n"
   "No objects will be damaged by these bolts.\n"
   "@tsexample\n"
   "// Generate a harmless lightning strike effect on all clients\n"
   "%lightning.warningFlashes();\n"
   "@endtsexample" )
{
   if (object->isServerObject()) 
      object->warningFlashes();
}

DefineEngineMethod(Lightning, strikeRandomPoint, void, (),,
   "Creates a LightningStrikeEvent which attempts to strike and damage a random "
   "object in range of the Lightning object.\n"
   "@tsexample\n"
   "// Generate a damaging lightning strike effect on all clients\n"
   "%lightning.strikeRandomPoint();\n"
   "@endtsexample" )
{
   if (object->isServerObject()) 
      object->strikeRandomPoint();
}

DefineEngineMethod(Lightning, strikeObject, void, (S32 id), (NULL),
   "Creates a LightningStrikeEvent which strikes a specific object.\n"
   "@note This method is currently unimplemented.\n" )
{
   ShapeBase* pSB;

   if (object->isServerObject() && Sim::findObject(id, pSB))
      object->strikeObject(pSB);
}

//**************************************************************************
// Lightning Bolt
//**************************************************************************
LightningBolt::LightningBolt()
{
   width = 0.1f;
   startPoint.zero();
   endPoint.zero();
   chanceOfSplit = 0.0f;
   isFading = false;
   elapsedTime = 0.0f;
   lifetime = 1.0f;
   startRender = false;
}

//--------------------------------------------------------------------------
// Destructor
//--------------------------------------------------------------------------
LightningBolt::~LightningBolt()
{
   splitList.clear();
}

//--------------------------------------------------------------------------
// Generate nodes
//--------------------------------------------------------------------------
void LightningBolt::NodeManager::generateNodes()
{
   F32 overallDist = VectorF( endPoint - startPoint ).magnitudeSafe();
   F32 minDistBetweenNodes = overallDist / (numNodes-1);
   F32 maxDistBetweenNodes = minDistBetweenNodes / mCos( maxAngle * M_PI_F / 180.0f );

   VectorF mainLineDir = endPoint - startPoint;
   mainLineDir.normalizeSafe();

   for( U32 i=0; i<numNodes; i++ )
   {
      Node node;

      if( i == 0 )
      {
         node.point = startPoint;
         node.dirToMainLine = mainLineDir;
         nodeList[i] = node;
         continue;
      }
      if( i == numNodes - 1 )
      {
         node.point = endPoint;
         nodeList[i] = node;
         break;
      }

      Node lastNode = nodeList[i-1];

      F32 segmentLength = gRandGen.randF( minDistBetweenNodes, maxDistBetweenNodes );
      VectorF segmentDir = MathUtils::randomDir( lastNode.dirToMainLine, 0, maxAngle );
      node.point = lastNode.point + segmentDir * segmentLength;

      node.dirToMainLine = endPoint - node.point;
      node.dirToMainLine.normalizeSafe();
      nodeList[i] = node;
   }
}


//--------------------------------------------------------------------------
// Render bolt
//--------------------------------------------------------------------------
void LightningBolt::render( const Point3F &camPos )
{
   if (!startRender)
      return;

   if (!isFading)
      generateMinorNodes();

   U32 maxVerts = 0;
   for (U32 i = 0; i < mMinorNodes.size(); i++)
      maxVerts += mMinorNodes[i].numNodes * 2;

   PrimBuild::begin(GFXTriangleStrip, maxVerts);

   for (U32 i = 0; i < mMinorNodes.size(); i++)
   {
      if (i+1 == mMinorNodes.size())
         renderSegment(mMinorNodes[i], camPos, true);
      else
         renderSegment(mMinorNodes[i], camPos, false);
   }

	PrimBuild::end();

   for(LightingBoltList::Iterator i = splitList.begin(); i != splitList.end(); ++i)
   {
      if( isFading )
      {
         i->isFading = true;
      }
      i->render( camPos );
   }

}

//--------------------------------------------------------------------------
// Render segment
//--------------------------------------------------------------------------
void LightningBolt::renderSegment( NodeManager &segment, const Point3F &camPos, bool renderLastPoint )
{

   for (U32 i = 0; i < segment.numNodes; i++)
   {
      Point3F  curPoint = segment.nodeList[i].point;

      Point3F  nextPoint;
      Point3F  segDir;

      if( i == (segment.numNodes-1) )
      {
         if( renderLastPoint )
         {
            segDir = curPoint - segment.nodeList[i-1].point;
         }
         else
         {
            continue;
         }
      }
      else
      {
         nextPoint = segment.nodeList[i+1].point;
         segDir = nextPoint - curPoint;
      }
      segDir.normalizeSafe();


      Point3F dirFromCam = curPoint - camPos;
      Point3F crossVec;
      mCross(dirFromCam, segDir, &crossVec);
      crossVec.normalize();
      crossVec *= width * 0.5f;

      F32 u = i % 2;

      PrimBuild::texCoord2f( u, 1.0 );
      PrimBuild::vertex3fv( curPoint - crossVec );

      PrimBuild::texCoord2f( u, 0.0 );
      PrimBuild::vertex3fv( curPoint + crossVec );
   }

}

//----------------------------------------------------------------------------
// Find height
//----------------------------------------------------------------------------
F32 LightningBolt::findHeight( Point3F &point, SceneManager *sceneManager )
{
   const Vector< SceneObject* > terrains = sceneManager->getContainer()->getTerrains();
   for( Vector< SceneObject* >::const_iterator iter = terrains.begin(); iter != terrains.end(); ++ iter )
   {
      TerrainBlock* terrain = dynamic_cast< TerrainBlock* >( *iter );
      if( !terrain )
         continue;

      Point3F terrPt = point;
      terrain->getWorldTransform().mulP(terrPt);

      F32 h;
      if( terrain->getHeight( Point2F( terrPt.x, terrPt.y ), &h ) )
         return h;
   }

   return 0.f;
}


//----------------------------------------------------------------------------
// Generate lightning bolt
//----------------------------------------------------------------------------
void LightningBolt::generate()
{
   mMajorNodes.startPoint   = startPoint;
   mMajorNodes.endPoint     = endPoint;
   mMajorNodes.numNodes     = numMajorNodes;
   mMajorNodes.maxAngle     = maxMajorAngle;

   mMajorNodes.generateNodes();

   generateMinorNodes();

}

//----------------------------------------------------------------------------
// Generate Minor Nodes
//----------------------------------------------------------------------------
void LightningBolt::generateMinorNodes()
{
   mMinorNodes.clear();

   for( int i=0; i<mMajorNodes.numNodes - 1; i++ )
   {
      NodeManager segment;
      segment.startPoint = mMajorNodes.nodeList[i].point;
      segment.endPoint = mMajorNodes.nodeList[i+1].point;
      segment.numNodes = numMinorNodes;
      segment.maxAngle = maxMinorAngle;
      segment.generateNodes();

      mMinorNodes.increment(1);
      mMinorNodes[i] = segment;
   }
}

//----------------------------------------------------------------------------
// Recursive algo to create bolts that split off from main bolt
//----------------------------------------------------------------------------
void LightningBolt::createSplit( const Point3F &startPoint, const Point3F &endPoint, U32 depth, F32 width )
{
   if( depth == 0 )
      return;
	  
   F32 chanceToEnd = gRandGen.randF();
   if( chanceToEnd > 0.70f )
      return;

   if( width < 0.75f )
      width = 0.75f;

   VectorF diff = endPoint - startPoint;
   F32 length = diff.len();
   diff.normalizeSafe();

   LightningBolt newBolt;
   newBolt.startPoint = startPoint;
   newBolt.endPoint = endPoint;
   newBolt.width = width;
   newBolt.numMajorNodes = 3;
   newBolt.maxMajorAngle = 30.0f;
   newBolt.numMinorNodes = 3;
   newBolt.maxMinorAngle = 10.0f;
   newBolt.startRender = true;
   newBolt.generate();

   splitList.pushBack( newBolt );

   VectorF newDir1 = MathUtils::randomDir( diff, 10.0f, 45.0f );
   Point3F newEndPoint1 = endPoint + newDir1 * gRandGen.randF( 0.5f, 1.5f ) * length;

   VectorF newDir2 = MathUtils::randomDir( diff, 10.0f, 45.0f );
   Point3F newEndPoint2 = endPoint + newDir2 * gRandGen.randF( 0.5f, 1.5f ) * length;

   createSplit( endPoint, newEndPoint1, depth - 1, width * 0.30f );
   createSplit( endPoint, newEndPoint2, depth - 1, width * 0.30f );

}

//----------------------------------------------------------------------------
// Start split - kick off the recursive 'createSplit' procedure
//----------------------------------------------------------------------------
void LightningBolt::startSplits()
{

   for( U32 i=0; i<mMajorNodes.numNodes-1; i++ )
   {
      if( gRandGen.randF() > 0.3f )
	     continue;

      Node node = mMajorNodes.nodeList[i];
      Node node2 = mMajorNodes.nodeList[i+1];

      VectorF segDir = node2.point - node.point;
      F32 length = segDir.len();
      segDir.normalizeSafe();

      VectorF newDir = MathUtils::randomDir( segDir, 20.0f, 40.0f );
      Point3F newEndPoint = node.point + newDir * gRandGen.randF( 0.5f, 1.5f ) * length;


      createSplit( node.point, newEndPoint, 4, width * 0.30f );
   }


}

//----------------------------------------------------------------------------
// Update
//----------------------------------------------------------------------------
void LightningBolt::update( F32 dt )
{
   elapsedTime += dt;

   F32 percentDone = elapsedTime / lifetime;

   if( elapsedTime > fadeTime )
   {
      isFading = true;
      percentFade = percentDone + (fadeTime/lifetime);
   }

   if( elapsedTime > renderTime && !startRender )
   {
      startRender = true;
      isFading = false;
      elapsedTime = 0.0f;
   }
}