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
#include "gui/editor/guiEditCtrl.h"

#include "core/frameAllocator.h"
#include "core/stream/fileStream.h"
#include "core/stream/memStream.h"
#include "console/consoleTypes.h"
#include "gui/core/guiCanvas.h"
#include "gui/containers/guiScrollCtrl.h"
#include "core/strings/stringUnit.h"
#include "console/engineAPI.h"


IMPLEMENT_CONOBJECT( GuiEditCtrl );

ConsoleDocClass( GuiEditCtrl,
   "@brief Native side of the GUI editor.\n\n"
   "Editor use only.\n\n"
   "@internal"
);

IMPLEMENT_CALLBACK( GuiEditCtrl, onHierarchyChanged, void, (), (),
   "" );
IMPLEMENT_CALLBACK( GuiEditCtrl, onDelete, void, (), (),
   "" );
IMPLEMENT_CALLBACK( GuiEditCtrl, onPreEdit, void, ( SimSet* selection ), ( selection ),
   "" );
IMPLEMENT_CALLBACK( GuiEditCtrl, onPostEdit, void, ( SimSet* selection ), ( selection ),
   "" );
IMPLEMENT_CALLBACK( GuiEditCtrl, onClearSelected, void, (), (),
   "" )
IMPLEMENT_CALLBACK( GuiEditCtrl, onSelect, void, ( GuiControl* control ), ( control ),
   "" );
IMPLEMENT_CALLBACK( GuiEditCtrl, onAddSelected, void, ( GuiControl* control ), ( control ),
   "" );
IMPLEMENT_CALLBACK( GuiEditCtrl, onRemoveSelected, void, ( GuiControl* control ), ( control ),
   "" );
IMPLEMENT_CALLBACK( GuiEditCtrl, onPreSelectionNudged, void, ( SimSet* selection ), ( selection ),
   "" );
IMPLEMENT_CALLBACK( GuiEditCtrl, onPostSelectionNudged, void, ( SimSet* selection ), ( selection ),
   "" );
IMPLEMENT_CALLBACK( GuiEditCtrl, onSelectionMoved, void, ( GuiControl* control ), ( control ),
   "" );
IMPLEMENT_CALLBACK( GuiEditCtrl, onSelectionCloned, void, ( SimSet* selection ), ( selection ),
   "" );
IMPLEMENT_CALLBACK( GuiEditCtrl, onTrashSelection, void, ( SimSet* selection ), ( selection ),
   "" );
IMPLEMENT_CALLBACK( GuiEditCtrl, onAddNewCtrl, void, ( GuiControl* control ), ( control ),   
   "" );
IMPLEMENT_CALLBACK( GuiEditCtrl, onAddNewCtrlSet, void, ( SimSet* set ), ( set ),
   "" );
IMPLEMENT_CALLBACK( GuiEditCtrl, onSelectionResized, void, ( GuiControl* control ), ( control ),
   "" );
IMPLEMENT_CALLBACK( GuiEditCtrl, onFitIntoParent, void, ( bool width, bool height ), ( width, height ),
   "" );
IMPLEMENT_CALLBACK( GuiEditCtrl, onMouseModeChange, void, (), (),
   "" );
IMPLEMENT_CALLBACK( GuiEditCtrl, onControlInspectPreApply, void, ( GuiControl* control ), ( control ),
   "" );
IMPLEMENT_CALLBACK( GuiEditCtrl, onControlInspectPostApply, void, ( GuiControl* control ), ( control ),
   "" );


StringTableEntry GuiEditCtrl::smGuidesPropertyName[ 2 ];


//-----------------------------------------------------------------------------

GuiEditCtrl::GuiEditCtrl()
   : mCurrentAddSet( NULL ),
     mContentControl( NULL ),
     mGridSnap( 0, 0 ),
     mDragBeginPoint( -1, -1 ),
     mSnapToControls( true ),
     mSnapToEdges( true ),
     mSnapToCenters( true ),
     mSnapToGuides( true ),
     mSnapToCanvas( true ),
     mSnapSensitivity( 2 ),
     mFullBoxSelection( false ),
     mDrawBorderLines( true ),
     mDrawGuides( true )
{
   VECTOR_SET_ASSOCIATION( mSelectedControls );
   VECTOR_SET_ASSOCIATION( mDragBeginPoints );
   VECTOR_SET_ASSOCIATION( mSnapHits[ 0 ] );
   VECTOR_SET_ASSOCIATION( mSnapHits[ 1 ] );
      
   mActive = true;
   mDotSB = NULL;
   
   mSnapped[ SnapVertical ] = false;
   mSnapped[ SnapHorizontal ] = false;

   mDragGuide[ GuideVertical ] = false;
   mDragGuide[ GuideHorizontal ] = false;
   
   if( !smGuidesPropertyName[ GuideVertical ] )
      smGuidesPropertyName[ GuideVertical ] = StringTable->insert( "guidesVertical" );
   if( !smGuidesPropertyName[ GuideHorizontal ] )
      smGuidesPropertyName[ GuideHorizontal ] = StringTable->insert( "guidesHorizontal" );
}

//-----------------------------------------------------------------------------

void GuiEditCtrl::initPersistFields()
{
   addGroup( "Snapping" );
   addField( "snapToControls",      TypeBool,   Offset( mSnapToControls, GuiEditCtrl ),
      "If true, edge and center snapping will work against controls." );
   addField( "snapToGuides",        TypeBool,   Offset( mSnapToGuides, GuiEditCtrl ),
      "If true, edge and center snapping will work against guides." );
   addField( "snapToCanvas",        TypeBool,   Offset( mSnapToCanvas, GuiEditCtrl ),
      "If true, edge and center snapping will work against canvas (toplevel control)." );
   addField( "snapToEdges",         TypeBool,   Offset( mSnapToEdges, GuiEditCtrl ),
      "If true, selection edges will snap into alignment when moved or resized." );
   addField( "snapToCenters",       TypeBool,   Offset( mSnapToCenters, GuiEditCtrl ),
      "If true, selection centers will snap into alignment when moved or resized." );
   addField( "snapSensitivity",     TypeS32,    Offset( mSnapSensitivity, GuiEditCtrl ),
      "Distance in pixels that edge and center snapping will work across." );
   endGroup( "Snapping" );
   
   addGroup( "Selection" );
   addField( "fullBoxSelection",    TypeBool,   Offset( mFullBoxSelection, GuiEditCtrl ),
      "If true, rectangle selection will only select controls fully inside the drag rectangle." );
   endGroup( "Selection" );
   
   addGroup( "Rendering" );
   addField( "drawBorderLines",  TypeBool,   Offset( mDrawBorderLines, GuiEditCtrl ),
      "If true, lines will be drawn extending along the edges of selected objects." );
   addField( "drawGuides", TypeBool, Offset( mDrawGuides, GuiEditCtrl ),
      "If true, guides will be included in rendering." );
   endGroup( "Rendering" );

   Parent::initPersistFields();
}

//=============================================================================
//    Events.
//=============================================================================
// MARK: ---- Events ----

//-----------------------------------------------------------------------------

bool GuiEditCtrl::onAdd()
{
   if( !Parent::onAdd() )
      return false;
      
   mTrash = new SimGroup();
   mSelectedSet = new SimSet();
      
   if( !mTrash->registerObject() )
      return false;
   if( !mSelectedSet->registerObject() )
      return false;

   return true;
}

//-----------------------------------------------------------------------------

void GuiEditCtrl::onRemove()
{
   Parent::onRemove();
   
   mDotSB = NULL;
   
   mTrash->deleteObject();
   mSelectedSet->deleteObject();
   
   mTrash = NULL;
   mSelectedSet = NULL;
}

//-----------------------------------------------------------------------------

bool GuiEditCtrl::onWake()
{
   if (! Parent::onWake())
      return false;

   // Set GUI Controls to DesignTime mode
   GuiControl::smDesignTime = true;
   GuiControl::smEditorHandle = this;

   setEditMode(true);

   return true;
}

//-----------------------------------------------------------------------------

void GuiEditCtrl::onSleep()
{
   // Set GUI Controls to run time mode
   GuiControl::smDesignTime = false;
   GuiControl::smEditorHandle = NULL;

   Parent::onSleep();
}

//-----------------------------------------------------------------------------

bool GuiEditCtrl::onKeyDown(const GuiEvent &event)
{
   if (! mActive)
      return Parent::onKeyDown(event);

   if (!(event.modifier & SI_PRIMARY_CTRL))
   {
      switch(event.keyCode)
      {
         case KEY_BACKSPACE:
         case KEY_DELETE:
            deleteSelection();
            onDelete_callback();
            return true;
         default:
            break;
      }
   }
   return false;
}

//-----------------------------------------------------------------------------

void GuiEditCtrl::onMouseDown(const GuiEvent &event)
{
   if (! mActive)
   {
      Parent::onMouseDown(event);
      return;
   }
   if(!mContentControl)
      return;

   setFirstResponder();
   mouseLock();

   mLastMousePos = globalToLocalCoord( event.mousePoint );

   // Check whether we've hit a guide.  If so, start a guide drag.
   // Don't do this if SHIFT is down.
   
   if( !( event.modifier & SI_SHIFT ) )
   {
      for( U32 axis = 0; axis < 2; ++ axis )
      {
         const S32 guide = findGuide( ( guideAxis ) axis, event.mousePoint, 1 );
         if( guide != -1 )
         {
            setMouseMode( DragGuide );
            
            mDragGuide[ axis ] = true;
            mDragGuideIndex[ axis ] = guide;
         }
      }
            
      if( mMouseDownMode == DragGuide )
         return;
   }
   
   // Check whether we have hit a sizing knob on any of the currently selected
   // controls.
   
   for( U32 i = 0, num = mSelectedControls.size(); i < num; ++ i )
   {
      GuiControl* ctrl = mSelectedControls[ i ];
      
      Point2I cext = ctrl->getExtent();
      Point2I ctOffset = globalToLocalCoord( ctrl->localToGlobalCoord( Point2I( 0, 0 ) ) );
      
      RectI box( ctOffset.x, ctOffset.y, cext.x, cext.y );

      if( ( mSizingMode = ( GuiEditCtrl::sizingModes ) getSizingHitKnobs( mLastMousePos, box ) ) != 0 )
      {
         setMouseMode( SizingSelection );
         mLastDragPos = event.mousePoint;
         
         // undo
         onPreEdit_callback( getSelectedSet() );
         return;
      }
   }

   // Find the control we have hit.
   
   GuiControl* ctrl = mContentControl->findHitControl( mLastMousePos, getCurrentAddSet()->mLayer );

   // Give the control itself the opportunity to handle the event
   // to implement custom editing logic.

   bool handledEvent = ctrl->onMouseDownEditor( event, localToGlobalCoord( Point2I(0,0) ) );
   if( handledEvent == true )
   {
      // The Control handled the event and requested the edit ctrl
      // *NOT* act on it.
      return;
   }
   else if( event.modifier & SI_SHIFT )
   {
      // Shift is down.  Start rectangle selection in add mode
      // no matter what we have hit.
      
      startDragRectangle( event.mousePoint );
      mDragAddSelection = true;
   }
   else if( selectionContains( ctrl ) )
   {
      // We hit a selected control.  If the multiselect key is pressed,
      // deselect the control.  Otherwise start a drag move.
      
      if( event.modifier & SI_MULTISELECT )
      {
         removeSelection( ctrl );

         //set the mode
         setMouseMode( Selecting );
      }
      else if( event.modifier & SI_PRIMARY_ALT )
      {
         // Alt is down.  Start a drag clone.
         
         startDragClone( event.mousePoint );
      }
      else
      {
         startDragMove( event.mousePoint );
      }
   }
   else
   {
      // We clicked an unselected control.
      
      if( ctrl == getContentControl() )
      {
         // Clicked in toplevel control.  Start a rectangle selection.
         
         startDragRectangle( event.mousePoint );
         mDragAddSelection = false;
      }
      else if( event.modifier & SI_PRIMARY_ALT && ctrl != getContentControl() )
      {
         // Alt is down.  Start a drag clone.
         
         clearSelection();
         addSelection( ctrl );
         
         startDragClone( event.mousePoint );
      }
      else if( event.modifier & SI_MULTISELECT )
         addSelection( ctrl );
      else
      {
         // Clicked on child control.  Start move.
         
         clearSelection();
         addSelection( ctrl );
         
         startDragMove( event.mousePoint );
      }
   }
}

