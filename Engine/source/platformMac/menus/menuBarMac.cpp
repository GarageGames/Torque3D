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

#include "platformMac/platformMacCarb.h"
#include "platform/menus/menuBar.h"
#include "platform/menus/popupMenu.h"
#include "gui/core/guiCanvas.h"
#include "windowManager/platformWindowMgr.h"
#include "windowManager/platformWindow.h"


class PlatformMenuBarData
{
public:
   PlatformMenuBarData() :
      mMenuEventHandlerRef(NULL),
      mCommandEventHandlerRef(NULL),
      mMenuOpenCount( 0 ),
      mLastCloseTime( 0 )
   {}

   EventHandlerRef mMenuEventHandlerRef;
   EventHandlerRef mCommandEventHandlerRef;
   
   /// More evil hacking for OSX.  There seems to be no way to disable menu shortcuts and
   /// they are automatically routed within that Cocoa thing outside of our control.  Also,
   /// there's no way of telling what triggered a command event and thus no way of knowing
   /// whether it was a keyboard shortcut.  Sigh.
   ///
   /// So what we do here is monitor the sequence of events leading to a command event.  We
   /// capture the time the last open menu was closed and then, when we receive a command
   /// event (which are dished out after the menus are closed) and keyboard accelerators are
   /// disabled, we check whether we are a certain very short time away in the event stream
   /// from the menu close event.  If so, we figure the event came from clicking in a menu.
   ///
   /// Utterly evil and dirty but seems to do the trick.
   U32 mMenuOpenCount;
   EventTime mLastCloseTime;
};

//-----------------------------------------------------------------------------

#pragma mark -
#pragma mark ---- menu event handler ----
static OSStatus _OnMenuEvent(EventHandlerCallRef nextHandler, EventRef theEvent, void *userData)
{
   PlatformMenuBarData* mbData = ( PlatformMenuBarData* ) userData;
   MenuRef menu;

   GetEventParameter(theEvent, kEventParamDirectObject, typeMenuRef, NULL, sizeof(MenuRef), NULL, &menu);
   
   // Count open/close for the sake of hotkey disabling.
   
   UInt32 kind = GetEventKind( theEvent );
   if( kind == kEventMenuOpening )
      mbData->mMenuOpenCount ++;
   else
   {
      AssertWarn( mbData->mMenuOpenCount > 0, "Unbalanced menu open/close events in _OnMenuEvent" );
      if( mbData->mMenuOpenCount )
         mbData->mMenuOpenCount --;
         
      // Initial menu closed.  Capture time.
         
      if( !mbData->mMenuOpenCount )
         mbData->mLastCloseTime = GetEventTime( theEvent );
   }

   OSStatus err = eventNotHandledErr;
   PopupMenu *torqueMenu;
   if( CountMenuItems( menu ) > 0 )
   {
      // I don't know of a way to get the Torque PopupMenu object from a Carbon MenuRef
      //   other than going through its first menu item
      err = GetMenuItemProperty(menu, 1, 'GG2d', 'ownr', sizeof(PopupMenu*), NULL, &torqueMenu);
      if( err == noErr && torqueMenu != NULL )
      {
         torqueMenu->onMenuSelect();
      }
   }

   return err;
}

//-----------------------------------------------------------------------------

#pragma mark -
#pragma mark ---- menu command event handler ----
static bool MacCarbHandleMenuCommand( void* hiCommand, PlatformMenuBarData* mbData )
{
   HICommand *cmd = (HICommand*)hiCommand;
      
   if(cmd->commandID != kHICommandTorque)
      return false;
      
   MenuRef menu = cmd->menu.menuRef;
   MenuItemIndex item = cmd->menu.menuItemIndex;
   
   // Run the command handler.
   
   PopupMenu* torqueMenu;
   OSStatus err = GetMenuItemProperty(menu, item, 'GG2d', 'ownr', sizeof(PopupMenu*), NULL, &torqueMenu);
   AssertFatal(err == noErr, "Could not resolve the PopupMenu stored on a native menu item");
   
   UInt32 command;
   err = GetMenuItemRefCon(menu, item, &command);
   AssertFatal(err == noErr, "Could not find the tag of a native menu item");
   
   if(!torqueMenu->canHandleID(command))
      Con::errorf("menu claims it cannot handle that id. how odd.");

   // un-highlight currently selected menu
   HiliteMenu( 0 );

   return torqueMenu->handleSelect(command,NULL);
}

//-----------------------------------------------------------------------------

#pragma mark -
#pragma mark ---- Command Events ----

