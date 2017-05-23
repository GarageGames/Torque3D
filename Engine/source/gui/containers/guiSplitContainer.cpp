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
#include "gui/containers/guiSplitContainer.h"

#include "gui/core/guiCanvas.h"
#include "console/consoleTypes.h"
#include "gfx/gfxDrawUtil.h"


IMPLEMENT_CONOBJECT( GuiSplitContainer );

ConsoleDocClass( GuiSplitContainer,
   "@brief A container that splits its area between two child controls.\n\n"
   
   "A GuiSplitContainer can be used to dynamically subdivide an area between two child controls.  "
   "A splitter bar is placed between the two controls and allows to dynamically adjust the sizing "
   "ratio between the two sides.  Splitting can be either horizontal (subdividing top and bottom) "
   "or vertical (subdividing left and right) depending on #orientation.\n\n"
   
   "By using #fixedPanel, one of the panels can be chosen to remain at a fixed size (#fixedSize)."
   
   "@tsexample\n"
   "// Create a vertical splitter with a fixed-size left panel.\n"
   "%splitter = new GuiSplitContainer()\n"
   "{\n"
   "   orientation = \"Vertical\";\n"
   "   fixedPanel = \"FirstPanel\";\n"
   "   fixedSize = 100;\n"
   "\n"
   "   new GuiScrollCtrl()\n"
   "   {\n"
   "      new GuiMLTextCtrl()\n"
   "      {\n"
   "         text = %longText;\n"
   "      };\n"
   "   };\n"
   "\n"
   "   new GuiScrollCtrl()\n"
   "   {\n"
   "      new GuiMLTextCtrl()\n"
   "      {\n"
   "         text = %moreLongText;\n"
   "      };\n"
   "   };\n"
   "};\n"
   "@endtsexample\n\n"
   
   "@note The children placed inside GuiSplitContainers must be GuiContainers.\n\n"
   
   "@ingroup GuiContainers"
);


ImplementEnumType( GuiSplitOrientation,
   "Axis along which to divide the container's space.\n\n"
   "@ingroup GuiContainers" )
   { GuiSplitContainer::Vertical, "Vertical", "Divide vertically placing one child left and one child right." },
   { GuiSplitContainer::Horizontal, "Horizontal", "Divide horizontally placing one child on top and one child below." }   
EndImplementEnumType;

ImplementEnumType( GuiSplitFixedPanel,
   "Which side of the splitter to keep at a fixed size (if any).\n\n"
   "@ingroup GuiContainers" )
   { GuiSplitContainer::None, "None", "Allow both childs to resize (default)." },
   { GuiSplitContainer::FirstPanel, "FirstPanel", "Keep " },
   { GuiSplitContainer::SecondPanel, "SecondPanel" }
EndImplementEnumType;


//-----------------------------------------------------------------------------

GuiSplitContainer::GuiSplitContainer()
 : mFixedPanel( None ),
   mFixedPanelSize( 100 ),
   mOrientation( Vertical ),
   mSplitterSize( 2 ),
   mSplitPoint( 0, 0 ),
   mSplitRect( 0, 0, mSplitterSize, mSplitterSize ),
   mDragging( false )
{
   setMinExtent( Point2I(64,64) );
   setExtent(200,200);
   setDocking( Docking::dockNone );

   mSplitPoint.set( 300, 100 );

   // We only support client docked items in a split container
   mValidDockingMask = Docking::dockClient;
}

//-----------------------------------------------------------------------------

void GuiSplitContainer::initPersistFields()
{
   addGroup( "Splitter", "Options to configure split panels contained by this control" );
   
      addField( "orientation",   TYPEID< Orientation >(),   Offset( mOrientation, GuiSplitContainer),
         "Whether to split between top and bottom (horizontal) or between left and right (vertical)." );
      addField( "splitterSize",  TypeS32,    Offset( mSplitterSize, GuiSplitContainer),
         "Width of the splitter bar between the two sides.  Default is 2." );
      addField( "splitPoint",    TypePoint2I, Offset( mSplitPoint, GuiSplitContainer),
         "Point on control through which the splitter goes.\n\n"
         "Changed relatively if size of control changes." );
      addField( "fixedPanel",    TYPEID< FixedPanel >(),   Offset( mFixedPanel, GuiSplitContainer),
         "Which (if any) side of the splitter to keep at a fixed size." );
      addField( "fixedSize",     TypeS32,    Offset( mFixedPanelSize, GuiSplitContainer),
         "Width of the fixed panel specified by #fixedPanel (if any)." );
      
   endGroup( "Splitter" );

   Parent::initPersistFields();
}

