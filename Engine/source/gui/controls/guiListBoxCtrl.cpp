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
#include "gui/controls/guiListBoxCtrl.h"
#include "gfx/gfxDrawUtil.h"
#include "console/engineAPI.h"

IMPLEMENT_CONOBJECT(GuiListBoxCtrl);

ConsoleDocClass( GuiListBoxCtrl,
   "@brief A list of text items.\n\n"

   "A list of text items where each individual entry can have its own text value, text color and associated SimObject.\n\n"

   "@tsexample\n"
   "new GuiListBoxCtrl(GuiMusicPlayerMusicList)\n"
   "{\n"
   "   allowMultipleSelections = \"true\";\n"
   "   fitParentWidth = \"true\";\n"
   "   mirrorSet = \"AnotherGuiListBoxCtrl\";\n"
   "   makeNameCallback = \"\";\n"
   "   colorBullet = \"1\";\n"
   "   //Properties not specific to this control have been omitted from this example.\n"
   "};\n"
   "@endtsexample\n\n"

   "@see GuiControl\n\n"

   "@ingroup GuiCore\n"
);

IMPLEMENT_CALLBACK( GuiListBoxCtrl, onMouseDragged, void, (),(),
   "@brief Called whenever the mouse is dragged across the control.\n\n"
   "@tsexample\n"
   "// Mouse is dragged across the control, causing the callback to occur.\n"
   "GuiListBoxCtrl::onMouseDragged(%this)\n"
   "  {\n"
   "     // Code to run whenever the mouse is dragged across the control\n"
   "  }\n"
   "@endtsexample\n\n"
   "@see GuiControl\n\n"
);

IMPLEMENT_CALLBACK( GuiListBoxCtrl, onClearSelection, void, (),(),
   "@brief Called whenever a selected item in the list is cleared.\n\n"
   "@tsexample\n"
   "// A selected item is cleared, causing the callback to occur.\n"
   "GuiListBoxCtrl::onClearSelection(%this)\n"
   "  {\n"
   "     // Code to run whenever a selected item is cleared\n"
   "  }\n"
   "@endtsexample\n\n"
   "@see GuiControl\n\n"
);

IMPLEMENT_CALLBACK( GuiListBoxCtrl, onUnSelect, void, ( S32 index, const char* itemText),( index, itemText ),
   "@brief Called whenever a selected item in the list has been unselected.\n\n"
   "@param index Index id of the item that was unselected\n"
   "@param itemText Text for the list entry at the index id that was unselected\n\n"
   "@tsexample\n"
   "// A selected item is unselected, causing the callback to occur\n"
   "GuiListBoxCtrl::onUnSelect(%this, %indexId, %itemText)\n"
   "  {\n"
   "     // Code to run whenever a selected list item is unselected\n"
   "  }\n"
   "@endtsexample\n\n"
   "@see GuiControl\n\n"
);

IMPLEMENT_CALLBACK( GuiListBoxCtrl, onSelect, void, ( S32 index , const char* itemText ),( index, itemText ),
   "@brief Called whenever an item in the list is selected.\n\n"
   "@param index Index id for the item in the list that was selected.\n"
   "@param itemText Text for the list item at the index that was selected.\n\n"
   "@tsexample\n"
   "// An item in the list is selected, causing the callback to occur\n"
   "GuiListBoxCtrl::onSelect(%this, %index, %itemText)\n"
   "  {\n"
   "     // Code to run whenever an item in the list is selected\n"
   "  }\n"
   "@endtsexample\n\n"
   "@see GuiControl\n\n"
);

IMPLEMENT_CALLBACK( GuiListBoxCtrl, onDoubleClick, void, (),(),
   "@brief Called whenever an item in the list has been double clicked.\n\n"
   "@tsexample\n"
   "// An item in the list is double clicked, causing the callback to occur.\n"
   "GuiListBoxCtrl::onDoubleClick(%this)\n"
   "  {\n"
   "     // Code to run whenever an item in the control has been double clicked\n"
   "  }\n"
   "@endtsexample\n\n"
   "@see GuiControl\n\n"
);

IMPLEMENT_CALLBACK( GuiListBoxCtrl, onMouseUp, void, ( S32 itemHit, S32 mouseClickCount ),( itemHit,mouseClickCount ),
   "@brief Called whenever the mouse has previously been clicked down (onMouseDown) and has now been raised on the control.\n"
   "If an item in the list was hit during the click cycle, then the index id of the clicked object along with how many clicks occured are passed\n"
   "into the callback.\n\n"
   "Detailed description\n\n"
   "@param itemHit Index id for the list item that was hit\n"
   "@param mouseClickCount How many mouse clicks occured on this list item\n\n"
   "@tsexample\n"
   "// Mouse was previously clicked down, and now has been released, causing the callback to occur.\n"
   "GuiListBoxCtrl::onMouseUp(%this, %itemHit, %mouseClickCount)\n"
   "  {\n"
   "     // Code to call whenever the mouse has been clicked and released on the control\n"
   "  }\n"
   "@endtsexample\n\n"
   "@see GuiControl\n\n"
);

IMPLEMENT_CALLBACK( GuiListBoxCtrl, onDeleteKey, void, (),(),
   "@brief Called whenever the Delete key on the keyboard has been pressed while in this control.\n\n"
   "@tsexample\n"
   "// The delete key on the keyboard has been pressed while this control is in focus, causing the callback to occur.\n"
   "GuiListBoxCtrl::onDeleteKey(%this)\n"
   "  {\n"
   "     // Code to call whenever the delete key is pressed\n"
   "  }\n"
   "@endtsexample\n\n"
   "@see GuiControl\n\n"
);

IMPLEMENT_CALLBACK( GuiListBoxCtrl, isObjectMirrored, bool, ( const char* indexIdString ),( indexIdString ),
   "@brief Checks if a list item at a defined index id is mirrored, and returns the result.\n\n"
   "@param indexIdString Index id of the list to check.\n"
   "@tsexample\n"
   "// Engine has requested of the script level to determine if a list entry is mirrored or not.\n"
   "GuiListBoxCtrl::isObjectMirrored(%this, %indexIdString)\n"
   "  {\n"
   "     // Perform code required to check and see if the list item at the index id is mirrored or not.\n"
   "     return %isMirrored;\n"
   "  }\n"
   "@endtsexample\n\n"
   "@return A boolean value on if the list item is mirrored or not.\n\n"
   "@see GuiControl\n\n"
);


