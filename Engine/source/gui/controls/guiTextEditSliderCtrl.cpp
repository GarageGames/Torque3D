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
#include "gui/controls/guiTextEditSliderCtrl.h"

#include "console/consoleTypes.h"
#include "console/console.h"
#include "gui/core/guiCanvas.h"
#include "gfx/gfxDevice.h"
#include "gfx/gfxDrawUtil.h"
#include "console/engineAPI.h"

IMPLEMENT_CONOBJECT(GuiTextEditSliderCtrl);

ConsoleDocClass( GuiTextEditSliderCtrl,
   "@brief GUI Control which displays a numerical value which can be increased or "
   "decreased using a pair of arrows.\n\n"

   "@tsexample\n"
   "new GuiTextEditSliderCtrl()\n"
   "{\n"
   "   format = \"%3.2f\";\n"
   "   range = \"-1e+03 1e+03\";\n"
   "   increment = \"0.1\";\n"
   "   focusOnMouseWheel = \"0\";\n"
   "   //Properties not specific to this control have been omitted from this example.\n"
   "};\n"
   "@endtsexample\n\n"

   "@see GuiTextEditCtrl\n\n"

   "@ingroup GuiCore\n"
);

GuiTextEditSliderCtrl::GuiTextEditSliderCtrl()
{
   mRange.set(0.0f, 1.0f);
   mIncAmount = 1.0f;
   mValue = 0.0f;
   mMulInc = 0;
   mIncCounter = 0.0f;
   mFormat = StringTable->insert("%3.2f");
   mTextAreaHit = None;
   mFocusOnMouseWheel = false;
}

GuiTextEditSliderCtrl::~GuiTextEditSliderCtrl()
{
}

void GuiTextEditSliderCtrl::initPersistFields()
{
   addField("format",    TypeString,  Offset(mFormat, GuiTextEditSliderCtrl), "Character format type to place in the control.\n");
   addField("range",     TypePoint2F, Offset(mRange, GuiTextEditSliderCtrl), "Maximum vertical and horizontal range to allow in the control.\n");
   addField("increment", TypeF32,     Offset(mIncAmount,     GuiTextEditSliderCtrl), "How far to increment the slider on each step.\n");
   addField("focusOnMouseWheel", TypeBool, Offset(mFocusOnMouseWheel, GuiTextEditSliderCtrl), "If true, the control will accept giving focus to the user when the mouse wheel is used.\n");

   Parent::initPersistFields();
}

void GuiTextEditSliderCtrl::getText(char *dest)
{
   Parent::getText(dest);
}

void GuiTextEditSliderCtrl::setText(const char *txt)
{
   mValue = dAtof(txt);
   checkRange();
   setValue();
}

bool GuiTextEditSliderCtrl::onKeyDown(const GuiEvent &event)
{
   return Parent::onKeyDown(event);
}

void GuiTextEditSliderCtrl::checkRange()
{
   if(mValue < mRange.x)
      mValue = mRange.x;
   else if(mValue > mRange.y)
      mValue = mRange.y;
}

void GuiTextEditSliderCtrl::setValue()
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

void GuiTextEditSliderCtrl::onMouseDown(const GuiEvent &event)
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

void GuiTextEditSliderCtrl::onMouseDragged(const GuiEvent &event)
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

void GuiTextEditSliderCtrl::onMouseUp(const GuiEvent &event)
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
      selectAllText();

  //if we released the mouse within this control, then the parent will call
  //the mConsoleCommand other wise we have to call it.
   Parent::onMouseUp(event);

   //if we didn't release the mouse within this control, then perform the action
   // if (!cursorInControl())
   execConsoleCallback();   
   execAltConsoleCallback();

   //Set the cursor position to where the user clicked
   mCursorPos = calculateCursorPos( event.mousePoint );

   mTextAreaHit = None;
}

bool GuiTextEditSliderCtrl::onMouseWheelUp(const GuiEvent &event)
{
   if ( !mActive || !mAwake || !mVisible )
      return Parent::onMouseWheelUp(event);

   if ( !isFirstResponder() && !mFocusOnMouseWheel )
   {
      GuiControl *parent = getParent();
      if ( parent )
         return parent->onMouseWheelUp( event );

      return false;
   }

   mValue += mIncAmount;

   checkRange();
   setValue();
   
   setFirstResponder();
   mCursorPos = mBlockStart = mBlockEnd = 0;
   setUpdate();

   return true;
}

