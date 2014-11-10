#include "gui/controls/guiTableControl.h"
#include "core/strings/stringUnit.h"
#include "console/consoleTypes.h"
#include "gui/core/guiDefaultControlRender.h"
#include "gfx/primBuilder.h"
#include "gfx/gfxDrawUtil.h"
#include "console/engineAPI.h"
#include "gui/core/guiCanvas.h"

IMPLEMENT_CALLBACK(GuiTableControl, onSortedColumn, void, ( S32 columnId, const char* columnName, bool increasing ), ( columnId, columnName, increasing ), "Callback when the column is sorted." );
IMPLEMENT_CALLBACK( GuiTableControl, onSelect, void, (const char* cellid, const char* text),( cellid , text ),
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

IMPLEMENT_CONOBJECT(GuiTableControl);

GuiTableControl::GuiTableControl(void) :
	mHeadingSize(20)
{
	VECTOR_SET_ASSOCIATION(mColumnOffsets);
	
	mCurRegion = NONE;

	mChildRelPos.set(0, 1);

	mTextListCtrl = NULL;

	mColumnOffsets.push_back( 0 );

	Heading h;
	h.text = "";
	h.column = -1;

	mHeadingList = h;

	mColumnOrder = new ArrayObject();
	mColumnOrder->registerObject();

	GuiControlProfile *prof;
	if ( Sim::findObject( "GuiScrollProfile", prof ) )
	{
		this->setControlProfile( prof );
	}

	mChildAdded = false;
	mFitParentWidth = false;

	mChildRelPos.set( 0, 0 );
}


GuiTableControl::~GuiTableControl(void)
{
}

void GuiTableControl::initPersistFields()
{
	addField("columns",                 TypeS32Vector, Offset(mColumnOffsets, GuiTableControl), 
		"A vector of column offsets.  The number of values determines the number of columns in the table.\n" );
	addField("fitParentWidth",          TypeBool, Offset(mFitParentWidth, GuiTableControl), 
	   "If true, the width of this control will match the width of its parent.\n");
	addField("headingSize",				TypeS32, Offset(mHeadingSize, GuiTableControl),
		"The size of the heading.\n" );

	Parent::initPersistFields();

	removeField("childMargin");
}


void GuiTableControl::addChildControls()
{
	mTextListCtrl = new GuiTextListCtrl();
	AssertFatal( mTextListCtrl, "Failed to created the GuiTextListCtrl for the table.");
	GuiControlProfile *profile;
	if( Sim::findObject( "GuiDefaultProfile", profile ) )
	{
		mTextListCtrl->setControlProfile( profile );
	}

	mTextListCtrl->registerObject();
	mTextListCtrl->setField( "internalName", "textList" );
	mTextListCtrl->mColumnOffsets = mColumnOffsets;
	mTextListCtrl->mClipColumnText = true;
	mTextListCtrl->mFitParentWidth = mFitParentWidth;

	this->addObject( mTextListCtrl );

	mChildAdded = true;
}

GuiControl* GuiTableControl::findHitControl(const Point2I &pt, S32 initialLayer)
{
	if(pt.y < mHeadingSize)
      return this;
   return Parent::findHitControl(pt, initialLayer);
}

void GuiTableControl::computeSizes()
{
   S32 thickness = (mProfile ? mProfile->mBorderThickness : 1);
   Point2I borderExtent(thickness, thickness);
   mContentPos = borderExtent + mChildMargin + Point2I(0, mHeadingSize );
   mContentExt = getExtent() - (mChildMargin * 2)
	   - (borderExtent * 2) - Point2I( 0, mHeadingSize );

   Point2I childLowerRight;

   mHBarEnabled = false;
   mVBarEnabled = false;
   mHasVScrollBar = (mForceVScrollBar == ScrollBarAlwaysOn);
   mHasHScrollBar = (mForceHScrollBar == ScrollBarAlwaysOn);

   setUpdate();

   if (calcChildExtents())
   {
      childLowerRight = mChildPos + mChildExt;

      if (mHasVScrollBar)
         mContentExt.x -= mScrollBarThickness;
      if (mHasHScrollBar)
         mContentExt.y -= mScrollBarThickness;
      if (mChildExt.x > mContentExt.x && (mForceHScrollBar == ScrollBarDynamic))
      {
         mHasHScrollBar = true;
         mContentExt.y -= mScrollBarThickness;
      }
      if (mChildExt.y > mContentExt.y && (mForceVScrollBar == ScrollBarDynamic))
      {
         mHasVScrollBar = true;
         mContentExt.x -= mScrollBarThickness;

         // If Extent X Changed, check Horiz Scrollbar.
         if (mChildExt.x > mContentExt.x && !mHasHScrollBar && (mForceHScrollBar == ScrollBarDynamic))
         {
            mHasHScrollBar = true;
            mContentExt.y -= mScrollBarThickness;
         }
      }
      Point2I contentLowerRight = mContentPos + mContentExt;

      // see if the child controls need to be repositioned (null space in control)
      Point2I delta(0,0);

      if (mChildPos.x > mContentPos.x)
         delta.x = mContentPos.x - mChildPos.x;
      else if (contentLowerRight.x > childLowerRight.x)
      {
         S32 diff = contentLowerRight.x - childLowerRight.x;
         delta.x = getMin(mContentPos.x - mChildPos.x, diff);
      }

      //reposition the children if the child extent > the scroll content extent
      if (mChildPos.y > mContentPos.y)
         delta.y = mContentPos.y - mChildPos.y;
      else if (contentLowerRight.y > childLowerRight.y)
      {
         S32 diff = contentLowerRight.y - childLowerRight.y;
         delta.y = getMin(mContentPos.y - mChildPos.y, diff);
      }

      // apply the deltas to the children...
      if (delta.x || delta.y)
      {
         SimGroup::iterator i;
         for(i = begin(); i != end();i++)
         {
            GuiControl *ctrl = (GuiControl *) (*i);
            ctrl->setPosition( ctrl->getPosition() + delta );
         }
         mChildPos += delta;
         childLowerRight += delta;
      }
      // enable needed scroll bars
      if (mChildExt.x > mContentExt.x)
         mHBarEnabled = true;
      if (mChildExt.y > mContentExt.y)
         mVBarEnabled = true;
      mChildRelPos = mContentPos - mChildPos;
   }

   // Prevent resizing our children from recalling this function!
   mIgnoreChildResized = true;

   if ( mLockVertScroll )
   {
      // If vertical scroll is locked we size our child's height to our own
      SimGroup::iterator i;
      for(i = begin(); i != end();i++)
      {
         GuiControl *ctrl = (GuiControl *) (*i);
         ctrl->setHeight( mContentExt.y  );
      }
   }

   if ( mLockHorizScroll )
   {
      // If horizontal scroll is locked we size our child's width to our own
      SimGroup::iterator i;
      for(i = begin(); i != end();i++)
      {
         GuiControl *ctrl = (GuiControl *) (*i);
         ctrl->setWidth( mContentExt.x  );
      }
   }

   mIgnoreChildResized = false;

   // build all the rectangles and such...
   calcScrollRects();
   calcThumbs();
}

void GuiTableControl::onRender(Point2I offset, const RectI &updateRect)
{
	Parent::onRender( offset, updateRect );

	if( mTextureObject )
	{
		// Reset the ClipRect as the parent call can modify it when rendering
		// the child controls
		GFX->setClipRect( updateRect );
		
		//Set the color of the heading text
		RectI headingRect( offset.x, offset.y, updateRect.extent.x - mVTrackRect.extent.x, mHeadingSize );
		GFX->getDrawUtil()->drawRectFill(headingRect, ColorI(72, 61, 139 ));

		const char *text = mHeadingList.text;
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

				Point2I pos(offset.x + 4 + mColumnOffsets[index] - mChildRelPos.x, offset.y);

				RectI saveClipRect;
				bool clipped = false;

				//Render the arrow.
				renderSortOrder( index, offset );

				if( index != 0 )
				{
					// Render the separator.
					RectI r;
					r.point = Point2I(offset.x + mColumnOffsets[index] - mChildRelPos.x -3 , offset.y);
					r.extent.set( 2, mHeadingSize );
					GFX->getDrawUtil()->drawRectFill(r, ColorI(255, 255, 255, 255));
				}

				if(index != (mColumnOffsets.size() - 1))
				{
					saveClipRect = GFX->getClipRect();

					RectI clipRect(pos, Point2I(mColumnOffsets[index+1] - mColumnOffsets[index] - 10, mHeadingSize));
					if(clipRect.intersect(saveClipRect))
					{
						clipped = true;
						GFX->setClipRect( clipRect );
					}
				}

				GFX->getDrawUtil()->drawTextN(mProfile->mFont, pos, text, slen, mProfile->mFontColors);

				if( !mTextListCtrl )
				{
					S32 textWidth;
					if(nextCol)
						textWidth = mProfile->mFont->getStrNWidth((const UTF8*)text, nextCol - text);
					else
						textWidth = mProfile->mFont->getStrWidth((const UTF8*)text);

					textWidth += mColumnOffsets[index];
					if( textWidth >=  headingRect.extent.x )
					{
						mChildExt.x = textWidth + mVTrackRect.extent.x + 5;
					
						mHasHScrollBar = true;
						mHBarEnabled = true;
						calcScrollRects();
						calcThumbs();
						setUpdate();
					}
				}


				if(clipped)
					GFX->setClipRect( saveClipRect );
			}
			if(!nextCol)
				break;
			text = nextCol+1;
		}
	}
	
}

