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
#include "forest/forestWindEmitter.h"

#include "forest/forestWindMgr.h"
#include "console/consoleInternal.h"
#include "core/stream/bitStream.h"
#include "core/util/safeDelete.h"
#include "platform/profiler.h"
#include "math/mathIO.h"
#include "math/mRandom.h"
#include "scene/sceneManager.h"
#include "scene/sceneRenderState.h"
#include "renderInstance/renderPassManager.h"
#include "gfx/gfxDrawUtil.h"
#include "gfx/gfxTransformSaver.h"
#include "sim/netConnection.h"
#include "T3D/gameBase/processList.h"
#include "console/engineAPI.h"

ConsoleDocClass( ForestWindEmitter,
   "@brief Object responsible for simulating wind in a level.\n\n"

   "When placed in the level, a ForestWindEmitter will cause tree branches to bend and sway, leaves "
   "to flutter, and create vertical bending on the tree's trunk.\n\n"

   "@tsexample\n"
   "// The following is a full declaration of a wind emitter\n"
   "new ForestWindEmitter()\n"
   "{\n"
   "   position = \"497.739 765.821 102.395\";\n"
   "   windEnabled = \"1\";\n"
   "   radialEmitter = \"1\";\n"
   "   strength = \"1\";\n"
   "   radius = \"3\";\n"
   "   gustStrength = \"0.5\";\n"
   "   gustFrequency = \"1\";\n"
   "   gustYawAngle = \"10\";\n"
   "   gustYawFrequency = \"4\";\n"
   "   gustWobbleStrength = \"2\";\n"
   "   turbulenceStrength = \"1\";\n"
   "   turbulenceFrequency = \"2\";\n"
   "   hasMount = \"0\";\n"
   "   scale = \"3 3 3\";\n"
   "   canSave = \"1\";\n"
   "   canSaveDynamicFields = \"1\";\n"
   "   rotation = \"1 0 0 0\";\n"
   "};\n"
   "@endtsexample\n\n"

   "@ingroup FX\n"
   "@ingroup Forest\n"
   "@ingroup Atmosphere\n"
);

// We need to know when the mission editor is enabled.
extern bool gEditingMission;

ForestWind::ForestWind(  ForestWindEmitter *emitter )
   :  mStrength( 0.0f ),
      mDirection( 1.0f, 0, 0 ),
      mLastGustTime( 0 ),
      mLastYawTime( 0 ),
      mCurrentTarget( 0, 0 ),
      mCurrentInterp( 0 ),
      mTargetYawAngle( 0 ),
      mParent( emitter ),
      mIsDirty( false ),
      mRandom( Platform::getRealMilliseconds() + 1 )
{
}

ForestWind::~ForestWind()
{
}

void ForestWind::processTick()
{
   PROFILE_SCOPE( ForestWind_ProcessTick );

   const F32 deltaTime = 0.032f;
   const U32 simTime = Sim::getCurrentTime();
   
   Point2F finalVec( 0, 0 );
   Point2F windDir( mParent->mWindDirection.x, mParent->mWindDirection.y );

   if ( mLastGustTime < simTime )
   {
      Point2F turbVec( 0, 0 );
      if ( mLastGustTime < simTime + (mParent->mWindTurbulenceFrequency * 1000.0f) )
         turbVec = (mRandom.randF() * mParent->mWindTurbulenceStrength) * windDir;

      mLastGustTime = simTime + (mParent->mWindGustFrequency * 1000.0f);
      Point2F gustVec = (mRandom.randF() * mParent->mWindGustStrength + mRandom.randF() * mParent->mWindGustWobbleStrength) * windDir;

      finalVec += gustVec + turbVec;
      //finalVec.normalizeSafe();
   }
   
   //bool rotationChange = false;

   if ( mLastYawTime < simTime )
   {
      mLastYawTime = simTime + (mParent->mWindGustYawFrequency * 1000.0f);
      F32 rotateAmt = mRandom.randF() * mParent->mWindGustYawAngle + mRandom.randF() * mParent->mWindGustWobbleStrength;
      
      if ( mRandom.randF() <= 0.5f )
         rotateAmt = -rotateAmt;

      rotateAmt = mDegToRad( rotateAmt );

      if ( rotateAmt > M_2PI_F )
         rotateAmt -= M_2PI_F;
      else if ( rotateAmt < -M_2PI_F )
         rotateAmt += M_2PI_F;

      mTargetYawAngle = rotateAmt;

      //finalVec.rotate( rotateAmt );
      mCurrentTarget.rotate( rotateAmt );
   }
   
   //mCurrentTarget.normalizeSafe();

   if ( mCurrentTarget.isZero() || mCurrentInterp >= 1.0f )
   {
      mCurrentInterp = 0;
      mCurrentTarget.set( 0, 0 );
   
      Point2F windDir( mDirection.x, mDirection.y );
      windDir.normalizeSafe();

      mCurrentTarget = finalVec + windDir;
   }
   else
   {
      mCurrentInterp += deltaTime;
      mCurrentInterp = mClampF( mCurrentInterp, 0.0f, 1.0f );
      mDirection.interpolate( mDirection, Point3F( mCurrentTarget.x, mCurrentTarget.y, 0 ), mCurrentInterp );
      //F32 rotateAmt = mLerp( 0, mTargetYawAngle, mCurrentInterp );

      //mTargetYawAngle -= rotateAmt;

      //Point2F dir( mDirection.x, mDirection.y );
      //if ( mTargetYawAngle > 0.0f )
        // dir.rotate( rotateAmt );

      //mDirection.set( dir.x, dir.y, 0 );
   }
}

