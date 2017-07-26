
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

#ifndef _AFX_SPELL_BOOK_H_
#define _AFX_SPELL_BOOK_H_

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#include "T3D/gameBase/gameBase.h"

class afxSpellBookDefs
{
public:
  enum {
    MAX_SPELLS_PER_PAGE = 12,
    MAX_PAGES_PER_BOOK = 12
  };
};

class afxMagicSpellData;
class afxRPGMagicSpellData;

class afxSpellBookData : public GameBaseData, public afxSpellBookDefs
{
  typedef GameBaseData  Parent;

  bool                  do_id_convert;

public:
  U8                    spells_per_page;
  U8                    pages_per_book;
  afxMagicSpellData*    spells[MAX_PAGES_PER_BOOK*MAX_SPELLS_PER_PAGE];
  afxRPGMagicSpellData* rpg_spells[MAX_PAGES_PER_BOOK*MAX_SPELLS_PER_PAGE];

public:
  /*C*/                 afxSpellBookData();

  virtual void          packData(BitStream*);
  virtual void          unpackData(BitStream*);

  bool                  preload(bool server, String &errorStr);

  bool                  verifyPageSlot(S32 page, S32 slot);
  S32                   getPageSlotIndex(S32 page, S32 slot);

  static void           initPersistFields();

  DECLARE_CONOBJECT(afxSpellBookData);
  DECLARE_CATEGORY("AFX");
};

inline bool afxSpellBookData::verifyPageSlot(S32 page, S32 slot)
{
  return (page >= 0 && page < pages_per_book && slot >= 0 && slot < spells_per_page);
}

inline S32 afxSpellBookData::getPageSlotIndex(S32 page, S32 slot)
{
  return (verifyPageSlot(page, slot)) ? page*spells_per_page + slot : -1;
}


//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

class afxMagicSpellData;
class afxSpellButton;

class afxSpellBook : public GameBase, public afxSpellBookDefs
{
  typedef GameBase        Parent;

  enum MaskBits 
  {
    AllSpellCooldownMask  = Parent::NextFreeMask << 0,
    NextFreeMask          = Parent::NextFreeMask << 1
  };

private:
  afxSpellBookData*       mDataBlock;
  F32                     all_spell_cooldown;

public:
  /*C*/                   afxSpellBook();
  /*D*/                   ~afxSpellBook();

  virtual bool            onNewDataBlock(GameBaseData* dptr, bool reload);
  virtual void            processTick(const Move*);
  virtual void            advanceTime(F32 dt);

  virtual bool            onAdd();
  virtual void            onRemove();

  virtual U32             packUpdate(NetConnection*, U32, BitStream*);
  virtual void            unpackUpdate(NetConnection*, BitStream*);

  static void             initPersistFields();

  S32                     getPageSlotIndex(S32 page, S32 slot);
  char*                   formatDesc(char* buffer, int len, S32 page, S32 slot) const;
  const char*             getSpellIcon(S32 page, S32 slot) const;
  bool                    isPlaceholder(S32 page, S32 slot) const;
  afxMagicSpellData*      getSpellData(S32 page, S32 slot);
  afxRPGMagicSpellData*   getSpellRPGData(S32 page, S32 slot);

  void                    startAllSpellCooldown();
  F32                     getCooldownFactor(S32 page, S32 slot);

  DECLARE_CONOBJECT(afxSpellBook);
  DECLARE_CATEGORY("AFX");
};

inline S32 afxSpellBook::getPageSlotIndex(S32 page, S32 slot)
{
  return (mDataBlock) ? mDataBlock->getPageSlotIndex(page, slot) : -1;
}


//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#endif // _AFX_SPELL_BOOK_H_
