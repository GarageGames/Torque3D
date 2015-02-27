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
#include "gui/controls/GuiIconListCtrl.h"

#include "console/consoleTypes.h"
#include "console/console.h"
#include "gui/containers/guiScrollCtrl.h"
#include "gui/core/guiDefaultControlRender.h"
#include "gfx/gfxDrawUtil.h"
#include "console/engineAPI.h"

IMPLEMENT_CONOBJECT(GuiIconListCtrl);

ConsoleDocClass( GuiIconListCtrl,
   "@brief GUI control that displays a list of text. Text items in the list can be individually selected.\n\n"

   "@tsexample\n"

   "      new GuiIconListCtrl(EndGameGuiList)\n"
   "		{\n"
   "			columns = \"0 256\";\n"
   "	        fitParentWidth = \"1\";\n"
   "			clipColumnText = \"0\";\n"
   "		    //Properties not specific to this control have been omitted from this example.\n"
   "		};\n"
   "@endtsexample\n\n"

   "@see Reference\n\n"

   "@ingroup GuiControls\n"
);


IMPLEMENT_CALLBACK( GuiIconListCtrl, onSelect, void, (const char* cellid, const char* text),( cellid , text ),
   "@brief Called whenever an item in the list is selected.\n\n"
   "@param cellid The ID of the cell that was selected\n"
   "@param text The text in the selected cel\n\n"
   "@tsexample\n"
   "// A cel in the control was selected, causing the callback to occur\n"
   "GuiIconListCtrl::onSelect(%this,%callid,%text)\n"
   "	{\n"
   "		// Code to run when a cel item is selected\n"
   "	}\n"
   "@endtsexample\n\n"
   "@see GuiControl\n\n"
);

IMPLEMENT_CALLBACK( GuiIconListCtrl, onDeleteKey, void, ( const char* id ),( id ),
   "@brief Called when the delete key has been pressed.\n\n"
   "@param id Id of the selected item in the list\n"
   "@tsexample\n"
   "// The delete key was pressed while the GuiIconListCtrl was in focus, causing the callback to occur.\n"
   "GuiIconListCtrl::onDeleteKey(%this,%id)\n"
   "	{\n"
   "		// Code to run when the delete key is pressed\n"
   "	}\n"
   "@endtsexample\n\n"
   "@see GuiControl\n\n"
);

static int sortColumn;
static bool sIncreasing;

static const char *getColumn(const char *text)
{
   int ct = sortColumn;
   while(ct--)
   {
      text = dStrchr(text, '\t');
      if(!text)
         return "";
      text++;
   }
   return text;
}

static S32 QSORT_CALLBACK textCompare( const void* a, const void* b )
{
   GuiIconListCtrl::Entry *ea = (GuiIconListCtrl::Entry *) (a);
   GuiIconListCtrl::Entry *eb = (GuiIconListCtrl::Entry *) (b);
   S32 result = dStrnatcasecmp( getColumn( ea->iconPath ), getColumn( eb->iconPath ) );
   return ( sIncreasing ? result : -result );
}

static S32 QSORT_CALLBACK numCompare(const void *a,const void *b)
{
   GuiIconListCtrl::Entry *ea = (GuiIconListCtrl::Entry *) (a);
   GuiIconListCtrl::Entry *eb = (GuiIconListCtrl::Entry *) (b);
   const char* aCol = getColumn( ea->iconPath );
   const char* bCol = getColumn( eb->iconPath );
   F32 result = dAtof(aCol) - dAtof(bCol);
   S32 res = result < 0 ? -1 : (result > 0 ? 1 : 0);

   return ( sIncreasing ? res : -res );
}

GuiIconListCtrl::GuiIconListCtrl()
{
   VECTOR_SET_ASSOCIATION(mList);
   VECTOR_SET_ASSOCIATION(mColumnOffsets);

   mActive = true;
   mSize.set(1, 0);
   mColumnOffsets.push_back(0);
   mFitParentWidth = true;
   mClipColumnText = false;
   mIconBounds = RectI::Zero;
}

void GuiIconListCtrl::initPersistFields()
{
   addField("columns",                 TypeS32Vector, Offset(mColumnOffsets, GuiIconListCtrl), "A vector of column offsets.  The number of values determines the number of columns in the table.\n" );
   addField("fitParentWidth",          TypeBool, Offset(mFitParentWidth, GuiIconListCtrl), "If true, the width of this control will match the width of its parent.\n");
   addField("clipColumnText",          TypeBool, Offset(mClipColumnText, GuiIconListCtrl), "If true, text exceeding a column's given width will get clipped.\n" );
   addField("mIconBounds",			   TypeRectI, Offset(mIconBounds, GuiIconListCtrl), "size and relative offset of icons\n" );
   Parent::initPersistFields();
}

