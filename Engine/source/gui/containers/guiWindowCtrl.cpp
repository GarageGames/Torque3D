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
#include "gui/containers/guiWindowCtrl.h"

#include "console/consoleTypes.h"
#include "console/console.h"
#include "console/engineAPI.h"
#include "gui/core/guiCanvas.h"
#include "gui/core/guiDefaultControlRender.h"
#include "gfx/gfxDevice.h"
#include "gfx/gfxDrawUtil.h"
#include "gui/containers/guiRolloutCtrl.h"


IMPLEMENT_CONOBJECT( GuiWindowCtrl );

ConsoleDocClass( GuiWindowCtrl,
   "@brief A window with a title bar and an optional set of buttons.\n\n"

   "The GuiWindowCtrl class implements windows that can be freely placed within the render window.  Additionally, "
   "the windows can be resized and maximized/minimized.\n\n"
   
   "@tsexample\n"
   "new GuiWindowCtrl( MyWindow )\n"
   "{\n"
   "   text = \"My Window\"; // The text that is displayed on the title bar.\n"
   "   resizeWidth = true; // Allow horizontal resizing by user via mouse.\n"
   "   resizeHeight = true; // Allow vertical resizing by user via mouse.\n"
   "   canClose = true; // Display a close button in the title bar.\n"
   "   canMinimize = true; // Display a minimize button in the title bar.\n"
   "   canMaximize = true; // Display a maximize button in the title bar.\n"
   "};\n"
   "@endtsexample\n\n"
   
   "@ingroup GuiContainers"
);

IMPLEMENT_CALLBACK( GuiWindowCtrl, onClose, void, (), (),
   "Called when the close button has been pressed." );
IMPLEMENT_CALLBACK( GuiWindowCtrl, onMinimize, void, (), (),
   "Called when the window has been minimized." );
IMPLEMENT_CALLBACK( GuiWindowCtrl, onMaximize, void, (), (),
   "Called when the window has been maximized." );
IMPLEMENT_CALLBACK( GuiWindowCtrl, onCollapse, void, (), (),
   "Called when the window is collapsed by clicking its title bar." );
IMPLEMENT_CALLBACK( GuiWindowCtrl, onRestore, void, (), (),
   "Called when the window is restored from minimized, maximized, or collapsed state." );

//-----------------------------------------------------------------------------

GuiWindowCtrl::GuiWindowCtrl()
   :  mResizeEdge(edgeNone),
      mResizeWidth(true),
      mResizeHeight(true),
      mResizeMargin(2.f),
      mCanMove(true),
      mCanClose(true),
      mCanMinimize(true),
      mCanMaximize(true),
      mCanDock(false),
      mCanCollapse(false),
      mEdgeSnap(true),
      mCollapseGroup(-1),
      mCollapseGroupNum(-1),
      mIsCollapsed(false),
      mIsMouseResizing(false)
{
   // mTitleHeight will change in instanciation most likely...
   mTitleHeight = 24;

   mIsContainer = true;

   mCloseCommand = StringTable->EmptyString();

   mMinimized = false;
   mMaximized = false;
   mMouseMovingWin = false;
   mMouseResizeWidth = false;
   mMouseResizeHeight = false;
   setExtent(100, 200);
   mMinimizeIndex = -1;
   mTabIndex = -1;
   mBitmapBounds = NULL;

   RectI closeRect(80, 2, 16, 16);
   mCloseButton = closeRect;
   closeRect.point.x -= 18;
   mMaximizeButton = closeRect;
   closeRect.point.x -= 18;
   mMinimizeButton = closeRect;

   // Other defaults
   mActive = true;
   mCloseButtonPressed = false;
   mMaximizeButtonPressed = false;
   mMinimizeButtonPressed = false;

   mText = "New Window";
}

//-----------------------------------------------------------------------------

void GuiWindowCtrl::initPersistFields()
{
   addGroup( "Window" );
   
      addField( "text",              TypeRealString,   Offset( mText, GuiWindowCtrl ),
         "Text label to display in titlebar." );
      addField( "resizeWidth",       TypeBool,         Offset( mResizeWidth, GuiWindowCtrl ),
         "Whether the window can be resized horizontally." );
      addField( "resizeHeight",      TypeBool,         Offset( mResizeHeight, GuiWindowCtrl ),
         "Whether the window can be resized vertically." );
      addField( "canMove",           TypeBool,         Offset( mCanMove, GuiWindowCtrl ),
         "Whether the window can be moved by dragging its titlebar." );
      addField( "canClose",          TypeBool,         Offset( mCanClose, GuiWindowCtrl ),
         "Whether the window has a close button." );
      addField( "canMinimize",       TypeBool,         Offset( mCanMinimize, GuiWindowCtrl ),
         "Whether the window has a minimize button." );
      addField( "canMaximize",       TypeBool,         Offset( mCanMaximize, GuiWindowCtrl ),
         "Whether the window has a maximize button." );
      addField( "canCollapse",       TypeBool,         Offset( mCanCollapse, GuiWindowCtrl ),
         "Whether the window can be collapsed by clicking its title bar." );
      addField( "closeCommand",      TypeString,       Offset( mCloseCommand, GuiWindowCtrl ),
         "Script code to execute when the window is closed." );
      addField( "edgeSnap",          TypeBool,         Offset( mEdgeSnap,GuiWindowCtrl ),
         "If true, the window will snap to the edges of other windows when moved close to them." );
         
   endGroup( "Window" );

   Parent::initPersistFields();
}

//=============================================================================
//    Collapsing.
//=============================================================================
// MARK: ---- Collapsing ----

//-----------------------------------------------------------------------------

void GuiWindowCtrl::moveFromCollapseGroup()
{
   GuiControl *parent = getParent();
   if( !parent )
      return;

   S32 groupVec = mCollapseGroup;
   S32 vecPos = mCollapseGroupNum;
   S32 groupVecCount = parent->mCollapseGroupVec[groupVec].size() - 1;

   CollapseGroupNumVec collapseGroupNumVec;

   if( groupVecCount > vecPos )
   {
      if (vecPos == 1)
      {
         CollapseGroupNumVec::iterator iter = parent->mCollapseGroupVec[groupVec].begin();
         for(; iter != parent->mCollapseGroupVec[groupVec].end(); iter++ )
         {
            if((*iter)->mCollapseGroupNum >= vecPos)
               collapseGroupNumVec.push_back((*iter));
         }

         parent->mCollapseGroupVec[groupVec].first()->mCollapseGroup = -1;
         parent->mCollapseGroupVec[groupVec].first()->mCollapseGroupNum = -1;
         parent->mCollapseGroupVec[groupVec].erase(U32(0));
         parent->mCollapseGroupVec[groupVec].setSize(groupVecCount - 1);
         parent->mCollapseGroupVec.erase(groupVec);
         if(groupVec > 0)
            parent->mCollapseGroupVec.setSize(groupVec);

         parent->mCollapseGroupVec.push_back( collapseGroupNumVec );
      }
      else
      {
         // Iterate through the group i was once in, gather myself and the controls below me and store them in an array
         CollapseGroupNumVec::iterator iter = parent->mCollapseGroupVec[groupVec].begin();
         for(; iter != parent->mCollapseGroupVec[groupVec].end(); iter++ )
         {
            if((*iter)->mCollapseGroupNum >= vecPos)
               collapseGroupNumVec.push_back((*iter));
         }
         
         // Iterate through the newly created array; delete my references in the old group, create a new group and organize me accord.
         S32 assignWindowNumber = 0;
         CollapseGroupNumVec::iterator iter2 = collapseGroupNumVec.begin();
         for(; iter2 != collapseGroupNumVec.end(); iter2++ )
         {
            parent->mCollapseGroupVec[groupVec].pop_back();
            parent->mCollapseGroupVec[groupVec].setSize(groupVecCount);
            (*iter2)->mCollapseGroup = (parent->mCollapseGroupVec.size());
            (*iter2)->mCollapseGroupNum = assignWindowNumber;
            
            assignWindowNumber++;
            groupVecCount--;
         }

         parent->mCollapseGroupVec.push_back( collapseGroupNumVec );
      }
   }
   else
   {
      parent->mCollapseGroupVec[groupVec].erase(mCollapseGroupNum);
      parent->mCollapseGroupVec[groupVec].setSize(groupVecCount);
      mCollapseGroup = -1;
      mCollapseGroupNum = -1;

      if(groupVecCount <= 1)
      {
         parent->mCollapseGroupVec[groupVec].first()->mCollapseGroup = -1;
         parent->mCollapseGroupVec[groupVec].first()->mCollapseGroupNum = -1;
         parent->mCollapseGroupVec[groupVec].erase(U32(0));
         parent->mCollapseGroupVec[groupVec].setSize(groupVecCount - 1);
         parent->mCollapseGroupVec.erase(groupVec);	
      }
   }
   
   // Re id collapse groups
   refreshCollapseGroups();
}

