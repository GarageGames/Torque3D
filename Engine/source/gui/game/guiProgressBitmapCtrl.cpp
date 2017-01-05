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
#include "gui/game/guiProgressBitmapCtrl.h"

#include "console/console.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "gfx/gfxDrawUtil.h"


IMPLEMENT_CONOBJECT( GuiProgressBitmapCtrl );

ConsoleDocClass( GuiProgressBitmapCtrl,
   "@brief A horizontal progress bar rendered from a repeating image.\n\n"
   
   "This class is used give progress feedback to the user.  Unlike GuiProgressCtrl which simply "
   "renders a filled rectangle, GuiProgressBitmapCtrl renders the bar using a bitmap.\n\n"
   
   "This bitmap can either be simple, plain image which is then stretched into the current extents of the bar "
   "as it fills up or it can be a bitmap array with three entries.  In the case of a bitmap array, the "
   "first entry in the array is used to render the left cap of the bar and the third entry in the array "
   "is used to render the right cap of the bar.  The second entry is streched in-between the two caps.\n\n"
   
   "@tsexample\n"
      "// This example shows one way to break down a long-running computation into phases\n"
      "// and incrementally update a progress bar between the phases.\n"
      "\n"
      "new GuiProgressBitmapCtrl( Progress )\n"
      "{\n"
      "   bitmap = \"core/art/gui/images/loading\";\n"
      "   extent = \"300 50\";\n"
      "   position = \"100 100\";\n"
      "};\n"
      "\n"
      "// Put the control on the canvas.\n"
      "%wrapper = new GuiControl();\n"
      "%wrapper.addObject( Progress );\n"
      "Canvas.pushDialog( %wrapper );\n"
      "\n"
      "// Start the computation.\n"
      "schedule( 1, 0, \"phase1\" );\n"
      "\n"
      "function phase1()\n"
      "{\n"
      "   Progress.setValue( 0 );\n"
      "\n"
      "   // Perform some computation.\n"
      "   //...\n"
      "\n"
      "   // Update progress.\n"
      "   Progress.setValue( 0.25 );\n"
      "\n"
      "   // Schedule next phase.  Don't call directly so engine gets a change to run refresh.\n"
      "   schedule( 1, 0, \"phase2\" );\n"
      "}\n"
      "\n"
      "function phase2()\n"
      "{\n"
      "   // Perform some computation.\n"
      "   //...\n"
      "\n"
      "   // Update progress.\n"
      "   Progress.setValue( 0.7 );\n"
      "\n"
      "   // Schedule next phase.  Don't call directly so engine gets a change to run refresh.\n"
      "   schedule( 1, 0, \"phase3\" );\n"
      "}\n"
      "\n"
      "function phase3()\n"
      "{\n"
      "   // Perform some computation.\n"
      "   //...\n"
      "\n"
      "   // Update progress.\n"
      "   Progress.setValue( 0.9 );\n"
      "\n"
      "   // Schedule next phase.  Don't call directly so engine gets a change to run refresh.\n"
      "   schedule( 1, 0, \"phase4\" );\n"
      "}\n"
      "\n"
      "function phase4()\n"
      "{\n"
      "   // Perform some computation.\n"
      "   //...\n"
      "\n"
      "   // Final update of progress.\n"
      "   Progress.setValue( 1.0 );\n"
      "}\n"
   "@endtsexample\n\n"
   
   "@see GuiProgressCtrl\n\n"
   
   "@ingroup GuiValues"
);


//-----------------------------------------------------------------------------

GuiProgressBitmapCtrl::GuiProgressBitmapCtrl()
   : mProgress( 0.f ),
     mBitmapName( StringTable->EmptyString() ),
     mUseVariable( false ),
     mTile( false )
{
}

//-----------------------------------------------------------------------------

void GuiProgressBitmapCtrl::initPersistFields()
{
   addProtectedField( "bitmap", TypeFilename, Offset( mBitmapName, GuiProgressBitmapCtrl ),
      _setBitmap, defaultProtectedGetFn,
      "~Path to the bitmap file to use for rendering the progress bar.\n\n"
      "If the profile assigned to the control already has a bitmap assigned, this property need not be "
      "set in which case the bitmap from the profile is used."
   );
   
   Parent::initPersistFields();
}

//-----------------------------------------------------------------------------