bool GuiIconListCtrl::onWake()
{
   if(!Parent::onWake())
      return false;

   setSize(mSize);
   return true;
}

U32 GuiIconListCtrl::getSelectedId()
{
   if (mSelectedCell.y == -1)
      return InvalidId;

   return mList[mSelectedCell.y].id;
}

U32 GuiIconListCtrl::getSelectedRow()
{
   return mSelectedCell.y;
}

bool GuiIconListCtrl::cellSelected(Point2I cell)
{
   // Is the selection being cleared?
   if( cell.x == -1 && cell.y == -1)
      return Parent::cellSelected(cell);

   // Do not allow selection of inactive cells
   if (cell.y >= 0 && cell.y < mSize.y && mList[cell.y].active)
      return Parent::cellSelected(cell);
   else
      return false;
}

void GuiIconListCtrl::onCellSelected(Point2I cell)
{
   if (selectEvent.valid())
      selectEvent(this, mList[cell.y].id, mList[cell.y].iconPath);
   onSelect_callback(Con::getIntArg(mList[cell.y].id), mList[cell.y].iconPath);
   execConsoleCallback();
}

U32 GuiIconListCtrl::getRowWidth(Entry *row)
{
   U32 width = 1;
   const char* text = row->iconPath.c_str();
   for(U32 index = 0; index < mColumnOffsets.size(); index++)
   {
      const char *nextCol = dStrchr(text, '\t');
      U32 textWidth;
      if(nextCol)
         textWidth = mFont->getStrNWidth((const UTF8*)text, nextCol - text);
      else
         textWidth = mFont->getStrWidth((const UTF8*)text);
      if(mColumnOffsets[index] >= 0)
         width = getMax(width, mColumnOffsets[index] + textWidth);
      if(!nextCol)
         break;
      text = nextCol+1;
   }
   return width;
}

void GuiIconListCtrl::insertEntry(U32 id, const String& iconPath, S32 index)
{
   Entry e;
   e.iconPath = iconPath;
   e.mTextureObject.set( iconPath, &GFXDefaultGUIProfile, avar("%s() - GuiIconListCtrl::Entry(mTextureObject %d)", __FUNCTION__, __LINE__) );
   e.id = id;
   e.active = true;
   if(!mList.size())
      mList.push_back(e);
   else
   {
      if(index > mList.size())
         index = mList.size();
      mList.insert(index);
      mList[index] = e;
   }
   setSize(Point2I(1, mList.size()));
}

void GuiIconListCtrl::addEntry(U32 id, const String& iconPath)
{
	Entry e;
	e.iconPath = iconPath;
	e.mTextureObject.set( iconPath, &GFXDefaultGUIProfile, avar("%s() - GuiIconListCtrl::Entry(mTextureObject %d)", __FUNCTION__, __LINE__) );
	e.id = id;
	e.active = true;
	mList.push_back(e);
	setSize(Point2I(1, mList.size()));
}

void GuiIconListCtrl::setEntry(U32 id, const String& iconPath)
{
   S32 e = findEntryById(id);
   if(e == -1)
      addEntry(id, iconPath);
   else
   {
	  mList[e].iconPath = iconPath;
	  mList[e].mTextureObject.set( iconPath, &GFXDefaultGUIProfile, avar("%s() - GuiIconListCtrl::Entry(mTextureObject %d)", __FUNCTION__, __LINE__) );

      // Still have to call this to make sure cells are wide enough for new values:
      setSize( Point2I( 1, mList.size() ) );
   }
   setUpdate();
}

void GuiIconListCtrl::setEntryActive(U32 id, bool active)
{
   S32 index = findEntryById( id );
   if ( index == -1 )
      return;

   if ( mList[index].active != active )
   {
      mList[index].active = active;

      // You can't have an inactive entry selected...
      if ( !active && mSelectedCell.y >= 0 && mSelectedCell.y < mList.size()
           && mList[mSelectedCell.y].id == id )
         setSelectedCell( Point2I( -1, -1 ) );

      setUpdate();
   }
}

S32 GuiIconListCtrl::findEntryById(U32 id)
{
   for(U32 i = 0; i < mList.size(); i++)
      if(mList[i].id == id)
         return i;
   return -1;
}

