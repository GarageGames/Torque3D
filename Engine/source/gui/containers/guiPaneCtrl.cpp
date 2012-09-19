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

#include "console/engineAPI.h"
#include "platform/platform.h"
#include "gui/containers/guiPaneCtrl.h"

#include "gfx/gfxDrawUtil.h"


IMPLEMENT_CONOBJECT(GuiPaneControl);

ConsoleDocClass( GuiPaneControl,
   "@brief A collapsable pane control.\n\n"

   "This class wraps a single child control and displays a header with caption "
   "above it. If you click the header it will collapse or expand (if <i>collapsable</i> "
   "is enabled). The control resizes itself based on its collapsed/expanded size.<br>"

   "In the GUI editor, if you just want the header you can make <i>collapsable</i> "
   "false. The caption field lets you set the caption; it expects a bitmap (from "
   "the GuiControlProfile) that contains two images - the first is displayed when "
   "the control is expanded and the second is displayed when it is collapsed. The "
   "header is sized based on the first image.\n\n"

   "@tsexample\n"
   "new GuiPaneControl()\n"
   "{\n"
   "   caption = \"Example Pane\";\n"
   "   collapsable = \"1\";\n"
   "   barBehindText = \"1\";\n"
   "   //Properties not specific to this control have been omitted from this example.\n"
   "};\n"
   "@endtsexample\n\n"

   "@ingroup GuiContainers"
);

//-----------------------------------------------------------------------------

GuiPaneControl::GuiPaneControl()
{
   setMinExtent(Point2I(16,16));
   
   mActive        = true;
   mCollapsable   = true;
   mCollapsed     = false;
   mBarBehindText = true;
   mMouseOver     = false;
   mDepressed     = false;
   mCaption       = "A Pane";
   mCaptionID     = StringTable->insert("");
   mIsContainer   = true;

   mOriginalExtents.set(10,10);
}

//-----------------------------------------------------------------------------

void GuiPaneControl::initPersistFields()
{
   addGroup( "Pane" );
   
      addField("caption",       TypeRealString,  Offset(mCaption,        GuiPaneControl),
         "Text label to display as the pane header." );
      addField("captionID",     TypeString,      Offset(mCaptionID,      GuiPaneControl),
         "String table text ID to use as caption string (overrides 'caption')." );
      addField("collapsable",   TypeBool,        Offset(mCollapsable,    GuiPaneControl),
         "Whether the pane can be collapsed by clicking its header." );
      addField("barBehindText", TypeBool,        Offset(mBarBehindText,  GuiPaneControl),
         "Whether to draw the bitmapped pane bar behind the header text, too." );
      
   endGroup( "Pane" );

   Parent::initPersistFields();
}

//-----------------------------------------------------------------------------

bool GuiPaneControl::onWake()
{
   if ( !Parent::onWake() )
      return false;

   if( !mProfile->mFont )
   {
      Con::errorf( "GuiPaneControl::onWake - profile has no valid font" );
      return false;
   }

   if(mCaptionID && *mCaptionID != 0)
		setCaptionID(mCaptionID);

   mProfile->constructBitmapArray();
   if(mProfile->mUseBitmapArray && mProfile->mBitmapArrayRects.size())
   {
      mThumbSize.set(   mProfile->mBitmapArrayRects[0].extent.x, mProfile->mBitmapArrayRects[0].extent.y );
      mThumbSize.setMax( mProfile->mBitmapArrayRects[1].extent );

      if( mProfile->mFont->getHeight() > mThumbSize.y )
         mThumbSize.y = mProfile->mFont->getHeight();
   }
   else
   {
      mThumbSize.set(20, 20);
   }

   return true;
}

//-----------------------------------------------------------------------------

void GuiPaneControl::setCaptionID(const char *id)
{
	S32 n = Con::getIntVariable(id, -1);
	if(n != -1)
	{
		mCaptionID = StringTable->insert(id);
		setCaptionID(n);
	}
}

//-----------------------------------------------------------------------------

void GuiPaneControl::setCaptionID(S32 id)
{
	mCaption = getGUIString(id);
}

//-----------------------------------------------------------------------------

bool GuiPaneControl::resize(const Point2I &newPosition, const Point2I &newExtent)
{
   // CodeReview WTF is going on here that we need to bypass parent sanity?
   //  Investigate this [7/1/2007 justind]
   if( !Parent::resize( newPosition, newExtent ) )
      return false;

   mOriginalExtents.x = getWidth();

   /*
   GuiControl *parent = getParent();
   if (parent)
      parent->childResized(this);
   setUpdate();
   */

   // Resize the child control if we're not collapsed
   if(size() && !mCollapsed)
   {
      GuiControl *gc = dynamic_cast<GuiControl*>(operator[](0));

      if(gc)
      {
         Point2I offset(0, mThumbSize.y);

         gc->resize(offset, newExtent - offset);
      }
   }

   // For now.
   return true;
}

//-----------------------------------------------------------------------------

