#include "console/engineAPI.h"
#include "platform/platform.h"
#include "gui/containers/guiUnorderedIconCollection.h"

IMPLEMENT_CONOBJECT(GuiUnorderedIconCollection);
ConsoleDocClass(GuiUnorderedIconCollection,
	"@brief A gui container that accepts dragged and dropped icons without enforcing any organization scheme\n\n"
	"@ingroup GuiContainers");

GuiUnorderedIconCollection::GuiUnorderedIconCollection()
{
	mAcceptDrops = true;
	mIsContainer = true;
}

GuiUnorderedIconCollection::~GuiUnorderedIconCollection()
{
}

// ConsoleObject...
void GuiUnorderedIconCollection::initPersistFields()
{
	Parent::initPersistFields();
}


// SimObject...
void GuiUnorderedIconCollection::inspectPostApply()
{
	resize(getPosition(), getExtent());
	Parent::inspectPostApply();
}

// SimSet...
void GuiUnorderedIconCollection::addObject(SimObject *obj)
{
	Parent::addObject(obj);
}

// GuiControl...
bool GuiUnorderedIconCollection::resize(const Point2I &newPosition, const Point2I &newExtent)
{
	return Parent::resize(newPosition, newExtent);
}

void GuiUnorderedIconCollection::childResized(GuiControl *child)
{
	Parent::childResized(child);
}

bool GuiUnorderedIconCollection::onControlDropped(GuiControl* payload, Point2I position)
{
	addObject(payload);

	Point2I localPosition = globalToLocalCoord(position) - payload->getExtent() / 2;
	payload->setPosition(localPosition);

	//payload->setPosition(position.x - getPosition().x - payload->getExtent().x/2, position.y - getPosition().y - payload->getExtent().y/2);
	return true;
}

void GuiUnorderedIconCollection::refresh()
{
	resize( getPosition(), getExtent() );
}

//DefineEngineMethod( GuiDynamicCtrlArrayControl, refresh, void, (),,
//				   "Recalculates the position and size of this control and all its children." )
//{
//	object->refresh();
//}
