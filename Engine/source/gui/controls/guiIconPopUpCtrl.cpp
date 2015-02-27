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
#include "gui/controls/guiIconPopUpCtrl.h"
#include "console/consoleTypes.h"
#include "gui/core/guiDefaultControlRender.h"
#include "gfx/primBuilder.h"
#include "gfx/gfxDrawUtil.h"
#include "console/engineAPI.h"

ConsoleDocClass( GuiIconPopUpMenuCtrl,
	"@brief A control that allows to select a value from a drop-down list.\n\n"

	"This is essentially a GuiPopUpMenuCtrl, but with quite a few more features.\n\n"

	"@tsexample\n"
	"new GuiIconPopUpMenuCtrl()\n"
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


GuiIconPopUpBackgroundCtrl::GuiIconPopUpBackgroundCtrl(GuiIconPopUpMenuCtrl *ctrl, GuiIconPopupListCtrl *textList)
{
   mPopUpCtrl = ctrl;
   mTextList = textList;
}

void GuiIconPopUpBackgroundCtrl::onMouseDown(const GuiEvent &event)
{
   //   mTextList->setSelectedCell(Point2I(-1,-1)); //  Removed
   mPopUpCtrl->mBackgroundCancel = true; //  Set that the user didn't click within the text list.  Replaces the line above.
   mPopUpCtrl->closePopUp();
}

//------------------------------------------------------------------------------
GuiIconPopupListCtrl::GuiIconPopupListCtrl()
{
   mPopUpCtrl = NULL;
}


//------------------------------------------------------------------------------
GuiIconPopupListCtrl::GuiIconPopupListCtrl(GuiIconPopUpMenuCtrl *ctrl)
{
   mPopUpCtrl = ctrl;
}

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//void GuiPopUpTextListCtrl::onCellSelected( Point2I /*cell*/ )
//{
//   // Do nothing, the parent control will take care of everything...
//}
void GuiIconPopupListCtrl::onCellSelected( Point2I cell )
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
}

bool GuiIconPopupListCtrl::hasCategories()
{
   for( S32 i = 0; i < mList.size(); i++)
   {
      if( mList[i].id == -1)
         return true;
   }

   return false;
}

//------------------------------------------------------------------------------
bool GuiIconPopupListCtrl::onKeyDown(const GuiEvent &event)
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

void GuiIconPopupListCtrl::onMouseUp(const GuiEvent &event)
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

