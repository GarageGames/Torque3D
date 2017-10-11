
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
#include "afxRPGMagicSpell.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxRPGMagicSpellData

IMPLEMENT_CO_DATABLOCK_V1(afxRPGMagicSpellData);

ConsoleDocClass( afxRPGMagicSpellData,
   "@brief A datablock for defining RPG aspects of a spell.\n\n"

   "@ingroup afxMisc\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

afxRPGMagicSpellData::afxRPGMagicSpellData()
{
  // spell parameters
  spell_name = ST_NULLSTRING;
  spell_desc = ST_NULLSTRING;
  spell_target = TARGET_NOTHING;
  spell_range = 0.0f;
  mana_cost = 0;

  n_reagents = 0;
  for (S32 i = 0; i < MAX_REAGENTS_PER_SPELL; i++)
  {
    reagent_cost[i] = 1;
    reagent_name[i] = ST_NULLSTRING;
  }

  // spell phase timing
  casting_dur = 0.0f;

  // interface elements
  icon_name = ST_NULLSTRING;
  source_pack = ST_NULLSTRING;

  is_placeholder = false;

  free_target_style = 0;
  target_optional = false;
}

ImplementEnumType( afxRPGMagicSpell_TargetType, "Possible RPG spell target types.\n" "@ingroup afxRPGMagicSpell\n\n" )
   { afxRPGMagicSpellData::TARGET_NOTHING,     "nothing",   "..." },
   { afxRPGMagicSpellData::TARGET_SELF,        "self",      "..." },
   { afxRPGMagicSpellData::TARGET_FRIEND,      "friend",    "..." },
   { afxRPGMagicSpellData::TARGET_ENEMY,       "enemy",     "..." },
   { afxRPGMagicSpellData::TARGET_CORPSE,      "corpse",    "..." },
   { afxRPGMagicSpellData::TARGET_FREE,        "free",      "..." },
EndImplementEnumType;

#define myOffset(field) Offset(field, afxRPGMagicSpellData)

void afxRPGMagicSpellData::initPersistFields()
{
  // spell parameters
  addField("spellName",         TypeString,     myOffset(spell_name),
    "...");
  addField("desc",              TypeString,     myOffset(spell_desc),
    "...");

  addField("target", TYPEID< afxRPGMagicSpellData::TargetType >(), myOffset(spell_target),
    "...");

  addField("range",             TypeF32,        myOffset(spell_range),
    "...");
  addField("manaCost",          TypeS32,        myOffset(mana_cost),
    "...");
  addField("reagentCost",       TypeS8,         myOffset(reagent_cost), MAX_REAGENTS_PER_SPELL,
    "...");
  addField("reagentName",       TypeString,     myOffset(reagent_name), MAX_REAGENTS_PER_SPELL,
    "...");

  // spell phase timing
  addField("castingDur",        TypeF32,        myOffset(casting_dur));

  // interface elements
  addField("iconBitmap",        TypeFilename,   myOffset(icon_name));
  addField("sourcePack",        TypeString,     myOffset(source_pack));
  addField("isPlaceholder",     TypeBool,       myOffset(is_placeholder));
  addField("freeTargetStyle",   TypeS8,         myOffset(free_target_style));
  addField("targetOptional",    TypeBool,       myOffset(target_optional));

  Parent::initPersistFields();
}

bool afxRPGMagicSpellData::onAdd()
{
  if (Parent::onAdd() == false)
    return false;
  
  n_reagents = 0;
  for (S32 i = 0; i < MAX_REAGENTS_PER_SPELL && reagent_name[i] != ST_NULLSTRING; i++)
    n_reagents++;
  
  return true;
}

void afxRPGMagicSpellData::packData(BitStream* stream)
{
	Parent::packData(stream);

  stream->write(spell_target);
  stream->write(spell_range);
  stream->write(mana_cost);
  stream->write(n_reagents);
  for (S32 i = 0; i < n_reagents; i++)
  {
    stream->write(reagent_cost[i]);
    stream->writeString(reagent_name[i]);
  }

  stream->write(casting_dur);

  stream->writeString(spell_name);
  stream->writeLongString(511, spell_desc);
  stream->writeString(icon_name);
  stream->writeString(source_pack);
  stream->writeFlag(is_placeholder);
  stream->writeFlag(target_optional);
  stream->write(free_target_style);
}

void afxRPGMagicSpellData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);

  stream->read(&spell_target);
  stream->read(&spell_range);
  stream->read(&mana_cost);
  stream->read(&n_reagents); 
  for (S32 i = 0; i < n_reagents; i++)
  {
    stream->read(&reagent_cost[i]);
    reagent_name[i] = stream->readSTString();
  }

  stream->read(&casting_dur);

  spell_name = stream->readSTString();
  {
    char value[512];
    stream->readLongString(511, value);
    spell_desc = StringTable->insert(value);
  }
  icon_name = stream->readSTString();
  source_pack = stream->readSTString();
  is_placeholder = stream->readFlag();
  target_optional = stream->readFlag();
  stream->read(&free_target_style);
}

