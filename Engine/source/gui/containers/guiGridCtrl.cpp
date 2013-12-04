#include "console/engineAPI.h"
#include "platform/platform.h"

#include "gfx/gfxDevice.h"
#include "gfx/gfxDrawUtil.h"
#include "gui/containers/guiGridCtrl.h"
#include "gui/core/guiDefaultControlRender.h"
#include "gui/controls/guiIconControl.h"

IMPLEMENT_CONOBJECT(GuiGridCtrl);
ConsoleDocClass(GuiGridCtrl,
	"@brief A container that accepts dragged and dropped icons and arranges them in a grid .\n\n"
	"@ingroup GuiContainers");

GuiGridCtrl::GuiGridCtrl()
{
	mGridType = GridType::Square;
	mAcceptDrops = true;
	mIsContainer = true;

	mColumns = 8;
	mRows = 8;
	mGrid.Initialize(NULL, mColumns, mRows);

	SetCellSize(64);

	mStartPoint = Point2I(0, 0);
	mWrap = false;
}

GuiGridCtrl::~GuiGridCtrl()
{
}

// ConsoleObject...
void GuiGridCtrl::initPersistFields()
{
	addGroup( "Background" );
	{
		addProtectedField( "bitmap", TypeImageFilename, Offset( mBitmapName, GuiGridCtrl ),
			&setBitmapNameField, &defaultProtectedGetFn,
			"The bitmap file to display in the control." );
		addField( "wrap", TypeBool, Offset( mWrap, GuiGridCtrl ),
			"If true, the bitmap is tiled inside the control rather than stretched to fit." );
	}
	endGroup( "Background" );

	addGroup("Grid");
	{
		addProtectedField( "gridType", TypeGridType_Enum, Offset( mGridType, GuiGridCtrl ),
			&setGridTypeField, &defaultProtectedGetFn, "Determines the grid type, which affects both rendering and adjacency" );
		addProtectedField( "columns", TypeS32, Offset( mColumns, GuiGridCtrl ),
			&setColumnsField, &defaultProtectedGetFn, "Total cell columns in the grid" );
		addProtectedField( "rows", TypeS32, Offset( mRows, GuiGridCtrl ),
			&setRowsField, &defaultProtectedGetFn, "Total cell rows in the grid" );
		addProtectedField( "cellSize", TypePoint2I, Offset( mCellSize, GuiGridCtrl ),
			&setCellSizeField, &defaultProtectedGetFn, "Size of each individual cell" );
	}
	endGroup("Grid");

	Parent::initPersistFields();
}

void GuiGridCtrl::setBitmap( const char *name, bool resize )
{
	mBitmapName = name;
	if ( !isAwake() )
		return;

	if ( mBitmapName.isNotEmpty() )
	{
		if ( !mBitmapName.equal("texhandle", String::NoCase) )
			mTextureObject.set( mBitmapName, &GFXDefaultGUIProfile, avar("%s() - mTextureObject (line %d)", __FUNCTION__, __LINE__) );

		// Resize the control to fit the bitmap
		if ( mTextureObject && resize )
		{
			setExtent( mTextureObject->getWidth(), mTextureObject->getHeight() );
			updateSizing();
		}
	}
	else
		mTextureObject = NULL;

	setUpdate();
}

void GuiGridCtrl::setBitmapHandle(GFXTexHandle handle, bool resize)
{
	mTextureObject = handle;

	mBitmapName = String("texhandle");

	// Resize the control to fit the bitmap
	if (resize) 
	{
		setExtent(mTextureObject->getWidth(), mTextureObject->getHeight());
		updateSizing();
	}
}


void GuiGridCtrl::SetGridType(GridType::Enum value)
{
	S32 baseSize;
	switch (mGridType)
	{
		case GridType::HexRows:
		{
			baseSize = mCellSize.y;
			break;
		}
		case GridType::HexColumns:
		case GridType::Square:
		default:
		{
			baseSize = mCellSize.x;
			break;
		}
	}

	mGridType = value;
	switch (value)
	{
		case GridType::HexColumns:
		{
			mCellSize = Vector2I(baseSize, baseSize * Math::Sqrt(3.f) / 2.f);
			break;
		}
		case GridType::HexRows:
		{
			mCellSize = Vector2I(baseSize * Math::Sqrt(3.f) / 2.f, baseSize);
			break;
		}
		case GridType::Square:
		default:
		{
			mCellSize = Vector2I(baseSize, baseSize);
			break;
		}
	}

	resizeGrid(mColumns, mRows, mCellSize);
}