//-----------------------------------------------------------------------------

void GuiWindowCtrl::moveToCollapseGroup(GuiWindowCtrl* hitWindow, bool orientation )
{
   // Orientation 0 - window in question is being connected to top of another window
   // Orientation 1 - window in question is being connected to bottom of another window

   GuiControl *parent = getParent();
   if( !parent )
      return;
   
   S32 groupVec = mCollapseGroup;
   S32 attatchedGroupVec = hitWindow->mCollapseGroup;
   S32 vecPos = mCollapseGroupNum;
   
   if(mCollapseGroup == attatchedGroupVec && vecPos != -1)
      return;

   CollapseGroupNumVec collapseGroupNumVec;

   // Window colliding with is not in a collapse group
   if(hitWindow->mCollapseGroup < 0) 
   {
      // We(the collider) are in a group of windows
      if(mCollapseGroup >= 0) 
      {
         S32 groupVecCount = parent->mCollapseGroupVec[groupVec].size() - 1;
            
         // Copy pointer window data in my array, and store in a temp array
         CollapseGroupNumVec::iterator iter = parent->mCollapseGroupVec[groupVec].begin();
         for(; iter != parent->mCollapseGroupVec[groupVec].end(); iter++ )
         {
            if((*iter)->mCollapseGroupNum >= vecPos)
            {
               collapseGroupNumVec.push_back((*iter));
               groupVecCount--;
            }
         }

         // Kill my old array group and erase its footprints
         if( vecPos <= 1 || groupVecCount == 0 )
         {
            // Isn't covered in the renumbering of groups, so we it here
            parent->mCollapseGroupVec[groupVec].first()->mCollapseGroup = -1;
            parent->mCollapseGroupVec[groupVec].first()->mCollapseGroupNum = -1;

            parent->mCollapseGroupVec[groupVec].clear();
            parent->mCollapseGroupVec[groupVec].setSize(U32(0));
            parent->mCollapseGroupVec.erase(groupVec);

            if(groupVec > 0)
               parent->mCollapseGroupVec.setSize(groupVec);
         }

         // Push the collided window
         if(orientation == 0)
            collapseGroupNumVec.push_back(hitWindow);
         else
            collapseGroupNumVec.push_front(hitWindow);

         // Push the temp array in the guiControl array holder
         parent->mCollapseGroupVec.push_back( collapseGroupNumVec );
      }
      else
      {
         if(orientation == 0)
         {
            collapseGroupNumVec.push_front(hitWindow);
            collapseGroupNumVec.push_front(this);
         }
         else
         {
            collapseGroupNumVec.push_front(this);
            collapseGroupNumVec.push_front(hitWindow);
         }

         parent->mCollapseGroupVec.push_back( collapseGroupNumVec );
      }
   }
   else // Window colliding with *IS* in a collapse group
   {
   
      if(mCollapseGroup >= 0)
      {
         S32 groupVecCount = parent->mCollapseGroupVec[groupVec].size() - 1;
            
         CollapseGroupNumVec::iterator iter = parent->mCollapseGroupVec[groupVec].begin();
         for(; iter != parent->mCollapseGroupVec[groupVec].end(); iter++ )
         {
            if((*iter)->mCollapseGroupNum >= vecPos)
            {
               // Push back the pointer window controls to the collided array
               parent->mCollapseGroupVec[attatchedGroupVec].push_back((*iter));
               groupVecCount--;
            }
         }

         if( vecPos <= 1 || groupVecCount == 0 )
         {
            // Isn't covered in the renumbering of groups, so we it here
            parent->mCollapseGroupVec[groupVec].first()->mCollapseGroup = -1;
            parent->mCollapseGroupVec[groupVec].first()->mCollapseGroupNum = -1;

            parent->mCollapseGroupVec[groupVec].clear();
            parent->mCollapseGroupVec[groupVec].setSize(U32(0));
            parent->mCollapseGroupVec.erase(groupVec);

            if(groupVec > 0)
               parent->mCollapseGroupVec.setSize(groupVec);
         }
         
         // Since we killed my old array group, run in case the guiControl array moved me down a notch
         if(attatchedGroupVec > groupVec )
            attatchedGroupVec--;

      }
      else
      {
         S32 groupVec = hitWindow->mCollapseGroup;
         
         if(orientation == 0)
            parent->mCollapseGroupVec[groupVec].push_front(this);
         else
            parent->mCollapseGroupVec[groupVec].push_back(this);
      }
   }
   
   // Re id collapse groups
   refreshCollapseGroups();

   // Select the current window and its collapse group
   selectWindow();
}

//-----------------------------------------------------------------------------

void GuiWindowCtrl::refreshCollapseGroups()
{
   GuiControl *parent = getParent();
   if( !parent )
      return;
   
   CollapseGroupNumVec	collapseGroupNumVec;

   // iterate through the collided array, renumbering the windows pointers
   S32 assignGroupNum = 0;
   CollapseGroupVec::iterator iter = parent->mCollapseGroupVec.begin();
   for(; iter != parent->mCollapseGroupVec.end(); iter++ )
   {
      S32 assignWindowNumber = 0;
      CollapseGroupNumVec::iterator iter2 = parent->mCollapseGroupVec[assignGroupNum].begin();
      for(; iter2 != parent->mCollapseGroupVec[assignGroupNum].end(); iter2++ )
      {
         (*iter2)->mCollapseGroup = assignGroupNum;
         (*iter2)->mCollapseGroupNum = assignWindowNumber;
         assignWindowNumber++;
      }
            
      assignGroupNum++;
   }
}

//-----------------------------------------------------------------------------

void GuiWindowCtrl::moveWithCollapseGroup(Point2I windowPosition)
{
   GuiControl *parent = getParent();
   if( !parent )
      return;

   Point2I newChildPosition(0, 0);
   S32 addedPosition = getExtent().y;

   // Iterate through windows below this. Move them according to my position.
   CollapseGroupNumVec collapseGroupNumVec;
   CollapseGroupNumVec::iterator iter = parent->mCollapseGroupVec[mCollapseGroup].begin();
   for(; iter != parent->mCollapseGroupVec[mCollapseGroup].end(); iter++ )
   {
      if((*iter)->mCollapseGroupNum > mCollapseGroupNum)
      {
         newChildPosition.x = windowPosition.x;
         newChildPosition.y = windowPosition.y + addedPosition;

         (*iter)->resize(newChildPosition, (*iter)->getExtent());
         addedPosition += (*iter)->getExtent().y;
      }
   }
}

//-----------------------------------------------------------------------------

void GuiWindowCtrl::setCollapseGroup(bool state)
{
   GuiControl *parent = getParent();
   if( !parent )
      return;

   if( mIsCollapsed != state )
   {
      mIsCollapsed = state;
      handleCollapseGroup();
   }
}

//-----------------------------------------------------------------------------

void GuiWindowCtrl::toggleCollapseGroup()
{
   GuiControl *parent = getParent();
   if( !parent )
      return;

   mIsCollapsed = !mIsCollapsed;
   handleCollapseGroup();
}

//-----------------------------------------------------------------------------