void GuiTableControl::renderSortOrder( S32 index, Point2I offset )
{
	S32 columnIndex = mColumnOrder->getIndexFromKey( String::ToString(mColumnOffsets[index]) );
	if( columnIndex != -1 )
	{
		GFXStateBlockDesc desc;
		desc.setBlend( false );
		desc.setZReadWrite( false, false );
		desc.cullMode = GFXCullNone;

		const char *value = mColumnOrder->getValueFromIndex( columnIndex );

		if( dStricmp( value, "" ) && mTextListCtrl )
		{
			bool increasing = !dStricmp( value, "true" );
			Point3F point1, point2, point3;

			Point2I positionOffset(0, 0);
			if( isLastIndex( index + 1 ) )
			{
				mTextListCtrl->getCellSize(positionOffset);
				positionOffset.x -= mChildRelPos.x;
			}
			else
			{
				positionOffset = mChildRelPos;
				positionOffset.x = mColumnOffsets[index + 1] - positionOffset.x;
			}

			if( increasing )
			{
				point1 = Point3F( offset.x + positionOffset.x - 10, offset.y + (F32)( 1 * mHeadingSize / 3 ), -1 );
				point2 = Point3F( offset.x + positionOffset.x - 14, offset.y + (F32)( 2 * mHeadingSize / 3 ), -1 );
				point3 = Point3F( offset.x + positionOffset.x - 6, offset.y + (F32)( 2 * mHeadingSize / 3 ), -1 );
			}
			else
			{
				point1 = Point3F( offset.x + positionOffset.x - 6, offset.y + (F32)( 1 * mHeadingSize / 3 ), -1 );
				point2 = Point3F( offset.x + positionOffset.x - 14, offset.y + (F32)( 1 * mHeadingSize / 3 ), -1 );
				point3 = Point3F( offset.x + positionOffset.x - 10, offset.y + (F32)( 2 * mHeadingSize / 3 ), -1 );
			}

			GFX->getDrawUtil()->drawTriangle( desc, point1, point2, point3, ColorI::WHITE );
		}
	}
}

