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

#include "gui/containers/guiAutoScrollCtrl.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"


IMPLEMENT_CONOBJECT( GuiAutoScrollCtrl );

ConsoleDocClass( GuiAutoScrollCtrl,
   "@brief A container that scrolls its child control up over time.\n\n"
   
   "This container can be used to scroll a single child control in either of the four directions.\n\n"
   
   "@tsexample\n"
   "// Create a GuiAutoScrollCtrl that scrolls a long text of credits.\n"
   "new GuiAutoScrollCtrl( CreditsScroller )\n"
   "{\n"
   "   position = \"0 0\";\n"
   "   extent = Canvas.extent.x SPC Canvas.extent.y;\n"
   "\n"
   "   scrollDirection = \"Up\"; // Scroll upwards.\n"
   "   startDelay = 4; // Wait 4 seconds before starting to scroll.\n"
   "   isLooping = false; // Don't loop the credits.\n"
   "   scrollOutOfSight = true; // Scroll up fully.\n"
   "\n"
   "   new GuiMLTextCtrl()\n"
   "   {\n"
   "      text = $CREDITS;\n"
   "   };\n"
   "};\n"
   "\n"
   "function CreditsScroller::onComplete( %this )\n"
   "{\n"
   "   // Switch back to main menu after credits have rolled.\n"
   "   Canvas.setContent( MainMenu );\n"
   "}\n"
   "\n"
   "// Start rolling credits.\n"
   "Canvas.setContent( CreditsScroller );\n"
   "@endtsexample\n\n"
   
   "@note Only the first child will be scrolled.\n\n"
   
   "@ingroup GuiContainers"
);


IMPLEMENT_CALLBACK( GuiAutoScrollCtrl, onTick, void, (), (),
   "Called every 32ms on the control." );
IMPLEMENT_CALLBACK( GuiAutoScrollCtrl, onStart, void, (), (),
   "Called when the control starts to scroll." );
IMPLEMENT_CALLBACK( GuiAutoScrollCtrl, onComplete, void, (), (),
   "Called when the child control has been scrolled in entirety." );
IMPLEMENT_CALLBACK( GuiAutoScrollCtrl, onReset, void, (), (),
   "Called when the child control is reset to its initial position and the cycle starts again." );


ImplementEnumType( GuiAutoScrollDirection,
   "Direction in which to scroll the child control.\n\n"
   "@ingroup GuiContainers" )
   { GuiAutoScrollCtrl::Up, "Up", "Scroll from bottom towards top." },
   { GuiAutoScrollCtrl::Down, "Down", "Scroll from top towards bottom." },
   { GuiAutoScrollCtrl::Left, "Left", "Scroll from right towards left." },
   { GuiAutoScrollCtrl::Right, "Right", "Scroll from left towards right." },
EndImplementEnumType;

//-----------------------------------------------------------------------------

GuiAutoScrollCtrl::GuiAutoScrollCtrl()
   : mDirection( Up ),
     mIsLooping( true ),
     mCurrentPhase( PhaseComplete ),
     mCurrentTime( 0.f ),
     mStartDelay( 3.f ),
     mResetDelay( 5.f ),
     mChildBorder( 10 ),
     mScrollSpeed( 1.f ),
     mScrollOutOfSight( false )
{
   mIsContainer = true;
}

//-----------------------------------------------------------------------------

void GuiAutoScrollCtrl::initPersistFields()
{
   addGroup( "Scrolling" );
   
      addField( "scrollDirection", TYPEID< Direction >(), Offset( mDirection, GuiAutoScrollCtrl ),
         "Direction in which the child control is moved." );
      addField( "startDelay", TypeF32, Offset( mStartDelay, GuiAutoScrollCtrl ),
         "Seconds to wait before starting to scroll." );
      addField( "resetDelay", TypeF32, Offset( mResetDelay, GuiAutoScrollCtrl ),
         "Seconds to wait after scrolling completes before resetting and starting over.\n\n"
         "@note Only takes effect if #isLooping is true." );
      addField( "childBorder", TypeS32, Offset( mChildBorder, GuiAutoScrollCtrl ),
         "Padding to put around child control (in pixels)." );
      addField( "scrollSpeed", TypeF32, Offset( mScrollSpeed, GuiAutoScrollCtrl ),
         "Scrolling speed in pixels per second." );
      addField( "isLooping", TypeBool, Offset( mIsLooping, GuiAutoScrollCtrl ),
         "If true, the scrolling will reset to the beginning once completing a cycle." );
      addField( "scrollOutOfSight", TypeBool, Offset( mScrollOutOfSight, GuiAutoScrollCtrl ),
         "If true, the child control will be completely scrolled out of sight; otherwise it will only scroll "
         "until the other end becomes visible." );
         
   endGroup( "Scrolling" );

   Parent::initPersistFields();
}

//-----------------------------------------------------------------------------

bool GuiAutoScrollCtrl::onWake()
{
   if( !Parent::onWake() )
      return false;
      
   setProcessTicks( true );
   return true;
}

//-----------------------------------------------------------------------------

void GuiAutoScrollCtrl::onSleep()
{
   setProcessTicks( false );
   Parent::onSleep();
}

//-----------------------------------------------------------------------------