void GuiIconPopupListCtrl::onMouseMove( const GuiEvent &event )
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
void GuiIconPopupListCtrl::onRenderCell(Point2I offset, Point2I cell, bool selected, bool mouseOver)
{
   Point2I size;
   getCellSize( size );

   // Render a background color for the cell
   RectI cellR( offset.x, offset.y, size.x, size.y );
   if ( mouseOver )
   {      
      GFX->getDrawUtil()->drawRectFill( cellR, mProfile->mFillColorHL );
   }
   else if ( selected )
   {
      GFX->getDrawUtil()->drawRectFill( cellR, mProfile->mFillColorSEL );
   }

   // Render 'Group' background
   if ( mList[cell.y].id == -1)
   {
      GFX->getDrawUtil()->drawRectFill( cellR, mProfile->mFillColorHL );
   }

   RectI iconBounds = mIconBounds;
   iconBounds.point += offset;
   if (mList[cell.y].mTextureObject)
	   GFX->getDrawUtil()->drawBitmapStretch(mList[cell.y].mTextureObject, iconBounds);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

IMPLEMENT_CONOBJECT(GuiIconPopUpMenuCtrl);

GuiIconPopUpMenuCtrl::GuiIconPopUpMenuCtrl(void)
{
   VECTOR_SET_ASSOCIATION(mEntries);

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
	mAutoSize = true;
	mCellSize = Point2I(32, 32);
}

//------------------------------------------------------------------------------
GuiIconPopUpMenuCtrl::~GuiIconPopUpMenuCtrl()
{
}

//------------------------------------------------------------------------------
void GuiIconPopUpMenuCtrl::initPersistFields(void)
{
   addField("maxPopupHeight",           TypeS32,          Offset(mMaxPopupHeight, GuiIconPopUpMenuCtrl), "Length of menu when it extends");
   addField("sbUsesNAColor",            TypeBool,         Offset(mRenderScrollInNA, GuiIconPopUpMenuCtrl), "Deprecated" "@internal");
   addField("reverseTextList",          TypeBool,         Offset(mReverseTextList, GuiIconPopUpMenuCtrl), "Reverses text list if popup extends up, instead of down");
   addField("bitmap",                   TypeFilename,     Offset(mBitmapName, GuiIconPopUpMenuCtrl), "File name of bitmap to use");
   addField("bitmapBounds",             TypePoint2I,      Offset(mBitmapBounds, GuiIconPopUpMenuCtrl), "Boundaries of bitmap displayed");
   addField("cellSize",					TypePoint2I,	  Offset(mCellSize, GuiIconPopUpMenuCtrl), "Maximum size of list cells");
   addField("columns",					TypeS32,		  Offset(mColumns, GuiIconPopUpMenuCtrl), "Number of cell columns");
   addField("hotTrackCallback",         TypeBool,         Offset(mHotTrackItems, GuiIconPopUpMenuCtrl),
      "Whether to provide a 'onHotTrackItem' callback when a list item is hovered over");
   addField("autoSize",                 TypeBool,         Offset(mAutoSize, GuiIconPopUpMenuCtrl), "Length of menu when it extends");

   Parent::initPersistFields();
}

//------------------------------------------------------------------------------
ConsoleDocFragment _GuiIconPopUpMenuCtrlAdd(
	"@brief Adds an entry to the list\n\n"
	"@param name String containing the name of the entry\n"
	"@param idNum Numerical value assigned to the name\n"
	"for font coloring, highlight coloring, and selection coloring\n\n",
	"GuiIconPopUpMenuCtrl",
	"void add(string name, S32 idNum);"
);

ConsoleMethod( GuiIconPopUpMenuCtrl, add, void, 3, 5, "(string name, int idNum)")
{
	if ( argc == 4 )
		object->addEntry(argv[2],dAtoi(argv[3]));
   else
      object->addEntry(argv[2]);
}

DefineEngineMethod( GuiIconPopUpMenuCtrl, clear, void, (),,
				   "@brief Clear the popup list.\n\n")
{
	object->clear();
}

DefineEngineMethod( GuiIconPopUpMenuCtrl, sort, void, (),,
				   "@brief Sort the list alphabetically.\n\n")
{
	object->sort();
}

DefineEngineMethod( GuiIconPopUpMenuCtrl, sortID, void, (),,
				   "@brief Sort the list by ID.\n\n")
{
	object->sortID();
}

DefineEngineMethod( GuiIconPopUpMenuCtrl, forceOnAction, void, (),,
				   "@brief Manually for the onAction function, which updates everything in this control.\n\n")
{
	object->onAction();
}

DefineEngineMethod( GuiIconPopUpMenuCtrl, forceClose, void, (),,
				   "@brief Manually force this control to collapse and close.\n\n")
{
	object->closePopUp();
}

DefineEngineMethod( GuiIconPopUpMenuCtrl, getSelected, S32, (),,
				   "@brief Get the current selection of the menu.\n\n"
				   "@return Returns the ID of the currently selected entry")
{
	return object->getSelected();
}

ConsoleDocFragment _GuiIconPopUpMenuCtrlsetSelected(
	"brief Manually set an entry as selected int his control\n\n"
	"@param id The ID of the entry to select\n"
	"@param scripCallback Optional boolean that forces the script callback if true\n",
	"GuiIconPopUpMenuCtrl",
	"setSelected(int id, bool scriptCallback=true);"
);

ConsoleMethod( GuiIconPopUpMenuCtrl, setSelected, void, 3, 4, "(int id, [scriptCallback=true])"
			  "@hide")
{
   if( argc > 3 )
      object->setSelected( dAtoi( argv[2] ), dAtob( argv[3] ) );
   else
      object->setSelected( dAtoi( argv[2] ) );
}

ConsoleDocFragment _GuiIconPopUpMenuCtrlsetFirstSelected(
	"brief Manually set the selection to the first entry\n\n"
	"@param scripCallback Optional boolean that forces the script callback if true\n",
	"GuiIconPopUpMenuCtrl",
	"setSelected(bool scriptCallback=true);"
);


ConsoleMethod( GuiIconPopUpMenuCtrl, setFirstSelected, void, 2, 3, "([scriptCallback=true])"
			  "@hide")
{
   if( argc > 2 )
      object->setFirstSelected( dAtob( argv[2] ) );
   else
      object->setFirstSelected();
}

DefineEngineMethod( GuiIconPopUpMenuCtrl, setNoneSelected, void, ( S32 param),,
				   "@brief Clears selection in the menu.\n\n")
{
	object->setNoneSelected();
}


DefineEngineMethod( GuiIconPopUpMenuCtrl, getTextById, const char*, (S32 id),,
				   "@brief Get the text of an entry based on an ID.\n\n"
				   "@param id The ID assigned to the entry being queried\n\n"
				   "@return String contained by the specified entry, NULL if empty or bad ID")
{
	return(object->getTextById(id));
}

ConsoleMethod( GuiIconPopUpMenuCtrl, setEnumContent, void, 4, 4,
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
      if(!dStricmp(classRep->getClassName(), argv[2]))
         break;
      classRep = classRep->getNextClass();
   }

   // get it?
   if(!classRep)
   {
      Con::warnf(ConsoleLogEntry::General, "failed to locate class rep for '%s'", argv[2]);
      return;
   }

   // walk the fields to check for this one (findField checks StringTableEntry ptrs...)
   U32 i;
   for(i = 0; i < classRep->mFieldList.size(); i++)
      if(!dStricmp(classRep->mFieldList[i].pFieldname, argv[3]))
         break;

   // found it?   
   if(i == classRep->mFieldList.size())
   {   
      Con::warnf(ConsoleLogEntry::General, "failed to locate field '%s' for class '%s'", argv[3], argv[2]);
      return;
   }

   const AbstractClassRep::Field & field = classRep->mFieldList[i];
   ConsoleBaseType* conType = ConsoleBaseType::getType( field.type );

   // check the type
   if( !conType->getEnumTable() )
   {
      Con::warnf(ConsoleLogEntry::General, "field '%s' is not an enumeration for class '%s'", argv[3], argv[2]);
      return;
   }

   // fill it
   const EngineEnumTable& table = *( conType->getEnumTable() );
   const U32 numValues = table.getNumValues();
   
   for(i = 0; i < numValues; i++)
      object->addEntry( table[i].getName(), table[i] );
}