bool GuiTextEditSliderCtrl::onMouseWheelDown(const GuiEvent &event)
{
   if ( !mActive || !mAwake || !mVisible )
      return Parent::onMouseWheelDown(event);

   if ( !isFirstResponder() && !mFocusOnMouseWheel )
   {
      GuiControl *parent = getParent();
      if ( parent )
         return parent->onMouseWheelUp( event );

      return false;
   }   

   mValue -= mIncAmount;

   checkRange();
   setValue();

   setFirstResponder();
   mCursorPos = mBlockStart = mBlockEnd = 0;
   setUpdate();

   return true;
}

void GuiTextEditSliderCtrl::checkIncValue()
{
   if(mMulInc > 1.0f)
      mMulInc = 1.0f;
   else if(mMulInc < -1.0f)
      mMulInc = -1.0f;
}

void GuiTextEditSliderCtrl::timeInc(U32 elapseTime)
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

void GuiTextEditSliderCtrl::onRender(Point2I offset, const RectI &updateRect)
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

   Point2I start(offset.x + getWidth() - 14, offset.y);
   Point2I midPoint(start.x + 7, start.y + (getExtent().y/2));

   GFX->getDrawUtil()->drawRectFill(Point2I(start.x+1,start.y+1), Point2I(start.x+13,start.y+getExtent().y-1) , mProfile->mFillColor);

   GFX->getDrawUtil()->drawLine(start, Point2I(start.x, start.y+getExtent().y),mProfile->mFontColor);
   GFX->getDrawUtil()->drawLine(Point2I(start.x,midPoint.y),
               Point2I(start.x+14,midPoint.y),
               mProfile->mFontColor);

   GFXVertexBufferHandle<GFXVertexPCT> verts(GFX, 6, GFXBufferTypeVolatile);
   verts.lock();

   verts[0].color.set( 0, 0, 0 );
	verts[1].color.set( 0, 0, 0 );
	verts[2].color.set( 0, 0, 0 );
	verts[3].color.set( 0, 0, 0 );
	verts[4].color.set( 0, 0, 0 );
	verts[5].color.set( 0, 0, 0 );

   if(mTextAreaHit == ArrowUp)
   {
      verts[0].point.set( (F32)midPoint.x, (F32)start.y + 1.0f, 0.0f );
      verts[1].point.set( (F32)start.x + 11.0f, (F32)midPoint.y - 2.0f, 0.0f );
      verts[2].point.set( (F32)start.x + 3.0f, (F32)midPoint.y - 2.0f, 0.0f );
   }
   else
   {
      verts[0].point.set( (F32)midPoint.x, (F32)start.y + 2.0f, 0.0f );
      verts[1].point.set( (F32)start.x + 11.0f, (F32)midPoint.y - 1.0f, 0.0f );
      verts[2].point.set( (F32)start.x + 3.0f, (F32)midPoint.y - 1.0f, 0.0f );
   }

   if(mTextAreaHit == ArrowDown)
   {
      verts[3].point.set( (F32)midPoint.x, (F32)(start.y + getExtent().y - 1), 0.0f );
      verts[4].point.set( (F32)start.x + 11.0f, (F32)midPoint.y + 3.0f, 0.0f );
      verts[5].point.set( (F32)start.x + 3.0f, (F32)midPoint.y + 3.0f, 0.0f );
   }
   else
   {
      verts[3].point.set( (F32)midPoint.x, (F32)(start.y + getExtent().y - 2), 0.0f );
      verts[4].point.set( (F32)start.x + 11.0f, (F32)midPoint.y + 2.0f, 0.0f );
      verts[5].point.set( (F32)start.x + 3.0f, (F32)midPoint.y + 2.0f, 0.0f );
   }

   verts.unlock();

   GFX->setVertexBuffer( verts );
   GFX->setupGenericShaders();
   GFX->drawPrimitive( GFXTriangleList, 0, 2 );
}

void GuiTextEditSliderCtrl::onPreRender()
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


