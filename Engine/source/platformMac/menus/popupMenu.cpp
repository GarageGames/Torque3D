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
#include "platform/menus/popupmenu.h"
#include "core/util/safeDelete.h"
#include "gui/core/guiCanvas.h"

void PopupMenu::createPlatformPopupMenuData()
{
   mData = new PlatformPopupMenuData;
}

void PopupMenu::deletePlatformPopupMenuData()
{
   SAFE_DELETE(mData);
}

void PopupMenu::createPlatformMenu()
{
   OSStatus err = CreateNewMenu( mData->tag, kMenuAttrAutoDisable,&(mData->mMenu));
   CFRetain(mData->mMenu);
   AssertFatal(err == noErr, "Could not create Carbon MenuRef");
}

static int _getModifierMask(const char* accel)
{
   int ret = 0;
   if(dStrstr(accel, "ctrl"))
      ret |= kMenuControlModifier;
   if(dStrstr(accel, "shift"))
      ret |= kMenuShiftModifier;
   if(dStrstr(accel, "alt"))
      ret |= kMenuOptionModifier;
   if(!(dStrstr(accel, "cmd") || dStrstr(accel, "command")))
      ret |= kMenuNoCommandModifier;
   
   return ret;
}

static void _assignCommandKeys(const char* accel, MenuRef menu, MenuItemIndex item)
{
   if(!(accel && *accel))
      return;
   
   // get the modifier keys
   String _accel = String::ToLower( accel );
   int mods = _getModifierMask(_accel);

   // accel is space or dash delimted.
   // the modifier key is either the last token in accel, or the first char in accel.
   const char* key = dStrrchr(_accel, ' ');
   if(!key)
      key = dStrrchr(_accel, '-');
   if(!key)
      key = _accel;
   else
      key++;
      
   if(dStrlen(key) <= 1)
   {
      char k = dToupper( key[0] );
      SetMenuItemCommandKey( menu, item, false, k );
   }
   else
   {
      SInt16 glyph = kMenuNullGlyph;

      //*** A lot of these mappings came from a listing at http://developer.apple.com/releasenotes/Carbon/HIToolboxOlderNotes.html
      if(!dStricmp(key, "DELETE"))
         glyph = kMenuDeleteRightGlyph;
         
      else if(!dStricmp(key, "HOME"))
         glyph = kMenuNorthwestArrowGlyph;
         
      else if(!dStricmp(key, "END"))
         glyph = kMenuSoutheastArrowGlyph;
         
      else if(!dStricmp(key, "BACKSPACE"))
         glyph =  kMenuDeleteLeftGlyph;
         
      else if(!dStricmp(key, "TAB"))
         glyph =  kMenuTabRightGlyph;
         
      else if(!dStricmp(key, "RETURN"))
         glyph =  kMenuReturnGlyph;
         
      else if(!dStricmp(key, "ENTER"))
         glyph =  kMenuEnterGlyph;
         
      else if(!dStricmp(key, "PG UP"))
         glyph =  kMenuPageUpGlyph;
         
      else if(!dStricmp(key, "PG DOWN"))
         glyph =  kMenuPageDownGlyph;
         
      else if(!dStricmp(key, "ESC"))
         glyph =  kMenuEscapeGlyph;
         
      else if(!dStricmp(key, "LEFT"))
         glyph =  kMenuLeftArrowGlyph;
         
      else if(!dStricmp(key, "RIGHT"))
         glyph =  kMenuRightArrowGlyph;
         
      else if(!dStricmp(key, "UP"))
         glyph =  kMenuUpArrowGlyph;

      else if(!dStricmp(key, "DOWN"))
         glyph =  kMenuDownArrowGlyph;

      else if(!dStricmp(key, "SPACE"))
         glyph =  kMenuSpaceGlyph;

      else if(!dStricmp(key, "F1"))
         glyph =  kMenuF1Glyph;

      else if(!dStricmp(key, "F2"))
         glyph =  kMenuF2Glyph;

      else if(!dStricmp(key, "F3"))
         glyph =  kMenuF3Glyph;

      else if(!dStricmp(key, "F4"))
         glyph =  kMenuF4Glyph;

      else if(!dStricmp(key, "F5"))
         glyph =  kMenuF5Glyph;

      else if(!dStricmp(key, "F6"))
         glyph =  kMenuF6Glyph;

      else if(!dStricmp(key, "F7"))
         glyph =  kMenuF7Glyph;

      else if(!dStricmp(key, "F8"))
         glyph =  kMenuF8Glyph;

      else if(!dStricmp(key, "F9"))
         glyph =  kMenuF9Glyph;

      else if(!dStricmp(key, "F10"))
         glyph =  kMenuF10Glyph;

      else if(!dStricmp(key, "F11"))
         glyph =  kMenuF11Glyph;

      else if(!dStricmp(key, "F12"))
         glyph =  kMenuF12Glyph;

      else if(!dStricmp(key, "F13"))
         glyph =  kMenuF13Glyph;

      else if(!dStricmp(key, "F14"))
         glyph =  kMenuF14Glyph;

      else if(!dStricmp(key, "F15"))
         glyph =  kMenuF15Glyph;

      SetMenuItemKeyGlyph(menu, item, glyph);
   }

   SetMenuItemModifiers(menu, item, mods);
}


