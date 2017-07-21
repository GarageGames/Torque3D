
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

#include "console/engineAPI.h"
#include "console/consoleTypes.h"
#include "core/stream/bitStream.h"
#include "T3D/gameBase/gameBase.h"

#include "afx/afxSpellBook.h"
#include "afx/afxMagicSpell.h"
#include "afx/rpg/afxRPGMagicSpell.h"
#include "afx/ui/afxSpellButton.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxSpellBookData

IMPLEMENT_CO_DATABLOCK_V1(afxSpellBookData);

ConsoleDocClass( afxSpellBookData,
   "@brief A spellbook datablock.\n\n"

   "@ingroup afxMisc\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

afxSpellBookData::afxSpellBookData()
{
  spells_per_page = 12;
  pages_per_book = 12;
  dMemset(spells, 0, sizeof(spells));
  dMemset(rpg_spells, 0, sizeof(rpg_spells));

  // marked true if datablock ids need to
  // be converted into pointers
  do_id_convert = false;
}

#define myOffset(field) Offset(field, afxSpellBookData)

void afxSpellBookData::initPersistFields()
{
  addField("spellsPerPage",  TypeS8,                myOffset(spells_per_page),
    "...");
  addField("pagesPerBook",   TypeS8,                myOffset(pages_per_book),
    "...");

  addField("spells",       TYPEID<GameBaseData>(),   myOffset(spells),     MAX_PAGES_PER_BOOK*MAX_SPELLS_PER_PAGE,
    "...");
  addField("rpgSpells",    TYPEID<GameBaseData>(),   myOffset(rpg_spells), MAX_PAGES_PER_BOOK*MAX_SPELLS_PER_PAGE,
    "...");

  Parent::initPersistFields();
}

bool afxSpellBookData::preload(bool server, String &errorStr)
{
  if (!Parent::preload(server, errorStr))
    return false;

  // Resolve objects transmitted from server
  if (!server)
  {
    if (do_id_convert)
    {
      for (S32 i = 0; i < pages_per_book*spells_per_page; i++)
      {
        SimObjectId db_id = SimObjectId((uintptr_t)rpg_spells[i]);
        if (db_id != 0)
        {
          // try to convert id to pointer
          if (!Sim::findObject(db_id, rpg_spells[i]))
          {
            Con::errorf(ConsoleLogEntry::General,
              "afxSpellBookData::preload() -- bad datablockId: 0x%x (afxRPGMagicSpellData)",
              db_id);
          }
        }
      }
      do_id_convert = false;
    }
  }

  return true;
}

void afxSpellBookData::packData(BitStream* stream)
{
	Parent::packData(stream);

  stream->write(spells_per_page);
  stream->write(pages_per_book);

  for (S32 i = 0; i < pages_per_book*spells_per_page; i++)
    writeDatablockID(stream, rpg_spells[i], packed);
}

void afxSpellBookData::unpackData(BitStream* stream)
{
  Parent::unpackData(stream);

  stream->read(&spells_per_page);
  stream->read(&pages_per_book);

  do_id_convert = true;
  for (S32 i = 0; i < pages_per_book*spells_per_page; i++)
    rpg_spells[i] = (afxRPGMagicSpellData*) readDatablockID(stream);
}

DefineEngineMethod(afxSpellBookData, getPageSlotIndex, S32, (Point2I bookSlot),,
                   "...\n\n"
                   "@ingroup AFX")
{
  return object->getPageSlotIndex(bookSlot.x, bookSlot.y);
}

DefineEngineMethod(afxSpellBookData, getCapacity, S32, (),,
                   "Get the capacity (total number of spell slots) in a spellbook.\n\n"
                   "@ingroup AFX")
{
  return object->spells_per_page*object->pages_per_book;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// afxSpellBook

IMPLEMENT_CO_NETOBJECT_V1(afxSpellBook);

ConsoleDocClass( afxSpellBook,
   "@brief A spellbook object.\n\n"

   "@ingroup afxMisc\n"
   "@ingroup AFX\n"
);

afxSpellBook::afxSpellBook()
{
	mNetFlags.set(Ghostable | ScopeAlways);
	mDataBlock = NULL;
  all_spell_cooldown = 1.0f;
}

afxSpellBook::~afxSpellBook()
{
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

void afxSpellBook::initPersistFields()
{
	Parent::initPersistFields();
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

void afxSpellBook::processTick(const Move* m)
{
	Parent::processTick(m);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

void afxSpellBook::advanceTime(F32 dt)
{
  Parent::advanceTime(dt);

  if (all_spell_cooldown < 1.0f)
  {
    all_spell_cooldown += dt/2.0f;
    if (all_spell_cooldown > 1.0f)
      all_spell_cooldown = 1.0f;
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

bool afxSpellBook::onNewDataBlock(GameBaseData* dptr, bool reload)
{
  mDataBlock = dynamic_cast<afxSpellBookData*>(dptr);
  if (!mDataBlock || !Parent::onNewDataBlock(dptr, reload))
    return false;

  scriptOnNewDataBlock();

  return true;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

bool afxSpellBook::onAdd()
{
	if (!Parent::onAdd())
    return(false);

	return(true);
}

void afxSpellBook::onRemove()
{
	Parent::onRemove();
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//

U32 afxSpellBook::packUpdate(NetConnection * con, U32 mask, BitStream * stream)
{
  U32 retMask = Parent::packUpdate(con, mask, stream);

  if (stream->writeFlag(mask & InitialUpdateMask))
  {
  }

  // AllSpellCooldown
  if (stream->writeFlag(mask & AllSpellCooldownMask))
  {
  }

	return(retMask);
}

void afxSpellBook::unpackUpdate(NetConnection * con, BitStream * stream)
{
	Parent::unpackUpdate(con, stream);

  // InitialUpdate
  if (stream->readFlag())
  {
  }

  // AllSpellCooldown
  if (stream->readFlag())
  {
    all_spell_cooldown = 0.0f;
  }
}

#define SPELL_DATA_NOT_FOUND "\n<just:center><font:Arial:20><color:FF0000>** Spell data not found **\n\n\n\n"

char* afxSpellBook::formatDesc(char* buffer, int len, S32 page, S32 slot) const
{
  S32 idx = mDataBlock->getPageSlotIndex(page, slot);
  if (idx < 0 || !mDataBlock->rpg_spells[idx])
    return SPELL_DATA_NOT_FOUND;

  return mDataBlock->rpg_spells[idx]->formatDesc(buffer, len);
}

const char* afxSpellBook::getSpellIcon(S32 page, S32 slot) const
{
  S32 idx = mDataBlock->getPageSlotIndex(page, slot);
  if (idx < 0 || !mDataBlock->rpg_spells[idx])
    return 0;

  return mDataBlock->rpg_spells[idx]->icon_name;
}

bool afxSpellBook::isPlaceholder(S32 page, S32 slot) const
{
  S32 idx = mDataBlock->getPageSlotIndex(page, slot);
  if (idx < 0 || !mDataBlock->rpg_spells[idx])
    return false;

  return mDataBlock->rpg_spells[idx]->is_placeholder;
}


afxMagicSpellData* afxSpellBook::getSpellData(S32 page, S32 slot)
{
  S32 idx = mDataBlock->getPageSlotIndex(page, slot);
  if (idx < 0 || !mDataBlock->spells[idx])
    return 0;

  return mDataBlock->spells[idx];
}

afxRPGMagicSpellData* afxSpellBook::getSpellRPGData(S32 page, S32 slot)
{
  S32 idx = mDataBlock->getPageSlotIndex(page, slot);
  if (idx < 0 || !mDataBlock->rpg_spells[idx])
    return 0;

  return mDataBlock->rpg_spells[idx];
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

void afxSpellBook::startAllSpellCooldown()
{
  //all_spell_cooldown = 0.0f;
  setMaskBits(AllSpellCooldownMask);
}

F32 afxSpellBook::getCooldownFactor(S32 page, S32 slot)
{
  return all_spell_cooldown;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

DefineEngineMethod(afxSpellBook, getPageSlotIndex, S32, (Point2I bookSlot),,
                   "...\n\n"
                   "@ingroup AFX")
{
  return object->getPageSlotIndex(bookSlot.x, bookSlot.y);
}

DefineEngineMethod(afxSpellBook, getSpellData, S32, (Point2I bookSlot),,
                   "Get spell datablock for spell stored at spellbook index, (page, slot).\n\n"
                   "@ingroup AFX")
{
  afxMagicSpellData* spell_data = object->getSpellData(bookSlot.x, bookSlot.y);
  return (spell_data) ? spell_data->getId() : 0;
}

DefineEngineMethod(afxSpellBook, getSpellRPGData, S32, (Point2I bookSlot),,
                   "Get spell RPG datablock for spell stored at spellbook index, (page, slot).\n\n"
                   "@ingroup AFX")
{
  afxRPGMagicSpellData* spell_data = object->getSpellRPGData(bookSlot.x, bookSlot.y);
  return (spell_data) ? spell_data->getId() : 0;
}

DefineEngineMethod(afxSpellBook, startAllSpellCooldown, void, (),,
                   "...\n\n"
                   "@ingroup AFX")
{
  object->startAllSpellCooldown();
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//



