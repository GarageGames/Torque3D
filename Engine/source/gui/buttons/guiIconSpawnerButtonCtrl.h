#ifndef GUIICONSPAWNERBUTTONCTRL_H_
#define GUIICONSPAWNERBUTTONCTRL_H_

#ifndef _GUIBUTTONBASECTRL_H_
#include "gui/buttons/guiIconButtonCtrl.h"
#endif

#ifndef GFX_Texture_Manager_H_
#include "gfx/gfxTextureManager.h"
#endif

/// A button that spawns drag and drop icons
class GuiIconSpawnerButtonCtrl : public GuiIconButtonCtrl
{
public:
	typedef GuiIconButtonCtrl Parent;
	DECLARE_CONOBJECT( GuiIconSpawnerButtonCtrl );
	DECLARE_DESCRIPTION( "A button that spawns drag and drop icons" );

protected:
	GuiControl* mDragSourceControl;

public:
	GuiIconSpawnerButtonCtrl();

	void onMouseDragged(const GuiEvent &event);

	bool resize(const Point2I &newPosition, const Point2I &newExtent);

	static void initPersistFields();

protected:
	void renderButton( Point2I &offset, const RectI& updateRect);
};

#endif // GUIICONSPAWNERBUTTONCTRL_H_