//-----------------------------------------------------------------------------

void GuiEditCtrl::onMouseUp(const GuiEvent &event)
{
   if (! mActive || !mContentControl || !getCurrentAddSet() )
   {
      Parent::onMouseUp(event);
      return;
   }

   //find the control we clicked
   GuiControl *ctrl = mContentControl->findHitControl(mLastMousePos, getCurrentAddSet()->mLayer);

   bool handledEvent = ctrl->onMouseUpEditor( event, localToGlobalCoord( Point2I(0,0) ) );
   if( handledEvent == true )
   {
      // The Control handled the event and requested the edit ctrl
      // *NOT* act on it.  The dude abides.
      return;
   }

   //unlock the mouse
   mouseUnlock();

   // Reset Drag Axis Alignment Information
   mDragBeginPoint.set(-1,-1);
   mDragBeginPoints.clear();

   mLastMousePos = globalToLocalCoord(event.mousePoint);
   if( mMouseDownMode == DragGuide )
   {
      // Check to see if the mouse has moved off the canvas.  If so,
      // remove the guides being dragged.
      
      for( U32 axis = 0; axis < 2; ++ axis )
         if( mDragGuide[ axis ] && !getContentControl()->getGlobalBounds().pointInRect( event.mousePoint ) )
            mGuides[ axis ].erase( mDragGuideIndex[ axis ] );
   }
   else if( mMouseDownMode == DragSelecting )
   {
      // If not multiselecting, clear the current selection.
      
      if( !( event.modifier & SI_MULTISELECT ) && !mDragAddSelection )
         clearSelection();
         
      RectI rect;
      getDragRect( rect );
            
      // If the region is somewhere less than at least 2x2, count this as a
      // normal, non-rectangular selection. 
      
      if( rect.extent.x <= 2 && rect.extent.y <= 2 )
         addSelectControlAt( rect.point );
      else
      {
         // Use HIT_AddParentHits by default except if ALT is pressed.
         // Use HIT_ParentPreventsChildHit if ALT+CTRL is pressed.
         
         U32 hitFlags = 0;
         if( !( event.modifier & SI_PRIMARY_ALT ) )
            hitFlags |= HIT_AddParentHits;
         if( event.modifier & SI_PRIMARY_ALT && event.modifier & SI_CTRL )
            hitFlags |= HIT_ParentPreventsChildHit;
            
         addSelectControlsInRegion( rect, hitFlags );      
      }
   }
   else if( ctrl == getContentControl() && mMouseDownMode == Selecting )
      setCurrentAddSet( NULL, true );
   
   // deliver post edit event if we've been editing
   // note: paxorr: this may need to be moved earlier, if the selection has changed.
   // undo
   if( mMouseDownMode == SizingSelection || ( mMouseDownMode == MovingSelection && mDragMoveUndo ) )
      onPostEdit_callback( getSelectedSet() );

   //reset the mouse mode
   setFirstResponder();
   setMouseMode( Selecting );
   mSizingMode = sizingNone;
   
   // Clear snapping state.
   
   mSnapped[ SnapVertical ] = false;
   mSnapped[ SnapHorizontal ] = false;
   
   mSnapTargets[ SnapVertical ] = NULL;
   mSnapTargets[ SnapHorizontal ] = NULL;
   
   // Clear guide drag state.
   
   mDragGuide[ GuideVertical ] = false;
   mDragGuide[ GuideHorizontal ] = false;
}

//-----------------------------------------------------------------------------

void GuiEditCtrl::onMouseDragged( const GuiEvent &event )
{
   if( !mActive || !mContentControl || !getCurrentAddSet() )
   {
      Parent::onMouseDragged(event);
      return;
   }

   Point2I mousePoint = globalToLocalCoord( event.mousePoint );

   //find the control we clicked
   GuiControl *ctrl = mContentControl->findHitControl( mousePoint, getCurrentAddSet()->mLayer );

   bool handledEvent = ctrl->onMouseDraggedEditor( event, localToGlobalCoord( Point2I(0,0) ) );
   if( handledEvent == true )
   {
      // The Control handled the event and requested the edit ctrl
      // *NOT* act on it.  The dude abides.
      return;
   }
   
   // If we're doing a drag clone, see if we have crossed the move threshold.  If so,
   // clone the selection and switch to move mode.
   
   if( mMouseDownMode == DragClone )
   {
      // If we haven't yet crossed the mouse delta to actually start the
      // clone, check if we have now.
      
      S32 delta = mAbs( ( mousePoint - mDragBeginPoint ).len() );
      if( delta >= 4 )
      {
         cloneSelection();
         mLastMousePos = mDragBeginPoint;
         mDragMoveUndo = false;
         
         setMouseMode( MovingSelection );
      }
   }
   
   if( mMouseDownMode == DragGuide )
   {
      for( U32 axis = 0; axis < 2; ++ axis )
         if( mDragGuide[ axis ] )
         {
            // Set the guide to the coordinate of the mouse cursor
            // on the guide's axis.
            
            Point2I point = event.mousePoint;
            point -= localToGlobalCoord( Point2I( 0, 0 ) );
            point[ axis ] = mClamp( point[ axis ], 0, getExtent()[ axis ] - 1 );
            
            mGuides[ axis ][ mDragGuideIndex[ axis ] ] = point[ axis ];
         }
   }
   else if( mMouseDownMode == SizingSelection )
   {
      // Snap the mouse cursor to grid if active.  Do this on the mouse cursor so that we handle
      // incremental drags correctly.
      
      Point2I mousePoint = event.mousePoint;
      snapToGrid( mousePoint );
                  
      Point2I delta = mousePoint - mLastDragPos;
      
      // If CTRL is down, apply smart snapping.
      
      if( event.modifier & SI_CTRL )
      {
         RectI selectionBounds = getSelectionBounds();
         
         doSnapping( event, selectionBounds, delta );
      }
      else
      {
         mSnapped[ SnapVertical ] = false;
         mSnapped[ SnapHorizontal ] = false;
      }
      
      // If ALT is down, do a move instead of a resize on the control
      // knob's axis.  Otherwise resize.

      if( event.modifier & SI_PRIMARY_ALT )
      {
         if( !( mSizingMode & sizingLeft ) && !( mSizingMode & sizingRight ) )
         {
            mSnapped[ SnapVertical ] = false;
            delta.x = 0;
         }
         if( !( mSizingMode & sizingTop ) && !( mSizingMode & sizingBottom ) )
         {
            mSnapped[ SnapHorizontal ] = false;
            delta.y = 0;
         }
            
         moveSelection( delta );
      }
      else
         resizeControlsInSelectionBy( delta, mSizingMode );
         
      // Remember drag point.
      
      mLastDragPos = mousePoint;
   }
   else if (mMouseDownMode == MovingSelection && mSelectedControls.size())
   {
      Point2I delta = mousePoint - mLastMousePos;
      RectI selectionBounds = getSelectionBounds();
      
      // Apply snaps.
      
      doSnapping( event, selectionBounds, delta );
      
      //RDTODO: to me seems to be in need of revision
      // Do we want to align this drag to the X and Y axes within a certain threshold?
      if( event.modifier & SI_SHIFT && !( event.modifier & SI_PRIMARY_ALT ) )
      {
         Point2I dragTotalDelta = event.mousePoint - localToGlobalCoord( mDragBeginPoint );
         if( dragTotalDelta.y < 10 && dragTotalDelta.y > -10 )
         {
            for(S32 i = 0; i < mSelectedControls.size(); i++)
            {
               Point2I selCtrlPos = mSelectedControls[i]->getPosition();
               Point2I snapBackPoint( selCtrlPos.x, mDragBeginPoints[i].y);
               // This is kind of nasty but we need to snap back if we're not at origin point with selection - JDD
               if( selCtrlPos.y != mDragBeginPoints[i].y )
                  mSelectedControls[i]->setPosition( snapBackPoint );
            }
            delta.y = 0;
         }
         if( dragTotalDelta.x < 10 && dragTotalDelta.x > -10 )
         {
            for(S32 i = 0; i < mSelectedControls.size(); i++)
            {
               Point2I selCtrlPos = mSelectedControls[i]->getPosition();
               Point2I snapBackPoint( mDragBeginPoints[i].x, selCtrlPos.y);
               // This is kind of nasty but we need to snap back if we're not at origin point with selection - JDD
               if( selCtrlPos.x != mDragBeginPoints[i].x )
                  mSelectedControls[i]->setPosition( snapBackPoint );
            }
            delta.x = 0;
         }
      }

      if( delta.x || delta.y )
         moveSelection( delta, mDragMoveUndo );

      // find the current control under the mouse

      canHitSelectedControls( false );
      GuiControl *inCtrl = mContentControl->findHitControl(mousePoint, getCurrentAddSet()->mLayer);
      canHitSelectedControls( true );

      // find the nearest control up the heirarchy from the control the mouse is in
      // that is flagged as a container.
      while( !inCtrl->mIsContainer )
         inCtrl = inCtrl->getParent();
         
      // if the control under the mouse is not our parent, move the selected controls
      // into the new parent.
      if(mSelectedControls[0]->getParent() != inCtrl && inCtrl->mIsContainer)
      {
         moveSelectionToCtrl( inCtrl, mDragMoveUndo );
         setCurrentAddSet( inCtrl, false );
      }

      mLastMousePos += delta;
   }
   else
      mLastMousePos = mousePoint;
}

//-----------------------------------------------------------------------------

void GuiEditCtrl::onRightMouseDown(const GuiEvent &event)
{
   if (! mActive || !mContentControl)
   {
      Parent::onRightMouseDown(event);
      return;
   }
   setFirstResponder();

   //search for the control hit in any layer below the edit layer
   GuiControl *hitCtrl = mContentControl->findHitControl(globalToLocalCoord(event.mousePoint), mLayer - 1);
   if (hitCtrl != getCurrentAddSet())
   {
      setCurrentAddSet( hitCtrl );
   }
   // select the parent if we right-click on the current add set 
   else if( getCurrentAddSet() != mContentControl)
   {
      setCurrentAddSet( hitCtrl->getParent() );
      select(hitCtrl);
   }

   //Design time mouse events
   GuiEvent designEvent = event;
   designEvent.mousePoint = mLastMousePos;
   hitCtrl->onRightMouseDownEditor( designEvent, localToGlobalCoord( Point2I(0,0) ) );

}

//=============================================================================
//    Rendering.
//=============================================================================
// MARK: ---- Rendering ----

//-----------------------------------------------------------------------------

void GuiEditCtrl::onPreRender()
{
   setUpdate();
}

//-----------------------------------------------------------------------------

