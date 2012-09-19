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
#include "gui/controls/guiTextEditSliderBitmapCtrl.h"

#include "console/consoleTypes.h"
#include "console/console.h"
#include "gui/core/guiCanvas.h"
#include "gfx/gfxDevice.h"
#include "gfx/gfxDrawUtil.h"


IMPLEMENT_CONOBJECT(GuiTextEditSliderBitmapCtrl);

ConsoleDocClass( GuiTextEditSliderBitmapCtrl,
   "@brief GUI Control which displays a numerical value which can be increased "
   "or decreased using a pair of bitmap up/down buttons. \n\n"

   "This control uses the bitmap specified in it's profile "
   "(GuiControlProfile::bitmapName). It takes this image and breaks up aspects "
   "of it to render the up and down arrows. It is also important to set "
   "GuiControlProfile::hasBitmapArray to true on the profile as well.\n\n"

   "The bitmap referenced should be broken up into a 1 x 4 grid (using the top "
   "left color pixel as a border color between each of the images) in which it "
   "will map to the following places:\n"
   "<ol>\n"
   "<li>Up arrow active</li>\n"
   "<li>Up arrow inactive</li>\n"
   "<li>Down arrow active</li>\n"
   "<li>Down arrow inactive</li>\n"
   "</ol>\n\n"

   "<pre>\n"
   "1\n"
   "2\n"
   "3\n"
   "4</pre>\n\n"

   "@tsexample\n"
   "singleton GuiControlProfile (SliderBitmapGUIProfile)\n"
   "{\n"
   "   bitmap = \"core/art/gui/images/sliderArray\";\n"
   "   hasBitmapArray = true;\n"
   "   opaque = false;\n"
   "};\n\n"

   "new GuiTextEditSliderBitmapCtrl()\n"
   "{\n"
   "   profile = \"SliderBitmapGUIProfile\";\n"
   "   format = \"%3.2f\";\n"
   "   range = \"-1e+03 1e+03\";\n"
   "   increment = \"0.1\";\n"
   "   focusOnMouseWheel = \"0\";\n"
   "   bitmap = \"\";\n"
   "   //Properties not specific to this control have been omitted from this example.\n"
   "};\n"
   "@endtsexample\n\n"

   "@see GuiTextEditSliderCtrl\n\n"
   "@see GuiTextEditCtrl\n\n"

   "@ingroup GuiCore\n"
);


GuiTextEditSliderBitmapCtrl::GuiTextEditSliderBitmapCtrl()
{
   mRange.set(0.0f, 1.0f);
   mIncAmount = 1.0f;
   mValue = 0.0f;
   mMulInc = 0;
   mIncCounter = 0.0f;
   mFormat = StringTable->insert("%3.2f");
   mTextAreaHit = None;
   mFocusOnMouseWheel = false;
   mBitmapName = StringTable->insert( "" );
}

GuiTextEditSliderBitmapCtrl::~GuiTextEditSliderBitmapCtrl()
{
}

void GuiTextEditSliderBitmapCtrl::initPersistFields()
{
   addField("format",    TypeString,  Offset(mFormat, GuiTextEditSliderBitmapCtrl), "Character format type to place in the control.\n");
   addField("range",     TypePoint2F, Offset(mRange, GuiTextEditSliderBitmapCtrl), "Maximum vertical and horizontal range to allow in the control.\n");
   addField("increment", TypeF32,     Offset(mIncAmount,     GuiTextEditSliderBitmapCtrl), "How far to increment the slider on each step.\n");
   addField("focusOnMouseWheel", TypeBool, Offset(mFocusOnMouseWheel, GuiTextEditSliderBitmapCtrl), "If true, the control will accept giving focus to the user when the mouse wheel is used.\n");
   addField("bitmap",    TypeFilename,Offset(mBitmapName, GuiTextEditSliderBitmapCtrl), "Unused" );

   Parent::initPersistFields();
}

void GuiTextEditSliderBitmapCtrl::getText(char *dest)
{
   Parent::getText(dest);
}

void GuiTextEditSliderBitmapCtrl::setText(const char *txt)
{
   mValue = dAtof(txt);
   checkRange();
   setValue();
}

bool GuiTextEditSliderBitmapCtrl::onKeyDown(const GuiEvent &event)
{
   return Parent::onKeyDown(event);
}

void GuiTextEditSliderBitmapCtrl::checkRange()
{
   if(mValue < mRange.x)
      mValue = mRange.x;
   else if(mValue > mRange.y)
      mValue = mRange.y;
}

