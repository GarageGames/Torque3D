
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

#include <typeinfo>
#include "afx/arcaneFX.h"

#include "sfx/sfxSystem.h"
#include "sfx/sfxSource.h"
#include "sfx/sfxProfile.h"

#include "afx/afxEffectDefs.h"
#include "afx/afxEffectWrapper.h"
#include "afx/afxChoreographer.h"
#include "afx/ce/afxAudioBank.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxEA_AudioBank 

class afxEA_AudioBank : public afxEffectWrapper
{
  typedef afxEffectWrapper Parent;

  SFXSource*    sound_handle;
  afxAudioBank* sound_bank; 

  S32           play_index;

  void          do_runtime_substitutions();

public:
  /*C*/         afxEA_AudioBank();
  /*D*/         ~afxEA_AudioBank();

  virtual void  ea_set_datablock(SimDataBlock*);
  virtual bool  ea_start();
  virtual bool  ea_update(F32 dt);
  virtual void  ea_finish(bool was_stopped);
};

//~~~~~~~~~~~~~~~~~~~~//

afxEA_AudioBank::afxEA_AudioBank()
{
  sound_handle = 0;
  sound_bank = 0;
  play_index = -1;
}

afxEA_AudioBank::~afxEA_AudioBank()
{
  if (sound_bank && sound_bank->isTempClone())
    delete sound_bank;
  sound_bank = 0;
  sound_handle = 0;
}

void afxEA_AudioBank::ea_set_datablock(SimDataBlock* db)
{
  sound_bank = dynamic_cast<afxAudioBank*>(db);
}

bool afxEA_AudioBank::ea_start()
{
  if (!sound_bank)
  {
    Con::errorf("afxEA_AudioBank::ea_start() -- missing or incompatible afxAudioBank.");
    return false;
  }

  do_runtime_substitutions();

  if (sound_bank->mPath == ST_NULLSTRING)
    return false;

  play_index = sound_bank->play_index;

  return true;
}

bool afxEA_AudioBank::ea_update(F32 dt)
{
  if (!sound_handle)
  {
    if (play_index >= 0 && play_index < 32 && sound_bank->mFilenames[play_index] != ST_NULLSTRING)
    {
      SFXProfile* temp_prof = new SFXProfile(sound_bank->mDescriptionObject, 
                                      avar("%s/%s", sound_bank->mPath, sound_bank->mFilenames[play_index]), 
                                      true);
      if (!temp_prof->registerObject())
      {
         Con::errorf( "afxEA_AudioBank::ea_update() -- unable to create profile");
         delete temp_prof;
      }
      else
      {
         sound_handle = SFX->createSource(temp_prof);
         if (sound_handle)
           sound_handle->play();
         temp_prof->setAutoDelete(true);
      }
    }
  }

  if (sound_handle)
  {
    sound_handle->setTransform(updated_xfm);
    sound_handle->setVolume(updated_scale.x*fade_value);  
  }

  return true;
}

void afxEA_AudioBank::ea_finish(bool was_stopped)
{
  if (sound_handle)
  {
    sound_handle->stop();
    SFX_DELETE(sound_handle);
  }
}

void afxEA_AudioBank::do_runtime_substitutions()
{
  sound_bank = sound_bank->cloneAndPerformSubstitutions(choreographer, group_index);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

class afxEA_SoundBankDesc : public afxEffectAdapterDesc, public afxEffectDefs 
{
  static afxEA_SoundBankDesc desc;

public:
  virtual bool  testEffectType(const SimDataBlock*) const;
  virtual bool  requiresStop(const afxEffectWrapperData*, const afxEffectTimingData&) const;
  virtual bool  runsOnServer(const afxEffectWrapperData*) const { return false; }
  virtual bool  runsOnClient(const afxEffectWrapperData*) const { return true; }
  //virtual void  prepEffect(afxEffectWrapperData*) const;

  virtual afxEffectWrapper* create() const { return new afxEA_AudioBank; }
};

afxEA_SoundBankDesc afxEA_SoundBankDesc::desc;

bool afxEA_SoundBankDesc::testEffectType(const SimDataBlock* db) const
{
  return (typeid(afxAudioBank) == typeid(*db));
}

bool afxEA_SoundBankDesc::requiresStop(const afxEffectWrapperData* ew, const afxEffectTimingData& timing) const
{
  SFXDescription* desc = ((SFXProfile*)ew->effect_data)->getDescription();
  return (desc && desc->mIsLooping) ? (timing.lifetime < 0) : false;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//