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

#include "gui/containers/guiRolloutCtrl.h"
#include "gui/containers/guiScrollCtrl.h"
#include "gfx/gfxDrawUtil.h"
#include "console/engineAPI.h"


IMPLEMENT_CONOBJECT( GuiRolloutCtrl );

ConsoleDocClass( GuiRolloutCtrl,
   "@brief A container that shows a single child with an optional header bar that can be used to collapse and expand the rollout.\n\n"
   
   "A rollout is a container that can be collapsed and expanded using smooth animation.  By default, rollouts will display a header "
   "with a caption along the top edge of the control which can be clicked by the user to toggle the collapse state of the rollout.\n\n"
   
   "Rollouts will automatically size themselves to exactly fit around their child control.  They will also automatically position their child "
   "control in their upper left corner below the header (if present).\n\n"
      
   "@note GuiRolloutCtrls will only work correctly with a single child control.  To put multiple controls in a rollout, put them "
      "in their own group using a new GuiControl which then can be put inside the rollout.\n\n"
   
   "@ingroup GuiContainers"
);

IMPLEMENT_CALLBACK( GuiRolloutCtrl, onHeaderRightClick, void, (), (),
   "Called when the user right-clicks on the rollout's header.  This is useful for implementing "
   "context menus for rollouts." );

IMPLEMENT_CALLBACK( GuiRolloutCtrl, onExpanded, void, (), (),
   "Called when the rollout is expanded." );

IMPLEMENT_CALLBACK( GuiRolloutCtrl, onCollapsed, void, (), (),
   "Called when the rollout is collapsed." );

//-----------------------------------------------------------------------------

GuiRolloutCtrl::GuiRolloutCtrl()
 : mHeader(0,0,0,0),
   mExpanded(0,0,0,0),
   mChildRect(0,0,0,0),
   mMargin(0,0,0,0)
{
   mExpanded.set(0,0,200,60);
   mCaption             = StringTable->EmptyString();
   mIsExpanded          = true;
   mIsAnimating         = false;
   mCollapsing          = false;
   mAnimateDestHeight   = 40;
   mAnimateStep         = 1;
   mDefaultHeight       = 40;
   mHideHeader          = false;
   mMargin.set( 0, 0, 0, 0 );
   mIsContainer = true;
   mCanCollapse = true;
   mAutoCollapseSiblings = false;
   mHasTexture = false;
   // Make sure we receive our ticks.
   setProcessTicks();
}

//-----------------------------------------------------------------------------

GuiRolloutCtrl::~GuiRolloutCtrl()
{
}

//-----------------------------------------------------------------------------

void GuiRolloutCtrl::initPersistFields()
{
   addGroup( "Rollout" );
   
      addField( "caption", TypeRealString, Offset( mCaption, GuiRolloutCtrl ),
         "Text label to display on the rollout header." );
      addField( "margin", TypeRectI, Offset( mMargin, GuiRolloutCtrl ),
         "Margin to put around child control." );
      addField( "defaultHeight", TypeS32, Offset( mDefaultHeight, GuiRolloutCtrl ),
         "Default height of the client area.  This is used when no child control has been added to the rollout." );
      addProtectedField( "expanded", TypeBool, Offset( mIsExpanded, GuiRolloutCtrl), &setExpanded, &defaultProtectedGetFn,
         "The current rollout expansion state." );
      addField( "clickCollapse", TypeBool, Offset( mCanCollapse, GuiRolloutCtrl ),
         "Whether the rollout can be collapsed by clicking its header." );
      addField( "hideHeader", TypeBool, Offset( mHideHeader, GuiRolloutCtrl ),
         "Whether to render the rollout header.\n\n"
         "@note If this is false, the user cannot toggle the rollout state with the mouse." );
      addField( "autoCollapseSiblings", TypeBool, Offset( mAutoCollapseSiblings, GuiRolloutCtrl ),
         "Whether to automatically collapse sibling rollouts.\n\n"
         "If this is true, the rollout will automatically collapse all sibling rollout controls when it "
         "is expanded.  If this is false, the auto-collapse behavior can be triggered by CTRL (CMD on MAC) "
         "clicking the rollout header.  CTRL/CMD clicking also works if this is false, in which case the "
         "auto-collapsing of sibling controls will be temporarily deactivated." );
         
   endGroup( "Rollout" );

   Parent::initPersistFields();
}