void GuiTextEditSliderBitmapCtrl::setValue()
{
   char buf[20];
   // For some reason this sprintf is failing to convert
   // a floating point number to anything with %d, so cast it.
   if( dStricmp( mFormat, "%d" ) == 0 )
      dSprintf(buf,sizeof(buf),mFormat, (S32)mValue);
   else
      dSprintf(buf,sizeof(buf),mFormat, mValue);
   Parent::setText(buf);
}

void GuiTextEditSliderBitmapCtrl::onMouseDown(const GuiEvent &event)
{
   // If we're not active then skip out.
   if ( !mActive || !mAwake || !mVisible )
   {
      Parent::onMouseDown(event);
      return;
   }

   char txt[20];
   Parent::getText(txt);
   mValue = dAtof(txt);

   mMouseDownTime = Sim::getCurrentTime();
   GuiControl *parent = getParent();
   if(!parent)
      return;
   Point2I camPos  = event.mousePoint;
   Point2I point = parent->localToGlobalCoord(getPosition());

   if(camPos.x > point.x + getExtent().x - 14)
   {
      if(camPos.y > point.y + (getExtent().y/2))
      {
         mValue -=mIncAmount;
         mTextAreaHit = ArrowDown;
         mMulInc = -0.15f;
      }
      else
      {
         mValue +=mIncAmount;
         mTextAreaHit = ArrowUp;
         mMulInc = 0.15f;
      }

      checkRange();
      setValue();
      mouseLock();

      // We should get the focus and set the 
      // cursor to the start of the text to 
      // mimic the standard Windows behavior.
      setFirstResponder();
      mCursorPos = mBlockStart = mBlockEnd = 0;
      setUpdate();

      return;
   }

   Parent::onMouseDown(event);
}

void GuiTextEditSliderBitmapCtrl::onMouseDragged(const GuiEvent &event)
{
   // If we're not active then skip out.
   if ( !mActive || !mAwake || !mVisible )
   {
      Parent::onMouseDragged(event);
      return;
   }

   if(mTextAreaHit == None || mTextAreaHit == Slider)
   {
      mTextAreaHit = Slider;
      GuiControl *parent = getParent();
      if(!parent)
         return;
      Point2I camPos = event.mousePoint;
      Point2I point = parent->localToGlobalCoord(getPosition());
      F32 maxDis = 100;
      F32 val;
      if(camPos.y < point.y)
      {
         if((F32)point.y < maxDis)
            maxDis = (F32)point.y;

         val = point.y - maxDis;
         
         if(point.y > 0)
            mMulInc= 1.0f-(((float)camPos.y - val) / maxDis);
         else
            mMulInc = 1.0f;
         
         checkIncValue();
         
         return;
      }
      else if(camPos.y > point.y + getExtent().y)
      {
         GuiCanvas *root = getRoot();
         val = (F32)(root->getHeight() - (point.y + getHeight()));
         if(val < maxDis)
            maxDis = val;
         if( val > 0)
            mMulInc= -(F32)(camPos.y - (point.y + getHeight()))/maxDis;
         else
            mMulInc = -1.0f;
         checkIncValue();
         return;
      }
      mTextAreaHit = None;
      Parent::onMouseDragged(event);
   }
}

void GuiTextEditSliderBitmapCtrl::onMouseUp(const GuiEvent &event)
{
   // If we're not active then skip out.
   if ( !mActive || !mAwake || !mVisible )
   {
      Parent::onMouseUp(event);
      return;
   }

   mMulInc = 0.0f;
   mouseUnlock();

   if ( mTextAreaHit != None )

  //if we released the mouse within this control, then the parent will call
  //the mConsoleCommand other wise we have to call it.
   Parent::onMouseUp(event);

   //if we didn't release the mouse within this control, then perform the action
   // if (!cursorInControl())
   execConsoleCallback();   

   execAltConsoleCallback();

   mTextAreaHit = None;
}

bool GuiTextEditSliderBitmapCtrl::onMouseWheelUp(const GuiEvent &event)
{
   if ( !mActive || !mAwake || !mVisible )
      return Parent::onMouseWheelUp(event);

   if ( !isFirstResponder() && !mFocusOnMouseWheel )
      return false;

   mValue += mIncAmount;

   checkRange();
   setValue();
   
   setFirstResponder();
   mCursorPos = mBlockStart = mBlockEnd = 0;
   setUpdate();

   return true;
}

bool GuiTextEditSliderBitmapCtrl::onMouseWheelDown(const GuiEvent &event)
{
   if ( !mActive || !mAwake || !mVisible )
      return Parent::onMouseWheelDown(event);

   if ( !isFirstResponder() && !mFocusOnMouseWheel )
      return false;

   mValue -= mIncAmount;

   checkRange();
   setValue();

   setFirstResponder();
   mCursorPos = mBlockStart = mBlockEnd = 0;
   setUpdate();

   return true;
}