#define NAME_FMT      "<just:left><font:Arial:20><color:FFD200>"
#define TARGET_FMT    "<just:right><font:Arial:20><color:ACACAC>"
#define MANACOST_FMT  "<just:left><font:Arial:16><color:FFFFFF>"
#define RANGE_FMT     "<just:right><font:Arial:16><color:FFFFFF>"
#define CASTLEN_FMT   "<just:left><font:Arial:16><color:FFFFFF>"
#define DESC_FMT      "<just:left><font:Arial:16><color:ACACAC>"
#define SMALL_BR      "<font:Arial:4>\n"
#define BR            "\n"
#define PACK_FMT      "<just:right><font:Arial:14><color:ACACAC>"
#define PACK_NAME_FMT "<color:FFD200>"


char* afxRPGMagicSpellData::fmt_placeholder_desc(char* buffer, int len) const
{
  char pack_str[32]; 
  if (source_pack == ST_NULLSTRING)
    dStrcpy(pack_str, "unknown");
  else
    dSprintf(pack_str, 32, "%s", source_pack);

  dSprintf(buffer, len, 
          NAME_FMT "%s" BR
          SMALL_BR
          DESC_FMT "%s" BR
          SMALL_BR SMALL_BR
          PACK_FMT "source: " PACK_NAME_FMT "%s",
          spell_name, spell_desc, pack_str);

  return buffer;
}

char* afxRPGMagicSpellData::formatDesc(char* buffer, int len) const
{
  if (is_placeholder)
    return fmt_placeholder_desc(buffer, len);

  char target_str[32]; target_str[0] = '\0';

  // CAUTION: The following block of code is fragile and tricky since it depends
  // on the underlying structures defined by ImplementEnumType/EndImplementEnumType.
  // This done so the enum strings can be used directly in the spell description text.
  for (unsigned int i = 0; i < _afxRPGMagicSpell_TargetType::_sEnumTable.getNumValues(); i++)
  {
    if (_afxRPGMagicSpell_TargetType::_sEnumTable[i].mInt == spell_target)
    {
      if (spell_target != TARGET_NOTHING)
      {
        dStrcpy(target_str, _afxRPGMagicSpell_TargetType::_sEnumTable[i].mName);
        if (spell_target != TARGET_FREE && target_optional)
          dStrcat(target_str, " (opt)");
      }
      break;
    }
  }


  char range_str[32]; range_str[0] = '\0';
  if (spell_range > 0)
  {
    if (spell_range == ((F32)((S32)spell_range)))
      dSprintf(range_str, 32, "%d meter range", (S32) spell_range);
    else
      dSprintf(range_str, 32, "%.1f meter range", spell_range);
  }

  char casting_str[32];
  if (casting_dur <= 0)
    dStrcpy(casting_str, "instant");
  else
    dSprintf(casting_str, 32, "%.1f sec cast", casting_dur);

  char pack_str[32]; 
  if (source_pack == ST_NULLSTRING)
    dStrcpy(pack_str, "unknown");
  else
    dSprintf(pack_str, 32, "%s", source_pack);

  dSprintf(buffer, len, 
          NAME_FMT "%s" TARGET_FMT "%s" BR
          SMALL_BR
          MANACOST_FMT "%d Mana" RANGE_FMT "%s" BR
          CASTLEN_FMT "%s" BR
          SMALL_BR SMALL_BR
          DESC_FMT "%s" BR SMALL_BR
          PACK_FMT "source: " PACK_NAME_FMT "%s",
          spell_name, target_str, 
          mana_cost, range_str, 
          casting_str, 
          spell_desc, pack_str);

  return buffer;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
