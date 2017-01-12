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
#include "gui/buttons/guiToggleButtonCtrl.h"

#include "console/console.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "gfx/gfxDevice.h"
#include "gfx/gfxDrawUtil.h"
#include "gui/core/guiCanvas.h"
#include "gui/core/guiDefaultControlRender.h"

IMPLEMENT_CONOBJECT(GuiToggleButtonCtrl);

ConsoleDocClass( GuiToggleButtonCtrl,
   "@brief Deprecated gui control.\n\n"
   
   "@deprecated GuiToggleButtonCtrl's functionality is solely based on GuiButtonBaseCtrl's ButtonTypeCheck type.\n\n"
   
   "@see GuiButtonCtrl\n"
   "@see GuiCheckBoxCtrl\n"
   
   "@ingroup GuiButtons"
);

//-----------------------------------------------------------------------------

GuiToggleButtonCtrl::GuiToggleButtonCtrl()
{
   setExtent(140, 30);
   mButtonText = StringTable->EmptyString();
   mStateOn = false;
   mButtonType = ButtonTypeCheck;
}

void GuiToggleButtonCtrl::onPreRender()
{
   Parent::onPreRender();

   // If we have a script variable, make sure we're in sync
   if ( mConsoleVariable[0] )
      mStateOn = Con::getBoolVariable( mConsoleVariable );
}

void GuiToggleButtonCtrl::onRender(Point2I      offset,
                                   const RectI& updateRect)
{
   bool highlight = mMouseOver;
   bool depressed = mDepressed;

   ColorI fontColor   = mActive ? ( highlight ? mProfile->mFontColorHL : mProfile->mFontColor ) : mProfile->mFontColorNA;
   ColorI fillColor   = mActive ? ( highlight ? mProfile->mFillColorHL : mProfile->mFillColor ) : mProfile->mFillColorNA;
   ColorI borderColor = mActive ? ( highlight ? mProfile->mBorderColorHL : mProfile->mBorderColor ) : mProfile->mBorderColorNA;

   RectI boundsRect(offset, getExtent());

   if( !mHasTheme )
   {
      if( mProfile->mBorder != 0 )
         renderFilledBorder( boundsRect, borderColor, fillColor, mProfile->mBorderThickness );
      else
         GFX->getDrawUtil()->drawRectFill( boundsRect, fillColor );
   }
   else if( mHasTheme )
   {
      S32 indexMultiplier = 1;
      if ( !mActive )
         indexMultiplier = 4;
      else if ( mDepressed || mStateOn )
         indexMultiplier = 2;
      else if ( mMouseOver )
         indexMultiplier = 3;


      renderSizableBitmapBordersFilled( boundsRect, indexMultiplier, mProfile );
   }

   Point2I textPos = offset;
   if(depressed)
      textPos += Point2I(1,1);

   GFX->getDrawUtil()->setBitmapModulation( fontColor );
   renderJustifiedText(textPos, getExtent(), mButtonText);

   //render the children
   renderChildControls( offset, updateRect);
}
