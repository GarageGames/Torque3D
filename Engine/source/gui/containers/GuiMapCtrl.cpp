//=============================================================================
// GuiMapCtrl.cpp ~ 04/13/2013 ~ Author: Tim
//
// Defines GuiMapCtrl: 
//
// Copyright (C) 2013 - All Rights Reserved
//=============================================================================

//- Headers -------------------------------------------------------------------
#include "console/engineAPI.h"
#include "GuiMapCtrl.h"
#include "gui/containers/guiGridCtrl.h"
#include "gui/core/guiCanvas.h"

//- Class Implementation ------------------------------------------------------
// GuiMapCtrl:

//- Static Data ---------------------------------------------------------------
IMPLEMENT_CONOBJECT( GuiMapCtrl );

ConsoleDocClass( GuiMapCtrl,
   "@brief A container that allows to view a larger GUI control inside its smaller area "
		"and provides mouse/keyboard navigation to strafe through this space..\n\n"
   "@ingroup GuiContainers"
);

//- Standard Functions --------------------------------------------------------
GuiMapCtrl::GuiMapCtrl()
{
	strafeControl = StrafeControl::RightMouseStrafe;
	mForceVScrollBar = ScrollBarAlwaysOff;
	mForceHScrollBar = ScrollBarAlwaysOff;
}

GuiMapCtrl::~GuiMapCtrl()
{

}

//- Accessors -----------------------------------------------------------------

//- Operators -----------------------------------------------------------------

//- Public Functions ----------------------------------------------------------
bool GuiMapCtrl::onKeyDown(const GuiEvent& event)
{
	return Parent::onKeyDown(event);
}

void GuiMapCtrl::onRightMouseDown(const GuiEvent& event)
{
	if (strafeControl == StrafeControl::RightMouseStrafe)
	{
		startStrafe(event.mousePoint);
	}
	else return Parent::onRightMouseDown(event);
}

void GuiMapCtrl::onRightMouseUp(const GuiEvent& event)
{
	if (strafeControl == StrafeControl::RightMouseStrafe)
	{
		mouseUnlock();
	}
	else return Parent::onRightMouseUp(event);
}

void GuiMapCtrl::onRightMouseDragged(const GuiEvent& event)
{
	if (strafeControl == StrafeControl::RightMouseStrafe)
	{
		Point2I delta = strafeStartPosition - event.mousePoint;
		delta += strafeStartOffset;
		scrollTo(delta.x, delta.y);
	}
	else return Parent::onRightMouseDragged(event);
}

void GuiMapCtrl::onMiddleMouseDown(const GuiEvent& event)
{
	if (strafeControl == StrafeControl::MiddleMouseStrafe)
	{
		startStrafe(event.mousePoint);
	}
	else return Parent::onMiddleMouseDown(event);
}

void GuiMapCtrl::onMiddleMouseUp(const GuiEvent& event)
{
	if (strafeControl == StrafeControl::MiddleMouseStrafe)
	{
		mouseUnlock();
	}
	else return Parent::onMiddleMouseUp(event);
}

void GuiMapCtrl::onMiddleMouseDragged(const GuiEvent& event)
{
	if (strafeControl == StrafeControl::MiddleMouseStrafe)
	{
		Point2I delta = strafeStartPosition - event.mousePoint;
		delta += strafeStartOffset;
		scrollTo(delta.x, delta.y);
	}
	else return Parent::onMiddleMouseDragged(event);
}

//- Protected Functions -------------------------------------------------------
void GuiMapCtrl::startStrafe(Point2I offset)
{
	GuiCanvas* canvas = getRoot();
	if( canvas->getMouseLockedControl() )
	{
		GuiEvent event;
		canvas->getMouseLockedControl()->onMouseLeave(event);
		canvas->mouseUnlock( canvas->getMouseLockedControl() );
	}
	canvas->mouseLock(this);
	canvas->setFirstResponder(this);
	strafeStartPosition = offset;
	strafeStartOffset = mChildRelPos;
}

//- Private Functions ---------------------------------------------------------

//- Input/Output --------------------------------------------------------------

//- Nested Types --------------------------------------------------------------