bool GuiTableControl::isLastIndex( S32 index )
{
	return (index >= StringUnit::getUnitCount( mHeadingList.text, "\t" ));
}

void GuiTableControl::getCursor( GuiCursor *&cursor, bool &showCursor, const GuiEvent &lastGuiEvent )
{
	GuiCanvas *pRoot = getRoot();
	if( !pRoot )
		return;

	Region pointRegion = NONE;

	PlatformWindow *pWindow = pRoot->getPlatformWindow();
	AssertFatal(pWindow != NULL,"GuiControl without owning platform window!  This should not be possible.");
	PlatformCursorController *pController = pWindow->getCursorController();
	AssertFatal(pController != NULL,"PlatformWindow without an owned CursorController!");

	Point2I curMousePos = globalToLocalCoord(lastGuiEvent.mousePoint);
	pointRegion = findHitRegion( curMousePos ); 

	switch(pointRegion)
	{
	case DIVIDER:
		// change to left-right cursor
		if(pRoot->mCursorChanged != PlatformCursorController::curResizeVert)
		{
			//*** We've already changed the cursor, so set it back before we change it again.
			if(pRoot->mCursorChanged != -1)
			pController->popCursor();

			//*** Now change the cursor shape
			pController->pushCursor(PlatformCursorController::curResizeVert);
			pRoot->mCursorChanged = PlatformCursorController::curResizeVert;

		}
		break;
	case NONE:
	default:
		if(pRoot->mCursorChanged != -1)
		{
			//*** We've already changed the cursor, so set it back before we change it again.
			pController->popCursor();

			pRoot->mCursorChanged = -1;
		}
		break;
	}
}