void GuiWindowCtrl::handleCollapseGroup()
{
   GuiControl *parent = getParent();
   if( !parent )
      return;

   CollapseGroupNumVec	collapseGroupNumVec;

   if( mIsCollapsed ) // minimize window up to its header bar
   {
      //save settings 
      mPreCollapsedYExtent = getExtent().y;
      mPreCollapsedYMinExtent = getMinExtent().y;

      //create settings for collapsed window to abide by
      mResizeHeight = false;
      setMinExtent( Point2I( getMinExtent().x, 24 ) );

      iterator i;
      for(i = begin(); i != end(); i++)
      {
         GuiControl *ctrl = static_cast<GuiControl *>(*i);
         ctrl->setVisible(false);
         ctrl->mCanResize = false;
      }

      resize( getPosition(), Point2I( getExtent().x, 24 ) );

      if(mCollapseGroup >= 0)
      {
         S32 moveChildYBy = (mPreCollapsedYExtent - 24);

         CollapseGroupNumVec::iterator iter = parent->mCollapseGroupVec[mCollapseGroup].begin();
         for(; iter != parent->mCollapseGroupVec[mCollapseGroup].end(); iter++ )
         {
            if((*iter)->mCollapseGroupNum > mCollapseGroupNum)
            {
               Point2I newChildPosition =  (*iter)->getPosition();
               newChildPosition.y -= moveChildYBy;
               (*iter)->resize(newChildPosition, (*iter)->getExtent());
            }
         }
      }
      
      onCollapse_callback();
   }
   else // maximize the window to its previous position
   {
      //create and load settings
      mResizeHeight = true;
      setMinExtent( Point2I( getMinExtent().x, mPreCollapsedYMinExtent ) );
      
      resize( getPosition(), Point2I( getExtent().x, mPreCollapsedYExtent ) );
      
      iterator i;
      for(i = begin(); i != end(); i++)
      {
         GuiControl *ctrl = static_cast<GuiControl *>(*i);
         ctrl->setVisible(true);
         ctrl->mCanResize = true;
      }

      if(mCollapseGroup >= 0)
      {
         S32 moveChildYBy = (mPreCollapsedYExtent - 24);

         CollapseGroupNumVec::iterator iter = parent->mCollapseGroupVec[mCollapseGroup].begin();
         for(; iter != parent->mCollapseGroupVec[mCollapseGroup].end(); iter++ )
         {
            if((*iter)->mCollapseGroupNum > mCollapseGroupNum)
            {
               Point2I newChildPosition =  (*iter)->getPosition();
               newChildPosition.y += moveChildYBy;					
               (*iter)->resize(newChildPosition, (*iter)->getExtent());
            }
         }
      }
      
      onRestore_callback();
   }
}

//-----------------------------------------------------------------------------

bool GuiWindowCtrl::resizeCollapseGroup(bool resizeX, bool resizeY, Point2I resizePos, Point2I resizeExtent)
{
   GuiControl *parent = getParent();
   if( !parent )
      return false;

   CollapseGroupNumVec	collapseGroupNumVec;

   bool canResize = true;
   CollapseGroupNumVec::iterator iter = parent->mCollapseGroupVec[mCollapseGroup].begin();
   for(; iter != parent->mCollapseGroupVec[mCollapseGroup].end(); iter++ )
   {
      if((*iter) == this)
         continue; 
      
      Point2I newChildPosition = (*iter)->getPosition();
      Point2I newChildExtent = (*iter)->getExtent();

      if( resizeX == true )
      {
         newChildPosition.x -= resizePos.x;
         newChildExtent.x -= resizeExtent.x;
         
      }
      if( resizeY == true )
      {
         if((*iter)->mCollapseGroupNum > mCollapseGroupNum)
         {
            newChildPosition.y -= resizeExtent.y;
            newChildPosition.y -= resizePos.y;
         }
         else if((*iter)->mCollapseGroupNum == mCollapseGroupNum - 1)
            newChildExtent.y -= resizePos.y;
      }

      // check is done for normal extent of windows. if false, check again in case its just giving false
      // due to being a collapsed window. if your truly being forced passed your extent, return false
      if( !(*iter)->mIsCollapsed && newChildExtent.y >= (*iter)->getMinExtent().y )
         (*iter)->resize( newChildPosition, newChildExtent);
      else
      {
         if( (*iter)->mIsCollapsed )
            (*iter)->resize( newChildPosition, newChildExtent);
         else
            canResize = false;
      }
   }
   return canResize;
}

//-----------------------------------------------------------------------------
// Mouse Methods
//-----------------------------------------------------------------------------
S32 GuiWindowCtrl::findHitEdges( const Point2I &globalPoint )
{
   // No Edges
   S32 edgeMask = edgeNone;

   GuiControl *parent = getParent();
   if( !parent )
      return edgeMask;

   RectI bounds( getGlobalBounds() );

   // Create an EdgeRectI structure that has four edges
   // Left/Right/Top/Bottom
   // Each Edge structure has a hit operation that will take
   // another edge and test for a hit on the edge with a margin
   // specified by the .margin scalar
   EdgeRectI edges = EdgeRectI(bounds, mResizeMargin);

   // Get Cursor Edges
   Edge cursorVertEdge = Edge( globalPoint, Point2F( 1.f, 0.f ) );
   Edge cursorHorzEdge = Edge( globalPoint, Point2F( 0.f, 1.f ) );

   if( edges.left.hit( cursorVertEdge ) )
      edgeMask |= edgeLeft;
   else if( edges.right.hit( cursorVertEdge ) )
      edgeMask |= edgeRight;

   if( edges.bottom.hit( cursorHorzEdge ) )
      edgeMask |= edgeBottom;
   else if( edges.top.hit( cursorHorzEdge ) )
   {
      // Only the top window in a collapse group can be extended from the top
      if( mCanCollapse && mCollapseGroup >= 0 )
      {
         if( parent->mCollapseGroupVec[mCollapseGroup].first() !=  this )
            return edgeMask;
      }

      edgeMask |= edgeTop;
   }

   // Return the resulting mask
   return edgeMask;
}

void GuiWindowCtrl::getSnappableWindows( Vector<GuiWindowCtrl*> &windowOutVector, bool canCollapse )
{
   GuiControl *parent = getParent();
   if( !parent )
      return;

   S32 parentSize = parent->size();
   for( S32 i = 0; i < parentSize; i++ )
   {
      GuiWindowCtrl *childWindow = dynamic_cast<GuiWindowCtrl*>(parent->at(i));
      if( !childWindow || !childWindow->isVisible() || childWindow == this || childWindow->mEdgeSnap == false)
         continue;
      
      if( canCollapse && !childWindow->mCanCollapse )
         continue;

      windowOutVector.push_back(childWindow);
   }

}

//=============================================================================
//    Events.
//=============================================================================
// MARK: ---- Events ----

//-----------------------------------------------------------------------------

bool GuiWindowCtrl::onWake()
{
   if (! Parent::onWake())
      return false;

   //get the texture for the close, minimize, and maximize buttons
   mTextureObject = mProfile->mTextureObject;
   bool result = mProfile->constructBitmapArray() >= NumBitmaps;
   if( !result )
   {
      Con::errorf( "GuiWindowCtrl::onWake - failed to create bitmap array from profile bitmap." );
      return false;
   }

   mBitmapBounds = mProfile->mBitmapArrayRects.address();
   S32 buttonHeight = mBitmapBounds[BmpStates * BmpClose].extent.y;

   mTitleHeight = buttonHeight + 4;

   //set the button coords
   positionButtons();

   //set the tab index
   mTabIndex = -1;
   GuiControl *parent = getParent();
   if (parent && mFirstResponder)
   {
      mTabIndex = 0;

      //count the number of windows preceeding this one
      iterator i;
      for (i = parent->begin(); i != parent->end(); i++)
      {
         GuiWindowCtrl *ctrl = dynamic_cast<GuiWindowCtrl *>(*i);
         if (ctrl)
         {
            if (ctrl == this) break;
            else if (ctrl->mFirstResponder) mTabIndex++;
         }
      }
   }

   return true;
}

//-----------------------------------------------------------------------------

void GuiWindowCtrl::onSleep()
{
   mTextureObject = NULL;
   mMousePosition = Point2I(0,0);
   Parent::onSleep();
}

//-----------------------------------------------------------------------------