S32 GuiIconListCtrl::findEntryByIconPath(const String& iconPath)
{
	for(U32 i = 0; i < mList.size(); i++)
		if(!dStricmp(mList[i].iconPath, iconPath))
			return i;
	return -1;
}

bool GuiIconListCtrl::isEntryActive(U32 id)
{
   S32 index = findEntryById( id );
   if ( index == -1 )
      return( false );

   return( mList[index].active );
}

void GuiIconListCtrl::setSize(Point2I newSize)
{
   mSize = newSize;

   Point2I newExtent( newSize.x * mCellSize.x + mHeaderDim.x, newSize.y * mCellSize.y + mHeaderDim.y );
   setExtent( newExtent );
}

void GuiIconListCtrl::clear()
{
   while (mList.size())
      removeEntry(mList[0].id);

   mMouseOverCell.set( -1, -1 );
   setSelectedCell(Point2I(-1, -1));
}

void GuiIconListCtrl::sort(U32 column, bool increasing)
{
   if (getNumEntries() < 2)
      return;
   sortColumn = column;
   sIncreasing = increasing;
   dQsort((void *)&(mList[0]), mList.size(), sizeof(Entry), textCompare);
}

void GuiIconListCtrl::sortNumerical( U32 column, bool increasing )
{
   if ( getNumEntries() < 2 )
      return;

   sortColumn = column;
   sIncreasing = increasing;
   dQsort( (void*) &( mList[0] ), mList.size(), sizeof( Entry ), numCompare );
}

void GuiIconListCtrl::onRemove()
{
   clear();
   Parent::onRemove();
}

U32 GuiIconListCtrl::getNumEntries()
{
   return mList.size();
}

void GuiIconListCtrl::removeEntryByIndex(S32 index)
{
   if(index < 0 || index >= mList.size())
      return;
   mList.erase(index);

   setSize(Point2I( 1, mList.size()));
   setSelectedCell(Point2I(-1, -1));
}

void GuiIconListCtrl::removeEntry(U32 id)
{
   S32 index = findEntryById(id);
   removeEntryByIndex(index);
}

const String& GuiIconListCtrl::getSelectedPath()
{
   if (mSelectedCell.y == -1)
      return String::EmptyString;

   return mList[mSelectedCell.y].iconPath;
}

const char* GuiIconListCtrl::getScriptValue()
{
   return getSelectedPath().c_str();
}

void GuiIconListCtrl::setScriptValue(const char *val)
{
   S32 e = findEntryByIconPath(val);
   if(e == -1)
      setSelectedCell(Point2I(-1, -1));
   else
      setSelectedCell(Point2I(0, e));
}

bool GuiIconListCtrl::onKeyDown( const GuiEvent &event )
{
   //if this control is a dead end, make sure the event stops here
   if ( !mVisible || !mActive || !mAwake )
      return true;

   S32 yDelta = 0;
   switch( event.keyCode )
   {
   case KEY_RETURN:
      execAltConsoleCallback();
      break;
   case KEY_LEFT:
   case KEY_UP:
      if ( mSelectedCell.y > 0 )
      {
         mSelectedCell.y--;
         yDelta = -mCellSize.y;
   }
      break;
   case KEY_DOWN:
   case KEY_RIGHT:
      if ( mSelectedCell.y < ( mList.size() - 1 ) )
   {
         mSelectedCell.y++;
         yDelta = mCellSize.y;
   }
      break;
   case KEY_HOME:
      if ( mList.size() )
      {
         mSelectedCell.y = 0;
         yDelta = -(mCellSize.y * mList.size() + 1 );
      }
      break;
   case KEY_END:
      if ( mList.size() )
   {
         mSelectedCell.y = mList.size() - 1;
         yDelta = (mCellSize.y * mList.size() + 1 );
      }
      break;
   case KEY_DELETE:
      if ( mSelectedCell.y >= 0 && mSelectedCell.y < mList.size() )
	  if (deleteEvent.valid())
	     deleteEvent(this, mList[mSelectedCell.y].id);
      onDeleteKey_callback(Con::getIntArg( mList[mSelectedCell.y].id ) );
      break;
   default:
   return( Parent::onKeyDown( event ) );
      break;
   };

   GuiScrollCtrl* parent = dynamic_cast<GuiScrollCtrl *>(getParent());
   if ( parent )
      parent->scrollDelta( 0, yDelta );

   return ( true );



}