void ForestWind::setStrengthAndDirection( F32 strength, const VectorF &direction )
{
   if (  mStrength != strength ||
         mDirection != direction )
   {
      mStrength = strength;
      mDirection = direction;
      mIsDirty = true;
   }
}

void ForestWind::setStrength( F32 strength )
{
   if ( mStrength != strength )
   {
      mStrength = strength;
      mIsDirty = true;
   }
}

void ForestWind::setDirection( const VectorF &direction )
{
   if ( mDirection != direction )
   {
      mDirection = direction;
      mIsDirty = true;
   }
}

IMPLEMENT_CO_NETOBJECT_V1(ForestWindEmitter);

ForestWindEmitter::ForestWindEmitter( bool makeClientObject )
   :  mEnabled( true ),
      mAddedToScene( false ),      
      mWind( NULL ),
      mWindStrength( 1 ),
      mWindDirection( 1, 0, 0 ),
      mWindGustFrequency( 3.0f ),
      mWindGustStrength( 0.25f ),
      mWindGustYawAngle( 10.0f ),
      mWindGustYawFrequency( 4.0f ),
      mWindGustWobbleStrength( 2.0f ),
      mWindTurbulenceFrequency( 2.0f ),
      mWindTurbulenceStrength( 0.25f ),
      mWindRadius( 0 ),
      mRadialEmitter( false ),
      mHasMount( false ),
      mIsMounted( false ),
      mMountObject( NULL )
{
   mTypeMask |= StaticObjectType | EnvironmentObjectType;

   if ( makeClientObject )
      mNetFlags.set( IsGhost );
   else
      mNetFlags.set( Ghostable | ScopeAlways );
}

ForestWindEmitter::~ForestWindEmitter()
{
}

void ForestWindEmitter::initPersistFields()
{
   // Initialise parents' persistent fields.
   Parent::initPersistFields();

   addGroup( "ForestWind" );
      addField( "windEnabled",         TypeBool,      Offset( mEnabled, ForestWindEmitter ), "Determines if the emitter will be counted in wind calculations." );
      addField( "radialEmitter",       TypeBool,      Offset( mRadialEmitter, ForestWindEmitter ), "Determines if the emitter is a global direction or local radial emitter." );
      addField( "strength",            TypeF32,       Offset( mWindStrength, ForestWindEmitter ), "The strength of the wind force." );
      addField( "radius",              TypeF32,       Offset( mWindRadius, ForestWindEmitter ), "The radius of the emitter for local radial emitters." );
      addField( "gustStrength",        TypeF32,       Offset( mWindGustStrength, ForestWindEmitter ), "The maximum strength of a gust." );
      addField( "gustFrequency",       TypeF32,       Offset( mWindGustFrequency, ForestWindEmitter ), "The frequency of gusting in seconds." );
      addField( "gustYawAngle",        TypeF32,       Offset( mWindGustYawAngle, ForestWindEmitter ), "The amount of degrees the wind direction can drift (both positive and negative)." );
      addField( "gustYawFrequency",    TypeF32,       Offset( mWindGustYawFrequency, ForestWindEmitter ), "The frequency of wind yaw drift, in seconds." );
      addField( "gustWobbleStrength",  TypeF32,       Offset( mWindGustWobbleStrength, ForestWindEmitter ), "The amount of random wobble added to gust and turbulence vectors." );
      addField( "turbulenceStrength",  TypeF32,       Offset( mWindTurbulenceStrength, ForestWindEmitter ), "The strength of gust turbulence." ); 
      addField( "turbulenceFrequency", TypeF32,       Offset( mWindTurbulenceFrequency, ForestWindEmitter ), "The frequency of gust turbulence, in seconds." ); 
      addField( "hasMount", TypeBool, Offset( mHasMount, ForestWindEmitter ), "Determines if the emitter is mounted to another object." );
   endGroup( "ForestWind" );
}

