#pragma once

#ifndef _ENGINEAPI_H_
#include "console/engineAPI.h"
#endif
#ifndef _EDITOR_TOOL_
#include "gui/worldEditor/tools/editorTool.h"
#endif

class @ : public EditorTool
{
	typedef EditorTool Parent;
private:

protected:

public:
	@();
	virtual ~@();

	//This registers the class type and namespace for the console system
	DECLARE_CONOBJECT(@);

    /// SimObject handling
	virtual bool onAdd();
    virtual void onRemove();

	/// Management
	//Called when the tool is activated on the World Editor
    virtual void onActivated(WorldEditor*);
	//Called when the tool is deactivated on the World Editor
	virtual void onDeactivated();

	/// Input handling
	virtual bool onMouseMove(const Gui3DMouseEvent &);
	virtual bool onMouseDown(const Gui3DMouseEvent &);
	virtual bool onMouseDragged(const Gui3DMouseEvent &);
	virtual bool onMouseUp(const Gui3DMouseEvent &);

	virtual bool onRightMouseDown(const Gui3DMouseEvent &);
	virtual bool onRightMouseDragged(const Gui3DMouseEvent &);
	virtual bool onRightMouseUp(const Gui3DMouseEvent &);

	virtual bool onMiddleMouseDown(const Gui3DMouseEvent &);
	virtual bool onMiddleMouseDragged(const Gui3DMouseEvent &);
	virtual bool onMiddleMouseUp(const Gui3DMouseEvent &);

	virtual bool onInputEvent(const InputEventInfo &);

	/// Rendering
	virtual void render();
};