//-----------------------------------------------------------------------------
// Copyright (c) 2014 Guy Allard
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

#include "gfx/gfxDrawUtil.h"
#include "gui/worldEditor/editorIconRegistry.h"
#include "gui/controls/guiTextEditCtrl.h"

#include "guiBTViewCtrl.h"

using namespace BadBehavior;

IMPLEMENT_CONOBJECT(GuiBehaviorTreeViewCtrl);

void GuiBehaviorTreeViewCtrl::onRenderCell(Point2I offset, Point2I cell, bool, bool )
{
   if( !mVisibleItems.size() )
      return;

   // Do some sanity checking and data retrieval.
   AssertFatal(cell.y < mVisibleItems.size(), "GuiTreeViewCtrl::onRenderCell: invalid cell");
   Item * item = mVisibleItems[cell.y];

   // If there's no object, deal with it.
   if(item->isInspectorData())
      if(!item->getObject())
         return;

   RectI drawRect( offset, mCellSize );
   GFXDrawUtil *drawer = GFX->getDrawUtil();
   drawer->clearBitmapModulation();

   FrameAllocatorMarker txtBuff;
   
   // Ok, we have the item. There are a few possibilities at this point:
   //    - We need to draw inheritance lines and a treeview-chosen icon
   //       OR
   //    - We have to draw an item-dependent icon
   //    - If we're mouseover, we have to highlight it.
   //
   //    - We have to draw the text for the item
   //       - Taking into account various mouseover states
   //       - Taking into account the value (set or not)
   //       - If it's an inspector data, we have to do some custom rendering
   //       - ADDED: If it is being renamed, we also have custom rendering.

   // Ok, first draw the tab and icon.

   // Do we draw the tree lines?
   if( mFlags.test(ShowTreeLines) )
   {
      drawRect.point.x += ( mTabSize * item->mTabLevel );
      Item* parent = item->mParent;
      for ( S32 i = item->mTabLevel; ( parent && i > 0 ); i-- )
      {
         drawRect.point.x -= mTabSize;
         if ( parent->mNext )
            drawer->drawBitmapSR( mProfile->mTextureObject, drawRect.point, mProfile->mBitmapArrayRects[BmpLine] );

         parent = parent->mParent;
      }
   }

   // Now, the icon...
   drawRect.point.x = offset.x + mTabSize * item->mTabLevel;

   // First, draw the rollover glow, if it's an inner node.
   if ( item->isParent() && item->mState.test( Item::MouseOverBmp ) )
      drawer->drawBitmapSR( mProfile->mTextureObject, drawRect.point, mProfile->mBitmapArrayRects[BmpGlow] );

   // Now, do we draw a treeview-selected item or an item dependent one?
   S32 newOffset = 0; // This is stored so we can render glow, then update render pos.

   S32 bitmap = 0;

   // Ok, draw the treeview lines as appropriate.
   
   bool drawBitmap = true;
   if ( !item->isParent() )
   {
      if( mFlags.test( ShowTreeLines ) )
      {
         if(    ( item->mNext && item->mPrevious )
             || ( item->mNext && item->mParent && ( !_isRootLevelItem( item ) || mShowRoot ) ) )
            bitmap = BmpChild;
         else if( item->mNext && ( !item->mParent || !mShowRoot ) )
            bitmap = BmpFirstChild;
         else if( item->mPrevious || ( item->mParent && !_isRootLevelItem( item ) ) )
            bitmap = BmpLastChild;
         else
            drawBitmap = false;
      }
      else
         drawBitmap = false;
   }
   else
   {
      bitmap = item->isExpanded() ? BmpExp : BmpCon;

      if( mFlags.test( ShowTreeLines ) )
      {
         // Shift indices to show versions with tree lines.
         
         if ( item->mParent || item->mPrevious )
            bitmap += ( item->mNext ? 3 : 2 );
         else
            bitmap += ( item->mNext ? 1 : 0 );
      }
   }

   if( ( bitmap >= 0 ) && ( bitmap < mProfile->mBitmapArrayRects.size() ) )
   {
      if( drawBitmap )
         drawer->drawBitmapSR( mProfile->mTextureObject, drawRect.point, mProfile->mBitmapArrayRects[bitmap] );
      newOffset = mProfile->mBitmapArrayRects[bitmap].extent.x;
   }

   if(item->isInspectorData())
   {
      // draw lock icon if need be
      S32 icon = Lock1;
      S32 icon2 = Hidden;

      if (item->getObject() && item->getObject()->isLocked())
      {
         if (mIconTable[icon])
         {
            //drawRect.point.x = offset.x + mTabSize * item->mTabLevel + mIconTable[icon].getWidth();
            drawRect.point.x += mIconTable[icon].getWidth();
            drawer->drawBitmap( mIconTable[icon], drawRect.point );
         }
      }

      if (item->getObject() && item->getObject()->isHidden())
      {
         if (mIconTable[icon2])
         {
            //drawRect.point.x = offset.x + mTabSize * item->mTabLevel + mIconTable[icon].getWidth();
            drawRect.point.x += mIconTable[icon2].getWidth();
            drawer->drawBitmap( mIconTable[icon2], drawRect.point );
         }
      }

      /*SimObject * pObject = item->getObject();
      SimGroup  * pGroup  = ( pObject == NULL ) ? NULL : dynamic_cast<SimGroup*>( pObject );

      // If this item is a VirtualParent we can use the generic SimGroup123 icons.
      // However if there is already an icon in the EditorIconRegistry for this
      // exact class (not counting parent class icons) we want to use that instead.
      bool hasClassIcon = gEditorIcons.hasIconNoRecurse( pObject );

      // draw the icon associated with the item
      if ( !hasClassIcon && item->mState.test(Item::VirtualParent))
      {
         if ( pGroup != NULL)
         {
            if (item->isExpanded())
               item->mIcon = SimGroup1;
            else
               item->mIcon = SimGroup2;
         }
         else
            item->mIcon = SimGroup2;
      }
      
      if ( !hasClassIcon && item->mState.test(Item::Marked))
      {
         if (item->isInspectorData())
         {
            if ( pGroup != NULL )
            {
               if (item->isExpanded())
                  item->mIcon = SimGroup3;
               else
                  item->mIcon = SimGroup4;
            }
         }
      }*/

      GFXTexHandle iconHandle;

      if ( ( item->mIcon != -1 ) && mIconTable[item->mIcon] )
         iconHandle = mIconTable[item->mIcon];
#ifdef TORQUE_TOOLS
      else
         iconHandle = gEditorIcons.findIcon( item->getObject() );
#endif

      if ( iconHandle.isValid() )
      {
         S32 iconHeight = (mItemHeight - iconHandle.getHeight()) / 2;
         S32 oldHeight = drawRect.point.y;
         if(iconHeight > 0)
            drawRect.point.y += iconHeight;
         drawRect.point.x += iconHandle.getWidth();
         drawer->drawBitmap( iconHandle, drawRect.point );
         drawRect.point.y = oldHeight;
      }
   }
   else
   {
      S32 icon = item->isExpanded() ? item->mScriptInfo.mExpandedImage : item->mScriptInfo.mNormalImage;
      if ( icon )
      {
         if (mIconTable[icon])
         {
            S32 iconHeight = (mItemHeight - mIconTable[icon].getHeight()) / 2;
            S32 oldHeight = drawRect.point.y;
            if(iconHeight > 0)
               drawRect.point.y += iconHeight;
            drawRect.point.x += mIconTable[icon].getWidth();
            drawer->drawBitmap( mIconTable[icon], drawRect.point );
            drawRect.point.y = oldHeight;
         }
      }
   }

   // Ok, update offset so we can render some text!
   drawRect.point.x += newOffset;

   // Ok, now we're off to rendering the actual data for the treeview item.

   U32 bufLen = 1024; //item->mDataRenderWidth + 1;
   char *displayText = (char *)txtBuff.alloc(bufLen);
   displayText[bufLen-1] = 0;
   item->getDisplayText(bufLen, displayText);

   // Draw the rollover/selected bitmap, if one was specified.
   drawRect.extent.x = mProfile->mFont->getStrWidth( displayText ) + ( 2 * mTextOffset );
   if ( item->mState.test( Item::Selected ) && mTexSelected )
      drawer->drawBitmapStretch( mTexSelected, drawRect );
   else if ( item->mState.test( Item::MouseOverText ) && mTexRollover )
      drawer->drawBitmapStretch( mTexRollover, drawRect );

   // Offset a bit so as to space text properly.
   drawRect.point.x += mTextOffset;

   // Determine what color the font should be.
   ColorI fontColor;

   fontColor = item->mState.test( Item::Selected ) ? mProfile->mFontColorSEL :
             ( item->mState.test( Item::MouseOverText ) ? mProfile->mFontColorHL : mProfile->mFontColor );

   if (item->mState.test(Item::Selected))
   {
      drawer->drawRectFill(drawRect, mProfile->mFillColorSEL);
   }
   else if (item->mState.test(Item::MouseOverText))
   {
      drawer->drawRectFill(drawRect, mProfile->mFillColorHL);
   }

   if( item->mState.test(Item::MouseOverText) )
   {
		fontColor	=	mProfile->mFontColorHL;
   }

   drawer->setBitmapModulation( fontColor );

   // Center the text horizontally.
   S32 height = (mItemHeight - mProfile->mFont->getHeight()) / 2;

   if(height > 0)
      drawRect.point.y += height;

   // JDD - offset by two pixels or so to keep the text from rendering RIGHT ONTOP of the outline
   drawRect.point.x += 2;

   drawer->drawText( mProfile->mFont, drawRect.point, displayText, mProfile->mFontColors );

   if ( mRenamingItem == item && mRenameCtrl )
   {
      Point2I ctrPos = globalToLocalCoord( drawRect.point );
      ctrPos.y -= height;
      ctrPos.x -= 2;

      Point2I ctrExtent( getWidth() - ctrPos.x, drawRect.extent.y );

      mRenameCtrl->setPosition( ctrPos );
      mRenameCtrl->setExtent( ctrExtent );
      mRenameCtrl->setVisible( true );
   }
}