void GuiWindowCtrl::onMouseDown(const GuiEvent &event)
{
   setUpdate();

   mOrigBounds = getBounds();

   mMouseDownPosition = event.mousePoint;
   Point2I localPoint = globalToLocalCoord(event.mousePoint);

   // Select this window - move it to the front, and set the first responder
   selectWindow();

   mMouseMovingWin = false;

   S32 hitEdges = findHitEdges( event.mousePoint );

   mResizeEdge = edgeNone;

   // Set flag by default so we only clear it
   // if we don't match either edge
   mMouseResizeHeight = true;

   // Check Bottom/Top edges (Mutually Exclusive)
   if( mResizeHeight && hitEdges & edgeBottom )
      mResizeEdge |= edgeBottom;
   else if( mResizeHeight && hitEdges & edgeTop )
      mResizeEdge |= edgeTop;
   else
      mMouseResizeHeight = false;

   // Set flag by default so we only clear it
   // if we don't match either edge
   mMouseResizeWidth = true;

   // Check Left/Right edges (Mutually Exclusive)
   if( mResizeWidth && hitEdges & edgeLeft )
      mResizeEdge |= edgeLeft;
   else if( mResizeWidth && hitEdges & edgeRight )
      mResizeEdge |= edgeRight;
   else
      mMouseResizeWidth = false;


   // If we clicked within the title bar
   if ( !(mResizeEdge & ( edgeTop | edgeLeft | edgeRight ) ) && localPoint.y < mTitleHeight)
   {
      if (mCanClose && mCloseButton.pointInRect(localPoint))
      {
         mCloseButtonPressed = mCanClose;
      }
      else if (mCanMaximize && mMaximizeButton.pointInRect(localPoint))
      {
         mMaximizeButtonPressed = mCanMaximize;
      }
      else if (mCanMinimize && mMinimizeButton.pointInRect(localPoint))
      {
         mMinimizeButtonPressed = mCanMinimize;
      }
      else // We clicked anywhere else within the title
      {
         S32 docking = getDocking();
         if( docking == Docking::dockInvalid || docking == Docking::dockNone )
            mMouseMovingWin = mCanMove;
      
         mMouseResizeWidth = false;
         mMouseResizeHeight = false;
      }
   }


   if (mMouseMovingWin || mResizeEdge != edgeNone ||
         mCloseButtonPressed || mMaximizeButtonPressed || mMinimizeButtonPressed)
   {
      mouseLock();
   }
   else
   {

      GuiControl *ctrl = findHitControl(localPoint);
      if (ctrl && ctrl != this)
         ctrl->onMouseDown(event);
   }
}

//-----------------------------------------------------------------------------

void GuiWindowCtrl::onMouseDragged(const GuiEvent &event)
{
   GuiControl *parent = getParent();
   GuiCanvas *root = getRoot();
   if ( !root ) 
      return;

   mMousePosition = globalToLocalCoord(event.mousePoint);
   Point2I deltaMousePosition = event.mousePoint - mMouseDownPosition;

   Point2I newPosition = getPosition();
   Point2I newExtent = getExtent();
   bool resizeX = false;
   bool resizeY = false;
   mResizeWindow = false;
   mRepositionWindow = false;

   if (mMouseMovingWin && parent)
   {
      if( parent != root )
      {
         newPosition.x = mOrigBounds.point.x + deltaMousePosition.x;
         newPosition.y = getMax(0, mOrigBounds.point.y + deltaMousePosition.y );
         mRepositionWindow = true;
      }
      else
      {
         newPosition.x = getMax(0, getMin(parent->getWidth() - getWidth(), mOrigBounds.point.x + deltaMousePosition.x));
         newPosition.y = getMax(0, getMin(parent->getHeight() - getHeight(), mOrigBounds.point.y + deltaMousePosition.y));
      }

      // Check snapping to other windows
      if( mEdgeSnap )
      {
         RectI bounds = getGlobalBounds();
         bounds.point = mOrigBounds.point + deltaMousePosition;
         EdgeRectI edges = EdgeRectI( bounds, mResizeMargin );
         
         // Create a global-space rectangle that covers the snapping
         // zone of this window. Double the space in which snapping occurs
         // for top and bottom.
         RectI snapZone = bounds;
         snapZone.point.x -= SnapDistance;
         snapZone.point.y -= SnapDistance;
         snapZone.extent.x += SnapDistance + SnapDistance;
         snapZone.extent.y += SnapDistance + SnapDistance;
         
         // Build valid snap and window vectors to compare against
         Vector< GuiWindowCtrl* > windowList;
         getSnappableWindows( windowList );

         for( S32 i =0; i < windowList.size(); i++ )
         {            
            // Make sure the window is both horizontally and vertically
            // within the snap zone for this window.
            if( !snapZone.overlaps( windowList[i]->getGlobalBounds() ) )
               continue;
            
            // Build edges for snap detection
            EdgeRectI snapRect( windowList[i]->getGlobalBounds(), mResizeMargin );

            if( snapRect.right.position.x <= edges.left.position.x + SnapDistance &&
               snapRect.right.position.x >= edges.left.position.x - SnapDistance )
            {
               newPosition.x = snapRect.right.position.x;
            }           
            else if( snapRect.left.position.x <= edges.right.position.x + SnapDistance &&
               snapRect.left.position.x >= edges.right.position.x - SnapDistance )
            {
               newPosition.x = snapRect.left.position.x - bounds.extent.x;
            }
            else if( snapRect.top.position.y <= edges.bottom.position.y + SnapDistance + SnapDistance &&
                  snapRect.top.position.y >= edges.bottom.position.y - SnapDistance - SnapDistance )
            {
               // Ensure that we're not snapping to the middle of collapse groups
               if( (windowList[i]->mCanCollapse && windowList[i]->mCollapseGroup >= 0) ||
                     (mCanCollapse && mCollapseGroup >= 0) )
                     continue;
               newPosition.x = snapRect.left.position.x;
               newPosition.y = snapRect.top.position.y - bounds.extent.y;
            }
            else if( snapRect.bottom.position.y <= edges.top.position.y + SnapDistance + SnapDistance &&
                  snapRect.bottom.position.y >= edges.top.position.y - SnapDistance - SnapDistance)
            {
               // Ensure that we're not snapping to the middle of collapse groups
               // We are not in a group, or we are not in the same group
               if( mCollapseGroup == -1 || ( mCollapseGroup >= 0 && mCollapseGroup != windowList[i]->mCollapseGroup ) )
               {
                  // If the window checked is in a group, if its anything but the last, its n/a
                  if( windowList[i]->mCollapseGroup >= 0 && 
                        windowList[i] != parent->mCollapseGroupVec[windowList[i]->mCollapseGroup].last() )
                     continue;
               }
               else  // We are in the same group, we can't obviously be [0]
               {
                  // If we are [-1/0] we have a serious problem. Also, we only allow connection to the window directly above us
                  if( mCollapseGroupNum <= 0 || 
                        windowList[i] != parent->mCollapseGroupVec[mCollapseGroup][mCollapseGroupNum-1] )
                     continue;
               }

               newPosition.x = snapRect.left.position.x;
               newPosition.y = snapRect.bottom.position.y;
            }
         }
      }
   }
   else if(mCloseButtonPressed || mMaximizeButtonPressed || mMinimizeButtonPressed)
   {
      setUpdate();
      return;
   }
   else
   {
      if( ( !mMouseResizeHeight && !mMouseResizeWidth ) || !parent )
         return;

      mResizeWindow = true;
      if( mResizeEdge & edgeBottom )
      {
         newExtent.y = getMin(parent->getHeight(), mOrigBounds.extent.y + deltaMousePosition.y);
         resizeY = true;
      }
      else if ( mResizeEdge & edgeTop )
      {
         newExtent.y = getMin(parent->getHeight(), mOrigBounds.extent.y - deltaMousePosition.y);
         if ( newExtent.y >= mMinExtent.y )
         {
            // Standard reposition as we're not travelling into the min extent range
            newPosition.y = mOrigBounds.point.y + deltaMousePosition.y;
         }
         else
         {
            // We're into the min extent, so adjust the position up to the min extent
            // so the window doesn't appear to jump
            newPosition.y = mOrigBounds.point.y + (mOrigBounds.extent.y - mMinExtent.y);
         }
         resizeY = true;
      }
      
      if( mResizeEdge & edgeRight )
      {
         newExtent.x = getMin(parent->getWidth(), mOrigBounds.extent.x + deltaMousePosition.x);
         resizeX = true;
      }
      else if( mResizeEdge & edgeLeft )
      {
         newExtent.x = getMin(parent->getWidth(), mOrigBounds.extent.x - deltaMousePosition.x);
         if ( newExtent.x >= mMinExtent.x )
         {
            // Standard reposition as we're not travelling into the min extent range
            newPosition.x = mOrigBounds.point.x + deltaMousePosition.x;
         }
         else
         {
            // We're into the min extent, so adjust the position up to the min extent
            // so the window doesn't appear to jump
            newPosition.x = mOrigBounds.point.x + (mOrigBounds.extent.x - mMinExtent.x);
         }
         resizeX = true;
      }
   }

   // Resize this
   Point2I pos = parent->localToGlobalCoord(getPosition());
   root->addUpdateRegion(pos, getExtent());

   if(mCanCollapse && mCollapseGroup >= 0 && mRepositionWindow == true)
      moveWithCollapseGroup(newPosition);

   if(mCanCollapse && mCollapseGroup >= 0 && mResizeWindow == true )
   {	
      // Resize the window if allowed
      if( newExtent.y >= getMinExtent().y && newExtent.x >= getMinExtent().x)
      {
         mIsMouseResizing = true;
         if( resizeCollapseGroup( resizeX, resizeY, (getPosition() - newPosition), (getExtent() - newExtent) ) )
            resize(newPosition, newExtent);
      }
   }
   else // Normal window sizing functionality
      resize(newPosition, newExtent);
}

