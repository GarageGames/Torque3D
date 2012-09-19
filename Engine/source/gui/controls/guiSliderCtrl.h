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

#ifndef _GUISLIDERCTRL_H_
#define _GUISLIDERCTRL_H_

#ifndef _GUICONTROL_H_
#include "gui/core/guiControl.h"
#endif


/// A slider control that selects out of a floating-point value range.
class GuiSliderCtrl : public GuiControl
{
   public:
   
      typedef GuiControl Parent;

   protected:

      Point2F mRange;
      U32  mTicks;
      bool mSnap;
      F32  mValue;
      RectI   mThumb;
      Point2I mThumbSize;
      S32 mShiftPoint;
      S32 mShiftExtent;
      F32 mIncAmount;
      bool mDisplayValue;
      bool mDepressed;
      bool mMouseOver;
      bool mMouseDragged;
      bool mHasTexture;

      enum
      {
         SliderLineLeft = 0,
         SliderLineCenter,
         SliderLineRight,
         SliderButtonNormal,
         SliderButtonHighlight,
         NumBitmaps
      };
         RectI *mBitmapBounds;

      F32 _getThumbValue( const GuiEvent& event );
      void _updateThumb( F32 value, bool snap = true, bool onWake = false, bool doCallback = true );
      
      /// @name Callbacks
      /// @{
      
      DECLARE_CALLBACK( void, onMouseDragged, () );
      
      /// @}
      
      static bool _setValue( void* object, const char* index, const char* data ) { static_cast< GuiSliderCtrl* >( object )->setValue( dAtof( data ) ); return false; }

   public:
         
      GuiSliderCtrl();
      
      bool isThumbBeingDragged() const { return mDepressed; }
         
      const Point2F& getRange() const { return mRange; }

      // GuiControl.
      bool onWake();

      void onMouseDown(const GuiEvent &event);
      void onMouseDragged(const GuiEvent &event);
      void onMouseUp(const GuiEvent &);
      void onMouseLeave(const GuiEvent &);
      void onMouseEnter(const GuiEvent &);
      bool onMouseWheelUp(const GuiEvent &event);
      bool onMouseWheelDown(const GuiEvent &event);
      
      void setActive( bool value );

      F32 getValue() const { return mValue; }
      void setScriptValue(const char *val) { setValue(dAtof(val)); }
      void setValue(F32 val, bool doCallback=false);

      void onRender(Point2I offset, const RectI &updateRect);
      
      virtual bool resize( const Point2I& newSize, const Point2I& newExtent );
      virtual void parentResized( const RectI& oldParentRect, const RectI& newParentRect );

      static void initPersistFields();

      DECLARE_CONOBJECT(GuiSliderCtrl);
      DECLARE_CATEGORY( "Gui Values" );
      DECLARE_DESCRIPTION( "A control that implements a horizontal or vertical slider to\n"
                           "select/represent values in a certain range." )
};

#endif
