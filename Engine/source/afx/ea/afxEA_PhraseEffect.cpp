
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

#include "console/compiler.h"
#include "T3D/player.h"

#include "afx/afxEffectDefs.h"
#include "afx/afxPhrase.h"
#include "afx/afxEffectWrapper.h"
#include "afx/afxChoreographer.h"
#include "afx/ce/afxPhraseEffect.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxEA_PhraseEffect 

class afxEA_PhraseEffect : public afxEffectWrapper
{
  typedef afxEffectWrapper Parent;

  afxPhraseEffectData*  phrase_fx_data;
  Vector<afxPhrase*>*   active_phrases;
  U32                   last_trigger_mask;

  Vector<afxPhrase*>    _phrases_a;
  Vector<afxPhrase*>    _phrases_b;

  void                  grab_constraint_triggers(U32& trigger_mask);
  void                  grab_player_triggers(U32& trigger_mask);

  void                  do_runtime_substitutions();
  void                  trigger_new_phrase();
  void                  update_active_phrases(F32 dt);
  void                  cleanup_finished_phrases();

public:
  /*C*/                 afxEA_PhraseEffect();
  /*D*/                 ~afxEA_PhraseEffect();

  virtual void          ea_set_datablock(SimDataBlock*);
  virtual bool          ea_start();
  virtual bool          ea_update(F32 dt);
  virtual void          ea_finish(bool was_stopped);

  virtual bool          ea_is_enabled() { return true; }
};

//~~~~~~~~~~~~~~~~~~~~//

afxEA_PhraseEffect::afxEA_PhraseEffect()
{
  phrase_fx_data = 0;
  active_phrases = &_phrases_a;
}

afxEA_PhraseEffect::~afxEA_PhraseEffect()
{
  if (phrase_fx_data && phrase_fx_data->isTempClone())
    delete phrase_fx_data;
  phrase_fx_data = 0;

  for (S32 i = 0; i < _phrases_a.size(); i++)
  {
    if (_phrases_a[i])
      delete _phrases_a[i];
  }

  for (S32 i = 0; i < _phrases_b.size(); i++)
  {
    if (_phrases_b[i])
      delete _phrases_b[i];
  }
}

void afxEA_PhraseEffect::ea_set_datablock(SimDataBlock* db)
{
  phrase_fx_data = dynamic_cast<afxPhraseEffectData*>(db);
}

bool afxEA_PhraseEffect::ea_start()
{
  if (!phrase_fx_data)
  {
    Con::errorf("afxEA_PhraseEffect::ea_start() -- missing or incompatible datablock.");
    return false;
  }

  //last_trigger_mask = choreographer->getTriggerMask();
  last_trigger_mask = 0xffffffff;

  return true;
}

void afxEA_PhraseEffect::grab_constraint_triggers(U32& trigger_mask)
{
  afxConstraint* pos_cons = getPosConstraint();
  if (pos_cons)
    trigger_mask |= pos_cons->getTriggers();
}

void afxEA_PhraseEffect::grab_player_triggers(U32& trigger_mask)
{
  afxConstraint* pos_cons = getPosConstraint();
  Player* player = (pos_cons) ? dynamic_cast<Player*>(pos_cons->getSceneObject()) : 0;
  if (player)
  {
    if (player->isClientObject())
      trigger_mask |= player->getClientEventTriggers();
    else
      trigger_mask |= player->getServerEventTriggers();
  }
}

