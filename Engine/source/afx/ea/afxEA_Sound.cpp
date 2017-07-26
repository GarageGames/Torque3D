
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

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxEA_Sound 

class afxEA_Sound : public afxEffectWrapper
{
  typedef afxEffectWrapper Parent;

  SFXProfile*   sound_prof;
  SFXDescription* sound_desc;
  SFXSource*    sound_handle;

  void          do_runtime_substitutions();

public:
  /*C*/         afxEA_Sound();
  /*D*/         ~afxEA_Sound();

  virtual void  ea_set_datablock(SimDataBlock*);
  virtual bool  ea_start();
  virtual bool  ea_update(F32 dt);
  virtual void  ea_finish(bool was_stopped);

  virtual bool  ea_is_enabled() { return true; }

  virtual void  onDeleteNotify(SimObject*);
};

//~~~~~~~~~~~~~~~~~~~~//

afxEA_Sound::afxEA_Sound()
{
  sound_prof = 0;
  sound_desc = 0;
  sound_handle = 0;
}

afxEA_Sound::~afxEA_Sound()
{
  if (sound_prof && sound_prof->isTempClone())
    delete sound_prof;
  sound_prof = 0;
  sound_desc = 0;
  sound_handle = 0;
}

void afxEA_Sound::ea_set_datablock(SimDataBlock* db)
{
  sound_prof = dynamic_cast<SFXProfile*>(db);
  if (sound_prof)
    sound_desc = sound_prof->getDescription();
}

bool afxEA_Sound::ea_start()
{
  if (!sound_prof)
  {
    Con::errorf("afxEA_Sound::ea_start() -- missing or incompatible AudioProfile.");
    return false;
  }
  if (!sound_desc)
  {
    Con::errorf("afxEA_Sound::ea_start() -- missing or incompatible AudioDescriptor.");
    return false;
  }

  do_runtime_substitutions();

  return true;
}

bool afxEA_Sound::ea_update(F32 dt)
{
  if (!sound_handle)
  {
    sound_handle = SFX->createSource(sound_prof, &updated_xfm, 0);
    if (sound_handle)
        sound_handle->play();
  }

  if (sound_handle)
  {
    sound_handle->setTransform(updated_xfm);
    sound_handle->setVolume((in_scope) ? updated_scale.x*fade_value : 0.0f);
	  deleteNotify(sound_handle);
  }

  return true;
}

void afxEA_Sound::ea_finish(bool was_stopped)
{
  if (sound_handle)
  {
    sound_handle->stop();
    SFX_DELETE(sound_handle);
  }
}

void afxEA_Sound::do_runtime_substitutions()
{
  sound_prof = sound_prof->cloneAndPerformSubstitutions(choreographer, group_index);
  sound_desc = sound_prof->getDescription();
}

void afxEA_Sound::onDeleteNotify(SimObject* obj)
{
  if (sound_handle == dynamic_cast<SFXSource*>(obj))
    sound_handle = 0;

  Parent::onDeleteNotify(obj);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

class afxEA_SoundDesc : public afxEffectAdapterDesc, public afxEffectDefs 
{
  static afxEA_SoundDesc desc;

public:
  virtual bool  testEffectType(const SimDataBlock*) const;
  virtual bool  requiresStop(const afxEffectWrapperData*, const afxEffectTimingData&) const;
  virtual bool  runsOnServer(const afxEffectWrapperData*) const { return false; }
  virtual bool  runsOnClient(const afxEffectWrapperData*) const { return true; }
  virtual void  prepEffect(afxEffectWrapperData*) const;

  virtual afxEffectWrapper* create() const { return new afxEA_Sound; }
};

afxEA_SoundDesc afxEA_SoundDesc::desc;

bool afxEA_SoundDesc::testEffectType(const SimDataBlock* db) const
{
  // dynamic cast used here so that both SFXProfile and AudioProfile match
  return (dynamic_cast<const SFXProfile*>(db) != 0);
}

bool afxEA_SoundDesc::requiresStop(const afxEffectWrapperData* ew, const afxEffectTimingData& timing) const
{
  SFXDescription* desc = ((SFXProfile*)ew->effect_data)->getDescription();
  return (desc && desc->mIsLooping) ? (timing.lifetime < 0) : false;
}

void afxEA_SoundDesc::prepEffect(afxEffectWrapperData* ew) const 
{ 
  if (ew->ewd_timing.lifetime < 0)
  {
    SFXProfile* snd = (SFXProfile*) ew->effect_data;
    SFXDescription* desc = snd->getDescription();
    if (desc && !desc->mIsLooping)
    {
      static bool test_for_audio = true;
      static bool can_get_audio_len = false;

      if (test_for_audio)
      {
        can_get_audio_len = true;
        test_for_audio = false;
      }

      if (can_get_audio_len)
      {
        SFXResource* sfx_rsrc = snd->getResource();
        if (sfx_rsrc)
        {
          ew->ewd_timing.lifetime = 0.001f*sfx_rsrc->getDuration();
          //Con::printf("SFX (%s) duration=%g", snd->mFilename, timing.lifetime);
        }
      }
      else
      {
        ew->ewd_timing.lifetime = 0;
        Con::printf("afxEA_SoundDesc::prepEffect() -- cannot get audio length from sound file.");
      }
    }
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//