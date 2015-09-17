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
#include "lightAnimData.h"

#include "console/consoleTypes.h"
#include "T3D/lightBase.h"
#include "math/mRandom.h"
#include "math/mathIO.h"
#include "T3D/gameBase/processList.h"
#include "core/stream/bitStream.h"


LightAnimData::LightAnimData()
{
}

LightAnimData::~LightAnimData()
{
}

IMPLEMENT_CO_DATABLOCK_V1( LightAnimData );

ConsoleDocClass( LightAnimData,
   "@brief A datablock which defines and performs light animation, such as rotation, brightness fade, and colorization.\n\n"

   "@tsexample\n"
   "datablock LightAnimData( SubtlePulseLightAnim )\n"
   "{\n"
   "   brightnessA = 0.5;\n"
   "   brightnessZ = 1;\n"
   "   brightnessPeriod = 1;\n"
   "   brightnessKeys = \"aza\";\n"
   "   brightnessSmooth = true;\n"
   "};\n"
   "@endtsexample\n\n"
   "@see LightBase\n\n"
   "@see LightDescription\n\n"
   "@ingroup FX\n"
   "@ingroup Lighting\n"
);

void LightAnimData::initPersistFields()
{
   addGroup( "Offset",
      "The XYZ translation animation state relative to the light position." );

      addField( "offsetA", TypeF32, Offset( mOffset.value1, LightAnimData ), 3,
         "The value of the A key in the keyframe sequence." );

      addField( "offsetZ", TypeF32, Offset( mOffset.value2, LightAnimData ), 3,
         "The value of the Z key in the keyframe sequence." );

      addField( "offsetPeriod", TypeF32, Offset( mOffset.period, LightAnimData ), 3,
         "The animation time for keyframe sequence." );

      addField( "offsetKeys", TypeString, Offset( mOffset.keys, LightAnimData ), 3,
         "The keyframe sequence encoded into a string where characters from A to Z define "
         "a position between the two animation values." );

      addField( "offsetSmooth", TypeBool, Offset( mOffset.smooth, LightAnimData ), 3,
         "If true the transition between keyframes will be smooth." );

   endGroup( "Offset" );

   addGroup( "Rotation",
      "The XYZ rotation animation state relative to the light orientation." );

      addField( "rotA", TypeF32, Offset( mRot.value1, LightAnimData ), 3,
         "The value of the A key in the keyframe sequence." );

      addField( "rotZ", TypeF32, Offset( mRot.value2, LightAnimData ), 3,
         "The value of the Z key in the keyframe sequence." );

      addField( "rotPeriod", TypeF32, Offset( mRot.period, LightAnimData ), 3,
         "The animation time for keyframe sequence." );

      addField( "rotKeys", TypeString, Offset( mRot.keys, LightAnimData ), 3,
         "The keyframe sequence encoded into a string where characters from A to Z define "
         "a position between the two animation values." );

      addField( "rotSmooth", TypeBool, Offset( mRot.smooth, LightAnimData ), 3,
         "If true the transition between keyframes will be smooth." );

   endGroup( "Rotation" );

   addGroup( "Color",
      "The RGB color animation state." );

      addField( "colorA", TypeF32, Offset( mColor.value1, LightAnimData ), 3,
         "The value of the A key in the keyframe sequence." );

      addField( "colorZ", TypeF32, Offset( mColor.value2, LightAnimData ), 3,
         "The value of the Z key in the keyframe sequence." );

      addField( "colorPeriod", TypeF32, Offset( mColor.period, LightAnimData ), 3,
         "The animation time for keyframe sequence." );

      addField( "colorKeys", TypeString, Offset( mColor.keys, LightAnimData ), 3,
         "The keyframe sequence encoded into a string where characters from A to Z define "
         "a position between the two animation values." );

      addField( "colorSmooth", TypeBool, Offset( mColor.smooth, LightAnimData ), 3,
         "If true the transition between keyframes will be smooth." );

   endGroup( "Color" );

   addGroup( "Brightness",
      "The brightness animation state." );

      addField( "brightnessA", TypeF32, Offset( mBrightness.value1, LightAnimData ),
         "The value of the A key in the keyframe sequence." );

      addField( "brightnessZ", TypeF32, Offset( mBrightness.value2, LightAnimData ),
         "The value of the Z key in the keyframe sequence." );

      addField( "brightnessPeriod", TypeF32, Offset( mBrightness.period, LightAnimData ),
         "The animation time for keyframe sequence." );

      addField( "brightnessKeys", TypeString, Offset( mBrightness.keys, LightAnimData ),
         "The keyframe sequence encoded into a string where characters from A to Z define "
         "a position between the two animation values." );

      addField( "brightnessSmooth", TypeBool, Offset( mBrightness.smooth, LightAnimData ),
         "If true the transition between keyframes will be smooth." );

   endGroup( "Brightness" );

   Parent::initPersistFields();
}