void GuiProgressBitmapCtrl::setBitmap( const char* name )
{
   bool awake = mAwake;
   if( awake )
      onSleep();

   mBitmapName = StringTable->insert( name );
   if( awake )
      onWake();
      
   setUpdate();
}

//-----------------------------------------------------------------------------

const char* GuiProgressBitmapCtrl::getScriptValue()
{
   static const U32 bufSize = 64;
   char * ret = Con::getReturnBuffer(bufSize);
   dSprintf(ret, bufSize, "%g", mProgress);
   return ret;
}

//-----------------------------------------------------------------------------

void GuiProgressBitmapCtrl::setScriptValue(const char *value)
{
   //set the value
   if (! value)
      mProgress = 0.0f;
   else
      mProgress = dAtof(value);

   //validate the value
   mProgress = mClampF(mProgress, 0.f, 1.f);
   setUpdate();
}

//-----------------------------------------------------------------------------

void GuiProgressBitmapCtrl::onPreRender()
{
   const char * var = getVariable();
   if(var)
   {
      F32 value = mClampF(dAtof(var), 0.f, 1.f);
      if(value != mProgress)
      {
         mProgress = value;
         setUpdate();
      }
   }
}

//-----------------------------------------------------------------------------

void GuiProgressBitmapCtrl::onRender(Point2I offset, const RectI &updateRect)
{	
	RectI ctrlRect(offset, getExtent());
	
	//grab lowest dimension
	if(getHeight() <= getWidth())
		mDim = getHeight();
	else
		mDim = getWidth();

   GFXDrawUtil* drawUtil = GFX->getDrawUtil();
	
	drawUtil->clearBitmapModulation();

	if(mNumberOfBitmaps == 1)
	{
		//draw the progress with image
		S32 width = (S32)((F32)(getWidth()) * mProgress);
		if (width > 0)
		{
			//drawing stretch bitmap
			RectI progressRect = ctrlRect;
			progressRect.extent.x = width;
			drawUtil->drawBitmapStretchSR(mProfile->mTextureObject, progressRect, mProfile->mBitmapArrayRects[0]);
		}
	}
	else if(mNumberOfBitmaps >= 3)
	{
		//drawing left-end bitmap
		RectI progressRectLeft(ctrlRect.point.x, ctrlRect.point.y, mDim, mDim);
		drawUtil->drawBitmapStretchSR(mProfile->mTextureObject, progressRectLeft, mProfile->mBitmapArrayRects[0]);

		//draw the progress with image
		S32 width = (S32)((F32)(getWidth()) * mProgress);
		if (width > mDim)
		{
			//drawing stretch bitmap
			RectI progressRect = ctrlRect;
			progressRect.point.x +=  mDim;
			progressRect.extent.x = (width - mDim - mDim);
			if (progressRect.extent.x < 0)
				progressRect.extent.x = 0;
			drawUtil->drawBitmapStretchSR(mProfile->mTextureObject, progressRect, mProfile->mBitmapArrayRects[1]);
		
			//drawing right-end bitmap
			RectI progressRectRight(progressRect.point.x + progressRect.extent.x, ctrlRect.point.y, mDim, mDim );
			drawUtil->drawBitmapStretchSR(mProfile->mTextureObject, progressRectRight, mProfile->mBitmapArrayRects[2]);
		}
	}
	else
		Con::warnf("guiProgressBitmapCtrl only processes an array of bitmaps == 1 or >= 3");

	//if there's a border, draw it
   if (mProfile->mBorder)
      drawUtil->drawRect(ctrlRect, mProfile->mBorderColor);

   Parent::onRender( offset, updateRect );

   //render the children
   renderChildControls(offset, updateRect);
	
}

//-----------------------------------------------------------------------------

bool GuiProgressBitmapCtrl::onWake()
{
   if(!Parent::onWake())
      return false;

	mNumberOfBitmaps = mProfile->constructBitmapArray();

   return true;
}

//=============================================================================
//    Console Methods.
//=============================================================================
// MARK: ---- Console Methods ----

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiProgressBitmapCtrl, setBitmap, void, ( const char* filename ),,
   "Set the bitmap to use for rendering the progress bar.\n\n"
   "@param filename ~Path to the bitmap file.\n\n"
   "@note Directly assign to #bitmap rather than using this method.\n\n"
   "@see GuiProgressBitmapCtrl::setBitmap" )
{
   object->setBitmap( filename );
}