//=============================================================================
//    Events.
//=============================================================================
// MARK: ---- Events ----

//-----------------------------------------------------------------------------

bool GuiRolloutCtrl::onAdd()
{
   if ( !Parent::onAdd() )
      return false;

   mHasTexture = ( mProfile ? mProfile->constructBitmapArray() > 0 : false );
   if ( mHasTexture )
      mBitmapBounds = mProfile->mBitmapArrayRects.address();

   // Calculate Heights for this control
   calculateHeights();

   return true;
}

//-----------------------------------------------------------------------------

bool GuiRolloutCtrl::onWake()
{
   if (! Parent::onWake())
      return false;

   if( !mIsAnimating && mIsExpanded )
      sizeToContents();

   return true;
}

//-----------------------------------------------------------------------------

void GuiRolloutCtrl::addObject( SimObject *obj )
{
   // Call Parent.
   Parent::addObject( obj );

   sizeToContents();
}

//-----------------------------------------------------------------------------

void GuiRolloutCtrl::removeObject( SimObject *obj )
{
   // Call Parent.
   Parent::removeObject( obj );

   // Recalculate our rectangles.
   calculateHeights();
}

//-----------------------------------------------------------------------------

void GuiRolloutCtrl::onMouseDown( const GuiEvent &event )
{
   mouseLock();
}

//-----------------------------------------------------------------------------

bool GuiRolloutCtrl::_onMouseUp( const GuiEvent &event, bool lockedMouse )
{
   Point2I localPoint = globalToLocalCoord( event.mousePoint );
   if( mCanCollapse && mHeader.pointInRect( localPoint ) && !mIsAnimating && ( !lockedMouse || isMouseLocked() ) )
   {
      // If Ctrl/Cmd-clicking a header, collapse all sibling GuiRolloutCtrls.
      
      if( (( mAutoCollapseSiblings && !mIsExpanded && !( event.modifier & SI_PRIMARY_CTRL ))
          || ( !mAutoCollapseSiblings && event.modifier & SI_PRIMARY_CTRL ) ) )
      {
         for( SimSet::iterator iter = getParent()->begin(); iter != getParent()->end(); ++ iter )
         {
            GuiRolloutCtrl* ctrl = dynamic_cast< GuiRolloutCtrl* >( *iter );
            if( ctrl && ctrl != this && ctrl->mCanCollapse )
               ctrl->instantCollapse();
         }
         
         if( !mIsExpanded )
            expand();
      }
      else
      {
         // Toggle expansion.
         
         toggleExpanded( false );
      }
      
      return true;
   }
   
   return false;
}

//-----------------------------------------------------------------------------

void GuiRolloutCtrl::onMouseUp( const GuiEvent &event )
{
   _onMouseUp( event, true );
   if( isMouseLocked() )
      mouseUnlock();
}

//-----------------------------------------------------------------------------

void GuiRolloutCtrl::onRightMouseUp( const GuiEvent& event )
{
   Parent::onRightMouseUp( event );
   
   Point2I localMouse = globalToLocalCoord( event.mousePoint );
   if( mHeader.pointInRect( localMouse ) )
      onHeaderRightClick_callback();
}

//-----------------------------------------------------------------------------

bool GuiRolloutCtrl::onMouseUpEditor( const GuiEvent& event, Point2I offset )
{
   return ( event.modifier & SI_PRIMARY_ALT && _onMouseUp( event, false ) );
}

//=============================================================================
//    Sizing.
//=============================================================================
// MARK: ---- Sizing ----

