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

#include "console/console.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "gfx/gfxTextureManager.h"
#include "gui/controls/guiSliderCtrl.h"
#include "gui/core/guiDefaultControlRender.h"
#include "gfx/primBuilder.h"
#include "gfx/gfxDrawUtil.h"
#include "sfx/sfxSystem.h"
#include "sfx/sfxTrack.h"


IMPLEMENT_CONOBJECT( GuiSliderCtrl );

ConsoleDocClass( GuiSliderCtrl,
   "@brief A control that displays a value between its minimal and maximal bounds using a slider placed on a vertical "
   "or horizontal axis.\n\n"
   
   "A slider displays a value and allows that value to be changed by dragging a thumb control along the axis of the "
   "slider.  In this way, the value is changed between its allowed minimum and maximum.\n\n"
   
   "To hook up script code to the value changes of a slider, use the #command and #altCommand properties.  #command is "
   "executed once the thumb is released by the user whereas #altCommand is called any time the slider value changes. "
   "When changing the slider value from script, however, trigger of #altCommand is suppressed by default.\n\n"
   
   "The orientation of a slider is automatically determined from the ratio of its width to its height.  If a slider is "
   "taller than it is wide, it will be rendered with a vertical orientation.  If it is wider than it is tall, it will be "
   "rendered with a horizontal orientation.\n\n"
   
   "The rendering of a slider depends on the bitmap in the slider's profile.  This bitmap must be a bitmap array comprised "
   "of at least five bitmap rectangles.  The rectangles are used such that:\n\n"
   
   "- Rectangle #1: Left edge of slider\n"
   "- Rectangle #2: Center piece of slider; this is stretched between the left and right edge\n"
   "- Rectangle #3: Right edge of slider\n"
   "- Rectangle #4: Thumb button in normal state\n"
   "- Rectangle #5: Thumb button in highlighted (mouse-over) state\n\n"
   
   "@tsexample\n"
      "// Create a sound source and a slider that changes the volume of the source.\n"
      "\n"
      "%source = sfxPlayOnce( \"art/sound/testing\", AudioLoop2D );\n"
      "\n"
      "new GuiSlider()\n"
      "{\n"
      "   // Update the sound source volume when the slider is being dragged and released.\n"
      "   command = %source @ \".setVolume( $ThisControl.value );\";\n"
      "\n"
      "   // Limit the range to 0..1 since that is the allowable range for sound volumes.\n"
      "   range = \"0 1\";\n"
      "};\n"
   "@endtsexample\n\n"
   
   "@see GuiTextEditSliderCtrl\n"
   "@see GuiTextEditSliderBitmapCtrl\n\n"
   
   "@ingroup GuiValues"
);


IMPLEMENT_CALLBACK( GuiSliderCtrl, onMouseDragged, void, (), (),
   "Called when the left mouse button is dragged across the slider." );


//----------------------------------------------------------------------------

GuiSliderCtrl::GuiSliderCtrl()
   : mRange( 0., 1.f ),
     mTicks( 10 ),
     mSnap( false ),
     mValue( 0.5f ),
     mThumbSize( 8, 20 ),
     mShiftPoint( 5 ),
     mShiftExtent( 10 ),
     mIncAmount( 0.f ),
     mDisplayValue( false ),
     mMouseOver( false ),
     mMouseDragged( false ),
     mDepressed( false )
{
}

//----------------------------------------------------------------------------

void GuiSliderCtrl::initPersistFields()
{
   addGroup( "Slider" );
   
      addField( "range", TypePoint2F, Offset( mRange, GuiSliderCtrl ),
         "Min and max values corresponding to left and right slider position." );
      addField( "ticks", TypeS32, Offset( mTicks, GuiSliderCtrl ),
         "Spacing between tick marks in pixels. 0=off." );
      addField( "snap",  TypeBool, Offset( mSnap,  GuiSliderCtrl ),
         "Whether to snap the slider to tick marks." );
      addProtectedField( "value", TypeF32, Offset( mValue, GuiSliderCtrl ),
         _setValue, defaultProtectedGetFn,
         "The value corresponding to the current slider position." );
      
   endGroup( "Slider" );

   Parent::initPersistFields();
}

