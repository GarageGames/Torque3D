
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

#ifndef _AFX_RPG_MAGIC_SPELL_H_
#define _AFX_RPG_MAGIC_SPELL_H_

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#include "core/util/tVector.h"
#include "T3D/gameBase/gameBase.h"
#include "console/typeValidators.h"

#include "afx/afxEffectDefs.h"
#include "afx/afxEffectWrapper.h"
#include "afx/afxMagicMissile.h"

class afxMagicMissileData;
class afxEffectWrapperData;
class SceneObject;

class afxRPGMagicSpellDefs
{
public:
  // Migrate this stuff to RPG Magic-System
  enum TargetType {
    TARGET_NOTHING,
    TARGET_SELF,
    TARGET_FRIEND,
    TARGET_ENEMY,
    TARGET_CORPSE,
    TARGET_FREE,
  };

  enum {
    MAX_REAGENTS_PER_SPELL = 8,
  };
};

typedef afxRPGMagicSpellDefs::TargetType afxRPGMagicSpell_TargetType;
DefineEnumType( afxRPGMagicSpell_TargetType );

class afxRPGMagicSpellData : public GameBaseData, public afxRPGMagicSpellDefs
{
  typedef GameBaseData Parent;

public:
  F32               casting_dur;
  StringTableEntry  spell_name;
  StringTableEntry  spell_desc;
  S32               spell_target;
  F32               spell_range;
  S32               mana_cost;
  U8                n_reagents;
  S8                reagent_cost[MAX_REAGENTS_PER_SPELL];
  StringTableEntry  reagent_name[MAX_REAGENTS_PER_SPELL];
  StringTableEntry  icon_name;
  StringTableEntry  source_pack;
  bool              is_placeholder;
  U8                free_target_style;
  bool              target_optional;

private:
  char*             fmt_placeholder_desc(char* buffer, int len) const;

public:
  /*C*/             afxRPGMagicSpellData();

  char*             formatDesc(char* buffer, int len) const;
  bool              requiresTarget() { return (spell_target == TARGET_ENEMY || spell_target == TARGET_CORPSE || spell_target == TARGET_FRIEND); }

  virtual bool      onAdd();
  virtual void      packData(BitStream*);
  virtual void      unpackData(BitStream*);

  static void       initPersistFields();

  DECLARE_CONOBJECT(afxRPGMagicSpellData);
  DECLARE_CATEGORY("AFX");
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#endif // _AFX_RPG_MAGIC_SPELL_H_