//-----------------------------------------------------------------------------
// Console Methods
//-----------------------------------------------------------------------------

DefineEngineMethod( GuiIconListCtrl, getSelectedId, S32, (),,
   "@brief Get the ID of the currently selected item.\n\n"
   "@tsexample\n"
   "// Acquire the ID of the selected item in the list.\n"
   "%id = %thisGuiIconListCtrl.getSelectedId();\n"
   "@endtsexample\n\n"
   "@return The id of the selected item in the list.\n\n"
   "@see GuiControl")
{
   return object->getSelectedId();
}

DefineEngineMethod( GuiIconListCtrl, setSelectedById, void, (int id),,
   "@brief Finds the specified entry by id, then marks its row as selected.\n\n"
   "@param id Entry within the text list to make selected.\n"
   "@tsexample\n"
   "// Define the id\n"
   "%id = \"5\";\n\n"
   "// Inform the GuiIconListCtrl control to set the defined id entry as selected\n"
   "%thisGuiIconListCtrl.setSelectedById(%id);\n"
   "@endtsexample\n\n"
   "@see GuiControl")
{
   S32 index = object->findEntryById(id);
   if(index < 0)
      return ;

   object->setSelectedCell(Point2I(0, index));
}

DefineEngineMethod( GuiIconListCtrl, setSelectedRow, void, (int rowNum),,
   "@briefSelects the specified row.\n\n"
   "@param rowNum Row number to set selected.\n"
   "@tsexample\n"
   "// Define the row number to set selected\n"
   "%rowNum = \"4\";\n\n"
   "%GuiIconListCtrl.setSelectedRow(%rowNum);\n"
   "@endtsexample\n\n"
   "@see GuiControl")
{
   object->setSelectedCell( Point2I( 0, rowNum ) );
}

DefineEngineMethod( GuiIconListCtrl, getSelectedRow, S32, (),,
   "@brief Returns the selected row index (not the row ID).\n\n"
   "@tsexample\n"
   "// Acquire the selected row index\n"
   "%rowIndex = %thisGuiIconListCtrl.getSelectedRow();\n"
   "@endtsexample\n\n"
   "@return Index of the selected row\n\n"
   "@see GuiControl")
{
   return object->getSelectedRow();
}

DefineEngineMethod( GuiIconListCtrl, clearSelection, void, (),,
   "@brief Set the selection to nothing.\n\n"
   "@tsexample\n"
   "// Deselect anything that is currently selected\n"
   "%thisGuiIconListCtrl.clearSelection();\n"
   "@endtsexample\n\n"
   "@see GuiControl")
{
   object->setSelectedCell(Point2I(-1, -1));
}

DefineEngineMethod( GuiIconListCtrl, addRow, S32, (int id, const char* iconPath, int index),(0,"",-1),
   "@brief Adds a new row at end of the list with the defined id and text.\n"
   "If index is used, then the new row is inserted at the row location of 'index'.\n\n"
   "@param id Id of the new row.\n"
   "@param text Text to display at the new row.\n"
   "@param index Index to insert the new row at. If not used, new row will be placed at the end of the list.\n"
   "@tsexample\n"
   "// Define the id\n"
   "%id = \"4\";\n\n"
   "// Define the text to display\n"
   "%text = \"Display Text\"\n\n"
   "// Define the index (optional)\n"
   "%index = \"2\"\n\n"
   "// Inform the GuiIconListCtrl control to add the new row with the defined information.\n"
   "%rowIndex = %thisGuiIconListCtrl.addRow(%id,%text,%index);\n"
   "@endtsexample\n\n"
   "@return Returns the row index of the new row. If 'index' was defined, then this just returns the number of rows in the list.\n\n"
   "@see References")
{
   S32 ret = object->mList.size();
   if(index == -1)
      object->addEntry(id, iconPath);
   else
      object->insertEntry(id, iconPath, index);

   return ret;
}

DefineEngineMethod( GuiIconListCtrl, setRowById, void, (int id, const char* iconPath),,
   "@brief Sets the text at the defined id.\n\n"
   "@param id Id to change.\n"
   "@param text Text to use at the Id.\n"
   "@tsexample\n"
   "// Define the id\n"
   "%id = \"4\";\n\n"
   "// Define the text\n"
   "%text = \"Text To Display\";\n\n"
   "// Inform the GuiIconListCtrl control to display the defined text at the defined id\n"
   "%thisGuiIconListCtrl.setRowById(%id,%text);\n"
   "@endtsexample\n\n"
   "@see GuiControl")
{
   object->setEntry(id, iconPath);
}