//----------------------------------------------------------------------------

void GuiSliderCtrl::setValue(F32 val, bool doCallback)
{
   _updateThumb( val, mSnap, false, doCallback );
}

//----------------------------------------------------------------------------

void GuiSliderCtrl::setActive( bool value )
{
   if( !value && mDepressed )
   {
      // We're in the middle of a drag.  Finish it here as once we've
      // been deactivated, we are not going to see a mouse-up event.

      mDepressed = false;
      mouseUnlock();
      execConsoleCallback();
   }

   Parent::setActive( value );
}

//----------------------------------------------------------------------------

bool GuiSliderCtrl::onWake()
{
   if( !Parent::onWake() )
      return false;
      
   mHasTexture = mProfile->constructBitmapArray() >= NumBitmaps;  
	if( mHasTexture )
   {
      mBitmapBounds = mProfile->mBitmapArrayRects.address();
		mThumbSize = Point2I( mBitmapBounds[ SliderButtonNormal ].extent.x, mBitmapBounds[ SliderButtonNormal ].extent.y );
	}

   F32 value;
   if( mConsoleVariable[ 0 ] )
      value = getFloatVariable();
   else
      value = mValue;

   mValue = mClampF( value, mRange.x, mRange.y );

   // mouse scroll increment percentage is 5% of the range
   mIncAmount = ( ( mRange.y - mRange.x ) * 0.05 );

   if( ( mThumbSize.y + mProfile->mFont->getHeight() - 4 ) <= getExtent().y )
      mDisplayValue = true;
   else
      mDisplayValue = false;

   _updateThumb( mValue, mSnap, true );

   return true;
}

//----------------------------------------------------------------------------

void GuiSliderCtrl::onMouseDown(const GuiEvent &event)
{
   if ( !mActive || !mAwake || !mVisible )
      return;

   mouseLock();
   setFirstResponder();
   mDepressed = true;

   Point2I curMousePos = globalToLocalCoord( event.mousePoint );
   F32 value;
   if (getWidth() >= getHeight())
      value = F32(curMousePos.x-mShiftPoint) / F32(getWidth()-mShiftExtent)*(mRange.y-mRange.x) + mRange.x;
   else
      value = F32(curMousePos.y) / F32(getHeight())*(mRange.y-mRange.x) + mRange.x;
   
   _updateThumb( value, mSnap || ( event.modifier & SI_SHIFT ) );
}

//----------------------------------------------------------------------------

void GuiSliderCtrl::onMouseDragged( const GuiEvent &event )
{
   if ( !mActive || !mAwake || !mVisible )
      return;
      
   mMouseDragged = true;

   F32 value = _getThumbValue( event );
   _updateThumb( value, mSnap || ( event.modifier & SI_SHIFT ) );

   onMouseDragged_callback();
}

//----------------------------------------------------------------------------

void GuiSliderCtrl::onMouseUp( const GuiEvent& event )
{
   if ( !mActive || !mAwake || !mVisible )
      return;

	mouseUnlock();

   mDepressed = false;
   mMouseDragged = false;

   _updateThumb( _getThumbValue( event ), event.modifier & SI_SHIFT );
   
   execConsoleCallback();
}   

//----------------------------------------------------------------------------

void GuiSliderCtrl::onMouseEnter(const GuiEvent &event)
{
   setUpdate();
   if( isMouseLocked() )
   {
      mDepressed = true;
      mMouseOver = true;
   }
   else
   {
      if( mActive && mProfile->mSoundButtonOver )
      {
         //F32 pan = (F32(event.mousePoint.x)/F32(getRoot()->getWidth())*2.0f-1.0f)*0.8f;
         SFX->playOnce( mProfile->mSoundButtonOver );
      }
      
      mMouseOver = true;
   }
}

//----------------------------------------------------------------------------

void GuiSliderCtrl::onMouseLeave(const GuiEvent &)
{
   setUpdate();
   if( isMouseLocked() )
      mDepressed = false;
   mMouseOver = false;
}
//----------------------------------------------------------------------------

