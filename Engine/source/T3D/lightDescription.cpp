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
#include "T3D/lightDescription.h"

#include "lighting/lightManager.h"
#include "T3D/lightFlareData.h"
#include "T3D/lightAnimData.h"
#include "core/stream/bitStream.h"
#include "lighting/lightInfo.h"
#include "console/engineAPI.h"


LightDescription::LightDescription()
 : color( ColorF::WHITE ),
   brightness( 1.0f ),
   range( 5.0f ),
   castShadows( false ),
   mStaticRefreshFreq( 250 ),
   mDynamicRefreshFreq( 8 ),
   animationData( NULL ),
   animationDataId( 0 ),
   animationPeriod( 1.0f ),
   animationPhase( 1.0f ),
   flareData( NULL ),
   flareDataId( 0 ),
   flareScale( 1.0f )
{   
}

LightDescription::~LightDescription()
{

}

IMPLEMENT_CO_DATABLOCK_V1( LightDescription );

ConsoleDocClass( LightDescription,
   "@brief A helper datablock used by classes (such as shapebase) "
   "that submit lights to the scene but do not use actual \"LightBase\" objects.\n\n"

   "This datablock stores the properties of that light as fields that can be initialized from script."

   "@tsexample\n"
   "// Declare a light description to be used on a rocket launcher projectile\n"
   "datablock LightDescription(RocketLauncherLightDesc)\n"
   "{\n"
   "   range = 4.0;\n"
   "   color = \"1 1 0\";\n"
   "   brightness = 5.0;\n"
   "   animationType = PulseLightAnim;\n"
   "   animationPeriod = 0.25;\n"
   "};\n\n"
   "// Declare a ProjectileDatablock which uses the light description\n"
   "datablock ProjectileData(RocketLauncherProjectile)\n"
   "{\n"
   "   lightDesc = RocketLauncherLightDesc;\n\n"
   "   projectileShapeName = \"art/shapes/weapons/SwarmGun/rocket.dts\";\n\n"
   "   directDamage = 30;\n"
   "   radiusDamage = 30;\n"
   "   damageRadius = 5;\n"
   "   areaImpulse = 2500;\n\n"
   "   // ... remaining ProjectileData fields not listed for this example\n"
   "};\n"
   "@endtsexample\n\n"

   "@see LightBase\n\n"
   "@ingroup Lighting\n"
);

void LightDescription::initPersistFields()
{
   addGroup( "Light" );

      addField( "color", TypeColorF, Offset( color, LightDescription ), "Changes the base color hue of the light." );
      addField( "brightness", TypeF32, Offset( brightness, LightDescription ), "Adjusts the lights power, 0 being off completely." );      
      addField( "range", TypeF32, Offset( range, LightDescription ), "Controls the size (radius) of the light" );
      addField( "castShadows", TypeBool, Offset( castShadows, LightDescription ), "Enables/disabled shadow casts by this light." );
      addField( "staticRefreshFreq", TypeS32, Offset( mStaticRefreshFreq, LightDescription ), "static shadow refresh rate (milliseconds)" );
      addField( "dynamicRefreshFreq", TypeS32, Offset( mDynamicRefreshFreq, LightDescription ), "dynamic shadow refresh rate (milliseconds)" );

   endGroup( "Light" );

   addGroup( "Light Animation" );

      addField( "animationType", TYPEID< LightAnimData >(), Offset( animationData, LightDescription ), "Datablock containing light animation information (LightAnimData)" );
      addField( "animationPeriod", TypeF32, Offset( animationPeriod, LightDescription ), "The length of time in seconds for a single playback of the light animation" );
      addField( "animationPhase", TypeF32, Offset( animationPhase, LightDescription ), "The phase used to offset the animation start time to vary the animation of nearby lights." );

   endGroup( "Light Animation" );

   addGroup( "Misc" );

      addField( "flareType", TYPEID< LightFlareData >(), Offset( flareData, LightDescription ), "Datablock containing light flare information (LightFlareData)" );
      addField( "flareScale", TypeF32, Offset( flareScale, LightDescription ), "Globally scales all features of the light flare" );

   endGroup( "Misc" );

   LightManager::initLightFields();

   Parent::initPersistFields();
}

void LightDescription::inspectPostApply()
{
   Parent::inspectPostApply();

   // Hack to allow changing properties in game.
   // Do the same work as preload.
   animationData = NULL;
   flareData = NULL;

   String str;
   _preload( false, str );
}

