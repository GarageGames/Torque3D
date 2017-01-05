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
#include "gui/controls/guiTextListCtrl.h"

#include "console/consoleTypes.h"
#include "console/console.h"
#include "gui/containers/guiScrollCtrl.h"
#include "gui/core/guiDefaultControlRender.h"
#include "gfx/gfxDrawUtil.h"
#include "console/engineAPI.h"

IMPLEMENT_CONOBJECT(GuiTextListCtrl);

ConsoleDocClass( GuiTextListCtrl,
   "@brief GUI control that displays a list of text. Text items in the list can be individually selected.\n\n"

   "@tsexample\n"

   "      new GuiTextListCtrl(EndGameGuiList)\n"
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


IMPLEMENT_CALLBACK( GuiTextListCtrl, onSelect, void, (S32 cellid, const char* text),( cellid , text ),
   "@brief Called whenever an item in the list is selected.\n\n"
   "@param cellid The ID of the cell that was selected\n"
   "@param text The text in the selected cel\n\n"
   "@tsexample\n"
   "// A cel in the control was selected, causing the callback to occur\n"
   "GuiTextListCtrl::onSelect(%this,%callid,%text)\n"
   "	{\n"
   "		// Code to run when a cel item is selected\n"
   "	}\n"
   "@endtsexample\n\n"
   "@see GuiControl\n\n"
);

IMPLEMENT_CALLBACK( GuiTextListCtrl, onDeleteKey, void, ( S32 id ),( id ),
   "@brief Called when the delete key has been pressed.\n\n"
   "@param id Id of the selected item in the list\n"
   "@tsexample\n"
   "// The delete key was pressed while the GuiTextListCtrl was in focus, causing the callback to occur.\n"
   "GuiTextListCtrl::onDeleteKey(%this,%id)\n"
   "	{\n"
   "		// Code to run when the delete key is pressed\n"
   "	}\n"
   "@endtsexample\n\n"
   "@see GuiControl\n\n"
);

static S32 sortColumn;
static bool sIncreasing;

static const char *getColumn(const char *text)
{
   S32 ct = sortColumn;
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
   GuiTextListCtrl::Entry *ea = (GuiTextListCtrl::Entry *) (a);
   GuiTextListCtrl::Entry *eb = (GuiTextListCtrl::Entry *) (b);
   S32 result = dStrnatcasecmp( getColumn( ea->text ), getColumn( eb->text ) );
   return ( sIncreasing ? result : -result );
}

static S32 QSORT_CALLBACK numCompare(const void *a,const void *b)
{
   GuiTextListCtrl::Entry *ea = (GuiTextListCtrl::Entry *) (a);
   GuiTextListCtrl::Entry *eb = (GuiTextListCtrl::Entry *) (b);
   const char* aCol = getColumn( ea->text );
   const char* bCol = getColumn( eb->text );
   F32 result = dAtof(aCol) - dAtof(bCol);
   S32 res = result < 0 ? -1 : (result > 0 ? 1 : 0);

   return ( sIncreasing ? res : -res );
}

GuiTextListCtrl::GuiTextListCtrl()
{
   VECTOR_SET_ASSOCIATION(mList);
   VECTOR_SET_ASSOCIATION(mColumnOffsets);

   mActive = true;
   mSize.set(1, 0);
   mColumnOffsets.push_back(0);
   mFitParentWidth = true;
   mClipColumnText = false;
}

void GuiTextListCtrl::initPersistFields()
{
   addField("columns",                 TypeS32Vector, Offset(mColumnOffsets, GuiTextListCtrl), "A vector of column offsets.  The number of values determines the number of columns in the table.\n" );
   addField("fitParentWidth",          TypeBool, Offset(mFitParentWidth, GuiTextListCtrl), "If true, the width of this control will match the width of its parent.\n");
   addField("clipColumnText",          TypeBool, Offset(mClipColumnText, GuiTextListCtrl), "If true, text exceeding a column's given width will get clipped.\n" );
   Parent::initPersistFields();
}

bool GuiTextListCtrl::onWake()
{
   if(!Parent::onWake())
      return false;

   setSize(mSize);
   return true;
}

U32 GuiTextListCtrl::getSelectedId()
{
   if (mSelectedCell.y == -1)
      return InvalidId;

   return mList[mSelectedCell.y].id;
}

U32 GuiTextListCtrl::getSelectedRow()
{
   return mSelectedCell.y;
}

bool GuiTextListCtrl::cellSelected(Point2I cell)
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

void GuiTextListCtrl::onCellSelected(Point2I cell)
{
   onSelect_callback(mList[cell.y].id, mList[cell.y].text);
   execConsoleCallback();
}

void GuiTextListCtrl::onRenderCell(Point2I offset, Point2I cell, bool selected, bool mouseOver)
{
   if ( mList[cell.y].active )
   {
      if (selected || (mProfile->mMouseOverSelected && mouseOver))
      {
         RectI highlightRect = RectI(offset.x, offset.y, mCellSize.x, mCellSize.y);
         highlightRect.inset( 0, -1 );
         renderFilledBorder( highlightRect, mProfile->mBorderColorHL, mProfile->mFillColorHL);
         GFX->getDrawUtil()->setBitmapModulation(mProfile->mFontColorHL);
      }
      else
         GFX->getDrawUtil()->setBitmapModulation(mouseOver ? mProfile->mFontColorHL : mProfile->mFontColor);
   }
   else
      GFX->getDrawUtil()->setBitmapModulation( mProfile->mFontColorNA );

   const char *text = mList[cell.y].text;
   for(U32 index = 0; index < mColumnOffsets.size(); index++)
   {
      const char *nextCol = dStrchr(text, '\t');
      if(mColumnOffsets[index] >= 0)
      {
         dsize_t slen;
         if(nextCol)
            slen = nextCol - text;
         else
            slen = dStrlen(text);

         Point2I pos(offset.x + 4 + mColumnOffsets[index], offset.y);

         RectI saveClipRect;
         bool clipped = false;

         if(mClipColumnText && (index != (mColumnOffsets.size() - 1)))
         {
            saveClipRect = GFX->getClipRect();

            RectI clipRect(pos, Point2I(mColumnOffsets[index+1] - mColumnOffsets[index] - 4, mCellSize.y));
            if(clipRect.intersect(saveClipRect))
            {
               clipped = true;
               GFX->setClipRect( clipRect );
            }
         }

         GFX->getDrawUtil()->drawTextN(mFont, pos, text, slen, mProfile->mFontColors);

         if(clipped)
            GFX->setClipRect( saveClipRect );
      }
      if(!nextCol)
         break;
      text = nextCol+1;
   }
}

U32 GuiTextListCtrl::getRowWidth(Entry *row)
{
   U32 width = 1;
   const char *text = row->text;
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

void GuiTextListCtrl::insertEntry(U32 id, const char *text, S32 index)
{
   Entry e;
   e.text = dStrdup(text);
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

void GuiTextListCtrl::addEntry(U32 id, const char *text)
{
   Entry e;
   e.text = dStrdup(text);
   e.id = id;
   e.active = true;
   mList.push_back(e);
   setSize(Point2I(1, mList.size()));
}

void GuiTextListCtrl::setEntry(U32 id, const char *text)
{
   S32 e = findEntryById(id);
   if(e == -1)
      addEntry(id, text);
   else
   {
      dFree(mList[e].text);
      mList[e].text = dStrdup(text);

      // Still have to call this to make sure cells are wide enough for new values:
      setSize( Point2I( 1, mList.size() ) );
   }
   setUpdate();
}

void GuiTextListCtrl::setEntryActive(U32 id, bool active)
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

S32 GuiTextListCtrl::findEntryById(U32 id)
{
   for(U32 i = 0; i < mList.size(); i++)
      if(mList[i].id == id)
         return i;
   return -1;
}

S32 GuiTextListCtrl::findEntryByText(const char *text)
{
   for(U32 i = 0; i < mList.size(); i++)
      if(!dStricmp(mList[i].text, text))
         return i;
   return -1;
}

bool GuiTextListCtrl::isEntryActive(U32 id)
{
   S32 index = findEntryById( id );
   if ( index == -1 )
      return( false );

   return( mList[index].active );
}

void GuiTextListCtrl::setSize(Point2I newSize)
{
   mSize = newSize;

   if ( bool( mFont ) )
   {
      if ( mSize.x == 1 && mFitParentWidth )
      {
         GuiScrollCtrl* parent = dynamic_cast<GuiScrollCtrl *>(getParent());
         if ( parent )
            mCellSize.x = parent->getContentExtent().x;
      }
      else
      {
         // Find the maximum width cell:
         S32 maxWidth = 1;
         for ( U32 i = 0; i < mList.size(); i++ )
         {
            U32 rWidth = getRowWidth( &mList[i] );
            if ( rWidth > maxWidth )
               maxWidth = rWidth;
         }

         mCellSize.x = maxWidth + 8;
      }

      mCellSize.y = mFont->getHeight() + 2;
   }

   Point2I newExtent( newSize.x * mCellSize.x + mHeaderDim.x, newSize.y * mCellSize.y + mHeaderDim.y );
   setExtent( newExtent );
}

void GuiTextListCtrl::clear()
{
   while (mList.size())
      removeEntry(mList[0].id);

   mMouseOverCell.set( -1, -1 );
   setSelectedCell(Point2I(-1, -1));
}

void GuiTextListCtrl::sort(U32 column, bool increasing)
{
   if (getNumEntries() < 2)
      return;
   sortColumn = column;
   sIncreasing = increasing;
   dQsort((void *)&(mList[0]), mList.size(), sizeof(Entry), textCompare);
}

void GuiTextListCtrl::sortNumerical( U32 column, bool increasing )
{
   if ( getNumEntries() < 2 )
      return;

   sortColumn = column;
   sIncreasing = increasing;
   dQsort( (void*) &( mList[0] ), mList.size(), sizeof( Entry ), numCompare );
}

void GuiTextListCtrl::onRemove()
{
   clear();
   Parent::onRemove();
}

U32 GuiTextListCtrl::getNumEntries()
{
   return mList.size();
}

void GuiTextListCtrl::removeEntryByIndex(S32 index)
{
   if(index < 0 || index >= mList.size())
      return;
   dFree(mList[index].text);
   mList.erase(index);

   setSize(Point2I( 1, mList.size()));
   setSelectedCell(Point2I(-1, -1));
}

void GuiTextListCtrl::removeEntry(U32 id)
{
   S32 index = findEntryById(id);
   removeEntryByIndex(index);
}

const char *GuiTextListCtrl::getSelectedText()
{
   if (mSelectedCell.y == -1)
      return NULL;

   return mList[mSelectedCell.y].text;
}

const char *GuiTextListCtrl::getScriptValue()
{
   return getSelectedText();
}

void GuiTextListCtrl::setScriptValue(const char *val)
{
   S32 e = findEntryByText(val);
   if(e == -1)
      setSelectedCell(Point2I(-1, -1));
   else
      setSelectedCell(Point2I(0, e));
}

bool GuiTextListCtrl::onKeyDown( const GuiEvent &event )
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
      onDeleteKey_callback( mList[mSelectedCell.y].id );
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

DefineEngineMethod( GuiTextListCtrl, getSelectedId, S32, (),,
   "@brief Get the ID of the currently selected item.\n\n"
   "@tsexample\n"
   "// Acquire the ID of the selected item in the list.\n"
   "%id = %thisGuiTextListCtrl.getSelectedId();\n"
   "@endtsexample\n\n"
   "@return The id of the selected item in the list.\n\n"
   "@see GuiControl")
{
   return object->getSelectedId();
}

DefineEngineMethod( GuiTextListCtrl, setSelectedById, void, (S32 id),,
   "@brief Finds the specified entry by id, then marks its row as selected.\n\n"
   "@param id Entry within the text list to make selected.\n"
   "@tsexample\n"
   "// Define the id\n"
   "%id = \"5\";\n\n"
   "// Inform the GuiTextListCtrl control to set the defined id entry as selected\n"
   "%thisGuiTextListCtrl.setSelectedById(%id);\n"
   "@endtsexample\n\n"
   "@see GuiControl")
{
   S32 index = object->findEntryById(id);
   if(index < 0)
      return ;

   object->setSelectedCell(Point2I(0, index));
}

DefineEngineMethod( GuiTextListCtrl, setSelectedRow, void, (S32 rowNum),,
   "@briefSelects the specified row.\n\n"
   "@param rowNum Row number to set selected.\n"
   "@tsexample\n"
   "// Define the row number to set selected\n"
   "%rowNum = \"4\";\n\n"
   "%guiTextListCtrl.setSelectedRow(%rowNum);\n"
   "@endtsexample\n\n"
   "@see GuiControl")
{
   object->setSelectedCell( Point2I( 0, rowNum ) );
}

DefineEngineMethod( GuiTextListCtrl, getSelectedRow, S32, (),,
   "@brief Returns the selected row index (not the row ID).\n\n"
   "@tsexample\n"
   "// Acquire the selected row index\n"
   "%rowIndex = %thisGuiTextListCtrl.getSelectedRow();\n"
   "@endtsexample\n\n"
   "@return Index of the selected row\n\n"
   "@see GuiControl")
{
   return object->getSelectedRow();
}

DefineEngineMethod( GuiTextListCtrl, clearSelection, void, (),,
   "@brief Set the selection to nothing.\n\n"
   "@tsexample\n"
   "// Deselect anything that is currently selected\n"
   "%thisGuiTextListCtrl.clearSelection();\n"
   "@endtsexample\n\n"
   "@see GuiControl")
{
   object->setSelectedCell(Point2I(-1, -1));
}

DefineEngineMethod( GuiTextListCtrl, addRow, S32, (S32 id, const char* text, S32 index),(0,"",-1),
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
   "// Inform the GuiTextListCtrl control to add the new row with the defined information.\n"
   "%rowIndex = %thisGuiTextListCtrl.addRow(%id,%text,%index);\n"
   "@endtsexample\n\n"
   "@return Returns the row index of the new row. If 'index' was defined, then this just returns the number of rows in the list.\n\n"
   "@see References")
{
   S32 ret = object->mList.size();
   if(index == -1)
      object->addEntry(id, text);
   else
      object->insertEntry(id, text, index);

   return ret;
}

DefineEngineMethod( GuiTextListCtrl, setRowById, void, (S32 id, const char* text),,
   "@brief Sets the text at the defined id.\n\n"
   "@param id Id to change.\n"
   "@param text Text to use at the Id.\n"
   "@tsexample\n"
   "// Define the id\n"
   "%id = \"4\";\n\n"
   "// Define the text\n"
   "%text = \"Text To Display\";\n\n"
   "// Inform the GuiTextListCtrl control to display the defined text at the defined id\n"
   "%thisGuiTextListCtrl.setRowById(%id,%text);\n"
   "@endtsexample\n\n"
   "@see GuiControl")
{
   object->setEntry(id, text);
}

DefineEngineMethod( GuiTextListCtrl, sort, void, ( S32 columnId, bool increasing ), ( true ),
   "@brief Performs a standard (alphabetical) sort on the values in the specified column.\n\n"
   "@param columnId Column ID to perform the sort on.\n"
   "@param increasing If false, sort will be performed in reverse.\n"
   "@tsexample\n"
   "// Define the columnId\n"
   "%id = \"1\";\n\n"
   "// Define if we are increasing or not\n"
   "%increasing = \"false\";\n\n"
   "// Inform the GuiTextListCtrl to perform the sort operation\n"
   "%thisGuiTextListCtrl.sort(%id,%increasing);\n"
   "@endtsexample\n\n"
   "@see GuiControl")
{
     object->sort( columnId, increasing );
}

DefineEngineMethod( GuiTextListCtrl, sortNumerical, void, (S32 columnID, bool increasing), ( true ),
   "@brief Perform a numerical sort on the values in the specified column.\n\n"
   "Detailed description\n\n"
   "@param columnId Column ID to perform the sort on.\n"
   "@param increasing If false, sort will be performed in reverse.\n"
   "@tsexample\n"
   "// Define the columnId\n"
   "%id = \"1\";\n\n"
   "// Define if we are increasing or not\n"
   "%increasing = \"false\";\n\n"
   "// Inform the GuiTextListCtrl to perform the sort operation\n"
   "%thisGuiTextListCtrl.sortNumerical(%id,%increasing);\n"
   "@endtsexample\n\n"
   "@see GuiControl")
{
     object->sortNumerical( columnID, increasing );
}

DefineEngineMethod( GuiTextListCtrl, clear, void, (),,
   "@brief Clear the list.\n\n"
   "@tsexample\n"
   "// Inform the GuiTextListCtrl control to clear its contents\n"
   "%thisGuiTextListCtrl.clear();\n"
   "@endtsexample\n\n"
   "@see GuiControl")
{
   object->clear();
}

DefineEngineMethod( GuiTextListCtrl, rowCount, S32, (),,
   "@brief Get the number of rows.\n\n"
   "@tsexample\n"
   "// Get the number of rows in the list\n"
   "%rowCount = %thisGuiTextListCtrl.rowCount();\n"
   "@endtsexample\n\n"
   "@return Number of rows in the list.\n\n"
   "@see GuiControl")
{
   return object->getNumEntries();
}

DefineEngineMethod( GuiTextListCtrl, getRowId, S32, (S32 index),,
   "@brief Get the row ID for an index.\n\n"
   "@param index Index to get the RowID at\n"
   "@tsexample\n"
   "// Define the index\n"
   "%index = \"3\";\n\n"
   "// Request the row ID at the defined index\n"
   "%rowId = %thisGuiTextListCtrl.getRowId(%index);\n"
   "@endtsexample\n\n"
   "@return RowId at the defined index.\n\n"
   "@see GuiControl")
{
   if(index >= object->getNumEntries())
      return -1;

   return object->mList[index].id;
}

DefineEngineMethod( GuiTextListCtrl, getRowTextById, const char*, (S32 id),,
   "@brief Get the text of a row with the specified id.\n\n"
   "@tsexample\n"
   "// Define the id\n"
   "%id = \"4\";\n\n"
   "// Inform the GuiTextListCtrl control to return the text at the defined row id\n"
   "%rowText = %thisGuiTextListCtrl.getRowTextById(%id);\n"
   "@endtsexample\n\n"
   "@return Row text at the requested row id.\n\n"
   "@see GuiControl")
{
   S32 index = object->findEntryById(id);
   if(index < 0)
      return "";
   return object->mList[index].text;
}

DefineEngineMethod( GuiTextListCtrl, getRowNumById, S32, (S32 id),,
   "@brief Get the row number for a specified id.\n\n"
   "@param id Id to get the row number at\n"
   "@tsexample\n"
   "// Define the id\n"
   "%id = \"4\";\n\n"
   "// Request the row number from the GuiTextListCtrl control at the defined id.\n"
   "%rowNumber = %thisGuiTextListCtrl.getRowNumById(%id);\n"
   "@endtsexample\n\n"
   "@see GuiControl")
{
   S32 index = object->findEntryById(id);
   if(index < 0)
      return -1;
   return index;
}

DefineEngineMethod( GuiTextListCtrl, getRowText, const char*, (S32 index),,
   "@brief Get the text of the row with the specified index.\n\n"
   "@param index Row index to acquire the text at.\n"
   "@tsexample\n"
   "// Define the row index\n"
   "%index = \"5\";\n\n"
   "// Request the text from the row at the defined index\n"
   "%rowText = %thisGuiTextListCtrl.getRowText(%index);\n"
   "@endtsexample\n\n"
   "@return Text at the defined row index.\n\n"
   "@see GuiControl")
{
   if(index < 0 || index >= object->mList.size())
      return "";
   return object->mList[index].text;
}

DefineEngineMethod( GuiTextListCtrl, removeRowById, void, (S32 id),,
   "@brief Remove row with the specified id.\n\n"
   "@param id Id to remove the row entry at\n"
   "@tsexample\n"
   "// Define the id\n"
   "%id = \"4\";\n\n"
   "// Inform the GuiTextListCtrl control to remove the row at the defined id\n"
   "%thisGuiTextListCtrl.removeRowById(%id);\n"
   "@endtsexample\n\n"
   "@see GuiControl")
{
   object->removeEntry(id);
}

DefineEngineMethod( GuiTextListCtrl, removeRow, void, (S32 index),,
   "@brief Remove a row from the table, based on its index.\n\n"
   "@param index Row index to remove from the list.\n"
   "@tsexample\n"
   "// Define the row index\n"
   "%index = \"4\";\n\n"
   "// Inform the GuiTextListCtrl control to remove the row at the defined row index\n"
   "%thisGuiTextListCtrl.removeRow(%index);\n"
   "@endtsexample\n\n"
   "@see GuiControl")
{
   object->removeEntryByIndex(index);
}

DefineEngineMethod( GuiTextListCtrl, scrollVisible, void, (S32 rowNum),,
   "@brief Scroll so the specified row is visible\n\n"
   "@param rowNum Row number to make visible\n"
   "@tsexample\n"
   "// Define the row number to make visible\n"
   "%rowNum = \"4\";\n\n"
   "// Inform the GuiTextListCtrl control to scroll the list so the defined rowNum is visible.\n"
   "%thisGuiTextListCtrl.scrollVisible(%rowNum);\n"
   "@endtsexample\n\n"
   "@see GuiControl")
{
   object->scrollCellVisible(Point2I(0, rowNum));
}

DefineEngineMethod( GuiTextListCtrl, findTextIndex, S32, (const char* needle),,
   "@brief Find needle in the list, and return the row number it was found in.\n\n"
   "@param needle Text to find in the list.\n"
   "@tsexample\n"
   "// Define the text to find in the list\n"
   "%needle = \"Text To Find\";\n\n"
   "// Request the row number that contains the defined text to find\n\n"
   "%rowNumber = %thisGuiTextListCtrl.findTextIndex(%needle);\n\n"
   "@endtsexample\n\n"
   "@return Row number that the defined text was found in,\n\n"
   "@see GuiControl")
{
   return( object->findEntryByText(needle) );
}

DefineEngineMethod( GuiTextListCtrl, setRowActive, void, (S32 rowNum, bool active),,
   "@brief Mark a specified row as active/not.\n\n"
   "@param rowNum Row number to change the active state.\n"
   "@param active Boolean active state to set the row number.\n"
   "@tsexample\n"
   "// Define the row number\n"
   "%rowNum = \"4\";\n\n"
   "// Define the boolean active state\n"
   "%active = \"true\";\n\n"
   "// Informthe GuiTextListCtrl control to set the defined active state at the defined row number.\n"
   "%thisGuiTextListCtrl.setRowActive(%rowNum,%active);\n"
   "@endtsexample\n\n"
   "@see GuiControl")
{
   object->setEntryActive( U32( rowNum ), active );
}

DefineEngineMethod( GuiTextListCtrl, isRowActive, bool, (S32 rowNum),,
   "@brief Check if the specified row is currently active or not.\n\n"
   "@param rowNum Row number to check the active state.\n"
   "@tsexample\n"
   "// Define the row number\n"
   "%rowNum = \"5\";\n\n"
   "// Request the active state of the defined row number from the GuiTextListCtrl control.\n"
   "%rowActiveState = %thisGuiTextListCtrl.isRowActive(%rowNum);\n"
   "@endtsexample\n\n"
   "@return Active state of the defined row number.\n\n"
   "@see GuiControl")
{
   return( object->isEntryActive( U32( rowNum ) ) );
}
