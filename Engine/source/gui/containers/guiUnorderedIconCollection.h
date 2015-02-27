#ifndef _GUIUNORDEREDICONCOLLECTION_H_
#define _GUIUNORDEREDICONCOLLECTION_H_

#ifndef _GUICONTAINER_H_
#include "gui/containers/guiContainer.h"
#endif

#include "gfx/gfxDevice.h"
#include "console/console.h"
#include "console/consoleTypes.h"

/// A gui container that accepts dragged and dropped icons without enforcing any organization scheme
class GuiUnorderedIconCollection : public GuiContainer
{
	typedef GuiContainer Parent;

public:
	GuiUnorderedIconCollection();
	virtual ~GuiUnorderedIconCollection();

	DECLARE_CONOBJECT(GuiUnorderedIconCollection);
	DECLARE_CATEGORY( "Gui Containers" );

	// ConsoleObject
	static void initPersistFields();

	// SimObject
	void inspectPostApply();

	// SimSet
	void addObject(SimObject *obj);

	virtual bool onControlDropped(GuiControl* payload, Point2I position);

	// GuiControl
	bool resize(const Point2I &newPosition, const Point2I &newExtent);
	void childResized(GuiControl *child);

	// GuiDynamicCtrlArrayCtrl
	void refresh();
};

#endif // _GUIUNORDEREDICONCOLLECTION_H_