//-----------------------------------------------------------------------------

bool GuiSplitContainer::onAdd()
{
   if ( !Parent::onAdd() )
      return false;

   return true;
}

//-----------------------------------------------------------------------------

bool GuiSplitContainer::onWake()
{
   if ( !Parent::onWake() )
      return false;

   // Create Panel 1
   if ( empty() )
   {
      GuiPanel *newPanel = new GuiPanel();
      AssertFatal( newPanel, "GuiSplitContainer::onAdd - Cannot create subordinate panel #1!" );
      newPanel->registerObject();
      newPanel->setInternalName( "Panel1" );
      newPanel->setDocking( Docking::dockClient );
      addObject( (SimObject*)newPanel );
   }
   else
   {
      GuiContainer *containerCtrl = dynamic_cast<GuiContainer*>( at(0) );
      if ( containerCtrl )
      {
         containerCtrl->setInternalName( "Panel1" );
         containerCtrl->setDocking( Docking::dockClient );
      }
   }

   if ( size() == 1 )
   {
      // Create Panel 2
      GuiPanel *newPanel = new GuiPanel();
      AssertFatal( newPanel, "GuiSplitContainer::onAdd - Cannot create subordinate panel #2!" );
      newPanel->registerObject();
      newPanel->setInternalName( "Panel2" );
      newPanel->setDocking( Docking::dockClient );
      addObject( (SimObject*)newPanel );
   }
   else
   {
      GuiContainer *containerCtrl = dynamic_cast<GuiContainer*>( at(1) );
      if ( containerCtrl )
      {
         containerCtrl->setInternalName( "Panel2" );
         containerCtrl->setDocking( Docking::dockClient );
      }
   }

   // Has FixedWidth been specified?
   if ( mFixedPanelSize == 0 )
   {
      // Nope, so try to guess as best we can
      GuiContainer *firstPanel = dynamic_cast<GuiContainer*>( at(0) );
      GuiContainer *secondPanel = dynamic_cast<GuiContainer*>( at(1) );
      if ( mFixedPanel == FirstPanel )
      {
         if ( mOrientation == Horizontal )
            mFixedPanelSize = firstPanel->getExtent().y;
         else
            mFixedPanelSize = firstPanel->getExtent().x;

         mSplitPoint = Point2I( mFixedPanelSize, mFixedPanelSize );
      }
      else if ( mFixedPanel == SecondPanel )
      {
         if ( mOrientation == Horizontal )
            mFixedPanelSize = getExtent().y - secondPanel->getExtent().y;
         else
            mFixedPanelSize = getExtent().x - secondPanel->getExtent().x;

         mSplitPoint = getExtent() - Point2I( mFixedPanelSize, mFixedPanelSize );
      }

   }

   setUpdateLayout();

   return true;
}

//-----------------------------------------------------------------------------

void GuiSplitContainer::onRender( Point2I offset, const RectI &updateRect )
{
   Parent::onRender( offset, updateRect );

   // Only render if we're dragging the splitter
   if ( mDragging && mSplitRect.isValidRect() )
   {
      // Splitter Rectangle (will adjust positioning only)
      RectI splitterRect = mSplitRect;

      // Currently being dragged to Rect 
      Point2I splitterPoint = localToGlobalCoord( mSplitRect.point );
      splitterRect.point = localToGlobalCoord( mSplitPoint );

      RectI clientRect = getClientRect();
      clientRect.point = localToGlobalCoord( clientRect.point );

      if ( mOrientation == Horizontal ) 
      {
         splitterRect.point.y -= mSplitterSize;
         splitterRect.point.x = splitterPoint.x;
      }
      else
      {
         splitterRect.point.x -= mSplitterSize;
         splitterRect.point.y = splitterPoint.y;
      }

      RectI oldClip = GFX->getClipRect();
      GFX->setClipRect( clientRect );
      GFX->getDrawUtil()->drawRectFill( splitterRect, mProfile->mFillColorHL );
      GFX->setClipRect( oldClip );

   }
   else
   {
      RectI splitterRect = mSplitRect;
      splitterRect.point += offset;
      GFX->getDrawUtil()->drawRectFill( splitterRect, mProfile->mFillColor );
   }
}

