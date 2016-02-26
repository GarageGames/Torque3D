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

//-----------------------------------------------------------------------------
// Menu Builder Helper Class
//-----------------------------------------------------------------------------
/// @class MenuBuilder 
/// @brief Create Dynamic Context and MenuBar Menus
///
///
/// Summary : The MenuBuilder script class exists merely as a helper for creating
///           popup menu's for use in torque editors.  It is setup as a single 
///           object with dynamic fields starting with item[0]..[n] that describe
///           how to create the menu in question.  An example is below.
///
/// isPopup : isPopup is a persistent field on PopupMenu console class which
///           when specified to true will allow you to perform .showPopup(x,y) 
///           commands which allow popupmenu's to be used/reused as menubar menus
///           as well as context menus.
///
/// barPosition : barPosition indicates which index on the menu bar (0 = leftmost)
///           to place this menu if it is attached.  Use the attachToMenuBar() command
///           to attach a menu.
///
/// barName : barName specifies the visible name of a menu item that is attached
///           to the global menubar.
///
/// canvas  : The GuiCanvas object the menu should be attached to. This defaults to
///           the global Canvas object if unspecified.
///
/// Remarks : If you wish to use a menu as a context popup menu, isPopup must be 
///           specified as true at the creation time of the menu.
///
/// 
/// @li @b item[n] (String) TAB (String) TAB (String) : <c>A Menu Item Definition.</c>
/// @code item[0] = "Open File..." TAB "Ctrl O" TAB "Something::OpenFile"; @endcode
///
/// @li @b isPopup (bool) : <c>If Specified the menu will be considered a popup menu and should be used via .showPopup()</c>
/// @code isPopup = true; @endcode
///
///
/// Example : Creating a @b MenuBar Menu
/// @code
/// %%editMenu = new PopupMenu()
/// {
///    barPosition = 3;
///    barName     = "View";
///    superClass = "MenuBuilder";
///    item[0] = "Undo" TAB "Ctrl Z" TAB "levelBuilderUndo(1);";
///    item[1] = "Redo" TAB "Ctrl Y" TAB "levelBuilderRedo(1);";
///    item[2] = "-";
/// };
///
/// %%editMenu.attachToMenuBar( 1, "Edit" );
///
/// @endcode
///
///
/// Example : Creating a @b Context (Popup) Menu
/// @code
/// %%contextMenu = new PopupMenu()
/// {
///    superClass = MenuBuilder;
///    isPopup    = true;
///    item[0] = "My Super Cool Item" TAB "Ctrl 2" TAB "echo(\"Clicked Super Cool Item\");";
///    item[1] = "-";
/// };
///
/// %%contextMenu.showPopup();
/// @endcode
///
///
/// Example : Modifying a Menu
/// @code
/// %%editMenu = new PopupMenu()
/// {
///    item[0] = "Foo" TAB "Ctrl F" TAB "echo(\"clicked Foo\")";
///    item[1] = "-";
/// };
/// %%editMenu.addItem( 2, "Bar" TAB "Ctrl B" TAB "echo(\"clicked Bar\")" );
/// %%editMenu.removeItem( 0 );
/// %%editMenu.addItem( 0, "Modified Foo" TAB "Ctrl F" TAB "echo(\"clicked modified Foo\")" );
/// @endcode
///
///
/// @see PopupMenu
///
//-----------------------------------------------------------------------------

// Adds one item to the menu.
// if %item is skipped or "", we will use %item[#], which was set when the menu was created.
// if %item is provided, then we update %item[#].
function MenuBuilder::addItem(%this, %pos, %item)
{
   if(%item $= "")
      %item = %this.item[%pos];
   
   if(%item !$= %this.item[%pos])
      %this.item[%pos] = %item;
   
   %name = getField(%item, 0);
   %accel = getField(%item, 1);
   %cmd = getField(%item, 2);
   
   // We replace the [this] token with our object ID
   %cmd = strreplace( %cmd, "[this]", %this );
   %this.item[%pos] = setField( %item, 2, %cmd );
   
   if(isObject(%accel))
   {
      // If %accel is an object, we want to add a sub menu
      %this.insertSubmenu(%pos, %name, %accel);
   }
   else
   {
      %this.insertItem(%pos, %name !$= "-" ? %name : "", %accel, %cmd);
   }
}

function MenuBuilder::appendItem(%this, %item)
{
   %this.addItem(%this.getItemCount(), %item);
}

function MenuBuilder::onAdd(%this)
{
   if(! isObject(%this.canvas))
      %this.canvas = Canvas;
      
   for(%i = 0;%this.item[%i] !$= "";%i++)
   {
      %this.addItem(%i);
   }
}

function MenuBuilder::onRemove(%this)
{
   %this.removeFromMenuBar();
}

//////////////////////////////////////////////////////////////////////////

function MenuBuilder::onSelectItem(%this, %id, %text)
{
   %cmd = getField(%this.item[%id], 2);
   if(%cmd !$= "")
   {
      eval( %cmd );
      return true;
   }
   return false;
}

/// Sets a new name on an existing menu item.
function MenuBuilder::setItemName( %this, %id, %name )
{
   %item = %this.item[%id];
   %accel = getField(%item, 1);
   %this.setItem( %id, %name, %accel );
}

/// Sets a new command on an existing menu item.
function MenuBuilder::setItemCommand( %this, %id, %command )
{
   %this.item[%id] = setField( %this.item[%id], 2, %command );
}

/// (SimID this)
/// Wraps the attachToMenuBar call so that it does not require knowledge of
/// barName or barIndex to be removed/attached.  This makes the individual 
/// MenuBuilder items very easy to add and remove dynamically from a bar.
///
function MenuBuilder::attachToMenuBar( %this )
{
   if( %this.barName $= "" )
   {
      error("MenuBuilder::attachToMenuBar - Menu property 'barName' not specified.");
      return false;
   }
   
   if( %this.barPosition < 0 )
   {
      error("MenuBuilder::attachToMenuBar - Menu " SPC %this.barName SPC "property 'barPosition' is invalid, must be zero or greater.");
      return false;
   }
   
   Parent::attachToMenuBar( %this, %this.canvas, %this.barPosition, %this.barName );
}

//////////////////////////////////////////////////////////////////////////

// Callbacks from PopupMenu. These callbacks are now passed on to submenus
// in C++, which was previously not the case. Thus, no longer anything to
// do in these. I am keeping the callbacks in case they are needed later.

function MenuBuilder::onAttachToMenuBar(%this, %canvas, %pos, %title)
{
}

function MenuBuilder::onRemoveFromMenuBar(%this, %canvas)
{
}

//////////////////////////////////////////////////////////////////////////

/// Method called to setup default state for the menu. Expected to be overriden
/// on an individual menu basis. See the mission editor for an example.
function MenuBuilder::setupDefaultState(%this)
{
   for(%i = 0;%this.item[%i] !$= "";%i++)
   {
      %name = getField(%this.item[%i], 0);
      %accel = getField(%this.item[%i], 1);
      %cmd = getField(%this.item[%i], 2);
      
      // Pass on to sub menus
      if(isObject(%accel))
         %accel.setupDefaultState();
   }
}

/// Method called to easily enable or disable all items in a menu.
function MenuBuilder::enableAllItems(%this, %enable)
{
   for(%i = 0; %this.item[%i] !$= ""; %i++)
   {
      %this.enableItem(%i, %enable);
   }
}