GuiListBoxCtrl::GuiListBoxCtrl()
{
   mItems.clear();
   mSelectedItems.clear();
   mMultipleSelections = true;
   mFitParentWidth = true;
   mColorBullet = true;
   mItemSize = Point2I(10,20);
   mLastClickItem = NULL;
   
   mRenderTooltipDelegate.bind( this, &GuiListBoxCtrl::renderTooltip );
}

GuiListBoxCtrl::~GuiListBoxCtrl()
{
   clearItems();
}

void GuiListBoxCtrl::initPersistFields()
{
   addField( "allowMultipleSelections", TypeBool, Offset( mMultipleSelections, GuiListBoxCtrl), "If true, will allow the selection of multiple items in the listbox.\n");
   addField( "fitParentWidth", TypeBool, Offset( mFitParentWidth, GuiListBoxCtrl), "If true, the width of the listbox will match the width of its parent control.\n");
   addField( "colorBullet", TypeBool, Offset( mColorBullet, GuiListBoxCtrl), "If true, colored items will render a colored rectangular bullet next to the item text.\n");

   addField( "mirrorSet", TypeRealString, Offset( mMirrorSetName, GuiListBoxCtrl ), "If populated with the name of another GuiListBoxCtrl, then this list box will mirror the contents of the mirrorSet listbox.\n");
   addField( "makeNameCallback", TypeRealString, Offset( mMakeNameCallback, GuiListBoxCtrl ), "A script snippet to control what is displayed in the list for a SimObject. Within this snippet, $ThisControl is bound to the guiListBoxCtrl and $ThisObject to the contained object in question.\n");

   Parent::initPersistFields();
}

bool GuiListBoxCtrl::onWake()
{
   if( !Parent::onWake() )
      return false;

   updateSize();

   return true;
}

//-----------------------------------------------------------------------------
// Item Accessors
//-----------------------------------------------------------------------------

DefineEngineMethod( GuiListBoxCtrl, setMultipleSelection, void, (bool allowMultSelections),,
   "@brief Enable or disable multiple selections for this GuiListBoxCtrl object.\n\n"
   "@param allowMultSelections Boolean variable to set the use of multiple selections or not.\n"
   "@tsexample\n"
   "// Define the multiple selection use state.\n"
   "%allowMultSelections = \"true\";\n\n"
   "// Set the allow  multiple selection state on the GuiListBoxCtrl object.\n"
   "%thisGuiListBoxCtrl.setMultipleSelection(%allowMultSelections);\n"
   "@endtsexample\n\n"
   "@see GuiControl\n")
{
   object->setMultipleSelection( allowMultSelections );
}

DefineEngineMethod( GuiListBoxCtrl, clearItems, void, (),,
   "@brief Clears all the items in the listbox.\n\n"
   "@tsexample\n"
   "// Inform the GuiListBoxCtrl object to clear all items from its list.\n"
   "%thisGuiListBoxCtrl.clearItems();\n"
   "@endtsexample\n\n"
   "@see GuiControl")
{
   object->clearItems();
}

void GuiListBoxCtrl::clearItems()
{
   // Free item list allocated memory
   while( mItems.size() )
      deleteItem( 0 );

   // Free our vector lists
   mItems.clear();
   mSelectedItems.clear();
   mFilteredItems.clear();
}

DefineEngineMethod( GuiListBoxCtrl, clearSelection, void, (),,
   "@brief Sets all currently selected items to unselected.\n\n"
   "Detailed description\n\n"
   "@tsexample\n"
   "// Inform the GuiListBoxCtrl object to set all of its items to unselected./n"
   "%thisGuiListBoxCtrl.clearSelection();\n"
   "@endtsexample\n\n"
   "@see GuiControl")
{
   object->clearSelection();
}

void GuiListBoxCtrl::clearSelection()
{
   if( !mSelectedItems.size() )
      return;

   VectorPtr<LBItem*>::iterator i = mSelectedItems.begin();
   for( ; i != mSelectedItems.end(); i++ )
      (*i)->isSelected = false;

   mSelectedItems.clear();

   onClearSelection_callback();
}

DefineEngineMethod( GuiListBoxCtrl, setSelected, void, (S32 index, bool setSelected), (true),
   "@brief Sets the item at the index specified to selected or not.\n\n"
   "Detailed description\n\n"
   "@param index Item index to set selected or unselected.\n"
   "@param setSelected Boolean selection state to set the requested item index.\n"
   "@tsexample\n"
   "// Define the index\n"
   "%index = \"5\";\n\n"
   "// Define the selection state\n"
   "%selected = \"true\"\n\n"
   "// Inform the GuiListBoxCtrl object of the new selection state for the requested index entry.\n"
   "%thisGuiListBoxCtrl.setSelected(%index,%selected);\n"
   "@endtsexample\n\n"
   "@see GuiControl")
{
   if( setSelected == true )
      object->addSelection( index );
   else
      object->removeSelection( index );
}

void GuiListBoxCtrl::removeSelection( S32 index )
{
   // Range Check
   if( index >= mItems.size() || index < 0 )
   {
      Con::warnf("GuiListBoxCtrl::removeSelection - index out of range!" );
      return;
   }

   removeSelection( mItems[index], index );
}
void GuiListBoxCtrl::removeSelection( LBItem *item, S32 index )
{
   if( !mSelectedItems.size() )
      return;

   if( !item )
      return;

   for( S32 i = 0 ; i < mSelectedItems.size(); i++ )
   {
      if( mSelectedItems[i] == item )
      {
         mSelectedItems.erase( &mSelectedItems[i] );
         item->isSelected = false;
         onUnSelect_callback(index, item->itemText);
         return;
      }
   }
}

void GuiListBoxCtrl::addSelection( S32 index )
{
   // Range Check
   if( index >= mItems.size() || index < 0 )
   {
      Con::warnf("GuiListBoxCtrl::addSelection- index out of range!" );
      return;
   }

   addSelection( mItems[index], index );

}
void GuiListBoxCtrl::addSelection( LBItem *item, S32 index )
{
   if( !mMultipleSelections )
   {
      if( !mSelectedItems.empty() )
      {
         LBItem* selItem = mSelectedItems.front();
         if( selItem != item )
            clearSelection();
         else
            return;
      }
   }
   else
   {
      if( !mSelectedItems.empty() )
      {
         for( S32 i = 0; i < mSelectedItems.size(); i++ )
         {
            if( mSelectedItems[ i ] == item )
               return;
         }
      }
   }

   item->isSelected = true;
   mSelectedItems.push_front( item );

   onSelect_callback(index, item->itemText);
}