void GuiEditCtrl::onRender(Point2I offset, const RectI &updateRect)
{   
   Point2I ctOffset;
   Point2I cext;
   bool keyFocused = isFirstResponder();

   GFXDrawUtil *drawer = GFX->getDrawUtil();

   if (mActive)
   {
      if( getCurrentAddSet() != getContentControl() )
      {
         // draw a white frame inset around the current add set.
         cext = getCurrentAddSet()->getExtent();
         ctOffset = getCurrentAddSet()->localToGlobalCoord(Point2I(0,0));
         RectI box(ctOffset.x, ctOffset.y, cext.x, cext.y);

			box.inset( -5, -5 );
         drawer->drawRect( box, ColorI( 50, 101, 152, 128 ) );
			box.inset( 1, 1 );
         drawer->drawRect( box, ColorI( 50, 101, 152, 128 ) );
			box.inset( 1, 1 );
         drawer->drawRect( box, ColorI( 50, 101, 152, 128 ) );
			box.inset( 1, 1 );
         drawer->drawRect( box, ColorI( 50, 101, 152, 128 ) );
			box.inset( 1, 1 );
			drawer->drawRect( box, ColorI( 50, 101, 152, 128 ) );
      }
      Vector<GuiControl *>::iterator i;
      bool multisel = mSelectedControls.size() > 1;
      for(i = mSelectedControls.begin(); i != mSelectedControls.end(); i++)
      {
         GuiControl *ctrl = (*i);
         cext = ctrl->getExtent();
         ctOffset = ctrl->localToGlobalCoord(Point2I(0,0));
         RectI box(ctOffset.x,ctOffset.y, cext.x, cext.y);
         ColorI nutColor = multisel ? ColorI( 255, 255, 255, 100 ) : ColorI( 0, 0, 0, 100 );
         ColorI outlineColor = multisel ? ColorI( 0, 0, 0, 100 ) : ColorI( 255, 255, 255, 100 );
         if(!keyFocused)
            nutColor.set( 128, 128, 128, 100 );

         drawNuts(box, outlineColor, nutColor);
      }
   }

   renderChildControls(offset, updateRect);
   
   // Draw selection rectangle.
   
   if( mActive && mMouseDownMode == DragSelecting )
   {
      RectI b;
      getDragRect(b);
      b.point += offset;
      
      // Draw outline.
      
      drawer->drawRect( b, ColorI( 100, 100, 100, 128 ) );
      
      // Draw fill.
      
      b.inset( 1, 1 );
      drawer->drawRectFill( b, ColorI( 150, 150, 150, 128 ) );
   }

   // Draw grid.

   if(   mActive &&
         ( mMouseDownMode == MovingSelection || mMouseDownMode == SizingSelection ) &&
         ( mGridSnap.x || mGridSnap.y ) )
   {
      Point2I cext = getContentControl()->getExtent();
      Point2I coff = getContentControl()->localToGlobalCoord(Point2I(0,0));
      
      // create point-dots
      const Point2I& snap = mGridSnap;
      U32 maxdot = (U32)(mCeil(cext.x / (F32)snap.x) - 1) * (U32)(mCeil(cext.y / (F32)snap.y) - 1);

      if( mDots.isNull() || maxdot != mDots->mNumVerts)
      {
         mDots.set(GFX, maxdot, GFXBufferTypeStatic);

         U32 ndot = 0;
         mDots.lock();
         for(U32 ix = snap.x; ix < cext.x; ix += snap.x)
         { 
            for(U32 iy = snap.y; ndot < maxdot && iy < cext.y; iy += snap.y)
            {
               mDots[ndot].color.set( 50, 50, 254, 100 );
               mDots[ndot].point.x = F32(ix + coff.x);
               mDots[ndot].point.y = F32(iy + coff.y);
               mDots[ndot].point.z = 0.0f;
               ndot++;
            }
         }
         mDots.unlock();
         AssertFatal(ndot <= maxdot, "dot overflow");
         AssertFatal(ndot == maxdot, "dot underflow");
      }

      if (!mDotSB)
      {
         GFXStateBlockDesc dotdesc;
         dotdesc.setBlend(true, GFXBlendSrcAlpha, GFXBlendInvSrcAlpha);
         dotdesc.setCullMode( GFXCullNone );
         mDotSB = GFX->createStateBlock( dotdesc );
      }

      GFX->setStateBlock(mDotSB);

      // draw the points.
      GFX->setVertexBuffer( mDots );
      GFX->drawPrimitive( GFXPointList, 0, mDots->mNumVerts );
   }

   // Draw snapping lines.
   
   if( mActive && getContentControl() )
   {      
      RectI bounds = getContentControl()->getGlobalBounds();
            
      // Draw guide lines.

      if( mDrawGuides )
      {
         for( U32 axis = 0; axis < 2; ++ axis )
         {
            for( U32 i = 0, num = mGuides[ axis ].size(); i < num; ++ i )
               drawCrossSection( axis, mGuides[ axis ][ i ] + bounds.point[ axis ],
                  bounds, ColorI( 0, 255, 0, 100 ), drawer );
         }
      }

      // Draw smart snap lines.
      
      for( U32 axis = 0; axis < 2; ++ axis )
      {
         if( mSnapped[ axis ] )
         {
            // Draw the snap line.
            
            drawCrossSection( axis, mSnapOffset[ axis ],
               bounds, ColorI( 0, 0, 255, 100 ), drawer );

            // Draw a border around the snap target control.
            
            if( mSnapTargets[ axis ] )
            {
               RectI bounds = mSnapTargets[ axis ]->getGlobalBounds();
               drawer->drawRect( bounds, ColorF( .5, .5, .5, .5 ) );
            }
         }
      }
   }
}

//-----------------------------------------------------------------------------

void GuiEditCtrl::drawNuts(RectI &box, ColorI &outlineColor, ColorI &nutColor)
{
   GFXDrawUtil *drawer = GFX->getDrawUtil();

   S32 lx = box.point.x, rx = box.point.x + box.extent.x - 1;
   S32 cx = (lx + rx) >> 1;
   S32 ty = box.point.y, by = box.point.y + box.extent.y - 1;
   S32 cy = (ty + by) >> 1;

   if( mDrawBorderLines )
   {
      ColorF lineColor( 0.7f, 0.7f, 0.7f, 0.25f );
      ColorF lightLineColor( 0.5f, 0.5f, 0.5f, 0.1f );
      
      if(lx > 0 && ty > 0)
      {
         drawer->drawLine(0, ty, lx, ty, lineColor);  // Left edge to top-left corner.
         drawer->drawLine(lx, 0, lx, ty, lineColor);  // Top edge to top-left corner.
      }

      if(lx > 0 && by > 0)
         drawer->drawLine(0, by, lx, by, lineColor);  // Left edge to bottom-left corner.

      if(rx > 0 && ty > 0)
         drawer->drawLine(rx, 0, rx, ty, lineColor);  // Top edge to top-right corner.

      Point2I extent = localToGlobalCoord(getExtent());

      if(lx < extent.x && by < extent.y)
         drawer->drawLine(lx, by, lx, extent.y, lightLineColor);  // Bottom-left corner to bottom edge.
      if(rx < extent.x && by < extent.y)
      {
         drawer->drawLine(rx, by, rx, extent.y, lightLineColor);  // Bottom-right corner to bottom edge.
         drawer->drawLine(rx, by, extent.x, by, lightLineColor);  // Bottom-right corner to right edge.
      }
      if(rx < extent.x && ty < extent.y)
         drawer->drawLine(rx, ty, extent.x, ty, lightLineColor);  // Top-right corner to right edge.
   }

   // Adjust nuts, so they dont straddle the controls.
   
   lx -= NUT_SIZE + 1;
   ty -= NUT_SIZE + 1;
   rx += 1;
   by += 1;
   
   // Draw nuts.

   drawNut( Point2I( lx - NUT_SIZE, ty - NUT_SIZE ), outlineColor, nutColor ); // Top left
   drawNut( Point2I( lx - NUT_SIZE, cy - NUT_SIZE / 2 ), outlineColor, nutColor ); // Mid left
   drawNut( Point2I( lx - NUT_SIZE, by ), outlineColor, nutColor ); // Bottom left
   drawNut( Point2I( rx, ty - NUT_SIZE ), outlineColor, nutColor ); // Top right
   drawNut( Point2I( rx, cy - NUT_SIZE / 2 ), outlineColor, nutColor ); // Mid right
   drawNut( Point2I( rx, by ), outlineColor, nutColor ); // Bottom right
   drawNut( Point2I( cx - NUT_SIZE / 2, ty - NUT_SIZE ), outlineColor, nutColor ); // Mid top
   drawNut( Point2I( cx - NUT_SIZE / 2, by ), outlineColor, nutColor ); // Mid bottom
}

//-----------------------------------------------------------------------------

void GuiEditCtrl::drawNut(const Point2I &nut, ColorI &outlineColor, ColorI &nutColor)
{
   RectI r( nut.x, nut.y, NUT_SIZE * 2, NUT_SIZE * 2 );
   GFX->getDrawUtil()->drawRect( r, outlineColor );
   r.inset( 1, 1 );
   GFX->getDrawUtil()->drawRectFill( r, nutColor );
}

//=============================================================================
//    Selections.
//=============================================================================
// MARK: ---- Selections ----

//-----------------------------------------------------------------------------

void GuiEditCtrl::clearSelection(void)
{
   mSelectedControls.clear();
   onClearSelected_callback();
}

//-----------------------------------------------------------------------------

void GuiEditCtrl::setSelection(GuiControl *ctrl, bool inclusive)
{
   //sanity check
   if( !ctrl )
      return;
      
   if( mSelectedControls.size() == 1 && mSelectedControls[ 0 ] == ctrl )
      return;
      
   if( !inclusive )
      clearSelection();

   if( mContentControl == ctrl )
      setCurrentAddSet( ctrl, false );
   else
      addSelection( ctrl );
}

//-----------------------------------------------------------------------------

void GuiEditCtrl::addSelection(S32 id)
{
   GuiControl * ctrl;
   if( Sim::findObject( id, ctrl ) )
      addSelection( ctrl );
}

//-----------------------------------------------------------------------------

void GuiEditCtrl::addSelection( GuiControl* ctrl )
{
   // Only add if this isn't the content control and the
   // control isn't yet in the selection.
   
   if( ctrl != getContentControl() && !selectionContains( ctrl ) )
   {
      mSelectedControls.push_back( ctrl );
      
      if( mSelectedControls.size() == 1 )
      {
         // Update the add set.
         
         if( ctrl->mIsContainer )
            setCurrentAddSet( ctrl, false );
         else
            setCurrentAddSet( ctrl->getParent(), false );
            
         // Notify script.

         onSelect_callback( ctrl );
      }
      else
      {
         // Notify script.
         
         onAddSelected_callback( ctrl );
      }
   }
}

//-----------------------------------------------------------------------------

void GuiEditCtrl::removeSelection( S32 id )
{
   GuiControl * ctrl;
   if ( Sim::findObject( id, ctrl ) )
      removeSelection( ctrl );
}

//-----------------------------------------------------------------------------

void GuiEditCtrl::removeSelection( GuiControl* ctrl )
{
   if( selectionContains( ctrl ) )
   {
      Vector< GuiControl* >::iterator i = ::find( mSelectedControls.begin(), mSelectedControls.end(), ctrl );
      if ( i != mSelectedControls.end() )
         mSelectedControls.erase( i );

      onRemoveSelected_callback( ctrl );
   }
}

//-----------------------------------------------------------------------------

void GuiEditCtrl::canHitSelectedControls( bool state )
{
   for( U32 i = 0, num = mSelectedControls.size(); i < num; ++ i )
      mSelectedControls[ i ]->setCanHit( state );
}

//-----------------------------------------------------------------------------

void GuiEditCtrl::moveSelectionToCtrl( GuiControl *newParent, bool callback )
{
   for( U32 i = 0; i < mSelectedControls.size(); ++ i )
   {
      GuiControl* ctrl = mSelectedControls[i];
      if( ctrl->getParent() == newParent
          || ctrl->isLocked()
          || selectionContainsParentOf( ctrl ) )
         continue;
   
      Point2I globalpos = ctrl->localToGlobalCoord(Point2I(0,0));
      newParent->addObject(ctrl);
      Point2I newpos = ctrl->globalToLocalCoord(globalpos) + ctrl->getPosition();
      ctrl->setPosition(newpos);
   }
   
   onHierarchyChanged_callback();
   
   //TODO: undo
}

//-----------------------------------------------------------------------------