bool GuiSliderCtrl::onMouseWheelUp(const GuiEvent &event)
{
   if ( !mActive || !mAwake || !mVisible )
      return Parent::onMouseWheelUp(event);

   _updateThumb( mValue + mIncAmount, ( event.modifier & SI_SHIFT ) );
   execConsoleCallback();

   return true;
}

//----------------------------------------------------------------------------

bool GuiSliderCtrl::onMouseWheelDown(const GuiEvent &event)
{
   if ( !mActive || !mAwake || !mVisible )
      return Parent::onMouseWheelUp(event);

   _updateThumb( mValue - mIncAmount, ( event.modifier & SI_SHIFT ) );   
   execConsoleCallback();

   return true;
}

//----------------------------------------------------------------------------

void GuiSliderCtrl::_updateThumb( F32 _value, bool snap, bool onWake, bool doCallback )
{      
   if( snap && mTicks > 0 )
   {
      // If the shift key is held, snap to the nearest tick, if any are being drawn

      F32 tickStep = (mRange.y - mRange.x) / F32(mTicks + 1);

      F32 tickSteps = (_value - mRange.x) / tickStep;
      S32 actualTick = S32(tickSteps + 0.5);

      _value = actualTick * tickStep + mRange.x;
   }
   
   // Clamp the thumb to legal values.

   if( _value < mRange.x )
      _value = mRange.x;
   if( _value > mRange.y )
      _value = mRange.y;
      
   // If value hasn't changed and this isn't the initial update on
   // waking, do nothing.

   if( mValue == _value && !onWake )
      return;

   mValue = _value;

   Point2I ext = getExtent();
	ext.x -= ( mShiftExtent + mThumbSize.x ) / 2;
   // update the bounding thumb rect
   if (getWidth() >= getHeight())
   {  // HORZ thumb
      S32 mx = (S32)((F32(ext.x) * (mValue-mRange.x) / (mRange.y-mRange.x)));
      S32 my = ext.y/2;
      if(mDisplayValue)
         my = mThumbSize.y/2;

      mThumb.point.x  = mx - (mThumbSize.x/2);
      mThumb.point.y  = my - (mThumbSize.y/2);
      mThumb.extent   = mThumbSize;
   }
   else
   {  // VERT thumb
      S32 mx = ext.x/2;
      S32 my = (S32)((F32(ext.y) * (mValue-mRange.x) / (mRange.y-mRange.x)));
      mThumb.point.x  = mx - (mThumbSize.y/2);
      mThumb.point.y  = my - (mThumbSize.x/2);
      mThumb.extent.x = mThumbSize.y;
      mThumb.extent.y = mThumbSize.x;
   }
   
   setFloatVariable(mValue);
   setUpdate();

   // Use the alt console command if you want to continually update:
   if ( !onWake && doCallback )
      execAltConsoleCallback();
}

//----------------------------------------------------------------------------