//-----------------------------------------------------------------------------

void GuiRolloutCtrl::calculateHeights()
{
   S32 barHeight = 20;

   if ( mHasTexture && mProfile && mProfile->mBitmapArrayRects.size() >= NumBitmaps )
   {
      // Store Header Rectangle
      mHeader.set( 0, 0, getWidth(), mProfile->mBitmapArrayRects[ CollapsedCenter ].extent.y );

      // Bottom Bar Max
      barHeight = mProfile->mBitmapArrayRects[ TopLeftHeader ].extent.y;
   }
   else
   {
      mHeader.set( 0, 0, getWidth(), barHeight );
   }
   
   if ( mHideHeader )
   {
      barHeight = 0;
      mHeader.extent.y = 0;
   }

   GuiControl *content = static_cast<GuiControl*>( at(0) );
   if ( content != NULL )
      mExpanded.set( 0, 0, getWidth(), barHeight + content->getHeight() + ( mMargin.point.y + mMargin.extent.y ) );
   else
      mExpanded.set( 0, 0, getWidth(), barHeight + mDefaultHeight );
}

//-----------------------------------------------------------------------------

bool GuiRolloutCtrl::resize( const Point2I &newPosition, const Point2I &newExtent )
{
   if ( !Parent::resize( newPosition, newExtent ) )
      return false;

   // Recalculate Heights and resize ourself appropriately.
   calculateHeights();

   GuiControl *content = dynamic_cast<GuiControl*>( at(0) );
   
   // Size Content Properly?!
   if ( mNotifyChildrenResized && content != NULL )
   {
      S32 barHeight = ( mHideHeader ) ? 0 : 20;
      if( !mHideHeader && mHasTexture && mProfile && mProfile->mBitmapArrayRects.size() >= NumBitmaps )
      {
         barHeight = mProfile->mBitmapArrayRects[ TopLeftHeader ].extent.y;
      }

      mChildRect.set( mMargin.point.x, 
                      mHeader.extent.y + mMargin.point.y, 
                      getWidth() - ( mMargin.point.x + mMargin.extent.x ), 
                      getHeight() - ( barHeight + ( mMargin.point.y + mMargin.extent.y ) ) );

      if ( content->resize( mChildRect.point, mChildRect.extent ) )
         return true;
   }

   // Nothing sized
   return false;
}

//-----------------------------------------------------------------------------

void GuiRolloutCtrl::sizeToContents()
{
   calculateHeights();

   // Set destination height
   if ( size() > 0 )
      instantExpand();
   else
      instantCollapse();
}

//-----------------------------------------------------------------------------

void GuiRolloutCtrl::instantExpand()
{
   mAnimateDestHeight = mExpanded.extent.y;
   mCollapsing = false;
   mIsExpanded = true;
   mIsAnimating = false;
   resize( getPosition() + mExpanded.point, mExpanded.extent );

   onExpanded_callback();
}

//-----------------------------------------------------------------------------

void GuiRolloutCtrl::instantCollapse()
{
   mAnimateDestHeight = mHeader.extent.y;
   mCollapsing = false;
   mIsExpanded = false;
   mIsAnimating = false;
   resize( getPosition() + mHeader.point, mHeader.extent );

   onCollapsed_callback();
}

//-----------------------------------------------------------------------------

void GuiRolloutCtrl::toggleExpanded( bool instant )
{   
   if ( mIsExpanded )
   {
      if ( instant )
         instantCollapse();
      else
         collapse();
   }
   else
   {
      if ( instant )
         instantExpand();
      else
         expand();
   }
}

//-----------------------------------------------------------------------------

void GuiRolloutCtrl::childResized( GuiControl *child )
{
    Parent::childResized( child );

    calculateHeights();

    // While we are animating we are constantly resizing our children
    // and therefore need to ignore this call to 'instantExpand' which would
    // halt the animation in some crappy intermediate stage.
    if ( mIsExpanded && !mIsAnimating )
    {
       mNotifyChildrenResized = false;
       instantExpand();
       mNotifyChildrenResized = true;
    }
}