S32 GuiListBoxCtrl::getItemIndex( LBItem *item )
{
   if( mItems.empty() )
      return -1;

   // Lookup the index of an item in our list, by the pointer to the item
   for( S32 i = 0; i < mItems.size(); i++ )
      if( mItems[i] == item )
         return i;

   return -1;
}

DefineEngineMethod( GuiListBoxCtrl, getItemCount, S32, (),,
   "@brief Returns the number of items in the list.\n\n"
   "@tsexample\n"
   "// Request the number of items in the list of the GuiListBoxCtrl object.\n"
   "%listItemCount = %thisGuiListBoxCtrl.getItemCount();\n"
   "@endtsexample\n\n"
   "@return The number of items in the list.\n\n"
   "@see GuiControl")
{
   return object->getItemCount();
}

S32 GuiListBoxCtrl::getItemCount()
{
   return mItems.size();
}

DefineEngineMethod( GuiListBoxCtrl, getSelCount, S32, (),,
   "@brief Returns the number of items currently selected.\n\n"
   "@tsexample\n"
   "// Request the number of currently selected items\n"
   "%selectedItemCount = %thisGuiListBoxCtrl.getSelCount();\n"
   "@endtsexample\n\n"
   "@return Number of currently selected items.\n\n"
   "@see GuiControl")
{
   return object->getSelCount();
}

S32 GuiListBoxCtrl::getSelCount()
{
   return mSelectedItems.size();
}

DefineEngineMethod( GuiListBoxCtrl, getSelectedItem, S32, (),,
   "@brief Returns the selected items index or -1 if none selected. If multiple selections exist it returns the first selected item. \n\n"
   "@tsexample\n"
   "// Request the index id of the currently selected item\n"
   "%selectedItemId = %thisGuiListBoxCtrl.getSelectedItem();\n"
   "@endtsexample\n\n"
   "@return The selected items index or -1 if none selected.\n\n"
   "@see GuiControl")
{
   return object->getSelectedItem();
}

S32 GuiListBoxCtrl::getSelectedItem()
{
   if( mSelectedItems.empty() || mItems.empty() )
      return -1;

   for( S32 i = 0 ; i < mItems.size(); i++ )
      if( mItems[i]->isSelected )
         return i;

   return -1;
}

DefineEngineMethod( GuiListBoxCtrl, getSelectedItems, const char*, (),,
   "@brief Returns a space delimited list of the selected items indexes in the list.\n\n"
   "@tsexample\n"
   "// Request a space delimited list of the items in the GuiListBoxCtrl object.\n"
   "%selectionList = %thisGuiListBoxCtrl.getSelectedItems();\n"
   "@endtsexample\n\n"
   "@return Space delimited list of the selected items indexes in the list\n\n"
   "@see GuiControl")
{
   S32 selCount = object->getSelCount();
   if( selCount == -1 || selCount == 0 )
      return StringTable->lookup("-1");
   else if( selCount == 1 )
      return Con::getIntArg(object->getSelectedItem());

   Vector<S32> selItems;
   object->getSelectedItems( selItems );

   if( selItems.empty() )
      return StringTable->lookup("-1");

   static const U32 bufSize = selItems.size() * 4;
   UTF8 *retBuffer = Con::getReturnBuffer( bufSize );
   dMemset( retBuffer, 0, bufSize );
   Vector<S32>::iterator i = selItems.begin();
   for( ; i != selItems.end(); i++ )
   {
      UTF8 retFormat[12];
      dSprintf( retFormat, 12, "%d ", (*i) );
      dStrcat( retBuffer, retFormat );
   }

   return retBuffer;
}

void GuiListBoxCtrl::getSelectedItems( Vector<S32> &Items )
{
   // Clear our return vector
   Items.clear();
   
   // If there are no selected items, return an empty vector
   if( mSelectedItems.empty() )
      return;
   
   for( S32 i = 0; i < mItems.size(); i++ )
      if( mItems[i]->isSelected )
         Items.push_back( i );
}

DefineEngineMethod( GuiListBoxCtrl, findItemText, S32, (const char* findText, bool bCaseSensitive), (false),
   "@brief Returns index of item with matching text or -1 if none found.\n\n"
   "@param findText Text in the list to find.\n"
   "@param isCaseSensitive If true, the search will be case sensitive.\n"
   "@tsexample\n"
   "// Define the text we wish to find in the list.\n"
   "%findText = \"Hickory Smoked Gideon\"/n/n"
   "// Define if this is a case sensitive search or not.\n"
   "%isCaseSensitive = \"false\";\n\n"
   "// Ask the GuiListBoxCtrl object what item id in the list matches the requested text.\n"
   "%matchingId = %thisGuiListBoxCtrl.findItemText(%findText,%isCaseSensitive);\n"
   "@endtsexample\n\n"
   "@return Index id of item with matching text or -1 if none found.\n\n"
   "@see GuiControl")
{
   return object->findItemText( findText, bCaseSensitive );
}

S32 GuiListBoxCtrl::findItemText( StringTableEntry text, bool caseSensitive )
{
   // Check Proper Arguments
   if( !text || !text[0] || text == StringTable->lookup("") )
   {
      Con::warnf("GuiListBoxCtrl::findItemText - No Text Specified!");
      return -1;
   }

   // Check Items Exist.
   if( mItems.empty() )
      return -1;

   // Lookup the index of an item in our list, by the pointer to the item
   for( S32 i = 0; i < mItems.size(); i++ )
   {
      // Case Sensitive Compare?
      if( caseSensitive && ( dStrcmp( mItems[i]->itemText, text ) == 0 ) )
         return i;
      else if (!caseSensitive && ( dStricmp( mItems[i]->itemText, text ) == 0 ))
         return i;
   }

   // Not Found!
   return -1;
}

DefineEngineMethod( GuiListBoxCtrl, setCurSel, void, (S32 indexId),,
   "@brief Sets the currently selected item at the specified index.\n\n"
   "@param indexId Index Id to set selected.\n"
   "@tsexample\n"
   "// Define the index id that we wish to select.\n"
   "%selectId = \"4\";\n\n"
   "// Inform the GuiListBoxCtrl object to set the requested index as selected.\n"
   "%thisGuiListBoxCtrl.setCurSel(%selectId);\n"
   "@endtsexample\n\n"
   "@see GuiControl")
{
   object->setCurSel( indexId );
}
void GuiListBoxCtrl::setCurSel( S32 index )
{
   // Range Check
   if( index >= mItems.size() )
   {
      Con::warnf("GuiListBoxCtrl::setCurSel - index out of range!" );
      return;
   }

   // If index -1 is specified, we clear the selection
   if( index == -1 )
   {
      mSelectedItems.clear();
      return;
   }

   // Add the selection
   addSelection( mItems[ index ], index );

}