//------------------------------------------------------------------------------
ConsoleMethod( GuiIconPopUpMenuCtrl, findText, S32, 3, 3, "(string text)"
              "Returns the id of the first entry containing the specified text or -1 if not found."
			  "@param text String value used for the query\n\n"
			  "@return Numerical ID of entry containing the text.")
{
   return( object->findText( argv[2] ) );   
}

//------------------------------------------------------------------------------
ConsoleMethod( GuiIconPopUpMenuCtrl, size, S32, 2, 2, 
			  "@brief Get the size of the menu\n\n"
			  "@return Number of entries in the menu\n")
{
   return( object->getNumEntries() ); 
}

//------------------------------------------------------------------------------
ConsoleMethod( GuiIconPopUpMenuCtrl, replaceText, void, 3, 3, 
			  "@brief Flag that causes each new text addition to replace the current entry\n\n"
			  "@param True to turn on replacing, false to disable it")
{
   object->replaceText(dAtoi(argv[2]));  
}

//------------------------------------------------------------------------------
//  Added
bool GuiIconPopUpMenuCtrl::onWake()
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
bool GuiIconPopUpMenuCtrl::onAdd()
{
   if ( !Parent::onAdd() )
      return false;
   mSelIndex = -1;
   mReplaceText = true;
   return true;
}

//------------------------------------------------------------------------------
void GuiIconPopUpMenuCtrl::onSleep()
{
   mTextureNormal = NULL; //  Added
   mTextureDepressed = NULL; //  Added
   Parent::onSleep();
   closePopUp();  // Tests in function.
}

//------------------------------------------------------------------------------
void GuiIconPopUpMenuCtrl::clear()
{
   mEntries.setSize(0);
   setSelectedIcon("");
   mSelIndex = -1;
   mRevNum = 0;
	mIdMax = -1;
}