void GuiSliderCtrl::onRender(Point2I offset, const RectI &updateRect)
{
   Point2I pos(offset.x+mShiftPoint, offset.y);
   Point2I ext(getWidth() - mShiftExtent, getHeight());
   RectI thumb = mThumb;

   GFXDrawUtil* drawUtil = GFX->getDrawUtil();

   if( mHasTexture )
   {
      if(mTicks > 0)
      {
         // TODO: tick marks should be positioned based on the bitmap dimensions.
         Point2I mid(ext.x, ext.y/2);
         Point2I oldpos = pos;
         pos += Point2I(1, 0);

         PrimBuild::color4f( 0.f, 0.f, 0.f, 1.f );
         PrimBuild::begin( GFXLineList, ( mTicks + 2 ) * 2 );
         // tick marks
         for (U32 t = 0; t <= (mTicks+1); t++)
         {
            S32 x = (S32)(F32(mid.x+1)/F32(mTicks+1)*F32(t)) + pos.x;
            S32 y = pos.y + mid.y;
            PrimBuild::vertex2i(x, y + mShiftPoint);
            PrimBuild::vertex2i(x, y + mShiftPoint*2 + 2);
         }
         PrimBuild::end();
         // TODO: it would be nice, if the primitive builder were a little smarter,
         // so that we could change colors midstream.
         PrimBuild::color4f(0.9f, 0.9f, 0.9f, 1.0f);
         PrimBuild::begin( GFXLineList, ( mTicks + 2 ) * 2 );
         // tick marks
         for (U32 t = 0; t <= (mTicks+1); t++)
         {
            S32 x = (S32)(F32(mid.x+1)/F32(mTicks+1)*F32(t)) + pos.x + 1;
            S32 y = pos.y + mid.y + 1;
            PrimBuild::vertex2i(x, y + mShiftPoint );
            PrimBuild::vertex2i(x, y + mShiftPoint * 2 + 3);
         }
         PrimBuild::end();
         pos = oldpos;
      }

      S32 index = SliderButtonNormal;
      if(mMouseOver)
         index = SliderButtonHighlight;
      drawUtil->clearBitmapModulation();

      //left border
      drawUtil->drawBitmapSR(mProfile->mTextureObject, Point2I(offset.x,offset.y), mBitmapBounds[SliderLineLeft]);
      //right border
      drawUtil->drawBitmapSR(mProfile->mTextureObject, Point2I(offset.x + getWidth() - mBitmapBounds[SliderLineRight].extent.x, offset.y), mBitmapBounds[SliderLineRight]);


      //draw our center piece to our slider control's border and stretch it
      RectI destRect;	
      destRect.point.x = offset.x + mBitmapBounds[SliderLineLeft].extent.x;
      destRect.extent.x = getWidth() - mBitmapBounds[SliderLineLeft].extent.x - mBitmapBounds[SliderLineRight].extent.x;
      destRect.point.y = offset.y;
      destRect.extent.y = mBitmapBounds[SliderLineCenter].extent.y;

      RectI stretchRect;
      stretchRect = mBitmapBounds[SliderLineCenter];
      stretchRect.inset(1,0);

      drawUtil->drawBitmapStretchSR(mProfile->mTextureObject, destRect, stretchRect);

      //draw our control slider button	
      thumb.point += pos;
      drawUtil->drawBitmapSR(mProfile->mTextureObject,Point2I(thumb.point.x,offset.y ),mBitmapBounds[index]);

   }
   else if (getWidth() >= getHeight())
   {
      Point2I mid(ext.x, ext.y/2);
      if(mDisplayValue)
         mid.set(ext.x, mThumbSize.y/2);

      PrimBuild::color4f( 0.f, 0.f, 0.f, 1.f );
      PrimBuild::begin( GFXLineList, ( mTicks + 2 ) * 2 + 2);
         // horz rule
         PrimBuild::vertex2i( pos.x, pos.y + mid.y );
         PrimBuild::vertex2i( pos.x + mid.x, pos.y + mid.y );

         // tick marks
         for( U32 t = 0; t <= ( mTicks + 1 ); t++ )
         {
            S32 x = (S32)( F32( mid.x - 1 ) / F32( mTicks + 1 ) * F32( t ) );
            PrimBuild::vertex2i( pos.x + x, pos.y + mid.y - mShiftPoint );
            PrimBuild::vertex2i( pos.x + x, pos.y + mid.y + mShiftPoint );
         }
         PrimBuild::end();
   }
   else
   {
      Point2I mid(ext.x/2, ext.y);

      PrimBuild::color4f( 0.f, 0.f, 0.f, 1.f );
      PrimBuild::begin( GFXLineList, ( mTicks + 2 ) * 2 + 2);
         // horz rule
         PrimBuild::vertex2i( pos.x + mid.x, pos.y );
         PrimBuild::vertex2i( pos.x + mid.x, pos.y + mid.y );

         // tick marks
         for( U32 t = 0; t <= ( mTicks + 1 ); t++ )
         {
            S32 y = (S32)( F32( mid.y - 1 ) / F32( mTicks + 1 ) * F32( t ) );
            PrimBuild::vertex2i( pos.x + mid.x - mShiftPoint, pos.y + y );
            PrimBuild::vertex2i( pos.x + mid.x + mShiftPoint, pos.y + y );
         }
         PrimBuild::end();
      mDisplayValue = false;
   }
   // draw the thumb
   thumb.point += pos;
   renderRaisedBox(thumb, mProfile);

   if(mDisplayValue)
   {
   	char buf[20];
  		dSprintf(buf,sizeof(buf),"%0.3f",mValue);

   	Point2I textStart = thumb.point;

      S32 txt_w = mProfile->mFont->getStrWidth((const UTF8 *)buf);

   	textStart.x += (S32)((thumb.extent.x/2.0f));
   	textStart.y += thumb.extent.y - 2; //19
   	textStart.x -=	(txt_w/2);
   	if(textStart.x	< offset.x)
   		textStart.x = offset.x;
   	else if(textStart.x + txt_w > offset.x+getWidth())
   		textStart.x -=((textStart.x + txt_w) - (offset.x+getWidth()));

    	drawUtil->setBitmapModulation(mProfile->mFontColor);
    	drawUtil->drawText(mProfile->mFont, textStart, buf, mProfile->mFontColors);
   }
   renderChildControls(offset, updateRect);
}