static Point2I snapPoint(Point2I point, Point2I delta, Point2I gridSnap)
{ 
   S32 snap;
   if(gridSnap.x && delta.x)
   {
      snap = point.x % gridSnap.x;
      point.x -= snap;
      if(delta.x > 0 && snap != 0)
         point.x += gridSnap.x;
   }
   if(gridSnap.y && delta.y)
   {
      snap = point.y % gridSnap.y;
      point.y -= snap;
      if(delta.y > 0 && snap != 0)
         point.y += gridSnap.y;
   }
   return point;
}

void GuiEditCtrl::moveAndSnapSelection( const Point2I &delta, bool callback )
{
   // move / nudge gets a special callback so that multiple small moves can be
   // coalesced into one large undo action.
   // undo
   
   if( callback )
      onPreSelectionNudged_callback( getSelectedSet() );

   Vector<GuiControl *>::iterator i;
   Point2I newPos;
   for(i = mSelectedControls.begin(); i != mSelectedControls.end(); i++)
   {
      GuiControl* ctrl = *i;
      if( !ctrl->isLocked() && !selectionContainsParentOf( ctrl ) )
      {
         newPos = ctrl->getPosition() + delta;
         newPos = snapPoint( newPos, delta, mGridSnap );
         ctrl->setPosition( newPos );
      }
   }

   // undo
   if( callback )
      onPostSelectionNudged_callback( getSelectedSet() );

   // allow script to update the inspector
   if( callback && mSelectedControls.size() > 0 )
      onSelectionMoved_callback( mSelectedControls[ 0 ] );
}

//-----------------------------------------------------------------------------

void GuiEditCtrl::moveSelection( const Point2I &delta, bool callback )
{
   Vector<GuiControl *>::iterator i;
   for(i = mSelectedControls.begin(); i != mSelectedControls.end(); i++)
   {
      GuiControl* ctrl = *i;
      if( !ctrl->isLocked() && !selectionContainsParentOf( ctrl ) )
         ctrl->setPosition( ctrl->getPosition() + delta );
   }

   // allow script to update the inspector
   if( callback )
      onSelectionMoved_callback( mSelectedControls[ 0 ] );
}

//-----------------------------------------------------------------------------

void GuiEditCtrl::justifySelection( Justification j )
{
   S32 minX, maxX;
   S32 minY, maxY;
   S32 extentX, extentY;

   if (mSelectedControls.size() < 2)
      return;

   Vector<GuiControl *>::iterator i = mSelectedControls.begin();
   minX = (*i)->getLeft();
   maxX = minX + (*i)->getWidth();
   minY = (*i)->getTop();
   maxY = minY + (*i)->getHeight();
   extentX = (*i)->getWidth();
   extentY = (*i)->getHeight();
   i++;
   for(;i != mSelectedControls.end(); i++)
   {
      minX = getMin(minX, (*i)->getLeft());
      maxX = getMax(maxX, (*i)->getLeft() + (*i)->getWidth());
      minY = getMin(minY, (*i)->getTop());
      maxY = getMax(maxY, (*i)->getTop() + (*i)->getHeight());
      extentX += (*i)->getWidth();
      extentY += (*i)->getHeight();
   }
   S32 deltaX = maxX - minX;
   S32 deltaY = maxY - minY;
   switch(j)
   {
      case JUSTIFY_LEFT:
         for(i = mSelectedControls.begin(); i != mSelectedControls.end(); i++)
            if( !( *i )->isLocked() )
               (*i)->setLeft( minX );
         break;
      case JUSTIFY_TOP:
         for(i = mSelectedControls.begin(); i != mSelectedControls.end(); i++)
            if( !( *i )->isLocked() )
               (*i)->setTop( minY );
         break;
      case JUSTIFY_RIGHT:
         for(i = mSelectedControls.begin(); i != mSelectedControls.end(); i++)
            if( !( *i )->isLocked() )
               (*i)->setLeft( maxX - (*i)->getWidth() + 1 );
         break;
      case JUSTIFY_BOTTOM:
         for(i = mSelectedControls.begin(); i != mSelectedControls.end(); i++)
            if( !( *i )->isLocked() )
               (*i)->setTop( maxY - (*i)->getHeight() + 1 );
         break;
      case JUSTIFY_CENTER_VERTICAL:
         for(i = mSelectedControls.begin(); i != mSelectedControls.end(); i++)
            if( !( *i )->isLocked() )
               (*i)->setLeft( minX + ((deltaX - (*i)->getWidth()) >> 1 ));
         break;
      case JUSTIFY_CENTER_HORIZONTAL:
         for(i = mSelectedControls.begin(); i != mSelectedControls.end(); i++)
            if( !( *i )->isLocked() )
               (*i)->setTop( minY + ((deltaY - (*i)->getHeight()) >> 1 ));
         break;
      case SPACING_VERTICAL:
         {
            Vector<GuiControl *> sortedList;
            Vector<GuiControl *>::iterator k;
            for(i = mSelectedControls.begin(); i != mSelectedControls.end(); i++)
            {
               for(k = sortedList.begin(); k != sortedList.end(); k++)
               {
                  if ((*i)->getTop() < (*k)->getTop())
                     break;
               }
               sortedList.insert(k, *i);
            }
            S32 space = (deltaY - extentY) / (mSelectedControls.size() - 1);
            S32 curY = minY;
            for(k = sortedList.begin(); k != sortedList.end(); k++)
            {
               if( !( *k )->isLocked() )
                  (*k)->setTop( curY );
               curY += (*k)->getHeight() + space;
            }
         }
         break;
      case SPACING_HORIZONTAL:
         {
            Vector<GuiControl *> sortedList;
            Vector<GuiControl *>::iterator k;
            for(i = mSelectedControls.begin(); i != mSelectedControls.end(); i++)
            {
               for(k = sortedList.begin(); k != sortedList.end(); k++)
               {
                  if ((*i)->getLeft() < (*k)->getLeft())
                     break;
               }
               sortedList.insert(k, *i);
            }
            S32 space = (deltaX - extentX) / (mSelectedControls.size() - 1);
            S32 curX = minX;
            for(k = sortedList.begin(); k != sortedList.end(); k++)
            {
               if( !( *k )->isLocked() )
                  (*k)->setLeft( curX );
               curX += (*k)->getWidth() + space;
            }
         }
         break;
   }
}

//-----------------------------------------------------------------------------

void GuiEditCtrl::cloneSelection()
{
   Vector< GuiControl* > newSelection;
   
   // Clone the controls in the current selection.
   
   const U32 numOldControls = mSelectedControls.size();
   for( U32 i = 0; i < numOldControls; ++ i )
   {
      GuiControl* ctrl = mSelectedControls[ i ];
      
      // If parent is in selection, too, skip to prevent multiple clones.
      
      if( ctrl->getParent() && selectionContains( ctrl->getParent() ) )
         continue;
         
      // Clone and add to set.
      
      GuiControl* clone = dynamic_cast< GuiControl* >( ctrl->deepClone() );
      if( clone )
         newSelection.push_back( clone );
   }
   
   // Exchange the selection set.
   
   clearSelection();
   const U32 numNewControls = newSelection.size();
   for( U32 i = 0; i < numNewControls; ++ i )
      addSelection( newSelection[ i ] );
      
   // Callback for undo.
      
   onSelectionCloned_callback( getSelectedSet() );
}

//-----------------------------------------------------------------------------

void GuiEditCtrl::deleteSelection()
{
   // Notify script for undo.
   
   onTrashSelection_callback( getSelectedSet() );
   
   // Move all objects in selection to trash.

   Vector< GuiControl* >::iterator i;
   for( i = mSelectedControls.begin(); i != mSelectedControls.end(); i ++ )
   {
      if( ( *i ) == getCurrentAddSet() )
         setCurrentAddSet( getContentControl(), false );
         
      mTrash->addObject( *i );
   }
      
   clearSelection();
   
   // Notify script it needs to update its views.
   
   onHierarchyChanged_callback();
}

//-----------------------------------------------------------------------------

void GuiEditCtrl::loadSelection( const char* filename )
{
   // Set redefine behavior to rename.
   
   const char* oldRedefineBehavior = Con::getVariable( "$Con::redefineBehavior" );
   Con::setVariable( "$Con::redefineBehavior", "renameNew" );
   
   // Exec the file or clipboard contents with the saved selection set.

   if( filename )
      Con::executef( "exec", filename );
   else
      Con::evaluate( Platform::getClipboard() );
      
   SimSet* set;
   if( !Sim::findObject( "guiClipboard", set ) )
   {
      if( filename )
         Con::errorf( "GuiEditCtrl::loadSelection() - could not find 'guiClipboard' in '%s'", filename );
      else
         Con::errorf( "GuiEditCtrl::loadSelection() - could not find 'guiClipboard'" );
      return;
   }
   
   // Restore redefine behavior.
   
   Con::setVariable( "$Con::redefineBehavior", oldRedefineBehavior );
   
   // Add the objects in the set.

   if( set->size() )
   {
      clearSelection();

      GuiControlVector ctrls;
      for( U32 i = 0, num = set->size(); i < num; ++ i )
      {
         GuiControl *ctrl = dynamic_cast< GuiControl* >( ( *set )[ i ] );
         if( ctrl )
         {
            getCurrentAddSet()->addObject( ctrl );
            ctrls.push_back( ctrl );
         }
      }
      
      // Select all controls.  We need to perform this here rather than in the
      // loop above as addSelection() will modify the current add set.
      for( U32 i = 0; i < ctrls.size(); ++ i )
      {
         addSelection( ctrls[i] );
      }

      // Undo 
      onAddNewCtrlSet_callback( getSelectedSet() );

      // Notify the script it needs to update its treeview.

      onHierarchyChanged_callback();
   }
   set->deleteObject();
}

//-----------------------------------------------------------------------------

void GuiEditCtrl::saveSelection( const char* filename )
{
   // If there are no selected objects, then don't save.
   
   if( mSelectedControls.size() == 0 )
      return;
      
   // Open the stream.

   Stream* stream;
   if( filename )
   {
      stream = FileStream::createAndOpen( filename, Torque::FS::File::Write );
      if( !stream )
      {
         Con::errorf( "GuiEditCtrl::saveSelection - could not open '%s' for writing", filename );
         return;
      }
   }
   else
      stream = new MemStream( 4096 );
      
   // Create a temporary SimSet.
      
   SimSet* clipboardSet = new SimSet;
   clipboardSet->registerObject();
   Sim::getRootGroup()->addObject( clipboardSet, "guiClipboard" );
   
   // Add the selected controls to the set.

   for( Vector< GuiControl* >::iterator i = mSelectedControls.begin();
        i != mSelectedControls.end(); ++ i )
   {
      GuiControl* ctrl = *i;
      if( !selectionContainsParentOf( ctrl ) )
         clipboardSet->addObject( ctrl );
   }
   
   // Write the SimSet.  Use the IgnoreCanSave to ensure the controls
   // get actually written out (also disables the default parent inheritance
   // behavior for the flag).

   clipboardSet->write( *stream, 0, IgnoreCanSave );
   clipboardSet->deleteObject();
   
   // If we were writing to a memory stream, copy to clipboard
   // now.
   
   if( !filename )
   {
      MemStream* memStream = static_cast< MemStream* >( stream );
      memStream->write( U8( 0 ) );
      Platform::setClipboard( ( const char* ) memStream->getBuffer() );
   }

   delete stream;
}

//-----------------------------------------------------------------------------

void GuiEditCtrl::selectAll()
{
   GuiControl::iterator i;
   
   clearSelection();
   for(i = getCurrentAddSet()->begin(); i != getCurrentAddSet()->end(); i++)
   {
      GuiControl *ctrl = dynamic_cast<GuiControl *>(*i);
      addSelection( ctrl );
   }
}

//-----------------------------------------------------------------------------

void GuiEditCtrl::bringToFront()
{
   if( getNumSelected() != 1 )
      return;

   GuiControl* ctrl = mSelectedControls.first();
   ctrl->getParent()->pushObjectToBack( ctrl );
}