//-----------------------------------------------------------------------------

void GuiWindowCtrl::onMouseUp(const GuiEvent &event)
{
   bool closing = mCloseButtonPressed;
   bool maximizing = mMaximizeButtonPressed;
   bool minimizing = mMinimizeButtonPressed;

   mCloseButtonPressed = false;
   mMaximizeButtonPressed = false;
   mMinimizeButtonPressed = false;

   TORQUE_UNUSED(event);
   mouseUnlock();

   mMouseMovingWin = false;
   mMouseResizeWidth = false;
   mMouseResizeHeight = false;

   GuiControl *parent = getParent();
   if (! parent)
      return;

   if( mIsMouseResizing )
   {
      mIsMouseResizing = false;
      return;
   }

   Point2I localPoint = globalToLocalCoord(event.mousePoint);
   if (closing && mCloseButton.pointInRect(localPoint))
   {
      // Here is where were going to put our other if statement
      // if the window closes, and there were only 2 windows in the array, then just delete the array and default there params
      // if not, delete the window from the array, default its params, and renumber the windows
      
      if( engineAPI::gUseConsoleInterop )
         evaluate( mCloseCommand );
      onClose_callback();
   }
   else if (maximizing && mMaximizeButton.pointInRect(localPoint))
   {
      if (mMaximized)
      {
         // Resize to the previous position and extent, bounded by the parent
         resize(Point2I(getMax(0, getMin(parent->getWidth() - mStandardBounds.extent.x, mStandardBounds.point.x)),
                        getMax(0, getMin(parent->getHeight() - mStandardBounds.extent.y, mStandardBounds.point.y))),
                        mStandardBounds.extent);
         // Set the flag
         mMaximized = false;
         
         onRestore_callback();
      }
      else
      {
         // Only save the position if we're not minimized
         if (! mMinimized)
            mStandardBounds = getBounds();
         else
            mMinimized = false;

         // Resize to fit the parent
         resize(Point2I(0, 0), parent->getExtent());

         // Set the flag
         mMaximized = true;
         
         onMaximize_callback();
      }
   }
   else if (minimizing && mMinimizeButton.pointInRect(localPoint))
   {
      if (mMinimized)
      {
         // Resize to the previous position and extent, bounded by the parent
         resize(Point2I(getMax(0, getMin(parent->getWidth() - mStandardBounds.extent.x, mStandardBounds.point.x)),
                        getMax(0, getMin(parent->getHeight() - mStandardBounds.extent.y, mStandardBounds.point.y))),
                        mStandardBounds.extent);
         // Set the flag
         mMinimized = false;
         
         onRestore_callback();
      }
      else
      {
         if (parent->getWidth() < 100 || parent->getHeight() < mTitleHeight + 3)
            return;

         // Only save the position if we're not maximized
         if (! mMaximized)
         {
            mStandardBounds = getBounds();
         }
         else
         {
            mMaximized = false;
         }

         // First find the lowest unused minimized index up to 32 minimized windows
         U32 indexMask = 0;
         iterator i;
         S32 count = 0;
         for (i = parent->begin(); i != parent->end() && count < 32; i++)
         {
            count++;
            S32 index;
            GuiWindowCtrl *ctrl = dynamic_cast<GuiWindowCtrl *>(*i);
            if (ctrl && ctrl->isMinimized(index))
            {
               indexMask |= (1 << index);
            }
         }

         // Now find the first unused bit
         for (count = 0; count < 32; count++)
         {
            if (! (indexMask & (1 << count))) break;
         }

         // If we have more than 32 minimized windows, use the first position
         count = getMax(0, count);

         // This algorithm assumes all window have the same title height, and will minimize to 98 pix
         Point2I newExtent(98, mTitleHeight);

         // First, how many can fit across
         S32 numAcross = getMax(1, (parent->getWidth() / newExtent.x + 2));

         // Find the new "mini position"
         Point2I newPosition;
         newPosition.x = (count % numAcross) * (newExtent.x + 2) + 2;
         newPosition.y = parent->getHeight() - (((count / numAcross) + 1) * (newExtent.y + 2)) - 2;

         // Find the minimized position and extent
         resize(newPosition, newExtent);

         // Set the index so other windows will not try to minimize to the same location
         mMinimizeIndex = count;

         // Set the flag
         mMinimized = true;
         
         onMinimize_callback();
      }
   }
   else if ( !(mResizeEdge & edgeTop) && localPoint.y < mTitleHeight && event.mousePoint == mMouseDownPosition)
   {
      if (mCanClose && mCloseButton.pointInRect(localPoint))
         return;
      else if (mCanMaximize && mMaximizeButton.pointInRect(localPoint))
         return;
      else if (mCanMinimize && mMinimizeButton.pointInRect(localPoint))
         return;
      else if( mCanCollapse ) // If we clicked anywhere else on the title bar
         toggleCollapseGroup();
   }
   else if( mEdgeSnap && mCanCollapse )
   {
      // Create storage pointer
      GuiWindowCtrl* hitWindow = NULL;
      S32 snapType = -1;

      Point2I deltaMousePosition = event.mousePoint - mMouseDownPosition;
      Point2I newExtent = getExtent();

      RectI bounds = getGlobalBounds();
      bounds.point = mOrigBounds.point + deltaMousePosition;
      EdgeRectI edges = EdgeRectI( bounds, mResizeMargin );
      
      RectI snapZone = bounds;
      snapZone.point.x -= SnapDistance;
      snapZone.point.y -= SnapDistance;
      snapZone.extent.x += SnapDistance + SnapDistance;
      snapZone.extent.y += SnapDistance + SnapDistance;

      Vector<EdgeRectI> snapList;
      Vector<GuiWindowCtrl*> windowList;
      
      getSnappableWindows( windowList, true );
      
      for( S32 i =0; i < windowList.size(); i++ )
      {
         if( !snapZone.overlaps( windowList[i]->getGlobalBounds() ) )
            continue;

         // Build edges for snap detection
         EdgeRectI snapRect( windowList[i]->getGlobalBounds(), mResizeMargin );
         
         if( windowList[i]->mCollapseGroupNum == -1 ) 
         {
            // BECOMES "PARENT"
            if( snapRect.top.position.y <= edges.bottom.position.y + SnapDistance + SnapDistance &&
                  snapRect.top.position.y >= edges.bottom.position.y - SnapDistance - SnapDistance )
            {
               hitWindow = windowList[i];
               snapType = 0;
               newExtent.x = snapRect.right.position.x - snapRect.left.position.x;
               break;
            }
         }
         
         if( (windowList[i]->mCollapseGroupNum == -1) || (windowList[i]->mCollapseGroupNum == mCollapseGroupNum - 1) ||
               (!parent->mCollapseGroupVec.empty() && parent->mCollapseGroupVec[windowList[i]->mCollapseGroup].last() ==  windowList[i]) )
         {
            // BECOMES "CHILD"
            if( snapRect.bottom.position.y <= edges.top.position.y + SnapDistance + SnapDistance &&
                  snapRect.bottom.position.y >= edges.top.position.y - SnapDistance - SnapDistance)
            {
               hitWindow = windowList[i];
               snapType = 1;
               newExtent.x = snapRect.right.position.x - snapRect.left.position.x;
               break;
            }
         }
      }
      
      // We're either moving out of a collapse group or moving to another one
      // Not valid for windows not previously in a group
      if( mCollapseGroup >= 0 && 
         ( snapType == -1 || ( snapType >= 0 && mCollapseGroup != hitWindow->mCollapseGroup) ) )
         moveFromCollapseGroup();
      
      // No window to connect to
      if( !hitWindow )
         return;

      moveToCollapseGroup( hitWindow, snapType );
      resize( getPosition(), newExtent );
   }
}

//-----------------------------------------------------------------------------

void GuiWindowCtrl::onMouseMove(const GuiEvent &event)
{
   mMousePosition = globalToLocalCoord(event.mousePoint);
}

