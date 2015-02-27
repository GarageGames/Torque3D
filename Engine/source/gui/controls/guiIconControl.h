#ifndef _GUIICONCONTROL_H_
#define _GUIICONCONTROL_H_

#ifndef _GUIICONBUTTON_H_
#include "gui/buttons/guiIconButtonCtrl.h"
#endif
#ifndef GFX_Texture_Manager_H_
#include "gfx/gfxTextureManager.h"
#endif


/// The GuiIconControl draws a movable icon and text caption with several layout options.
class GuiIconControl : public GuiIconButtonCtrl
{
private:
	typedef GuiIconButtonCtrl Parent;

public:   
	GuiIconControl();

	static void initPersistFields();

	DECLARE_CONOBJECT(GuiIconControl);
	DECLARE_CATEGORY( "Gui Images" );
	DECLARE_DESCRIPTION( "A moveable control that displays an icon and optional text label." );

protected:
	virtual void renderButton( Point2I &offset, const RectI& updateRect);
};

#endif //_GUIICONCONTROL_H_