//-----------------------------------------------------------------------------

void GuiEditCtrl::pushToBack()
{
   if( getNumSelected() != 1 )
      return;

   GuiControl* ctrl = mSelectedControls.first();
   ctrl->getParent()->bringObjectToFront( ctrl );
}

//-----------------------------------------------------------------------------

RectI GuiEditCtrl::getSelectionBounds() const
{
   Vector<GuiControl *>::const_iterator i = mSelectedControls.begin();
   
   Point2I minPos = (*i)->localToGlobalCoord( Point2I( 0, 0 ) );
   Point2I maxPos = minPos;
   
   for(; i != mSelectedControls.end(); i++)
   {
      Point2I iPos = (**i).localToGlobalCoord( Point2I( 0 , 0 ) );
      
      minPos.x = getMin( iPos.x, minPos.x );
      minPos.y = getMin( iPos.y, minPos.y );
         
      Point2I iExt = ( **i ).getExtent();
      
      iPos.x += iExt.x;
      iPos.y += iExt.y;

      maxPos.x = getMax( iPos.x, maxPos.x );
      maxPos.y = getMax( iPos.y, maxPos.y );
   }
   
   minPos = getContentControl()->globalToLocalCoord( minPos );
   maxPos = getContentControl()->globalToLocalCoord( maxPos );
   
   return RectI( minPos.x, minPos.y, ( maxPos.x - minPos.x ), ( maxPos.y - minPos.y ) );
}

//-----------------------------------------------------------------------------

RectI GuiEditCtrl::getSelectionGlobalBounds() const
{
   Point2I minb( S32_MAX, S32_MAX );
   Point2I maxb( S32_MIN, S32_MIN );
   
   for( U32 i = 0, num = mSelectedControls.size(); i < num; ++ i )
   {
      // Min.

      Point2I pos = mSelectedControls[ i ]->localToGlobalCoord( Point2I( 0, 0 ) );
      
      minb.x = getMin( minb.x, pos.x );
      minb.y = getMin( minb.y, pos.y );
      
      // Max.
      
      const Point2I extent = mSelectedControls[ i ]->getExtent();
      
      maxb.x = getMax( maxb.x, pos.x + extent.x );
      maxb.y = getMax( maxb.y, pos.y + extent.y );
   }
   
   RectI bounds( minb.x, minb.y, maxb.x - minb.x, maxb.y - minb.y );
   return bounds;
}

//-----------------------------------------------------------------------------

bool GuiEditCtrl::selectionContains( GuiControl *ctrl )
{
   Vector<GuiControl *>::iterator i;
   for (i = mSelectedControls.begin(); i != mSelectedControls.end(); i++)
      if (ctrl == *i) return true;
   return false;
}

//-----------------------------------------------------------------------------

bool GuiEditCtrl::selectionContainsParentOf( GuiControl* ctrl )
{
   GuiControl* parent = ctrl->getParent();
   while( parent && parent != getContentControl() )
   {
      if( selectionContains( parent ) )
         return true;
         
      parent = parent->getParent();
   }
   
   return false;
}

//-----------------------------------------------------------------------------

void GuiEditCtrl::select( GuiControl* ctrl )
{
   clearSelection();
   addSelection( ctrl );
}

//-----------------------------------------------------------------------------

void GuiEditCtrl::updateSelectedSet()
{
   mSelectedSet->clear();
   Vector<GuiControl*>::iterator i;
   for(i = mSelectedControls.begin(); i != mSelectedControls.end(); i++)
   {
      mSelectedSet->addObject(*i);
   }
}

//-----------------------------------------------------------------------------

void GuiEditCtrl::addSelectControlsInRegion( const RectI& rect, U32 flags )
{
   // Do a hit test on the content control.
   
   canHitSelectedControls( false );
   Vector< GuiControl* > hits;
   
   if( mFullBoxSelection )
      flags |= GuiControl::HIT_FullBoxOnly;
      
   getContentControl()->findHitControls( rect, hits, flags );
   canHitSelectedControls( true );
   
   // Add all controls that got hit.
   
   for( U32 i = 0, num = hits.size(); i < num; ++ i )
      addSelection( hits[ i ] );
}

//-----------------------------------------------------------------------------

void GuiEditCtrl::addSelectControlAt( const Point2I& pos )
{
   // Run a hit test.
   
   canHitSelectedControls( false );
   GuiControl* hit = getContentControl()->findHitControl( pos );
   canHitSelectedControls( true );
   
   // Add to selection.
   
   if( hit )
      addSelection( hit );
}

//-----------------------------------------------------------------------------

void GuiEditCtrl::resizeControlsInSelectionBy( const Point2I& delta, U32 mode )
{
   for( U32 i = 0, num = mSelectedControls.size(); i < num; ++ i )
   {
      GuiControl *ctrl = mSelectedControls[ i ];
      if( ctrl->isLocked() )
         continue;
         
      Point2I minExtent    = ctrl->getMinExtent();
      Point2I newPosition  = ctrl->getPosition();
      Point2I newExtent    = ctrl->getExtent();

      if( mSizingMode & sizingLeft )
      {
         newPosition.x     += delta.x;
         newExtent.x       -= delta.x;
         
         if( newExtent.x < minExtent.x )
         {
            newPosition.x  -= minExtent.x - newExtent.x;
            newExtent.x    = minExtent.x;
         }
      }
      else if( mSizingMode & sizingRight )
      {
         newExtent.x       += delta.x;

         if( newExtent.x < minExtent.x )
            newExtent.x    = minExtent.x;
      }

      if( mSizingMode & sizingTop )
      {
         newPosition.y     += delta.y;
         newExtent.y       -= delta.y;
         
         if( newExtent.y < minExtent.y )
         {
            newPosition.y  -= minExtent.y - newExtent.y;
            newExtent.y    = minExtent.y;
         }
      }
      else if( mSizingMode & sizingBottom )
      {
         newExtent.y       += delta.y;
         
         if( newExtent.y < minExtent.y )
            newExtent.y = minExtent.y;
      }
      
      ctrl->resize( newPosition, newExtent );
   }
   
   if( mSelectedControls.size() == 1 )
      onSelectionResized_callback( mSelectedControls[ 0 ] );
}

//-----------------------------------------------------------------------------

void GuiEditCtrl::fitIntoParents( bool width, bool height )
{
   // Record undo.
   
   onFitIntoParent_callback( width, height );
   
   // Fit.
   
   for( U32 i = 0; i < mSelectedControls.size(); ++ i )
   {
      GuiControl* ctrl = mSelectedControls[ i ];
      GuiControl* parent = ctrl->getParent();
      
      Point2I position = ctrl->getPosition();
      if( width )
         position.x = 0;
      if( height )
         position.y = 0;
      
      Point2I extents = ctrl->getExtent();
      if( width )
         extents.x = parent->getWidth();
      if( height )
         extents.y = parent->getHeight();
      
      ctrl->resize( position, extents );
   }
}

//-----------------------------------------------------------------------------

void GuiEditCtrl::selectParents( bool addToSelection )
{
   Vector< GuiControl* > parents;
   
   // Collect all parents.
   
   for( U32 i = 0; i < mSelectedControls.size(); ++ i )
   {
      GuiControl* ctrl = mSelectedControls[ i ];
      if( ctrl != mContentControl && ctrl->getParent() != mContentControl )
         parents.push_back( mSelectedControls[ i ]->getParent() );
   }
   
   // If there's no parents to select, don't
   // change the selection.
   
   if( parents.empty() )
      return;
   
   // Blast selection if need be.
   
   if( !addToSelection )
      clearSelection();
      
   // Add the parents.
   
   for( U32 i = 0; i < parents.size(); ++ i )
      addSelection( parents[ i ] );
}

//-----------------------------------------------------------------------------

void GuiEditCtrl::selectChildren( bool addToSelection )
{
   Vector< GuiControl* > children;
   
   // Collect all children.
   
   for( U32 i = 0; i < mSelectedControls.size(); ++ i )
   {
      GuiControl* parent = mSelectedControls[ i ];
      for( GuiControl::iterator iter = parent->begin(); iter != parent->end(); ++ iter )
      {
         GuiControl* child = dynamic_cast< GuiControl* >( *iter );
         if( child )
            children.push_back( child );
      }
   }
   
   // If there's no children to select, don't
   // change the selection.
   
   if( children.empty() )
      return;

   // Blast selection if need be.
   
   if( !addToSelection )
      clearSelection();
      
   // Add the children.
   
   for( U32 i = 0; i < children.size(); ++ i )
      addSelection( children[ i ] );
}

//=============================================================================
//    Guides.
//=============================================================================
// MARK: ---- Guides ----

//-----------------------------------------------------------------------------

void GuiEditCtrl::readGuides( guideAxis axis, GuiControl* ctrl )
{
   // Read the guide indices from the vector stored on the respective dynamic
   // property of the control.
   
   const char* guideIndices = ctrl->getDataField( smGuidesPropertyName[ axis ], NULL );
   if( guideIndices && guideIndices[ 0 ] )
   {
      U32 index = 0;
      while( true )
      {
         const char* posStr = StringUnit::getUnit( guideIndices, index, " \t" );
         if( !posStr[ 0 ] )
            break;
            
         mGuides[ axis ].push_back( dAtoi( posStr ) );
         
         index ++;
      }
   }
}

//-----------------------------------------------------------------------------

void GuiEditCtrl::writeGuides( guideAxis axis, GuiControl* ctrl )
{
   // Store the guide indices of the given axis in a vector on the respective
   // dynamic property of the control.
   
   StringBuilder str;
   bool isFirst = true;
   for( U32 i = 0, num = mGuides[ axis ].size(); i < num; ++ i )
   {
      if( !isFirst )
         str.append( ' ' );
         
      char buffer[ 32 ];
      dSprintf( buffer, sizeof( buffer ), "%i", mGuides[ axis ][ i ] );
      
      str.append( buffer );
      
      isFirst = false;
   }
   
   String value = str.end();
   ctrl->setDataField( smGuidesPropertyName[ axis ], NULL, value );
}

//-----------------------------------------------------------------------------

S32 GuiEditCtrl::findGuide( guideAxis axis, const Point2I& point, U32 tolerance )
{
   const S32 p = ( point - localToGlobalCoord( Point2I( 0, 0 ) ) )[ axis ];
   
   for( U32 i = 0, num = mGuides[ axis ].size(); i < num; ++ i )
   {
      const S32 g = mGuides[ axis ][ i ];
      if( p >= ( g - tolerance ) &&
          p <= ( g + tolerance ) )
         return i;
   }
         
   return -1;
}

//=============================================================================
//    Snapping.
//=============================================================================
// MARK: ---- Snapping ----

//-----------------------------------------------------------------------------

RectI GuiEditCtrl::getSnapRegion( snappingAxis axis, const Point2I& center ) const
{
   RectI rootBounds = getContentControl()->getBounds();
   
   RectI rect;
   if( axis == SnapHorizontal )
      rect = RectI( rootBounds.point.x,
                    center.y - mSnapSensitivity,
                    rootBounds.extent.x,
                    mSnapSensitivity * 2 );
   else // SnapVertical
      rect = RectI( center.x - mSnapSensitivity,
                    rootBounds.point.y,
                    mSnapSensitivity * 2,
                    rootBounds.extent.y );
   
   // Clip against root bounds.
   
   rect.intersect( rootBounds );
      
   return rect;
}

//-----------------------------------------------------------------------------