DefineEngineMethod( GuiIconListCtrl, sort, void, ( int columnId, bool increasing ), ( true ),
   "@brief Performs a standard (alphabetical) sort on the values in the specified column.\n\n"
   "@param columnId Column ID to perform the sort on.\n"
   "@param increasing If false, sort will be performed in reverse.\n"
   "@tsexample\n"
   "// Define the columnId\n"
   "%id = \"1\";\n\n"
   "// Define if we are increasing or not\n"
   "%increasing = \"false\";\n\n"
   "// Inform the GuiIconListCtrl to perform the sort operation\n"
   "%thisGuiIconListCtrl.sort(%id,%increasing);\n"
   "@endtsexample\n\n"
   "@see GuiControl")
{
     object->sort( columnId, increasing );
}

DefineEngineMethod( GuiIconListCtrl, sortNumerical, void, (int columnID, bool increasing), ( true ),
   "@brief Perform a numerical sort on the values in the specified column.\n\n"
   "Detailed description\n\n"
   "@param columnId Column ID to perform the sort on.\n"
   "@param increasing If false, sort will be performed in reverse.\n"
   "@tsexample\n"
   "// Define the columnId\n"
   "%id = \"1\";\n\n"
   "// Define if we are increasing or not\n"
   "%increasing = \"false\";\n\n"
   "// Inform the GuiIconListCtrl to perform the sort operation\n"
   "%thisGuiIconListCtrl.sortNumerical(%id,%increasing);\n"
   "@endtsexample\n\n"
   "@see GuiControl")
{
     object->sortNumerical( columnID, increasing );
}

DefineEngineMethod( GuiIconListCtrl, clear, void, (),,
   "@brief Clear the list.\n\n"
   "@tsexample\n"
   "// Inform the GuiIconListCtrl control to clear its contents\n"
   "%thisGuiIconListCtrl.clear();\n"
   "@endtsexample\n\n"
   "@see GuiControl")
{
   object->clear();
}

DefineEngineMethod( GuiIconListCtrl, rowCount, S32, (),,
   "@brief Get the number of rows.\n\n"
   "@tsexample\n"
   "// Get the number of rows in the list\n"
   "%rowCount = %thisGuiIconListCtrl.rowCount();\n"
   "@endtsexample\n\n"
   "@return Number of rows in the list.\n\n"
   "@see GuiControl")
{
   return object->getNumEntries();
}

DefineEngineMethod( GuiIconListCtrl, getRowId, S32, (int index),,
   "@brief Get the row ID for an index.\n\n"
   "@param index Index to get the RowID at\n"
   "@tsexample\n"
   "// Define the index\n"
   "%index = \"3\";\n\n"
   "// Request the row ID at the defined index\n"
   "%rowId = %thisGuiIconListCtrl.getRowId(%index);\n"
   "@endtsexample\n\n"
   "@return RowId at the defined index.\n\n"
   "@see GuiControl")
{
   if(index >= object->getNumEntries())
      return -1;

   return object->mList[index].id;
}

DefineEngineMethod( GuiIconListCtrl, getRowIconPathById, const char*, (int id),,
   "@brief Get the text of a row with the specified id.\n\n"
   "@tsexample\n"
   "// Define the id\n"
   "%id = \"4\";\n\n"
   "// Inform the GuiIconListCtrl control to return the text at the defined row id\n"
   "%iconPath = %thisGuiIconListCtrl.getRowIconPathById(%id);\n"
   "@endtsexample\n\n"
   "@return Row text at the requested row id.\n\n"
   "@see GuiControl")
{
   S32 index = object->findEntryById(id);
   if(index < 0)
      return "";
   return object->mList[index].iconPath;
}

DefineEngineMethod( GuiIconListCtrl, getRowNumById, S32, (int id),,
   "@brief Get the row number for a specified id.\n\n"
   "@param id Id to get the row number at\n"
   "@tsexample\n"
   "// Define the id\n"
   "%id = \"4\";\n\n"
   "// Request the row number from the GuiIconListCtrl control at the defined id.\n"
   "%rowNumber = %thisGuiIconListCtrl.getRowNumById(%id);\n"
   "@endtsexample\n\n"
   "@see GuiControl")
{
   S32 index = object->findEntryById(id);
   if(index < 0)
      return -1;
   return index;
}

