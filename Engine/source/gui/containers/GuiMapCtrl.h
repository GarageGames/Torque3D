//=============================================================================
// GuiMapCtrl.h ~ 04/13/2013 ~ Author: Tim
//
// Declares GuiMapCtrl: 
//
// Copyright (C) 2013 - All Rights Reserved
//=============================================================================

#if !defined(GUIMAPCTRL_H)
#define GUIMAPCTRL_H

//- Headers -------------------------------------------------------------------
#ifndef _GUISCROLLCTRL_H_
#include "gui/containers/guiScrollCtrl.h"
#endif

//- Forward Declarations ------------------------------------------------------
class GuiGridCtrl;

//- Class Declaration ---------------------------------------------------------
// GuiMapCtrl:
class GuiMapCtrl : public GuiScrollCtrl
{
	typedef GuiScrollCtrl Parent;

	DECLARE_CONOBJECT(GuiMapCtrl);
	DECLARE_DESCRIPTION( "A container that allows to view a larger GUI control inside its smaller area "
		"and provides mouse/keyboard navigation to strafe through this space." );

public:
    //- Nested Types ----------------------------------------------------------
	struct StrafeControl
	{
	public:
	    enum Enum
	    {
	        MiddleMouseStrafe,
			RightMouseStrafe,
	        Total,
	    };
	
	private:
	    StrafeControl();
	};

	//- Public Member Data ----------------------------------------------------

    //- Static Data -----------------------------------------------------------

protected:
    //- Protected Member Data -------------------------------------------------
	StrafeControl::Enum strafeControl;
	Point2I strafeStartPosition;
	Point2I strafeStartOffset;

public:
    //- Standard Functions ----------------------------------------------------
    GuiMapCtrl();
	virtual ~GuiMapCtrl();

    //- Accessors -------------------------------------------------------------

    //- Operators -------------------------------------------------------------

    //- Public Functions ------------------------------------------------------
	virtual bool onKeyDown(const GuiEvent& event);

	virtual void onRightMouseDown(const GuiEvent& event);
	virtual void onRightMouseUp(const GuiEvent& event);
	virtual void onRightMouseDragged(const GuiEvent& event);

	virtual void onMiddleMouseDown(const GuiEvent& event);
	virtual void onMiddleMouseUp(const GuiEvent& event);
	virtual void onMiddleMouseDragged(const GuiEvent& event);

	//- Static Functions ------------------------------------------------------

protected:
    //- Protected Functions ---------------------------------------------------
	void startStrafe(Point2I offset);

private:
    //- Private Functions -----------------------------------------------------
};

#endif //GUIMAPCTRL_H