void GuiEditCtrl::registerSnap( snappingAxis axis, const Point2I& mousePoint, const Point2I& point, snappingEdges edge, GuiControl* ctrl )
{
   bool takeNewSnap = false;
   const Point2I globalPoint = getContentControl()->localToGlobalCoord( point );

   // If we have no snap yet, just take this one.
   
   if( !mSnapped[ axis ] )
      takeNewSnap = true;
   
   // Otherwise see if this snap is the better one.
   
   else
   {
      // Compare deltas to pointer.
      
      S32 deltaCurrent = mAbs( mSnapOffset[ axis ] - mousePoint[ axis ] );
      S32 deltaNew = mAbs( globalPoint[ axis ] - mousePoint[ axis ] );
                              
      if( deltaCurrent > deltaNew )
         takeNewSnap = true;
   }

   if( takeNewSnap )
   {      
      mSnapped[ axis ]     = true;
      mSnapOffset[ axis ]  = globalPoint[ axis ];
      mSnapEdge[ axis ]    = edge;
      mSnapTargets[ axis ] = ctrl;
   }
}

//-----------------------------------------------------------------------------

void GuiEditCtrl::findSnaps( snappingAxis axis, const Point2I& mousePoint, const RectI& minRegion, const RectI& midRegion, const RectI& maxRegion )
{
   // Find controls with edge in either minRegion, midRegion, or maxRegion
   // (depending on snap settings).
   
   for( U32 i = 0, num = mSnapHits[ axis ].size(); i < num; ++ i )
   {
      GuiControl* ctrl = mSnapHits[ axis ][ i ];
      if( ctrl == getContentControl() && !mSnapToCanvas )
         continue;
         
      RectI bounds = ctrl->getGlobalBounds();
      bounds.point = getContentControl()->globalToLocalCoord( bounds.point );
      
      // Compute points on min, mid, and max lines of control.
      
      Point2I min = bounds.point;
      Point2I max = min + bounds.extent - Point2I( 1, 1 );

      Point2I mid = min;
      mid.x += bounds.extent.x / 2;
      mid.y += bounds.extent.y / 2;
            
      // Test edge snap cases.
      
      if( mSnapToEdges )
      {
         // Min to min.
         
         if( minRegion.pointInRect( min ) )
            registerSnap( axis, mousePoint, min, SnapEdgeMin, ctrl );
      
         // Max to max.
         
         if( maxRegion.pointInRect( max ) )
            registerSnap( axis, mousePoint, max, SnapEdgeMax, ctrl );
         
         // Min to max.
         
         if( minRegion.pointInRect( max ) )
            registerSnap( axis, mousePoint, max, SnapEdgeMin, ctrl );
         
         // Max to min.
         
         if( maxRegion.pointInRect( min ) )
            registerSnap( axis, mousePoint, min, SnapEdgeMax, ctrl );
      }
      
      // Test center snap cases.
      
      if( mSnapToCenters )
      {
         // Mid to mid.
         
         if( midRegion.pointInRect( mid ) )
            registerSnap( axis, mousePoint, mid, SnapEdgeMid, ctrl );
      }
      
      // Test combined center+edge snap cases.
      
      if( mSnapToEdges && mSnapToCenters )
      {
         // Min to mid.
         
         if( minRegion.pointInRect( mid ) )
            registerSnap( axis, mousePoint, mid, SnapEdgeMin, ctrl );
         
         // Max to mid.
         
         if( maxRegion.pointInRect( mid ) )
            registerSnap( axis, mousePoint, mid, SnapEdgeMax, ctrl );
         
         // Mid to min.
         
         if( midRegion.pointInRect( min ) )
            registerSnap( axis, mousePoint, min, SnapEdgeMid, ctrl );
         
         // Mid to max.
         
         if( midRegion.pointInRect( max ) )
            registerSnap( axis, mousePoint, max, SnapEdgeMid, ctrl );
      }
   }
}

//-----------------------------------------------------------------------------

void GuiEditCtrl::doControlSnap( const GuiEvent& event, const RectI& selectionBounds, const RectI& selectionBoundsGlobal, Point2I& delta )
{
   if( !mSnapToControls || ( !mSnapToEdges && !mSnapToCenters ) )
      return;
      
   // Allow restricting to just vertical (ALT+SHIFT) or just horizontal (ALT+CTRL)
   // snaps.
   
   bool snapAxisEnabled[ 2 ];
            
   if( event.modifier & SI_PRIMARY_ALT && event.modifier & SI_SHIFT )
      snapAxisEnabled[ SnapHorizontal ] = false;
   else
      snapAxisEnabled[ SnapHorizontal ] = true;
      
   if( event.modifier & SI_PRIMARY_ALT && event.modifier & SI_CTRL )
      snapAxisEnabled[ SnapVertical ] = false;
   else
      snapAxisEnabled[ SnapVertical ] = true;
   
   // Compute snap regions.  There is one region centered on and aligned with
   // each of the selection bounds edges plus two regions aligned on the selection
   // bounds center.  For the selection bounds origin, we use the point that the
   // selection would be at, if we had already done the mouse drag.
   
   RectI snapRegions[ 2 ][ 3 ];
   Point2I projectedOrigin( selectionBounds.point + delta );
   dMemset( snapRegions, 0, sizeof( snapRegions ) );
   
   for( U32 axis = 0; axis < 2; ++ axis )
   {
      if( !snapAxisEnabled[ axis ] )
         continue;
         
      if( mSizingMode == sizingNone ||
          ( axis == 0 && mSizingMode & sizingLeft ) ||
          ( axis == 1 && mSizingMode & sizingTop ) )
         snapRegions[ axis ][ 0 ] = getSnapRegion( ( snappingAxis ) axis, projectedOrigin );
         
      if( mSizingMode == sizingNone )
         snapRegions[ axis ][ 1 ] = getSnapRegion( ( snappingAxis ) axis, projectedOrigin + Point2I( selectionBounds.extent.x / 2, selectionBounds.extent.y / 2 ) );
         
      if( mSizingMode == sizingNone ||
          ( axis == 0 && mSizingMode & sizingRight ) ||
          ( axis == 1 && mSizingMode & sizingBottom ) )
         snapRegions[ axis ][ 2 ] = getSnapRegion( ( snappingAxis ) axis, projectedOrigin + selectionBounds.extent - Point2I( 1, 1 ) );
   }
   
   // Find hit controls.
   
   canHitSelectedControls( false );
   
   if( mSnapToEdges )
   {
      for( U32 axis = 0; axis < 2; ++ axis )
         if( snapAxisEnabled[ axis ] )
         {
            getContentControl()->findHitControls( snapRegions[ axis ][ 0 ], mSnapHits[ axis ], HIT_NoCanHitNoRecurse );
            getContentControl()->findHitControls( snapRegions[ axis ][ 2 ], mSnapHits[ axis ], HIT_NoCanHitNoRecurse );
         }
   }
   if( mSnapToCenters && mSizingMode == sizingNone )
   {
      for( U32 axis = 0; axis < 2; ++ axis )
         if( snapAxisEnabled[ axis ] )
            getContentControl()->findHitControls( snapRegions[ axis ][ 1 ], mSnapHits[ axis ], HIT_NoCanHitNoRecurse );
   }
   
   canHitSelectedControls( true );
   
   // Add the content control itself to the hit controls
   // so we can always get a snap on it.
   
   if( mSnapToCanvas )
   {
      mSnapHits[ 0 ].push_back( mContentControl );
      mSnapHits[ 1 ].push_back( mContentControl );
   }
            
   // Find snaps.
   
   for( U32 i = 0; i < 2; ++ i )
      if( snapAxisEnabled[ i ] )
         findSnaps( ( snappingAxis ) i,
                    event.mousePoint,
                    snapRegions[ i ][ 0 ],
                    snapRegions[ i ][ 1 ],
                    snapRegions[ i ][ 2 ] );
                                       
   // Clean up.
   
   mSnapHits[ 0 ].clear();
   mSnapHits[ 1 ].clear();
}

//-----------------------------------------------------------------------------

void GuiEditCtrl::doGridSnap( const GuiEvent& event, const RectI& selectionBounds, const RectI& selectionBoundsGlobal, Point2I& delta )
{
   delta += selectionBounds.point;
         
   if( mGridSnap.x )
      delta.x -= delta.x % mGridSnap.x;
   if( mGridSnap.y )
      delta.y -= delta.y % mGridSnap.y;

   delta -= selectionBounds.point;
}

//-----------------------------------------------------------------------------

void GuiEditCtrl::doGuideSnap( const GuiEvent& event, const RectI& selectionBounds, const RectI& selectionBoundsGlobal, Point2I& delta )
{
   if( !mSnapToGuides )
      return;
      
   Point2I min = getContentControl()->localToGlobalCoord( selectionBounds.point + delta );
   Point2I mid = min + selectionBounds.extent / 2;
   Point2I max = min + selectionBounds.extent - Point2I( 1, 1 );
   
   for( U32 axis = 0; axis < 2; ++ axis )
   {
      if( mSnapToEdges )
      {
         S32 guideMin = -1;
         S32 guideMax = -1;
         
         if( mSizingMode == sizingNone ||
             ( axis == 0 && mSizingMode & sizingLeft ) ||
             ( axis == 1 && mSizingMode & sizingTop ) )
            guideMin = findGuide( ( guideAxis ) axis, min, mSnapSensitivity );
         if( mSizingMode == sizingNone ||
             ( axis == 0 && mSizingMode & sizingRight ) ||
             ( axis == 1 && mSizingMode & sizingBottom ) )
            guideMax = findGuide( ( guideAxis ) axis, max, mSnapSensitivity );
         
         Point2I pos( 0, 0 );
         
         if( guideMin != -1 )
         {
            pos[ axis ] = mGuides[ axis ][ guideMin ];
            registerSnap( ( snappingAxis ) axis, event.mousePoint, pos, SnapEdgeMin );
         }
         
         if( guideMax != -1 )
         {
            pos[ axis ] = mGuides[ axis ][ guideMax ];
            registerSnap( ( snappingAxis ) axis, event.mousePoint, pos, SnapEdgeMax );
         }
      }
      
      if( mSnapToCenters && mSizingMode == sizingNone )
      {
         const S32 guideMid = findGuide( ( guideAxis ) axis, mid, mSnapSensitivity );
         if( guideMid != -1 )
         {
            Point2I pos( 0, 0 );
            pos[ axis ] = mGuides[ axis ][ guideMid ];
            registerSnap( ( snappingAxis ) axis, event.mousePoint, pos, SnapEdgeMid );
         }
      }
   }
}

//-----------------------------------------------------------------------------

void GuiEditCtrl::doSnapping( const GuiEvent& event, const RectI& selectionBounds, Point2I& delta )
{
   // Clear snapping.  If we have snapping on, we want to find a new best snap.
   
   mSnapped[ SnapVertical ] = false;
   mSnapped[ SnapHorizontal ] = false;
   
   // Compute global bounds.
   
   RectI selectionBoundsGlobal = selectionBounds;
   selectionBoundsGlobal.point = getContentControl()->localToGlobalCoord( selectionBoundsGlobal.point );

   // Apply the snaps.
   
   doGridSnap( event, selectionBounds, selectionBoundsGlobal, delta );
   doGuideSnap( event, selectionBounds, selectionBoundsGlobal, delta );
   doControlSnap( event, selectionBounds, selectionBoundsGlobal, delta );

  // If we have a horizontal snap, compute a delta.

   if( mSnapped[ SnapVertical ] )
      snapDelta( SnapVertical, mSnapEdge[ SnapVertical ], mSnapOffset[ SnapVertical ], selectionBoundsGlobal, delta );
   
   // If we have a vertical snap, compute a delta.
   
   if( mSnapped[ SnapHorizontal ] )
      snapDelta( SnapHorizontal, mSnapEdge[ SnapHorizontal ], mSnapOffset[ SnapHorizontal ], selectionBoundsGlobal, delta );
}

//-----------------------------------------------------------------------------

void GuiEditCtrl::snapToGrid( Point2I& point )
{
   if( mGridSnap.x )
      point.x -= point.x % mGridSnap.x;
   if( mGridSnap.y )
      point.y -= point.y % mGridSnap.y;
}

//=============================================================================
//    Misc.
//=============================================================================
// MARK: ---- Misc ----

