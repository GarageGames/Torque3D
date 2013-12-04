#include "gui/containers/guiMoveableCtrl.h"
#include "gui/core/guiCanvas.h"
#include "console/engineAPI.h"


IMPLEMENT_CONOBJECT( GuiMoveableControl );

ConsoleDocClass( GuiMoveableControl,
	"@brief A special control that allows controls to be moved by dragging and dropping,"
	"without duplication (as in the case of guiDragAndDropCtrl\n\n"
   
   "@see GuiDragAndDropCtrl\n"
   "@see GuiControl::onControlDragEnter\n"
   "@see GuiControl::onControlDragExit\n"
   "@see GuiControl::onControlDragged\n"
   "@see GuiControl::onControlDropped\n\n"
   
   "@ingroup GuiUtil"
);

void GuiMoveableControl::setTarget(GuiControl* control)
{
	if (control != NULL)
	{
		sourceContainer = dynamic_cast<GuiContainer*>(control->getParent());
		sourcePosition = control->getPosition();

		addObject(control);

		setExtent(control->getExtent());
		control->setPosition(Point2I::Zero);
	}
}

void GuiMoveableControl::onMouseUp(const GuiEvent& event)
{
	mouseUnlock();

	GuiControl* target = findDragTarget( event.mousePoint );
	GuiControl* payload = dynamic_cast< GuiControl* >(at( 0 ));
	if( target )
	{
		bool succeeded = target->onControlDropped(payload, getDropPoint());
		//removeObject(payload);

		if (!succeeded && payload != NULL)
		{
			if (sourceContainer == NULL)
			{
				payload->unregisterObject();
				delete payload;
			}
			else
			{
				payload->setPosition(sourcePosition);
				sourceContainer->addObject(payload);
			}
		}
		sourceContainer = NULL;
	}
	else if (payload != NULL)
	{
		AssertFatal(sourceContainer != NULL, "No target and invalid source container - Gui payload will be lost");
		payload->setPosition(sourcePosition);
		sourceContainer->addObject(payload);
		sourceContainer = NULL;
	}

	if (mDeleteOnMouseUp)
		deleteObject();
}

void GuiMoveableControl::onRender(Point2I offset, const RectI &updateRect)
{
	// Render Children
	renderChildControls(offset, updateRect);
}

void GuiMoveableControl::initPersistFields()
{
	//addField( "deleteOnMouseUp", TypeBool, Offset( mDeleteOnMouseUp, GuiDragAndDropControl ),
	// "If true, the control deletes itself when the left mouse button is released.\n\n"
	// "If at this point, the drag&drop control still contains its payload, it will be deleted along with the control." );

	Parent::initPersistFields();
}

DefineEngineMethod( GuiMoveableControl, setTarget, void, ( GuiControl* control), ,
   "Specify drag target's source container and position in order to restore on failed move\n\n")
{
   object->setTarget(control);
}