bool afxEA_PhraseEffect::ea_update(F32 dt)
{
  if (fade_value >= 1.0f)
  {
    //
    // Choreographer Triggers:
    //    These triggers can come from the choreographer owning this effect.
    //    They must be set explicitly by calls to afxChoreographer
    //    console-methods, setTriggerBit(), or clearTriggerBit().
    //
    U32 trigger_mask = (phrase_fx_data->no_choreographer_trigs) ? 0 : choreographer->getTriggerMask();

    //
    // Constraint Triggers:
    //    These triggers can come from the position contraint if it is:
    //      -- a TSStatic or ShapeBase derived object with dts triggers. 
    //      -- a trigger producing effect such as afxModel.
    //
    if (!phrase_fx_data->no_cons_trigs)
      grab_constraint_triggers(trigger_mask);

    //
    // Player Triggers:
    //    These triggers can come from the position contraint if it is
    //    a Player or Player-derived object.
    //
    if (!phrase_fx_data->no_player_trigs)
      grab_player_triggers(trigger_mask);

    // any change in the triggers?
    if (trigger_mask != last_trigger_mask)
    {
      if (phrase_fx_data->phrase_type == afxPhraseEffectData::PHRASE_CONTINUOUS)
      {
        bool last_state, new_state;
        if (phrase_fx_data->match_type == afxPhraseEffectData::MATCH_ANY)
        {
          last_state = ((last_trigger_mask & phrase_fx_data->trigger_mask) != 0);
          new_state = ((trigger_mask & phrase_fx_data->trigger_mask) != 0);
        }
        else
        {
          last_state = ((last_trigger_mask & phrase_fx_data->trigger_mask) == phrase_fx_data->trigger_mask);
          new_state = ((trigger_mask & phrase_fx_data->trigger_mask) == phrase_fx_data->trigger_mask);
        }
        if (new_state != last_state)
        {
          bool state_on = phrase_fx_data->match_state & afxPhraseEffectData::STATE_ON;
          if (new_state == state_on) // start trigger
          {
            trigger_new_phrase();
          }
          else // stop trigger
          {
            for (S32 i = 0; i < active_phrases->size(); i++)
            {
              (*active_phrases)[i]->stop(life_elapsed);
            }          
          }
        }
     }
      else // if (phrase_fx_data->phrase_type == afxPhraseEffectData::PHRASE_TRIGGERED)
      {
        bool did_trigger = false;
        U32 changed_bits = (last_trigger_mask ^ trigger_mask);

        if ((phrase_fx_data->match_state & afxPhraseEffectData::STATE_ON) != 0)
        {
          // check for trigger bits that just switched to on state
          U32 changed_on_bits = (changed_bits & trigger_mask);
          if (phrase_fx_data->match_type == afxPhraseEffectData::MATCH_ANY)
            did_trigger = ((changed_on_bits & phrase_fx_data->trigger_mask) != 0);
          else
            did_trigger = ((changed_on_bits & phrase_fx_data->trigger_mask) == phrase_fx_data->trigger_mask);
        }

        if (!did_trigger && ((phrase_fx_data->match_state & afxPhraseEffectData::STATE_OFF) != 0))
        {
          // check for trigger bits that just switched to off state
          U32 changed_off_bits = (changed_bits & last_trigger_mask);
          if (phrase_fx_data->match_type == afxPhraseEffectData::MATCH_ANY)
            did_trigger = ((changed_off_bits & phrase_fx_data->trigger_mask) != 0);
          else
            did_trigger = ((changed_off_bits & phrase_fx_data->trigger_mask) == phrase_fx_data->trigger_mask);
        }

        if (did_trigger)
          trigger_new_phrase();
      }

      last_trigger_mask = trigger_mask;
    }
  }

  update_active_phrases(dt);

  cleanup_finished_phrases();

  return true;
}

void afxEA_PhraseEffect::ea_finish(bool was_stopped)
{
  for (S32 i = 0; i < active_phrases->size(); i++)
  {
    (*active_phrases)[i]->stop(life_elapsed);
  }
}

void afxEA_PhraseEffect::do_runtime_substitutions()
{
  // only clone the datablock if there are substitutions
  if (phrase_fx_data->getSubstitutionCount() > 0)
  {
    // clone the datablock and perform substitutions
    afxPhraseEffectData* orig_db = phrase_fx_data;
    phrase_fx_data = new afxPhraseEffectData(*orig_db, true);
    orig_db->performSubstitutions(phrase_fx_data, choreographer, group_index);
  }
}