bool LightDescription::onAdd()
{
   if ( !Parent::onAdd() )
      return false;

   return true;
}
bool LightDescription::preload( bool server, String &errorStr )
{
   if ( !Parent::preload( server, errorStr ) )
      return false;

   return _preload( server, errorStr );  
}

void LightDescription::packData( BitStream *stream )
{
   Parent::packData( stream );

   stream->write( color );
   stream->write( brightness );
   stream->write( range );
   stream->writeFlag( castShadows );
   stream->write(mStaticRefreshFreq);
   stream->write(mDynamicRefreshFreq);

   stream->write( animationPeriod );
   stream->write( animationPhase );
   stream->write( flareScale );

   if ( stream->writeFlag( animationData ) )
   {
      stream->writeRangedU32( animationData->getId(),
         DataBlockObjectIdFirst, 
         DataBlockObjectIdLast );
   }

   if ( stream->writeFlag( flareData ) )
   {
      stream->writeRangedU32( flareData->getId(),
         DataBlockObjectIdFirst, 
         DataBlockObjectIdLast );
   }
}

void LightDescription::unpackData( BitStream *stream )
{
   Parent::unpackData( stream );

   stream->read( &color );
   stream->read( &brightness );     
   stream->read( &range );
   castShadows = stream->readFlag();
   stream->read(&mStaticRefreshFreq);
   stream->read(&mDynamicRefreshFreq);

   stream->read( &animationPeriod );
   stream->read( &animationPhase );
   stream->read( &flareScale );
   
   if ( stream->readFlag() )   
      animationDataId = stream->readRangedU32( DataBlockObjectIdFirst, DataBlockObjectIdLast );  

   if ( stream->readFlag() )
      flareDataId = stream->readRangedU32( DataBlockObjectIdFirst, DataBlockObjectIdLast );  
}

void LightDescription::submitLight( LightState *state, const MatrixF &xfm, LightManager *lm, SimObject *object )
{
   LightInfo *li = state->lightInfo;
   
   li->setRange( range );
   li->setColor( color );
   li->setCastShadows( castShadows );
   li->setStaticRefreshFreq(mStaticRefreshFreq);
   li->setDynamicRefreshFreq(mDynamicRefreshFreq);
   li->setTransform( xfm );

   if ( animationData )
   {      
      LightAnimState *animState = &state->animState;   

      animState->brightness = brightness;
      animState->transform = xfm;
      animState->color = color;
      animState->animationPeriod = animationPeriod;
      animState->animationPhase = animationPhase;      

      animationData->animate( li, animState );
   }

   lm->registerGlobalLight( li, object );
}

void LightDescription::prepRender( SceneRenderState *sceneState, LightState *lightState, const MatrixF &xfm )
{
   if ( flareData )
   {
      LightFlareState *flareState = &lightState->flareState;
      flareState->fullBrightness = brightness;
      flareState->scale = flareScale;
      flareState->lightMat = xfm;
      flareState->lightInfo = lightState->lightInfo;

      flareData->prepRender( sceneState, flareState );
   }
}

bool LightDescription::_preload( bool server, String &errorStr )
{
   if (!animationData && animationDataId != 0)
      if (Sim::findObject(animationDataId, animationData) == false)
         Con::errorf(ConsoleLogEntry::General, "LightDescription::onAdd: Invalid packet, bad datablockId(animationData): %d", animationDataId);

   if (!flareData && flareDataId != 0)
      if (Sim::findObject(flareDataId, flareData) == false)
         Con::errorf(ConsoleLogEntry::General, "LightDescription::onAdd: Invalid packet, bad datablockId(flareData): %d", flareDataId); 

   return true;
}

DefineEngineMethod( LightDescription, apply, void, (),,
   "@brief Force an inspectPostApply call for the benefit of tweaking via the console\n\n"
   
   "Normally this functionality is only exposed to objects via the World Editor, once changes have been made. "
   "Exposing apply to script allows you to make changes to it on the fly without the World Editor.\n\n"

   "@note This is intended for debugging and tweaking, not for game play\n\n"

   "@tsexample\n"
   "// Change a property of the light description\n"
   "RocketLauncherLightDesc.brightness = 10;\n\n"
   "// Make it so\n"
   "RocketLauncherLightDesc.apply();\n"
   
   "@endtsexample\n\n"
)
{
  object->inspectPostApply();
}

//ConsoleMethod( LightDescription, apply, void, 2, 2, "force an inspectPostApply for the benefit of tweaking via the console" )
//{
//   object->inspectPostApply();
//}