#include "platform/platform.h"
#include "gui/buttons/guiIconSpawnerButtonCtrl.h"

#include "console/console.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "gfx/gfxDevice.h"
#include "gfx/gfxDrawUtil.h"

#include "gui/controls/guiIconControl.h"
#include "gui/containers/guiDragAndDropCtrl.h"
#include "gui/core/guiCanvas.h"
#include "gui/core/guiDefaultControlRender.h"

IMPLEMENT_CONOBJECT( GuiIconSpawnerButtonCtrl );

ConsoleDocClass( GuiIconSpawnerButtonCtrl,
				"@brief An icon spawner button generates drag and drop objects which will spawn in new icon instances when dropped on an icon panel\n\n"
				//"@tsexample\n"
				//"@endtsexample\n\n"
				"@ingroup GuiButtons"
				);

//-----------------------------------------------------------------------------

GuiIconSpawnerButtonCtrl::GuiIconSpawnerButtonCtrl()
{
	mDragSourceControl = NULL;
	mButtonText = StringTable->insert( "" );   

	mFitBitmapToButton = true;
	mMakeIconSquare = false;
	mAutoSize = true;
	mCanDrag = true;

	setExtent(32, 32);    

	static StringTableEntry sProfile = StringTable->insert( "profile" );
	setDataField( sProfile, NULL, "GuiIconSpawnerButtonCtrl" );
}

void GuiIconSpawnerButtonCtrl::initPersistFields()
{
	addField( "buttonMargin",     TypePoint2I,   Offset( mButtonMargin, GuiIconSpawnerButtonCtrl ),"Margin area around the button.\n");
	addField( "iconBitmap",       TypeFilename,  Offset( mBitmapName, GuiIconSpawnerButtonCtrl ),"Bitmap file for the icon to display on the button.\n");
	addField( "sizeIconToButton", TypeBool,      Offset( mFitBitmapToButton, GuiIconSpawnerButtonCtrl ),"If true, the icon will be scaled to be the same size as the button.\n");
	addField( "makeIconSquare",   TypeBool,      Offset( mMakeIconSquare, GuiIconSpawnerButtonCtrl ),"If true, will make sure the icon is square.\n");
	addField( "autoSize",         TypeBool,      Offset( mAutoSize, GuiIconSpawnerButtonCtrl ),"If true, the text and icon will be automatically sized to the size of the control.\n");

	Parent::Parent::initPersistFields();
}

void GuiIconSpawnerButtonCtrl::onMouseDragged(const GuiEvent &event)
{
	GuiDragAndDropControl* container = new GuiDragAndDropControl();
	container->registerObject();
	//container->assignFieldsFrom(this);
	container->setCanSaveDynamicFields(false);

	Point2I localCoords = globalToLocalCoord(event.mousePoint);
	container->setPosition(event.mousePoint);
	container->setMinExtent(Point2I(4, 4));
	container->setVisible(true);
	container->setCanSave(true);
	container->setDataField( StringTable->insert("hovertime"), NULL, "1000" );

	GuiIconControl* icon;
	icon = new GuiIconControl();
	icon->registerObject();
	icon->setBitmap(getBitmapName());
	icon->setExtent(getExtent());
	icon->setMakeIconSquare(getMakeIconSquare());
	icon->setAutoSize(getAutoSize());
	icon->setFitBitmapToButton(getFitBitmapToButton());
	icon->setControlProfile(getControlProfile());
	icon->setTooltipProfile(getTooltipProfile());
	icon->setUseMouseEvents(true);
	icon->SetStringData(GetStringData());

	container->setExtent(icon->getExtent());
	container->addObject(icon);
	getRoot()->getContentControl()->addObject(container);
	container->startDragging(localCoords);
}

bool GuiIconSpawnerButtonCtrl::resize(const Point2I &newPosition, const Point2I &newExtent)
{   
	if (!mAutoSize)   
		return Parent::resize( newPosition, newExtent );

	Point2I autoExtent( mMinExtent );
	autoExtent.y = mTextureNormal.getHeight() + mButtonMargin.y * 2;
	autoExtent.x = mTextureNormal.getWidth() + mButtonMargin.x * 2;
	return Parent::Parent::resize( newPosition, autoExtent );
}

void GuiIconSpawnerButtonCtrl::renderButton( Point2I &offset, const RectI& updateRect )
{
	bool depressed = mDepressed;

	RectI boundsRect(offset, getExtent());
	GFXDrawUtil *drawer = GFX->getDrawUtil();
	if (mDepressed || mStateOn)
	{
		// If there is a bitmap array then render using it.  
		// Otherwise use a standard fill.
		if(mProfile->mUseBitmapArray && mProfile->mBitmapArrayRects.size())
			renderBitmapArray(boundsRect, statePressed);
		else
			renderSlightlyLoweredBox(boundsRect, mProfile);
	}
	else if(mMouseOver && mActive)
	{
		// If there is a bitmap array then render using it.  
		// Otherwise use a standard fill.
		if(mProfile->mUseBitmapArray && mProfile->mBitmapArrayRects.size())
			renderBitmapArray(boundsRect, stateMouseOver);
		else
			renderSlightlyRaisedBox(boundsRect, mProfile);
	}
	else
	{
		// If there is a bitmap array then render using it.  
		// Otherwise use a standard fill.
		if(mProfile->mUseBitmapArray && mProfile->mBitmapArrayRects.size())
		{
			if(mActive)
				renderBitmapArray(boundsRect, stateNormal);
			else
				renderBitmapArray(boundsRect, stateDisabled);
		}
		else
		{
			drawer->drawRectFill(boundsRect, mProfile->mFillColorNA);
			drawer->drawRect(boundsRect, mProfile->mBorderColorNA);
		}
	}

	Point2I textPos = offset;
	if(depressed)
		textPos += Point2I(1,1);


	// Render the icon
	if (mTextureNormal)
	{
		// Render the normal bitmap
		drawer->clearBitmapModulation();

		// Maintain the bitmap size or fill the button?
		RectI iconRect( 0, 0, 0, 0 );
		if ( !mFitBitmapToButton )
		{
			Point2I textureSize( mTextureNormal->getWidth(), mTextureNormal->getHeight() );
			iconRect.set( offset + mButtonMargin, textureSize );

			// Center the icon
			iconRect.point.x = offset.x + ( getWidth() - textureSize.x ) / 2;
			iconRect.point.y = offset.y + ( getHeight() - textureSize.y ) / 2;

			drawer->drawBitmapStretch( mTextureNormal, iconRect );
		} 
		else
		{
			iconRect.set( offset + mButtonMargin, getExtent() - (mButtonMargin * 2) );
			if ( mMakeIconSquare )
			{
				// Square the icon to the smaller axis extent.
				if ( iconRect.extent.x < iconRect.extent.y )
					iconRect.extent.y = iconRect.extent.x;
				else
					iconRect.extent.x = iconRect.extent.y;            
			}
			drawer->drawBitmapStretch( mTextureNormal, iconRect );
		}
	}
	renderChildControls( offset, updateRect);
}
