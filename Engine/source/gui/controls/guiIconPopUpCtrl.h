//-----------------------------------------------------------------------------
// Copyright (c) 2012 GarageGames, LLC
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
//-----------------------------------------------------------------------------

#ifndef GUIICONPOPUPCTRL_H
#define GUIICONPOPUPCTRL_H

#ifndef _GUITEXTCTRL_H_
#include "gui/controls/guiTextCtrl.h"
#endif
#ifndef _GUIICONLISTCTRL_H_
#include "gui/controls/guiIconListCtrl.h"
#endif
#ifndef _GUIBUTTONCTRL_H_
#include "gui/buttons/guiButtonCtrl.h"
#endif
#ifndef _GUISCROLLCTRL_H_
#include "gui/containers/guiScrollCtrl.h"
#endif
class GuiIconPopUpMenuCtrl;
class GuiIconPopupListCtrl;

class GuiIconPopUpBackgroundCtrl : public GuiControl
{
protected:
   GuiIconPopUpMenuCtrl *mPopUpCtrl;
   GuiIconPopupListCtrl *mTextList; 
public:
   GuiIconPopUpBackgroundCtrl(GuiIconPopUpMenuCtrl *ctrl, GuiIconPopupListCtrl* textList);
   void onMouseDown(const GuiEvent &event);
};

class GuiIconPopupListCtrl : public GuiIconListCtrl
{
   private:
      typedef GuiIconListCtrl Parent;

      bool hasCategories();

   protected:
      GuiIconPopUpMenuCtrl *mPopUpCtrl;

   public:
      GuiIconPopupListCtrl(); // for inheritance
      GuiIconPopupListCtrl(GuiIconPopUpMenuCtrl *ctrl);

      // GuiArrayCtrl overload:
      void onCellSelected(Point2I cell);

      // GuiControl overloads:
      bool onKeyDown(const GuiEvent &event);
      void onMouseUp(const GuiEvent &event);
      void onMouseMove(const GuiEvent &event);
      void onRenderCell(Point2I offset, Point2I cell, bool selected, bool mouseOver);
};

class GuiIconPopUpMenuCtrl : public GuiTextCtrl
{
   typedef GuiTextCtrl Parent;

  public:
   struct Entry
   {
      String iconPath;
      S32 id;
   };

   bool mBackgroundCancel; //  Added

  protected:
   GuiIconPopupListCtrl *mTl;
   GuiScrollCtrl *mSc;
   GuiIconPopUpBackgroundCtrl *mBackground;
   Vector<Entry> mEntries;
   S32 mSelIndex;
   S32 mMaxPopupHeight;
   F32 mIncValue;
   F32 mScrollCount;
   S32 mLastYvalue;
   GuiEvent mEventSave;
   S32 mRevNum;
   bool mInAction;
   bool mReplaceText;
   bool mMouseOver; //  Added
   bool mRenderScrollInNA; //  Added
   bool mReverseTextList;	//  Added - Should we reverse the text list if we display up?
   bool mHotTrackItems;
   StringTableEntry mBitmapName; //  Added
   Point2I mBitmapBounds; //  Added
   GFXTexHandle mTextureNormal; //  Added
   GFXTexHandle mTextureDepressed; //  Added
   Point2I mCellSize;
	S32 mIdMax;
	bool mAutoSize;
	S32 mColumns;

	String mSelectedIconPath;
	GFXTexHandle mSelectedIconTexture;

	static const S32 arrowWidth = 8;
	static const S32 arrowMargin = 4;
	
   virtual void addChildren();
   virtual void repositionPopup();

  public:
   GuiIconPopUpMenuCtrl(void);
   ~GuiIconPopUpMenuCtrl();   
   GuiScrollCtrl::Region mScrollDir;
   bool onWake(); //  Added
   bool onAdd();
   void onSleep();
   void setBitmap(const char *name); //  Added
   void setSelectedIcon(const String& iconPath);
   void sort();
   void sortID(); //  Added
	void addEntry(const String& iconPath, S32 id = -1);
   void onRender(Point2I offset, const RectI &updateRect);
   void onAction();
   virtual void closePopUp();
   void clear();
	void clearEntry( S32 entry ); //  Added
   void onMouseDown(const GuiEvent &event);
   void onMouseUp(const GuiEvent &event);
   void onMouseEnter(const GuiEvent &event); //  Added
   void onMouseLeave(const GuiEvent &); //  Added
   void setupAutoScroll(const GuiEvent &event);
   void autoScroll();
   bool onKeyDown(const GuiEvent &event);
   void reverseTextList();
   bool getFontColor(ColorI &fontColor, S32 id, bool selected, bool mouseOver);

   S32 getSelected();
   void setSelected(S32 id, bool bNotifyScript = true);
   void setFirstSelected(bool bNotifyScript = true); //  Added
   void setNoneSelected();	//  Added
   const char *getScriptValue();
   const String& getTextById(S32 id);
   S32 findText( const char* text );
   S32 getNumEntries()   { return( mEntries.size() ); }
   void replaceText(S32);

   bool resize(const Point2I& newPosition, const Point2I& newExtent);
   
   DECLARE_CONOBJECT(GuiIconPopUpMenuCtrl);
   DECLARE_CATEGORY( "Gui Lists" );
   DECLARE_DESCRIPTION( "A control that allows to select an icon from a drop-down list." );

   static void initPersistFields(void);
};

#endif //GUIICONPOPUPCTRL_H
