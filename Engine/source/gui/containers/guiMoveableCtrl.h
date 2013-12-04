#ifndef _GUIMOVEABLECTRL_H_
#define _GUIMOVEABLECTRL_H_

#ifndef _GUIDRAGANDDROPCTRL_H_
#include "gui/containers/guiDragAndDropCtrl.h"
#endif
#ifndef _GUICONTAINER_H_
#include "gui/containers/guiContainer.h"
#endif

/// A special control that allows controls to be moved by dragging and dropping, 
/// without duplication (as in the case of guiDragAndDropCtrl)
class GuiMoveableControl : public GuiDragAndDropControl
{
public:
	typedef GuiDragAndDropControl Parent;

private:
	GuiContainer* sourceContainer;
	Point2I sourcePosition;

public:
	GuiMoveableControl() : sourceContainer(NULL) {}

	void setTarget(GuiControl* control);

	virtual void onMouseUp(const GuiEvent& event);
	virtual void onRender(Point2I offset, const RectI &updateRect);

	static void initPersistFields();

	DECLARE_CONOBJECT( GuiMoveableControl );
	DECLARE_CATEGORY( "Gui Other" );
	DECLARE_DESCRIPTION( "A special control that allows controls to be moved by dragging and dropping,"
	"without duplication (as in the case of guiDragAndDropCtrl\n"
	"The control will notify other controls as it moves across the canvas.\n"
	"Content can be attached through dynamic fields or child objects." );
};

#endif //_GUIMOVEABLECTRL_H_