DefineEngineMethod( GuiListBoxCtrl, setCurSelRange, void, (S32 indexStart, S32 indexStop), (999999),
   "@brief Sets the current selection range from index start to stop. If no stop is specified it sets from start index to the end of the list\n\n"
   "@param indexStart Index Id to start selection.\n"
   "@param indexStop Index Id to end selection.\n"
   "@tsexample\n"
   "// Set start id\n"
   "%indexStart = \"3\";\n\n"
   "// Set end id\n"
   "%indexEnd = \"6\";\n\n"
   "// Request the GuiListBoxCtrl object to select the defined range.\n"
   "%thisGuiListBoxCtrl.setCurSelRange(%indexStart,%indexEnd);\n"
   "@endtsexample\n\n"
   "@see GuiControl")
{
   object->setCurSelRange( indexStart , indexStop );
}

void GuiListBoxCtrl::setCurSelRange( S32 start, S32 stop )
{
   // Verify Selection Range
   if( start < 0 )
      start = 0;
   else if( start > mItems.size() )
      start = mItems.size();

   if( stop < 0 )
      stop = 0;
   else if( stop > mItems.size() )
      stop = mItems.size();

   S32 iterStart = ( start < stop ) ? start : stop;
   S32 iterStop  = ( start < stop ) ? stop : start;

   for( ; iterStart <= iterStop; iterStart++ )
      addSelection( mItems[iterStart], iterStart );
}

DefineEngineMethod( GuiListBoxCtrl, addItem, S32, (const char* newItem, const char* color), ( "" ),
   "@brief Adds an item to the end of the list with an optional color.\n\n"
   "@param newItem New item to add to the list.\n"
   "@param color Optional color parameter to add to the new item.\n"
   "@tsexample\n"
   "// Define the item to add to the list.\n"
   "%newItem = \"Gideon's Blue Coat\";\n\n"
   "// Define the optional color for the new list item.\n"
   "%color = \"0.0 0.0 1.0\";\n\n"
   "// Inform the GuiListBoxCtrl object to add the item to the end of the list with the defined color.\n"
   "%thisGuiListBoxCtrl.addItem(%newItem,%color);\n"
   "@endtsexample\n\n"
   "@return If not void, return value and description\n\n"
   "@see GuiControl\n"
   "@hide")
{
   if(dStricmp(color,"") == 0)
   {
      return object->addItem( newItem );
   }
   else
   {
      U32 elementCount = GuiListBoxCtrl::getStringElementCount(color);

     if(elementCount == 3)
     {
         F32 red, green, blue;

         red = dAtof(GuiListBoxCtrl::getStringElement( color, 0 ));
         green = dAtof(GuiListBoxCtrl::getStringElement( color, 1 ));
         blue = dAtof(GuiListBoxCtrl::getStringElement( color, 2 ));

         return object->addItemWithColor( newItem, ColorF(red, green, blue) );
     }
     else if(elementCount == 1)
     {
         U32 objId = dAtoi( color );
         return object->addItem( newItem, (void*)objId );
     }
     else
     {
         Con::warnf("GuiListBoxCtrl::addItem() - Invalid number of parameters for the color!");
         return -1;
     }
   }
}

   static ConsoleDocFragment sGuiControlSetExtent1(
      "@brief Adds an item to the control with the specific text.\n\n"
      "@param text Text item to add to the list.\n"
      "GuiListBoxCtrl", // The class to place the method in; use NULL for functions.
      "void addItem( const char* text );" );


S32 GuiListBoxCtrl::addItem( StringTableEntry text, void *itemData )
{
   // This just calls insert item at the end of the list
   return insertItem( mItems.size(), text, itemData );
}

S32 GuiListBoxCtrl::addItemWithColor( StringTableEntry text, ColorF color, void *itemData )
{
   // This just calls insert item at the end of the list
   return insertItemWithColor( mItems.size(), text, color, itemData );
}

DefineEngineMethod( GuiListBoxCtrl, setItemColor, void, (S32 index, ColorF color),,
   "@brief Sets the color of a single list entry at the specified index id.\n\n"
   "@param index Index id to modify the color of in the list.\n"
   "@param color Color value to set the list entry to.\n"
   "@tsexample\n"
   "// Define the index id value\n"
   "%index = \"5\";\n\n"
   "// Define the color value\n"
   "%color = \"1.0 0.0 0.0\";\n\n"
   "// Inform the GuiListBoxCtrl object to change the color of the requested index\n"
   "%thisGuiListBoxCtrl.setItemColor(%index,%color);\n"
   "@endtsexample\n\n"
   "@see GuiControl")
{
   object->setItemColor( index, color );
}

void GuiListBoxCtrl::setItemColor(S32 index, const ColorF& color)
{
   if ((index >= mItems.size()) || index < 0)
   {
      Con::warnf("GuiListBoxCtrl::setItemColor - invalid index");
      return;
   }

   LBItem* item = mItems[index];
   item->hasColor = true;
   item->color = color;
}

DefineEngineMethod( GuiListBoxCtrl, clearItemColor, void, (S32 index),,
   "@brief Removes any custom coloring from an item at the defined index id in the list.\n\n"
   "@param index Index id for the item to clear any custom color from.\n"
   "@tsexample\n"
   "// Define the index id\n"
   "%index = \"4\";\n\n"
   "// Request the GuiListBoxCtrl object to remove any custom coloring from the defined index entry\n"
   "%thisGuiListBoxCtrl.clearItemColor(%index);\n"
   "@endtsexample\n\n"
   "@see GuiControl")
{
   object->clearItemColor(index);
}

void GuiListBoxCtrl::clearItemColor( S32 index )
{
   if ((index >= mItems.size()) || index < 0)
   {
      Con::warnf("GuiListBoxCtrl::setItemColor - invalid index");
      return;
   }

   LBItem* item = mItems[index];
   item->hasColor = false;
}

