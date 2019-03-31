#include "editorTool.h"

IMPLEMENT_CONOBJECT(EditorTool);

EditorTool::EditorTool()
{
   mWorldEditor = NULL;

   mUseMouseDown = true;
   mUseMouseUp = true;
   mUseMouseMove = true;

   mUseRightMouseDown = false;
   mUseRightMouseUp = false;
   mUseRightMouseMove = false;

   mUseMiddleMouseDown = true;
   mUseMiddleMouseUp = true;
   mUseMiddleMouseMove = true;

   mUseKeyInput = true;
}

bool EditorTool::onAdd()
{
   return Parent::onAdd();
}

void EditorTool::onRemove()
{
   Parent::onRemove();
}

//Called when the tool is activated on the World Editor
void EditorTool::onActivated(WorldEditor* editor)
{
   mWorldEditor = editor;
   Con::executef(this, "onActivated");
}

//Called when the tool is deactivated on the World Editor
void EditorTool::onDeactivated()
{
   mWorldEditor = NULL;
   Con::executef(this, "onDeactivated");
}

//
bool EditorTool::onMouseMove(const Gui3DMouseEvent &e)
{
   if (!mUseMouseDown)
      return false;

   Con::executef(this, "onMouseMove", e.mousePoint);
   return true;
}
bool EditorTool::onMouseDown(const Gui3DMouseEvent &e)
{
   if (!mUseMouseDown)
      return false;

   Con::executef(this, "onMouseDown", e.mousePoint);
   return true;
}
bool EditorTool::onMouseDragged(const Gui3DMouseEvent &e)
{
   Con::executef(this, "onMouseDragged", e.mousePoint);
   return true;
}
bool EditorTool::onMouseUp(const Gui3DMouseEvent &e)
{
   if (!mUseMouseDown)
      return false;

   Con::executef(this, "onMouseUp", e.mousePoint);
   return true;
}

//
bool EditorTool::onRightMouseDown(const Gui3DMouseEvent &e)
{
   if (!mUseRightMouseDown)
      return false;

   Con::executef(this, "onRightMouseDown", e.mousePoint);
   return true;
}
bool EditorTool::onRightMouseDragged(const Gui3DMouseEvent &e)
{
   Con::executef(this, "onRightMouseDragged", e.mousePoint);
   return true;
}
bool EditorTool::onRightMouseUp(const Gui3DMouseEvent &e)
{
   if (!mUseRightMouseDown)
      return false;

   Con::executef(this, "onRightMouseUp", e.mousePoint);
   return true;
}

//
bool EditorTool::onMiddleMouseDown(const Gui3DMouseEvent &e)
{
   if (!mUseMiddleMouseDown)
      return false;

   Con::executef(this, "onMiddleMouseDown", e.mousePoint);
   return true;
}
bool EditorTool::onMiddleMouseDragged(const Gui3DMouseEvent &e)
{
   Con::executef(this, "onMiddleMouseDragged", e.mousePoint);
   return true;
}
bool EditorTool::onMiddleMouseUp(const Gui3DMouseEvent &e)
{
   if (!mUseMiddleMouseDown)
      return false;

   Con::executef(this, "onMiddleMouseUp", e.mousePoint);
   return true;
}

//
bool EditorTool::onInputEvent(const InputEventInfo &e)
{
   if (!mUseKeyInput)
      return false;

   Con::executef(this, "onKeyPress", e.ascii, e.modifier);
   return true;
}

//
void render(SceneRenderState *);