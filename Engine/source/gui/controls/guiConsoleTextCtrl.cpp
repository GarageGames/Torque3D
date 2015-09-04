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
#include "gui/controls/guiConsoleTextCtrl.h"

#include "console/consoleTypes.h"
#include "console/console.h"
#include "core/color.h"
#include "gfx/gfxDrawUtil.h"
#include "gui/core/guiDefaultControlRender.h"

IMPLEMENT_CONOBJECT(GuiConsoleTextCtrl);

ConsoleDocClass( GuiConsoleTextCtrl,
   "@brief Used by GUIConsole system internally.\n\n"
   "@internal"
);

GuiConsoleTextCtrl::GuiConsoleTextCtrl()
{
}

GuiConsoleTextCtrl::~GuiConsoleTextCtrl()
{
}

void GuiConsoleTextCtrl::initPersistFields()
{
   addGroup("GuiConsoleTextCtrl");		
   addField("expression",  TypeRealString,  Offset(mConsoleExpression, GuiConsoleTextCtrl));
   endGroup("GuiConsoleTextCtrl");
   Parent::initPersistFields();
}

bool GuiConsoleTextCtrl::onWake()
{
   if (! Parent::onWake())
      return false;

   mFont = mProfile->mFont;
   return true;
}

void GuiConsoleTextCtrl::onSleep()
{
   Parent::onSleep();
   mFont = NULL;
}

void GuiConsoleTextCtrl::setText(const char *txt)
{
   //make sure we don't call this before onAdd();
   AssertFatal(mProfile, "Can't call setText() until setProfile() has been called.");

   if (txt)
      mConsoleExpression = txt;
   else
      mConsoleExpression = String::EmptyString;

   // make sure we have a font
   mProfile->incLoadCount();
   mFont = mProfile->mFont;

   setUpdate();

   // decrement the profile reference
   mProfile->decLoadCount();
}

void GuiConsoleTextCtrl::calcResize()
{
   if ( mResult.isEmpty() )
      return;

   // The width is the longest line.
   U32 ctrlWidth = 0;
   for ( U32 i = 0; i < mLineLen.size(); i++ )
   {
      U32 width = mFont->getStrNWidth( mResult.c_str() + mStartLineOffset[i], mLineLen[i] );

      if ( width > ctrlWidth )
         ctrlWidth = width;
   }

   // The height is the number of lines times the height of the font.
   U32 ctrlHeight = mLineLen.size() * mFont->getHeight();   

   setExtent( Point2I( ctrlWidth, ctrlHeight ) + mProfile->mTextOffset * 2 );
}


void GuiConsoleTextCtrl::onPreRender()
{   
   if ( mConsoleExpression.isNotEmpty() )
   {
      Con::evaluatef( "$guiConsoleTextCtrlTemp = %s;", mConsoleExpression.c_str() );
      
      //Fixes a bug with the above not always grabbing the console text.
      mResult = Con::getVariable("$guiConsoleTextCtrlTemp");
      
      // Of the resulting string we will be printing,
      // Find the number of lines and length of each.      
      mProfile->mFont->wrapString( mResult, U32_MAX, mStartLineOffset, mLineLen );
   }
   else
      mResult = String::EmptyString;         

   calcResize();
}

void GuiConsoleTextCtrl::onRender( Point2I offset, const RectI &updateRect )
{
   RectI ctrlRect( offset, getExtent() );

   // if opaque, fill the update rect with the fill color
   if ( mProfile->mOpaque )
      GFX->getDrawUtil()->drawRectFill( ctrlRect, mProfile->mFillColor );

   // if there's a border, draw the border
   if ( mProfile->mBorder )
      renderBorder( ctrlRect, mProfile );

   // If we have text to render.
   if ( mResult.isNotEmpty() )
   {
      GFont *font = mProfile->mFont;      
      
      GFX->getDrawUtil()->setBitmapModulation( mProfile->mFontColor );

      for ( U32 i = 0; i < mLineLen.size(); i++ )
      {      
         Point2I tempOffset = offset; 
         tempOffset += mProfile->mTextOffset;
         tempOffset.y += i * font->getHeight();
         
         const UTF8 *line = mResult.c_str() + mStartLineOffset[i];
         U32 lineLen = mLineLen[i];
         GFX->getDrawUtil()->drawTextN( font, tempOffset, line, lineLen, mProfile->mFontColors );
      }
   }

   // render the child controlsmResult
   renderChildControls(offset, updateRect);
}

const char *GuiConsoleTextCtrl::getScriptValue()
{
   return getText();
}

void GuiConsoleTextCtrl::setScriptValue(const char *val)
{
   setText(val);
}