DefineEngineMethod( GuiIconListCtrl, getRowText, const char*, (int index),,
   "@brief Get the text of the row with the specified index.\n\n"
   "@param index Row index to acquire the text at.\n"
   "@tsexample\n"
   "// Define the row index\n"
   "%index = \"5\";\n\n"
   "// Request the text from the row at the defined index\n"
   "%rowText = %thisGuiIconListCtrl.getRowText(%index);\n"
   "@endtsexample\n\n"
   "@return Text at the defined row index.\n\n"
   "@see GuiControl")
{
   if(index < 0 || index >= object->mList.size())
      return "";
   return object->mList[index].iconPath;
}

DefineEngineMethod( GuiIconListCtrl, removeRowById, void, (int id),,
   "@brief Remove row with the specified id.\n\n"
   "@param id Id to remove the row entry at\n"
   "@tsexample\n"
   "// Define the id\n"
   "%id = \"4\";\n\n"
   "// Inform the GuiIconListCtrl control to remove the row at the defined id\n"
   "%thisGuiIconListCtrl.removeRowById(%id);\n"
   "@endtsexample\n\n"
   "@see GuiControl")
{
   object->removeEntry(id);
}

DefineEngineMethod( GuiIconListCtrl, removeRow, void, (int index),,
   "@brief Remove a row from the table, based on its index.\n\n"
   "@param index Row index to remove from the list.\n"
   "@tsexample\n"
   "// Define the row index\n"
   "%index = \"4\";\n\n"
   "// Inform the GuiIconListCtrl control to remove the row at the defined row index\n"
   "%thisGuiIconListCtrl.removeRow(%index);\n"
   "@endtsexample\n\n"
   "@see GuiControl")
{
   object->removeEntryByIndex(index);
}

DefineEngineMethod( GuiIconListCtrl, scrollVisible, void, (int rowNum),,
   "@brief Scroll so the specified row is visible\n\n"
   "@param rowNum Row number to make visible\n"
   "@tsexample\n"
   "// Define the row number to make visible\n"
   "%rowNum = \"4\";\n\n"
   "// Inform the GuiIconListCtrl control to scroll the list so the defined rowNum is visible.\n"
   "%thisGuiIconListCtrl.scrollVisible(%rowNum);\n"
   "@endtsexample\n\n"
   "@see GuiControl")
{
   object->scrollCellVisible(Point2I(0, rowNum));
}

DefineEngineMethod( GuiIconListCtrl, findPathIndex, S32, (const char* needle),,
   "@brief Find needle in the list, and return the row number it was found in.\n\n"
   "@param needle Text to find in the list.\n"
   "@tsexample\n"
   "// Define the text to find in the list\n"
   "%needle = \"path to find\";\n\n"
   "// Request the row number that contains the defined text to find\n\n"
   "%rowNumber = %thisGuiIconListCtrl.findTextIndex(%needle);\n\n"
   "@endtsexample\n\n"
   "@return Row number that the defined text was found in,\n\n"
   "@see GuiControl")
{
   return( object->findEntryByIconPath(needle) );
}

DefineEngineMethod( GuiIconListCtrl, setRowActive, void, (int rowNum, bool active),,
   "@brief Mark a specified row as active/not.\n\n"
   "@param rowNum Row number to change the active state.\n"
   "@param active Boolean active state to set the row number.\n"
   "@tsexample\n"
   "// Define the row number\n"
   "%rowNum = \"4\";\n\n"
   "// Define the boolean active state\n"
   "%active = \"true\";\n\n"
   "// Informthe GuiIconListCtrl control to set the defined active state at the defined row number.\n"
   "%thisGuiIconListCtrl.setRowActive(%rowNum,%active);\n"
   "@endtsexample\n\n"
   "@see GuiControl")
{
   object->setEntryActive( U32( rowNum ), active );
}

DefineEngineMethod( GuiIconListCtrl, isRowActive, bool, (int rowNum),,
   "@brief Check if the specified row is currently active or not.\n\n"
   "@param rowNum Row number to check the active state.\n"
   "@tsexample\n"
   "// Define the row number\n"
   "%rowNum = \"5\";\n\n"
   "// Request the active state of the defined row number from the GuiIconListCtrl control.\n"
   "%rowActiveState = %thisGuiIconListCtrl.isRowActive(%rowNum);\n"
   "@endtsexample\n\n"
   "@return Active state of the defined row number.\n\n"
   "@see GuiControl")
{
   return( object->isEntryActive( U32( rowNum ) ) );
}
