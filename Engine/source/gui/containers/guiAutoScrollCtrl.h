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

#ifndef _GUIAUTOSCROLLCTRL_H_
#define _GUIAUTOSCROLLCTRL_H_

#ifndef _GUICONTROL_H_
   #include "gui/core/guiControl.h"
#endif
#ifndef _GUITICKCTRL_H_
   #include "gui/shiny/guiTickCtrl.h"
#endif


/// A control that automatically scrolls its child control upwards.
class GuiAutoScrollCtrl : public GuiTickCtrl
{
   public:
   
      typedef GuiTickCtrl Parent;
      
      /// Scrolling direction.
      enum Direction
      {
         Up,
         Down,
         Left,
         Right
      };

   protected:
   
      enum Phase
      {
         PhaseInitial,     ///< Waiting to begin scrolling.
         PhaseScrolling,   ///< Currently scrolling.
         PhaseComplete,    ///< Scrolling complete.
         PhaseWait         ///< Wait before starting a new loop.
      };
      
      /// The direction in which to scroll.
      Direction mDirection;
         
      /// If true, scrolling will start from the beginning once finished.
      bool mIsLooping;
      
      /// Whether to scroll the child control completely out of sight.
      bool mScrollOutOfSight;
      
      /// Current phase in the scrolling animation.
      Phase mCurrentPhase;
      
      /// The current animation time.
      F32 mCurrentTime;
      
      /// The time scrolling was completed.
      F32 mCompleteTime;
      
      /// Current scrolling position.  This is kept separate from the control's
      /// current position value since we will receive time updates in increments
      /// less than a second and thus need to have this value in floating-point.
      F32 mCurrentPosition;

      /// Seconds to wait before starting to scroll.
      F32 mStartDelay;
      
      /// Seconds to wait after scrolling is complete before reseting the control
      /// to the initial state (only if #mIsLooping is true).
      F32 mResetDelay;
      
      /// Border to put around scrolled child control.
      S32 mChildBorder;
      
      /// Speed at which to scroll in pixels per second.
      F32 mScrollSpeed;
            
      /// @name Callbacks
      /// @{

      DECLARE_CALLBACK( void, onTick, () );
      DECLARE_CALLBACK( void, onStart, () );
      DECLARE_CALLBACK( void, onComplete, () );
      DECLARE_CALLBACK( void, onReset, () );
      
      /// @}

      void _reset( GuiControl* control );
      bool _isScrollComplete() const;
      
      U32 _getScrollAxis() const
      {
         switch( mDirection )
         {
            case Up:    return 1;
            case Down:  return 1;
            case Left:  return 0;
            case Right: return 0;
         }
         return 0;
      }
      
      F32 _getScrollAmount() const
      {
         switch( mDirection )
         {
            case Up:    return - mScrollSpeed;
            case Down:  return mScrollSpeed;
            case Left:  return - mScrollSpeed;
            case Right: return mScrollSpeed;
         }
         return 0.f;
      }

   public:
   
      GuiAutoScrollCtrl();
      
      void reset();
            
      virtual bool onWake();
      virtual void onSleep();

      virtual void onChildAdded( GuiControl* control );
      virtual void onChildRemoved( GuiControl* control );
      virtual bool resize( const Point2I& newPosition, const Point2I& newExtent );
      virtual void childResized( GuiControl *child );

      virtual void processTick();
      virtual void advanceTime( F32 timeDelta );
      virtual void inspectPostApply();

      static void initPersistFields();

      DECLARE_CONOBJECT( GuiAutoScrollCtrl );
      DECLARE_CATEGORY( "Gui Containers" );
      DECLARE_DESCRIPTION( "A container that automatically scrolls its child control upwards.\n"
         "Can be used, for example, for credits screens." );
};

typedef GuiAutoScrollCtrl::Direction GuiAutoScrollDirection;
DefineEnumType( GuiAutoScrollDirection );

#endif