U32 ForestWindEmitter::packUpdate(NetConnection * con, U32 mask, BitStream * stream)
{
   U32 retMask = Parent::packUpdate( con, mask, stream );

   mathWrite( *stream, mObjToWorld );

   if ( stream->writeFlag( mask & EnabledMask ) )
      stream->writeFlag( mEnabled );

   if ( stream->writeFlag( mask & WindMask ) )
   {
      stream->write( mWindStrength );

      stream->write( mWindRadius );
      stream->writeFlag( mRadialEmitter );

      stream->write( mWindGustStrength );
      stream->write( mWindGustFrequency );

      stream->write( mWindGustYawAngle );
      stream->write( mWindGustYawFrequency );
      stream->write( mWindGustWobbleStrength );

      stream->write( mWindTurbulenceStrength );
      stream->write( mWindTurbulenceFrequency );

      // The wind direction should be normalized!
      if ( mWindDirection.isZero() )
      {
         VectorF forwardVec( 0, 0, 0 );
         mWorldToObj.getColumn( 1, &mWindDirection );
      }
      else
         mWindDirection.normalize();

      stream->writeNormalVector( mWindDirection, 8 );  

      stream->writeFlag( mHasMount );
   } 

   return retMask;
}

void ForestWindEmitter::unpackUpdate(NetConnection * con, BitStream * stream)
{
   // Unpack Parent.
   Parent::unpackUpdate( con, stream );

   MatrixF xfm;
   mathRead( *stream, &xfm );
   Parent::setTransform( xfm );

   U32 windMask = 0;

   if ( stream->readFlag() ) // EnabledMask
      mEnabled = stream->readFlag();

   if ( stream->readFlag() ) // WindMask
   {
      stream->read( &mWindStrength );
      stream->read( &mWindRadius );
      
      mRadialEmitter = stream->readFlag();

      stream->read( &mWindGustStrength );
      stream->read( &mWindGustFrequency );
      
      stream->read( &mWindGustYawAngle );
      stream->read( &mWindGustYawFrequency );
      stream->read( &mWindGustWobbleStrength );

      stream->read( &mWindTurbulenceStrength );
      stream->read( &mWindTurbulenceFrequency );

      stream->readNormalVector( &mWindDirection, 8 );
      windMask |= WindMask;

      mHasMount = stream->readFlag();
   }

   // This does nothing if the masks are not set!
   if ( windMask != 0 && isProperlyAdded() )
   {
      Point3F boxRad( 0, 0, 0 );

      if ( !isRadialEmitter() )
         boxRad.set( 10000.0f, 10000.0f, 10000.0f );
      else
         boxRad.set( mWindRadius, mWindRadius, mWindRadius ); 
         
      mObjBox.set( -boxRad, boxRad );
      resetWorldBox();
      
      _initWind( windMask );
   }
}

bool ForestWindEmitter::onAdd()
{
   if ( !Parent::onAdd() )
      return false;
  
   // Only the client side actually does wind.
   if ( isClientObject() )
   {
      // TODO: wasn't this a big hack we already fixed better?
      //Projectile::getGhostReceivedSignal().notify( this, &ForestWindEmitter::_onMountObjectGhostReceived );
   
      _initWind();
      WINDMGR->addEmitter( this );
   }

   Point3F boxRad( 0, 0, 0 );

   if ( !isRadialEmitter() )
      boxRad.set( 10000.0f, 10000.0f, 10000.0f );
   else
      boxRad.set( mWindRadius, mWindRadius, mWindRadius ); 
      
   mObjBox.set( -boxRad, boxRad );
   resetWorldBox();

   enableCollision();

   // If we are we editing the mission then
   // be sure to add us to the scene.
   if ( gEditingMission || mHasMount )
   {
      addToScene();
      mAddedToScene = true;
   }

   return true;
} 

void ForestWindEmitter::onRemove()
{
   // Only the client side actually does wind.
   if ( isClientObject() )
   {
      //Projectile::getGhostReceivedSignal().remove( this, &ForestWindEmitter::_onMountObjectGhostReceived );

      WINDMGR->removeEmitter( this );
      SAFE_DELETE( mWind );
   }

   // If we are editing the mission then remove
   // us from the scene graph.
   if ( gEditingMission || mHasMount )
   {
      removeFromScene();
      mAddedToScene = false;
   }

   // Do Parent.
   Parent::onRemove();
}

void ForestWindEmitter::_onMountObjectGhostReceived( SceneObject *object )
{
   if ( !object )
      return;

   attachToObject( object );
}

void ForestWindEmitter::inspectPostApply()
{
   // Force the client update!
   setMaskBits(0xffffffff);
}

void ForestWindEmitter::onEditorEnable()
{
   if ( !mAddedToScene )
   {
      addToScene();
      mAddedToScene = true;
   }
}

