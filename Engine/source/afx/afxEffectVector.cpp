
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

  for (S32 i = 0; i < fx_v->size(); i++)
  {
    if ((*fx_v)[i]->datablock->runsHere(on_server))
      fx_v2->push_back((*fx_v)[i]);
    else
    {
      delete (*fx_v)[i];
      (*fx_v)[i] = 0;
    }
  }

  swap_vecs();

  fx_v2->clear();
}

void afxEffectVector::calc_fx_dur_and_afterlife()
{
  total_fx_dur = 0.0f;
  after_life = 0.0f;

  if (empty())
    return;

  for (S32 i = 0; i < fx_v->size(); i++)
  {
    afxEffectWrapper* ew = (*fx_v)[i];
    if (ew)
    {
      F32 ew_dur;
      if (ew->ew_timing.lifetime < 0)
      {
        if (phrase_dur > ew->ew_timing.delay)
          ew_dur = phrase_dur + ew->afterStopTime();
        else
          ew_dur = ew->ew_timing.delay + ew->afterStopTime();
      }
      else
        ew_dur = ew->ew_timing.delay + ew->ew_timing.lifetime + ew->ew_timing.fade_out_time;

      if (ew_dur > total_fx_dur)
        total_fx_dur = ew_dur;

      F32 after = ew->afterStopTime();
      if (after > after_life)
        after_life = after;
    }
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

afxEffectVector::afxEffectVector()
{
  fx_v = 0;
  fx_v2 = 0;
  active = false;
  on_server = false;
  total_fx_dur = 0;
  after_life = 0;
}

afxEffectVector::~afxEffectVector()
{
  stop(true);
  delete fx_v;
  delete fx_v2;
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
            fx_v->push_back(effect);
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
  this->on_server = on_server;
  this->phrase_dur = phrase_dur;

  fx_v = new Vector<afxEffectWrapper*>;

  effects_init(chor, effects, will_stop, time_factor, group_index);

  fx_v2 = new Vector<afxEffectWrapper*>(fx_v->size());
}

void afxEffectVector::start(F32 timestamp)
{
  if (empty())
    return;

  // At this point both client and server effects are in the list.
  // Timing adjustments are made during prestart().
  for (S32 i = 0; i < fx_v->size(); i++)
    (*fx_v)[i]->prestart();

  // duration and afterlife values are pre-calculated here
  calc_fx_dur_and_afterlife();

  // now we filter out client-only or server-only effects that
  // don't belong here,
  filter_client_server();

  active = true;

  for (S32 j = 0; j < fx_v->size(); j++)
  {
    if ((*fx_v)[j]->start(timestamp))
      fx_v2->push_back((*fx_v)[j]);
    else
    {
      delete (*fx_v)[j];
      (*fx_v)[j] = 0;
    }
  }

  swap_vecs();
  fx_v2->clear();
}

void afxEffectVector::update(F32 dt)
{
  if (empty())
  {
    active = false;
    return;
  }

  for (int i = 0; i < fx_v->size(); i++)
  {
    (*fx_v)[i]->update(dt);

    if ((*fx_v)[i]->isDone() || (*fx_v)[i]->isAborted())
    {
      // effect has ended, cleanup and delete
      (*fx_v)[i]->cleanup();
      delete (*fx_v)[i];
      (*fx_v)[i] = 0;
    }
    else
    {
      // effect is still going, so keep it around
      fx_v2->push_back((*fx_v)[i]);
    }
  }

  swap_vecs();

  fx_v2->clear();

  if (empty())
  {
    active = false;
    delete fx_v; fx_v =0;
    delete fx_v2; fx_v2 = 0;
  }
}

void afxEffectVector::stop(bool force_cleanup)
{
  if (empty())
  {
    active = false;
    return;
  }

  for (int i = 0; i < fx_v->size(); i++)
  {
    (*fx_v)[i]->stop();

    if (force_cleanup || (*fx_v)[i]->deleteWhenStopped())
    {
      // effect is over when stopped, cleanup and delete 
      (*fx_v)[i]->cleanup();
      delete (*fx_v)[i];
      (*fx_v)[i] = 0;
    }
    else
    {
      // effect needs to fadeout or something, so keep it around
      fx_v2->push_back((*fx_v)[i]);
    }
  }

  swap_vecs();

  fx_v2->clear();

  if (empty())
  {
    active = false;
    delete fx_v; fx_v =0;
    delete fx_v2; fx_v2 = 0;
  }
}

void afxEffectVector::interrupt()
{
  if (empty())
  {
    active = false;
    return;
  }

  for (int i = 0; i < fx_v->size(); i++)
  {
    (*fx_v)[i]->stop();
    (*fx_v)[i]->cleanup();
    delete (*fx_v)[i];
    (*fx_v)[i] = 0;
  }

  swap_vecs();

  fx_v2->clear();

  if (empty())
  {
    active = false;
    delete fx_v; fx_v =0;
    delete fx_v2; fx_v2 = 0;
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//


