
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

#include "afxChoreographer.h"
#include "afxEffectVector.h"
#include "afxConstraint.h"
#include "afxEffectWrapper.h"
#include "afxEffectGroup.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

void afxEffectVector::filter_client_server()
{
  if (empty())
    return;

  for (S32 i = 0; i < mFX_v->size(); i++)
  {
    if ((*mFX_v)[i]->mDatablock->runsHere(mOn_server))
      mFX_v2->push_back((*mFX_v)[i]);
    else
    {
      delete (*mFX_v)[i];
      (*mFX_v)[i] = 0;
    }
  }

  swap_vecs();

  mFX_v2->clear();
}

void afxEffectVector::calc_fx_dur_and_afterlife()
{
  mTotal_fx_dur = 0.0f;
  mAfter_life = 0.0f;

  if (empty())
    return;

  for (S32 i = 0; i < mFX_v->size(); i++)
  {
    afxEffectWrapper* ew = (*mFX_v)[i];
    if (ew)
    {
      F32 ew_dur;
      if (ew->mEW_timing.lifetime < 0)
      {
        if (mPhrase_dur > ew->mEW_timing.delay)
          ew_dur = mPhrase_dur + ew->afterStopTime();
        else
          ew_dur = ew->mEW_timing.delay + ew->afterStopTime();
      }
      else
        ew_dur = ew->mEW_timing.delay + ew->mEW_timing.lifetime + ew->mEW_timing.fade_out_time;

      if (ew_dur > mTotal_fx_dur)
        mTotal_fx_dur = ew_dur;

      F32 after = ew->afterStopTime();
      if (after > mAfter_life)
        mAfter_life = after;
    }
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

afxEffectVector::afxEffectVector()
{
  mFX_v = 0;
  mFX_v2 = 0;
  mActive = false;
  mOn_server = false;
  mTotal_fx_dur = 0;
  mAfter_life = 0;
}

afxEffectVector::~afxEffectVector()
{
  stop(true);
  delete mFX_v;
  delete mFX_v2;
}

void afxEffectVector::effects_init(afxChoreographer* chor, afxEffectList& effects, bool will_stop, F32 time_factor, 
                                   S32 group_index, const afxGroupTimingData* group_timing)
{ 
  afxConstraintMgr* cons_mgr = chor->getConstraintMgr();

  for (S32 i = 0; i < effects.size(); i++)
  {
    if (dynamic_cast<afxEffectGroupData*>(effects[i]))
    {
      afxEffectGroupData* eg = (afxEffectGroupData*)effects[i];
      if (eg->getSubstitutionCount() > 0)
      {
        // clone the datablock and perform substitutions
        afxEffectGroupData* orig_db = eg;
        eg = new afxEffectGroupData(*orig_db, true);
        orig_db->performSubstitutions(eg, chor, group_index);
      }

      if (eg->group_enabled)
      {
        if (eg->assign_idx)
        {
          for (S32 j = 0; j < eg->group_count; j++)
            effects_init(chor, eg->fx_list, will_stop, time_factor, j+eg->idx_offset, &eg->timing);
        }
        else
        {
          for (S32 j = 0; j < eg->group_count; j++)
            effects_init(chor, eg->fx_list, will_stop, time_factor, group_index, &eg->timing);
        }
      }

      if (eg->isTempClone())
        delete eg;
    }
    else if (dynamic_cast<afxEffectWrapperData*>(effects[i]))
    {
      afxEffectWrapperData* ewd = (afxEffectWrapperData*)effects[i];

      if (ewd->getSubstitutionCount() > 0)
      {
        // clone the ewd and perform substitutions
        afxEffectWrapperData* orig_db = ewd;
        ewd = new afxEffectWrapperData(*orig_db, true);
        orig_db->performSubstitutions(ewd, chor, group_index);
      }

      if (ewd->effect_enabled)
      {
        static afxEffectTimingData inherited_timing;
        bool use_inherited_timing = false;
        if (ewd->inherit_timing != 0)
        {
          if (group_timing)
          {
            inherited_timing = ewd->ewd_timing;
            if ((ewd->inherit_timing & afxEffectDefs::TIMING_DELAY) != 0)
              inherited_timing.delay = group_timing->delay;
            if ((ewd->inherit_timing & afxEffectDefs::TIMING_LIFETIME) != 0)
              inherited_timing.lifetime = group_timing->lifetime;
            if ((ewd->inherit_timing & afxEffectDefs::TIMING_FADE_IN) != 0)
              inherited_timing.fade_in_time = group_timing->fade_in_time;
            if ((ewd->inherit_timing & afxEffectDefs::TIMING_FADE_OUT) != 0)
              inherited_timing.fade_out_time = group_timing->fade_out_time;
          }
          else
          {
            Con::warnf("afxEffectVector::effects_init() -- %s::inheritGroupTiming is non-zero but wrapper is not in a group.");
          }
        }

        const afxEffectTimingData& timing = (use_inherited_timing) ? inherited_timing : ewd->ewd_timing;

        if ( (will_stop || !ewd->requiresStop(timing)) && 
             (chor->testRanking(ewd->ranking_range.low, ewd->ranking_range.high)) &&
             (chor->testLevelOfDetail(ewd->lod_range.low, ewd->lod_range.high)) && 
             (ewd->testExecConditions(chor->getExecConditions()))
            )
        {
          afxEffectWrapper* effect;
          effect = afxEffectWrapper::ew_create(chor, ewd, cons_mgr, time_factor, group_index);
          if (effect)
            mFX_v->push_back(effect);
        }      
      }
      else
      {
        if (ewd->isTempClone())
          delete ewd;
      }

    }
  }
}

void afxEffectVector::ev_init(afxChoreographer* chor, afxEffectList& effects, bool on_server, 
                              bool will_stop, F32 time_factor, F32 phrase_dur, S32 group_index)
{
  mOn_server = on_server;
  mPhrase_dur = phrase_dur;

  mFX_v = new Vector<afxEffectWrapper*>;

  effects_init(chor, effects, will_stop, time_factor, group_index);

  mFX_v2 = new Vector<afxEffectWrapper*>(mFX_v->size());
}

void afxEffectVector::start(F32 timestamp)
{
  if (empty())
    return;

  // At this point both client and server effects are in the list.
  // Timing adjustments are made during prestart().
  for (S32 i = 0; i < mFX_v->size(); i++)
    (*mFX_v)[i]->prestart();

  // duration and afterlife values are pre-calculated here
  calc_fx_dur_and_afterlife();

  // now we filter out client-only or server-only effects that
  // don't belong here,
  filter_client_server();

  mActive = true;

  for (S32 j = 0; j < mFX_v->size(); j++)
  {
    if ((*mFX_v)[j]->start(timestamp))
      mFX_v2->push_back((*mFX_v)[j]);
    else
    {
      delete (*mFX_v)[j];
      (*mFX_v)[j] = 0;
    }
  }

  swap_vecs();
  mFX_v2->clear();
}

void afxEffectVector::update(F32 dt)
{
  if (empty())
  {
    mActive = false;
    return;
  }

  for (int i = 0; i < mFX_v->size(); i++)
  {
    (*mFX_v)[i]->update(dt);

    if ((*mFX_v)[i]->isDone() || (*mFX_v)[i]->isAborted())
    {
      // effect has ended, cleanup and delete
      (*mFX_v)[i]->cleanup();
      delete (*mFX_v)[i];
      (*mFX_v)[i] = 0;
    }
    else
    {
      // effect is still going, so keep it around
      mFX_v2->push_back((*mFX_v)[i]);
    }
  }

  swap_vecs();

  mFX_v2->clear();

  if (empty())
  {
    mActive = false;
    delete mFX_v; mFX_v =0;
    delete mFX_v2; mFX_v2 = 0;
  }
}

void afxEffectVector::stop(bool force_cleanup)
{
  if (empty())
  {
    mActive = false;
    return;
  }

  for (int i = 0; i < mFX_v->size(); i++)
  {
    (*mFX_v)[i]->stop();

    if (force_cleanup || (*mFX_v)[i]->deleteWhenStopped())
    {
      // effect is over when stopped, cleanup and delete 
      (*mFX_v)[i]->cleanup();
      delete (*mFX_v)[i];
      (*mFX_v)[i] = 0;
    }
    else
    {
      // effect needs to fadeout or something, so keep it around
		mFX_v2->push_back((*mFX_v)[i]);
    }
  }

  swap_vecs();

  mFX_v2->clear();

  if (empty())
  {
    mActive = false;
    delete mFX_v; mFX_v =0;
    delete mFX_v2; mFX_v2 = 0;
  }
}

void afxEffectVector::interrupt()
{
  if (empty())
  {
    mActive = false;
    return;
  }

  for (int i = 0; i < mFX_v->size(); i++)
  {
    (*mFX_v)[i]->stop();
    (*mFX_v)[i]->cleanup();
    delete (*mFX_v)[i];
    (*mFX_v)[i] = 0;
  }

  swap_vecs();

  mFX_v2->clear();

  if (empty())
  {
    mActive = false;
    delete mFX_v; mFX_v =0;
    delete mFX_v2; mFX_v2 = 0;
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//


