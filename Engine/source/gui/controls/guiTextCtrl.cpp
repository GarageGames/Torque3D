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
#include "gui/controls/guiTextCtrl.h"

#include "gui/core/guiDefaultControlRender.h"
#include "console/consoleTypes.h"
#include "console/console.h"
#include "core/color.h"
#include "i18n/lang.h"
#include "gfx/gfxDrawUtil.h"
#include "console/engineAPI.h"

IMPLEMENT_CONOBJECT( GuiTextCtrl );

ConsoleDocClass( GuiTextCtrl,
   "@brief GUI control object this displays a single line of text, without TorqueML.\n\n"

   "@tsexample\n"
   "	new GuiTextCtrl()\n"
   "	{\n"
   "		text = \"Hello World\";\n"
   "		textID = \"\"STR_HELLO\"\";\n"
   "		maxlength = \"1024\";\n"
   "	    //Properties not specific to this control have been omitted from this example.\n"
   "	};\n"
   "@endtsexample\n\n"

   "@see GuiControl\n"
   "@see Localization\n\n"
   "@ingroup GuiCore\n"
);

GuiTextCtrl::GuiTextCtrl()
{
   //default fonts
   mInitialText = StringTable->insert("");
   mInitialTextID = StringTable->insert("");
   mText[0] = '\0';
   mMaxStrLen = GuiTextCtrl::MAX_STRING_LENGTH;
}

DefineEngineMethod( GuiTextCtrl, setText, void, (const char* text),,
   "@brief Sets the text in the control.\n\n"
   "@param text Text to display in the control.\n"
   "@tsexample\n"
   "// Set the text to show in the control\n"
   "%text = \"Gideon - Destroyer of World\";\n\n"
   "// Inform the GuiTextCtrl control to change its text to the defined value\n"
   "%thisGuiTextCtrl.setText(%text);\n"
   "@endtsexample\n\n"
   "@see GuiControl")
{
   object->setText( text );
}

DefineEngineMethod( GuiTextCtrl, setTextID, void, (const char* textID),,
   "@brief Maps the text ctrl to a variable used in localization, rather than raw text.\n\n"
   "@param textID Name of variable text should be mapped to\n"
   "@tsexample\n"
   "// Inform the GuiTextCtrl control of the textID to use\n"
   "%thisGuiTextCtrl.setTextID(\"STR_QUIT\");\n"
   "@endtsexample\n\n"
   "@see GuiControl"
   "@see Localization")
{
	object->setTextID( textID );
}

void GuiTextCtrl::initPersistFields()
{
   addProtectedField("text", TypeCaseString, Offset(mInitialText, GuiTextCtrl), setText, getTextProperty,
      "The text to show on the control.");

   addField( "textID",     TypeString,      Offset( mInitialTextID, GuiTextCtrl ),
      "Maps the text of this control to a variable used in localization, rather than raw text.");   

   addField( "maxLength",  TypeS32,         Offset( mMaxStrLen, GuiTextCtrl ),
      "Defines the maximum length of the text.  The default is 1024." );

   Parent::initPersistFields();    
}

bool GuiTextCtrl::onAdd()
{
   if(!Parent::onAdd())
      return false;
      
   dStrncpy(mText, (UTF8*)mInitialText, MAX_STRING_LENGTH);
   mText[MAX_STRING_LENGTH] = '\0';
   
   
   return true;
}

void GuiTextCtrl::inspectPostApply()
{
   Parent::inspectPostApply();
   if(mInitialTextID && *mInitialTextID != 0)
	   setTextID(mInitialTextID);
   else if( mConsoleVariable[ 0 ] )
      setText( getVariable() );
   else
      setText(mInitialText);
}

bool GuiTextCtrl::onWake()
{
   if ( !Parent::onWake() )
      return false;
   
   if( !mProfile->mFont )
   {
      Con::errorf( "GuiTextCtrl::onWake() - no valid font in profile '%s'", mProfile->getName() );
      return false;
   }
   if(mInitialTextID && *mInitialTextID != 0)
	   setTextID(mInitialTextID);

   if ( mConsoleVariable[0] )
   {
      const char *txt = Con::getVariable( mConsoleVariable );
      if ( txt )
      {
         if ( dStrlen( txt ) > mMaxStrLen )
         {
            char* buf = new char[mMaxStrLen + 1];
            dStrncpy( buf, txt, mMaxStrLen );
            buf[mMaxStrLen] = 0;
            setScriptValue( buf );
            delete [] buf;
         }
         else
            setScriptValue( txt );
      }
   }
   
   //resize
   autoResize();

   return true;
}

void GuiTextCtrl::autoResize()
{
   if( mProfile->mAutoSizeWidth || mProfile->mAutoSizeHeight)
   {
      if( !mProfile->mFont )
      {
         mProfile->loadFont();
         if( !mProfile->mFont )
            return;
      }
         
      Point2I newExtents = getExtent();
      if ( mProfile->mAutoSizeWidth )
         newExtents.x = mProfile->mFont->getStrWidth((const UTF8 *) mText );
      if ( mProfile->mAutoSizeHeight )
         newExtents.y = mProfile->mFont->getHeight() + 4;

      setExtent( newExtents );
   }
}

void GuiTextCtrl::setText(const char *txt)
{
   //make sure we don't call this before onAdd();
   if( !mProfile )
      return;
   
   if (txt)
      dStrncpy(mText, (UTF8*)txt, MAX_STRING_LENGTH);
   mText[MAX_STRING_LENGTH] = '\0';
   
   setVariable((char*)mText);
   setUpdate();
   
   autoResize();
} 

void GuiTextCtrl::setTextID(const char *id)
{
	S32 n = Con::getIntVariable(id, -1);
	if(n != -1)
	{
		mInitialTextID = StringTable->insert(id);
		setTextID(n);
	}
}
void GuiTextCtrl::setTextID(S32 id)
{
	const UTF8 *str = getGUIString(id);
	if(str)
		setText((const char*)str);
	//mInitialTextID = id;
}

void GuiTextCtrl::onPreRender()
{
   Parent::onPreRender();
   
   const char * var = getVariable();
   if(var && var[0] && dStricmp((char*)mText, var))
      setText(var);
}

void GuiTextCtrl::onRender(Point2I offset, const RectI &updateRect)
{
   renderBorder( RectI( offset, getExtent() ), mProfile );

   GFX->getDrawUtil()->setBitmapModulation( mProfile->mFontColor );
   renderJustifiedText(offset, getExtent(), (char*)mText);

   //render the child controls
   renderChildControls(offset, updateRect);
}

const char *GuiTextCtrl::getScriptValue()
{
   return getText();
}

void GuiTextCtrl::setScriptValue(const char *val)
{
   setText(val);
}