//------------------------------------------------------------------------------
void GuiIconPopUpMenuCtrl::clearEntry( S32 entry )
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
		setSelectedIcon("");
		mSelIndex = -1;
		mRevNum = 0;
	}
	else
	{
		if( entry == mSelIndex )
		{
			setSelectedIcon("");
			mSelIndex = -1;
		}
		else
		{
			mSelIndex--;
		}
	}
}

//------------------------------------------------------------------------------
ConsoleMethod( GuiIconPopUpMenuCtrl, clearEntry, void, 3, 3, "(S32 entry)")
{
   object->clearEntry(dAtoi(argv[2]));  
}

//------------------------------------------------------------------------------
static S32 QSORT_CALLBACK pathCompare(const void *a,const void *b)
{
   GuiIconPopUpMenuCtrl::Entry *ea = (GuiIconPopUpMenuCtrl::Entry *) (a);
   GuiIconPopUpMenuCtrl::Entry *eb = (GuiIconPopUpMenuCtrl::Entry *) (b);
   return (dStrnatcasecmp(ea->iconPath, eb->iconPath));
} 

//  Added to sort by entry ID
//------------------------------------------------------------------------------
static S32 QSORT_CALLBACK idCompare(const void *a,const void *b)
{
   GuiIconPopUpMenuCtrl::Entry *ea = (GuiIconPopUpMenuCtrl::Entry *) (a);
   GuiIconPopUpMenuCtrl::Entry *eb = (GuiIconPopUpMenuCtrl::Entry *) (b);
   return ( (ea->id < eb->id) ? -1 : ((ea->id > eb->id) ? 1 : 0) );
} 

//------------------------------------------------------------------------------
//  Added
void GuiIconPopUpMenuCtrl::setBitmap(const char *name)
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
void GuiIconPopUpMenuCtrl::setSelectedIcon(const String& iconPath)
{
	mSelectedIconPath = iconPath;
	if ( iconPath.isNotEmpty() )
	{
		if ( !iconPath.equal("texhandle", String::NoCase) )
			mSelectedIconTexture.set( iconPath, &GFXDefaultGUIProfile, avar("%s() - mTextureObject (line %d)", __FUNCTION__, __LINE__) );
	}
	else
		mSelectedIconTexture = NULL;
}

//------------------------------------------------------------------------------
void GuiIconPopUpMenuCtrl::sort()
{
   S32 size = mEntries.size();
   if( size > 0 )
      dQsort( mEntries.address(), size, sizeof(Entry), pathCompare);

	// Entries need to re-Id themselves
	for( U32 i = 0; i < mEntries.size(); i++ )
		mEntries[i].id = i;
}

//  Added to sort by entry ID
//------------------------------------------------------------------------------
void GuiIconPopUpMenuCtrl::sortID()
{
   S32 size = mEntries.size();
   if( size > 0 )
      dQsort( mEntries.address(), size, sizeof(Entry), idCompare);

	// Entries need to re-Id themselves
	for( U32 i = 0; i < mEntries.size(); i++ )
		mEntries[i].id = i;
}

//------------------------------------------------------------------------------
void GuiIconPopUpMenuCtrl::addEntry(const String& iconPath, S32 id)
{
   if (iconPath.isEmpty())
   {
      return;
   }
	
	// Ensure that there are no other entries with exactly the same name
	for ( U32 i = 0; i < mEntries.size(); i++ )
   {
      if ( dStrcmp( mEntries[i].iconPath, iconPath ) == 0 )
         return;
   }

	// If we don't give an id, create one from mIdMax
	if( id == -1 )
		id = mIdMax + 1;
	
	// Increase mIdMax when an id is greater than it
	if( id > mIdMax )
		mIdMax = id;

   Entry e;
   e.iconPath = iconPath;
   e.id = id;

   mEntries.push_back(e);

   if ( mInAction && mTl )
   {
      // Add the new entry:
      mTl->addEntry( e.id, e.iconPath );
      repositionPopup();
   }
}

//------------------------------------------------------------------------------
S32 GuiIconPopUpMenuCtrl::getSelected()
{
   if (mSelIndex == -1)
      return 0;
   return mEntries[mSelIndex].id;
}