DefineEngineMethod( GuiListBoxCtrl, insertItem, void, (const char* text, S32 index),,
   "@brief Inserts an item into the list at the specified index and returns the index assigned or -1 on error.\n\n"
   "@param text Text item to add.\n"
   "@param index Index id to insert the list item text at.\n"
   "@tsexample\n"
   "// Define the text to insert\n"
   "%text = \"Secret Agent Gideon\";\n\n"
   "// Define the index entry to insert the text at\n"
   "%index = \"14\";\n\n"
   "// In form the GuiListBoxCtrl object to insert the text at the defined index.\n"
   "%assignedId = %thisGuiListBoxCtrl.insertItem(%text,%index);\n"
   "@endtsexample\n\n"
   "@return If successful will return the index id assigned. If unsuccessful, will return -1.\n\n"
   "@see GuiControl")
{
   object->insertItem( index, text );
}

S32 GuiListBoxCtrl::insertItem( S32 index, StringTableEntry text, void *itemData )
{
   // If the index is greater than our list size, insert it at the end
   if( index >= mItems.size() )
      index = mItems.size();

   // Sanity checking
   if( !text )
   {
      Con::warnf("GuiListBoxCtrl::insertItem - cannot add NULL string" );
      return -1;
   }

   LBItem *newItem = new LBItem;

   // Assign item data
   newItem->itemText    = StringTable->insert(text, true);
   newItem->itemData    = itemData;
   newItem->isSelected  = false;
   newItem->hasColor    = false;

   // Add to list
   mItems.insert(index);
   mItems[index] = newItem;

   // Resize our list to fit our items
   updateSize();

   // Return our index in list (last)
   return index;

}

S32 GuiListBoxCtrl::insertItemWithColor( S32 index, StringTableEntry text, ColorF color, void *itemData )
{
   // If the index is greater than our list size, insert it at the end
   if( index >= mItems.size() )
      index = mItems.size();

   // Sanity checking
   if( !text )
   {
      Con::warnf("GuiListBoxCtrl::insertItem - cannot add NULL string" );
      return -1;
   }

   if( color == ColorF(-1, -1, -1) )
   {
      Con::warnf("GuiListBoxCtrl::insertItem - cannot add NULL color" );
      return -1;
   }

   LBItem *newItem = new LBItem;

   // Assign item data
   newItem->itemText    = StringTable->insert(text, true);
   newItem->itemData    = itemData;
   newItem->isSelected  = false;
   newItem->hasColor    = true;
   newItem->color       = color;

   // Add to list
   mItems.insert(index);
   mItems[index] = newItem;

   // Resize our list to fit our items
   updateSize();

   // Return our index in list (last)
   return index;

}

DefineEngineMethod( GuiListBoxCtrl, deleteItem, void, (S32 itemIndex),,
   "@brief Removes the list entry at the requested index id from the control and clears the memory associated with it.\n\n"
   "@param itemIndex Index id location to remove the item from.\n"
   "@tsexample\n"
   "// Define the index id we want to remove from the list\n"
   "%itemIndex = \"8\";\n\n"
   "// Inform the GuiListBoxCtrl object to remove the item at the defined index id.\n"
   "%thisGuiListBoxCtrl.deleteItem(%itemIndex);\n"
   "@endtsexample\n\n"
   "@see References")
{
   object->deleteItem( itemIndex );
}

void  GuiListBoxCtrl::deleteItem( S32 index )
{
   // Range Check
   if( index >= mItems.size() || index < 0 )
   {
      Con::warnf("GuiListBoxCtrl::deleteItem - index out of range!" );
      return;
   }

   // Grab our item
   LBItem* item = mItems[ index ];
   if( !item )
   {
      Con::warnf("GuiListBoxCtrl::deleteItem - Bad Item Data!" );
      return;
   }

   // Remove it from the selected list.
   if( item->isSelected )
   {
      for( VectorPtr<LBItem*>::iterator i = mSelectedItems.begin(); i != mSelectedItems.end(); i++ )
      {
         if( item == *i )
         {
            mSelectedItems.erase_fast( i );
            break;
         }
      }
   }

   // Remove it from the list
   mItems.erase( &mItems[ index ] );

   // Free the memory associated with it
   delete item;
}

DefineEngineMethod( GuiListBoxCtrl, getItemText, const char*, (S32 index),,
   "@brief Returns the text of the item at the specified index.\n\n"
   "@param index Index id to return the item text from.\n"
   "@tsexample\n"
   "// Define the index id entry to request the text from\n"
   "%index = \"12\";\n\n"
   "// Request the item id text from the GuiListBoxCtrl object.\n"
   "%text = %thisGuiListBoxCtrl.getItemText(%index);\n"
   "@endtsexample\n\n"
   "@return The text of the requested index id.\n\n"
   "@see GuiControl")
{
   return object->getItemText( index );
}

StringTableEntry GuiListBoxCtrl::getItemText( S32 index )
{
   // Range Checking
   if( index > mItems.size() || index < 0 )
   {
      Con::warnf( "GuiListBoxCtrl::getItemText - index out of range!" );
      return StringTable->lookup("");
   }
   
   return mItems[ index ]->itemText;   
}

DefineEngineMethod( GuiListBoxCtrl, getItemObject,  const char*, (S32 index),,
   "@brief Returns the object associated with an item. This only makes sense if you are mirroring a simset.\n\n"
   "@param index Index id to request the associated item from.\n"
   "@tsexample\n"
   "// Define the index id\n"
   "%index = \"12\";\n\n"
   "// Request the item from the GuiListBoxCtrl object\n"
   "%object = %thisGuiListBoxCtrl.getItemObject(%index);\n"
   "@endtsexample\n\n"
   "@return The object associated with the item in the list.\n\n"
   "@see References")
{
   SimObject *outObj = object->getItemObject( index );
   if ( !outObj )
      return NULL;

   return outObj->getIdString();
}

SimObject* GuiListBoxCtrl::getItemObject( S32 index )
{
   // Range Checking
   if( index > mItems.size() || index < 0 )
   {
      Con::warnf( "GuiListBoxCtrl::getItemObject - index out of range!" );
      return NULL;
   }

   SimObject *outObj;
   Sim::findObject( (SimObjectId)(uintptr_t)(mItems[ index ]->itemData), outObj );

   return outObj;   
}

DefineEngineMethod( GuiListBoxCtrl, setItemText, void, (S32 index, const char* newtext),,
   "@brief Sets the items text at the specified index.\n\n"
   "@param index Index id to set the item text at.\n"
   "@param newtext Text to change the list item at index id to.\n"
   "@tsexample\n"
   "// Define the index id/n"
   "%index = \"12\";\n\n"
   "// Define the text to set the list item to\n"
   "%newtext = \"Gideon's Fancy Goggles\";\n\n"
   "// Inform the GuiListBoxCtrl object to change the text at the requested index\n"
   "%thisGuiListBoxCtrl.setItemText(%index,%newText);\n"
   "@endtsexample\n\n"
   "@see GuiControl")
{
   object->setItemText(index, newtext );
}

