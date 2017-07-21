
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// Arcane-FX for MIT Licensed Open Source version of Torque 3D from GarageGames
// Copyright (C) 2015 Faust Logic, Inc.
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
//
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#include "afx/arcaneFX.h"

#include "console/consoleTypes.h"
#include "core/stream/bitStream.h"
#include "T3D/lightAnimData.h"

#include "afx/ce/afxLightBase_T3D.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxT3DLightBaseData

IMPLEMENT_CO_DATABLOCK_V1(afxT3DLightBaseData);

ConsoleDocClass( afxT3DLightBaseData,
   "@brief A datablock baseclass for afxT3DPointLightData and afxT3DSpotLightData.\n\n"

   "Not intended to be used directly, afxT3DLightBaseData exists to provide base member variables and generic functionality "
   "for the derived classes afxT3DPointLightData and afxT3DSpotLightData."
   "\n\n"

   "@see afxT3DPointLightData\n\n"
   "@see afxT3DSpotLightData\n\n"
   "@see PointLight\n\n"
   "@see SpotLight\n\n"

   "@ingroup afxEffects\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

afxT3DLightBaseData::afxT3DLightBaseData()
   :  mIsEnabled( true ),
      mColor( ColorF::WHITE ),
      mBrightness( 1.0f ),
      mCastShadows( false ),
      mPriority( 1.0f ),
      mAnimationData( NULL ),
      mFlareData( NULL ),
      mFlareScale( 1.0f )
{

  mLocalRenderViz = false;

  // marked true if datablock ids need to
  // be converted into pointers
  do_id_convert = false;
}

afxT3DLightBaseData::afxT3DLightBaseData(const afxT3DLightBaseData& other, bool temp_clone) : GameBaseData(other, temp_clone)
{
  mIsEnabled = other.mIsEnabled;
  mColor = other.mColor;
  mBrightness = other.mBrightness;
  mCastShadows = other.mCastShadows;
  mPriority = other.mPriority;
  mAnimationData = other.mAnimationData;
  mAnimState = other.mAnimState;
  mFlareData = other.mFlareData;
  mFlareScale = other.mFlareScale;

  mLocalRenderViz = other.mLocalRenderViz;

  do_id_convert = other.do_id_convert;
}

//
// NOTE: keep this as consistent as possible with LightBase::initPersistFields()
//
void afxT3DLightBaseData::initPersistFields()
{
   // We only add the basic lighting options that all lighting
   // systems would use... the specific lighting system options
   // are injected at runtime by the lighting system itself.

   addGroup( "Light" );

      addField( "isEnabled", TypeBool, Offset( mIsEnabled, afxT3DLightBaseData ),
        "Enables/Disables the object rendering and functionality in the scene.");
      addField( "color", TypeColorF, Offset( mColor, afxT3DLightBaseData ),
        "Changes the base color hue of the light.");
      addField( "brightness", TypeF32, Offset( mBrightness, afxT3DLightBaseData ),
        "Adjusts the lights power, 0 being off completely.");
      addField( "castShadows", TypeBool, Offset( mCastShadows, afxT3DLightBaseData ),
        "Enables/disables shadow casts by this light.");
      addField( "priority", TypeF32, Offset( mPriority, afxT3DLightBaseData ),
        "Used for sorting of lights by the light manager. Priority determines if a light "
        "has a stronger effect than, those with a lower value");
      addField( "localRenderViz", TypeBool, Offset( mLocalRenderViz, afxT3DLightBaseData ),
        "Enables/disables a semi-transparent geometry to help visualize the light's "
        "range and placement.");

   endGroup( "Light" );

   addGroup( "Light Animation" );

      addField( "animate", TypeBool, Offset( mAnimState.active, afxT3DLightBaseData ),
        "Toggles animation for the light on and off");
      addField( "animationType", TYPEID<LightAnimData>(), Offset( mAnimationData, afxT3DLightBaseData ),
        "Datablock containing light animation information (LightAnimData)");
      addField( "animationPeriod", TypeF32, Offset( mAnimState.animationPeriod, afxT3DLightBaseData ),
        "The length of time in seconds for a single playback of the light animation");
      addField( "animationPhase", TypeF32, Offset( mAnimState.animationPhase, afxT3DLightBaseData ),
        "The phase used to offset the animation start time to vary the animation of "
        "nearby lights.");

   endGroup( "Light Animation" );

   addGroup( "Misc" );

      addField( "flareType", TYPEID<LightFlareData>(), Offset( mFlareData, afxT3DLightBaseData ),
        "Datablock containing light flare information (LightFlareData)");
      addField( "flareScale", TypeF32, Offset( mFlareScale, afxT3DLightBaseData ),
        "Globally scales all features of the light flare");

   endGroup( "Misc" );

   /*
   // Now inject any light manager specific fields.
   LightManager::initLightFields();
   */

   // We do the parent fields at the end so that
   // they show up that way in the inspector.
   Parent::initPersistFields();
}

bool afxT3DLightBaseData::onAdd()
{
  if (Parent::onAdd() == false)
    return false;

  return true;
}

void afxT3DLightBaseData::packData(BitStream* stream)
{
	Parent::packData(stream);

  // note: BitStream's overloaded write() for ColorF will convert
  // to ColorI for transfer and then back to ColorF. This is fine
  // for most color usage but for lighting colors we want to preserve
  // "pushed" color values which may be greater than 1.0 so the color
  // is instead sent as individual color primaries.
  stream->write( mColor.red );
  stream->write( mColor.green );
  stream->write( mColor.blue );

  stream->write( mBrightness );
  stream->writeFlag( mCastShadows );
  stream->write( mAnimState.animationPeriod );
  stream->write( mAnimState.animationPhase );
  stream->write( mFlareScale );

  writeDatablockID(stream, mAnimationData, packed);
  writeDatablockID(stream, mFlareData, packed);
}

void afxT3DLightBaseData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);

  stream->read( &mColor.red );
  stream->read( &mColor.green );
  stream->read( &mColor.blue );
  mColor.alpha = 1.0f;

  stream->read( &mBrightness );
  mCastShadows = stream->readFlag();
  stream->read( &mAnimState.animationPeriod );
  stream->read( &mAnimState.animationPhase );
  stream->read( &mFlareScale );

  mAnimationData = (LightAnimData*) readDatablockID(stream);
  mFlareData = (LightFlareData*) readDatablockID(stream);

  do_id_convert = true;
}

bool afxT3DLightBaseData::preload(bool server, String &errorStr)
{
  if (!Parent::preload(server, errorStr))
    return false;

  // Resolve objects transmitted from server
  if (!server)
  {
    if (do_id_convert)
    {
      SimObjectId anim_id = SimObjectId((uintptr_t)mAnimationData);
      if (anim_id != 0)
      {
        // try to convert id to pointer
        if (!Sim::findObject(anim_id, mAnimationData))
        {
          Con::errorf(ConsoleLogEntry::General,
            "afxT3DLightBaseData::preload() -- bad datablockId: 0x%x (animationType)",
            anim_id);
        }
      }
      SimObjectId flare_id = SimObjectId((uintptr_t)mFlareData);
      if (flare_id != 0)
      {
        // try to convert id to pointer
        if (!Sim::findObject(flare_id, mFlareData))
        {
          Con::errorf(ConsoleLogEntry::General,
            "afxT3DLightBaseData::preload() -- bad datablockId: 0x%x (flareType)",
            flare_id);
        }
      }
      do_id_convert = false;
    }
  }

  return true;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