void GuiTextEditSliderBitmapCtrl::checkIncValue()
{
   if(mMulInc > 1.0f)
      mMulInc = 1.0f;
   else if(mMulInc < -1.0f)
      mMulInc = -1.0f;
}

void GuiTextEditSliderBitmapCtrl::timeInc(U32 elapseTime)
{
   S32 numTimes = elapseTime / 750;
   if(mTextAreaHit != Slider && numTimes > 0)
   {
      if(mTextAreaHit == ArrowUp)
         mMulInc = 0.15f * numTimes;
      else
         mMulInc = -0.15f * numTimes;

      checkIncValue();
   }
}
bool GuiTextEditSliderBitmapCtrl::onWake()
{
   if(!Parent::onWake())
      return false;

	mNumberOfBitmaps = mProfile->constructBitmapArray();

   return true;
}

void GuiTextEditSliderBitmapCtrl::onPreRender()
{
   if (isFirstResponder())
   {
      U32 timeElapsed = Platform::getVirtualMilliseconds() - mTimeLastCursorFlipped;
      mNumFramesElapsed++;
      if ((timeElapsed > 500) && (mNumFramesElapsed > 3))
      {
         mCursorOn = !mCursorOn;
         mTimeLastCursorFlipped = Sim::getCurrentTime();
         mNumFramesElapsed = 0;
         setUpdate();
      }

      //update the cursor if the text is scrolling
      if (mDragHit)
      {
         if ((mScrollDir < 0) && (mCursorPos > 0))
         {
            mCursorPos--;
         }
         else if ((mScrollDir > 0) && (mCursorPos < (S32)dStrlen(mText)))
         {
            mCursorPos++;
         }
      }
   }
}

void GuiTextEditSliderBitmapCtrl::onRender(Point2I offset, const RectI &updateRect)
{
   if(mTextAreaHit != None)
   {
      U32 elapseTime = Sim::getCurrentTime() - mMouseDownTime;
      if(elapseTime > 750 || mTextAreaHit == Slider)
      {
         timeInc(elapseTime);
         mIncCounter += mMulInc;
         if(mIncCounter >= 1.0f || mIncCounter <= -1.0f)
         {
            mValue = (mMulInc > 0.0f) ? mValue+mIncAmount : mValue-mIncAmount;
            mIncCounter = (mIncCounter > 0.0f) ? mIncCounter-1 : mIncCounter+1;
            checkRange();
            setValue();
            mCursorPos = 0;
         }
      }
   }

	Parent::onRender(offset, updateRect);

	// Arrow placement coordinates
   Point2I arrowUpStart(offset.x + getWidth() - 14, offset.y + 1 );
   Point2I arrowUpEnd(13, getExtent().y/2);

	Point2I arrowDownStart(offset.x + getWidth() - 14, offset.y + 1 + getExtent().y/2);
	Point2I arrowDownEnd(13, getExtent().y/2);
	
	// Draw the line that splits the number and bitmaps
	GFX->getDrawUtil()->drawLine(Point2I(offset.x + getWidth() - 14 -2, offset.y + 1 ),
		Point2I(arrowUpStart.x -2, arrowUpStart.y + getExtent().y),
		mProfile->mBorderColor);

	GFX->getDrawUtil()->clearBitmapModulation();
	
	if(mNumberOfBitmaps == 0)
		Con::warnf("No image provided for GuiTextEditSliderBitmapCtrl; do not render");
	else
	{
		// This control needs 4 images in order to render correctly
		if(mTextAreaHit == ArrowUp)
			GFX->getDrawUtil()->drawBitmapStretchSR( mProfile->mTextureObject, RectI(arrowUpStart,arrowUpEnd), mProfile->mBitmapArrayRects[0] );
		else
			GFX->getDrawUtil()->drawBitmapStretchSR( mProfile->mTextureObject, RectI(arrowUpStart,arrowUpEnd), mProfile->mBitmapArrayRects[1] );

		if(mTextAreaHit == ArrowDown)
			GFX->getDrawUtil()->drawBitmapStretchSR( mProfile->mTextureObject, RectI(arrowDownStart,arrowDownEnd), mProfile->mBitmapArrayRects[2] );
		else
			GFX->getDrawUtil()->drawBitmapStretchSR( mProfile->mTextureObject, RectI(arrowDownStart,arrowDownEnd), mProfile->mBitmapArrayRects[3] );
	}
}

void GuiTextEditSliderBitmapCtrl::setBitmap(const char *name)
{
   bool awake = mAwake;
   if(awake)
      onSleep();

   mBitmapName = StringTable->insert(name);
   if(awake)
      onWake();
   setUpdate();
}