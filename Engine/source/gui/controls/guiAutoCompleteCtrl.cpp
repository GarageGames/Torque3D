#include "gui/core/guiCanvas.h"
#include "gui/controls/guiAutoCompleteCtrl.h"
#include "console/consoleTypes.h"
#include "gui/core/guiDefaultControlRender.h"
#include "gfx/primBuilder.h"
#include "gfx/gfxDrawUtil.h"
#include "console/engineAPI.h"

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

PopUpBackgroundCtrl::PopUpBackgroundCtrl(GuiAutoCompleteCtrl *ctrl, PopupTextListCtrl *textList)
{
   mPopUpCtrl = ctrl;
   mTextList = textList;
}

void PopUpBackgroundCtrl::onMouseDown(const GuiEvent &event)
{
   mPopUpCtrl->mBackgroundCancel = true; //  Set that the user didn't click within the text list.  Replaces the line above.
   mPopUpCtrl->closePopUp();
}

//------------------------------------------------------------------------------
PopupTextListCtrl::PopupTextListCtrl()
{
   mPopUpCtrl = NULL;
   mLastPositionInside = false;
}


//------------------------------------------------------------------------------
PopupTextListCtrl::PopupTextListCtrl(GuiAutoCompleteCtrl *ctrl)
{
   mPopUpCtrl = ctrl;
   mLastPositionInside = false;
}

//------------------------------------------------------------------------------

void PopupTextListCtrl::onCellSelected( Point2I cell )
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

void PopupTextListCtrl::onMouseMove(const GuiEvent &event)
{
   Point2I pt = globalToLocalCoord(event.mousePoint);
   pt.x -= mHeaderDim.x; pt.y -= mHeaderDim.y;
   Point2I cell((pt.x < 0 ? -1 : pt.x / mCellSize.x), (pt.y < 0 ? -1 : pt.y / mCellSize.y));
   if (( cell.y > -1 && cell.y < mList.size() && cell.x == 0))
   {
	   mLastPositionInside = true;

	   if( (cell.x == mMouseOverCell.x && cell.y == mMouseOverCell.y) )
		   return;

      if (mMouseOverCell.x != -1)
      {
         setUpdateRegion(Point2I(mMouseOverCell.x * mCellSize.x + mHeaderDim.x,
                           mMouseOverCell.y * mCellSize.y + mHeaderDim.y), mCellSize);
      }

      if (cell.x >= 0 && cell.x < mSize.x && cell.y >= 0 && cell.y < mSize.y)
      {
         setUpdateRegion(Point2I(cell.x * mCellSize.x + mHeaderDim.x,
                           cell.y * mCellSize.y + mHeaderDim.y), mCellSize);
         mMouseOverCell = cell;
      }
   }
   else
   {
	   if( mLastPositionInside )
		   mMouseOverCell.set(-1,-1);

	   mLastPositionInside = false;
   }
   onCellHighlighted(mMouseOverCell);
}