void GuiTableControl::onStaticModified( const char* slotName, const char* newValue )
{
	if( !dStricmp( slotName, "columns" ) )
	{
		if( mColumnOrder )
		{
			Vector<String> columns;
			String::ToString(newValue).split(" ", columns);

			for( Vector<String>::iterator itr = columns.begin(); itr != columns.end(); itr++ )
			{
				S32 index = mColumnOrder->getIndexFromKey(*itr);
				if( index == -1 )
					mColumnOrder->push_back( *itr, "" );
			}
		}

		if( mTextListCtrl )
			mTextListCtrl->setField( "columns", newValue );
	}
	if( !dStricmp( slotName, "fitParentWidth" ) )
		if( mTextListCtrl )
			mTextListCtrl->setField( "fitParentWidth", newValue );
	if( !dStricmp( slotName, "clipColumnText" ) )
		if( mTextListCtrl )
			mTextListCtrl->setField( "clipColumnText", newValue );
}

GuiTableControl::Region GuiTableControl::findHitRegion( const Point2I &mousePos )
{
	S32 i = 1;
	mCurVertHit = -1;

	for( int j = 0; j < mColumnOffsets.size(); j++, i++ )
	{
		if( mousePos.y <= mHeadingSize)
		{
			Point2I columnOffset(0,0);
			if( isLastIndex( j + 1 ) )
			{
				if( !mTextListCtrl )
					columnOffset.x = mChildExt.x;
				else
					mTextListCtrl->getCellSize(columnOffset);
			}
			else
				columnOffset.x = mColumnOffsets[j +1];
			
			if( mousePos.x > columnOffset.x - mChildRelPos.x - 3 &&  mousePos.x < columnOffset.x - mChildRelPos.x - 1 )
			{
				mCurVertHit = i;
				mDividerLockPos = mousePos.x;
				return (mCurRegion = DIVIDER);
			}
			else if( mousePos.x <= columnOffset.x - mChildRelPos.x - 4 )
			{
				mCurVertHit = i;
				return ( mCurRegion = HEADER );
			}
		}
	}

	return (mCurRegion = NONE );
}

void GuiTableControl::onMouseDown( const GuiEvent &event )
{
	Point2I curMousePos = globalToLocalCoord(event.mousePoint);
    findHitRegion(curMousePos);   

	if( mCurRegion == DIVIDER )
	{
		mouseLock();
		setFirstResponder();
		setUpdate();
	}
	else if( mCurRegion == HEADER )
	{
		if( mTextListCtrl )
		{
			//-1 because, mCurVertHit captures the index of the next column index.
			S32 index = mColumnOrder->getIndexFromKey(String::ToString(mColumnOffsets[mCurVertHit - 1]));
			if( index != -1 )
			{
				for( S32 i = 0; i < mColumnOrder->count(); i++ )
				{
					//Resetting other sorts to None, except for the captured index.
					if( i != index )
						mColumnOrder->setValue( "", i );
				}

				//Setting the value of the captured index.
				bool order = (mColumnOrder->getValueFromIndex( index ) == String::ToString("true") ) ? false : true;
				mColumnOrder->setValue( order ? "true" : "false", index );

				//Sort the textList. 
				mTextListCtrl->sort( mCurVertHit -1 , order );

				onSortedColumn_callback( mCurVertHit - 1, StringUnit::getUnit( mHeadingList.text, index, "\t" ), order );
			}
		}
	}
	else
		Parent::onMouseDown( event );
}

void GuiTableControl::onMouseDragged( const GuiEvent &event )
{
	Point2I mousePos = globalToLocalCoord( event.mousePoint);
	if( mCurRegion == DIVIDER )
	{
		S32 move = mousePos.x - mDividerLockPos;
	
		if(move < 0  && mColumnOffsets[mCurVertHit] <= ( mColumnOffsets[mCurVertHit -1] + 30 ))
		{
			S32 index = mColumnOrder->getIndexFromKey(String::ToString(mColumnOffsets[mCurVertHit]));
			if( index != -1 )
			{
				mColumnOffsets[mCurVertHit] = mColumnOffsets[mCurVertHit - 1] + 30;
				mColumnOrder->setKey( String::ToString(mColumnOffsets[mCurVertHit]) , index );
				if( mTextListCtrl )
					mTextListCtrl->mColumnOffsets[mCurVertHit] = mColumnOffsets[mCurVertHit - 1] + 30;
			}
			return;
		}

		S32 itr;
		for (itr = mCurVertHit; itr < mColumnOffsets.size(); itr++)
		{
			S32 index = mColumnOrder->getIndexFromKey(String::ToString(mColumnOffsets[itr]));
			if( index != -1 )
			{
				mColumnOffsets[itr] = mColumnOffsets[itr] + move;
				mColumnOrder->setKey( String::ToString(mColumnOffsets[itr]) , index );
			
				if( mTextListCtrl )
					mTextListCtrl->mColumnOffsets[itr] = mColumnOffsets[itr] + move;
			}
		}

		if( mTextListCtrl )
			mTextListCtrl->setSize( Point2I( 1, mTextListCtrl->mList.size() ));
		

		mDividerLockPos = mousePos.x;
	}
	else
		Parent::onMouseDragged( event );
}