void GuiGridCtrl::setOffset(S32 x, S32 y)
{
	if (mTextureObject)
	{
		x += mTextureObject->getWidth() / 2;
		y += mTextureObject->getHeight() / 2;
	}
	while (x < 0)
		x += 256;
	mStartPoint.x = x % 256;

	while (y < 0)
		y += 256;
	mStartPoint.y = y % 256;
}


void GuiGridCtrl::SetColumns(S32 value)
{
	mColumns = value;
	resizeGrid(mColumns, mRows, mCellSize);
}

void GuiGridCtrl::SetRows(S32 value)
{
	mRows = value;
	resizeGrid(mColumns, mRows, mCellSize);
}

void GuiGridCtrl::SetCellSize(const Vector2I& value)
{
	resizeGrid(mColumns, mRows, value);
}

void GuiGridCtrl::SetCellSize(S32 value)
{
	resizeGrid(mColumns, mRows, value);
}

// SimObject...
void GuiGridCtrl::inspectPostApply()
{
	if (!mWrap && (getExtent().x == 0) && (getExtent().y == 0) && mTextureObject)
	{
		setExtent( mTextureObject->getWidth(), mTextureObject->getHeight());
	}
	else
	{
		resize(getPosition(), getExtent());
	}
	Parent::inspectPostApply();
}


// SimSet...
void GuiGridCtrl::addObject(SimObject *obj)
{
	Parent::addObject(obj);
}

void GuiGridCtrl::addObjectToGrid(GuiControl* control, const Vector2I& coordinates)
{
	Point2I position;
	gridCoordinatesToLocalPosition(position, coordinates);

	mGrid(coordinates) = control;
	addObject(control);
	control->setPosition(position);
}

void GuiGridCtrl::removeObject(SimObject *obj)
{
	GuiControl* control = dynamic_cast<GuiControl*>(obj);
	if (control != NULL)
	{
		Point2I center = control->getPosition() + control->getExtent() / 2;
		Point2I coordinates;
		localPositionToGridCoordinates(coordinates, center);

		if (mGrid(coordinates) == control)
		{
			mGrid(coordinates) = NULL; 
		}
	}
	Parent::removeObject(obj);
}

void GuiGridCtrl::updateSizing()
{
	if(!getParent())
		return;
	// updates our bounds according to our horizSizing and verSizing rules
	RectI fakeBounds( getPosition(), getParent()->getExtent());
	parentResized( fakeBounds, fakeBounds);
}

bool GuiGridCtrl::onWake()
{
	if (!Parent::onWake())
		return false;
	setActive(true);
	setBitmap(mBitmapName);
	return true;
}

void GuiGridCtrl::onSleep()
{
	if (!mBitmapName.equal("texhandle", String::NoCase))
		mTextureObject = NULL;

	Parent::onSleep();
}

void GuiGridCtrl::onRender(Point2I offset, const RectI& updateRect)
{
	renderBackground(offset, updateRect);
	renderGrid(offset, updateRect);

	// Render Children
	renderChildControls(offset, updateRect);
}

bool GuiGridCtrl::onControlDropped(GuiControl* payload, Point2I position)
{
	if (!Parent::onControlDropped(payload, position))
		return false;

	Point2I index;
	globalPositionToGridCoordinates(index, position);
	if (mGrid(index) == NULL)
	{
		Point2I payloadSize = payload->getExtent();
		Point2I snappedPosition;
		gridCoordinatesToLocalPosition(snappedPosition, index);

		RectI payloadBounds = payload->getBounds();
		payloadBounds.setCenter(snappedPosition);
		//Point2I snappedPosition(index.x * mCellSize + (mCellSize - payload->getExtent().x) / 2, 
		//	index.y * mCellSize + (mCellSize - payload->getExtent().y) / 2);

		mGrid(index) = payload;
		addObject(payload);
		payload->setPosition(payloadBounds.point);
		return true;
	}
	else 
	{
		payload->unregisterObject();
		delete payload;
		return false;
	}
}

