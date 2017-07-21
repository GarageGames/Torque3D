
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

#ifndef _AFX_SPELL_BUTTON_H_
#define _AFX_SPELL_BUTTON_H_

#include "gui/buttons/guiButtonCtrl.h"

///-------------------------------------
/// Bitmap Button Contrl
/// Set 'bitmap' comsole field to base name of bitmaps to use.  This control will 
/// append '_n' for normal
/// append '_h' for hilighted
/// append '_d' for depressed
///
/// if bitmap cannot be found it will use the default bitmap to render.
///
/// if the extent is set to (0,0) in the gui editor and appy hit, this control will
/// set it's extent to be exactly the size of the normal bitmap (if present)
///

class afxSpellBook;
class afxMagicSpellData;
class afxRPGMagicSpellData;

class afxSpellButton : public GuiButtonCtrl
{
private:
  typedef GuiButtonCtrl Parent;

  enum { NUM_COOLDOWN_FRAMES = 36 };
  
protected:
  static StringTableEntry sUnknownSpellBitmap;
  static StringTableEntry sSpellCooldownBitmaps;

  StringTableEntry  mBitmapName;
  GFXTexHandle      mTextureNormal;
  GFXTexHandle      mTextureHilight;
  GFXTexHandle      mTextureDepressed;
  GFXTexHandle      mTextureInactive;
  
  afxSpellBook*     spellbook;
  Point2I           book_slot;

  GFXTexHandle      cooldown_txrs[NUM_COOLDOWN_FRAMES];
  
  void              update_bitmap();
  void              renderButton(GFXTexHandle &texture, Point2I &offset, const RectI& updateRect);
  
public:   
  /*C*/             afxSpellButton();
  /*D*/             ~afxSpellButton();
  
  void              setBitmap(const char *name, bool placholder=false);
  void              setSpellBook(afxSpellBook*, U8 page);
  void              setPage(U8 page);
  char*             formatDesc(char* buffer, int len) const;

  afxMagicSpellData*      getSpellDataBlock() const;
  afxRPGMagicSpellData*   getSpellRPGDataBlock() const;
  
  virtual bool      onAdd();
  virtual bool      onWake();
  virtual void      onSleep();
  virtual void      inspectPostApply();
  virtual void      onMouseEnter(const GuiEvent &event);
  virtual void      onMouseLeave(const GuiEvent &event);
  virtual void      onRender(Point2I offset, const RectI &updateRect);

  virtual void      onDeleteNotify(SimObject*);
  
  static void       initPersistFields();
  
  DECLARE_CONOBJECT(afxSpellButton);
  DECLARE_CATEGORY("AFX");
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#endif //_GUI_BITMAP_BUTTON_CTRL_H