void GuiListBoxCtrl::setItemText( S32 index, StringTableEntry text )
{
   // Sanity Checking
   if( !text )
   {
      Con::warnf("GuiListBoxCtrl::setItemText - Invalid Text Specified!" );
      return;
   }
   // Range Checking
   if( index > mItems.size() || index < 0 )
   {
      Con::warnf( "GuiListBoxCtrl::getItemText - index out of range!" );
      return;
   }

   mItems[ index ]->itemText = StringTable->insert( text, true );
}

DefineEngineMethod( GuiListBoxCtrl, setItemTooltip, void, (S32 index, const char* text),,
   "@brief Set the tooltip text to display for the given list item.\n\n"
   "@param index Index id to change the tooltip text\n"
   "@param text Text for the tooltip.\n"
   "@tsexample\n"
   "// Define the index id\n"
   "%index = \"12\";\n\n"
   "// Define the tooltip text\n"
   "%tooltip = \"Gideon's goggles can see through space and time.\"\n\n"
   "// Inform the GuiListBoxCtrl object to set the tooltop for the item at the defined index id\n"
   "%thisGuiListBoxCtrl.setItemToolTip(%index,%tooltip);\n"
   "@endtsexample\n\n"
   "@see GuiControl")
{
   if( index > object->mItems.size() || index < 0 )
   {
      Con::errorf( "GuiListBoxCtrl::setItemTooltip - index '%i' out of range", index );
      return;
   }
   
   object->mItems[ index ]->itemTooltip = text;
}

DefineEngineMethod( GuiListBoxCtrl, getLastClickItem, S32, (),,
   "@brief Request the item index for the item that was last clicked.\n\n"
   "@tsexample\n"
   "// Request the item index for the last clicked item in the list\n"
   "%lastClickedIndex = %thisGuiListBoxCtrl.getLastClickItem();\n"
   "@endtsexample\n\n"
   "@return Index id for the last clicked item in the list.\n\n"
   "@see GuiControl")
{
   GuiListBoxCtrl::LBItem *lastItem = object->mLastClickItem;
   if ( !lastItem )
      return -1;

   return object->getItemIndex( lastItem );
}

//-----------------------------------------------------------------------------
// Sizing Functions
//-----------------------------------------------------------------------------
void GuiListBoxCtrl::updateSize()
{
   if( !mProfile || !mProfile->mFont )
      return;

   GFont *font = mProfile->mFont;
   GuiScrollCtrl* parent = dynamic_cast<GuiScrollCtrl *>(getParent());

   if ( mFitParentWidth && parent )
      mItemSize.x = parent->getContentExtent().x;
   else
   {
      // Find the maximum width cell:
      S32 maxWidth = 1;
      for ( U32 i = 0; i < mItems.size(); i++ )
      {
         S32 width = font->getStrWidth( mItems[i]->itemText );
         if( width > maxWidth )
            maxWidth = width;
      }
      mItemSize.x = maxWidth + 6;
   }

   mItemSize.y = font->getHeight() + 2;

   Point2I newExtent( mItemSize.x, mItemSize.y * mItems.size() );
   setExtent( newExtent );

}

void GuiListBoxCtrl::parentResized(const RectI &oldParentRect, const RectI &newParentRect)
{
   Parent::parentResized( oldParentRect, newParentRect );

   updateSize();
}

//-----------------------------------------------------------------------------
// Overrides
//-----------------------------------------------------------------------------
void GuiListBoxCtrl::onRender( Point2I offset, const RectI &updateRect )
{
   RectI clipRect(updateRect.point, updateRect.extent);

   if( !mProfile )
      return;

   _mirror();

   // Save our original clip rect
   RectI oldClipRect = clipRect;

   for ( S32 i = 0; i < mItems.size(); i++)
   {
      S32 colorBoxSize = 0;
      ColorI boxColor = ColorI(0, 0, 0);
      // Only render visible items
      if ((i + 1) * mItemSize.y + offset.y < updateRect.point.y)
         continue;

      // Break our once we're no longer in visible item range
      if( i * mItemSize.y + offset.y >= updateRect.point.y + updateRect.extent.y)
         break;

      // Render color box if needed
      if(mColorBullet && mItems[i]->hasColor)
      {
         // Set the size of the color box to be drawn next to the item text
         colorBoxSize = 3;
         boxColor = ColorI(mItems[i]->color);
         // Draw the box first
         ColorI black = ColorI(0, 0, 0);
         drawBox(  Point2I(offset.x + mProfile->mTextOffset.x + colorBoxSize, offset.y + ( i * mItemSize.y ) + 8), colorBoxSize, black, boxColor );
      }

      RectI itemRect = RectI( offset.x + mProfile->mTextOffset.x + (colorBoxSize * 3), offset.y + ( i * mItemSize.y ), mItemSize.x, mItemSize.y );

      // Render our item
      onRenderItem( itemRect, mItems[i] );
   }

   GFX->setClipRect( oldClipRect );
}

void GuiListBoxCtrl::onRenderItem(const RectI& itemRect, LBItem *item)
{
   if( item->isSelected )
      GFX->getDrawUtil()->drawRectFill( itemRect, mProfile->mFillColorSEL );

   GFX->getDrawUtil()->setBitmapModulation( item->hasColor ? (ColorI)item->color : mProfile->mFontColor);
   renderJustifiedText(itemRect.point + Point2I( 2, 0 ), itemRect.extent, item->itemText);
}

void GuiListBoxCtrl::drawBox(const Point2I &box, S32 size, ColorI &outlineColor, ColorI &boxColor)
{
   RectI r(box.x - size, box.y - size, 2 * size + 1, 2 * size + 1);
   r.inset(1, 1);
   GFX->getDrawUtil()->drawRectFill(r, boxColor);
   r.inset(-1, -1);
   GFX->getDrawUtil()->drawRect(r, outlineColor);
}

bool GuiListBoxCtrl::renderTooltip( const Point2I &hoverPos, const Point2I& cursorPos, const char* tipText )
{
   S32 hitItemIndex;
   if( hitTest( hoverPos, hitItemIndex ) )
      tipText = mItems[ hitItemIndex ]->itemTooltip;
      
   return defaultTooltipRender( hoverPos, cursorPos, tipText );
}

//-----------------------------------------------------------------------------
// Hit Detection
//-----------------------------------------------------------------------------