S32 PopupMenu::insertItem(S32 pos, const char *title, const char* accel)
{
   MenuItemIndex item;
   CFStringRef cftitle;
   MenuItemAttributes attr = 0;
   
   bool needRelease = false;
   if(title && *title)
   {
      cftitle = CFStringCreateWithCString(NULL,title,kCFStringEncodingUTF8);
      needRelease = true;
   }
   else
   {
      cftitle = CFSTR("-");
      attr = kMenuItemAttrSeparator;
   }
      
   InsertMenuItemTextWithCFString(mData->mMenu, cftitle, pos, attr, kHICommandTorque + 1);
   if( needRelease )
      CFRelease( cftitle );

   // ensure that we have the correct index for the new menu item
   MenuRef outref;
   GetIndMenuItemWithCommandID(mData->mMenu, kHICommandTorque+1, 1, &outref, &item);
   SetMenuItemCommandID(mData->mMenu, item, kHICommandTorque);
   
   // save a ref to the PopupMenu that owns this item.
   PopupMenu* thisMenu = this;
   SetMenuItemProperty(mData->mMenu, item, 'GG2d', 'ownr', sizeof(PopupMenu*), &thisMenu);
   
   // construct the accelerator keys
   _assignCommandKeys(accel, mData->mMenu, item);
   
   S32 tag = PlatformPopupMenuData::getTag();
   SetMenuItemRefCon(mData->mMenu, item, tag);
   
   return tag;
}

S32 PopupMenu::insertSubMenu(S32 pos, const char *title, PopupMenu *submenu)
{
   for(S32 i = 0;i < mSubmenus->size();i++)
   {
      if(submenu == (*mSubmenus)[i])
      {
         Con::errorf("PopupMenu::insertSubMenu - Attempting to add submenu twice");
         return -1;
      }
   }

   CFStringRef cftitle = CFStringCreateWithCString(NULL,title,kCFStringEncodingUTF8);
   InsertMenuItemTextWithCFString(mData->mMenu, cftitle, pos, 0, kHICommandTorque + 1);
   CFRelease( cftitle );
   
   // ensure that we have the correct index for the new menu item
   MenuRef outref;
   MenuItemIndex item;
   GetIndMenuItemWithCommandID(mData->mMenu, kHICommandTorque+1, 1, &outref, &item);
   SetMenuItemCommandID(mData->mMenu, item, 0);
   
   S32 tag = PlatformPopupMenuData::getTag();
   SetMenuItemRefCon( mData->mMenu, item, tag);
   
   // store a pointer to the PopupMenu this item represents. See PopupMenu::removeItem()
   SetMenuItemProperty(mData->mMenu, item, 'GG2d', 'subm', sizeof(PopupMenu*), submenu);
   
   SetMenuItemHierarchicalMenu( mData->mMenu, item, submenu->mData->mMenu);
   mSubmenus->addObject(submenu);
   
   return tag;
}

void PopupMenu::removeItem(S32 itemPos)
{
   PopupMenu* submenu;
   itemPos++; // adjust torque -> mac menu index

   OSStatus err = GetMenuItemProperty(mData->mMenu, itemPos, 'GG2d', 'subm', sizeof(PopupMenu*),NULL,&submenu);
   if(err == noErr)
      mSubmenus->removeObject(submenu);
      
   // deleting the item decrements the ref count on the mac submenu.
   DeleteMenuItem(mData->mMenu, itemPos);
}

//////////////////////////////////////////////////////////////////////////

void PopupMenu::enableItem(S32 pos, bool enable)
{
   pos++; // adjust torque -> mac menu index.
   if(enable)
      EnableMenuItem(mData->mMenu, pos);
   else
      DisableMenuItem(mData->mMenu, pos);
}

void PopupMenu::checkItem(S32 pos, bool checked)
{
   pos++;
   CheckMenuItem(mData->mMenu, pos, checked);
}