bool LightAnimData::preload( bool server, String &errorStr )
{
   if ( !Parent::preload( server, errorStr ) )
      return false;

   _updateKeys();

   return true;
}

void LightAnimData::inspectPostApply()
{
   Parent::inspectPostApply();
   _updateKeys();
}

void LightAnimData::_updateKeys()
{
   mOffset.updateKey();
   mRot.updateKey();
   mColor.updateKey();
   mBrightness.updateKey();
}

template<U32 COUNT>
void LightAnimData::AnimValue<COUNT>::updateKey()
{
   for ( U32 i=0; i < COUNT; i++ )
   {
      timeScale[i] = 0.0f;
      keyLen[i] = 0.0f;
      
      if ( keys[i] && keys[i][0] && period[i] > 0.0f )
      {
         keyLen[i] = dStrlen( keys[i] );
         timeScale[i] = (F32)( keyLen[i] - 1 ) / period[i];
      }
   }
}

template<U32 COUNT>
bool LightAnimData::AnimValue<COUNT>::animate( F32 time, F32 *output )
{
   F32 scaledTime, lerpFactor, valueRange, keyFrameLerp;
   U32 posFrom, posTo;
   S32 keyFrameFrom, keyFrameTo;
   F32 initialValue = *output;
   
   bool wasAnimated = false;

   for ( U32 i=0; i < COUNT; i++ )
   {
      if ( mIsZero( timeScale[i] ) )
         continue;

      wasAnimated = true;

      scaledTime = mFmod( time, period[i] ) * timeScale[i];

	   posFrom = mFloor( scaledTime );
	   posTo = mCeil( scaledTime );

      keyFrameFrom = dToupper( keys[i][posFrom] ) - 65;
      keyFrameTo = dToupper( keys[i][posTo] ) - 65;
	   valueRange = ( value2[i] - value1[i] ) / 25.0f;

      if ( !smooth[i] )
         output[i] = (value1[i] + (keyFrameFrom * valueRange)) * initialValue;
      else
      {
         lerpFactor = scaledTime - posFrom;
   	   keyFrameLerp = ( keyFrameTo - keyFrameFrom ) * lerpFactor;

         output[i] = (value1[i] + ((keyFrameFrom + keyFrameLerp) * valueRange)) * initialValue;
      }
   }

   return wasAnimated;
}

template<U32 COUNT>
void LightAnimData::AnimValue<COUNT>::write( BitStream *stream ) const
{
   for ( U32 i=0; i < COUNT; i++ )
   {
      stream->write( value1[i] );
      stream->write( value2[i] );
      stream->write( period[i] );
      stream->writeString( keys[i] );
   }
}

template<U32 COUNT>
void LightAnimData::AnimValue<COUNT>::read( BitStream *stream )
{
   for ( U32 i=0; i < COUNT; i++ )
   {
      stream->read( &value1[i] );
      stream->read( &value2[i] );
      stream->read( &period[i] );
      keys[i] = stream->readSTString();
   }
}

void LightAnimData::packData( BitStream *stream )
{
   Parent::packData( stream );

   mOffset.write( stream );
   mRot.write( stream );
   mColor.write( stream );
   mBrightness.write( stream );
}

void LightAnimData::unpackData( BitStream *stream )
{
   Parent::unpackData( stream );

   mOffset.read( stream );
   mRot.read( stream );
   mColor.read( stream );
   mBrightness.read( stream );
}

void LightAnimData::animate( LightInfo *lightInfo, LightAnimState *state )
{   
   PROFILE_SCOPE( LightAnimData_animate );

   // Calculate the input time for animation.
   F32 time =  state->animationPhase + 
               ( (F32)Sim::getCurrentTime() * 0.001f ) / 
               state->animationPeriod;

   MatrixF transform( state->transform );

   EulerF euler( Point3F::Zero );
   if ( mRot.animate( time, euler ) )
   {
      euler.x = mDegToRad( euler.x );
      euler.y = mDegToRad( euler.y );
      euler.z = mDegToRad( euler.z );
      MatrixF rot( euler );
      transform.mul( rot );
   }

   Point3F offset( Point3F::Zero );
   if ( mOffset.animate( time, offset ) )
      transform.displace( offset );

   lightInfo->setTransform( transform );

   ColorF color = state->color;
   mColor.animate( time, color );
   lightInfo->setColor( color );

   F32 brightness = state->brightness;
   mBrightness.animate( time, &brightness );
   lightInfo->setBrightness( brightness );
}