//------------------------------------------------------------------------------
const String& GuiIconPopUpMenuCtrl::getTextById(S32 id)
{
   for ( U32 i = 0; i < mEntries.size(); i++ )
   {
      if ( mEntries[i].id == id )
         return( mEntries[i].iconPath );
   }

   return String::EmptyString;
}

//------------------------------------------------------------------------------
S32 GuiIconPopUpMenuCtrl::findText( const char* text )
{
   for ( U32 i = 0; i < mEntries.size(); i++ )
   {
      if ( dStrcmp( text, mEntries[i].iconPath ) == 0 )
         return( mEntries[i].id );        
   }
   return( -1 );
}

//------------------------------------------------------------------------------
void GuiIconPopUpMenuCtrl::setSelected(S32 id, bool bNotifyScript )
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
            setSelectedIcon(mEntries[i].iconPath);
         }

         // Now perform the popup action:
         char idval[24];
         dSprintf( idval, sizeof(idval), "%d", mEntries[mSelIndex].id );
         if ( isMethod( "onSelect" ) && bNotifyScript )
            Con::executef( this, "onSelect", idval, mEntries[mSelIndex].iconPath );
         return;
      }
   }

   if ( mReplaceText ) //  Only change the displayed text if appropriate.
   {
      setSelectedIcon("");
   }
   mSelIndex = -1;

   if ( isMethod( "onCancel" ) && bNotifyScript )
      Con::executef( this, "onCancel" );

   if ( id == -1 )
      return;

   // Execute the popup console command:
   if ( bNotifyScript )
      execConsoleCallback();
}

//------------------------------------------------------------------------------
//  Added to set the first item as selected.
void GuiIconPopUpMenuCtrl::setFirstSelected( bool bNotifyScript )
{
   if ( mEntries.size() > 0 )
   {
      mSelIndex = 0;
      if ( mReplaceText ) //  Only change the displayed text if appropriate.
      {
         setSelectedIcon( mEntries[0].iconPath );
      }

      // Now perform the popup action:
      char idval[24];
      dSprintf( idval, sizeof(idval), "%d", mEntries[mSelIndex].id );
      if ( isMethod( "onSelect" ) )
         Con::executef( this, "onSelect", idval, mEntries[mSelIndex].iconPath );
      
		// Execute the popup console command:
		if ( bNotifyScript )
			execConsoleCallback();
   }
	else
	{
		if ( mReplaceText ) //  Only change the displayed text if appropriate.
			setSelectedIcon("");
		
		mSelIndex = -1;

		if ( bNotifyScript )
			Con::executef( this, "onCancel" );
	}
}

//------------------------------------------------------------------------------
//  Added to set no items as selected.
void GuiIconPopUpMenuCtrl::setNoneSelected()
{
   if ( mReplaceText ) //  Only change the displayed text if appropriate.
   {
      setSelectedIcon("");
   }
   mSelIndex = -1;
}

//------------------------------------------------------------------------------
const char *GuiIconPopUpMenuCtrl::getScriptValue()
{
   return mSelectedIconPath.c_str();
}

//------------------------------------------------------------------------------
bool GuiIconPopUpMenuCtrl::resize(const Point2I& newPosition, const Point2I& newExtent)
{
	Point2I autoSize = newExtent;

	// Auto-size height of control based on bitmap array height
	if (mAutoSize)
	{
		if (!mAwake)
		{
			mProfile->incLoadCount();
			if( !mProfile->mBitmapArrayRects.size() )
				mProfile->constructBitmapArray();
		}

		if (mProfile->mBitmapArrayRects.size() > 0)
			autoSize.y = mProfile->mBitmapArrayRects[0].extent.y;

		if (!mAwake)
			mProfile->decLoadCount();
	}

	return Parent::resize(newPosition, autoSize);
}