void GuiGridCtrl::resizeGrid(S32 inColumns, S32 inRows, S32 inCellSize)
{
	switch (mGridType)
	{
		case GridType::HexColumns:
		{
			mCellSize = Vector2I(inCellSize, inCellSize * Math::Sqrt(3.f) / 2.f);
			break;
		}
		case GridType::HexRows:
		{
			mCellSize = Vector2I(inCellSize * Math::Sqrt(3.f) / 2.f, inCellSize);
			break;
		}
		case GridType::Square:
		default:
		{
			mCellSize = Vector2I(inCellSize, inCellSize);
			break;
		}
	}

	// Only rebuild grid if grid dimensions changed
	if (inColumns != mColumns || inRows != mRows)
	{
		clear();
		mColumns = inColumns;
		mRows = inRows;
		mGrid.Initialize(NULL, mColumns, mRows);
	}
	
	switch (mGridType)
	{
		case GridType::HexColumns:
		{
			resize(getPosition(), Vector2I(mCellSize.x + (((mColumns - 1) * mCellSize.x * 3)) / 4, mCellSize.y * mRows + (mColumns == 1 ? 0 : mCellSize.y / 2)));
			break;
		}
		case GridType::HexRows:
		{
			resize(getPosition(), Vector2I(mCellSize.x * mColumns + (mRows == 1 ? 0 : mCellSize.x / 2), mCellSize.y + ((mRows - 1) * mCellSize.x * 3) / 4));
			break;
		}
		case GridType::Square:
		default:
		{
			resize(getPosition(), Vector2I(mColumns * mCellSize.x, mRows * mCellSize.y));
			break;
		}
	}
}

void GuiGridCtrl::resizeGrid(S32 inColumns, S32 inRows, const Vector2I& inCellSize)
{
	// Only rebuild grid if grid dimensions changed
	if (inColumns != mColumns || inRows != mRows)
	{
		clear();
		mColumns = inColumns;
		mRows = inRows;
		mGrid.Initialize(NULL, mColumns, mRows);
	}
	
	mCellSize = inCellSize;
	switch (mGridType)
	{
		case GridType::HexColumns:
		{
			resize(getPosition(), Vector2I(inCellSize.x + (((mColumns - 1) * inCellSize.x * 3)) / 4, inCellSize.y * mRows + (mColumns == 1 ? 0 : inCellSize.y / 2)));
			break;
		}
		case GridType::HexRows:
		{
			resize(getPosition(), Vector2I(inCellSize.x * mColumns + (mRows == 1 ? 0 : inCellSize.x / 2), inCellSize.y + ((mRows - 1) * inCellSize.x * 3) / 4));
			break;
		}
		case GridType::Square:
		default:
		{
			resize(getPosition(), Vector2I(mColumns * inCellSize.x, mRows * inCellSize.y));
			break;
		}
	}
}


bool GuiGridCtrl::resize(const Point2I &newPosition, const Point2I &newExtent)
{
	return Parent::resize(newPosition, newExtent);
}

void GuiGridCtrl::refresh()
{
	resize(getPosition(), getExtent());
}

void GuiGridCtrl::localPositionToGridCoordinates(Point2I& outCoordinates, const Point2I& inLocalPosition)
{
	Math::PositionToCoordinates(outCoordinates, inLocalPosition, mCellSize, mGridType);	
	outCoordinates.x = Math::Clamp(outCoordinates.x, 0, mColumns-1);
	outCoordinates.y = Math::Clamp(outCoordinates.y, 0, mRows-1);
}
 
void GuiGridCtrl::globalPositionToGridCoordinates(Point2I& outCoordinates, const Point2I& inGlobalPosition)
{
	Point2I localPosition = globalToLocalCoord(inGlobalPosition);
	return localPositionToGridCoordinates(outCoordinates, localPosition);
}

void GuiGridCtrl::gridCoordinatesToLocalPosition(Point2I& outLocalPosition, const Point2I& inCoordinates)
{
	return Math::CoordinatesToPosition(outLocalPosition, inCoordinates, mCellSize, mGridType);
}

void GuiGridCtrl::gridCoordinatesToGlobalPosition(Point2I& outGlobalPosition, const Point2I& inCoordinates)
{
	Point2I localPosition;
	gridCoordinatesToLocalPosition(localPosition, inCoordinates);
	outGlobalPosition = localToGlobalCoord(localPosition);
}