//=============================================================================
//    Animation.
//=============================================================================
// MARK: ---- Animation ----

//-----------------------------------------------------------------------------

void GuiRolloutCtrl::animateTo( S32 height )
{
   // We do nothing if we're already animating
   if( mIsAnimating )
      return;

   bool collapsing = (bool)( getHeight() > height );

   // If we're already at the destination height, bail
   if ( getHeight() >= height && !collapsing )
   {
      mIsExpanded = true;
      return;
   }

   // If we're already at the destination height, bail
   if ( getHeight() <= height && collapsing )
   {
      mIsExpanded = false;
      return;
   }

   // Set destination height
   mAnimateDestHeight = height;

   // Set Animation Mode
   mCollapsing = collapsing;

   // Set Animation Step (Increment)
   if ( collapsing )
      mAnimateStep = (S32)mFloor( (F32)( getHeight() - height ) / 3.f );
   else
      mAnimateStep = (S32)mFloor( (F32)( height - getHeight() ) / 3.f );

   // Start our animation
   mIsAnimating = true;
}

//-----------------------------------------------------------------------------

void GuiRolloutCtrl::processTick()
{
   // We do nothing here if we're NOT animating
   if ( !mIsAnimating )
      return;

   // Sanity check to fix non collapsing panels.
   if ( mAnimateStep == 0 )
      mAnimateStep = 1;

   S32 newHeight = getHeight();
   // We're collapsing ourself down (Hiding our contents)
   if( mCollapsing )
   {
      if ( newHeight < mAnimateDestHeight )
         newHeight = mAnimateDestHeight;
      else if ( ( newHeight - mAnimateStep ) < mAnimateDestHeight )
         newHeight = mAnimateDestHeight;

      if ( newHeight == mAnimateDestHeight )
         mIsAnimating = false;
      else
         newHeight -= mAnimateStep;

      if( !mIsAnimating )
     {
         mIsExpanded = false;
     }
   }
   else // We're expanding ourself (Showing our contents)
   {
      if ( newHeight > mAnimateDestHeight )
         newHeight = mAnimateDestHeight;
      else if ( ( newHeight + mAnimateStep ) > mAnimateDestHeight )
         newHeight = mAnimateDestHeight;

      if ( newHeight == mAnimateDestHeight )
         mIsAnimating = false;
      else
         newHeight += mAnimateStep;

      if ( !mIsAnimating )
         mIsExpanded = true;
   }

   if ( newHeight != getHeight() )
      setHeight( newHeight );

   if ( !mIsAnimating )
   {
      if( mCollapsing )
         onCollapsed_callback();
      else if( !mCollapsing )
         onExpanded_callback();

      calculateHeights();
   }

   GuiControl* parent = getParent();
   if ( parent )
   {
      parent->childResized( this );
      // if our parent's parent is a scroll control, scrollvisible.
      GuiScrollCtrl* scroll = dynamic_cast<GuiScrollCtrl*>( parent->getParent() );
      if ( scroll )
      {
         scroll->scrollRectVisible( getBounds() );
      }
   }
}

//=============================================================================
//    Rendering.
//=============================================================================
// MARK: ---- Rendering ----

//-----------------------------------------------------------------------------