//------------------------------------------------------------------------------
void GuiIconPopUpMenuCtrl::onRender(Point2I offset, const RectI &updateRect)
{
   TORQUE_UNUSED(updateRect);
   Point2I localStart;

   if ( mScrollDir != GuiScrollCtrl::None )
      autoScroll();

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
         GFX->getDrawUtil()->drawRectFill( r, mProfile->mFillColor );
      }

      //  Draw a bitmap over the background?
      if ( mTextureDepressed )
      {
         RectI rect(offset, mBitmapBounds);
         GFX->getDrawUtil()->clearBitmapModulation();
         GFX->getDrawUtil()->drawBitmapStretch( mTextureDepressed, rect );
      } 
      else if ( mTextureNormal )
      {
         RectI rect(offset, mBitmapBounds);
         GFX->getDrawUtil()->clearBitmapModulation();
         GFX->getDrawUtil()->drawBitmapStretch( mTextureNormal, rect );
      }

      // Do we render a bitmap border or lines?
      if ( !( mProfile->getChildrenProfile() && mProfile->mBitmapArrayRects.size() ) )
      {
         GFX->getDrawUtil()->drawLine( l, t, l, b, colorWhite );
         GFX->getDrawUtil()->drawLine( l, t, r2, t, colorWhite );
         GFX->getDrawUtil()->drawLine( l + 1, b, r2, b, mProfile->mBorderColor );
         GFX->getDrawUtil()->drawLine( r2, t + 1, r2, b - 1, mProfile->mBorderColor );
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
            GFX->getDrawUtil()->drawRectFill( r, mProfile->mFillColorHL );
         }

         //  Draw a bitmap over the background?
         if ( mTextureNormal )
         {
            RectI rect( offset, mBitmapBounds );
            GFX->getDrawUtil()->clearBitmapModulation();
            GFX->getDrawUtil()->drawBitmapStretch( mTextureNormal, rect );
         }

         // Do we render a bitmap border or lines?
         if ( !( mProfile->getChildrenProfile() && mProfile->mBitmapArrayRects.size() ) )
         {
            GFX->getDrawUtil()->drawLine( l, t, l, b, colorWhite );
            GFX->getDrawUtil()->drawLine( l, t, r2, t, colorWhite );
            GFX->getDrawUtil()->drawLine( l + 1, b, r2, b, mProfile->mBorderColor );
            GFX->getDrawUtil()->drawLine( r2, t + 1, r2, b - 1, mProfile->mBorderColor );
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
            GFX->getDrawUtil()->drawRectFill( r, mProfile->mFillColorNA );
         }

         //  Draw a bitmap over the background?
         if ( mTextureNormal )
         {
            RectI rect(offset, mBitmapBounds);
            GFX->getDrawUtil()->clearBitmapModulation();
            GFX->getDrawUtil()->drawBitmapStretch( mTextureNormal, rect );
         }

         // Do we render a bitmap border or lines?
         if ( !( mProfile->getChildrenProfile() && mProfile->mBitmapArrayRects.size() ) )
         {
            GFX->getDrawUtil()->drawRect( r, mProfile->mBorderColorNA );
         }
      }
      //      renderSlightlyRaisedBox(r, mProfile); //  Used to be the only 'else' condition to mInAction above.

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
            RectI* mBitmapBounds = mProfile->mBitmapArrayRects.address();
            localStart.x = getWidth() - mBitmapBounds[2].extent.x - mCellSize.x;
         } 
         else
         {
            localStart.x = getWidth() - mCellSize.x;  
         }
         break;
      case GuiControlProfile::CenterJustify:
         if ( mProfile->getChildrenProfile() && mProfile->mBitmapArrayRects.size() )
         {
            // We're making use of a bitmap border, so take into account the
            // right cap of the border.
            RectI* mBitmapBounds = mProfile->mBitmapArrayRects.address();
            localStart.x = (getWidth() - mBitmapBounds[2].extent.x - mCellSize.x) / 2;

         } else
         {
            localStart.x = (getWidth() - mCellSize.x) / 2;
         }
         break;
      default:
         // GuiControlProfile::LeftJustify
         if ( mCellSize.x > getWidth() )
         {
            //  The width of the text is greater than the width of the control.
            // In this case we will right justify the text and leave some space
            // for the down arrow.
            if ( mProfile->getChildrenProfile() && mProfile->mBitmapArrayRects.size() )
            {
               // We're making use of a bitmap border, so take into account the
               // right cap of the border.
               RectI* mBitmapBounds = mProfile->mBitmapArrayRects.address();
               localStart.x = getWidth() - mBitmapBounds[2].extent.x - mCellSize.x;
            } 
            else
            {
               localStart.x = getWidth() - mCellSize.x - 12;
            }
         } 
         else
         {
            localStart.x = mProfile->mTextOffset.x; //  Use mProfile->mTextOffset as a controlable margin for the control's text.
         }
         break;
      }

      // Draw the text
      Point2I globalStart = localToGlobalCoord( localStart );
      ColorI fontColor   = mActive ? ( mInAction ? mProfile->mFontColor : mProfile->mFontColorNA ) : mProfile->mFontColorNA;
      GFX->getDrawUtil()->setBitmapModulation( fontColor ); //  was: (mProfile->mFontColor);

      //  Get the number of columns in the text
      S32 colcount = mColumns;

      // If we're rendering a bitmap border, then it will take care of the arrow.
      if ( !(mProfile->getChildrenProfile() && mProfile->mBitmapArrayRects.size()) )
      {
         //  Draw a triangle (down arrow)
         S32 left = r.point.x + r.extent.x - (arrowWidth + arrowMargin);
         S32 right = left + arrowWidth;
         S32 middle = left + arrowMargin;
         S32 top = r.extent.y / 2 + r.point.y - 4;
         S32 bottom = top + 8;

         PrimBuild::color( mProfile->mFontColor );

         PrimBuild::begin( GFXTriangleList, 3 );
            PrimBuild::vertex2fv( Point3F( (F32)left, (F32)top, 0.0f ) );
            PrimBuild::vertex2fv( Point3F( (F32)right, (F32)top, 0.0f ) );                                                                                                        
            PrimBuild::vertex2fv( Point3F( (F32)middle, (F32)bottom, 0.0f ) );
         PrimBuild::end();
      }

	  if (mSelectedIconTexture.isValid())
	  {
		  RectI iconRect(globalStart, mCellSize);
		  RectI boundsRect = getBounds();
		  boundsRect.extent.x -= arrowMargin + arrowWidth + arrowMargin;
		  iconRect.AlignInside(boundsRect, Align::CenterMiddle, 0, false);
		  GFX->getDrawUtil()->drawBitmapStretch(mSelectedIconTexture, iconRect);
	  }
}

