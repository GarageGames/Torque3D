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
#include "guiClockHud.h"

//-----------------------------------------------------------------------------

IMPLEMENT_CONOBJECT( GuiClockHud );

ConsoleDocClass( GuiClockHud,
   "@brief Basic HUD clock. Displays the current simulation time offset from some base.\n"

   "@tsexample\n"
		"\n new GuiClockHud()"
		"{\n"
		"	fillColor = \"0.0 1.0 0.0 1.0\"; // Fills with a solid green color\n"
		"	frameColor = \"1.0 1.0 1.0 1.0\"; // Solid white frame color\n"
		"	textColor = \"1.0 1.0 1.0 1.0\"; // Solid white text Color\n"
		"	showFill = \"true\";\n"
		"	showFrame = \"true\";\n"
		"};\n"
   "@endtsexample\n\n"

   "@ingroup GuiGame\n"
);

GuiClockHud::GuiClockHud()
{
   mShowFrame = mShowFill = true;
   mFillColor.set(0, 0, 0, 0.5);
   mFrameColor.set(0, 1, 0, 1);
   mTextColor.set( 0, 1, 0, 1 );

   mTimeOffset = 0;
}

void GuiClockHud::initPersistFields()
{
   addGroup("Misc");		
   addField( "showFill", TypeBool, Offset( mShowFill, GuiClockHud ), "If true, draws a background color behind the control.");
   addField( "showFrame", TypeBool, Offset( mShowFrame, GuiClockHud ), "If true, draws a frame around the control." );
   addField( "fillColor", TypeColorF, Offset( mFillColor, GuiClockHud ), "Standard color for the background of the control." );
   addField( "frameColor", TypeColorF, Offset( mFrameColor, GuiClockHud ), "Color for the control's frame." );
   addField( "textColor", TypeColorF, Offset( mTextColor, GuiClockHud ), "Color for the text on this control." );
   endGroup("Misc");

   Parent::initPersistFields();

   removeField( "controlFontColor" );

   removeField( "controlFillColor" );

   removeField( "backgroundColor" );

   removeField( "contextFontColor" );

   removeField( "contextBackColor" );

   removeField( "contextFillColor" );
}


void GuiClockHud::copyProfileSettings()
{
	if(!mProfileSettingsCopied)
	{
		mClockFillColorCopy = mFillColor;
		mClockFrameColorCopy = mFrameColor;
		mClockTextColorCopy = mTextColor;
		
		Parent::copyProfileSettings();
	}
}

//-----------------------------------------------------------------------------

void GuiClockHud::resetProfileSettings()
{
	mFillColor = mClockFillColorCopy;
	mFrameColor = mClockFrameColorCopy;
	mTextColor = mClockTextColorCopy;
	
	Parent::resetProfileSettings();
}

//-----------------------------------------------------------------------------

void GuiClockHud::applyProfileSettings()
{
   Parent::applyProfileSettings();

   /// Set the frame, fill and text alpha.
   if(mFillColor)
	   mFillColor.alpha = mClockFillColorCopy.alpha * mRenderAlpha;
   if(mFrameColor)
	   mFrameColor.alpha = mClockFrameColorCopy.alpha * mRenderAlpha;;
   if(mTextColor)
	   mTextColor.alpha = mClockTextColorCopy.alpha * mRenderAlpha;
}

//-----------------------------------------------------------------------------

void GuiClockHud::onStaticModified( const char *slotName, const char *newValue )
{
	if( !dStricmp( slotName, "fillColor") || !dStricmp( slotName, "frameColor") || !dStricmp( slotName, "textColor") )
	{
		ColorF color(1, 0, 0, 1);
		dSscanf( newValue, "%f %f %f %f", &color.red, &color.green, &color.blue, &color.alpha );
		
		if( !dStricmp( slotName, "fillColor") )
			mClockFillColorCopy = color;
		else if( !dStricmp( slotName, "frameColor") )
			mClockFrameColorCopy = color;
		else
			mClockTextColorCopy = color;
	}
}

//-----------------------------------------------------------------------------

void GuiClockHud::onRender(Point2I offset, const RectI &updateRect)
{
   // Background first
   if (mShowFill)
      GFX->getDrawUtil()->drawRectFill(updateRect, mFillColor);

   // Convert ms time into hours, minutes and seconds.
   S32 time = S32(getTime());
   S32 secs = time % 60;
   S32 mins = (time % 3600) / 60;

   // Currently only displays min/sec
   char buf[256];
   dSprintf(buf,sizeof(buf), "%02d:%02d",mins,secs);

   // Center the text
   offset.x += (getWidth() - mProfile->mFont->getStrWidth((const UTF8 *)buf)) / 2;
   offset.y += (getHeight() - mProfile->mFont->getHeight()) / 2;
   GFX->getDrawUtil()->setBitmapModulation(mTextColor);
   GFX->getDrawUtil()->drawText(mProfile->mFont, offset, buf);
   GFX->getDrawUtil()->clearBitmapModulation();

   // Border last
   if (mShowFrame)
      GFX->getDrawUtil()->drawRect(updateRect, mFrameColor);
}


//-----------------------------------------------------------------------------

void GuiClockHud::setReverseTime(F32 time)  
{  
   // Set the current time in seconds.  
   mTimeReversed = true;  
   mTimeOffset = S32(time * 1000) + Platform::getVirtualMilliseconds();  
}

void GuiClockHud::setTime(F32 time)
{
   // Set the current time in seconds.
   mTimeReversed = false;
   mTimeOffset = S32(time * 1000) - Platform::getVirtualMilliseconds();
}

F32 GuiClockHud::getTime()
{
   // Return elapsed time in seconds.
   if(mTimeReversed)
      return F32(mTimeOffset - Platform::getVirtualMilliseconds()) / 1000;  
   else
      return F32(mTimeOffset + Platform::getVirtualMilliseconds()) / 1000;
}

DefineEngineMethod(GuiClockHud, setTime, void, (F32 timeInSeconds),(60), "Sets the current base time for the clock.\n"
													"@param timeInSeconds Time to set the clock, in seconds (IE: 00:02 would be 120)\n"
													"@tsexample\n"
														"// Define the time, in seconds\n"
														"%timeInSeconds = 120;\n\n"
														"// Change the time on the GuiClockHud control\n"
														"%guiClockHud.setTime(%timeInSeconds);\n"
													"@endtsexample\n"
				  )
{
   object->setTime(timeInSeconds);
}

DefineEngineMethod(GuiClockHud, setReverseTime, void, (F32 timeInSeconds),(60), "@brief Sets a time for a countdown clock.\n\n"
   												"Setting the time like this will cause the clock to count backwards from the specified time.\n\n"
													"@param timeInSeconds Time to set the clock, in seconds (IE: 00:02 would be 120)\n\n"
													"@see setTime\n"
				  )
{
   object->setReverseTime(timeInSeconds);
}

DefineEngineMethod(GuiClockHud, getTime, F32, (),, "Returns the current time, in seconds.\n"
													"@return timeInseconds Current time, in seconds\n"
													"@tsexample\n"
														"// Get the current time from the GuiClockHud control\n"
														"%timeInSeconds = %guiClockHud.getTime();\n"
													"@endtsexample\n"
				  )
{
   return object->getTime();
}
