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


singleton GuiControlProfile (NavPanelProfile) 
{
   opaque = false;
   border = -2;
   category = "Editor";
};


singleton GuiControlProfile (NavPanel : NavPanelProfile) 
{
   bitmap = "./navPanel";
   category = "Editor";
};

singleton GuiControlProfile (NavPanelBlue : NavPanelProfile) 
{
   bitmap = "./navPanel_blue";
   category = "Editor";
};

singleton GuiControlProfile (NavPanelGreen : NavPanelProfile) 
{
   bitmap = "./navPanel_green";
   category = "Editor";
};

singleton GuiControlProfile (NavPanelRed : NavPanelProfile) 
{
   bitmap = "./navPanel_red";
   category = "Editor";
};

singleton GuiControlProfile (NavPanelWhite : NavPanelProfile) 
{
   bitmap = "./navPanel_white";
   category = "Editor";
};

singleton GuiControlProfile (NavPanelYellow : NavPanelProfile) 
{
   bitmap = "./navPanel_yellow";
   category = "Editor";
};
singleton GuiControlProfile (menubarProfile : NavPanelProfile) 
{
   bitmap = "./menubar";
   category = "Editor";
};
singleton GuiControlProfile (editorMenubarProfile : NavPanelProfile) 
{
   bitmap = "./editor-menubar";
   category = "Editor";
};
singleton GuiControlProfile (editorMenu_wBorderProfile : NavPanelProfile) 
{
   bitmap = "./menu-fullborder";
   category = "Editor";
};
singleton GuiControlProfile (inspectorStyleRolloutProfile : NavPanelProfile) 
{
   bitmap = "./inspector-style-rollout";
   category = "Editor";
};
singleton GuiControlProfile (inspectorStyleRolloutListProfile : NavPanelProfile) 
{
   bitmap = "./inspector-style-rollout-list";
   category = "Editor";
};
singleton GuiControlProfile (inspectorStyleRolloutDarkProfile : NavPanelProfile) 
{
   bitmap = "./inspector-style-rollout-dark";
   category = "Editor";
};
singleton GuiControlProfile (inspectorStyleRolloutInnerProfile : NavPanelProfile) 
{
   bitmap = "./inspector-style-rollout_inner";
   category = "Editor";
};
singleton GuiControlProfile (inspectorStyleRolloutNoHeaderProfile : NavPanelProfile)
{
   bitmap = "./inspector-style-rollout-noheader";
   category = "Editor";
};
singleton GuiControlProfile (IconDropdownProfile : NavPanelProfile) 
{
   bitmap = "./icon-dropdownbar";
   category = "Editor";
};