static OSStatus _OnCommandEvent(EventHandlerCallRef nextHandler, EventRef theEvent, void* userData)
{
   PlatformMenuBarData* mbData = ( PlatformMenuBarData* ) userData;
   
   HICommand commandStruct;

   OSStatus result = eventNotHandledErr;
   
   GetEventParameter(theEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &commandStruct);
   
   // pass menu command events to a more specific handler.
   if(commandStruct.attributes & kHICommandFromMenu)
   {
      bool handleEvent = true;
      
      // Do menu-close check hack.
      
      PlatformWindow* window = PlatformWindowManager::get()->getFocusedWindow();
      if( !window || !window->getAcceleratorsEnabled() )
      {
         F32 deltaTime = mFabs( GetEventTime( theEvent ) - mbData->mLastCloseTime );
         if( deltaTime > 0.1f )
            handleEvent = false;
      }
      
      if( handleEvent && MacCarbHandleMenuCommand(&commandStruct, mbData) )
            result = noErr;
   }
   
   return result;
}



//-----------------------------------------------------------------------------
// MenuBar Methods
//-----------------------------------------------------------------------------

void MenuBar::createPlatformPopupMenuData()
{
   
   mData = new PlatformMenuBarData;

}

void MenuBar::deletePlatformPopupMenuData()
{
   
    SAFE_DELETE(mData);
}

//-----------------------------------------------------------------------------

void MenuBar::attachToCanvas(GuiCanvas *owner, S32 pos)
{
   if(owner == NULL || isAttachedToCanvas())
      return;
      
   mCanvas = owner;
   
   
  // Add the items
   for(S32 i = 0;i < size();++i)
   {
      PopupMenu *mnu = dynamic_cast<PopupMenu *>(at(i));
      if(mnu == NULL)
      {
         Con::warnf("MenuBar::attachToMenuBar - Non-PopupMenu object in set");
         continue;
      }

      if(mnu->isAttachedToMenuBar())
         mnu->removeFromMenuBar();

      mnu->attachToMenuBar(owner, pos + i, mnu->getBarTitle());
   }
   
   // register as listener for menu opening events
   static EventTypeSpec menuEventTypes[ 2 ];
   
   menuEventTypes[ 0 ].eventClass = kEventClassMenu;
   menuEventTypes[ 0 ].eventKind = kEventMenuOpening;
   menuEventTypes[ 1 ].eventClass = kEventClassMenu;
   menuEventTypes[ 1 ].eventKind = kEventMenuClosed;

   EventHandlerUPP menuEventHandlerUPP;
   menuEventHandlerUPP = NewEventHandlerUPP(_OnMenuEvent);   
   InstallEventHandler(GetApplicationEventTarget(), menuEventHandlerUPP, 2, menuEventTypes, mData, &mData->mMenuEventHandlerRef);
   
   // register as listener for process command events
   static EventTypeSpec comEventTypes[1];
   comEventTypes[0].eventClass = kEventClassCommand;
   comEventTypes[0].eventKind = kEventCommandProcess;
   
   EventHandlerUPP commandEventHandlerUPP;
   commandEventHandlerUPP = NewEventHandlerUPP(_OnCommandEvent);
   InstallEventHandler(GetApplicationEventTarget(), commandEventHandlerUPP, 1, comEventTypes, mData, &mData->mCommandEventHandlerRef);
}

//-----------------------------------------------------------------------------

void MenuBar::removeFromCanvas()
{
   if(mCanvas == NULL || ! isAttachedToCanvas())
      return;
   
   if(mData->mCommandEventHandlerRef != NULL)
      RemoveEventHandler( mData->mCommandEventHandlerRef );
   mData->mCommandEventHandlerRef = NULL;
   
   if(mData->mMenuEventHandlerRef != NULL)
      RemoveEventHandler( mData->mMenuEventHandlerRef );
   mData->mMenuEventHandlerRef = NULL;

   // Add the items
   for(S32 i = 0;i < size();++i)
   {
      PopupMenu *mnu = dynamic_cast<PopupMenu *>(at(i));
      if(mnu == NULL)
      {
         Con::warnf("MenuBar::removeFromMenuBar - Non-PopupMenu object in set");
         continue;
      }

      mnu->removeFromMenuBar();
   }

   mCanvas = NULL;
}

//-----------------------------------------------------------------------------

void MenuBar::updateMenuBar(PopupMenu* menu)
{
   if(! isAttachedToCanvas())
      return;
   
   menu->removeFromMenuBar();
   SimSet::iterator itr = find(begin(), end(), menu);
   if(itr == end())
      return;
   
   // Get the item currently at the position we want to add to
   S32 pos = itr - begin();
   S16 posID = 0;
   
   PopupMenu *nextMenu = NULL;
   for(S32 i = pos + 1; i < size(); i++)
   {
      PopupMenu *testMenu = dynamic_cast<PopupMenu *>(at(i));
      if (testMenu && testMenu->isAttachedToMenuBar())
      {
         nextMenu = testMenu;
         break;
      }
   }

   if(nextMenu)
      posID = GetMenuID(nextMenu->mData->mMenu);
   
   menu->attachToMenuBar(mCanvas, posID, menu->mBarTitle);
}