void GuiTableControl::onMouseUp( const GuiEvent &event )
{
	if( mCurRegion == NONE )
		Parent::onMouseUp( event );
	
	if( mCurRegion == DIVIDER )
		mouseUnlock();

	mCurRegion = NONE;
	mCurVertHit = -1;
	setUpdate();
}

void GuiTableControl::scrollTo( S32 x, S32 y )
{
	if( !size() && mHeadingList.text == "" )
      return;

	if ( x == mChildRelPos.x && y == mChildRelPos.y )
		return;

	setUpdate();
	if (x > mChildExt.x - mContentExt.x)
		x = mChildExt.x - mContentExt.x;
	if (x < 0)
		x = 0;

	if (y > mChildExt.y - mContentExt.y)
		y = mChildExt.y - mContentExt.y;
	if (y < 0)
		y = 0;

	Point2I delta(x - mChildRelPos.x, y - mChildRelPos.y);
	mChildRelPos += delta;
	mChildPos -= delta;

	for(SimSet::iterator i = begin(); i != end();i++)
	{
		GuiControl *ctrl = (GuiControl *) (*i);
		ctrl->setPosition( ctrl->getPosition() - delta );
	}
	calcThumbs();
}

void GuiTableControl::addChildRow( U32 id, const char *text )
{
	mChildRelPos.x = 0;
	if( !mChildAdded )
		addChildControls();
	mTextListCtrl->addEntry( id, text );
}

void GuiTableControl::addHeading( const char* heading, int column )
{
	Heading h;
	h.text = dStrdup(heading);
	h.column = column;
	mHeadingList = h;
}

void GuiTableControl::clearChildren( )
{
	if( mColumnOrder )
		for( S32 i = 0; i < mColumnOrder->count(); i++ )
			mColumnOrder->setValue( "", i );
	if( mTextListCtrl )
		mTextListCtrl->clear();
}

void GuiTableControl::setColumnSort( S32 column, bool ascending )
{
	if( mColumnOrder )
	{
		S32 columnIndex = mColumnOrder->getIndexFromKey( String::ToString(mColumnOffsets[column]) );
		mColumnOrder->setValue( ascending ? "true" : "false" , columnIndex );
	}
}

S32 GuiTableControl::findEntryByColumnText( S32 columnId, const char *text)
{
	if( mTextListCtrl != NULL )
		return mTextListCtrl->findEntryByColumnText( columnId, text );
	return -1;
}

U32 GuiTableControl::getSelectedRow()
{
	if( mTextListCtrl != NULL )
		return mTextListCtrl->getSelectedRow();
	return -1;
}

void GuiTableControl::setSelectedRow(Point2I cell)
{
	if( mTextListCtrl != NULL )
		mTextListCtrl->setSelectedCell(cell);
}


DefineEngineMethod( GuiTableControl, addHeading, void, (const char* text, int index),,"")
{
	object->addHeading( text, index );
}

DefineEngineMethod( GuiTableControl, addChildRow, void, (const char* text, int index),,"")
{
	object->addChildRow( index, text );
}

DefineEngineMethod( GuiTableControl, clearChildren, void,(),,"")
{
	object->clearChildren( );
}

DefineEngineMethod( GuiTableControl, setColumnSort, void,(S32 column, bool ascending),,"")
{
	object->setColumnSort( column, ascending );
}

DefineEngineMethod( GuiTableControl, findColumnTextIndex, S32, (S32 columnId, const char* columnText),,"")
{
	return object->findEntryByColumnText( columnId, columnText );
}

DefineEngineMethod( GuiTableControl, setSelectedRow, void, (int rowNum),,
   "@briefSelects the specified row.\n\n"
   "@param rowNum Row number to set selected.\n"
   "@tsexample\n"
   "// Define the row number to set selected\n"
   "%rowNum = \"4\";\n\n"
   "%guiTextListCtrl.setSelectedRow(%rowNum);\n"
   "@endtsexample\n\n"
   "@see GuiControl")
{
   object->setSelectedRow( Point2I( 0, rowNum ) );
}

DefineEngineMethod( GuiTableControl, getSelectedRow, S32, (),,
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