bool GuiWindowCtrl::onKeyDown(const GuiEvent &event)
{
   // If this control is a dead end, kill the event
   if ((! mVisible) || (! mActive) || (! mAwake)) return true;

   if ((event.keyCode == KEY_TAB) && (event.modifier & SI_PRIMARY_CTRL))
   {
      // Find the next sibling window, and select it
      GuiControl *parent = getParent();
      if (parent)
      {
         GuiWindowCtrl *firstWindow = NULL;
         iterator i;
         for (i = parent->begin(); i != parent->end(); i++)
         {
            GuiWindowCtrl *ctrl = dynamic_cast<GuiWindowCtrl *>(*i);
            if (ctrl && ctrl->getTabIndex() == mTabIndex + 1)
            {
               ctrl->selectWindow();
               return true;
            }
            else if (ctrl && ctrl->getTabIndex() == 0)
            {
               firstWindow = ctrl;
            }
         }
         // Recycle from the beginning
         if (firstWindow != this)
         {
            firstWindow->selectWindow();
            return true;
         }
      }
   }

   return Parent::onKeyDown(event);
}

//-----------------------------------------------------------------------------

void GuiWindowCtrl::onRender(Point2I offset, const RectI &updateRect)
{
   if( !mProfile || mProfile->mFont == NULL || mProfile->mBitmapArrayRects.size() < NumBitmaps )
      return Parent::onRender( offset, updateRect );

   // Draw the outline
   RectI winRect;
   winRect.point = offset;
   winRect.extent = getExtent();
   GuiCanvas *root = getRoot();
   GuiControl *firstResponder = root ? root->getFirstResponder() : NULL;

   bool isKey = (!firstResponder || controlIsChild(firstResponder));

   U32 topBase = isKey ? BorderTopLeftKey : BorderTopLeftNoKey;
   winRect.point.x += mBitmapBounds[BorderLeft].extent.x;
   winRect.point.y += mBitmapBounds[topBase + 2].extent.y;

   winRect.extent.x -= mBitmapBounds[BorderLeft].extent.x + mBitmapBounds[BorderRight].extent.x;
   winRect.extent.y -= mBitmapBounds[topBase + 2].extent.y + mBitmapBounds[BorderBottom].extent.y;
   
   winRect.extent.x += 1;

   GFX->getDrawUtil()->drawRectFill(winRect, mProfile->mFillColor);

   GFX->getDrawUtil()->clearBitmapModulation();
   GFX->getDrawUtil()->drawBitmapSR(mTextureObject, offset, mBitmapBounds[topBase]);
   GFX->getDrawUtil()->drawBitmapSR(mTextureObject, Point2I(offset.x + getWidth() - mBitmapBounds[topBase+1].extent.x, offset.y),
                   mBitmapBounds[topBase + 1]);

   RectI destRect;
   destRect.point.x = offset.x + mBitmapBounds[topBase].extent.x;
   destRect.point.y = offset.y;
   destRect.extent.x = getWidth() - mBitmapBounds[topBase].extent.x - mBitmapBounds[topBase + 1].extent.x;
   destRect.extent.y = mBitmapBounds[topBase + 2].extent.y;
   RectI stretchRect = mBitmapBounds[topBase + 2];
   stretchRect.inset(1,0);
   GFX->getDrawUtil()->drawBitmapStretchSR(mTextureObject, destRect, stretchRect);

   destRect.point.x = offset.x;
   destRect.point.y = offset.y + mBitmapBounds[topBase].extent.y;
   destRect.extent.x = mBitmapBounds[BorderLeft].extent.x;
   destRect.extent.y = getHeight() - mBitmapBounds[topBase].extent.y - mBitmapBounds[BorderBottomLeft].extent.y;
   stretchRect = mBitmapBounds[BorderLeft];
   stretchRect.inset(0,1);
   GFX->getDrawUtil()->drawBitmapStretchSR(mTextureObject, destRect, stretchRect);

   destRect.point.x = offset.x + getWidth() - mBitmapBounds[BorderRight].extent.x;
   destRect.extent.x = mBitmapBounds[BorderRight].extent.x;
   destRect.point.y = offset.y + mBitmapBounds[topBase + 1].extent.y;
   destRect.extent.y = getHeight() - mBitmapBounds[topBase + 1].extent.y - mBitmapBounds[BorderBottomRight].extent.y;

   stretchRect = mBitmapBounds[BorderRight];
   stretchRect.inset(0,1);
   GFX->getDrawUtil()->drawBitmapStretchSR(mTextureObject, destRect, stretchRect);

   GFX->getDrawUtil()->drawBitmapSR(mTextureObject, offset + Point2I(0, getHeight() - mBitmapBounds[BorderBottomLeft].extent.y), mBitmapBounds[BorderBottomLeft]);
   GFX->getDrawUtil()->drawBitmapSR(mTextureObject, offset + getExtent() - mBitmapBounds[BorderBottomRight].extent, mBitmapBounds[BorderBottomRight]);

   destRect.point.x = offset.x + mBitmapBounds[BorderBottomLeft].extent.x;
   destRect.extent.x = getWidth() - mBitmapBounds[BorderBottomLeft].extent.x - mBitmapBounds[BorderBottomRight].extent.x;

   destRect.point.y = offset.y + getHeight() - mBitmapBounds[BorderBottom].extent.y;
   destRect.extent.y = mBitmapBounds[BorderBottom].extent.y;
   stretchRect = mBitmapBounds[BorderBottom];
   stretchRect.inset(1,0);

   GFX->getDrawUtil()->drawBitmapStretchSR(mTextureObject, destRect, stretchRect);

   // Draw the title
   // dhc addition: copied/modded from renderJustifiedText, since we enforce a
   // different color usage here. NOTE: it currently CAN overdraw the controls
   // if mis-positioned or 'scrunched' in a small width.
   GFX->getDrawUtil()->setBitmapModulation(mProfile->mFontColor);
   S32 textWidth = mProfile->mFont->getStrWidth((const UTF8 *)mText);
   Point2I start(0,0);

   // Align the horizontal
   if ( mProfile->mAlignment == GuiControlProfile::RightJustify )
      start.set( winRect.extent.x - textWidth, 0 );
   else if ( mProfile->mAlignment == GuiControlProfile::CenterJustify )
      start.set( ( winRect.extent.x - textWidth) / 2, 0 );
   else // GuiControlProfile::LeftJustify or garbage... ;)
      start.set( 0, 0 );
   // If the text is longer then the box size, (it'll get clipped) so force Left Justify
   if( textWidth > winRect.extent.x ) start.set( 0, 0 );
   // center the vertical
//   start.y = ( winRect.extent.y - ( font->getHeight() - 2 ) ) / 2;
   GFX->getDrawUtil()->drawText( mProfile->mFont, start + offset + mProfile->mTextOffset, mText );

   // Deal with rendering the titlebar controls
   AssertFatal(root, "Unable to get the root GuiCanvas.");

   // Draw the close button
   Point2I tempUL;
   Point2I tempLR;
   S32 bmp = BmpStates * BmpClose;

   if( mCanClose ) {
      if( mCloseButton.pointInRect( mMousePosition ) )
      {
         if( mCloseButtonPressed )
            bmp += BmpDown;
         else
            bmp += BmpHilite;
      }

      GFX->getDrawUtil()->clearBitmapModulation();
      GFX->getDrawUtil()->drawBitmapSR(mTextureObject, offset + mCloseButton.point, mBitmapBounds[bmp]);
   }

   // Draw the maximize button
   if( mMaximized )
      bmp = BmpStates * BmpNormal;
   else
      bmp = BmpStates * BmpMaximize;

   if( mCanMaximize ) {
      if( mMaximizeButton.pointInRect( mMousePosition ) )
      {
         if( mMaximizeButtonPressed )
            bmp += BmpDown;
         else
            bmp += BmpHilite;
      }

      GFX->getDrawUtil()->clearBitmapModulation();
      GFX->getDrawUtil()->drawBitmapSR( mTextureObject, offset + mMaximizeButton.point, mBitmapBounds[bmp] );
   }

   // Draw the minimize button
   if( mMinimized )
      bmp = BmpStates * BmpNormal;
   else
      bmp = BmpStates * BmpMinimize;

   if( mCanMinimize ) {
      if( mMinimizeButton.pointInRect( mMousePosition ) )
      {
         if( mMinimizeButtonPressed )
            bmp += BmpDown;
         else
            bmp += BmpHilite;
      }

      GFX->getDrawUtil()->clearBitmapModulation();
      GFX->getDrawUtil()->drawBitmapSR( mTextureObject, offset + mMinimizeButton.point, mBitmapBounds[bmp] );
   }

   if( !mMinimized )
   {
      // Render the children
      renderChildControls( offset, updateRect );
   }
}

