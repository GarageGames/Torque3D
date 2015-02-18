//-----------------------------------------------------------------------------
// Copyright (c) 2012 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Shape Editor Profiles
//-----------------------------------------------------------------------------

singleton GuiControlProfile(GuiShapeEdScrollProfile : GuiEditorScrollProfile)
{
   // Don't clear the scroll area (since we need to be able to see the GuiContainer
   // underneath that provides the fill color for the header row)
   opaque = false;
   category = "Editor";
};

singleton GuiControlProfile(GuiShapeEdTextListProfile : ToolsGuiTextListProfile)
{
   // Customise the not-active font used for the header row
   fontColorNA = "75 75 75";
   category = "Editor";
};

singleton GuiControlProfile(GuiShapeEdRolloutProfile : GuiInspectorRolloutProfile0)
{
   bitmap = "tools/editorClasses/gui/images/rollout";
   category = "Editor";
};

singleton GuiControlProfile( GuiShapeEdTransitionSliderProfile )
{
   bitmap = "tools/shapeEditor/images/transition_slider";
   category = "Core";
};