void GuiPaneControl::onRender(Point2I offset, const RectI &updateRect)
{
   // Render our awesome little doogong
   if(mProfile->mBitmapArrayRects.size() >= 2 && mCollapsable)
   {
      S32 idx = mCollapsed ? 0 : 1;

      GFX->getDrawUtil()->clearBitmapModulation();
      GFX->getDrawUtil()->drawBitmapStretchSR(
         mProfile->mTextureObject,
         RectI(offset, mProfile->mBitmapArrayRects[idx].extent),
         mProfile->mBitmapArrayRects[idx]
      );

   }

   S32 textWidth = 0;

   if(!mBarBehindText)
   {
      GFX->getDrawUtil()->setBitmapModulation((mMouseOver ? mProfile->mFontColorHL : mProfile->mFontColor));
      textWidth = GFX->getDrawUtil()->drawText(
            mProfile->mFont,
            Point2I(mThumbSize.x, 0) + offset,
            mCaption,
            mProfile->mFontColors
         );
   }


   // Draw our little bar, too
   if(mProfile->mBitmapArrayRects.size() >= 5)
   {
      GFX->getDrawUtil()->clearBitmapModulation();

      S32 barStart = mThumbSize.x + offset.x + textWidth;
      S32 barTop   = mThumbSize.y/2 + offset.y - mProfile->mBitmapArrayRects[3].extent.y /2;

      Point2I barOffset(barStart, barTop);

      // Draw the start of the bar...
      GFX->getDrawUtil()->drawBitmapStretchSR(
         mProfile->mTextureObject,
         RectI(barOffset, mProfile->mBitmapArrayRects[2].extent),
         mProfile->mBitmapArrayRects[2]
         );

      // Now draw the middle...
      barOffset.x += mProfile->mBitmapArrayRects[2].extent.x;

      S32 barMiddleSize = (getExtent().x - (barOffset.x - offset.x)) - mProfile->mBitmapArrayRects[4].extent.x;

      if(barMiddleSize>0)
      {
         // We have to do this inset to prevent nasty stretching artifacts
         RectI foo = mProfile->mBitmapArrayRects[3];
         foo.inset(1,0);

         GFX->getDrawUtil()->drawBitmapStretchSR(
            mProfile->mTextureObject,
            RectI(barOffset, Point2I(barMiddleSize, mProfile->mBitmapArrayRects[3].extent.y)),
            foo
            );
      }

      // And the end
      barOffset.x += barMiddleSize;

      GFX->getDrawUtil()->drawBitmapStretchSR(
         mProfile->mTextureObject,
         RectI(barOffset, mProfile->mBitmapArrayRects[4].extent),
         mProfile->mBitmapArrayRects[4]
         );
   }

   if(mBarBehindText)
   {
      GFX->getDrawUtil()->setBitmapModulation((mMouseOver ? mProfile->mFontColorHL : mProfile->mFontColor));
      GFX->getDrawUtil()->drawText(
            mProfile->mFont,
            Point2I(mThumbSize.x, 0) + offset,
            mCaption,
            mProfile->mFontColors
         );
   }

   // Draw child controls if appropriate
   if(!mCollapsed)
      renderChildControls(offset, updateRect);
}

//-----------------------------------------------------------------------------

void GuiPaneControl::setCollapsed(bool isCollapsed)
{
   // Get the child
   if(size() == 0 || !mCollapsable) return;

   GuiControl *gc = dynamic_cast<GuiControl*>(operator[](0));

   if(mCollapsed && !isCollapsed)
   {
      resize(getPosition(), mOriginalExtents);
      mCollapsed = false;

      if(gc)
         gc->setVisible(true);
   }
   else if(!mCollapsed && isCollapsed)
   {
      mCollapsed = true;

      mOriginalExtents = getExtent();
      resize(getPosition(), Point2I(getExtent().x, mThumbSize.y));

      if(gc)
         gc->setVisible(false);
   }
}

//-----------------------------------------------------------------------------

void GuiPaneControl::onMouseMove(const GuiEvent &event)
{
   Point2I localMove = globalToLocalCoord(event.mousePoint);

   // If we're clicking in the header then resize
   mMouseOver = (localMove.y < mThumbSize.y);
   if(isMouseLocked())
      mDepressed = mMouseOver;

}

//-----------------------------------------------------------------------------

void GuiPaneControl::onMouseEnter(const GuiEvent &event)
{
   setUpdate();
   if(isMouseLocked())
   {
      mDepressed = true;
      mMouseOver = true;
   }
   else
   {
      mMouseOver = true;
   }

}

//-----------------------------------------------------------------------------

void GuiPaneControl::onMouseLeave(const GuiEvent &event)
{
   setUpdate();
   if(isMouseLocked())
      mDepressed = false;
   mMouseOver = false;
}

//-----------------------------------------------------------------------------

void GuiPaneControl::onMouseDown(const GuiEvent &event)
{
   if(!mCollapsable)
      return;

   Point2I localClick = globalToLocalCoord(event.mousePoint);

   // If we're clicking in the header then resize
   if(localClick.y < mThumbSize.y)
   {
      mouseLock();
      mDepressed = true;

      //update
      setUpdate();
   }
}

//-----------------------------------------------------------------------------

void GuiPaneControl::onMouseUp(const GuiEvent &event)
{
   // Make sure we only get events we ought to be getting...
   if (! mActive)
      return;

   if(!mCollapsable)
      return;

   mouseUnlock();
   setUpdate();

   Point2I localClick = globalToLocalCoord(event.mousePoint);

   // If we're clicking in the header then resize
   if(localClick.y < mThumbSize.y && mDepressed)
      setCollapsed(!mCollapsed);
}

//=============================================================================
//    Console Methods.
//=============================================================================

DefineEngineMethod( GuiPaneControl, setCollapsed, void, ( bool collapse ),,
   "Collapse or un-collapse the control.\n\n"
   "@param collapse True to collapse the control, false to un-collapse it\n" )
{
   object->setCollapsed( collapse );
}
