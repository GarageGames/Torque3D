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

singleton GuiControlProfile (EditorDefaultProfile)
{
   opaque = true;
   category = "Editor";
};

singleton GuiControlProfile (EditorToolButtonProfile)
{
   opaque = true;
   border = 2;
   category = "Editor";
};

singleton GuiControlProfile (EditorTextProfile)
{
   fontType = "Arial Bold";
   fontColor = "0 0 0";
   autoSizeWidth = true;
   autoSizeHeight = true;
   category = "Editor";
};

singleton GuiControlProfile (EditorTextProfileWhite)
{
   fontType = "Arial Bold";
   fontColor = "255 255 255";
   autoSizeWidth = true;
   autoSizeHeight = true;
   category = "Editor";
};

singleton GuiControlProfile (WorldEditorProfile)
{
   canKeyFocus = true;
   category = "Editor";
};

singleton GuiControlProfile (EditorScrollProfile)
{
   opaque = true;
   fillColor = "192 192 192 192";
   border = 3;
   borderThickness = 2;
   borderColor = "0 0 0";
   bitmap = "tools/gui/images/scrollBar";
   hasBitmapArray = true;
   category = "Editor";
};

singleton GuiControlProfile (GuiEditorClassProfile)
{
   opaque = true;
   fillColor = "232 232 232";
   border = true;
   borderColor   = "0 0 0";
   borderColorHL = "127 127 127";
   fontColor = "0 0 0";
   fontColorHL = "50 50 50";
   fixedExtent = true;
   justify = "center";
   bitmap = "tools/gui/images/scrollBar";
   hasBitmapArray = true;
   category = "Editor";
};

singleton GuiControlProfile( EPainterBitmapProfile )
{
   opaque = false;
   border = false;
   borderColor ="243 242 241";
   Color ="230 230 230";
   category = "Editor";
};

singleton GuiControlProfile( EPainterBorderButtonProfile : ToolsGuiDefaultProfile )
{
   border = true;
   borderColor = "0 0 0";
   borderThickness = 2;
   
   fontColorHL = "255 0 0";
   fontColorSEL = "0 0 255";
   category = "Editor";
};

singleton GuiControlProfile( EPainterDragDropProfile )
{
   justify = "center";
   fontColor = "0 0 0";
   border = 0;
   textOffset = "0 0";   
   opaque = true;
   fillColor = "221 221 221 150";
   category = "Editor";
};

singleton GizmoProfile( GlobalGizmoProfile )
{
   // This isnt a GuiControlProfile but fits in well here.
   // Don't really have to initialize this now because that will be done later 
   // based on the saved editor prefs.
   screenLength = 100;
   category = "Editor";
};