//-----------------------------------------------------------------------------

Point2I GuiSplitContainer::getMinExtent() const
{
   GuiContainer *panelOne = dynamic_cast<GuiContainer*>( at(0) );
   GuiContainer *panelTwo = dynamic_cast<GuiContainer*>( at(1) );

   if ( !panelOne || !panelTwo )
      return Parent::getMinExtent();

   Point2I minExtent = Point2I(0,0);
   Point2I panelOneMinExtent = panelOne->getMinExtent();
   Point2I panelTwoMinExtent = panelTwo->getMinExtent();

   if ( mOrientation == Horizontal )
   {
      minExtent.y = 2 * mSplitterSize + panelOneMinExtent.y + panelTwoMinExtent.y;
      minExtent.x = getMax( panelOneMinExtent.x, panelTwoMinExtent.x );
   }
   else
   {
      minExtent.x = 2 * mSplitterSize + panelOneMinExtent.x + panelTwoMinExtent.x;
      minExtent.y = getMax( panelOneMinExtent.y, panelTwoMinExtent.y );
   }

   return minExtent;
}

//-----------------------------------------------------------------------------

void GuiSplitContainer::parentResized( const RectI &oldParentRect, const RectI &newParentRect )
{
   Parent::parentResized( oldParentRect, newParentRect );
   return;

   // TODO: Is this right James?  This isn't needed anymore?
   /*

   // GuiSplitContainer overrides parentResized to make sure that the proper fixed frame's width/height
   // is not compromised in the call
   
   if ( size() < 2 )
      return;

   GuiContainer *panelOne = dynamic_cast<GuiContainer*>( at(0) );
   GuiContainer *panelTwo = dynamic_cast<GuiContainer*>( at(1) );

   AssertFatal( panelOne && panelTwo, "GuiSplitContainer::parentResized - Missing/Invalid Subordinate Controls! Split contained controls must derive from GuiContainer!" );

   Point2I newDragPos;
   if ( mFixedPanel == FirstPanel )
   {
      newDragPos = panelOne->getExtent();
      newDragPos += Point2I( mSplitterSize, mSplitterSize );
   }
   else if ( mFixedPanel == SecondPanel )
   {
      newDragPos = getExtent() - panelTwo->getExtent();
      newDragPos -= Point2I( mSplitterSize, mSplitterSize );
   }
   else // None
      newDragPos.set( 1, 1);   
  
   RectI clientRect = getClientRect();
   solvePanelConstraints( newDragPos, panelOne, panelTwo, clientRect );

   setUpdateLayout(); 
   */
}

//-----------------------------------------------------------------------------

bool GuiSplitContainer::resize( const Point2I &newPosition, const Point2I &newExtent )
{
   // Save previous extent.
   Point2I oldExtent = getExtent();

   // Resize ourselves.
   if ( !Parent::resize( newPosition, newExtent ) || size() < 2 )
      return false;

   GuiContainer *panelOne = dynamic_cast<GuiContainer*>( at(0) );
   GuiContainer *panelTwo = dynamic_cast<GuiContainer*>( at(1) );
	
   // 
   AssertFatal( panelOne && panelTwo, "GuiSplitContainer::resize - Missing/Invalid Subordinate Controls! Split contained controls must derive from GuiContainer!" );

   // We only need to update the split point if our second panel is fixed.  
   // If the first is fixed, then we can leave the split point alone because
   // the remainder of the size will be added to or taken from the second panel
   Point2I newDragPos;
   if ( mFixedPanel == SecondPanel )
   {      
      S32 deltaX = newExtent.x - oldExtent.x;
      S32 deltaY = newExtent.y - oldExtent.y;

      if( mOrientation == Horizontal )
         mSplitPoint.y += deltaY;
      else
         mSplitPoint.x += deltaX;
   }

   // If we got here, parent returned true
   return true;
}

//-----------------------------------------------------------------------------