void PopupMenu::checkRadioItem(S32 firstPos, S32 lastPos, S32 checkPos)
{
   // uncheck items
   for(int i = firstPos; i <= lastPos; i++)
      checkItem( i, false);
   
   // check the selected item
   checkItem( checkPos, true);
}

bool PopupMenu::isItemChecked(S32 pos)
{
   CharParameter mark;
   GetItemMark(mData->mMenu, pos, &mark);
   return (mark == checkMark);
}

//////////////////////////////////////////////////////////////////////////

// this method really isn't necessary for the mac implementation
bool PopupMenu::canHandleID(U32 iD)
{
   for(S32 i = 0;i < mSubmenus->size();i++)
   {
      PopupMenu *subM = dynamic_cast<PopupMenu *>((*mSubmenus)[i]);
      if(subM == NULL)
         continue;

      if(subM->canHandleID(iD))
         return true;
   }

   UInt32 refcon;
   U32 nItems = CountMenuItems(mData->mMenu);
   for(int i = 1; i <= nItems; i++)
   {
      GetMenuItemRefCon(mData->mMenu, i, &refcon);
      if(refcon == iD)
         return true;
   }

   return false;
}

bool PopupMenu::handleSelect(U32 command, const char *text /* = NULL */)
{
   // [tom, 8/20/2006] Pass off to a sub menu if it's for them
   for(S32 i = 0;i < mSubmenus->size();i++)
   {
      PopupMenu *subM = dynamic_cast<PopupMenu *>((*mSubmenus)[i]);
      if(subM == NULL)
         continue;

      if(subM->canHandleID(command))
      {
         return subM->handleSelect(command, text);
      }
   }

   // ensure that this menu actually has an item with the specificed command / refcon.
   // this is not strictly necessary, we're just doing it here to keep the behavior
   // in line with the windows implementation.
   UInt32 refcon;
   U32 nItems = CountMenuItems(mData->mMenu);
   S32 pos = -1;
   for(int i = 1; i <= nItems; i++)
   {
      GetMenuItemRefCon(mData->mMenu, i, &refcon);
      if(refcon == command)
         pos = i;
   }
   if(pos == -1)
   {
      Con::errorf("PopupMenu::handleSelect - Could not find menu item position for ID %d ... this shouldn't happen!", command);
      return false;
   }

   char textbuf[1024];
   if(!text)
   {
      CFStringRef cfstr;
      CopyMenuItemTextAsCFString(mData->mMenu, pos, &cfstr);
      CFStringGetCString(cfstr,textbuf,sizeof(textbuf) - 1,kCFStringEncodingUTF8);
      CFRelease( cfstr );
      text = textbuf;
   }
   
   // [tom, 8/20/2006] Wasn't handled by a submenu, pass off to script
   return dAtob(Con::executef(this, "onSelectItem", Con::getIntArg(pos - 1), text ? text : ""));
}

//////////////////////////////////////////////////////////////////////////

void PopupMenu::showPopup(GuiCanvas* canvas, S32 x /* = -1 */, S32 y /* = -1 */)
{
   if(x < 0 || y < 0)
   {
      Point2I p = canvas->getCursorPos();
      x = p.x;
      y = p.y;
   }

   PopUpMenuSelect(mData->mMenu, y, x, 0);
}

//////////////////////////////////////////////////////////////////////////

void PopupMenu::attachToMenuBar(GuiCanvas* canvas, S32 pos, const char *title)
{
   CFStringRef cftitle = CFStringCreateWithCString(NULL,title,kCFStringEncodingUTF8);
   SetMenuTitleWithCFString(mData->mMenu, cftitle);
   CFRelease( cftitle );
   InsertMenu(mData->mMenu, pos);

   onAttachToMenuBar(canvas, pos, title);
}

void PopupMenu::removeFromMenuBar()
{
   DeleteMenu(mData->tag);
   
   onRemoveFromMenuBar(mCanvas);
}

U32 PopupMenu::getItemCount()
{
   return CountMenuItems( mData->mMenu );
}

bool PopupMenu::setItem(S32 pos, const char *title, const char *accelerator)
{
   //TODO: update accelerator?
   
   pos += 1; // Torque to mac index
   
   CFStringRef cftitle = CFStringCreateWithCString( NULL, title, kCFStringEncodingUTF8 );
   SetMenuItemTextWithCFString( mData->mMenu, pos, cftitle );
   CFRelease( cftitle );
   
   return true;
} 

S32 PopupMenu::getPosOnMenuBar()
{
   return -1;
}

void PopupMenu::attachToMenuBar(GuiCanvas *owner, S32 pos)
{

}