void GuiGridCtrl::renderGrid(Point2I offset, const RectI& updateRect)
{
	GFXDrawUtil *drawer = GFX->getDrawUtil();
	if (mProfile->mBorder || !mTextureObject)
	{
		RectI rect(offset.x, offset.y, getExtent().x, getExtent().y);
		drawer->drawRect(rect, mProfile->mBorderColor);
	}

	////if there's a border, draw the border
	//if ( mProfile->mBorder )
	//	renderBorder(ctrlRect, mProfile);

	RectI bounds = getGlobalBounds();
	switch (mGridType)
	{
		case GridType::HexColumns:
		{
			for (S32 x = 0; x < mColumns; ++x)
			{
				for (S32 y = 0; y < mRows; ++y)
				{
					S32 sideLength = mCellSize.x / 2;
					S32 radius = mCellSize.y / 2;

					Point2I center(offset);
					center.x += sideLength + (x * sideLength * 3) / 2;
					if (Math::IsEven(x))
					{
						center.y += radius + 2 * radius * y;
					}
					else
					{
						center.y += 2 * radius + 2 * radius * y;
					}

					Point2I vertices[6];
					vertices[0] = Vector2I(center.x + sideLength / 2, center.y - radius);
					vertices[1] = Vector2I(center.x + sideLength, center.y);
					vertices[2] = Vector2I(center.x + sideLength / 2, center.y + radius);
					vertices[3] = Vector2I(center.x - sideLength / 2, center.y + radius);
					vertices[4] = Vector2I(center.x - sideLength, center.y);
					vertices[5] = Vector2I(center.x - sideLength /2, center.y - radius);

					for (S32 i = 0; i < 5; ++i)
					{
						drawer->drawLine(vertices[i], vertices[i+1], ColorI::BLACK);
					}
					drawer->drawLine(vertices[5], vertices[0], ColorI::BLACK);
				}
			}
			break;
		}
		case GridType::HexRows:
		{
			for (S32 x = 0; x < mColumns; ++x)
			{
				for (S32 y = 0; y < mRows; ++y)
				{
					S32 sideLength = mCellSize.y / 2;
					S32 radius = mCellSize.x / 2;

					Point2I center(offset);
					center.y += sideLength + (y * sideLength * 3) / 2;
					if (Math::IsEven(y))
					{
						center.x += radius + 2 * radius * x;
					}
					else
					{
						center.x += 2 * radius + 2 * radius * x;
					}

					Point2I vertices[6];
					vertices[0] = Vector2I(center.x, center.y - sideLength);
					vertices[1] = Vector2I(center.x + radius, center.y - sideLength / 2);
					vertices[2] = Vector2I(center.x + radius, center.y + sideLength / 2);
					vertices[3] = Vector2I(center.x, center.y + sideLength);
					vertices[4] = Vector2I(center.x - radius, center.y + sideLength / 2);
					vertices[5] = Vector2I(center.x - radius, center.y - sideLength / 2);

					for (S32 i = 0; i < 5; ++i)
					{
						drawer->drawLine(vertices[i], vertices[i+1], ColorI::BLACK);
					}
					drawer->drawLine(vertices[5], vertices[0], ColorI::BLACK);
				}
			}
			break;
		}
		case GridType::Square:
		default:
		{
			for (S32 x = 0; x <= mColumns; ++x)
			{
				drawer->drawLine(bounds.point.x + x * mCellSize.x, bounds.point.y, 
					bounds.point.x + x * mCellSize.x, bounds.point.y + mRows * mCellSize.y,
					ColorI::BLACK);
			}
			for (S32 y = 0; y <= mRows; ++y)
			{
				drawer->drawLine(bounds.point.x, bounds.point.y + y * mCellSize.y, 
					bounds.point.x + mColumns * mCellSize.x, bounds.point.y + y * mCellSize.y, 
					ColorI::BLACK);
			}
			break;
		}
	}
}