bool GuiSplitContainer::layoutControls( RectI &clientRect )
{
   if ( size() < 2 )
      return false;

   GuiContainer *panelOne = dynamic_cast<GuiContainer*>( at(0) );
   GuiContainer *panelTwo = dynamic_cast<GuiContainer*>( at(1) );

   // 
   AssertFatal( panelOne && panelTwo, "GuiSplitContainer::layoutControl - Missing/Invalid Subordinate Controls! Split contained controls must derive from GuiContainer!" );

   RectI panelOneRect = RectI( clientRect.point, Point2I( 0, 0 ) );
   RectI panelTwoRect;
   RectI splitRect;

   solvePanelConstraints( getSplitPoint(), panelOne, panelTwo, clientRect );

   switch( mOrientation )
   {
   case Horizontal:
      panelOneRect.extent = Point2I( clientRect.extent.x, getSplitPoint().y );
      panelTwoRect = panelOneRect;
      panelTwoRect.intersect( clientRect );
      panelTwoRect.point.y = panelOneRect.extent.y;
      panelTwoRect.extent.y = clientRect.extent.y - panelOneRect.extent.y;

      // Generate new Splitter Rectangle
      splitRect = panelTwoRect;
      splitRect.extent.y = 0;
      splitRect.inset( 0, -mSplitterSize );

      panelOneRect.extent.y -= mSplitterSize;
      panelTwoRect.point.y += mSplitterSize;
      panelTwoRect.extent.y -= mSplitterSize;

      break;

   case Vertical:
      panelOneRect.extent = Point2I( getSplitPoint().x, clientRect.extent.y );
      panelTwoRect = panelOneRect;
      panelTwoRect.intersect( clientRect );
      panelTwoRect.point.x = panelOneRect.extent.x;
      panelTwoRect.extent.x = clientRect.extent.x - panelOneRect.extent.x;

      // Generate new Splitter Rectangle
      splitRect = panelTwoRect;
      splitRect.extent.x = 0;
      splitRect.inset( -mSplitterSize, 0 );

      panelOneRect.extent.x -= mSplitterSize;
      panelTwoRect.point.x += mSplitterSize;
      panelTwoRect.extent.x -= mSplitterSize;

      break;
   }

   // Update Split Rect
   mSplitRect = splitRect;

   // Dock Appropriately
   if( !( mFixedPanel == FirstPanel && !panelOne->isVisible() ) )
      dockControl( panelOne, panelOne->getDocking(), panelOneRect );
   if( !( mFixedPanel == FirstPanel && !panelTwo->isVisible() ) )
      dockControl( panelTwo, panelOne->getDocking(), panelTwoRect );   

   // 
   return false;
}

//-----------------------------------------------------------------------------

void GuiSplitContainer::solvePanelConstraints(Point2I newDragPos, GuiContainer * firstPanel, GuiContainer * secondPanel, const RectI& clientRect)
{
   if( !firstPanel || !secondPanel )
      return;

   if ( mOrientation == Horizontal )
   {
      // Constrain based on Y axis (Horizontal Splitter)

      // This accounts for the splitter width 
      S32 splitterSize = (S32)(mSplitRect.extent.y * 0.5);

      // Collapsed fixed panel
      if ( mFixedPanel == SecondPanel && !secondPanel->isVisible() )
      {
         newDragPos = Point2I(mSplitPoint.x, getExtent().y - splitterSize );
      }
      else if( mFixedPanel == SecondPanel && !firstPanel->isVisible() )
      {
         newDragPos = Point2I(mSplitPoint.x, splitterSize );
      }
      else // Normal constraints
      {
         //newDragPos.y -= splitterSize;
         S32 newPosition = mClamp( newDragPos.y, 
                                   firstPanel->getMinExtent().y + splitterSize,
                                   getExtent().y - secondPanel->getMinExtent().y - splitterSize );
         newDragPos = Point2I( mSplitPoint.x, newPosition );
      }
   }
   else
   {
      // Constrain based on X axis (Vertical Splitter)

      // This accounts for the splitter width 
      S32 splitterSize = (S32)(mSplitRect.extent.x * 0.5);

      // Collapsed fixed panel
      if ( mFixedPanel == SecondPanel && !secondPanel->isVisible() )
      {
         newDragPos = Point2I(getExtent().x - splitterSize, mSplitPoint.y  );
      }
      else if ( mFixedPanel == FirstPanel && !firstPanel->isVisible() )
      {
         newDragPos = Point2I( splitterSize, mSplitPoint.x );
      }
      else // Normal constraints
      {
         S32 newPosition = mClamp( newDragPos.x, firstPanel->getMinExtent().x + splitterSize,
            getExtent().x - secondPanel->getMinExtent().x - splitterSize );
         newDragPos = Point2I( newPosition, mSplitPoint.y );
      }
   }

   // Just in case, clamp to bounds of controls
   newDragPos.x = mClamp( newDragPos.x, clientRect.point.x, clientRect.point.x + clientRect.extent.x );
   newDragPos.y = mClamp( newDragPos.y, clientRect.point.y, clientRect.point.y + clientRect.extent.y );

   mSplitPoint = newDragPos;
}

