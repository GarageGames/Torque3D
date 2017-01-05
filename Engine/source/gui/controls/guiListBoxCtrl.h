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
#ifndef _GUI_LISTBOXCTRL_H_
#define _GUI_LISTBOXCTRL_H_

#ifndef _CONSOLETYPES_H_
#include "console/consoleTypes.h"
#endif

#ifndef _GUICONTROL_H_
#include "gui/core/guiControl.h"
#endif

#ifndef _DGL_H_
#include "gfx/gfxDevice.h"
#endif

#ifndef _H_GUIDEFAULTCONTROLRENDER_
#include "gui/core/guiDefaultControlRender.h"
#endif

#ifndef _GUISCROLLCTRL_H_
#include "gui/containers/guiScrollCtrl.h"
#endif


class GuiListBoxCtrl : public GuiControl
{
private:
   typedef GuiControl Parent;
public:

   GuiListBoxCtrl();
   ~GuiListBoxCtrl();
   DECLARE_CONOBJECT(GuiListBoxCtrl);
   DECLARE_CATEGORY( "Gui Lists" );
   DECLARE_DESCRIPTION( "Linear list of text items." );

   DECLARE_CALLBACK( void, onMouseDragged, ());
   DECLARE_CALLBACK( void, onClearSelection, ());
   DECLARE_CALLBACK( void, onUnSelect, ( S32 index, const char* itemText));
   DECLARE_CALLBACK( void, onSelect, ( S32 index , const char* itemText ));
   DECLARE_CALLBACK( void, onDoubleClick, ());
   DECLARE_CALLBACK( void, onMouseUp, ( S32 itemHit, S32 mouseClickCount ));
   DECLARE_CALLBACK( void, onDeleteKey, ());
   DECLARE_CALLBACK( bool, isObjectMirrored, ( const char* indexIdString ));

   struct LBItem
   {
      StringTableEntry  itemText;
      String            itemTooltip;
      bool              isSelected;
      void*             itemData;
      ColorF            color;
      bool              hasColor;
   };

   VectorPtr<LBItem*>   mItems;
   VectorPtr<LBItem*>   mSelectedItems;

	VectorPtr<LBItem*>	mFilteredItems;

   bool                 mMultipleSelections;
   Point2I              mItemSize;
   bool                 mFitParentWidth;
   bool                 mColorBullet;
   LBItem*              mLastClickItem;

   // Persistence
   static void       initPersistFields();   

   // Item Accessors
   S32               getItemCount();
   S32               getSelCount();
   S32               getSelectedItem();
   void              getSelectedItems( Vector<S32> &Items );
   S32               getItemIndex( LBItem *item );
   StringTableEntry  getItemText( S32 index );
   SimObject*        getItemObject( S32 index );
   
   void              setCurSel( S32 index );
   void              setCurSelRange( S32 start, S32 stop );
   void              setItemText( S32 index, StringTableEntry text );

   S32               addItem( StringTableEntry text, void *itemData = NULL );
   S32               addItemWithColor( StringTableEntry text, ColorF color = ColorF(-1, -1, -1), void *itemData = NULL);
   S32               insertItem( S32 index, StringTableEntry text, void *itemData = NULL );
   S32               insertItemWithColor( S32 index, StringTableEntry text, ColorF color = ColorF(-1, -1, -1), void *itemData = NULL);
   S32               findItemText( StringTableEntry text, bool caseSensitive = false );

   void              setItemColor(S32 index, const ColorF& color);
   void              clearItemColor(S32 index);

   void              deleteItem( S32 index );
   void              clearItems();
   void              clearSelection();
   void              removeSelection( LBItem *item, S32 index );
   void              removeSelection( S32 index );
   void              addSelection( LBItem *item, S32 index );
   void              addSelection( S32 index );
   inline void       setMultipleSelection( bool allowMultipleSelect = true ) { mMultipleSelections = allowMultipleSelect; };
   
   bool              hitTest( const Point2I& point, S32& outItem );

   // Sizing
   void              updateSize();
   virtual void      parentResized(const RectI& oldParentRect, const RectI& newParentRect);
   virtual bool      onWake();

   // Rendering
   virtual void      onRender( Point2I offset, const RectI &updateRect );
   virtual void      onRenderItem(const RectI& itemRect, LBItem *item);
   void              drawBox( const Point2I &box, S32 size, ColorI &outlineColor, ColorI &boxColor );
   bool              renderTooltip( const Point2I &hoverPos, const Point2I& cursorPos, const char* tipText );
	void					addFilteredItem( String item );
	void					removeFilteredItem( String item );
   // Mouse/Key Events
   virtual void      onMouseDown( const GuiEvent &event );
   virtual void      onMouseDragged(const GuiEvent &event);
   virtual void      onMouseUp( const GuiEvent& event );
   virtual bool      onKeyDown( const GuiEvent &event );   

   // String Utility
   static U32        getStringElementCount( const char *string );
   static const char* getStringElement( const char* inString, const U32 index );
   
   // SimSet Mirroring Stuff
   void setMirrorObject( SimSet *inObj );
   void _mirror();
   StringTableEntry _makeMirrorItemName( SimObject *inObj );

   String mMirrorSetName;
   String mMakeNameCallback;
};

#endif