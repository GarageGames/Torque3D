
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

//-------------------------------------
//
// Bitmap Button Contrl
// Set 'bitmap' comsole field to base name of bitmaps to use.  This control will 
// append '_n' for normal
// append '_h' for hilighted
// append '_d' for depressed
//
// if bitmap cannot be found it will use the default bitmap to render.
//
// if the extent is set to (0,0) in the gui editor and appy hit, this control will
// set it's extent to be exactly the size of the normal bitmap (if present)
//

#include "afx/arcaneFX.h"

#include "console/engineAPI.h"
#include "gfx/gfxDrawUtil.h"

#include "afx/ui/afxSpellButton.h"
#include "afx/afxSpellBook.h"
#include "afx/afxMagicSpell.h"
#include "afx/rpg/afxRPGMagicSpell.h"

IMPLEMENT_CONOBJECT(afxSpellButton);

ConsoleDocClass( afxSpellButton,
   "@brief A GUI button with some special features that are useful for casting AFX spells.\n\n"

   "@ingroup afxGUI\n"
   "@ingroup AFX\n"
);

#define COOLDOWN_PROFILE	&GFXDefaultGUIProfile, avar("%s() - Cooldown Texture (line %d)", __FUNCTION__, __LINE__)

StringTableEntry afxSpellButton::sUnknownSpellBitmap = "";
StringTableEntry afxSpellButton::sSpellCooldownBitmaps = "";

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

afxSpellButton::afxSpellButton()
{
  if (sUnknownSpellBitmap == NULL)
    sUnknownSpellBitmap = ST_NULLSTRING;
  if (sSpellCooldownBitmaps == NULL)
    sSpellCooldownBitmaps = ST_NULLSTRING;
  mBitmapName = ST_NULLSTRING;
  setExtent(140, 30);
  spellbook = NULL;
  book_slot.set(0, 0);
}

