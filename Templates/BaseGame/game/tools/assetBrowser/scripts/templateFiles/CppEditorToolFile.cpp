#include "@.h"

@::@() : EditorTool()
{
}

@::~@()
{
}

IMPLEMENT_CONOBJECT(@);

/// SimObject handling
bool @::onAdd()
{
	if(!Parent::onAdd())
		return false;

	return true;
}

void @::onRemove()
{
	Parent::onRemove();
}

//Called when the tool is activated on the World Editor
void @::onActivated(WorldEditor* editor)
{
   Parent::onActivated(editor);
}

//Called when the tool is deactivated on the World Editor
void @::onDeactivated()
{
   Parent::onDeactivated();
}

//
bool @::onMouseMove(const Gui3DMouseEvent &e)
{
   if (!mUseMouseDown)
      return false;

   Con::executef(this, "onMouseMove", e.mousePoint);
   return true;
}
bool @::onMouseDown(const Gui3DMouseEvent &e)
{
   if (!mUseMouseDown)
      return false;

   Con::executef(this, "onMouseDown", e.mousePoint);
   return true;
}
bool @::onMouseDragged(const Gui3DMouseEvent &e)
{
   Con::executef(this, "onMouseDragged", e.mousePoint);
   return true;
}
bool @::onMouseUp(const Gui3DMouseEvent &e)
{
   if (!mUseMouseDown)
      return false;

   Con::executef(this, "onMouseUp", e.mousePoint);
   return true;
}

//
bool @::onRightMouseDown(const Gui3DMouseEvent &e)
{
   if (!mUseRightMouseDown)
      return false;

   Con::executef(this, "onRightMouseDown", e.mousePoint);
   return true;
}
bool @::onRightMouseDragged(const Gui3DMouseEvent &e)
{
   Con::executef(this, "onRightMouseDragged", e.mousePoint);
   return true;
}
bool @::onRightMouseUp(const Gui3DMouseEvent &e)
{
   if (!mUseRightMouseDown)
      return false;

   Con::executef(this, "onRightMouseUp", e.mousePoint);
   return true;
}

//
bool @::onMiddleMouseDown(const Gui3DMouseEvent &e)
{
   if (!mUseMiddleMouseDown)
      return false;

   Con::executef(this, "onMiddleMouseDown", e.mousePoint);
   return true;
}
bool @::onMiddleMouseDragged(const Gui3DMouseEvent &e)
{
   Con::executef(this, "onMiddleMouseDragged", e.mousePoint);
   return true;
}
bool @::onMiddleMouseUp(const Gui3DMouseEvent &e)
{
   if (!mUseMiddleMouseDown)
      return false;

   Con::executef(this, "onMiddleMouseUp", e.mousePoint);
   return true;
}

//
bool @::onInputEvent(const InputEventInfo &e)
{
   if (!mUseKeyInput)
      return false;

   Con::executef(this, "onKeyPress", e.ascii, e.modifier);
   return true;
}

//
void @::render()
{
}