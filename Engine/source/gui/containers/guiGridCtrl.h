#ifndef _GUIGRIDCTRL_H_
#define _GUIGRIDCTRL_H_

#ifndef _GUICONTAINER_H_
#include "gui/containers/guiContainer.h"
#endif
#ifndef ARRAY2D_H
#include "core/util/Array2D.h"
#endif

#include "gfx/gfxDevice.h"
#include "console/console.h"
#include "console/consoleTypes.h"

class GuiGridCtrl : public GuiContainer
{
	typedef GuiContainer Parent;
	DECLARE_CONOBJECT(GuiGridCtrl);
	DECLARE_CATEGORY( "Gui Containers" );

protected:
	GridType::Enum mGridType;

	Vector2I mCellSize;
	S32 mColumns;
	S32 mRows;

	Array2D<GuiControl*> mGrid;

	/// Name of the bitmap file.  If this is 'texhandle' the bitmap is not loaded
	/// from a file but rather set explicitly on the control.
	String mBitmapName;

	/// Loaded texture.
	GFXTexHandle mTextureObject;

	Point2I mStartPoint;

	/// If true, bitmap tiles inside control.  Otherwise stretches.
	bool mWrap;

public:

	GuiGridCtrl();
	virtual ~GuiGridCtrl();

	// ConsoleObject
	static void initPersistFields();

	const GridType::Enum GetGridType() const { return mGridType; }
	void SetGridType(GridType::Enum value);

	void setBitmap(const char *name,bool resize = false);
	void setBitmapHandle(GFXTexHandle handle, bool resize = false);
	void setOffset(S32 x, S32 y);

	const S32 GetColumns() const { return mColumns; }
	void SetColumns(S32 value);

	const S32 GetRows() const { return mRows; }
	void SetRows(S32 value);

	const Vector2I& GetCellSize() const { return mCellSize; }
	void SetCellSize(S32 value);
	void SetCellSize(const Vector2I& value);

	const String& GetBitmapName() const { return mBitmapName; }
	GuiControl* getChild(const Point2I& position) { return mGrid(position); }

	// SimObject
	virtual void inspectPostApply();

	// SimSet
	virtual void addObject(SimObject *obj);
	virtual void addObjectToGrid(GuiControl* control, const Vector2I& coordinates);

	virtual void removeObject(SimObject *obj);

	// GuiControl
	virtual void updateSizing();
	virtual bool resize(const Point2I &newPosition, const Point2I &newExtent);
	
	virtual void resizeGrid(S32 inColumns, S32 inRows, S32 inCellSize);
	virtual void resizeGrid(S32 inColumns, S32 inRows, const Vector2I& inCellSize);

	// GuiDynamicCtrlArrayCtrl
	virtual void refresh();

	virtual bool onWake();
	virtual void onSleep();
	virtual void onRender(Point2I offset, const RectI& updateRect);

	virtual bool onControlDropped(GuiControl* payload, Point2I position);

	virtual void localPositionToGridCoordinates(Point2I& outCoordinates, const Point2I& inLocalPosition);
	virtual void globalPositionToGridCoordinates(Point2I& outCoordinates, const Point2I& inGlobalPosition);

	virtual void gridCoordinatesToLocalPosition(Point2I& outLocalPosition, const Point2I& inCoordinates);
	virtual void gridCoordinatesToGlobalPosition(Point2I& outGlobalPosition, const Point2I& inCoordinates);

protected:
	void renderBackground(Point2I offset, const RectI& updateRect);
	void renderGrid(Point2I offset, const RectI& updateRect);

	// Property field accessors (editor access)
	static bool setBitmapNameField( void *object, const char *index, const char *data );
	static bool setGridTypeField( void *object, const char *index, const char *data );
	static bool setColumnsField( void *object, const char *index, const char *data );
	static bool setRowsField( void *object, const char *index, const char *data );
	static bool setCellSizeField( void *object, const char *index, const char *data );
};

#endif // _GUIGRIDCTRL_H_