//----------------------------------------------------------------------------

bool GuiSliderCtrl::resize( const Point2I& newPosition, const Point2I& newSize )
{
   if( !Parent::resize( newPosition, newSize ) )
      return false;
   
   _updateThumb( mValue, false, true, false );
   return true;
}

//----------------------------------------------------------------------------

void GuiSliderCtrl::parentResized( const RectI& oldParentRect, const RectI& newParentRect )
{
   Parent::parentResized( oldParentRect, newParentRect );
   
   _updateThumb( mValue, false, true, false );
}

//----------------------------------------------------------------------------

F32 GuiSliderCtrl::_getThumbValue( const GuiEvent& event )
{
   Point2I curMousePos = globalToLocalCoord( event.mousePoint );

   F32 value;
   if( getWidth() >= getHeight() )
      value = F32( curMousePos.x - mShiftPoint ) / F32( getWidth() - mShiftExtent ) * ( mRange.y - mRange.x ) + mRange.x;
   else
      value = F32( curMousePos.y ) / F32( getHeight() ) * ( mRange.y - mRange.x ) + mRange.x;

   if(value > mRange.y )
      value = mRange.y;
   else if( value < mRange.x )
      value = mRange.x;

   if( mSnap || ( event.modifier & SI_SHIFT && mTicks >= 1 ) )
   {
      // If the shift key is held, snap to the nearest tick, if any are being drawn

      F32 tickStep = ( mRange.y - mRange.x ) / F32( mTicks + 1 );

      F32 tickSteps = (value - mRange.x ) / tickStep;
      S32 actualTick = S32( tickSteps + 0.5 );

      value = actualTick * tickStep + mRange.x;
      AssertFatal( value <= mRange.y && value >= mRange.x, "Error, out of bounds value generated from shift-snap of slider" );
   }

   return value;
}

//=============================================================================
//    Console Methods.
//=============================================================================

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiSliderCtrl, getValue, F32, (),,
   "Get the current value of the slider based on the position of the thumb.\n"
   "@return Slider position (from range.x to range.y)." )
{
   return object->getValue();
}

//----------------------------------------------------------------------------

DefineEngineMethod( GuiSliderCtrl, setValue, void, ( F32 pos, bool doCallback ), ( false ),
   "Set position of the thumb on the slider.\n"
   "@param pos New slider position (from range.x to range.y)\n"
   "@param doCallback If true, the altCommand callback will be invoked\n" )
{
   object->setValue( pos, doCallback );
}

//----------------------------------------------------------------------------

DefineEngineMethod( GuiSliderCtrl, isThumbBeingDragged, bool, (),,
   "Returns true if the thumb is currently being dragged by the user.  This method is mainly useful "
   "for scrubbing type sliders where the slider position is sync'd to a changing value.  When the "
   "user is dragging the thumb, however, the sync'ing should pause and not get in the way of the user." )
{
   return object->isThumbBeingDragged();
}