//-----------------------------------------------------------------------------

void GuiEditCtrl::setContentControl(GuiControl *root)
{
   mContentControl = root;
   if( root != NULL )
      root->mIsContainer = true;
      
   setCurrentAddSet( root );
}

//-----------------------------------------------------------------------------

void GuiEditCtrl::setEditMode(bool value)
{
   mActive = value;

   clearSelection();
   if( mActive && mAwake )
      mCurrentAddSet = NULL;
}

//-----------------------------------------------------------------------------

void GuiEditCtrl::setMouseMode( mouseModes mode )
{
   if( mMouseDownMode != mode )
   {
      mMouseDownMode = mode;

      onMouseModeChange_callback();
   }
}

//-----------------------------------------------------------------------------

void GuiEditCtrl::setCurrentAddSet(GuiControl *ctrl, bool doclearSelection)
{
   if (ctrl != mCurrentAddSet)
   {
      if(doclearSelection)
         clearSelection();

      mCurrentAddSet = ctrl;
   }
}

//-----------------------------------------------------------------------------

GuiControl* GuiEditCtrl::getCurrentAddSet()
{
   if( !mCurrentAddSet )
      setCurrentAddSet( mContentControl, false );
      
   return mCurrentAddSet;
}

//-----------------------------------------------------------------------------

void GuiEditCtrl::addNewControl(GuiControl *ctrl)
{
   getCurrentAddSet()->addObject(ctrl);
   select( ctrl );

   // undo
   onAddNewCtrl_callback( ctrl );
}

//-----------------------------------------------------------------------------

S32 GuiEditCtrl::getSizingHitKnobs(const Point2I &pt, const RectI &box)
{
   S32 lx = box.point.x, rx = box.point.x + box.extent.x - 1;
   S32 cx = (lx + rx) >> 1;
   S32 ty = box.point.y, by = box.point.y + box.extent.y - 1;
   S32 cy = (ty + by) >> 1;
   
   // adjust nuts, so they dont straddle the controls
   lx -= NUT_SIZE;
   ty -= NUT_SIZE;
   rx += NUT_SIZE;
   by += NUT_SIZE;

   if (inNut(pt, lx, ty))
      return sizingLeft | sizingTop;
   if (inNut(pt, cx, ty))
      return sizingTop;
   if (inNut(pt, rx, ty))
      return sizingRight | sizingTop;
   if (inNut(pt, lx, by))
      return sizingLeft | sizingBottom;
   if (inNut(pt, cx, by))
      return sizingBottom;
   if (inNut(pt, rx, by))
      return sizingRight | sizingBottom;
   if (inNut(pt, lx, cy))
      return sizingLeft;
   if (inNut(pt, rx, cy))
      return sizingRight;
   return sizingNone;
}

//-----------------------------------------------------------------------------

void GuiEditCtrl::getDragRect(RectI &box)
{
   box.point.x = getMin(mLastMousePos.x, mSelectionAnchor.x);
   box.extent.x = getMax(mLastMousePos.x, mSelectionAnchor.x) - box.point.x + 1;
   box.point.y = getMin(mLastMousePos.y, mSelectionAnchor.y);
   box.extent.y = getMax(mLastMousePos.y, mSelectionAnchor.y) - box.point.y + 1;
}

//-----------------------------------------------------------------------------

void GuiEditCtrl::getCursor(GuiCursor *&cursor, bool &showCursor, const GuiEvent &lastGuiEvent)
{
   GuiCanvas *pRoot = getRoot();
   if( !pRoot )
      return;

   showCursor = false;
   cursor = NULL;
   
   Point2I ctOffset;
   Point2I cext;
   GuiControl *ctrl;

   Point2I mousePos  = globalToLocalCoord(lastGuiEvent.mousePoint);

   PlatformWindow *pWindow = static_cast<GuiCanvas*>(getRoot())->getPlatformWindow();
   AssertFatal(pWindow != NULL,"GuiControl without owning platform window!  This should not be possible.");
   PlatformCursorController *pController = pWindow->getCursorController();
   AssertFatal(pController != NULL,"PlatformWindow without an owned CursorController!");

   S32 desiredCursor = PlatformCursorController::curArrow;

   // first see if we hit a sizing knob on the currently selected control...
   if (mSelectedControls.size() == 1 )
   {
      ctrl = mSelectedControls.first();
      cext = ctrl->getExtent();
      ctOffset = globalToLocalCoord(ctrl->localToGlobalCoord(Point2I(0,0)));
      RectI box(ctOffset.x,ctOffset.y,cext.x, cext.y);

      GuiEditCtrl::sizingModes sizeMode = (GuiEditCtrl::sizingModes)getSizingHitKnobs(mousePos, box);

      if( mMouseDownMode == SizingSelection )
      {
         if ( ( mSizingMode == ( sizingBottom | sizingRight ) ) || ( mSizingMode == ( sizingTop | sizingLeft ) ) )
            desiredCursor = PlatformCursorController::curResizeNWSE;
         else if (  ( mSizingMode == ( sizingBottom | sizingLeft ) ) || ( mSizingMode == ( sizingTop | sizingRight ) ) )
            desiredCursor = PlatformCursorController::curResizeNESW;
         else if ( mSizingMode == sizingLeft || mSizingMode == sizingRight ) 
            desiredCursor = PlatformCursorController::curResizeVert;
         else if (mSizingMode == sizingTop || mSizingMode == sizingBottom )
            desiredCursor = PlatformCursorController::curResizeHorz;
      }
      else
      {
         // Check for current mouse position after checking for actual sizing mode
         if ( ( sizeMode == ( sizingBottom | sizingRight ) ) || ( sizeMode == ( sizingTop | sizingLeft ) ) )
            desiredCursor = PlatformCursorController::curResizeNWSE;
         else if ( ( sizeMode == ( sizingBottom | sizingLeft ) ) || ( sizeMode == ( sizingTop | sizingRight ) ) )
            desiredCursor = PlatformCursorController::curResizeNESW;
         else if (sizeMode == sizingLeft || sizeMode == sizingRight )
            desiredCursor = PlatformCursorController::curResizeVert;
         else if (sizeMode == sizingTop || sizeMode == sizingBottom )
            desiredCursor = PlatformCursorController::curResizeHorz;
      }
   }
   
   if( mMouseDownMode == MovingSelection && cursor == NULL )
      desiredCursor = PlatformCursorController::curResizeAll;

   if( pRoot->mCursorChanged != desiredCursor )
   {
      // We've already changed the cursor, 
      // so set it back before we change it again.
      if(pRoot->mCursorChanged != -1)
         pController->popCursor();

      // Now change the cursor shape
      pController->pushCursor(desiredCursor);
      pRoot->mCursorChanged = desiredCursor;
   }
}

//-----------------------------------------------------------------------------

void GuiEditCtrl::setSnapToGrid(U32 gridsize)
{
   if( gridsize != 0 )
      gridsize = getMax( gridsize, ( U32 ) MIN_GRID_SIZE );
   mGridSnap.set( gridsize, gridsize );
}

//-----------------------------------------------------------------------------

void GuiEditCtrl::controlInspectPreApply(GuiControl* object)
{
   // undo
   onControlInspectPreApply_callback( object );
}

//-----------------------------------------------------------------------------

void GuiEditCtrl::controlInspectPostApply(GuiControl* object)
{
   // undo
   onControlInspectPostApply_callback( object );
}

//-----------------------------------------------------------------------------

void GuiEditCtrl::startDragMove( const Point2I& startPoint )
{
   mDragMoveUndo = true;
   
   // For calculating mouse delta
   mDragBeginPoint = globalToLocalCoord( startPoint );

   // Allocate enough space for our selected controls
   mDragBeginPoints.reserve( mSelectedControls.size() );

   // For snapping to origin
   Vector<GuiControl *>::iterator i;
   for(i = mSelectedControls.begin(); i != mSelectedControls.end(); i++)
      mDragBeginPoints.push_back( (*i)->getPosition() );

   // Set Mouse Mode
   setMouseMode( MovingSelection );
   
   // undo
   onPreEdit_callback( getSelectedSet() );
}

//-----------------------------------------------------------------------------

void GuiEditCtrl::startDragRectangle( const Point2I& startPoint )
{
   mSelectionAnchor = globalToLocalCoord( startPoint );
   setMouseMode( DragSelecting );
}

//-----------------------------------------------------------------------------

void GuiEditCtrl::startDragClone( const Point2I& startPoint )
{
   mDragBeginPoint = globalToLocalCoord( startPoint );
   
   setMouseMode( DragClone );
}

//-----------------------------------------------------------------------------

void GuiEditCtrl::startMouseGuideDrag( guideAxis axis, U32 guideIndex, bool lockMouse )
{
   mDragGuideIndex[ axis ] = guideIndex;
   mDragGuide[ axis ] = true;
   
   setMouseMode( DragGuide );
   
   // Grab the mouse.
   
   if( lockMouse )
      mouseLock();
}

//=============================================================================
//    Console Methods.
//=============================================================================
// MARK: ---- Console Methods ----

//-----------------------------------------------------------------------------

ConsoleMethod( GuiEditCtrl, getContentControl, S32, 2, 2, "() - Return the toplevel control edited inside the GUI editor." )
{
   GuiControl* ctrl = object->getContentControl();
   if( ctrl )
      return ctrl->getId();
   else
      return 0;
}

//-----------------------------------------------------------------------------

ConsoleMethod( GuiEditCtrl, setContentControl, void, 3, 3, "( GuiControl ctrl ) - Set the toplevel control to edit in the GUI editor." )
{
   GuiControl *ctrl;
   if(!Sim::findObject(argv[2], ctrl))
      return;
   object->setContentControl(ctrl);
}

//-----------------------------------------------------------------------------

ConsoleMethod( GuiEditCtrl, addNewCtrl, void, 3, 3, "(GuiControl ctrl)")
{
   GuiControl *ctrl;
   if(!Sim::findObject(argv[2], ctrl))
      return;
   object->addNewControl(ctrl);
}

//-----------------------------------------------------------------------------

ConsoleMethod( GuiEditCtrl, addSelection, void, 3, 3, "selects a control.")
{
   S32 id = dAtoi(argv[2]);
   object->addSelection(id);
}

//-----------------------------------------------------------------------------

ConsoleMethod( GuiEditCtrl, removeSelection, void, 3, 3, "deselects a control.")
{
   S32 id = dAtoi(argv[2]);
   object->removeSelection(id);
}

//-----------------------------------------------------------------------------

ConsoleMethod( GuiEditCtrl, clearSelection, void, 2, 2, "Clear selected controls list.")
{
   object->clearSelection();
}

//-----------------------------------------------------------------------------

ConsoleMethod( GuiEditCtrl, select, void, 3, 3, "(GuiControl ctrl)")
{
   GuiControl *ctrl;

   if(!Sim::findObject(argv[2], ctrl))
      return;

   object->setSelection(ctrl, false);
}

//-----------------------------------------------------------------------------

ConsoleMethod( GuiEditCtrl, setCurrentAddSet, void, 3, 3, "(GuiControl ctrl)")
{
   GuiControl *addSet;

   if (!Sim::findObject(argv[2], addSet))
   {
      Con::printf("%s(): Invalid control: %s", argv[0], argv[2]);
      return;
   }
   object->setCurrentAddSet(addSet);
}

//-----------------------------------------------------------------------------

ConsoleMethod( GuiEditCtrl, getCurrentAddSet, S32, 2, 2, "Returns the set to which new controls will be added")
{
   const GuiControl* add = object->getCurrentAddSet();
   return add ? add->getId() : 0;
}

//-----------------------------------------------------------------------------

ConsoleMethod( GuiEditCtrl, toggle, void, 2, 2, "Toggle activation.")
{
   object->setEditMode( !object->isActive() );
}

//-----------------------------------------------------------------------------