void GuiAutoScrollCtrl::onChildAdded( GuiControl* control )
{
   _reset( control );
   Parent::onChildAdded( control );
}

//-----------------------------------------------------------------------------

void GuiAutoScrollCtrl::onChildRemoved( GuiControl* control )
{
   mCurrentPhase = PhaseComplete;
   Parent::onChildRemoved( control );
}

//-----------------------------------------------------------------------------

bool GuiAutoScrollCtrl::_isScrollComplete() const
{
   if( empty() )
      return true;
      
   GuiControl* control = static_cast< GuiControl* >( at( 0 ) );
   U32 axis = _getScrollAxis();
   F32 amount = _getScrollAmount();
   
   if( mScrollOutOfSight )
   {
      // If scrolling out of sight, scrolling is complete when the control's rectangle
      // does not intersect our own rectangle anymore.
      
      RectI thisRect( Point2I( 0, 0 ), getExtent() );
      return !( thisRect.overlaps( control->getBounds() ) );
   }
   else
   {
      if( amount < 0 )
         return ( control->getPosition()[ axis ] + control->getExtent()[ axis ] ) < ( getExtent()[ axis ] - mChildBorder );
      else
         return ( control->getPosition()[ axis ] >= mChildBorder );
   }
}

//-----------------------------------------------------------------------------

void GuiAutoScrollCtrl::_reset( GuiControl* control )
{
   U32 axis = _getScrollAxis();
   U32 counterAxis = ( axis == 1 ? 0 : 1 );

   Point2I newPosition( mChildBorder, mChildBorder );
   Point2I newExtent = control->getExtent();
   
   // Fit control on axis that is not scrolled.
   newExtent[ counterAxis ] = getExtent()[ counterAxis ] - mChildBorder * 2;
   
   // For the right and down scrolls, position the control away from the
   // right/bottom edge of our control.
   
   if( mDirection == Right )
      newPosition.x = - ( newExtent.x - getExtent().x + mChildBorder );
   else if( mDirection == Down )
      newPosition.y = - ( newExtent.y - getExtent().y + mChildBorder );
      
   // Set the child geometry.
      
   control->setPosition( newPosition );
   control->setExtent( newExtent );   
   
   // Reset counters.

   mCurrentTime = 0.0f;
   mCurrentPhase = PhaseInitial;
   mCurrentPosition = control->getPosition()[ axis ];
}

//-----------------------------------------------------------------------------

void GuiAutoScrollCtrl::reset()
{
   if( !empty() )
      _reset( static_cast< GuiControl* >( at( 0 ) ) );
}

//-----------------------------------------------------------------------------

bool GuiAutoScrollCtrl::resize( const Point2I &newPosition, const Point2I &newExtent )
{
   if( !Parent::resize( newPosition, newExtent ) )
      return false;

   for( iterator i = begin(); i != end(); ++ i )
   {
      GuiControl* control = static_cast< GuiControl* >( *i );
      if( control )
         _reset( control );
   }

   return true;
}

//-----------------------------------------------------------------------------

void GuiAutoScrollCtrl::childResized( GuiControl* child )
{
   Parent::childResized( child );
   _reset(child);
}

//-----------------------------------------------------------------------------

void GuiAutoScrollCtrl::processTick()
{
   onTick_callback();
}

//-----------------------------------------------------------------------------

void GuiAutoScrollCtrl::advanceTime( F32 timeDelta )
{
   if( mCurrentPhase == PhaseComplete )
      return;
      
   // Wait out initial delay.
            
   if( ( mCurrentTime + timeDelta ) < mStartDelay)
   {
      mCurrentTime += timeDelta;
      return;
   }
   
   // Start scrolling if we haven't already.
   
   if( mCurrentPhase == PhaseInitial )
   {
      onStart_callback();
      mCurrentPhase = PhaseScrolling;
   }

   GuiControl* control = static_cast< GuiControl* >( at( 0 ) );
   if( !control ) // Should not happen.
      return;

   // If not yet complete, scroll some more.
   
   if( !_isScrollComplete() )
   {
      U32 axis = _getScrollAxis();
      F32 amount = _getScrollAmount();
      
      mCurrentPosition += amount * timeDelta;
      Point2I newPosition = control->getPosition();
      newPosition[ axis ] = mCurrentPosition;
      
      control->setPosition( newPosition );
   }
   else
   {
      mCurrentTime += timeDelta;
      
      if( mCurrentPhase != PhaseComplete && mCurrentPhase != PhaseWait )
      {
         if( mCurrentPhase != PhaseWait )
         {
            onComplete_callback();
            mCurrentPhase = PhaseComplete;
         }
         
         mCompleteTime = mCurrentTime;
      }
      
      // Reset, if looping.
      
      if( mIsLooping )
      {
         // Wait out reset time and restart.
         
         mCurrentPhase = PhaseWait;
         if( mCurrentTime > ( mCompleteTime + mResetDelay ) )
         {
            onReset_callback();
            _reset( control );
         }
      }
   }
}

//-----------------------------------------------------------------------------

void GuiAutoScrollCtrl::inspectPostApply()
{
   Parent::inspectPostApply();
   reset();
}

//=============================================================================
//    API.
//=============================================================================
// MARK: ---- API ----

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiAutoScrollCtrl, reset, void, (),,
   "Reset scrolling." )
{
   object->reset();
}