bool GuiListBoxCtrl::hitTest( const Point2I& point, S32& outItem )
{
   Point2I localPoint = globalToLocalCoord( point );
   
   S32 itemHit = ( localPoint.y < 0 ) ? -1 : (S32)mFloor( (F32)localPoint.y / (F32)mItemSize.y );
   if ( itemHit >= mItems.size() || itemHit == -1 )
      return false;

   LBItem *hitItem = mItems[ itemHit ];
   if ( hitItem == NULL )
      return false;
      
   outItem = itemHit;
   return true;
}

//-----------------------------------------------------------------------------
// Mouse Events
//-----------------------------------------------------------------------------

void GuiListBoxCtrl::onMouseDragged(const GuiEvent &event)
{
   Parent::onMouseDragged(event);

   onMouseDragged_callback();
}

void GuiListBoxCtrl::onMouseDown( const GuiEvent &event )
{
   S32 itemHit;
   if( !hitTest( event.mousePoint, itemHit ) )
      return;
      
   LBItem* hitItem = mItems[ itemHit ];

   // If we're not a multiple selection listbox, we simply select/unselect an item
   if( !mMultipleSelections )
   {
      // No current selection?  Just select the cell and move on
      S32 selItem = getSelectedItem();

      if ( selItem != itemHit && selItem != -1 )
         clearSelection();

      // Set the current selection
      setCurSel( itemHit );

      if( itemHit == selItem && event.mouseClickCount == 2 )
         onDoubleClick_callback();

      // Store the clicked item
      mLastClickItem = hitItem;

      // Evaluate the console command if we clicked the same item twice
      if( selItem == itemHit && event.mouseClickCount > 1 )
         execAltConsoleCallback();

      return;

   }
   
   // Deal with multiple selections
   if( event.modifier & SI_MULTISELECT)
   {
      // Ctrl-Click toggles selection
      if( hitItem->isSelected )
      {
         removeSelection( hitItem, itemHit );

         // We return here when we deselect an item because we don't store last clicked when we deselect
         return;
      }
      else
         addSelection( hitItem, itemHit );
   }
   else if( event.modifier & SI_RANGESELECT )
   {
      if( !mLastClickItem )
         addSelection( hitItem, itemHit );
      else
         setCurSelRange( getItemIndex( mLastClickItem ), itemHit );
   }
   else
   {
      if( getSelCount() != 0 )
      {
         S32 selItem = getSelectedItem();
         if( selItem != -1 && mItems[selItem] != hitItem )
            clearSelection();
      }
      addSelection( hitItem, itemHit );
   }

   if( hitItem == mLastClickItem && event.mouseClickCount == 2 )
      onDoubleClick_callback();

   mLastClickItem = hitItem;
}

void GuiListBoxCtrl::onMouseUp( const GuiEvent& event )
{
   S32 itemHit = -1;
   if( hitTest( event.mousePoint, itemHit ) )
      onMouseUp_callback( itemHit, event.mouseClickCount );

   // Execute console command
   execConsoleCallback();
   
   Parent::onMouseUp( event );
}

bool GuiListBoxCtrl::onKeyDown( const GuiEvent &event )
{
   if ( event.keyCode == KEY_DELETE )
   {
      onDeleteKey_callback();
      return true;
   }

   return Parent::onKeyDown( event );
}

U32 GuiListBoxCtrl::getStringElementCount( const char* inString )
{
    // Non-whitespace chars.
    static const char* set = " \t\n";

    // End of string.
    if ( *inString == 0 )
        return 0;

    U32 wordCount = 0;
    U8 search = 0;

    // Search String.
    while( *inString )
    {
        // Get string element.
        search = *inString;

        // End of string?
        if ( search == 0 )
            break;

        // Move to next element.
        inString++;

        // Search for seperators.
        for( U32 i = 0; set[i]; i++ )
        {
            // Found one?
            if( search == set[i] )
            {
                // Yes...
                search = 0;
                break;
            }   
        }

        // Found a seperator?
        if ( search == 0 )
            continue;

        // We've found a non-seperator.
        wordCount++;

        // Search for end of non-seperator.
        while( 1 )
        {
            // Get string element.
            search = *inString;

            // End of string?
            if ( search == 0 )
                break;

            // Move to next element.
            inString++;

            // Search for seperators.
            for( U32 i = 0; set[i]; i++ )
            {
                // Found one?
                if( search == set[i] )
                {
                    // Yes...
                    search = 0;
                    break;
                }   
            }

            // Found Seperator?
            if ( search == 0 )
                break;
        }

        // End of string?
        if ( *inString == 0 )
        {
            // Bah!
            break;
        }
    }

    // We've finished.
    return wordCount;
}

//------------------------------------------------------------------------------
// Get String Element.
//------------------------------------------------------------------------------
const char* GuiListBoxCtrl::getStringElement( const char* inString, const U32 index )
{
    // Non-whitespace chars.
    static const char* set = " \t\n";

    U32 wordCount = 0;
    U8 search = 0;
    const char* pWordStart = NULL;

    // End of string?
    if ( *inString != 0 )
    {
        // No, so search string.
        while( *inString )
        {
            // Get string element.
            search = *inString;

            // End of string?
            if ( search == 0 )
                break;

            // Move to next element.
            inString++;

            // Search for seperators.
            for( U32 i = 0; set[i]; i++ )
            {
                // Found one?
                if( search == set[i] )
                {
                    // Yes...
                    search = 0;
                    break;
                }   
            }

            // Found a seperator?
            if ( search == 0 )
                continue;

            // Found are word?
            if ( wordCount == index )
            {
                // Yes, so mark it.
                pWordStart = inString-1;
            }

            // We've found a non-seperator.
            wordCount++;

            // Search for end of non-seperator.
            while( 1 )
            {
                // Get string element.
                search = *inString;

                // End of string?
                if ( search == 0 )
                    break;

                // Move to next element.
                inString++;

                // Search for seperators.
                for( U32 i = 0; set[i]; i++ )
                {
                    // Found one?
                    if( search == set[i] )
                    {
                        // Yes...
                        search = 0;
                        break;
                    }   
                }

                // Found Seperator?
                if ( search == 0 )
                    break;
            }

            // Have we found our word?
            if ( pWordStart )
            {
                // Yes, so we've got our word...

                // Result Buffer.
                static char buffer[4096];

                // Calculate word length.
                const U32 length = inString - pWordStart - ((*inString)?1:0);

                // Copy Word.
                dStrncpy( buffer, pWordStart, length);
                buffer[length] = '\0';

                // Return Word.
                return buffer;
            }

            // End of string?
            if ( *inString == 0 )
            {
                // Bah!
                break;
            }
        }
    }

    // Sanity!
    AssertFatal( false, "t2dSceneObject::getStringElement() - Couldn't find specified string element!" );
    // Didn't find it
    return " ";
}