void afxEA_PhraseEffect::trigger_new_phrase()
{
  //afxPhrase* phrase = new afxPhrase(choreographer->isServerObject(), /*willStop=*/false);
  bool will_stop = phrase_fx_data->phrase_type == afxPhraseEffectData::PHRASE_CONTINUOUS;
  afxPhrase* phrase = new afxPhrase(choreographer->isServerObject(), will_stop);
  phrase->init(phrase_fx_data->fx_list, datablock->ewd_timing.lifetime, choreographer, time_factor, phrase_fx_data->n_loops, group_index);
  phrase->start(0, 0);
  if (phrase->isEmpty())
  {
    delete phrase;
    return;
  }

  if (phrase_fx_data->on_trig_cmd != ST_NULLSTRING)
  {
    char obj_str[32];
    dStrcpy(obj_str, Con::getIntArg(choreographer->getId()));

    char index_str[32];
    dStrcpy(index_str, Con::getIntArg(group_index));

    char buffer[1024];
    char* b = buffer;
    const char* v = phrase_fx_data->on_trig_cmd;
    while (*v != '\0')
    {
      if (v[0] == '%' && v[1] == '%')
      {
        const char* s = obj_str;
        while (*s != '\0')
        {
          b[0] = s[0];
          b++;
          s++;
        }
        v += 2;
      }
      else if (v[0] == '#' && v[1] == '#')
      {
        const char* s = index_str;
        while (*s != '\0')
        {
          b[0] = s[0];
          b++;
          s++;
        }
        v += 2;
      }
      else
      {
        b[0] = v[0];
        b++; 
        v++;
      }
    }
    b[0] = '\0';

    Compiler::gSyntaxError = false;
    //Con::errorf("EVAL [%s]", avar("%s;", buffer));
    Con::evaluate(avar("%s;", buffer), false, 0);
    if (Compiler::gSyntaxError)
    {
      Con::errorf("onTriggerCommand \"%s\" -- syntax error", phrase_fx_data->on_trig_cmd);
      Compiler::gSyntaxError = false;
    }
  }

  active_phrases->push_back(phrase);
}

void afxEA_PhraseEffect::update_active_phrases(F32 dt)
{
  for (S32 i = 0; i < active_phrases->size(); i++)
  {
    afxPhrase* phrase = (*active_phrases)[i];
    if (phrase->expired(life_elapsed))
      phrase->recycle(life_elapsed);
    phrase->update(dt, life_elapsed);
  }
}

void afxEA_PhraseEffect::cleanup_finished_phrases()
{
  Vector<afxPhrase*>* surviving_phrases = (active_phrases == &_phrases_a) ? &_phrases_b : &_phrases_a;
  
  surviving_phrases->clear();
  for (S32 i = 0; i < active_phrases->size(); i++)
  {
    afxPhrase* phrase = (*active_phrases)[i];
    if (!phrase->isEmpty())
      surviving_phrases->push_back(phrase);
    else
      delete phrase;
  }  

  active_phrases->clear();
  active_phrases = surviving_phrases;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

class afxEA_PhraseEffectDesc : public afxEffectAdapterDesc, public afxEffectDefs 
{
  static afxEA_PhraseEffectDesc desc;

public:
  virtual bool  testEffectType(const SimDataBlock*) const;
  virtual bool  requiresStop(const afxEffectWrapperData*, const afxEffectTimingData&) const;
  virtual bool  runsOnServer(const afxEffectWrapperData*) const { return true; }
  virtual bool  runsOnClient(const afxEffectWrapperData*) const { return true; }

  virtual afxEffectWrapper* create() const { return new afxEA_PhraseEffect; }
};

afxEA_PhraseEffectDesc afxEA_PhraseEffectDesc::desc;

bool afxEA_PhraseEffectDesc::testEffectType(const SimDataBlock* db) const
{
  return (typeid(afxPhraseEffectData) == typeid(*db));
}

bool afxEA_PhraseEffectDesc::requiresStop(const afxEffectWrapperData* ew, const afxEffectTimingData& timing) const
{
  return (timing.lifetime < 0);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//