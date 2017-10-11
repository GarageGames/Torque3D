
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

#ifndef _AFX_PHRASE_EFFECT_H_
#define _AFX_PHRASE_EFFECT_H_

#include "console/typeValidators.h"

#include "afx/ce/afxComponentEffect.h"
#include "afx/afxEffectDefs.h"
#include "afx/afxEffectWrapper.h"
#include "afx/afxPhrase.h"

class afxPhraseEffectData : public GameBaseData, public afxEffectDefs, public afxComponentEffectData
{
  typedef GameBaseData  Parent;

  class ewValidator : public TypeValidator
  {
    U32 id;
  public:
    ewValidator(U32 id) { this->id = id; }
    void validateType(SimObject *object, void *typePtr);
  };

  bool            do_id_convert;

public:
  enum MatchType {
    MATCH_ANY = 0,
    MATCH_ALL = 1
  };
  enum StateType {
    STATE_ON = 1,
    STATE_OFF = 2,
    STATE_ON_AND_OFF = STATE_ON | STATE_OFF
  };
  enum PhraseType
  {
    PHRASE_TRIGGERED = 0,
    PHRASE_CONTINUOUS = 1
  };

public:
  afxEffectList fx_list;
  F32           duration;
  S32           n_loops;
  U32           trigger_mask;
  U32           match_type;
  U32           match_state;
  U32           phrase_type;

  bool          no_choreographer_trigs;
  bool          no_cons_trigs;
  bool          no_player_trigs;

  StringTableEntry   on_trig_cmd;

  afxEffectBaseData* dummy_fx_entry;
  
private:
  void            pack_fx(BitStream* stream, const afxEffectList& fx, bool packed);
  void            unpack_fx(BitStream* stream, afxEffectList& fx);

public:
  /*C*/           afxPhraseEffectData();
  /*C*/           afxPhraseEffectData(const afxPhraseEffectData&, bool = false);

  virtual void    reloadReset();

  virtual bool    onAdd();
  virtual void    packData(BitStream*);
  virtual void    unpackData(BitStream*);

  bool            preload(bool server, String &errorStr);

  virtual void    gather_cons_defs(Vector<afxConstraintDef>& defs);

  virtual bool    allowSubstitutions() const { return true; }

  static void     initPersistFields();

  DECLARE_CONOBJECT(afxPhraseEffectData);
  DECLARE_CATEGORY("AFX");
};

typedef afxPhraseEffectData::MatchType afxPhraseEffect_MatchType;
DefineEnumType( afxPhraseEffect_MatchType );

typedef afxPhraseEffectData::StateType afxPhraseEffect_StateType;
DefineEnumType( afxPhraseEffect_StateType );

typedef afxPhraseEffectData::PhraseType afxPhraseEffect_PhraseType;
DefineEnumType( afxPhraseEffect_PhraseType );

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#endif // _AFX_PHRASE_EFFECT_H_