void ForestWindEmitter::onEditorDisable()
{
   // Remove us from the scene.
   if ( mAddedToScene )
   {
      removeFromScene();
      mAddedToScene = false;
   }
}

void ForestWindEmitter::_initWind( U32 mask )
{
   AssertFatal( !isServerObject(), "SpeedWind is never updated on the server!" );

   // If we don't have a wind 
   // object create one now.
   if ( !mWind ) 
      mWind = new ForestWind( this );

   // Do we need to apply a new direction and strength?
   if ( mask & WindMask )
   {
      mWorldToObj.getColumn( 1, &mWindDirection );
      mWind->setStrengthAndDirection( mWindStrength, mWindDirection );
   }
}

void ForestWindEmitter::setTransform( const MatrixF &mat )
{
   Parent::setTransform( mat );

   // Force the client update!
   setMaskBits(0xffffffff);

   if ( isClientObject() )
      _initWind();
}

void ForestWindEmitter::prepRenderImage( SceneRenderState* state )
{
   PROFILE_SCOPE( ForestWindEmitter_PrepRenderImage );

   // Only render the radius and
   // direction if we're in the editor.
   // Don't render them if this is a reflect pass.
   if ( !state->isDiffusePass() || !gEditingMission )
      return;

   // This should be sufficient for most objects that don't manage zones, and
   // don't need to return a specialized RenderImage...
   ObjectRenderInst *ri = state->getRenderPass()->allocInst<ObjectRenderInst>();
   ri->renderDelegate.bind( this, &ForestWindEmitter::_renderEmitterInfo );
   ri->type = RenderPassManager::RIT_Editor;
   state->getRenderPass()->addInst( ri );
}

void ForestWindEmitter::_renderEmitterInfo( ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance *overrideMat )
{
   if ( overrideMat )
      return;

   GFXTransformSaver saver;

   GFXDrawUtil *drawer = GFX->getDrawUtil();

   AssertFatal( drawer, "Got NULL GFXDrawUtil!" );

   const Point3F &pos = getPosition();
   const VectorF &windVec = mWind->getDirection();

   GFXStateBlockDesc desc;
   desc.setBlend( true );
   desc.setZReadWrite( true, false );

   // Draw an arrow pointing
   // in the wind direction.
   drawer->drawArrow( desc, pos, pos + (windVec * mWindStrength), ColorI( 0, 0, 255, 255 ) );//Point3F( -235.214, 219.589, 34.0991 ), Point3F( -218.814, 244.731, 37.5587 ), ColorI( 255, 255, 0, 255 ) );//
   drawer->drawArrow( desc, pos, pos + (mWind->getTarget() * mWindStrength ), ColorI( 255, 0, 0, 85 ) );

   // Draw a 2D circle for the wind radius.
   if ( isRadialEmitter() )
      drawer->drawSphere( desc, mWindRadius, pos, ColorI( 255, 0, 0, 80 ) );
}

F32 ForestWindEmitter::getStrength() const
{ 
   return mWind->getStrength(); 
}

void ForestWindEmitter::setStrength( F32 strength )
{
   mWindStrength = strength;
   mWind->setStrength( mWindStrength );
}

void ForestWindEmitter::attachToObject( SceneObject *obj )
{
   if ( !obj )
      return;

   mMountObject = obj;
   mIsMounted = true;

   if ( isServerObject() )
      deleteNotify( mMountObject );
}

void ForestWindEmitter::updateMountPosition()
{
   AssertFatal( isClientObject(), "ForestWindEmitter::updateMountPosition - This should only happen on the client!" );

   if ( !mHasMount || !mMountObject )
      return;

   MatrixF mat( true ); 
   mat.setPosition( mMountObject->getPosition() );
   Parent::setTransform( mat );
}

void ForestWindEmitter::onDeleteNotify(SimObject *object)
{
   AssertFatal( isServerObject(), "ForestWindEmitter::onDeleteNotify - This should never happen on the client!" );
   safeDeleteObject();
}

DefineEngineMethod( ForestWindEmitter, attachToObject, void, ( U32 objectID ),,
   "@brief Mounts the wind emitter to another scene object\n\n"

   "@param objectID Unique ID of the object wind emitter should attach to"
   
   "@tsexample\n"
   "// Wind emitter previously created and named %windEmitter\n"
   "// Going to attach it to the player, making him a walking wind storm\n"
   "%windEmitter.attachToObject(%player);\n"
   "@endtsexample\n\n")
{
	SceneObject *obj = dynamic_cast<SceneObject*>( Sim::findObject( objectID ) );
	if ( !obj )
		Con::warnf( "ForestWindEmitter::attachToObject - failed to find object with ID: %d", objectID );

	object->attachToObject( obj );
}