//=============================================================================
//    Misc.
//=============================================================================
// MARK: ---- Misc ----

//-----------------------------------------------------------------------------

const RectI GuiWindowCtrl::getClientRect()
{
   if( !mProfile || mProfile->mBitmapArrayRects.size() < NumBitmaps )
      return Parent::getClientRect();
      
   if( !mBitmapBounds )
      mBitmapBounds = mProfile->mBitmapArrayRects.address();

   RectI winRect;
   winRect.point.x = mBitmapBounds[BorderLeft].extent.x;
   winRect.point.y = mBitmapBounds[BorderTopKey].extent.y;

   winRect.extent.y = getHeight() - ( winRect.point.y  + mBitmapBounds[BorderBottom].extent.y );
   winRect.extent.x = getWidth() - ( winRect.point.x  + mBitmapBounds[BorderRight].extent.x );

   // Finally, inset it by padding
   // Inset by padding.  margin is specified for all t/b/l/r but 
   // uses only pointx pointy uniformly on both ends. This should be fixed. - JDD
   // winRect.inset( mSizingOptions.mPadding.point.x, mSizingOptions.mPadding.point.y );

   return winRect;
}

//-----------------------------------------------------------------------------

bool GuiWindowCtrl::isMinimized(S32 &index)
{
   index = mMinimizeIndex;
   return mMinimized && mVisible;
}

//-----------------------------------------------------------------------------

void GuiWindowCtrl::positionButtons(void)
{
   if( !mBitmapBounds || !mAwake )
      return;

   S32 buttonWidth = mBitmapBounds[BmpStates * BmpClose].extent.x;
   S32 buttonHeight = mBitmapBounds[BmpStates * BmpClose].extent.y;
   Point2I mainOff = mProfile->mTextOffset;

   // Until a pref, if alignment is LEFT, put buttons RIGHT justified.
   // ELSE, put buttons LEFT justified.
   int closeLeft = mainOff.x, closeTop = mainOff.y, closeOff = buttonWidth + 2;
   if ( mProfile->mAlignment == GuiControlProfile::LeftJustify )
   {
      closeOff = -closeOff;
      closeLeft = getWidth() - buttonWidth - mainOff.x;
   }
   RectI closeRect(closeLeft, closeTop, buttonHeight, buttonWidth);
   mCloseButton = closeRect;

   // Always put Minimize on left side of Maximize.
   closeRect.point.x += closeOff;
   if (closeOff>0)
   {
      mMinimizeButton = closeRect;
      closeRect.point.x += closeOff;
      mMaximizeButton = closeRect;
   }
   else
   {
      mMaximizeButton = closeRect;
      closeRect.point.x += closeOff;
      mMinimizeButton = closeRect;
   }
}

//-----------------------------------------------------------------------------

void GuiWindowCtrl::setCloseCommand(const char *newCmd)
{
   if (newCmd)
      mCloseCommand = StringTable->insert(newCmd);
   else
      mCloseCommand = StringTable->insert("");
}

//-----------------------------------------------------------------------------

GuiControl* GuiWindowCtrl::findHitControl(const Point2I &pt, S32 initialLayer)
{
   if (! mMinimized)
      return Parent::findHitControl(pt, initialLayer);
   else
      return this;
}

//-----------------------------------------------------------------------------

bool GuiWindowCtrl::resize(const Point2I &newPosition, const Point2I &newExtent)
{
   if( !Parent::resize(newPosition, newExtent) )
      return false;

   // Set the button coords
   positionButtons();

   return true;
}

//-----------------------------------------------------------------------------

GuiControl *GuiWindowCtrl::findNextTabable(GuiControl *curResponder, bool firstCall)
{
   // Set the global if this is the first call (directly from the canvas)
   if (firstCall)
   {
      GuiControl::smCurResponder = NULL;
   }

   // If the window does not already contain the first responder, return false
   // ie.  Can't tab into or out of a window
   if (! controlIsChild(curResponder))
   {
      return NULL;
   }

   // Loop through, checking each child to see if it is the one that follows the firstResponder
   GuiControl *tabCtrl = NULL;
   iterator i;
   for (i = begin(); i != end(); i++)
   {
      GuiControl *ctrl = static_cast<GuiControl *>(*i);
      tabCtrl = ctrl->findNextTabable(curResponder, false);
      if (tabCtrl) break;
   }

   // To ensure the tab cycles within the current window...
   if (! tabCtrl)
   {
      tabCtrl = findFirstTabable();
   }

   mFirstResponder = tabCtrl;
   return tabCtrl;
}

//-----------------------------------------------------------------------------

GuiControl *GuiWindowCtrl::findPrevTabable(GuiControl *curResponder, bool firstCall)
{
   if (firstCall)
   {
      GuiControl::smPrevResponder = NULL;
   }

   // If the window does not already contain the first responder, return false
   // ie.  Can't tab into or out of a window
   if (! controlIsChild(curResponder))
   {
      return NULL;
   }

   // Loop through, checking each child to see if it is the one that follows the firstResponder
   GuiControl *tabCtrl = NULL;
   iterator i;
   for (i = begin(); i != end(); i++)
   {
      GuiControl *ctrl = static_cast<GuiControl *>(*i);
      tabCtrl = ctrl->findPrevTabable(curResponder, false);
      if (tabCtrl) break;
   }

   // To ensure the tab cycles within the current window...
   if (! tabCtrl)
   {
      tabCtrl = findLastTabable();
   }

   mFirstResponder = tabCtrl;
   return tabCtrl;
}

//-----------------------------------------------------------------------------

void GuiWindowCtrl::selectWindow(void)
{
   // First make sure this window is the front most of its siblings
   GuiControl *parent = getParent();
   if (parent && *parent->end() != this )
   {
      // Valid collapse groups have to be selected together
      if( mCanCollapse && mCollapseGroup >= 0 )
      {
         CollapseGroupNumVec::iterator iter = parent->mCollapseGroupVec[mCollapseGroup].begin();
         for(; iter != parent->mCollapseGroupVec[mCollapseGroup].end(); iter++ )
         {
            parent->pushObjectToBack( (*iter) );
         }
      }
      else
      {
         parent->pushObjectToBack( this );
      }
   }

   // Also set the first responder to be the one within this window
   setFirstResponder(mFirstResponder);
}

//-----------------------------------------------------------------------------

void GuiWindowCtrl::getCursor(GuiCursor *&cursor, bool &showCursor, const GuiEvent &lastGuiEvent)
{
   GuiCanvas *pRoot = getRoot();
   if( !pRoot )
      return;
   PlatformWindow *pWindow = static_cast<GuiCanvas*>(getRoot())->getPlatformWindow();
   AssertFatal(pWindow != NULL,"GuiControl without owning platform window!  This should not be possible.");
   PlatformCursorController *pController = pWindow->getCursorController();
   AssertFatal(pController != NULL,"PlatformWindow without an owned CursorController!");

   S32 desiredCursor = PlatformCursorController::curArrow;
   S32 hitEdges = findHitEdges( lastGuiEvent.mousePoint );

   if( hitEdges & edgeBottom && hitEdges & edgeLeft && mResizeHeight )
      desiredCursor = PlatformCursorController::curResizeNESW;
   else if( hitEdges & edgeBottom && hitEdges & edgeRight && mResizeHeight  )
      desiredCursor = PlatformCursorController::curResizeNWSE;
   else if( hitEdges & edgeBottom && mResizeHeight )
      desiredCursor = PlatformCursorController::curResizeHorz;
   else if( hitEdges & edgeTop && hitEdges & edgeLeft && mResizeHeight )
      desiredCursor = PlatformCursorController::curResizeNWSE;
   else if( hitEdges & edgeTop && hitEdges & edgeRight && mResizeHeight )
      desiredCursor = PlatformCursorController::curResizeNESW;
   else if( hitEdges & edgeTop && mResizeHeight )
      desiredCursor = PlatformCursorController::curResizeHorz;
   else if ( hitEdges & edgeLeft && mResizeWidth )
      desiredCursor = PlatformCursorController::curResizeVert;
   else if( hitEdges & edgeRight && mResizeWidth )
      desiredCursor = PlatformCursorController::curResizeVert;
   else
      desiredCursor = PlatformCursorController::curArrow;

   // Bail if we're already at the desired cursor
   if(pRoot->mCursorChanged == desiredCursor )
      return;

   // Now change the cursor shape
   pController->popCursor();
   pController->pushCursor(desiredCursor);
   pRoot->mCursorChanged = desiredCursor;
}

