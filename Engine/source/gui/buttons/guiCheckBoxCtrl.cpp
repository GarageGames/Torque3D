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

#include "platform/platform.h"
#include "gui/buttons/guiCheckBoxCtrl.h"

#include "console/console.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "gfx/gfxDevice.h"
#include "gfx/gfxDrawUtil.h"
#include "gui/core/guiCanvas.h"
#include "sfx/sfxSystem.h"
#include "sfx/sfxTrack.h"


IMPLEMENT_CONOBJECT( GuiCheckBoxCtrl );

ConsoleDocClass( GuiCheckBoxCtrl,
   "@brief A named checkbox that can be toggled on and off.\n\n"
   
   "A GuiCheckBoxCtrl displays a text label next to a checkbox that can be toggled on and off by the user. "
   "Checkboxes are usually used to present boolean choices like, for example, a switch to toggle fullscreen "
   "video on and off.\n\n"
   
   "@tsexample\n"
   "// Create a checkbox that allows to toggle fullscreen on and off.\n"
   "new GuiCheckBoxCtrl( FullscreenToggle )\n"
   "{\n"
   "   text = \"Fullscreen\";\n"
   "};\n"
   "\n"
   "// Set the initial state to match the current fullscreen setting.\n"
   "FullscreenToggle.setStateOn( Canvas.isFullscreen() );\n"
   "\n"
   "// Define function to be called when checkbox state is toggled.\n"
   "function FullscreenToggle::onClick( %this )\n"
   "{\n"
   "   Canvas.toggleFullscreen();\n"
   "}\n"
   "@endtsexample\n\n"
   
   "@ingroup GuiButtons"
);


//-----------------------------------------------------------------------------

GuiCheckBoxCtrl::GuiCheckBoxCtrl()
{
   setExtent(140, 30);
   mStateOn = false;
   mIndent = 0;
   mButtonType = ButtonTypeCheck;
}

//-----------------------------------------------------------------------------

bool GuiCheckBoxCtrl::onWake()
{
   if(!Parent::onWake())
      return false;

   // make sure there is a bitmap array for this control type
   // if it is declared as such in the control
   if( !mProfile->mBitmapArrayRects.size() && !mProfile->constructBitmapArray() )
   {
      Con::errorf( "GuiCheckBoxCtrl::onWake - failed to create bitmap array from profile '%s'", mProfile->getName() );
      return false;
   }

   return true;
}

//-----------------------------------------------------------------------------

void GuiCheckBoxCtrl::onRender(Point2I offset, const RectI &updateRect)
{
   // RLP/Sickhead NOTE: New/experimental code
   // for notifying the GuiCheckBoxCtrl of changes
   // to its mConsoleVariable's state.
   if ( mConsoleVariable[0] )
   {
      bool stateOn = Con::getBoolVariable( mConsoleVariable );
      if ( stateOn != mStateOn )
         mStateOn = stateOn;
   }

   ColorI backColor = mActive ? mProfile->mFillColor : mProfile->mFillColorNA;
   ColorI fontColor = mActive ? (mMouseOver ? mProfile->mFontColorHL : mProfile->mFontColor) : mProfile->mFontColorNA;
   ColorI insideBorderColor = isFirstResponder() ? mProfile->mBorderColorHL : mProfile->mBorderColor;

   // just draw the check box and the text:
   S32 xOffset = 0;
   GFX->getDrawUtil()->clearBitmapModulation();
   if(mProfile->mBitmapArrayRects.size() >= 4)
   {
      S32 index = mStateOn;
      if(!mActive)
      {
         if(mProfile->mBitmapArrayRects.size() >= 6)
         {
            // New style checkbox bitmap with 6 images
            index = 4 + mStateOn;
         }
         else
         {
            // Old style checkbox bitmap
            index = 2;
         }
      }
      else if(mDepressed)
      {
         index += 2;
      }
      xOffset = mProfile->mBitmapArrayRects[0].extent.x + 2 + mIndent;
      S32 y = (getHeight() - mProfile->mBitmapArrayRects[0].extent.y) / 2;
      GFX->getDrawUtil()->drawBitmapSR(mProfile->mTextureObject, offset + Point2I(mIndent, y), mProfile->mBitmapArrayRects[index]);
   }
   
   if(mButtonText[0] != '\0')
   {
	  GFX->getDrawUtil()->setBitmapModulation( fontColor );
      renderJustifiedText(Point2I(offset.x + xOffset, offset.y),
                          Point2I(getWidth() - getHeight(), getHeight()),
                          mButtonText);
   }
   //render the children
   renderChildControls(offset, updateRect);
}

//-----------------------------------------------------------------------------

void GuiCheckBoxCtrl::autoSize()
{
   U32 width, height;
   U32 bmpArrayRect0Width = 0;
   
   if( !mAwake )
   {
      mProfile->incLoadCount();
            
      if( !mProfile->mBitmapArrayRects.size() )
         mProfile->constructBitmapArray();
      if( mProfile->mBitmapArrayRects.size() )
         bmpArrayRect0Width = mProfile->mBitmapArrayRects[ 0 ].extent.x;
   }

   U32 bmpWidth = bmpArrayRect0Width + 2 + mIndent;
   U32 strWidth = mProfile->mFont->getStrWidthPrecise( mButtonText );

   width = bmpWidth + strWidth + 2;

   U32 bmpHeight = mProfile->mBitmapArrayRects[0].extent.y;
   U32 fontHeight = mProfile->mFont->getHeight();

   height = getMax( bmpHeight, fontHeight ) + 4;

   setExtent( width, height );
   
   if( !mAwake )
      mProfile->decLoadCount();
}

//=============================================================================
//    API.
//=============================================================================
// MARK: ---- API ----

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiCheckBoxCtrl, setStateOn, void, ( bool newState ),,
   "Set whether the checkbox is ticked or not.\n"
   "@param newState If true the box will be checked, if false, it will be unchecked.\n\n"
   "@note This method will @b not trigger the command associated with the control.  To toggle the "
      "checkbox state as if the user had clicked the control, use performClick()." )
{
   object->setStateOn( newState );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiCheckBoxCtrl, isStateOn, bool, (),,
   "Test whether the checkbox is currently checked.\n"
   "@return True if the checkbox is currently ticked, false otherwise.\n" )
{
   return object->getStateOn();
}