ConsoleMethod( GuiEditCtrl, justify, void, 3, 3, "(int mode)" )
{
   object->justifySelection((GuiEditCtrl::Justification)dAtoi(argv[2]));
}

//-----------------------------------------------------------------------------

ConsoleMethod( GuiEditCtrl, bringToFront, void, 2, 2, "")
{
   object->bringToFront();
}

//-----------------------------------------------------------------------------

ConsoleMethod( GuiEditCtrl, pushToBack, void, 2, 2, "")
{
   object->pushToBack();
}

//-----------------------------------------------------------------------------

ConsoleMethod( GuiEditCtrl, deleteSelection, void, 2, 2, "() - Delete the selected controls.")
{
   object->deleteSelection();
}

//-----------------------------------------------------------------------------

ConsoleMethod( GuiEditCtrl, moveSelection, void, 4, 4, "(int dx, int dy) - Move all controls in the selection by (dx,dy) pixels.")
{
   object->moveAndSnapSelection(Point2I(dAtoi(argv[2]), dAtoi(argv[3])));
}

//-----------------------------------------------------------------------------

ConsoleMethod( GuiEditCtrl, saveSelection, void, 2, 3, "( string fileName=null ) - Save selection to file or clipboard.")
{
   const char* filename = NULL;
   if( argc > 2 )
      filename = argv[ 2 ];
      
   object->saveSelection( filename );
}

//-----------------------------------------------------------------------------

ConsoleMethod( GuiEditCtrl, loadSelection, void, 2, 3, "( string fileName=null ) - Load selection from file or clipboard.")
{
   const char* filename = NULL;
   if( argc > 2 )
      filename = argv[ 2 ];

   object->loadSelection( filename );
}

//-----------------------------------------------------------------------------

ConsoleMethod( GuiEditCtrl, selectAll, void, 2, 2, "()")
{
   object->selectAll();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiEditCtrl, getSelection, SimSet*, (),,
   "Gets the set of GUI controls currently selected in the editor." )
{
   return object->getSelectedSet();
}

//-----------------------------------------------------------------------------

ConsoleMethod( GuiEditCtrl, getNumSelected, S32, 2, 2, "() - Return the number of controls currently selected." )
{
   return object->getNumSelected();
}

//-----------------------------------------------------------------------------

ConsoleMethod( GuiEditCtrl, getSelectionGlobalBounds, const char*, 2, 2, "() - Returns global bounds of current selection as vector 'x y width height'." )
{
   RectI bounds = object->getSelectionGlobalBounds();
   String str = String::ToString( "%i %i %i %i", bounds.point.x, bounds.point.y, bounds.extent.x, bounds.extent.y );
   
   char* buffer = Con::getReturnBuffer( str.length() );
   dStrcpy( buffer, str.c_str() );
   
   return buffer;
}

//-----------------------------------------------------------------------------

ConsoleMethod( GuiEditCtrl, selectParents, void, 2, 3, "( bool addToSelection=false ) - Select parents of currently selected controls." )
{
   bool addToSelection = false;
   if( argc > 2 )
      addToSelection = dAtob( argv[ 2 ] );
      
   object->selectParents( addToSelection );
}

//-----------------------------------------------------------------------------

ConsoleMethod( GuiEditCtrl, selectChildren, void, 2, 3, "( bool addToSelection=false ) - Select children of currently selected controls." )
{
   bool addToSelection = false;
   if( argc > 2 )
      addToSelection = dAtob( argv[ 2 ] );
      
   object->selectChildren( addToSelection );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiEditCtrl, getTrash, SimGroup*, (),,
   "Gets the GUI controls(s) that are currently in the trash.")
{
   return object->getTrash();
}

//-----------------------------------------------------------------------------

ConsoleMethod(GuiEditCtrl, setSnapToGrid, void, 3, 3, "GuiEditCtrl.setSnapToGrid(gridsize)")
{
   U32 gridsize = dAtoi(argv[2]);
   object->setSnapToGrid(gridsize);
}

//-----------------------------------------------------------------------------

ConsoleMethod( GuiEditCtrl, readGuides, void, 3, 4, "( GuiControl ctrl [, int axis ] ) - Read the guides from the given control." )
{
   // Find the control.
   
   GuiControl* ctrl;
   if( !Sim::findObject( argv[ 2 ], ctrl ) )
   {
      Con::errorf( "GuiEditCtrl::readGuides - no control '%s'", argv[ 2 ] );
      return;
   }
   
   // Read the guides.
   
   if( argc > 3 )
   {
      S32 axis = dAtoi( argv[ 3 ] );
      if( axis < 0 || axis > 1 )
      {
         Con::errorf( "GuiEditCtrl::readGuides - invalid axis '%s'", argv[ 3 ] );
         return;
      }
      
      object->readGuides( ( GuiEditCtrl::guideAxis ) axis, ctrl );
   }
   else
   {
      object->readGuides( GuiEditCtrl::GuideHorizontal, ctrl );
      object->readGuides( GuiEditCtrl::GuideVertical, ctrl );
   }
}

//-----------------------------------------------------------------------------

ConsoleMethod( GuiEditCtrl, writeGuides, void, 3, 4, "( GuiControl ctrl [, int axis ] ) - Write the guides to the given control." )
{
   // Find the control.
   
   GuiControl* ctrl;
   if( !Sim::findObject( argv[ 2 ], ctrl ) )
   {
      Con::errorf( "GuiEditCtrl::writeGuides - no control '%i'", argv[ 2 ] );
      return;
   }
   
   // Write the guides.
   
   if( argc > 3 )
   {
      S32 axis = dAtoi( argv[ 3 ] );
      if( axis < 0 || axis > 1 )
      {
         Con::errorf( "GuiEditCtrl::writeGuides - invalid axis '%s'", argv[ 3 ] );
         return;
      }
      
      object->writeGuides( ( GuiEditCtrl::guideAxis ) axis, ctrl );
   }
   else
   {
      object->writeGuides( GuiEditCtrl::GuideHorizontal, ctrl );
      object->writeGuides( GuiEditCtrl::GuideVertical, ctrl );
   }
}

//-----------------------------------------------------------------------------

ConsoleMethod( GuiEditCtrl, clearGuides, void, 2, 3, "( [ int axis ] ) - Clear all currently set guide lines." )
{
   if( argc > 2 )
   {
      S32 axis = dAtoi( argv[ 2 ] );
      if( axis < 0 || axis > 1 )
      {
         Con::errorf( "GuiEditCtrl::clearGuides - invalid axis '%i'", axis );
         return;
      }
      
      object->clearGuides( ( GuiEditCtrl::guideAxis ) axis );
   }
   else
   {
      object->clearGuides( GuiEditCtrl::GuideHorizontal );
      object->clearGuides( GuiEditCtrl::GuideVertical );
   }
}

//-----------------------------------------------------------------------------

ConsoleMethod( GuiEditCtrl, fitIntoParents, void, 2, 4, "( bool width=true, bool height=true ) - Fit selected controls into their parents." )
{
   bool width = true;
   bool height = true;
   
   if( argc > 2 )
      width = dAtob( argv[ 2 ] );
   if( argc > 3 )
      height = dAtob( argv[ 3 ] );
      
   object->fitIntoParents( width, height );
}

//-----------------------------------------------------------------------------

ConsoleMethod( GuiEditCtrl, getMouseMode, const char*, 2, 2, "() - Return the current mouse mode." )
{
   switch( object->getMouseMode() )
   {
      case GuiEditCtrl::Selecting:
         return "Selecting";
      
      case GuiEditCtrl::DragSelecting:
         return "DragSelecting";

      case GuiEditCtrl::MovingSelection:
         return "MovingSelection";
      
      case GuiEditCtrl::SizingSelection:
         return "SizingSelection";
            
      case GuiEditCtrl::DragGuide:
         return "DragGuide";
         
      case GuiEditCtrl::DragClone:
         return "DragClone";
         
      default:
         return "";
   }
}

//=============================================================================
//    GuiEditorRuler.
//=============================================================================

class GuiEditorRuler : public GuiControl
{
   public:
   
      typedef GuiControl Parent;
      
   protected:
   
      String mRefCtrlName;
      String mEditCtrlName;
      
      GuiScrollCtrl* mRefCtrl;
      GuiEditCtrl* mEditCtrl;

   public:

      enum EOrientation
      {
         ORIENTATION_Horizontal,
         ORIENTATION_Vertical
      };
      
      GuiEditorRuler()
         : mRefCtrl( 0 ),
           mEditCtrl( 0 )
      {
      }
      
      EOrientation getOrientation() const
      {
         if( getWidth() > getHeight() )
            return ORIENTATION_Horizontal;
         else
            return ORIENTATION_Vertical;
      }
      
      bool onWake()
      {
         if( !Parent::onWake() )
            return false;
            
         if( !mEditCtrlName.isEmpty() && !Sim::findObject( mEditCtrlName, mEditCtrl ) )
            Con::errorf( "GuiEditorRuler::onWake() - no GuiEditCtrl '%s'", mEditCtrlName.c_str() );
         
         if( !mRefCtrlName.isEmpty() && !Sim::findObject( mRefCtrlName, mRefCtrl ) )
            Con::errorf( "GuiEditorRuler::onWake() - no GuiScrollCtrl '%s'", mRefCtrlName.c_str() );
         
         return true;
      }

      void onPreRender()
      {
         setUpdate();
      }
      
      void onMouseDown( const GuiEvent& event )
      {         
         if( !mEditCtrl )
            return;
            
         // Determine the guide axis.
         
         GuiEditCtrl::guideAxis axis;
         if( getOrientation() == ORIENTATION_Horizontal )
            axis = GuiEditCtrl::GuideHorizontal;
         else
            axis = GuiEditCtrl::GuideVertical;
            
         // Start dragging a new guide out in the editor.
         
         U32 guideIndex = mEditCtrl->addGuide( axis, 0 );
         mEditCtrl->startMouseGuideDrag( axis, guideIndex );
      }
      
      void onRender(Point2I offset, const RectI &updateRect)
      {
         GFX->getDrawUtil()->drawRectFill(updateRect, ColorF(1,1,1,1));
         
         Point2I choffset(0,0);
         if( mRefCtrl != NULL )
            choffset = mRefCtrl->getChildPos();

         if( getOrientation() == ORIENTATION_Horizontal )
         {
            // it's horizontal.
            for(U32 i = 0; i < getWidth(); i++)
            {
               S32 x = offset.x + i;
               S32 pos = i - choffset.x;
               if(!(pos % 10))
               {
                  S32 start = 6;
                  if(!(pos %20))
                     start = 4;
                  if(!(pos % 100))
                     start = 1;
                  GFX->getDrawUtil()->drawLine(x, offset.y + start, x, offset.y + 10, ColorF(0,0,0,1));
               }
            }
         }
         else
         {
            // it's vertical.
            for(U32 i = 0; i < getHeight(); i++)
            {
               S32 y = offset.y + i;
               S32 pos = i - choffset.y;
               if(!(pos % 10))
               {
                  S32 start = 6;
                  if(!(pos %20))
                     start = 4;
                  if(!(pos % 100))
                     start = 1;
                  GFX->getDrawUtil()->drawLine(offset.x + start, y, offset.x + 10, y, ColorF(0,0,0,1));
               }
            }
         }
      }
      static void initPersistFields()
      {
         addField( "refCtrl", TypeRealString, Offset( mRefCtrlName, GuiEditorRuler ) );
         addField( "editCtrl", TypeRealString, Offset( mEditCtrlName, GuiEditorRuler ) );
         
         Parent::initPersistFields();
      }
      
      DECLARE_CONOBJECT(GuiEditorRuler);
      DECLARE_CATEGORY( "Gui Editor" );
};

IMPLEMENT_CONOBJECT(GuiEditorRuler);

ConsoleDocClass( GuiEditorRuler,
   "@brief Visual representation of markers on top and left sides of GUI Editor\n\n"
   "Editor use only.\n\n"
   "@internal"
);
