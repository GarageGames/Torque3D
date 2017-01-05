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

#include "gui/core/guiCanvas.h"
#include "gui/controls/guiPopUpCtrlEx.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "gui/core/guiDefaultControlRender.h"
#include "gfx/primBuilder.h"
#include "gfx/gfxDrawUtil.h"
#include "console/engineAPI.h"

ConsoleDocClass( GuiPopUpMenuCtrlEx,
	"@brief A control that allows to select a value from a drop-down list.\n\n"

	"This is essentially a GuiPopUpMenuCtrl, but with quite a few more features.\n\n"

	"@tsexample\n"
	"new GuiPopUpMenuCtrlEx()\n"
	"{\n"
	"	maxPopupHeight = \"200\";\n"
	"	sbUsesNAColor = \"0\";\n"
	"	reverseTextList = \"0\";\n"
	"	bitmapBounds = \"16 16\";\n"
	"	hotTrackCallback = \"0\";\n"
	"	extent = \"64 64\";\n"
	"	profile = \"GuiDefaultProfile\";\n"
	"	tooltipProfile = \"GuiToolTipProfile\";\n"
	"};\n"
	"@endtsexample\n\n"

	"@see GuiPopUpMenuCtrl\n"

	"@ingroup GuiControls\n");

static ColorI colorWhite(255,255,255); //  Added

// Function to return the number of columns in 'string' given delimeters in 'set'
static U32 getColumnCount(const char *string, const char *set)
{
   U32 count = 0;
   U8 last = 0;
   while(*string)
   {
      last = *string++;

      for(U32 i =0; set[i]; i++)
      {
         if(last == set[i])
         {
            count++;
            last = 0;
            break;
         }   
      }
   }
   if(last)
      count++;
   return count;
}   

// Function to return the 'index' column from 'string' given delimeters in 'set'
static const char *getColumn(const char *string, char* returnbuff, U32 index, const char *set)
{
   U32 sz;
   while(index--)
   {
      if(!*string)
         return "";
      sz = dStrcspn(string, set);
      if (string[sz] == 0)
         return "";
      string += (sz + 1);    
   }
   sz = dStrcspn(string, set);
   if (sz == 0)
      return "";
   char *ret = returnbuff;
   dStrncpy(ret, string, sz);
   ret[sz] = '\0';
   return ret;
}   

GuiPopUpBackgroundCtrlEx::GuiPopUpBackgroundCtrlEx(GuiPopUpMenuCtrlEx *ctrl, GuiPopupTextListCtrlEx *textList)
{
   mPopUpCtrl = ctrl;
   mTextList = textList;
}

void GuiPopUpBackgroundCtrlEx::onMouseDown(const GuiEvent &event)
{
   //   mTextList->setSelectedCell(Point2I(-1,-1)); //  Removed
   mPopUpCtrl->mBackgroundCancel = true; //  Set that the user didn't click within the text list.  Replaces the line above.
   mPopUpCtrl->closePopUp();
}

//------------------------------------------------------------------------------
GuiPopupTextListCtrlEx::GuiPopupTextListCtrlEx()
{
   mPopUpCtrl = NULL;
}


//------------------------------------------------------------------------------
GuiPopupTextListCtrlEx::GuiPopupTextListCtrlEx(GuiPopUpMenuCtrlEx *ctrl)
{
   mPopUpCtrl = ctrl;
}

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//void GuiPopUpTextListCtrl::onCellSelected( Point2I /*cell*/ )
//{
//   // Do nothing, the parent control will take care of everything...
//}
void GuiPopupTextListCtrlEx::onCellSelected( Point2I cell )
{
   //  The old function is above.  This new one will only call the the select
   //      functions if we were not cancelled by a background click.

   //  Check if we were cancelled by the user clicking on the Background ie: anywhere
   //      other than within the text list.
   if(mPopUpCtrl->mBackgroundCancel)
      return;

   if( isMethod( "onSelect" ) )
      Con::executef(this, "onSelect", Con::getFloatArg(cell.x), Con::getFloatArg(cell.y));

   //call the console function
   execConsoleCallback();
   //if (mConsoleCommand[0])
   //   Con::evaluate(mConsoleCommand, false);

}

bool GuiPopupTextListCtrlEx::hasCategories()
{
   for( S32 i = 0; i < mList.size(); i++)
   {
      if( mList[i].id == -1)
         return true;
   }

   return false;
}

//------------------------------------------------------------------------------
bool GuiPopupTextListCtrlEx::onKeyDown(const GuiEvent &event)
{
   //if the control is a dead end, don't process the input:
   if ( !mVisible || !mActive || !mAwake )
      return false;

   //see if the key down is a <return> or not
   if ( event.modifier == 0 )
   {
      if ( event.keyCode == KEY_RETURN )
      {
         mPopUpCtrl->closePopUp();
         return true;
      }
      else if ( event.keyCode == KEY_ESCAPE )
      {
         mSelectedCell.set( -1, -1 );
         mPopUpCtrl->closePopUp();
         return true;
      }
   }

   //otherwise, pass the event to it's parent
   return Parent::onKeyDown(event);
}

void GuiPopupTextListCtrlEx::onMouseUp(const GuiEvent &event)
{
   Point2I pt = globalToLocalCoord(event.mousePoint);
   pt.x -= mHeaderDim.x; pt.y -= mHeaderDim.y;
   Point2I cell(
      (pt.x < 0 ? -1 : pt.x / mCellSize.x), 
      (pt.y < 0 ? -1 : pt.y / mCellSize.y)
      );

   if (cell.x >= 0 && cell.x < mSize.x && cell.y >= 0 && cell.y < mSize.y)
   {
      if (mList[cell.y].id == -1)
         return;
   }

   Parent::onMouseUp(event);
   mPopUpCtrl->closePopUp();
}

void GuiPopupTextListCtrlEx::onMouseMove( const GuiEvent &event )
{
   if( !mPopUpCtrl || !mPopUpCtrl->isMethod("onHotTrackItem") )
      return Parent::onMouseMove( event );

   Point2I pt = globalToLocalCoord(event.mousePoint);
   pt.x -= mHeaderDim.x; pt.y -= mHeaderDim.y;
   Point2I cell( (pt.x < 0 ? -1 : pt.x / mCellSize.x), (pt.y < 0 ? -1 : pt.y / mCellSize.y) );

   // Within Bounds?
   if (cell.x >= 0 && cell.x < mSize.x && cell.y >= 0 && cell.y < mSize.y)
      // Hot Track notification
      Con::executef( mPopUpCtrl, "onHotTrackItem", Con::getIntArg(mList[cell.y].id) );
   else 
      // Hot Track -1
      Con::executef( mPopUpCtrl, "onHotTrackItem", Con::getIntArg(-1) );

   // Call Parent
   Parent::onMouseMove(event);
}