//-----------------------------------------------------------------------------

void GuiSplitContainer::getCursor( GuiCursor *&cursor, bool &showCursor, const GuiEvent &lastGuiEvent )
{
   GuiCanvas *rootCtrl = getRoot();
   if ( !rootCtrl )
      return;

   S32 desiredCursor = 0;
   RectI splitRect = getSplitRect();

   // Figure out which cursor we want if we need one
   if ( mOrientation == Horizontal )
      desiredCursor = PlatformCursorController::curResizeHorz;
   else if ( mOrientation == Vertical )
      desiredCursor = PlatformCursorController::curResizeVert;

   PlatformWindow *platformWindow = static_cast<GuiCanvas*>(getRoot())->getPlatformWindow();
   AssertFatal( platformWindow != NULL,"GuiControl without owning platform window!  This should not be possible." );

   PlatformCursorController *cusrorController = platformWindow->getCursorController();
   AssertFatal( cusrorController != NULL,"PlatformWindow without an owned CursorController!" );

   // Check to see if we need one or just the default...

   Point2I localPoint = Point2I( globalToLocalCoord( lastGuiEvent.mousePoint ) );
   if ( splitRect.pointInRect( localPoint ) || mDragging  )
   {
      // Do we need to change it or is it already set?
      if ( rootCtrl->mCursorChanged != desiredCursor )
      {
         // We've already changed the cursor, so set it back
         if ( rootCtrl->mCursorChanged != -1 )
            cusrorController->popCursor();

         // Now change the cursor shape
         cusrorController->pushCursor( desiredCursor );
         rootCtrl->mCursorChanged = desiredCursor;
      }
   }
   else if ( rootCtrl->mCursorChanged != -1 )
   {
      // Just the default
      cusrorController->popCursor();
      rootCtrl->mCursorChanged = -1;
   }
}

//-----------------------------------------------------------------------------

void GuiSplitContainer::onMouseDown( const GuiEvent &event )
{
   GuiContainer *firstPanel = dynamic_cast<GuiContainer*>(at(0));
   GuiContainer *secondPanel = dynamic_cast<GuiContainer*>(at(1));

   // This function will constrain the panels to their minExtents and update the mSplitPoint
   if ( firstPanel && secondPanel )
   {
      mouseLock();
      mDragging = true;

      RectI clientRect = getClientRect();
      Point2I newDragPos = globalToLocalCoord( event.mousePoint );

      solvePanelConstraints(newDragPos, firstPanel, secondPanel, clientRect);
   }
}

//-----------------------------------------------------------------------------

void GuiSplitContainer::onMouseUp( const GuiEvent &event )
{
   // If we've been dragging, we need to update the fixed panel extent.  
   // NOTE : This should ONLY ever happen in this function.  the Fixed panel
   // is to REMAIN FIXED unless the user changes it.
   if ( mDragging )
   {
      Point2I newSplitPoint = getSplitPoint();

      // Update Fixed Panel Extent
      if ( mFixedPanel == FirstPanel )
         mFixedPanelSize = ( mOrientation == Horizontal ) ? newSplitPoint.y : newSplitPoint.x;
      else
         mFixedPanelSize = ( mOrientation == Horizontal ) ? getExtent().y - newSplitPoint.y : getExtent().x - newSplitPoint.x;

      setUpdateLayout();
   }

   mDragging = false;
   mouseUnlock();
}

//-----------------------------------------------------------------------------

void GuiSplitContainer::onMouseDragged( const GuiEvent &event )
{
   GuiContainer *firstPanel = dynamic_cast<GuiContainer*>(at(0));
   GuiContainer *secondPanel = dynamic_cast<GuiContainer*>(at(1));

   // This function will constrain the panels to their minExtents and update the mSplitPoint
   if ( mDragging && firstPanel && secondPanel )
   {
      RectI clientRect = getClientRect();
      Point2I newDragPos = globalToLocalCoord( event.mousePoint );

      solvePanelConstraints(newDragPos, firstPanel, secondPanel, clientRect);
   }
}
