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

//------------------------------------------------------------------------------
// Editor Cursors
//------------------------------------------------------------------------------

new GuiCursor(EditorHandCursor)
{
   hotSpot = "7 0";
   bitmapName = "~/worldEditor/images/CUR_hand.png";
};

new GuiCursor(EditorRotateCursor)
{
   hotSpot = "11 18";
   bitmapName = "~/worldEditor/images/CUR_rotate.png";
};

new GuiCursor(EditorMoveCursor)
{
   hotSpot = "9 13";
   bitmapName = "~/worldEditor/images/CUR_grab.png";
};

new GuiCursor(EditorArrowCursor)
{
   hotSpot = "0 0";
   bitmapName = "~/worldEditor/images/CUR_3darrow.png";
};

new GuiCursor(EditorUpDownCursor)
{
   hotSpot = "5 10";
   bitmapName = "~/worldEditor/images/CUR_3dupdown";
};
new GuiCursor(EditorLeftRightCursor)
{
   hotSpot = "9 5";
   bitmapName = "~/worldEditor/images/CUR_3dleftright";
};

new GuiCursor(EditorDiagRightCursor)
{
   hotSpot = "8 8";
   bitmapName = "~/worldEditor/images/CUR_3ddiagright";
};

new GuiCursor(EditorDiagLeftCursor)
{
   hotSpot = "8 8";
   bitmapName = "~/worldEditor/images/CUR_3ddiagleft";
};

new GuiControl(EmptyControl)
{
   profile = "ToolsGuiButtonProfile";
};