void GuiListBoxCtrl::_mirror()
{
   SimSet *mirrorSet;
   if ( !Sim::findObject( mMirrorSetName, mirrorSet ) )
      return;

   // Allow script to filter out objects if desired.

   Vector<SimObjectId> workingSet;   

   // If the method is not defined we assume user wants us to add
   // all objects.
   bool isObjMirroredDefined = isMethod( "isObjectMirrored" );
   
   for ( S32 i = 0; i < mirrorSet->size(); i++ )
   {
      bool addObj = true;
        
      if ( isObjMirroredDefined )
         addObj = isObjectMirrored_callback(mirrorSet->at(i)->getIdString()); 
        
      if ( addObj )
         workingSet.push_back( mirrorSet->at(i)->getId() );
   }


   // Remove existing items that are no longer in the SimSet.
   // Or are no longer valid objects.

   SimObjectId curId;
   SimObject *curObj;

   for ( U32 i = 0; i < mItems.size(); i++ )
   {
      curId = (SimObjectId)(uintptr_t)mItems[i]->itemData;

      Sim::findObject( curId, curObj );

      bool keep = false;

      if ( curObj )
      {
         if ( workingSet.contains( curId ) )
         {         
            mItems[i]->itemText = _makeMirrorItemName( curObj );            
            keep = true;
         }
      }

      if ( !keep )
      {
         deleteItem( i );
         i--;
      }
   }   


   // Add items that are in the SimSet but not yet in the list.

   for ( U32 i = 0; i < workingSet.size(); i++ )
   {
      curId = workingSet[i];
      Sim::findObject( curId, curObj );

      bool found = false;

      for ( U32 j = 0; j < mItems.size(); j++ )
      {
         if ( (SimObjectId)(uintptr_t)(mItems[j]->itemData) == curId )
         {
            found = true;
            break;
         }
      }
      
      for ( U32 j = 0; j < mFilteredItems.size(); j++ )
      {
         if ( (SimObjectId)(uintptr_t)(mFilteredItems[j]->itemData) == curId )
         {
            found = true;
            break;
         }
      }

      if ( !found )
      {                  
         addItem( _makeMirrorItemName( curObj ), (void*)curId );         
      }
   }
}

StringTableEntry GuiListBoxCtrl::_makeMirrorItemName( SimObject *inObj )
{
   StringTableEntry outName = StringTable->EmptyString();

   if ( mMakeNameCallback.isNotEmpty() )
   {
      Con::setIntVariable( "$ThisControl", getId() );
      Con::setIntVariable( "$ThisObject", inObj->getId() );
      
      outName = StringTable->insert( Con::evaluate( mMakeNameCallback ), true );      
   }
   else if ( inObj->getName() )
      outName = StringTable->insert( inObj->getName() );
   
   if ( !outName || !outName[0] )
      outName = StringTable->insert( "(no name)" );

   return outName;
}

DefineEngineMethod( GuiListBoxCtrl, doMirror, void, (),,
   "@brief Informs the GuiListBoxCtrl object to mirror the contents of the GuiListBoxCtrl stored in the mirrorSet field.\n\n"
   "@tsexample\n"
   "\\ Inform the object to mirror the object located at %thisGuiListBox.mirrorSet\n"
   "%thisGuiListBox.doMirror();\n"
   "@endtsexample\n\n"
   "@see GuiCore")
{
   object->_mirror();
}

DefineEngineMethod( GuiListBoxCtrl, addFilteredItem, void, (const char* newItem),,
   "@brief Checks if there is an item with the exact text of what is passed in, and if so\n"
   "the item is removed from the list and adds that item's data to the filtered list.\n\n"
   "@param itemName Name of the item that we wish to add to the filtered item list of the GuiListBoxCtrl.\n"
   "@tsexample\n"
   "// Define the itemName that we wish to add to the filtered item list.\n"
   "%itemName = \"This Item Name\";\n\n"
   "// Add the item name to the filtered item list.\n"
   "%thisGuiListBoxCtrl.addFilteredItem(%filteredItemName);\n"
   "@endtsexample\n\n"
   "@see GuiControl")
{
   String item(newItem);
   if( item == String::EmptyString )
      return;

   object->addFilteredItem( item );
}

void GuiListBoxCtrl::addFilteredItem( String item )
{
   // Delete from selected items list
   for ( S32 i = 0; i < mSelectedItems.size(); i++ ) 
   {
      String itemText = mSelectedItems[i]->itemText;
      if ( dStrcmp( itemText.c_str(), item.c_str() ) == 0 ) 
      {
         mSelectedItems.erase_fast( i );
         break;
      }
   }

   for ( S32 i = 0; i < mItems.size(); i++ ) 
   {
      String itemText = mItems[i]->itemText;
      if( dStrcmp( itemText.c_str(), item.c_str() ) == 0 )
      {  
         mItems[i]->isSelected = false;      
         mFilteredItems.push_front( mItems[i] );
         mItems.erase( &mItems[i] );
         break;
      }
   }
}

DefineEngineMethod( GuiListBoxCtrl, removeFilteredItem, void, ( const char* itemName ),,
   "@brief Removes an item of the entered name from the filtered items list.\n\n"
   "@param itemName Name of the item to remove from the filtered list.\n"
   "@tsexample\n"
   "// Define the itemName that you wish to remove.\n"
   "%itemName = \"This Item Name\";\n\n"
   "// Remove the itemName from the GuiListBoxCtrl\n"
   "%thisGuiListBoxCtrl.removeFilteredItem(%itemName);\n"
   "@endtsexample\n\n"
   "@see GuiControl")
{
   String item(itemName);
   if( item == String::EmptyString )
      return;

   object->removeFilteredItem( item );
}

void GuiListBoxCtrl::removeFilteredItem( String item )
{
   for ( S32 i = 0; i < mFilteredItems.size(); i++ ) 
   {
      String itemText = mFilteredItems[i]->itemText;
      if( dStrcmp( itemText.c_str(), item.c_str() ) == 0 )
      {        
         mItems.push_front( mFilteredItems[i] );
         mFilteredItems.erase( &mFilteredItems[i] );
         break;
      }
   }
}
