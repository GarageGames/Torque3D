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
#include "T3D/lightBase.h"

#include "console/consoleTypes.h"
#include "console/typeValidators.h"
#include "core/stream/bitStream.h"
#include "sim/netConnection.h"
#include "lighting/lightManager.h"
#include "lighting/shadowMap/lightShadowMap.h"
#include "scene/sceneRenderState.h"
#include "renderInstance/renderPassManager.h"
#include "console/engineAPI.h"
#include "gfx/gfxDrawUtil.h"

extern bool gEditingMission;

bool LightBase::smRenderViz = false;

IMPLEMENT_CONOBJECT( LightBase );

ConsoleDocClass( LightBase,
   "@brief This is the base class for light objects.\n\n"

   "It is *NOT* intended to be used directly in script, but exists to provide the base member variables "
   "and generic functionality. You should be using the derived classes PointLight and SpotLight, which "
   "can be declared in TorqueScript or added from the World Editor.\n\n"

   "For this class, we only add basic lighting options that all lighting systems would use. "
   "The specific lighting system options are injected at runtime by the lighting system itself.\n\n"

   "@see PointLight\n\n"
   "@see SpotLight\n\n"
   "@ingroup Lighting\n"
);

LightBase::LightBase()
   :  mIsEnabled( true ),
      mColor( ColorF::WHITE ),
      mBrightness( 1.0f ),
      mCastShadows( false ),
      mStaticRefreshFreq( 250 ),
      mDynamicRefreshFreq( 8 ),
      mPriority( 1.0f ),
      mAnimationData( NULL ),      
      mFlareData( NULL ),
      mFlareScale( 1.0f )
{
   mNetFlags.set( Ghostable | ScopeAlways );
   mTypeMask = LightObjectType;

   mLight = LightManager::createLightInfo();

   mFlareState.clear();
}

LightBase::~LightBase()
{
   SAFE_DELETE( mLight );
}

void LightBase::initPersistFields()
{
   // We only add the basic lighting options that all lighting
   // systems would use... the specific lighting system options
   // are injected at runtime by the lighting system itself.

   addGroup( "Light" );
      
      addField( "isEnabled", TypeBool, Offset( mIsEnabled, LightBase ), "Enables/Disables the object rendering and functionality in the scene." );
      addField( "color", TypeColorF, Offset( mColor, LightBase ), "Changes the base color hue of the light." );
      addField( "brightness", TypeF32, Offset( mBrightness, LightBase ), "Adjusts the lights power, 0 being off completely." );      
      addField( "castShadows", TypeBool, Offset( mCastShadows, LightBase ), "Enables/disabled shadow casts by this light." );
      addField( "staticRefreshFreq", TypeS32, Offset( mStaticRefreshFreq, LightBase ), "static shadow refresh rate (milliseconds)" );
      addField( "dynamicRefreshFreq", TypeS32, Offset( mDynamicRefreshFreq, LightBase ), "dynamic shadow refresh rate (milliseconds)", AbstractClassRep::FieldFlags::FIELD_HideInInspectors);
      addField( "priority", TypeF32, Offset( mPriority, LightBase ), "Used for sorting of lights by the light manager. "
		  "Priority determines if a light has a stronger effect than, those with a lower value" );

   endGroup( "Light" );

   addGroup( "Light Animation" );

      addField( "animate", TypeBool, Offset( mAnimState.active, LightBase ), "Toggles animation for the light on and off" );
      addField( "animationType", TYPEID< LightAnimData >(), Offset( mAnimationData, LightBase ), "Datablock containing light animation information (LightAnimData)" );
      addFieldV( "animationPeriod", TypeF32, Offset( mAnimState.animationPeriod, LightBase ), &CommonValidators::PositiveNonZeroFloat, "The length of time in seconds for a single playback of the light animation (must be > 0)" );
      addField( "animationPhase", TypeF32, Offset( mAnimState.animationPhase, LightBase ), "The phase used to offset the animation start time to vary the animation of nearby lights." );      

   endGroup( "Light Animation" );

   addGroup( "Misc" );

      addField( "flareType", TYPEID< LightFlareData >(), Offset( mFlareData, LightBase ), "Datablock containing light flare information (LightFlareData)" );
      addField( "flareScale", TypeF32, Offset( mFlareScale, LightBase ), "Globally scales all features of the light flare" );

   endGroup( "Misc" );

   // Now inject any light manager specific fields.
   LightManager::initLightFields();

   // We do the parent fields at the end so that
   // they show up that way in the inspector.
   Parent::initPersistFields();

   Con::addVariable( "$Light::renderViz", TypeBool, &smRenderViz,
      "Toggles visualization of light object's radius or cone.\n"
	   "@ingroup Lighting");

   Con::addVariable( "$Light::renderLightFrustums", TypeBool, &LightShadowMap::smDebugRenderFrustums,
      "Toggles rendering of light frustums when the light is selected in the editor.\n\n"
      "@note Only works for shadow mapped lights.\n\n"
      "@ingroup Lighting" );
}

