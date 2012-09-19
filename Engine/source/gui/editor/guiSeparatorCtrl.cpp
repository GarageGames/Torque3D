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
#include "gui/editor/guiSeparatorCtrl.h"

#include "gfx/gfxDevice.h"
#include "gfx/gfxDrawUtil.h"
#include "console/console.h"
#include "console/consoleTypes.h"
#include "gui/core/guiCanvas.h"
#include "gui/core/guiDefaultControlRender.h"

IMPLEMENT_CONOBJECT(GuiSeparatorCtrl);

ConsoleDocClass( GuiSeparatorCtrl,
	"@brief A control that renders a horizontal or vertical separator with "
	"an optional text label (horizontal only)\n\n"

	"@tsexample\n"
	"new GuiSeparatorCtrl()\n"
	"{\n"
	"   profile = \"GuiDefaultProfile\";\n"
	"   position = \"505 0\";\n"
	"   extent = \"10 17\";\n"
	"   minExtent = \"10 17\";\n"
	"   canSave = \"1\";\n"
	"   visible = \"1\";\n"
	"   horizSizing = \"left\";\n"
	"};\n"
	"@endtsexample\n\n"

	"@ingroup GuiControls\n");

ImplementEnumType( GuiSeparatorType,
   "GuiSeparatorCtrl orientations\n\n"
   "@ingroup GuiControls" )
   { GuiSeparatorCtrl::separatorTypeVertical, "Vertical"  },
   { GuiSeparatorCtrl::separatorTypeHorizontal,"Horizontal" }
EndImplementEnumType;


//--------------------------------------------------------------------------
GuiSeparatorCtrl::GuiSeparatorCtrl() : GuiControl()
{
   mInvisible = false;
   mTextLeftMargin = 0;
   mMargin = 2;
   setExtent( 12, 35 );
   mSeparatorType = GuiSeparatorCtrl::separatorTypeVertical;
}

//--------------------------------------------------------------------------
void GuiSeparatorCtrl::initPersistFields()
{
   addField("caption",        TypeRealString, Offset(mText,           GuiSeparatorCtrl),
      "Optional text label to display." );
   addField("type",           TYPEID< separatorTypeOptions >(), Offset(mSeparatorType,  GuiSeparatorCtrl),
      "Orientation of separator." );
   addField("borderMargin",   TypeS32,    Offset(mMargin,         GuiSeparatorCtrl));
   addField("invisible",      TypeBool,   Offset(mInvisible,      GuiSeparatorCtrl));// Nonsense.  Should use GuiControl's visibility.
   addField("leftMargin",     TypeS32,    Offset(mTextLeftMargin, GuiSeparatorCtrl),
      "Left margin of text label." );

   Parent::initPersistFields();
}

//--------------------------------------------------------------------------
void GuiSeparatorCtrl::onRender(Point2I offset, const RectI &updateRect)
{
   Parent::onRender( offset, updateRect );

   if( mInvisible )
      return;
      
   if( mText.isNotEmpty() && mSeparatorType != separatorTypeVertical )
   {
      // If text is present and we have a left margin, then draw some separator, then the
      // text, and then the rest of the separator.
      
      S32 posx = offset.x + mMargin;
      S32 fontheight = mProfile->mFont->getHeight();
      S32 seppos = (fontheight - 2) / 2 + offset.y;
      if( mTextLeftMargin > 0 )
      {
         RectI rect( Point2I( posx, seppos ), Point2I( mTextLeftMargin, 2 ) );
         renderSlightlyLoweredBox(rect, mProfile);
         posx += mTextLeftMargin;
      }

      GFX->getDrawUtil()->setBitmapModulation( mProfile->mFontColor );
      posx = GFX->getDrawUtil()->drawText(mProfile->mFont, Point2I(posx,offset.y), mText, mProfile->mFontColors);

      RectI rect( Point2I( posx, seppos ), Point2I( getWidth() - posx + offset.x, 2 ) );
      
      // Space text and separator a bit apart at right end.
      
      rect.point.x += 2;
      rect.extent.x -= 2;
      
      if( rect.extent.x > 0 )
         renderSlightlyLoweredBox( rect, mProfile );
   }
   else
   {
      if( mSeparatorType == separatorTypeHorizontal )
      {
         S32 seppos = getHeight() / 2 + offset.y; 
         RectI rect(Point2I(offset.x + mMargin ,seppos),Point2I(getWidth() - (mMargin * 2),2));
         renderSlightlyLoweredBox(rect, mProfile);
      }
      else
      {
         S32 seppos = getWidth() / 2 + offset.x; 
         RectI rect(Point2I(seppos, offset.y + mMargin),Point2I(2, getHeight() - (mMargin * 2)));
         renderSlightlyLoweredBox(rect, mProfile);
      }
   }

   renderChildControls(offset, updateRect);
}