void GuiGridCtrl::renderBackground(Point2I offset, const RectI& updateRect)
{
	RectI ctrlRect(offset, getExtent());

	//if opaque, fill the update rect with the fill color
	if ( mProfile->mOpaque )
		GFX->getDrawUtil()->drawRectFill(ctrlRect, mProfile->mFillColor);

	if (mTextureObject)
	{
		GFX->getDrawUtil()->clearBitmapModulation();
		if (mWrap)
		{
			// We manually draw each repeat because non power of two textures will 
			// not tile correctly when rendered with GFX->drawBitmapTile(). The non POT
			// bitmap will be padded by the hardware, and we'll see lots of slack
			// in the texture. So... lets do what we must: draw each repeat by itself:
			GFXTextureObject* texture = mTextureObject;
			RectI srcRegion;
			RectI dstRegion;
			float xdone = ((float)getExtent().x/(float)texture->mBitmapSize.x)+1;
			float ydone = ((float)getExtent().y/(float)texture->mBitmapSize.y)+1;

			int xshift = mStartPoint.x%texture->mBitmapSize.x;
			int yshift = mStartPoint.y%texture->mBitmapSize.y;
			for(int y = 0; y < ydone; ++y)
				for(int x = 0; x < xdone; ++x)
				{
					srcRegion.set(0,0,texture->mBitmapSize.x,texture->mBitmapSize.y);
					dstRegion.set( ((texture->mBitmapSize.x*x)+offset.x)-xshift,
						((texture->mBitmapSize.y*y)+offset.y)-yshift,
						texture->mBitmapSize.x,
						texture->mBitmapSize.y);
					GFX->getDrawUtil()->drawBitmapStretchSR(texture,dstRegion, srcRegion, GFXBitmapFlip_None, GFXTextureFilterLinear);
				}

		}
		else
		{
			RectI rect(offset, getExtent());
			GFX->getDrawUtil()->drawBitmapStretch(mTextureObject, rect, GFXBitmapFlip_None, GFXTextureFilterLinear, false);
		}
	}
}

bool GuiGridCtrl::setBitmapNameField( void *object, const char *index, const char *data )
{
	// Prior to this, you couldn't do bitmap.bitmap = "foo.jpg" and have it work.
	// With protected console types you can now call the setBitmap function and
	// make it load the image.
	static_cast<GuiGridCtrl*>( object )->setBitmap( data );

	// Return false because the setBitmap method will assign 'mBitmapName' to the
	// argument we are specifying in the call.
	return false;
}

bool GuiGridCtrl::setGridTypeField( void *object, const char *index, const char *data )
{
	GuiGridCtrl* guiGrid = static_cast<GuiGridCtrl*>(object);

	// Parse enum string
	struct EngineUnmarshallData<GridType::Enum> unmarshalFunctor;
	guiGrid->SetGridType(unmarshalFunctor(data));
	return false;
}

bool GuiGridCtrl::setColumnsField(void *object, const char *index, const char *data)
{
	GuiGridCtrl* guiGrid = static_cast<GuiGridCtrl*>(object);
	guiGrid->resizeGrid(Math::Clamp(dAtoi( data ), 0), guiGrid->GetRows(), guiGrid->GetCellSize());
	return false;
}

bool GuiGridCtrl::setRowsField(void *object, const char *index, const char *data)
{
	GuiGridCtrl* guiGrid = static_cast<GuiGridCtrl*>(object);
	guiGrid->resizeGrid(guiGrid->GetColumns(), Math::Clamp(dAtoi( data ), 0), guiGrid->GetCellSize());
	return false;
}

bool GuiGridCtrl::setCellSizeField(void *object, const char *index, const char *data)
{
	GuiGridCtrl* guiGrid = static_cast<GuiGridCtrl*>(object);

	Vector2I cellSize;
	Con::setData( TypePoint2I, &cellSize, 0, 1, &data);
	//Vector2I cellSize(Math::Clamp(dAtoi(StringUnit::getUnit(data, 0, " \t\n"))), Math::Clamp(dAtoi(StringUnit::getUnit(data, 1, " \t\n"))));
	guiGrid->resizeGrid(guiGrid->GetColumns(), guiGrid->GetRows(), cellSize);
	return false;
}

DefineEngineMethod( GuiGridCtrl, setOffset, void, ( S32 x, S32 y ),,
	"Set the offset of the bitmap within the control.\n"
	"@param x The x-axis offset of the image.\n"
	"@param y The y-axis offset of the image.\n")
{
	object->setOffset(x, y);
}

//"Set the bitmap displayed in the control. Note that it is limited in size, to 256x256."
ConsoleMethod( GuiGridCtrl, setBitmap, void, 3, 4,
   "( String filename | String filename, bool resize ) Assign an image to the control.\n\n"
   "@hide" )
{
   char filename[1024];
   Con::expandScriptFilename(filename, sizeof(filename), argv[2]);
   object->setBitmap(filename, argc > 3 ? dAtob( argv[3] ) : false );
}