bool LightBase::onAdd()
{
   if ( !Parent::onAdd() )
      return false;

   // Update the light parameters.
   _conformLights();
	addToScene();

   return true;
}

void LightBase::onRemove()
{
   removeFromScene();
   Parent::onRemove();
}

void LightBase::submitLights( LightManager *lm, bool staticLighting )
{
   if ( !mIsEnabled || staticLighting )
      return;

   if (  mAnimState.active && 
         mAnimState.animationPeriod > 0.0f &&
         mAnimationData )
   {
      mAnimState.brightness = mBrightness;
      mAnimState.transform = getRenderTransform();
      mAnimState.color = mColor;

      mAnimationData->animate( mLight, &mAnimState );
   }

   lm->registerGlobalLight( mLight, this );
}

void LightBase::inspectPostApply()
{
   // We do not call the parent here as it 
   // will call setScale() and screw up the 
   // real sizing fields on the light.

   // Ok fine... then we must set MountedMask ourself.

   _conformLights();
   setMaskBits( EnabledMask | UpdateMask | TransformMask | DatablockMask | MountedMask );
}

void LightBase::setTransform( const MatrixF &mat )
{
   setMaskBits( TransformMask );
   Parent::setTransform( mat );
}

void LightBase::prepRenderImage( SceneRenderState *state )
{
   if ( mIsEnabled && mFlareData )
   {
      mFlareState.fullBrightness = mBrightness;
      mFlareState.scale = mFlareScale;
      mFlareState.lightInfo = mLight;
      mFlareState.lightMat = getRenderTransform();

      mFlareData->prepRender( state, &mFlareState );
   }

   if ( !state->isDiffusePass() )
      return;

   const bool isSelectedInEditor = ( gEditingMission && isSelected() );

   // If the light is selected or light visualization
   // is enabled then register the callback.
   if ( smRenderViz || isSelectedInEditor )
   {
      ObjectRenderInst *ri = state->getRenderPass()->allocInst<ObjectRenderInst>();
      ri->renderDelegate.bind( this, &LightBase::_onRenderViz );
      ri->type = RenderPassManager::RIT_Editor;
      state->getRenderPass()->addInst( ri );
   }
}

void LightBase::_onRenderViz( ObjectRenderInst *ri, 
                              SceneRenderState *state, 
                              BaseMatInstance *overrideMat )
{
   if ( overrideMat )
      return;
   
   _renderViz( state );
}

void LightBase::_onSelected()
{
   #ifdef TORQUE_DEBUG
   // Enable debug rendering on the light.
   if( isClientObject() )
      mLight->enableDebugRendering( true );
   #endif

   Parent::_onSelected();
}

void LightBase::_onUnselected()
{
   #ifdef TORQUE_DEBUG
   // Disable debug rendering on the light.
   if( isClientObject() )
      mLight->enableDebugRendering( false );
   #endif

   Parent::_onUnselected();
}

void LightBase::interpolateTick( F32 delta )
{
}

void LightBase::processTick()
{
}

void LightBase::advanceTime( F32 timeDelta )
{
   if ( isMounted() )
   {
      MatrixF mat( true );
      mMount.object->getRenderMountTransform( 0.f, mMount.node, mMount.xfm, &mat );
      mLight->setTransform( mat );
      Parent::setTransform( mat );
   }
}

U32 LightBase::packUpdate( NetConnection *conn, U32 mask, BitStream *stream )
{
   U32 retMask = Parent::packUpdate( conn, mask, stream );

   stream->writeFlag( mIsEnabled );

   if ( stream->writeFlag( mask & TransformMask ) )
      stream->writeAffineTransform( mObjToWorld );

   if ( stream->writeFlag( mask & UpdateMask ) )
   {
      stream->write( mColor );
      stream->write( mBrightness );

      stream->writeFlag( mCastShadows );
      stream->write(mStaticRefreshFreq);
      stream->write(mDynamicRefreshFreq);

      stream->write( mPriority );      

      mLight->packExtended( stream ); 

      stream->writeFlag( mAnimState.active );
      stream->write( mAnimState.animationPeriod );
      stream->write( mAnimState.animationPhase );
      stream->write( mFlareScale );
   }

   if ( stream->writeFlag( mask & DatablockMask ) )
   {
      if ( stream->writeFlag( mAnimationData ) )
      {
         stream->writeRangedU32( mAnimationData->getId(),
                                 DataBlockObjectIdFirst, 
                                 DataBlockObjectIdLast );
      }

      if ( stream->writeFlag( mFlareData ) )
      {
         stream->writeRangedU32( mFlareData->getId(),
                                 DataBlockObjectIdFirst, 
                                 DataBlockObjectIdLast );
      }
   }

   return retMask;
}