//------------------------------------------------------------------------------
void GuiPopupTextListCtrlEx::onRenderCell(Point2I offset, Point2I cell, bool selected, bool mouseOver)
{
   Point2I size;
   getCellSize( size );

   // Render a background color for the cell
   if ( mouseOver )
   {      
      RectI cellR( offset.x, offset.y, size.x, size.y );
      GFX->getDrawUtil()->drawRectFill( cellR, mProfile->mFillColorHL );
   }
   else if ( selected )
   {
      RectI cellR( offset.x, offset.y, size.x, size.y );
      GFX->getDrawUtil()->drawRectFill( cellR, mProfile->mFillColorSEL );
   }

   //  Define the default x offset for the text
   U32 textXOffset = offset.x + mProfile->mTextOffset.x;

   //  Do we also draw a colored box beside the text?
   ColorI boxColor;
   bool drawbox = mPopUpCtrl->getColoredBox( boxColor, mList[cell.y].id );
   if ( drawbox )
   {
      Point2I coloredboxsize( 15, 10 );
      RectI r( offset.x + mProfile->mTextOffset.x, offset.y + 2, coloredboxsize.x, coloredboxsize.y );
      GFX->getDrawUtil()->drawRectFill( r, boxColor );
      GFX->getDrawUtil()->drawRect( r, ColorI( 0, 0, 0 ) );

      textXOffset += coloredboxsize.x + mProfile->mTextOffset.x;
   }

   // Render 'Group' background
   if ( mList[cell.y].id == -1)
   {
      RectI cellR( offset.x, offset.y, size.x, size.y );
      GFX->getDrawUtil()->drawRectFill( cellR, mProfile->mFillColorHL );
   }

   ColorI fontColor;
   mPopUpCtrl->getFontColor( fontColor, mList[cell.y].id, selected, mouseOver );

   GFX->getDrawUtil()->setBitmapModulation( fontColor );
   //GFX->drawText( mFont, Point2I( offset.x + 4, offset.y ), mList[cell.y].text );

   //  Get the number of columns in the cell
   S32 colcount = getColumnCount(mList[cell.y].text, "\t");

   //  Are there two or more columns?
   if(colcount >= 2)
   {
      char buff[256];

      // Draw the first column
      getColumn(mList[cell.y].text, buff, 0, "\t");
      GFX->getDrawUtil()->drawText( mFont, Point2I( textXOffset, offset.y ), buff ); //  Used mTextOffset as a margin for the text list rather than the hard coded value of '4'.

      // Draw the second column to the right
      getColumn(mList[cell.y].text, buff, 1, "\t");
      S32 txt_w = mFont->getStrWidth(buff);

      GFX->getDrawUtil()->drawText( mFont, Point2I( offset.x+size.x-mProfile->mTextOffset.x-txt_w, offset.y ), buff ); //  Used mTextOffset as a margin for the text list rather than the hard coded value of '4'.

   } else
   {
      if ((mList[cell.y].id == -1) || (!hasCategories()))
         GFX->getDrawUtil()->drawText( mFont, Point2I( textXOffset, offset.y ), mList[cell.y].text ); //  Used mTextOffset as a margin for the text list rather than the hard coded value of '4'.
      else
         GFX->getDrawUtil()->drawText( mFont, Point2I( textXOffset + 8, offset.y ), mList[cell.y].text ); //  Used mTextOffset as a margin for the text list rather than the hard coded value of '4'.
   }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

IMPLEMENT_CONOBJECT(GuiPopUpMenuCtrlEx);

GuiPopUpMenuCtrlEx::GuiPopUpMenuCtrlEx(void)
{
   VECTOR_SET_ASSOCIATION(mEntries);
   VECTOR_SET_ASSOCIATION(mSchemes);

   mSelIndex = -1;
   mActive = true;
   mMaxPopupHeight = 200;
   mScrollDir = GuiScrollCtrl::None;
   mScrollCount = 0;
   mLastYvalue = 0;
   mIncValue = 0;
   mRevNum = 0;
   mInAction = false;
   mMouseOver = false; //  Added
   mRenderScrollInNA = false; //  Added
   mBackgroundCancel = false; //  Added
   mReverseTextList = false; //  Added - Don't reverse text list if displaying up
   mBitmapName = StringTable->insert(""); //  Added
   mBitmapBounds.set(16, 16); //  Added
   mHotTrackItems = false;
	mIdMax = -1;
}

//------------------------------------------------------------------------------
GuiPopUpMenuCtrlEx::~GuiPopUpMenuCtrlEx()
{
}

//------------------------------------------------------------------------------
void GuiPopUpMenuCtrlEx::initPersistFields(void)
{
   addField("maxPopupHeight",           TypeS32,          Offset(mMaxPopupHeight, GuiPopUpMenuCtrlEx), "Length of menu when it extends");
   addField("sbUsesNAColor",            TypeBool,         Offset(mRenderScrollInNA, GuiPopUpMenuCtrlEx), "Deprecated" "@internal");
   addField("reverseTextList",          TypeBool,         Offset(mReverseTextList, GuiPopUpMenuCtrlEx), "Reverses text list if popup extends up, instead of down");
   addField("bitmap",                   TypeFilename,     Offset(mBitmapName, GuiPopUpMenuCtrlEx), "File name of bitmap to use");
   addField("bitmapBounds",             TypePoint2I,      Offset(mBitmapBounds, GuiPopUpMenuCtrlEx), "Boundaries of bitmap displayed");
   addField("hotTrackCallback",         TypeBool,         Offset(mHotTrackItems, GuiPopUpMenuCtrlEx),
      "Whether to provide a 'onHotTrackItem' callback when a list item is hovered over");

   Parent::initPersistFields();
}

//------------------------------------------------------------------------------
ConsoleDocFragment _GuiPopUpMenuCtrlExAdd(
	"@brief Adds an entry to the list\n\n"
	"@param name String containing the name of the entry\n"
	"@param idNum Numerical value assigned to the name\n"
	"@param scheme Optional ID associated with a scheme "
	"for font coloring, highlight coloring, and selection coloring\n\n",
	"GuiPopUpMenuCtrlEx",
	"void add(string name, S32 idNum, S32 scheme=0);"
);

DefineConsoleMethod( GuiPopUpMenuCtrlEx, add, void, (const char * name, S32 idNum, U32 scheme), ("", -1, 0), "(string name, int idNum, int scheme=0)")
{
   object->addEntry(name, idNum, scheme);
}

DefineEngineMethod( GuiPopUpMenuCtrlEx, addCategory, void, (const char* text),,
				   "@brief Add a category to the list.\n\n"

				   "Acts as a separator between entries, allowing for sub-lists\n\n"

				   "@param text Name of the new category\n\n")
{
	object->addEntry(text, -1, 0);
}

DefineEngineMethod( GuiPopUpMenuCtrlEx, addScheme, void, (S32 id, ColorI fontColor, ColorI fontColorHL, ColorI fontColorSEL),,
				   "@brief Create a new scheme and add it to the list of choices for when a new text entry is added.\n\n"
				   "@param id Numerical id associated with this scheme\n"
				   "@param fontColor The base text font color. Formatted as \"Red Green Blue\", each a numerical between 0 and 255.\n"
				   "@param fontColorHL Color of text when being highlighted. Formatted as \"Red Green Blue\", each a numerical between 0 and 255.\n"
				   "@param fontColorSel Color of text when being selected. Formatted as \"Red Green Blue\", each a numerical between 0 and 255.\n")
{
	/*ColorI fontColor, fontColorHL, fontColorSEL;
   U32 r, g, b;
   char buf[64];

   dStrcpy( buf, argv[3] );
   char* temp = dStrtok( buf, " \0" );
   r = temp ? dAtoi( temp ) : 0;
   temp = dStrtok( NULL, " \0" );
   g = temp ? dAtoi( temp ) : 0;
   temp = dStrtok( NULL, " \0" );
   b = temp ? dAtoi( temp ) : 0;
   fontColor.set( r, g, b );

   dStrcpy( buf, argv[4] );
   temp = dStrtok( buf, " \0" );
   r = temp ? dAtoi( temp ) : 0;
   temp = dStrtok( NULL, " \0" );
   g = temp ? dAtoi( temp ) : 0;
   temp = dStrtok( NULL, " \0" );
   b = temp ? dAtoi( temp ) : 0;
   fontColorHL.set( r, g, b );

   dStrcpy( buf, argv[5] );
   temp = dStrtok( buf, " \0" );
   r = temp ? dAtoi( temp ) : 0;
   temp = dStrtok( NULL, " \0" );
   g = temp ? dAtoi( temp ) : 0;
   temp = dStrtok( NULL, " \0" );
   b = temp ? dAtoi( temp ) : 0;
   fontColorSEL.set( r, g, b );*/

   object->addScheme( id, fontColor, fontColorHL, fontColorSEL );
}

//ConsoleMethod( GuiPopUpMenuCtrlEx, addScheme, void, 6, 6, "(int id, ColorI fontColor, ColorI fontColorHL, ColorI fontColorSEL)")
//{
//   ColorI fontColor, fontColorHL, fontColorSEL;
//   U32 r, g, b;
//   char buf[64];
//
//   dStrcpy( buf, argv[3] );
//   char* temp = dStrtok( buf, " \0" );
//   r = temp ? dAtoi( temp ) : 0;
//   temp = dStrtok( NULL, " \0" );
//   g = temp ? dAtoi( temp ) : 0;
//   temp = dStrtok( NULL, " \0" );
//   b = temp ? dAtoi( temp ) : 0;
//   fontColor.set( r, g, b );
//
//   dStrcpy( buf, argv[4] );
//   temp = dStrtok( buf, " \0" );
//   r = temp ? dAtoi( temp ) : 0;
//   temp = dStrtok( NULL, " \0" );
//   g = temp ? dAtoi( temp ) : 0;
//   temp = dStrtok( NULL, " \0" );
//   b = temp ? dAtoi( temp ) : 0;
//   fontColorHL.set( r, g, b );
//
//   dStrcpy( buf, argv[5] );
//   temp = dStrtok( buf, " \0" );
//   r = temp ? dAtoi( temp ) : 0;
//   temp = dStrtok( NULL, " \0" );
//   g = temp ? dAtoi( temp ) : 0;
//   temp = dStrtok( NULL, " \0" );
//   b = temp ? dAtoi( temp ) : 0;
//   fontColorSEL.set( r, g, b );
//
//   object->addScheme( dAtoi( argv[2] ), fontColor, fontColorHL, fontColorSEL );
//}

DefineEngineMethod( GuiPopUpMenuCtrlEx, setText, void, ( const char* text),,
				   "@brief Set the current text to a specified value.\n\n"
				   "@param text String containing new text to set\n\n")
{
	object->setText(text);
}

DefineEngineMethod( GuiPopUpMenuCtrlEx, getText, const char*, (),,
				   "@brief Get the.\n\n"

				   "Detailed description\n\n"

				   "@param param Description\n\n"

				   "@tsexample\n"
				   "// Comment\n"
				   "code();\n"
				   "@endtsexample\n\n"

				   "@return Returns current text in string format")
{
	return object->getText();
}

DefineEngineMethod( GuiPopUpMenuCtrlEx, clear, void, (),,
				   "@brief Clear the popup list.\n\n")
{
	object->clear();
}

DefineEngineMethod( GuiPopUpMenuCtrlEx, sort, void, (),,
				   "@brief Sort the list alphabetically.\n\n")
{
	object->sort();
}

DefineEngineMethod( GuiPopUpMenuCtrlEx, sortID, void, (),,
				   "@brief Sort the list by ID.\n\n")
{
	object->sortID();
}

DefineEngineMethod( GuiPopUpMenuCtrlEx, forceOnAction, void, (),,
				   "@brief Manually for the onAction function, which updates everything in this control.\n\n")
{
	object->onAction();
}

DefineEngineMethod( GuiPopUpMenuCtrlEx, forceClose, void, (),,
				   "@brief Manually force this control to collapse and close.\n\n")
{
	object->closePopUp();
}

DefineEngineMethod( GuiPopUpMenuCtrlEx, getSelected, S32, (),,
				   "@brief Get the current selection of the menu.\n\n"
				   "@return Returns the ID of the currently selected entry")
{
	return object->getSelected();
}

ConsoleDocFragment _GuiPopUpMenuCtrlExsetSelected(
	"brief Manually set an entry as selected int his control\n\n"
	"@param id The ID of the entry to select\n"
	"@param scripCallback Optional boolean that forces the script callback if true\n",
	"GuiPopUpMenuCtrlEx",
	"setSelected(int id, bool scriptCallback=true);"
);

DefineConsoleMethod( GuiPopUpMenuCtrlEx, setSelected, void, (S32 id, bool scriptCallback), (true), "(int id, [scriptCallback=true])"
			  "@hide")
{
   object->setSelected( id, scriptCallback );
}

ConsoleDocFragment _GuiPopUpMenuCtrlExsetFirstSelected(
	"brief Manually set the selection to the first entry\n\n"
	"@param scripCallback Optional boolean that forces the script callback if true\n",
	"GuiPopUpMenuCtrlEx",
	"setSelected(bool scriptCallback=true);"
);


DefineConsoleMethod( GuiPopUpMenuCtrlEx, setFirstSelected, void, (bool scriptCallback), (true), "([scriptCallback=true])"
			  "@hide")
{
   object->setFirstSelected( scriptCallback );
}

DefineEngineMethod( GuiPopUpMenuCtrlEx, setNoneSelected, void, ( S32 param),,
				   "@brief Clears selection in the menu.\n\n")
{
	object->setNoneSelected();
}


DefineEngineMethod( GuiPopUpMenuCtrlEx, getTextById, const char*, (S32 id),,
				   "@brief Get the text of an entry based on an ID.\n\n"
				   "@param id The ID assigned to the entry being queried\n\n"
				   "@return String contained by the specified entry, NULL if empty or bad ID")
{
	return(object->getTextById(id));
}


DefineConsoleMethod( GuiPopUpMenuCtrlEx, getColorById,  ColorI, (S32 id), ,  
			  "@brief Get color of an entry's box\n\n"
			  "@param id ID number of entry to query\n\n"
			  "@return ColorI in the format of \"Red Green Blue Alpha\", each of with is a value between 0 - 255")
{
   ColorI color;
   object->getColoredBox(color, id);
	return color;

}

DefineConsoleMethod( GuiPopUpMenuCtrlEx, setEnumContent, void, ( const char * className, const char * enumName ), ,
			  "@brief This fills the popup with a classrep's field enumeration type info.\n\n"
              "More of a helper function than anything.   If console access to the field list is added, "
              "at least for the enumerated types, then this should go away.\n\n"
			  "@param class Name of the class containing the enum\n"
			  "@param enum Name of the enum value to acces\n")
{
   AbstractClassRep * classRep = AbstractClassRep::getClassList();

   // walk the class list to get our class
   while(classRep)
   {
      if(!dStricmp(classRep->getClassName(), className))
         break;
      classRep = classRep->getNextClass();
   }

   // get it?
   if(!classRep)
   {
      Con::warnf(ConsoleLogEntry::General, "failed to locate class rep for '%s'", className);
      return;
   }

   // walk the fields to check for this one (findField checks StringTableEntry ptrs...)
   U32 i;
   for(i = 0; i < classRep->mFieldList.size(); i++)
      if(!dStricmp(classRep->mFieldList[i].pFieldname, enumName))
         break;

   // found it?   
   if(i == classRep->mFieldList.size())
   {   
      Con::warnf(ConsoleLogEntry::General, "failed to locate field '%s' for class '%s'", enumName, className);
      return;
   }

   const AbstractClassRep::Field & field = classRep->mFieldList[i];
   ConsoleBaseType* conType = ConsoleBaseType::getType( field.type );

   // check the type
   if( !conType->getEnumTable() )
   {
      Con::warnf(ConsoleLogEntry::General, "field '%s' is not an enumeration for class '%s'", enumName, className);
      return;
   }

   // fill it
   const EngineEnumTable& table = *( conType->getEnumTable() );
   const U32 numValues = table.getNumValues();
   
   for(i = 0; i < numValues; i++)
      object->addEntry( table[i].getName(), table[i] );
}

//------------------------------------------------------------------------------
DefineConsoleMethod( GuiPopUpMenuCtrlEx, findText, S32, (const char * text), , "(string text)"
              "Returns the id of the first entry containing the specified text or -1 if not found."
			  "@param text String value used for the query\n\n"
			  "@return Numerical ID of entry containing the text.")
{
   return( object->findText( text ) );
}

//------------------------------------------------------------------------------
DefineConsoleMethod( GuiPopUpMenuCtrlEx, size, S32, (), , 
			  "@brief Get the size of the menu\n\n"
			  "@return Number of entries in the menu\n")
{
   return( object->getNumEntries() ); 
}

//------------------------------------------------------------------------------
DefineConsoleMethod( GuiPopUpMenuCtrlEx, replaceText, void, (S32 boolVal), , 
			  "@brief Flag that causes each new text addition to replace the current entry\n\n"
			  "@param True to turn on replacing, false to disable it")
{
   object->replaceText(boolVal);
}

//------------------------------------------------------------------------------
//  Added
bool GuiPopUpMenuCtrlEx::onWake()
{
   if ( !Parent::onWake() )
      return false;

   // Set the bitmap for the popup.
   setBitmap( mBitmapName );

   // Now update the Form Control's bitmap array, and possibly the child's too
   mProfile->constructBitmapArray();

   if ( mProfile->getChildrenProfile() )
      mProfile->getChildrenProfile()->constructBitmapArray();

   return true;
}

//------------------------------------------------------------------------------
bool GuiPopUpMenuCtrlEx::onAdd()
{
   if ( !Parent::onAdd() )
      return false;
   mSelIndex = -1;
   mReplaceText = true;
   return true;
}

//------------------------------------------------------------------------------
void GuiPopUpMenuCtrlEx::onSleep()
{
   mTextureNormal = NULL; //  Added
   mTextureDepressed = NULL; //  Added
   Parent::onSleep();
   closePopUp();  // Tests in function.
}

//------------------------------------------------------------------------------
void GuiPopUpMenuCtrlEx::clear()
{
   mEntries.setSize(0);
   setText("");
   mSelIndex = -1;
   mRevNum = 0;
	mIdMax = -1;
}

//------------------------------------------------------------------------------
void GuiPopUpMenuCtrlEx::clearEntry( S32 entry )
{
	if( entry == -1 )
		return;

	U32 i = 0;
	for ( ; i < mEntries.size(); i++ )
   {
      if ( mEntries[i].id == entry )
         break;
   }

	mEntries.erase( i );

	if( mEntries.size() <= 0 )
	{
		mEntries.setSize(0);
		setText("");
		mSelIndex = -1;
		mRevNum = 0;
	}
	else
	{
		if( entry == mSelIndex )
		{
			setText("");
			mSelIndex = -1;
		}
		else
		{
			mSelIndex--;
		}
	}
}

//------------------------------------------------------------------------------
DefineConsoleMethod( GuiPopUpMenuCtrlEx, clearEntry, void, (S32 entry), , "(S32 entry)")
{
   object->clearEntry(entry);
}

//------------------------------------------------------------------------------
static S32 QSORT_CALLBACK textCompare(const void *a,const void *b)
{
   GuiPopUpMenuCtrlEx::Entry *ea = (GuiPopUpMenuCtrlEx::Entry *) (a);
   GuiPopUpMenuCtrlEx::Entry *eb = (GuiPopUpMenuCtrlEx::Entry *) (b);
   return (dStrnatcasecmp(ea->buf, eb->buf));
} 

//  Added to sort by entry ID
//------------------------------------------------------------------------------
static S32 QSORT_CALLBACK idCompare(const void *a,const void *b)
{
   GuiPopUpMenuCtrlEx::Entry *ea = (GuiPopUpMenuCtrlEx::Entry *) (a);
   GuiPopUpMenuCtrlEx::Entry *eb = (GuiPopUpMenuCtrlEx::Entry *) (b);
   return ( (ea->id < eb->id) ? -1 : ((ea->id > eb->id) ? 1 : 0) );
} 

//------------------------------------------------------------------------------
//  Added
void GuiPopUpMenuCtrlEx::setBitmap(const char *name)
{
   mBitmapName = StringTable->insert( name );
   if ( !isAwake() )
      return;

   if ( *mBitmapName )
   {
      char buffer[1024];
      char *p;
      dStrcpy(buffer, name);
      p = buffer + dStrlen(buffer);

      dStrcpy(p, "_n");
      mTextureNormal = GFXTexHandle( (StringTableEntry)buffer, &GFXDefaultGUIProfile, avar("%s() - mTextureNormal (line %d)", __FUNCTION__, __LINE__) );

      dStrcpy(p, "_d");
      mTextureDepressed = GFXTexHandle( (StringTableEntry)buffer, &GFXDefaultGUIProfile, avar("%s() - mTextureDepressed (line %d)", __FUNCTION__, __LINE__) );
      if ( !mTextureDepressed )
         mTextureDepressed = mTextureNormal;
   }
   else
   {
      mTextureNormal = NULL;
      mTextureDepressed = NULL;
   }
   setUpdate();
}   

//------------------------------------------------------------------------------
void GuiPopUpMenuCtrlEx::sort()
{
   S32 size = mEntries.size();
   if( size > 0 )
      dQsort( mEntries.address(), size, sizeof(Entry), textCompare);

	// Entries need to re-Id themselves
	for( U32 i = 0; i < mEntries.size(); i++ )
		mEntries[i].id = i;
}

//  Added to sort by entry ID
//------------------------------------------------------------------------------
void GuiPopUpMenuCtrlEx::sortID()
{
   S32 size = mEntries.size();
   if( size > 0 )
      dQsort( mEntries.address(), size, sizeof(Entry), idCompare);

	// Entries need to re-Id themselves
	for( U32 i = 0; i < mEntries.size(); i++ )
		mEntries[i].id = i;
}

//------------------------------------------------------------------------------
void GuiPopUpMenuCtrlEx::addEntry(const char *buf, S32 id, U32 scheme)
{
   if( !buf )
   {
      //Con::printf( "GuiPopupMenuCtrlEx::addEntry - Invalid buffer!" );
      return;
   }
	
	// Ensure that there are no other entries with exactly the same name
	for ( U32 i = 0; i < mEntries.size(); i++ )
   {
      if ( dStrcmp( mEntries[i].buf, buf ) == 0 )
         return;
   }

	// If we don't give an id, create one from mIdMax
	if( id == -1 )
		id = mIdMax + 1;
	
	// Increase mIdMax when an id is greater than it
	if( id > mIdMax )
		mIdMax = id;

   Entry e;
   dStrcpy( e.buf, buf );
   e.id = id;
   e.scheme = scheme;

   // see if there is a shortcut key
   char * cp = dStrchr( e.buf, '~' );
   e.ascii = cp ? cp[1] : 0;

   //  See if there is a colour box defined with the text
   char *cb = dStrchr( e.buf, '|' );
   if ( cb )
   {
      e.usesColorBox = true;
      cb[0] = '\0';

      char* red = &cb[1];
      cb = dStrchr(red, '|');
      cb[0] = '\0';
      char* green = &cb[1];
      cb = dStrchr(green, '|');
      cb[0] = '\0';
      char* blue = &cb[1];

      U32 r = dAtoi(red);
      U32 g = dAtoi(green);
      U32 b = dAtoi(blue);

      e.colorbox = ColorI(r,g,b);

   } 
   else
   {
      e.usesColorBox = false;
   }

   mEntries.push_back(e);

   if ( mInAction && mTl )
   {
      // Add the new entry:
      mTl->addEntry( e.id, e.buf );
      repositionPopup();
   }
}

//------------------------------------------------------------------------------
void GuiPopUpMenuCtrlEx::addScheme( U32 id, ColorI fontColor, ColorI fontColorHL, ColorI fontColorSEL )
{
   if ( !id )
      return;

   Scheme newScheme;
   newScheme.id = id;
   newScheme.fontColor = fontColor;
   newScheme.fontColorHL = fontColorHL;
   newScheme.fontColorSEL = fontColorSEL;

   mSchemes.push_back( newScheme );
}

//------------------------------------------------------------------------------
S32 GuiPopUpMenuCtrlEx::getSelected()
{
   if (mSelIndex == -1)
      return 0;
   return mEntries[mSelIndex].id;
}

//------------------------------------------------------------------------------
const char* GuiPopUpMenuCtrlEx::getTextById(S32 id)
{
   for ( U32 i = 0; i < mEntries.size(); i++ )
   {
      if ( mEntries[i].id == id )
         return( mEntries[i].buf );
   }

   return( "" );
}

//------------------------------------------------------------------------------
S32 GuiPopUpMenuCtrlEx::findText( const char* text )
{
   for ( U32 i = 0; i < mEntries.size(); i++ )
   {
      if ( dStrcmp( text, mEntries[i].buf ) == 0 )
         return( mEntries[i].id );        
   }
   return( -1 );
}

//------------------------------------------------------------------------------
void GuiPopUpMenuCtrlEx::setSelected(S32 id, bool bNotifyScript )
{
   S32 i;
   for ( i = 0; U32(i) < mEntries.size(); i++ )
   {
      if ( id == mEntries[i].id )
      {
         i = ( mRevNum > i ) ? mRevNum - i : i;
         mSelIndex = i;
         if ( mReplaceText ) //  Only change the displayed text if appropriate.
         {
            setText(mEntries[i].buf);
         }

         // Now perform the popup action:
         char idval[24];
         dSprintf( idval, sizeof(idval), "%d", mEntries[mSelIndex].id );
         if ( isMethod( "onSelect" ) && bNotifyScript )
            Con::executef( this, "onSelect", idval, mEntries[mSelIndex].buf );
         return;
      }
   }

   if ( mReplaceText ) //  Only change the displayed text if appropriate.
   {
      setText("");
   }
   mSelIndex = -1;

   if ( isMethod( "onCancel" ) && bNotifyScript )
      Con::executef( this, "onCancel" );

   if ( id == -1 )
      return;

   // Execute the popup console command:
   if ( bNotifyScript )
      execConsoleCallback();
   //if ( mConsoleCommand[0]  && bNotifyScript )
   //   Con::evaluate( mConsoleCommand, false );
}

//------------------------------------------------------------------------------
//  Added to set the first item as selected.
void GuiPopUpMenuCtrlEx::setFirstSelected( bool bNotifyScript )
{
   if ( mEntries.size() > 0 )
   {
      mSelIndex = 0;
      if ( mReplaceText ) //  Only change the displayed text if appropriate.
      {
         setText( mEntries[0].buf );
      }

      // Now perform the popup action:
      char idval[24];
      dSprintf( idval, sizeof(idval), "%d", mEntries[mSelIndex].id );
      if ( isMethod( "onSelect" ) )
         Con::executef( this, "onSelect", idval, mEntries[mSelIndex].buf );
      
		// Execute the popup console command:
		if ( bNotifyScript )
			execConsoleCallback();
   }
	else
	{
		if ( mReplaceText ) //  Only change the displayed text if appropriate.
			setText("");
		
		mSelIndex = -1;

		if ( bNotifyScript )
			Con::executef( this, "onCancel" );
	}
}

//------------------------------------------------------------------------------
//  Added to set no items as selected.
void GuiPopUpMenuCtrlEx::setNoneSelected()
{
   if ( mReplaceText ) //  Only change the displayed text if appropriate.
   {
      setText("");
   }
   mSelIndex = -1;
}

//------------------------------------------------------------------------------
const char *GuiPopUpMenuCtrlEx::getScriptValue()
{
   return getText();
}

//------------------------------------------------------------------------------
void GuiPopUpMenuCtrlEx::onRender(Point2I offset, const RectI &updateRect)
{
   TORQUE_UNUSED(updateRect);
   Point2I localStart;

   if ( mScrollDir != GuiScrollCtrl::None )
      autoScroll();

   GFXDrawUtil* drawUtil = GFX->getDrawUtil();

   RectI r( offset, getExtent() );
   if ( mInAction )
   {
      S32 l = r.point.x, r2 = r.point.x + r.extent.x - 1;
      S32 t = r.point.y, b = r.point.y + r.extent.y - 1;

      // Do we render a bitmap border or lines?
      if ( mProfile->getChildrenProfile() && mProfile->mBitmapArrayRects.size() )
      {
         // Render the fixed, filled in border
         renderFixedBitmapBordersFilled(r, 3, mProfile );

      } 
      else
      {
         //renderSlightlyLoweredBox(r, mProfile);
         drawUtil->drawRectFill( r, mProfile->mFillColor );
      }

      //  Draw a bitmap over the background?
      if ( mTextureDepressed )
      {
         RectI rect(offset, mBitmapBounds);
         drawUtil->clearBitmapModulation();
         drawUtil->drawBitmapStretch( mTextureDepressed, rect );
      } 
      else if ( mTextureNormal )
      {
         RectI rect(offset, mBitmapBounds);
         drawUtil->clearBitmapModulation();
         drawUtil->drawBitmapStretch( mTextureNormal, rect );
      }

      // Do we render a bitmap border or lines?
      if ( !( mProfile->getChildrenProfile() && mProfile->mBitmapArrayRects.size() ) )
      {
         drawUtil->drawLine( l, t, l, b, colorWhite );
         drawUtil->drawLine( l, t, r2, t, colorWhite );
         drawUtil->drawLine( l + 1, b, r2, b, mProfile->mBorderColor );
         drawUtil->drawLine( r2, t + 1, r2, b - 1, mProfile->mBorderColor );
      }

   }
   else   
      // TODO: Implement
      // TODO: Add onMouseEnter() and onMouseLeave() and a definition of mMouseOver (see guiButtonBaseCtrl) for this to work.
      if ( mMouseOver ) 
      {
         S32 l = r.point.x, r2 = r.point.x + r.extent.x - 1;
         S32 t = r.point.y, b = r.point.y + r.extent.y - 1;

         // Do we render a bitmap border or lines?
         if ( mProfile->getChildrenProfile() && mProfile->mBitmapArrayRects.size() )
         {
            // Render the fixed, filled in border
            renderFixedBitmapBordersFilled( r, 2, mProfile );

         } 
         else
         {
            drawUtil->drawRectFill( r, mProfile->mFillColorHL );
         }

         //  Draw a bitmap over the background?
         if ( mTextureNormal )
         {
            RectI rect( offset, mBitmapBounds );
            drawUtil->clearBitmapModulation();
            drawUtil->drawBitmapStretch( mTextureNormal, rect );
         }

         // Do we render a bitmap border or lines?
         if ( !( mProfile->getChildrenProfile() && mProfile->mBitmapArrayRects.size() ) )
         {
            drawUtil->drawLine( l, t, l, b, colorWhite );
            drawUtil->drawLine( l, t, r2, t, colorWhite );
            drawUtil->drawLine( l + 1, b, r2, b, mProfile->mBorderColor );
            drawUtil->drawLine( r2, t + 1, r2, b - 1, mProfile->mBorderColor );
         }
      }
      else
      {
         // Do we render a bitmap border or lines?
         if ( mProfile->getChildrenProfile() && mProfile->mBitmapArrayRects.size() )
         {
            // Render the fixed, filled in border
            renderFixedBitmapBordersFilled( r, 1, mProfile );
         } 
         else
         {
            drawUtil->drawRectFill( r, mProfile->mFillColorNA );
         }

         //  Draw a bitmap over the background?
         if ( mTextureNormal )
         {
            RectI rect(offset, mBitmapBounds);
            drawUtil->clearBitmapModulation();
            drawUtil->drawBitmapStretch( mTextureNormal, rect );
         }

         // Do we render a bitmap border or lines?
         if ( !( mProfile->getChildrenProfile() && mProfile->mBitmapArrayRects.size() ) )
         {
            drawUtil->drawRect( r, mProfile->mBorderColorNA );
         }
      }
      //      renderSlightlyRaisedBox(r, mProfile); //  Used to be the only 'else' condition to mInAction above.

      S32 txt_w = mProfile->mFont->getStrWidth(mText);
      localStart.x = 0;
      localStart.y = (getHeight() - (mProfile->mFont->getHeight())) / 2;

      // align the horizontal
      switch (mProfile->mAlignment)
      {
      case GuiControlProfile::RightJustify:
         if ( mProfile->getChildrenProfile() && mProfile->mBitmapArrayRects.size() )
         {
            // We're making use of a bitmap border, so take into account the
            // right cap of the border.
            RectI* bitmapBounds = mProfile->mBitmapArrayRects.address();
            localStart.x = getWidth() - bitmapBounds[2].extent.x - txt_w;
         } 
         else
         {
            localStart.x = getWidth() - txt_w;  
         }
         break;
      case GuiControlProfile::CenterJustify:
         if ( mProfile->getChildrenProfile() && mProfile->mBitmapArrayRects.size() )
         {
            // We're making use of a bitmap border, so take into account the
            // right cap of the border.
            RectI* bitmapBounds = mProfile->mBitmapArrayRects.address();
            localStart.x = (getWidth() - bitmapBounds[2].extent.x - txt_w) / 2;

         } else
         {
            localStart.x = (getWidth() - txt_w) / 2;
         }
         break;
      default:
         // GuiControlProfile::LeftJustify
         if ( txt_w > getWidth() )
         {
            //  The width of the text is greater than the width of the control.
            // In this case we will right justify the text and leave some space
            // for the down arrow.
            if ( mProfile->getChildrenProfile() && mProfile->mBitmapArrayRects.size() )
            {
               // We're making use of a bitmap border, so take into account the
               // right cap of the border.
               RectI* bitmapBounds = mProfile->mBitmapArrayRects.address();
               localStart.x = getWidth() - bitmapBounds[2].extent.x - txt_w;
            } 
            else
            {
               localStart.x = getWidth() - txt_w - 12;
            }
         } 
         else
         {
            localStart.x = mProfile->mTextOffset.x; //  Use mProfile->mTextOffset as a controlable margin for the control's text.
         }
         break;
      }

      //  Do we first draw a coloured box beside the text?
      ColorI boxColor;
      bool drawbox = getColoredBox( boxColor, mSelIndex);
      if ( drawbox )
      {
         Point2I coloredboxsize( 15, 10 );
         RectI r( offset.x + mProfile->mTextOffset.x, offset.y + ( (getHeight() - coloredboxsize.y ) / 2 ), coloredboxsize.x, coloredboxsize.y );
         drawUtil->drawRectFill( r, boxColor);
         drawUtil->drawRect( r, ColorI(0,0,0));

         localStart.x += coloredboxsize.x + mProfile->mTextOffset.x;
      }

      // Draw the text
      Point2I globalStart = localToGlobalCoord( localStart );
      ColorI fontColor   = mActive ? ( mInAction ? mProfile->mFontColor : mProfile->mFontColorNA ) : mProfile->mFontColorNA;
      drawUtil->setBitmapModulation( fontColor ); //  was: (mProfile->mFontColor);

      //  Get the number of columns in the text
      S32 colcount = getColumnCount( mText, "\t" );

      //  Are there two or more columns?
      if ( colcount >= 2 )
      {
         char buff[256];

         // Draw the first column
         getColumn( mText, buff, 0, "\t" );
         drawUtil->drawText( mProfile->mFont, globalStart, buff, mProfile->mFontColors );

         // Draw the second column to the right
         getColumn( mText, buff, 1, "\t" );
         S32 txt_w = mProfile->mFont->getStrWidth( buff );
         if ( mProfile->getChildrenProfile() && mProfile->mBitmapArrayRects.size() )
         {
            // We're making use of a bitmap border, so take into account the
            // right cap of the border.
            RectI* bitmapBounds = mProfile->mBitmapArrayRects.address();
            Point2I textpos = localToGlobalCoord( Point2I( getWidth() - txt_w - bitmapBounds[2].extent.x, localStart.y ) );
            drawUtil->drawText( mProfile->mFont, textpos, buff, mProfile->mFontColors );

         } else
         {
            Point2I textpos = localToGlobalCoord( Point2I( getWidth() - txt_w - 12, localStart.y ) );
            drawUtil->drawText( mProfile->mFont, textpos, buff, mProfile->mFontColors );
         }

      } else
      {
         drawUtil->drawText( mProfile->mFont, globalStart, mText, mProfile->mFontColors );
      }

      // If we're rendering a bitmap border, then it will take care of the arrow.
      if ( !(mProfile->getChildrenProfile() && mProfile->mBitmapArrayRects.size()) )
      {
         //  Draw a triangle (down arrow)
         S32 left = r.point.x + r.extent.x - 12;
         S32 right = left + 8;
         S32 middle = left + 4;
         S32 top = r.extent.y / 2 + r.point.y - 4;
         S32 bottom = top + 8;

         PrimBuild::color( mProfile->mFontColor );

         PrimBuild::begin( GFXTriangleList, 3 );
            PrimBuild::vertex2fv( Point3F( (F32)left, (F32)top, 0.0f ) );
            PrimBuild::vertex2fv( Point3F( (F32)right, (F32)top, 0.0f ) );
            PrimBuild::vertex2fv( Point3F( (F32)middle, (F32)bottom, 0.0f ) );
         PrimBuild::end();
      }
}

//------------------------------------------------------------------------------
void GuiPopUpMenuCtrlEx::closePopUp()
{
   if ( !mInAction )
      return;

   // Get the selection from the text list:
   mSelIndex = mTl->getSelectedCell().y;
   mSelIndex = ( mRevNum >= mSelIndex && mSelIndex != -1 ) ? mRevNum - mSelIndex : mSelIndex;
   if ( mSelIndex != -1 )
   {
      if ( mReplaceText )
         setText( mEntries[mSelIndex].buf );
      setIntVariable( mEntries[mSelIndex].id );
   }

   // Release the mouse:
   mInAction = false;
   mTl->mouseUnlock();

   //  Commented out below and moved to the end of the function.  See the
   // note below for the reason why.
   /*
   // Pop the background:
   getRoot()->popDialogControl(mBackground);

   // Kill the popup:
   mBackground->removeObject( mSc );
   mTl->deleteObject();
   mSc->deleteObject();
   mBackground->deleteObject();

   // Set this as the first responder:
   setFocus();
   */

   // Now perform the popup action:
   if ( mSelIndex != -1 )
   {
      char idval[24];
      dSprintf( idval, sizeof(idval), "%d", mEntries[mSelIndex].id );
      if ( isMethod( "onSelect" ) )
         Con::executef( this, "onSelect", idval, mEntries[mSelIndex].buf );
   }
   else if ( isMethod( "onCancel" ) )
      Con::executef( this, "onCancel" );

   // Execute the popup console command:
   execConsoleCallback();
   //if ( mConsoleCommand[0] )
   //   Con::evaluate( mConsoleCommand, false );

   //  Reordered this pop dialog to be after the script select callback.  When the
   // background was popped it was causing all sorts of focus issues when
   // suddenly the first text edit control took the focus before it came back
   // to this popup.

   // Pop the background:
   GuiCanvas *root = getRoot();
   if ( root )
      root->popDialogControl(mBackground);

   // Kill the popup:
   mBackground->removeObject( mSc );
   mTl->deleteObject();
   mSc->deleteObject();
   mBackground->deleteObject();

   // Set this as the first responder:
   setFirstResponder();
}

//------------------------------------------------------------------------------
bool GuiPopUpMenuCtrlEx::onKeyDown(const GuiEvent &event)
{
   //if the control is a dead end, don't process the input:
   if ( !mVisible || !mActive || !mAwake )
      return false;

   //see if the key down is a <return> or not
   if ( event.keyCode == KEY_RETURN && event.modifier == 0 )
   {
      onAction();
      return true;
   }

   //otherwise, pass the event to its parent
   return Parent::onKeyDown( event );
}

//------------------------------------------------------------------------------
void GuiPopUpMenuCtrlEx::onAction()
{
   GuiControl *canCtrl = getParent();

   addChildren();

   GuiCanvas *root = getRoot();
   Point2I windowExt = root->getExtent();

   mBackground->resize( Point2I(0,0), root->getExtent() );
   
   S32 textWidth = 0, width = getWidth();
   //const S32 menuSpace = 5; //  Removed as no longer used.
   const S32 textSpace = 2;
   bool setScroll = false;

   for ( U32 i = 0; i < mEntries.size(); ++i )
      if ( S32(mProfile->mFont->getStrWidth( mEntries[i].buf )) > textWidth )
         textWidth = mProfile->mFont->getStrWidth( mEntries[i].buf );

   //if(textWidth > getWidth())
   S32 sbWidth = mSc->getControlProfile()->mBorderThickness * 2 + mSc->scrollBarThickness(); //  Calculate the scroll bar width
   if ( textWidth > ( getWidth() - sbWidth-mProfile->mTextOffset.x - mSc->getChildMargin().x * 2 ) ) //  The text draw area to test against is the width of the drop-down minus the scroll bar width, the text margin and the scroll bar child margins.
   {
      //textWidth +=10;
      textWidth +=sbWidth + mProfile->mTextOffset.x + mSc->getChildMargin().x * 2; //  The new width is the width of the text plus the scroll bar width plus the text margin size plus the scroll bar child margins.
      width = textWidth;

      //  If a child margin is not defined for the scroll control, let's add
      //      some space between the text and scroll control for readability
      if(mSc->getChildMargin().x == 0)
         width += textSpace;
   }

   //mTl->setCellSize(Point2I(width, mFont->getHeight()+3));
   mTl->setCellSize(Point2I(width, mProfile->mFont->getHeight() + textSpace)); //  Modified the above line to use textSpace rather than the '3' as this is what is used below.

   for ( U32 j = 0; j < mEntries.size(); ++j )
      mTl->addEntry( mEntries[j].id, mEntries[j].buf );

   if ( mSelIndex >= 0 )
      mTl->setSelectedCell( Point2I( 0, mSelIndex ) );

   Point2I pointInGC = canCtrl->localToGlobalCoord( getPosition() );
   Point2I scrollPoint( pointInGC.x, pointInGC.y + getHeight() ); 

   //Calc max Y distance, so Scroll Ctrl will fit on window 
   //S32 maxYdis = windowExt.y - pointInGC.y - getHeight() - menuSpace; 
   S32 sbBorder = mSc->getControlProfile()->mBorderThickness * 2 + mSc->getChildMargin().y * 2; //  Added to take into account the border thickness and the margin of the child controls of the scroll control when figuring out the size of the contained text list control
   S32 maxYdis = windowExt.y - pointInGC.y - getHeight() - sbBorder; // - menuSpace; //  Need to remove the border thickness from the contained control maximum extent and got rid of the 'menuspace' variable

   //If scroll bars need to be added
   mRevNum = 0; //  Added here rather than within the following 'if' statements.
   if ( maxYdis < mTl->getHeight() + sbBorder ) //  Instead of adding sbBorder, it was: 'textSpace'
   {
      //Should we pop menu list above the button
      if ( maxYdis < pointInGC.y ) //  removed: '- menuSpace)' from check to see if there is more space above the control than below.
      {
         if(mReverseTextList) //  Added this check if we should reverse the text list.
            reverseTextList();

         maxYdis = pointInGC.y; //  Was at the end: '- menuSpace;'
         //Does the menu need a scroll bar 
         if ( maxYdis < mTl->getHeight() + sbBorder ) //  Instead of adding sbBorder, it was: 'textSpace'
         {
            //  Removed width recalculation for scroll bar as the scroll bar is already being taken into account.
            //Calc for the width of the scroll bar 
            //            if(textWidth >=  width)
            //               width += 20;
            //            mTl->setCellSize(Point2I(width,mFont->getHeight() + textSpace));

            //Pop menu list above the button
            //            scrollPoint.set(pointInGC.x, menuSpace - 1); //  Removed as calculated outside the 'if', and was wrong to begin with
            setScroll = true;
         }
         //No scroll bar needed
         else
         {
            maxYdis = mTl->getHeight() + sbBorder; //  Instead of adding sbBorder, it was: 'textSpace'
            //            scrollPoint.set(pointInGC.x, pointInGC.y - maxYdis -1); //  Removed as calculated outside the 'if' and the '-1' at the end is wrong
         }

         //  Added the next two lines
         scrollPoint.set(pointInGC.x, pointInGC.y - maxYdis); //  Used to have the following on the end: '-1);'
      } 
      //Scroll bar needed but Don't pop above button
      else
      {
         //mRevNum = 0; //  Commented out as it has been added at the beginning of this function

         //  Removed width recalculation for scroll bar as the scroll bar is already being taken into account.
         //Calc for the width of the scroll bar 
         //         if(textWidth >=  width)
         //            width += 20;
         //         mTl->setCellSize(Point2I(width,mFont->getHeight() + textSpace));
         setScroll = true;
      }
   }
   //No scroll bar needed
   else
   {

      //maxYdis = mTl->getHeight() + textSpace;
      maxYdis = mTl->getHeight() + sbBorder; //  Added in the border thickness of the scroll control and removed the addition of textSpace
   }

   RectI newBounds = mSc->getBounds();

   //offset it from the background so it lines up properly
   newBounds.point = mBackground->globalToLocalCoord( scrollPoint );

   if ( newBounds.point.x + width > mBackground->getWidth() )
      if ( width - getWidth() > 0 )
         newBounds.point.x -= width - getWidth();

   //mSc->getExtent().set(width-1, maxYdis);
   newBounds.extent.set( width, maxYdis );
   mSc->setBounds( newBounds ); //  Not sure why the '-1' above.

   mSc->registerObject();
   mTl->registerObject();
   mBackground->registerObject();

   mSc->addObject( mTl );
   mBackground->addObject( mSc );

   mBackgroundCancel = false; //  Setup check if user clicked on the background instead of the text list (ie: didn't want to change their current selection).

   root->pushDialogControl( mBackground, 99 );

   if ( setScroll )
   {
      // Resize the text list
	  Point2I cellSize;
	  mTl->getCellSize( cellSize );
	  cellSize.x = width - mSc->scrollBarThickness() - sbBorder;
	  mTl->setCellSize( cellSize );
	  mTl->setWidth( cellSize.x );

      if ( mSelIndex )
         mTl->scrollCellVisible( Point2I( 0, mSelIndex ) );
      else
         mTl->scrollCellVisible( Point2I( 0, 0 ) );
   }

   mTl->setFirstResponder();

   mInAction = true;
}

//------------------------------------------------------------------------------
void GuiPopUpMenuCtrlEx::addChildren()
{
   mTl = new GuiPopupTextListCtrlEx( this );
   AssertFatal( mTl, "Failed to create the GuiPopUpTextListCtrlEx for the PopUpMenu" );
   // Use the children's profile rather than the parent's profile, if it exists.
   mTl->setControlProfile( mProfile->getChildrenProfile() ? mProfile->getChildrenProfile() : mProfile ); 
   mTl->setField("noDuplicates", "false");

   mSc = new GuiScrollCtrl;
   AssertFatal( mSc, "Failed to create the GuiScrollCtrl for the PopUpMenu" );
   GuiControlProfile *prof;
   if ( Sim::findObject( "GuiScrollProfile", prof ) )
   {
      mSc->setControlProfile( prof );
   }
   else
   {
      // Use the children's profile rather than the parent's profile, if it exists.
	  mSc->setControlProfile( mProfile->getChildrenProfile() ? mProfile->getChildrenProfile() : mProfile );
   }
   mSc->setField( "hScrollBar", "AlwaysOff" );
   mSc->setField( "vScrollBar", "dynamic" );
   //if(mRenderScrollInNA) //  Force the scroll control to render using fillColorNA rather than fillColor
   // mSc->mUseNABackground = true;

   mBackground = new GuiPopUpBackgroundCtrlEx( this, mTl );
   AssertFatal( mBackground, "Failed to create the GuiBackgroundCtrlEx for the PopUpMenu" );
}

//------------------------------------------------------------------------------
void GuiPopUpMenuCtrlEx::repositionPopup()
{
   if ( !mInAction || !mSc || !mTl )
      return;

   // I'm not concerned with this right now...
}

//------------------------------------------------------------------------------
void GuiPopUpMenuCtrlEx::reverseTextList()
{
   mTl->clear();
   for ( S32 i = mEntries.size()-1; i >= 0; --i )
      mTl->addEntry( mEntries[i].id, mEntries[i].buf );

   // Don't lose the selected cell:
   if ( mSelIndex >= 0 )
      mTl->setSelectedCell( Point2I( 0, mEntries.size() - mSelIndex - 1 ) ); 

   mRevNum = mEntries.size() - 1;
}

//------------------------------------------------------------------------------
bool GuiPopUpMenuCtrlEx::getFontColor( ColorI &fontColor, S32 id, bool selected, bool mouseOver )
{
   U32 i;
   Entry* entry = NULL;
   for ( i = 0; i < mEntries.size(); i++ )
   {
      if ( mEntries[i].id == id )
      {
         entry = &mEntries[i];
         break;
      }
   }

   if ( !entry )
      return( false );

   if ( entry->scheme != 0 )
   {
      // Find the entry's color scheme:
      for ( i = 0; i < mSchemes.size(); i++ )
      {
         if ( mSchemes[i].id == entry->scheme )
         {
            fontColor = selected ? mSchemes[i].fontColorSEL : mouseOver ? mSchemes[i].fontColorHL : mSchemes[i].fontColor;
            return( true );
         }
      }
   }

   if(id == -1)
      fontColor = mProfile->mFontColorHL;
   else
   // Default color scheme...
   fontColor = selected ? mProfile->mFontColorSEL : mouseOver ? mProfile->mFontColorHL : mProfile->mFontColorNA; //  Modified the final color choice from mProfile->mFontColor to mProfile->mFontColorNA

   return( true );
}

//------------------------------------------------------------------------------
//  Added
bool GuiPopUpMenuCtrlEx::getColoredBox( ColorI &fontColor, S32 id )
{
   U32 i;
   Entry* entry = NULL;
   for ( i = 0; i < mEntries.size(); i++ )
   {
      if ( mEntries[i].id == id )
      {
         entry = &mEntries[i];
         break;
      }
   }

   if ( !entry )
      return false;

   if ( entry->usesColorBox == false )
      return false;

   fontColor = entry->colorbox;

   return true;
}

//------------------------------------------------------------------------------
void GuiPopUpMenuCtrlEx::onMouseDown(const GuiEvent &event)
{
   TORQUE_UNUSED(event);
   onAction();
}

//------------------------------------------------------------------------------
void GuiPopUpMenuCtrlEx::onMouseUp(const GuiEvent &event)
{
   TORQUE_UNUSED(event);
}

//------------------------------------------------------------------------------
//  Added
void GuiPopUpMenuCtrlEx::onMouseEnter(const GuiEvent &event)
{
   mMouseOver = true;
}

//------------------------------------------------------------------------------
//  Added
void GuiPopUpMenuCtrlEx::onMouseLeave(const GuiEvent &)
{
   mMouseOver = false;
}

//------------------------------------------------------------------------------
void GuiPopUpMenuCtrlEx::setupAutoScroll(const GuiEvent &event)
{
   GuiControl *parent = getParent();
   if ( !parent ) 
      return;

   Point2I mousePt = mSc->globalToLocalCoord( event.mousePoint );

   mEventSave = event;      

   if ( mLastYvalue != mousePt.y )
   {
      mScrollDir = GuiScrollCtrl::None;
      if ( mousePt.y > mSc->getHeight() || mousePt.y < 0 )
      {
         S32 topOrBottom = ( mousePt.y > mSc->getHeight() ) ? 1 : 0;
         mSc->scrollTo( 0, topOrBottom );
         return;
      }   

      F32 percent = (F32)mousePt.y / (F32)mSc->getHeight();
      if ( percent > 0.7f && mousePt.y > mLastYvalue )
      {
         mIncValue = percent - 0.5f;
         mScrollDir = GuiScrollCtrl::DownArrow;
      }
      else if ( percent < 0.3f && mousePt.y < mLastYvalue )
      {
         mIncValue = 0.5f - percent;         
         mScrollDir = GuiScrollCtrl::UpArrow;
      }
      mLastYvalue = mousePt.y;
   }
}

//------------------------------------------------------------------------------
void GuiPopUpMenuCtrlEx::autoScroll()
{
   mScrollCount += mIncValue;

   while ( mScrollCount > 1 )
   {
      mSc->autoScroll( mScrollDir );
      mScrollCount -= 1;
   }
   mTl->onMouseMove( mEventSave );
}

//------------------------------------------------------------------------------
void GuiPopUpMenuCtrlEx::replaceText(S32 boolVal)
{
   mReplaceText = boolVal;
}
