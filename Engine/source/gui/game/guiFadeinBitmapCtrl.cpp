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
#include "gui/game/guiFadeinBitmapCtrl.h"
#include "console/console.h"
#include "console/consoleTypes.h"
#include "gfx/gfxDrawUtil.h"
#include "math/mathTypes.h"
#include "console/engineAPI.h"

IMPLEMENT_CONOBJECT( GuiFadeinBitmapCtrl );

ConsoleDocClass( GuiFadeinBitmapCtrl,
   "@brief A GUI control which renders a black square over a bitmap image. The black square will fade out, then fade back in after a determined time.\n"
   "This control is especially useful for transitions and splash screens.\n\n"

   "@tsexample\n"
	"new GuiFadeinBitmapCtrl()\n"
	"	{\n"
    "		fadeinTime = \"1000\";\n"
    "		waitTime = \"2000\";\n"
    "		fadeoutTime = \"1000\";\n"
    "		done = \"1\";\n"
	"		// Additional GUI properties that are not specific to GuiFadeinBitmapCtrl have been omitted from this example.\n"
	"	};\n"
   "@endtsexample\n\n"

   "@see GuiBitmapCtrl\n\n"
   "@ingroup GuiCore\n"
);

IMPLEMENT_CALLBACK( GuiFadeinBitmapCtrl, click, void, (),(),
   "@brief Informs the script level that this object received a Click event from the cursor or keyboard.\n\n"
   "@tsexample\n"
   "GuiFadeInBitmapCtrl::click(%this)\n"
   "	{\n"
   "		// Code to run when click occurs\n"
   "	}\n"
   "@endtsexample\n\n"
   "@see GuiCore\n\n"
);

IMPLEMENT_CALLBACK( GuiFadeinBitmapCtrl, onDone, void, (),(),
   "@brief Informs the script level that this object has completed is fade cycle.\n\n"
   "@tsexample\n"
   "GuiFadeInBitmapCtrl::onDone(%this)\n"
   "	{\n"
   "		// Code to run when the fade cycle completes\n"
   "	}\n"
   "@endtsexample\n\n"
   "@see GuiCore\n\n"
);

//-----------------------------------------------------------------------------

GuiFadeinBitmapCtrl::GuiFadeinBitmapCtrl()
   : mFadeColor( 0.f, 0.f, 0.f ),
     mStartTime( 0 ),
     mFadeInTime( 1000 ),
     mWaitTime( 2000 ),
     mFadeOutTime( 1000 ),
     mDone( false )
{
}

//-----------------------------------------------------------------------------

void GuiFadeinBitmapCtrl::initPersistFields()
{
   addGroup( "Fading" );
   
      addField( "fadeColor", TypeColorF, Offset( mFadeColor, GuiFadeinBitmapCtrl ),
         "Color to fade in from and fade out to." );
      addField( "fadeInTime", TypeS32, Offset( mFadeInTime, GuiFadeinBitmapCtrl ),
         "Milliseconds for the bitmap to fade in." );
      addField( "waitTime", TypeS32, Offset( mWaitTime, GuiFadeinBitmapCtrl ),
         "Milliseconds to wait after fading in before fading out the bitmap." );
      addField( "fadeOutTime", TypeS32, Offset( mFadeOutTime, GuiFadeinBitmapCtrl ),
         "Milliseconds for the bitmap to fade out." );
      addField( "fadeInEase", TypeEaseF, Offset( mFadeInEase, GuiFadeinBitmapCtrl ),
         "Easing curve for fade-in." );
      addField( "fadeOutEase", TypeEaseF, Offset( mFadeOutEase, GuiFadeinBitmapCtrl ),
         "Easing curve for fade-out." );
      addField( "done", TypeBool, Offset( mDone, GuiFadeinBitmapCtrl ),
         "Whether the fade cycle has finished running." );
      
   endGroup( "Fading" );
   
   Parent::initPersistFields();
}

//-----------------------------------------------------------------------------

void GuiFadeinBitmapCtrl::onPreRender()
{
   Parent::onPreRender();
   setUpdate();
}

//-----------------------------------------------------------------------------

void GuiFadeinBitmapCtrl::onMouseDown(const GuiEvent &)
{
   click_callback();
}

//-----------------------------------------------------------------------------

bool GuiFadeinBitmapCtrl::onKeyDown(const GuiEvent &)
{
   click_callback();
   return true;
}

//-----------------------------------------------------------------------------

bool GuiFadeinBitmapCtrl::onWake()
{
   if( !Parent::onWake() )
      return false;
      
   // Reset reference time.
   mStartTime = 0;
   
   return true;
}

//-----------------------------------------------------------------------------

void GuiFadeinBitmapCtrl::onRender(Point2I offset, const RectI &updateRect)
{
   Parent::onRender(offset, updateRect);
   
   // Set reference time if we haven't already.  This is done here when rendering
   // starts so that we begin counting from the time the control is actually
   // visible rather than from its onWake() (which may be a considerable time
   // before the control actually gets to render).
   
   if( !mStartTime )
      mStartTime = Platform::getRealMilliseconds();
      
   // Compute overlay alpha.
   
   U32 elapsed = Platform::getRealMilliseconds() - mStartTime;

   U32 alpha;
   if( elapsed < mFadeInTime )
   {
      // fade-in
      alpha = 255.f * ( 1.0f - mFadeInEase.getValue( elapsed, 0.f, 1.f, mFadeInTime ) );
   }
   else if( elapsed < ( mFadeInTime + mWaitTime ) )
   {
      // wait
      alpha = 0;
   }
   else if( elapsed < ( mFadeInTime + mWaitTime + mFadeOutTime ) )
   {
      // fade out
      elapsed -= ( mFadeInTime + mWaitTime );
      alpha = mFadeOutEase.getValue( elapsed, 0.f, 255.f, mFadeOutTime );
   }
   else
   {
      // done state
      alpha = mFadeOutTime ? 255 : 0;
      mDone = true;
      
      // Trigger onDone callback except when in Gui Editor.

      if( !smDesignTime )
		  onDone_callback();
   }
   
   // Render overlay on top of bitmap.
   
   ColorI color = mFadeColor;
   color.alpha = alpha;
   
   GFX->getDrawUtil()->drawRectFill( offset, getExtent() + offset, color );
}