//-----------------------------------------------------------------------------

void GuiWindowCtrl::parentResized(const RectI &oldParentRect, const RectI &newParentRect)
{
   if(!mCanResize)
      return;

   GuiControl *parent = getParent();
   if( !parent )
      return;

   // Bail if were not sized both by windowrelative
   if( mHorizSizing != horizResizeWindowRelative || mHorizSizing != vertResizeWindowRelative )
      return Parent::parentResized( oldParentRect, newParentRect );

   Point2I newPosition = getPosition();
   Point2I newExtent = getExtent();
         
   bool doCollapse = false;

   S32 deltaX = newParentRect.extent.x - oldParentRect.extent.x;
   S32 deltaY = newParentRect.extent.y - oldParentRect.extent.y;// + mProfile->mYPositionOffset;

   if( newPosition.x > ( oldParentRect.extent.x / 2 ) )
      newPosition.x = newPosition.x + deltaX;

   if (oldParentRect.extent.y != 0)
   {
      // Only if were apart of a group
      if ( mCanCollapse && mCollapseGroup >= 0 )
      {
         // Setup parsing mechanisms
         CollapseGroupNumVec collapseGroupNumVec;
         
         // Lets grab the information we need (this should probably be already stored on each individual window object)
         S32 groupNum = mCollapseGroup;
         S32 groupMax = parent->mCollapseGroupVec[ groupNum ].size() - 1;

         // Set up vars that we're going to be using
         S32 groupPos = 0;
         S32 groupExtent = 0;
         S32 tempGroupExtent = 0;

         // Set up the vector/iterator combo we need
         collapseGroupNumVec = parent->mCollapseGroupVec[ groupNum ];
         CollapseGroupNumVec::iterator iter = collapseGroupNumVec.begin();

         // Grab some more information we need later ( this info should also be located on each ind. window object )
         for( ; iter != collapseGroupNumVec.end(); iter++ )
         {
            if((*iter)->getCollapseGroupNum() == 0)
            {
               groupPos = (*iter)->getPosition().y;
            }

            groupExtent += (*iter)->getExtent().y;
         }
         
         // Use the information we just gatherered; only enter this block if we need to
         tempGroupExtent = groupPos + groupExtent;
         if( tempGroupExtent > ( newParentRect.extent.y / 2 ) )
         {
            // Lets size the collapse group
            S32 windowParser = groupMax;
            bool secondLoop = false;
            while( tempGroupExtent >= newParentRect.extent.y )
            {
               
               if( windowParser == -1)
               {
                  if( !secondLoop )
                  {
                     secondLoop = true;
                     windowParser = groupMax;
                  }
                  else
                     break;
               }

               GuiWindowCtrl *tempWindow = collapseGroupNumVec[windowParser];
               if(tempWindow->mIsCollapsed)
               {
                  windowParser--;
                  continue;
               }
               
               // Resizing block for the loop... if we can get away with just resizing the bottom window do it before
               // resizing the whole block. else, go through the group and start setting min extents. if that doesnt work
               // on the second go around, start collpsing the windows.
               if( tempGroupExtent - ( tempWindow->getExtent().y - tempWindow->getMinExtent().y ) <= newParentRect.extent.y )
               {
                  if( this == tempWindow )
                     newExtent.y = newExtent.y - ( tempGroupExtent - newParentRect.extent.y );
                     
                  tempGroupExtent = tempGroupExtent - newParentRect.extent.y;
               }
               else
               {
                  if( secondLoop )
                  {
                     tempGroupExtent = tempGroupExtent - (tempWindow->getExtent().y + 32);

                     if( this == tempWindow )
                        doCollapse = true;
                  }
                  else
                  {
                     tempGroupExtent = tempGroupExtent - (tempWindow->getExtent().y - tempWindow->getMinExtent().y);
                     if( this == tempWindow )
                        newExtent.y = tempWindow->getMinExtent().y;
                  }
               }
               windowParser--;
            }
         }
      }
      else if( newPosition.y > ( oldParentRect.extent.y / 2 ) )
      {
         newPosition.y = newPosition.y + deltaY - 1;
      }
   }

   if( newExtent.x >= getMinExtent().x && newExtent.y >= getMinExtent().y )
   {
      // If we are already outside the reach of the main window, lets not place ourselves
      // further out; but if were trying to improve visibility, go for it
      // note: small tolerance (4) added to keep guis that are very slightly outside
      // the main window (like all of the editor windows!) from appearing to 'undock'
      // when the resolution is changed.
      if( (newPosition.x + newExtent.x) > newParentRect.extent.x + 4 )
      {
         if( (newPosition.x + newExtent.x) > (getPosition().x + getExtent().x) )
         { 
            newPosition.x = getPosition().x;
            newExtent.x = getExtent().x;
         }
      }
      if( (newPosition.y + newExtent.y) > newParentRect.extent.y + 4)
      {
         if( (newPosition.y + newExtent.y) > (getPosition().y + getExtent().y) )
         {
            newPosition.y = getPosition().y;
            newExtent.y = getExtent().y;
         }
      }

      // Only for collpasing groups, if were not, then do it like normal windows
      if( mCanCollapse && mCollapseGroup >= 0 )
      {	
         bool resizeMe = false;
         
         // Only the group window should control positioning
         if( mCollapseGroupNum == 0 )
         {
            resizeMe = resizeCollapseGroup( true, true, (getPosition() - newPosition), (getExtent() - newExtent) );
            if(resizeMe == true)
               resize(newPosition, newExtent);
         }
         else if( getExtent() != newExtent)
         {
            resizeMe = resizeCollapseGroup( false, true, (getPosition() - newPosition), (getExtent() - newExtent) );
            if(resizeMe == true)
               resize(getPosition(), newExtent);
         }
      }
      else
      {
         resize(newPosition, newExtent);
      }
   }

   if( mCanCollapse && !mIsCollapsed && doCollapse )
      toggleCollapseGroup();

   // If docking is invalid on this control, then bail out here
   if( getDocking() & Docking::dockInvalid || getDocking() & Docking::dockNone)
      return;

   // Update Self
   RectI oldThisRect = getBounds();
   anchorControl( this, Point2I( deltaX, deltaY ) );
   RectI newThisRect = getBounds();

   // Update Deltas to pass on to children
   deltaX = newThisRect.extent.x - oldThisRect.extent.x;
   deltaY = newThisRect.extent.y - oldThisRect.extent.y;

   // Iterate over all children and update their anchors
   iterator nI = begin();
   for( ; nI != end(); nI++ )
   {
      // Sanity
      GuiControl *control = dynamic_cast<GuiControl*>( (*nI) );
      if( control )
         control->parentResized( oldThisRect, newThisRect );
   }
}

//=============================================================================
//    Console Methods.
//=============================================================================
// MARK: ---- Console Methods ----

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiWindowCtrl, selectWindow, void, (),,
   "Bring the window to the front." )
{
   object->selectWindow();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiWindowCtrl, setCollapseGroup, void, ( bool state ),,
   "Set the window's collapsing state." )
{
   object->setCollapseGroup(state);
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiWindowCtrl, toggleCollapseGroup, void, (),,
   "Toggle the window collapsing." )
{
   object->toggleCollapseGroup();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiWindowCtrl, attachTo, void, ( GuiWindowCtrl* window ),,
   "" )
{
   object->moveToCollapseGroup( window, 1 );
}

//-----------------------------------------------------------------------------

DefineEngineStaticMethod( GuiWindowCtrl, attach, void, ( GuiWindowCtrl* bottomWindow, GuiWindowCtrl* topWindow ),,
   "Attach @a bottomWindow to @topWindow so that @a bottomWindow moves along with @a topWindow when it is dragged.\n\n"
   "@param bottomWindow \n"
   "@param topWindow " )
{   
   if(bottomWindow == NULL || topWindow == NULL)
   {
      Con::warnf("Warning: AttachWindows - could not find windows");
      return;
   }

   bottomWindow->moveToCollapseGroup( topWindow, 1 );
}