void LightBase::unpackUpdate( NetConnection *conn, BitStream *stream )
{
   Parent::unpackUpdate( conn, stream );

   mIsEnabled = stream->readFlag();

   if ( stream->readFlag() ) // TransformMask
      stream->readAffineTransform( &mObjToWorld );

   if ( stream->readFlag() ) // UpdateMask
   {   
      stream->read( &mColor );
      stream->read( &mBrightness );      
      mCastShadows = stream->readFlag();
      stream->read(&mStaticRefreshFreq);
      stream->read(&mDynamicRefreshFreq);

      stream->read( &mPriority );      
      
      mLight->unpackExtended( stream );

      mAnimState.active = stream->readFlag();
      stream->read( &mAnimState.animationPeriod );
      stream->read( &mAnimState.animationPhase );
      stream->read( &mFlareScale );
   }

   if ( stream->readFlag() ) // DatablockMask
   {
      if ( stream->readFlag() )
      {
         SimObjectId id = stream->readRangedU32( DataBlockObjectIdFirst, DataBlockObjectIdLast );  
         LightAnimData *datablock = NULL;
         
         if ( Sim::findObject( id, datablock ) )
            mAnimationData = datablock;
         else
         {
            conn->setLastError( "Light::unpackUpdate() - invalid LightAnimData!" );
            mAnimationData = NULL;
         }
      }
      else
         mAnimationData = NULL;

      if ( stream->readFlag() )
      {
         SimObjectId id = stream->readRangedU32( DataBlockObjectIdFirst, DataBlockObjectIdLast );  
         LightFlareData *datablock = NULL;

         if ( Sim::findObject( id, datablock ) )
            mFlareData = datablock;
         else
         {
            conn->setLastError( "Light::unpackUpdate() - invalid LightCoronaData!" );
            mFlareData = NULL;
         }
      }
      else
         mFlareData = NULL;
   }
   
   if ( isProperlyAdded() )
      _conformLights();
}

void LightBase::setLightEnabled( bool enabled )
{
   if ( mIsEnabled != enabled )
   {
      mIsEnabled = enabled;
      setMaskBits( EnabledMask );
   }
}

DefineEngineMethod( LightBase, setLightEnabled, void, ( bool state ),,
   "@brief Toggles the light on and off\n\n"
   
   "@param state Turns the light on (true) or off (false)\n"

   "@tsexample\n"
   "// Disable the light\n"
   "CrystalLight.setLightEnabled(false);\n\n"
   "// Renable the light\n"
   "CrystalLight.setLightEnabled(true);\n"
   
   "@endtsexample\n\n"
)
{
  object->setLightEnabled( state );
}

//ConsoleMethod( LightBase, setLightEnabled, void, 3, 3, "( bool enabled )\t"
//   "Toggles the light on and off." )
//{
//   object->setLightEnabled( dAtob( argv[2] ) );
//}

static ConsoleDocFragment _lbplayAnimation1(
	"@brief Plays the light animation assigned to this light with the existing LightAnimData datablock.\n\n"
   
   "@tsexample\n"
   "// Play the animation assigned to this light\n"
   "CrystalLight.playAnimation();\n"
   "@endtsexample\n\n",
   "LightBase",
   "void playAnimation();"
);
static ConsoleDocFragment _lbplayAnimation2(
   "@brief Plays the light animation on this light using a new LightAnimData. If no LightAnimData "
   "is passed the existing one is played.\n\n"
   "@param anim Name of the LightAnimData datablock to be played\n\n"
   "@tsexample\n"
   "// Play the animation using a new LightAnimData datablock\n"
   "CrystalLight.playAnimation(SubtlePulseLightAnim);\n"
   "@endtsexample\n\n",
   "LightBase",
   "void playAnimation(LightAnimData anim);"
);

DefineConsoleMethod( LightBase, playAnimation, void, (const char * anim), (""), "( [LightAnimData anim] )\t"
   "Plays a light animation on the light.  If no LightAnimData is passed the "
   "existing one is played."
   "@hide")
{
	if ( String::isEmpty(anim) )
    {
        object->playAnimation();
        return;
    }

    LightAnimData *animData;
    if ( !Sim::findObject( anim, animData ) )
    {
        Con::errorf( "LightBase::playAnimation() - Invalid LightAnimData '%s'.", anim );
        return;
    }

    // Play Animation.
    object->playAnimation( animData );
}

void LightBase::playAnimation( void )
{
    if ( !mAnimState.active )
    {
        mAnimState.active = true;
        setMaskBits( UpdateMask );
    }
}

void LightBase::playAnimation( LightAnimData *animData )
{
    // Play Animation.
    playAnimation();

    // Update Datablock?
    if ( mAnimationData != animData )
    {
        mAnimationData = animData;
        setMaskBits( DatablockMask );
    }
}

DefineConsoleMethod( LightBase, pauseAnimation, void, (), , "Stops the light animation." )
{
    object->pauseAnimation();
}

void LightBase::pauseAnimation( void )
{
    if ( mAnimState.active )
    {
        mAnimState.active = false;
        setMaskBits( UpdateMask );
    }
}