//------------------------------------------------------------------------------
bool PopupTextListCtrl::onKeyDown(const GuiEvent &event)
{
   //if the control is a dead end, don't process the input:
   if ( !mVisible || !mActive || !mAwake )
      return false;

   //see if the key down is a <return> or not
   if ( event.modifier == 0 )
   {
	   if( event.keyCode == KEY_UP )
	   {
		   mMouseOverCell.set( 0, (mMouseOverCell.y - 1 < 0 ) ? 0 : mMouseOverCell.y - 1 );
		   onCellHighlighted(mMouseOverCell);
	   }
	   if( event.keyCode == KEY_DOWN )
	   {
		   mMouseOverCell.set( 0, (mMouseOverCell.y + 1 > mList.size() - 1 ) ? mList.size() - 1 : mMouseOverCell.y + 1 );
		   onCellHighlighted(mMouseOverCell);
	   } 
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

void PopupTextListCtrl::onMouseUp(const GuiEvent &event)
{
   Parent::onMouseUp( event );
   mPopUpCtrl->closePopUp();
}

//------------------------------------------------------------------------------
void PopupTextListCtrl::onRenderCell(Point2I offset, Point2I cell, bool selected, bool mouseOver)
{
   Point2I size;
   getCellSize( size );

   // Render a background color for the cell
   if ( mouseOver )
   {      
      RectI cellR( offset.x, offset.y, size.x, size.y );
	  GFX->getDrawUtil()->drawRectFill(cellR, mProfile->mFillColorHL);

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
   bool drawbox = mPopUpCtrl->getColoredBox( boxColor, mList[cell.y].id);
   if(drawbox)
   {
      Point2I coloredboxsize(15,10);
      RectI r(offset.x + mProfile->mTextOffset.x, offset.y+2, coloredboxsize.x, coloredboxsize.y);
      GFX->getDrawUtil()->drawRectFill( r, boxColor);
      GFX->getDrawUtil()->drawRect( r, ColorI(0,0,0));

      textXOffset += coloredboxsize.x + mProfile->mTextOffset.x;
   }

   ColorI fontColor;
   mPopUpCtrl->getFontColor( fontColor, mList[cell.y].id, selected, mouseOver );

   GFX->getDrawUtil()->setBitmapModulation( ColorI(0, 0, 0) );
 
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
      GFX->getDrawUtil()->drawText( mFont, Point2I( textXOffset, offset.y ), mList[cell.y].text ); //  Used mTextOffset as a margin for the text list rather than the hard coded value of '4'.
   }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

IMPLEMENT_CONOBJECT(GuiAutoCompleteCtrl);

ConsoleDocClass( GuiAutoCompleteCtrl,
	"@brief A control that allows to select a value from a drop-down list.\n\n"

	"For a nearly identical GUI with additional features, use GuiAutoCompleteCtrlEx.\n\n"

	"@tsexample\n"
	"new GuiAutoCompleteCtrl()\n"
	"{\n"
	"	maxPopupHeight = \"200\";\n"
	"	sbUsesNAColor = \"0\";\n"
	"	reverseTextList = \"0\";\n"
	"	bitmapBounds = \"16 16\";\n"
	"	maxLength = \"1024\";\n"
	"	position = \"56 31\";\n"
	"	extent = \"64 64\";\n"
	"	minExtent = \"8 2\";\n"
	"	profile = \"GuiPopUpMenuProfile\";\n"
	"	tooltipProfile = \"GuiToolTipProfile\";\n"
	"};\n"
	"@endtsexample\n\n"

	"@note This is definitely going to be deprecated soon.\n\n"

	"@see GuiAutoCompleteCtrlEx for more features and better explanations.\n"

	"@ingroup GuiControls\n");

GuiAutoCompleteCtrl::GuiAutoCompleteCtrl(void)
{
   VECTOR_SET_ASSOCIATION(mEntries);
   VECTOR_SET_ASSOCIATION(mFilteredEntries);
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
	mIdMax = -1;
}

//------------------------------------------------------------------------------
GuiAutoCompleteCtrl::~GuiAutoCompleteCtrl()
{
}

//------------------------------------------------------------------------------
void GuiAutoCompleteCtrl::initPersistFields(void)
{
   addField("maxPopupHeight",           TypeS32,          Offset(mMaxPopupHeight, GuiAutoCompleteCtrl));
   addField("sbUsesNAColor",            TypeBool,         Offset(mRenderScrollInNA, GuiAutoCompleteCtrl));
   addField("reverseTextList",          TypeBool,         Offset(mReverseTextList, GuiAutoCompleteCtrl));
   addField("bitmap",                   TypeFilename,     Offset(mBitmapName, GuiAutoCompleteCtrl));
   addField("bitmapBounds",             TypePoint2I,      Offset(mBitmapBounds, GuiAutoCompleteCtrl));

   Parent::initPersistFields();
}

//------------------------------------------------------------------------------
ConsoleMethod( GuiAutoCompleteCtrl, add, void, 3, 5, "(string name, int idNum, int scheme=0)")
{
	if ( argc == 4 )
		object->addEntry(argv[2],dAtoi(argv[3]));
   if ( argc == 5 )
      object->addEntry(argv[2],dAtoi(argv[3]),dAtoi(argv[4]));
   else
      object->addEntry(argv[2]);
}

ConsoleMethod( GuiAutoCompleteCtrl, addScheme, void, 6, 6, "(int id, ColorI fontColor, ColorI fontColorHL, ColorI fontColorSEL)")
{
   ColorI fontColor, fontColorHL, fontColorSEL;
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
   fontColorSEL.set( r, g, b );

   object->addScheme( dAtoi( argv[2] ), fontColor, fontColorHL, fontColorSEL );
}

ConsoleMethod( GuiAutoCompleteCtrl, getText, void, 2, 2, "")
{
   object->getText("");
}

ConsoleMethod( GuiAutoCompleteCtrl, clear, void, 2, 2, "Clear the popup list.")
{
   object->clear();
}

//FIXME: clashes with SimSet.sort
ConsoleMethod(GuiAutoCompleteCtrl, sort, void, 2, 2, "Sort the list alphabetically.")
{
   object->sort();
}

//  Added to sort the entries by ID
ConsoleMethod(GuiAutoCompleteCtrl, sortID, void, 2, 2, "Sort the list by ID.")
{
   object->sortID();
}

ConsoleMethod( GuiAutoCompleteCtrl, forceOnAction, void, 2, 2, "")
{
   object->onAction();
}

ConsoleMethod( GuiAutoCompleteCtrl, forceClose, void, 2, 2, "")
{
   object->closePopUp();
}

ConsoleMethod( GuiAutoCompleteCtrl, getSelected, S32, 2, 2, "")
{
   return object->getSelected();
}

ConsoleMethod( GuiAutoCompleteCtrl, setSelected, void, 3, 4, "(int id, [scriptCallback=true])")
{
   if( argc > 3 )
      object->setSelected( dAtoi( argv[2] ), dAtob( argv[3] ) );
   else
      object->setSelected( dAtoi( argv[2] ) );
}

ConsoleMethod( GuiAutoCompleteCtrl, setFirstSelected, void, 2, 3, "([scriptCallback=true])")
{
	if( argc > 2 )
      object->setFirstSelected( dAtob( argv[2] ) );
   else
      object->setFirstSelected();
}

ConsoleMethod( GuiAutoCompleteCtrl, setNoneSelected, void, 2, 2, "")
{
   object->setNoneSelected();
}

ConsoleMethod( GuiAutoCompleteCtrl, getTextById, const char*, 3, 3,  "(int id)")
{
   return(object->getTextById(dAtoi(argv[2])));
}

ConsoleMethod( GuiAutoCompleteCtrl, changeTextById, void, 4, 4, "( int id, string text )" )
{
   object->setEntryText( dAtoi( argv[ 2 ] ), argv[ 3 ] );
}

ConsoleMethod( GuiAutoCompleteCtrl, setEnumContent, void, 4, 4, "(string class, string enum)"
              "This fills the popup with a classrep's field enumeration type info.\n\n"
              "More of a helper function than anything.   If console access to the field list is added, "
              "at least for the enumerated types, then this should go away..")
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
ConsoleMethod( GuiAutoCompleteCtrl, findText, S32, 3, 3, "(string text)"
              "Returns the position of the first entry containing the specified text.")
{
   return( object->findText( argv[2] ) );   
}

//------------------------------------------------------------------------------
ConsoleMethod( GuiAutoCompleteCtrl, size, S32, 2, 2, "Get the size of the menu - the number of entries in it.")
{
   return( object->getNumEntries() ); 
}

//------------------------------------------------------------------------------
ConsoleMethod( GuiAutoCompleteCtrl, replaceText, void, 3, 3, "(bool doReplaceText)")
{
   object->replaceText(dAtoi(argv[2]));  
}

//------------------------------------------------------------------------------
//  Added
bool GuiAutoCompleteCtrl::onWake()
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
bool GuiAutoCompleteCtrl::onAdd()
{
   if ( !Parent::onAdd() )
      return false;
   mSelIndex = -1;
   mReplaceText = true;
   mProfile = dynamic_cast<GuiControlProfile *>(Sim::findObject( "GuiTextEditProfile"));
   return true;
}

//------------------------------------------------------------------------------
void GuiAutoCompleteCtrl::onSleep()
{
   mTextureNormal = NULL; //  Added
   mTextureDepressed = NULL; //  Added
   Parent::onSleep();
   closePopUp();  // Tests in function.
}

//------------------------------------------------------------------------------
void GuiAutoCompleteCtrl::clear()
{
   mEntries.setSize(0);
   mFilteredEntries.setSize(0);
   setText("");
   mSelIndex = -1;
   mRevNum = 0;
	mIdMax = -1;
}

//------------------------------------------------------------------------------
void GuiAutoCompleteCtrl::clearEntry( S32 entry )
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
		if (entry < mSelIndex)
		{
			mSelIndex--;
		}
		else if( entry == mSelIndex )
		{
			setText("");
			mSelIndex = -1;
		}
	}
}

//------------------------------------------------------------------------------
ConsoleMethod( GuiAutoCompleteCtrl, clearEntry, void, 3, 3, "(S32 entry)")
{
   object->clearEntry(dAtoi(argv[2]));  
}

//------------------------------------------------------------------------------
static S32 QSORT_CALLBACK textCompare(const void *a,const void *b)
{
   GuiAutoCompleteCtrl::Entry *ea = (GuiAutoCompleteCtrl::Entry *) (a);
   GuiAutoCompleteCtrl::Entry *eb = (GuiAutoCompleteCtrl::Entry *) (b);
   return (dStrnatcasecmp(ea->buf, eb->buf));
} 

//  Added to sort by entry ID
//------------------------------------------------------------------------------
static S32 QSORT_CALLBACK idCompare(const void *a,const void *b)
{
   GuiAutoCompleteCtrl::Entry *ea = (GuiAutoCompleteCtrl::Entry *) (a);
   GuiAutoCompleteCtrl::Entry *eb = (GuiAutoCompleteCtrl::Entry *) (b);
   return ( (ea->id < eb->id) ? -1 : ((ea->id > eb->id) ? 1 : 0) );
} 

//------------------------------------------------------------------------------
//  Added
void GuiAutoCompleteCtrl::setBitmap( const char *name )
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
void GuiAutoCompleteCtrl::sort()
{
   S32 selId = getSelected();
   
   S32 size = mEntries.size();
   if( size > 0 )
      dQsort( mEntries.address(), size, sizeof(Entry), textCompare);
      
   if( selId != -1 )
      setSelected( selId, false );
}

// Added to sort by entry ID
//------------------------------------------------------------------------------
void GuiAutoCompleteCtrl::sortID()
{
   S32 selId = getSelected();
   
   S32 size = mEntries.size();
   if( size > 0 )
      dQsort( mEntries.address(), size, sizeof(Entry), idCompare);

   if( selId != -1 )
      setSelected( selId, false );
}

//------------------------------------------------------------------------------
void GuiAutoCompleteCtrl::addEntry( const char *buf, S32 id, U32 scheme )
{
   if( !buf )
   {
      //Con::printf( "GuiAutoCompleteCtrlEx::addEntry - Invalid buffer!" );
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
}

//------------------------------------------------------------------------------
void GuiAutoCompleteCtrl::addScheme( U32 id, ColorI fontColor, ColorI fontColorHL, ColorI fontColorSEL )
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
S32 GuiAutoCompleteCtrl::getSelected()
{
   if (mSelIndex == -1)
      return 0;
   return mFilteredEntries[mSelIndex].id;
}

//------------------------------------------------------------------------------
bool GuiAutoCompleteCtrl::setEntryText( S32 id, const char* buf )
{
   const U32 numEntries = getNumEntries();
   for( U32 i = 0; i < numEntries; i++ )
   {
      if( mEntries[ i ].id == id )
      {
         Entry& entry = mEntries[ i ];
         dStrncpy( entry.buf, buf, sizeof( entry.buf ) );
         entry.buf[ sizeof( entry.buf ) - 1 ] = '\0';
         return true;
      }
   }

   return false;
}

//------------------------------------------------------------------------------
const char* GuiAutoCompleteCtrl::getTextById(S32 id)
{
   for ( U32 i = 0; i < mEntries.size(); i++ )
   {
      if ( mEntries[i].id == id )
         return( mEntries[i].buf );
   }

   return( "" );
}

//------------------------------------------------------------------------------
S32 GuiAutoCompleteCtrl::findText( const char* text )
{
   for ( U32 i = 0; i < mEntries.size(); i++ )
   {
      if ( dStrcmp( text, mEntries[i].buf ) == 0 )
         return( mEntries[i].id );        
   }
   return( -1 );
}

//------------------------------------------------------------------------------
void GuiAutoCompleteCtrl::setSelected(S32 id, bool bNotifyScript )
{
   for( S32 i = 0; i < mEntries.size(); i++ )
   {
      if( id == mEntries[i].id )
      {
         i = ( mRevNum > i ) ? mRevNum - i : i;
         mSelIndex = i;
         
		 if (mReplaceText) //  Only change the displayed text if appropriate.
			 //setText( mEntries[ i ].buf );
			 setText(mFilteredEntries[i].buf);

         // Now perform the popup action:
         
         if( bNotifyScript )
         {
            if( isMethod( "onSelect" ) )

			   Con::executef(this, "onSelect", Con::getIntArg(mFilteredEntries[mSelIndex].id), mFilteredEntries[mSelIndex].buf);
               
            execConsoleCallback();
         }
         
         return;
      }
   }

   if( mReplaceText ) //  Only change the displayed text if appropriate.
   {
      setText("");
   }
   mSelIndex = -1;

   if( bNotifyScript && isMethod( "onCancel" ) )
      Con::executef( this, "onCancel" );

   if( id == -1 )
      return;

   // Execute the popup console command:
   if( bNotifyScript )
      execConsoleCallback();
}

//------------------------------------------------------------------------------
//  Added to set the first item as selected.
void GuiAutoCompleteCtrl::setFirstSelected( bool bNotifyScript )
{
   if( mEntries.size() > 0 )
   {
      mSelIndex = 0;
      if ( mReplaceText ) //  Only change the displayed text if appropriate.
      {
         setText( mEntries[0].buf );
      }

		// Execute the popup console command:
		if( bNotifyScript )
      {
         if ( isMethod( "onSelect" ) )
            Con::executef( this, "onSelect", Con::getIntArg( mEntries[ mSelIndex ].id ), mEntries[mSelIndex].buf );

			execConsoleCallback();
      }
   }
	else
	{
		if ( mReplaceText ) //  Only change the displayed text if appropriate.
			setText("");
		
		mSelIndex = -1;

		if( bNotifyScript )
      {
			Con::executef( this, "onCancel" );
         execConsoleCallback();
      }
	}
}

//------------------------------------------------------------------------------
//  Added to set no items as selected.
void GuiAutoCompleteCtrl::setNoneSelected()
{
   if ( mReplaceText ) //  Only change the displayed text if appropriate.
   {
      setText("");
   }
   mSelIndex = -1;
}

//------------------------------------------------------------------------------
const char *GuiAutoCompleteCtrl::getScriptValue()
{
   //return getText();
	return "";
}

//------------------------------------------------------------------------------
void GuiAutoCompleteCtrl::closePopUp()
{
   if ( !mInAction )
      return;

   // Get the selection from the text list:
   
   if( !mBackgroundCancel )
   {
      mSelIndex = mTl->getSelectedCell().y;
      mSelIndex = ( mRevNum >= mSelIndex && mSelIndex != -1 ) ? mRevNum - mSelIndex : mSelIndex;
      if ( mSelIndex != -1 )
      {
		  if( mReplaceText )
			  setText( mFilteredEntries[mSelIndex].buf );
		  setIntVariable( mFilteredEntries[mSelIndex].id );
      }
   }

   // Release the mouse:
   mInAction = false;
   mTl->mouseUnlock();

   // Now perform the popup action:
   if( mSelIndex != -1 && !mBackgroundCancel )
   {
      if ( isMethod( "onSelect" ) )
	  {
		  Con::executef( this, "onSelect", Con::getIntArg( mFilteredEntries[ mSelIndex ].id ), mFilteredEntries[mSelIndex].buf );
	  }

      // Execute the popup console command:
      execConsoleCallback();
   }
   else if ( isMethod( "onCancel" ) )
      Con::executef( this, "onCancel" );

   // Pop the background:
   GuiCanvas *root = getRoot();
   if ( root )
      root->popDialogControl(mBackground);

   // Kill the popup:
   mBackground->removeObject( mSc );
   mTl->deleteObject();
   mSc->deleteObject();
   mBackground->deleteObject();
   
   mBackground = NULL;
   mTl = NULL;
   mSc = NULL;

   // Set this as the first responder:
   //setFirstResponder();
}

//------------------------------------------------------------------------------
bool GuiAutoCompleteCtrl::onKeyDown(const GuiEvent &event)
{
   //if the control is a dead end, don't process the input:
   if ( !mVisible || !mActive || !mAwake )
      return false;

   //see if the key down is a <return> or not
   if ( event.keyCode == KEY_DOWN && event.modifier == 0 )
   {
	   mTl->setFirstResponder();
	   mTl->mouseLock();
	   mTl->onKeyDown(event);
      return true;
   }
   else if( event.keyCode == KEY_RETURN && event.modifier == 0 )
   {
	   onReturn_callback();
	   return true;
   }

   Parent::onKeyDown( event );

   closePopUp();
   rebuildResultEntries(); 
   onAction();

   setFirstResponder();
   return true;
}

void GuiAutoCompleteCtrl::rebuildResultEntries()
{
	mFilteredEntries.setSize(0);

	for ( U32 i = 0; i < mEntries.size(); ++i )
	{
		char dest[256];
		mTextBuffer.getCopy8((UTF8*)dest, GuiTextCtrl::MAX_STRING_LENGTH+1);
		char copy[256];
		dStrcpy(copy, mEntries[i].buf);
		if ( dStrstr( dStrlwr(copy), dStrlwr(dest) ) != NULL )
			mFilteredEntries.push_back( mEntries[i] );
	}
}

//------------------------------------------------------------------------------
void GuiAutoCompleteCtrl::onAction()
{
   GuiControl *canCtrl = getParent();

   addChildren();

   GuiCanvas *root = getRoot();
   Point2I windowExt = root->getExtent();

   mBackground->resize( Point2I(0,0), root->getExtent() );
   
   S32 textWidth = 0, width = getWidth();
   const S32 textSpace = 2;
   bool setScroll = false;

   for ( U32 i = 0; i < mFilteredEntries.size(); ++i )
      if ( S32(mProfile->mFont->getStrWidth( mFilteredEntries[i].buf )) > textWidth )
         textWidth = mProfile->mFont->getStrWidth( mFilteredEntries[i].buf );

   S32 sbWidth = mSc->getControlProfile()->mBorderThickness * 2 + mSc->scrollBarThickness(); //  Calculate the scroll bar width
   if ( textWidth > ( getWidth() - sbWidth-mProfile->mTextOffset.x - mSc->getChildMargin().x * 2 ) ) //  The text draw area to test against is the width of the drop-down minus the scroll bar width, the text margin and the scroll bar child margins.
   {
      textWidth +=sbWidth + mProfile->mTextOffset.x + mSc->getChildMargin().x * 2; //  The new width is the width of the text plus the scroll bar width plus the text margin size plus the scroll bar child margins.
      width = textWidth;

      //  If a child margin is not defined for the scroll control, let's add
      //      some space between the text and scroll control for readability
      if(mSc->getChildMargin().x == 0)
         width += textSpace;
   }

   mTl->setCellSize(Point2I(width, mProfile->mFont->getHeight() + textSpace));

   for ( U32 j = 0; j < mFilteredEntries.size(); ++j )
	   mTl->addEntry( mFilteredEntries[j].id, mFilteredEntries[j].buf );

   if ( mSelIndex >= 0 )
      mTl->setSelectedCell( Point2I( 0, mSelIndex ) );

   Point2I pointInGC = canCtrl->localToGlobalCoord( getPosition() );
   Point2I scrollPoint( pointInGC.x, pointInGC.y + getHeight() ); 

   //Calc max Y distance, so Scroll Ctrl will fit on window 

   S32 sbBorder = mSc->getControlProfile()->mBorderThickness * 2 + mSc->getChildMargin().y * 2;
   S32 maxYdis = windowExt.y - pointInGC.y - getHeight() - sbBorder;

   //If scroll bars need to be added
   mRevNum = 0;
   if ( maxYdis < mTl->getHeight() + sbBorder )
   {
      //Should we pop menu list above the button
      if ( maxYdis < pointInGC.y )
      {
         if(mReverseTextList)
            reverseTextList();

         maxYdis = pointInGC.y;
         //Does the menu need a scroll bar 
         if ( maxYdis < mTl->getHeight() + sbBorder )
         {
            setScroll = true;
         }
         //No scroll bar needed
         else
         {
            maxYdis = mTl->getHeight() + sbBorder;
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
      maxYdis = mTl->getHeight() + sbBorder;
   }

   RectI newBounds = mSc->getBounds();

   //offset it from the background so it lines up properly
   newBounds.point = mBackground->globalToLocalCoord( scrollPoint );

   if ( newBounds.point.x + width > mBackground->getWidth() )
      if ( width - getWidth() > 0 )
         newBounds.point.x -= width - getWidth();

   newBounds.extent.set( width, maxYdis );
   mSc->setBounds( newBounds );

   mSc->registerObject();
   mTl->registerObject();
   mBackground->registerObject();

   mSc->addObject( mTl );
   mBackground->addObject( mSc );

   mBackgroundCancel = false; //  Setup check if user clicked on the background instead of the text list (ie: didn't want to change their current selection).

   root->pushDialogControl( mBackground, 98 );

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

   //mTl->setFirstResponder();

   mInAction = true;
}

//------------------------------------------------------------------------------
void GuiAutoCompleteCtrl::addChildren()
{
   // Create Text List.
   mTl = new PopupTextListCtrl( this );
   AssertFatal( mTl, "Failed to create the PopupTextListCtrl for the PopUpMenu" );
   // Use the children's profile rather than the parent's profile, if it exists.
   GuiControlProfile *prof;
   if ( Sim::findObject( "GuiPopUpMenuDefault", prof ) )
   {
      mTl->setControlProfile( prof );
   }
   else
   {
      // Use the children's profile rather than the parent's profile, if it exists.
	  mTl->setControlProfile( mProfile->getChildrenProfile() ? mProfile->getChildrenProfile() : mProfile );
   }

   //mTl->setControlProfile(dynamic_cast<GuiControlProfile *>(Sim::findObject( "ToolsGuiTextListProfile" )));
   //mTl->setControlProfile( mProfile->getChildrenProfile() ? mProfile->getChildrenProfile() : mProfile ); 
   mTl->setField("noDuplicates", "false");

   mSc = new GuiScrollCtrl;
   AssertFatal( mSc, "Failed to create the GuiScrollCtrl for the PopUpMenu" );
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

   mBackground = new PopUpBackgroundCtrl( this, mTl );
   AssertFatal( mBackground, "Failed to create the GuiBackgroundCtrl for the PopUpMenu" );
}

//------------------------------------------------------------------------------
void GuiAutoCompleteCtrl::repositionPopup()
{
   if ( !mInAction || !mSc || !mTl )
      return;

   // I'm not concerned with this right now...
}

//------------------------------------------------------------------------------
void GuiAutoCompleteCtrl::reverseTextList()
{
   mTl->clear();

   for ( S32 i = mFilteredEntries.size()-1; i >= 0; --i )
	   mTl->addEntry( mFilteredEntries[i].id, mFilteredEntries[i].buf );

   // Don't lose the selected cell:
   if ( mSelIndex >= 0 )
   { 
	   mTl->setSelectedCell( Point2I( 0, mFilteredEntries.size() - mSelIndex - 1 ) ); 
   }

   mRevNum = mFilteredEntries.size() - 1;
}

//------------------------------------------------------------------------------
bool GuiAutoCompleteCtrl::getFontColor( ColorI &fontColor, S32 id, bool selected, bool mouseOver )
{
   U32 i;
   Entry* entry = NULL;
   for( i = 0; i < mFilteredEntries.size(); i++ )
   {
	   if( mFilteredEntries[i].id == id )
	   {
		   entry = &mFilteredEntries[i];
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

   // Default color scheme...
   fontColor = selected ? mProfile->mFontColorSEL : mouseOver ? mProfile->mFontColorHL : mProfile->mFontColorNA; //  Modified the final color choice from mProfile->mFontColor to mProfile->mFontColorNA

   return( true );
}

//------------------------------------------------------------------------------
//  Added
bool GuiAutoCompleteCtrl::getColoredBox( ColorI &fontColor, S32 id )
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
void GuiAutoCompleteCtrl::onMouseDown( const GuiEvent &event )
{
	Parent::onMouseDown(event);
	
   TORQUE_UNUSED(event);

   if( !mVisible || !mActive || !mAwake )
      return;
}

//------------------------------------------------------------------------------
//  Added
void GuiAutoCompleteCtrl::onMouseEnter( const GuiEvent &event )
{
   mMouseOver = true;

   // fade control
   fadeControl();    // Copyright (C) 2013 WinterLeaf Entertainment LLC.
}

//------------------------------------------------------------------------------
//  Added
void GuiAutoCompleteCtrl::onMouseLeave( const GuiEvent &event )
{
   mMouseOver = false;
   smCapturedControl = this;     // Copyright (C) 2013 WinterLeaf Entertainment LLC.
}

//------------------------------------------------------------------------------
void GuiAutoCompleteCtrl::setupAutoScroll( const GuiEvent &event )
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
void GuiAutoCompleteCtrl::autoScroll()
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
void GuiAutoCompleteCtrl::replaceText(S32 boolVal)
{
   mReplaceText = boolVal;
}