void GuiRolloutCtrl::onRender( Point2I offset, const RectI &updateRect )
{
   if( !mProfile || mProfile->mFont == NULL )
      return;

   // Calculate actual world bounds for rendering
   RectI worldBounds( offset, getExtent() );

   // if opaque, fill the update rect with the fill color
   if ( mProfile->mOpaque )
      GFX->getDrawUtil()->drawRectFill( worldBounds, mProfile->mFillColor );

   if ( mProfile->mBitmapArrayRects.size() >= NumBitmaps )
   {
      GFX->getDrawUtil()->clearBitmapModulation();

      // Draw Rollout From Skin
      if ( !mIsExpanded && !mIsAnimating )
         renderFixedBitmapBordersFilled( worldBounds, 1, mProfile );
      else if ( mHideHeader )
         renderSizableBitmapBordersFilledIndex( worldBounds, MidPageLeft, mProfile );
      else
         renderSizableBitmapBordersFilledIndex( worldBounds, TopLeftHeader, mProfile );
   }

   if ( !(mIsExpanded && mHideHeader ) )
   {
      // Draw Caption ( Vertically Centered )
      ColorI currColor;
      GFX->getDrawUtil()->getBitmapModulation( &currColor );
      Point2I textPosition = mHeader.point + offset + mProfile->mTextOffset;
      GFX->getDrawUtil()->setBitmapModulation( mProfile->mFontColor );
      renderJustifiedText( textPosition, mHeader.extent, mCaption );
      GFX->getDrawUtil()->setBitmapModulation( currColor );
   }

   // If we're collapsed we contain the first child as our content
   // thus we don't render it when collapsed.  but to support modified
   // rollouts with custom header buttons etc we still render our other
   // children. -JDD
   GuiControl *pChild = dynamic_cast<GuiControl*>( at(0) );
   if ( pChild )
   {
      if ( !mIsExpanded && !mIsAnimating && pChild->isVisible() )
     {
         pChild->setVisible( false );
     }
      else if ( (mIsExpanded || mIsAnimating) && !pChild->isVisible() )
     {
         pChild->setVisible( true );
     }
   }
   renderChildControls( offset, updateRect );

   // Render our border should we have it specified in our profile.
   renderBorder(worldBounds, mProfile);
}

//=============================================================================
//    Console Methods.
//=============================================================================
// MARK: ---- Console Methods ----

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiRolloutCtrl, isExpanded, bool, (),,
   "Determine whether the rollout is currently expanded, i.e. whether the child control is visible.\n\n"
   "@return True if the rollout is expanded, false if not." )
{
   return object->isExpanded();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiRolloutCtrl, collapse, void, (),,
   "Collapse the rollout if it is currently expanded.  This will make the rollout's child control invisible.\n\n"
   "@note The rollout will animate to collapsed state.  To instantly collapse without animation, use instantCollapse()." )
{
   object->collapse();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiRolloutCtrl, expand, void, (),,
   "Expand the rollout if it is currently collapsed.  This will make the rollout's child control visible.\n\n"
   "@note The rollout will animate to expanded state.  To instantly expand without animation, use instantExpand()." )
{
   object->expand();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiRolloutCtrl, toggleCollapse, void, (),,
   "Toggle the current collapse state of the rollout.  If it is currently expanded, then collapse it.  If it "
   "is currently collapsed, then expand it." )
{
   if( object->isExpanded() )
      object->collapse();
   else
     object->expand();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiRolloutCtrl, toggleExpanded, void, ( bool instantly ), ( false ),
   "Toggle the current expansion state of the rollout  If it is currently expanded, then collapse it.  If it "
   "is currently collapsed, then expand it.\n\n"
   "@param instant If true, the rollout will toggle its state without animation.  Otherwise, the rollout will "
      "smoothly slide into the opposite state." )
{
   object->toggleExpanded( instantly );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiRolloutCtrl, instantCollapse, void, (),,
   "Instantly collapse the rollout without animation.  To smoothly slide the rollout to collapsed state, use collapse()." )
{
   object->instantCollapse();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiRolloutCtrl, instantExpand, void, (),,
   "Instantly expand the rollout without animation.  To smoothly slide the rollout to expanded state, use expand()." )
{
   object->instantExpand();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiRolloutCtrl, sizeToContents, void, (),,
   "Resize the rollout to exactly fit around its child control.  This can be used to manually trigger a recomputation of "
   "the rollout size." )
{
   object->sizeToContents();
}