//------------------------------------------------------------------------------
void GuiIconPopUpMenuCtrl::closePopUp()
{
   if ( !mInAction )
      return;

   // Get the selection from the text list:
   mSelIndex = mTl->getSelectedCell().y;
   mSelIndex = ( mRevNum >= mSelIndex && mSelIndex != -1 ) ? mRevNum - mSelIndex : mSelIndex;
   if ( mSelIndex != -1 )
   {
      if ( mReplaceText )
         setSelectedIcon( mEntries[mSelIndex].iconPath );
      setIntVariable( mEntries[mSelIndex].id );
   }

   // Release the mouse:
   mInAction = false;
   mTl->mouseUnlock();

   // Now perform the popup action:
   if ( mSelIndex != -1 )
   {
      char idval[24];
      dSprintf( idval, sizeof(idval), "%d", mEntries[mSelIndex].id );
      if ( isMethod( "onSelect" ) )
         Con::executef( this, "onSelect", idval, mEntries[mSelIndex].iconPath );
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
bool GuiIconPopUpMenuCtrl::onKeyDown(const GuiEvent &event)
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
void GuiIconPopUpMenuCtrl::onAction()
{
   GuiControl *canCtrl = getParent();

   addChildren();

   GuiCanvas *root = getRoot();
   Point2I windowExt = root->getExtent();

   mBackground->resize( Point2I(0,0), root->getExtent() );
   
   S32 cellWidth = mCellSize.x;
   S32 width = getWidth();
   const S32 textSpace = 2;
   bool setScroll = false;

   //if(textWidth > getWidth())
   S32 sbWidth = mSc->getControlProfile()->mBorderThickness * 2 + mSc->scrollBarThickness(); //  Calculate the scroll bar width
   if ( cellWidth > ( getWidth() - sbWidth-mProfile->mTextOffset.x - mSc->getChildMargin().x * 2 ) ) //  The text draw area to test against is the width of the drop-down minus the scroll bar width, the text margin and the scroll bar child margins.
   {
      cellWidth +=sbWidth + mSc->getChildMargin().x * 2; //  The new width is the width of the text plus the scroll bar width plus the text margin size plus the scroll bar child margins.
      width = cellWidth;

      //  If a child margin is not defined for the scroll control, let's add
      //      some space between the text and scroll control for readability
      if(mSc->getChildMargin().x == 0)
         width += textSpace;
   }

   Point2I cellSize = Point2I(width, mCellSize.y);
   mTl->setCellSize(cellSize);

   RectI cellBounds(Point2I::Zero, Point2I(cellSize.x - (arrowWidth + arrowMargin + arrowMargin), cellSize.y));
   RectI iconBounds(Point2I::Zero, mCellSize);
   iconBounds.AlignInside(cellBounds, Align::CenterMiddle);
   mTl->setIconBounds(iconBounds);

   for ( U32 j = 0; j < mEntries.size(); ++j )
      mTl->addEntry( mEntries[j].id, mEntries[j].iconPath );

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
            //Pop menu list above the button
            //            scrollPoint.set(pointInGC.x, menuSpace - 1); //  Removed as calculated outside the 'if', and was wrong to begin with
            setScroll = true;
         }
         //No scroll bar needed
         else
         {
            maxYdis = mTl->getHeight() + sbBorder; //  Instead of adding sbBorder, it was: 'textSpace'
         }

         //  Added the next two lines
         scrollPoint.set(pointInGC.x, pointInGC.y - maxYdis); //  Used to have the following on the end: '-1);'
      } 
      //Scroll bar needed but Don't pop above button
      else
      {
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
void GuiIconPopUpMenuCtrl::addChildren()
{
   mTl = new GuiIconPopupListCtrl( this );
   AssertFatal( mTl, "Failed to create the GuiIconPopupTextListCtrl for the PopUpMenu" );
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

   mBackground = new GuiIconPopUpBackgroundCtrl( this, mTl );
   AssertFatal( mBackground, "Failed to create the GuiBackgroundCtrlEx for the PopUpMenu" );
}

//------------------------------------------------------------------------------
void GuiIconPopUpMenuCtrl::repositionPopup()
{
   if ( !mInAction || !mSc || !mTl )
      return;

   // I'm not concerned with this right now...
}

//------------------------------------------------------------------------------
void GuiIconPopUpMenuCtrl::reverseTextList()
{
   mTl->clear();
   for ( S32 i = mEntries.size()-1; i >= 0; --i )
      mTl->addEntry( mEntries[i].id, mEntries[i].iconPath );

   // Don't lose the selected cell:
   if ( mSelIndex >= 0 )
      mTl->setSelectedCell( Point2I( 0, mEntries.size() - mSelIndex - 1 ) ); 

   mRevNum = mEntries.size() - 1;
}

//------------------------------------------------------------------------------
bool GuiIconPopUpMenuCtrl::getFontColor( ColorI &fontColor, S32 id, bool selected, bool mouseOver )
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

   if(id == -1)
      fontColor = mProfile->mFontColorHL;
   else
	   fontColor = selected ? mProfile->mFontColorSEL : mouseOver ? mProfile->mFontColorHL : mProfile->mFontColorNA; //  Modified the final color choice from mProfile->mFontColor to mProfile->mFontColorNA

   return( true );
}

//------------------------------------------------------------------------------
void GuiIconPopUpMenuCtrl::onMouseDown(const GuiEvent &event)
{
   TORQUE_UNUSED(event);
   onAction();
}

//------------------------------------------------------------------------------
void GuiIconPopUpMenuCtrl::onMouseUp(const GuiEvent &event)
{
   TORQUE_UNUSED(event);
}

//------------------------------------------------------------------------------
//  Added
void GuiIconPopUpMenuCtrl::onMouseEnter(const GuiEvent &event)
{
   mMouseOver = true;
}

//------------------------------------------------------------------------------
//  Added
void GuiIconPopUpMenuCtrl::onMouseLeave(const GuiEvent &)
{
   mMouseOver = false;
}

//------------------------------------------------------------------------------
void GuiIconPopUpMenuCtrl::setupAutoScroll(const GuiEvent &event)
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
void GuiIconPopUpMenuCtrl::autoScroll()
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
void GuiIconPopUpMenuCtrl::replaceText(S32 boolVal)
{
   mReplaceText = boolVal;
}