afxSpellButton::~afxSpellButton()
{
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

void afxSpellButton::initPersistFields()
{
  addField("bitmap",      TypeFilename,   Offset(mBitmapName, afxSpellButton),
    "...");
  addField("book_slot",   TypePoint2I,    Offset(book_slot, afxSpellButton),
    "...");

  Parent::initPersistFields();

  Con::addVariable("pref::afxSpellButton::unknownSpellBitmap", TypeFilename, &sUnknownSpellBitmap);
  Con::addVariable("pref::afxSpellButton::spellCooldownBitmaps", TypeFilename, &sSpellCooldownBitmaps);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

bool afxSpellButton::onAdd()
{
  if (!Parent::onAdd())
    return false;

  if (sSpellCooldownBitmaps != NULL)
  {
    char buffer[256];
    for (int i = 0; i < NUM_COOLDOWN_FRAMES; i++)
    {
      dSprintf(buffer, 256, "%s_%.2d", sSpellCooldownBitmaps, i);
      cooldown_txrs[i].set(buffer, COOLDOWN_PROFILE);
    }
  }

  return true;
}

bool afxSpellButton::onWake()
{
  if (! Parent::onWake())
    return false;

  setActive(true);

  update_bitmap();

  return true;
}

void afxSpellButton::onSleep()
{
  mTextureNormal = NULL;
  mTextureHilight = NULL;
  mTextureDepressed = NULL;
  Parent::onSleep();
}

void afxSpellButton::onMouseEnter(const GuiEvent &event)
{
  Parent::onMouseEnter(event);
  Con::executef(this, "onMouseEnter");
}

void afxSpellButton::onMouseLeave(const GuiEvent &event)
{
  Parent::onMouseLeave(event);
  Con::executef(this, "onMouseLeave");
}

void afxSpellButton::inspectPostApply()
{
  // if the extent is set to (0,0) in the gui editor and apply hit, this control will
  // set it's extent to be exactly the size of the normal bitmap (if present)
  Parent::inspectPostApply();
  
  if ((getWidth() == 0) && (getHeight() == 0) && mTextureNormal)
  {
    setExtent(mTextureNormal->getWidth(), mTextureNormal->getHeight());
  }
}

void afxSpellButton::setBitmap(const char *name, bool placeholder)
{
  mBitmapName = (name) ? StringTable->insert(name) : ST_NULLSTRING;
  if (!isAwake())
    return;
  
  if (mBitmapName != ST_NULLSTRING)
  {
    char buffer[1024];
    char *p;

    if (placeholder)
    {
      dStrcpy(buffer, name);
      p = buffer + dStrlen(buffer);
    
      dStrcpy(p, "_i");
      mTextureInactive.set(buffer, COOLDOWN_PROFILE);
      mTextureNormal = mTextureInactive;
      mTextureHilight = mTextureInactive;
      mTextureDepressed = mTextureInactive;
      setActive(false);
    }
    else
    {
      dStrcpy(buffer, name);
      p = buffer + dStrlen(buffer);   
      dStrcpy(p, "_n");
      mTextureNormal.set(buffer, COOLDOWN_PROFILE);
      dStrcpy(p, "_h");
      mTextureHilight.set(buffer, COOLDOWN_PROFILE);
      if (!mTextureHilight)
        mTextureHilight = mTextureNormal;
      dStrcpy(p, "_d");
      mTextureDepressed.set(buffer, COOLDOWN_PROFILE);
      if (!mTextureDepressed)
        mTextureDepressed = mTextureHilight;
      dStrcpy(p, "_i");
      mTextureInactive.set(buffer, COOLDOWN_PROFILE);
      if (!mTextureInactive)
        mTextureInactive = mTextureNormal;
      setActive(true);
    }
  }
  else
  {
    mTextureNormal = NULL;
    mTextureHilight = NULL;
    mTextureDepressed = NULL;
    mTextureInactive = NULL;
  }

  setUpdate();
}   

void afxSpellButton::onRender(Point2I offset, const RectI& updateRect)
{
  enum { NORMAL, HILIGHT, DEPRESSED, INACTIVE } state = NORMAL;

  if (mActive)
  {
    if (mMouseOver) state = HILIGHT;
    if (mDepressed || mStateOn) state = DEPRESSED;
  }
  else
    state = INACTIVE;
  
  switch (state)
  {
  case NORMAL:      renderButton(mTextureNormal, offset, updateRect); break;
  case HILIGHT:     renderButton(mTextureHilight ? mTextureHilight : mTextureNormal, offset, updateRect); break;
  case DEPRESSED:   renderButton(mTextureDepressed, offset, updateRect); break;
  case INACTIVE:    renderButton(mTextureInactive ? mTextureInactive : mTextureNormal, offset, updateRect); break;
  }
}

void afxSpellButton::onDeleteNotify(SimObject* obj)
{
  // Handle Shape Deletion
  afxSpellBook* book = dynamic_cast<afxSpellBook*>(obj);
  if (book != NULL)
  {
    if (book == spellbook)
    {
      spellbook = NULL;
      setBitmap("");
      setVisible(false);
      return;
    }
  }

  Parent::onDeleteNotify(obj);
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// protected:

void afxSpellButton::renderButton(GFXTexHandle &texture, Point2I &offset, 
                                  const RectI& updateRect)
{
  if (texture)
  {
    RectI rect(offset, getExtent());
    GFX->getDrawUtil()->clearBitmapModulation();
    GFX->getDrawUtil()->drawBitmapStretch(texture, rect);

    if (spellbook)
    {
      F32 cooldown = spellbook->getCooldownFactor(book_slot.x, book_slot.y);
      if (cooldown < 1.0f)
      {

        if (cooldown_txrs[(int)(36.0f*cooldown)])
          GFX->getDrawUtil()->drawBitmapStretch(cooldown_txrs[(int)(36.0f*cooldown)], rect);
      }
    }

    renderChildControls( offset, updateRect);
  }
  else
    Parent::onRender(offset, updateRect);
}

void afxSpellButton::update_bitmap()
{
  const char* icon_name = 0;

  bool is_placeholder = false; 
  if (spellbook)
  {
    icon_name = spellbook->getSpellIcon(book_slot.x, book_slot.y);
    is_placeholder = spellbook->isPlaceholder(book_slot.x, book_slot.y);
    if (icon_name && icon_name[0] == '\0')
      icon_name = sUnknownSpellBitmap;
  }

  if (icon_name)
  {
    setBitmap(icon_name, is_placeholder);
    setVisible(true);
  }
  else
  {
    setBitmap("");
    setVisible(false);
  }
}

void afxSpellButton::setSpellBook(afxSpellBook* book, U8 page)
{
  book_slot.x = page;

  if (spellbook)
    clearNotify(spellbook);

  spellbook = book;
  update_bitmap();

  if (spellbook)
    deleteNotify(spellbook);
}

void afxSpellButton::setPage(U8 page)
{
  book_slot.x = page;
  update_bitmap();
}

char* afxSpellButton::formatDesc(char* buffer, int len) const
{
  return (spellbook) ? spellbook->formatDesc(buffer, len, book_slot.x, book_slot.y) : (char*)"";
}

afxMagicSpellData* afxSpellButton::getSpellDataBlock() const
{
  return (spellbook) ? spellbook->getSpellData(book_slot.x, book_slot.y) : 0;
}

afxRPGMagicSpellData* afxSpellButton::getSpellRPGDataBlock() const
{
  return (spellbook) ? spellbook->getSpellRPGData(book_slot.x, book_slot.y) : 0;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

DefineEngineMethod( afxSpellButton, onSpellbookChange, void, ( afxSpellBook* spellbook, unsigned int page ),,
   "Notify an afxSpellButton when its associated spellbook has changed.\n" )
{
  object->setSpellBook(spellbook, (U8)page);
}

DefineEngineMethod( afxSpellButton, onTurnPage, void, ( unsigned int page ),,
   "Notify an afxSpellButton when the spellbook turns to a new page.\n" )
{
  object->setPage((U8)page);
}

DefineEngineMethod( afxSpellButton, getSpellDescription, const char*, (),,
   "Get the text description of a spell.\n" )
{
  char buf[1024];
  return object->formatDesc(buf, 1024);
}

DefineEngineMethod( afxSpellButton, getSpellDataBlock, S32, (),,
   "Get the spell's datablock.\n" )
{
  afxMagicSpellData* spell_data = object->getSpellDataBlock();
  return (spell_data) ?  spell_data->getId() : -1;
}

DefineEngineMethod( afxSpellButton, getSpellRPGDataBlock, S32, (),,
   "Get the spell's RPG datablock.\n" )
{
  afxRPGMagicSpellData* spell_rpg_data = object->getSpellRPGDataBlock();
  return (spell_rpg_data) ?  spell_rpg_data->getId() : -1;
}

DefineEngineMethod( afxSpellButton, useFreeTargeting, bool, (),,
   "Test if spell uses free targeting.\n" )
{
  afxRPGMagicSpellData* spell_rpg_data = object->getSpellRPGDataBlock();
  return (spell_rpg_data) ?  (spell_rpg_data->spell_target == afxRPGMagicSpellData::TARGET_FREE) : false;
}

DefineEngineMethod( afxSpellButton, getFreeTargetStyle, S32, (),,
   "Get the free targeting style used by the spell.\n" )
{
  afxRPGMagicSpellData* spell_rpg_data = object->getSpellRPGDataBlock();
  return (spell_rpg_data) ?  spell_rpg_data->free_target_style : 0;
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
