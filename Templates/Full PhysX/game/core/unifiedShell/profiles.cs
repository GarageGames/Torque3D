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
// Gui Profiles for the unified shell
//------------------------------------------------------------------------------

singleton GuiGameListMenuProfile(DefaultListMenuProfile)
{
   fontType = "Arial Bold";
   fontSize = 20;
   fontColor = "120 120 120";
   fontColorSEL = "16 16 16";
   fontColorNA = "200 200 200";
   fontColorHL = "100 100 120";
   HitAreaUpperLeft = "16 20";
   HitAreaLowerRight = "503 74";
   IconOffset = "40 0";
   TextOffset = "100 0";
   RowSize = "525 93";
   bitmap = "./images/listMenuArray";
   canKeyFocus = true;
};

singleton GuiGameListOptionsProfile(DefaultOptionsMenuProfile)
{
   fontType = "Arial Bold";
   fontSize = 20;
   fontColor = "120 120 120";
   fontColorSEL = "16 16 16";
   fontColorNA = "200 200 200";
   fontColorHL = "100 100 120";
   HitAreaUpperLeft = "16 20";
   HitAreaLowerRight = "503 74";
   IconOffset = "40 0";
   TextOffset = "90 0";
   RowSize = "525 93";
   ColumnSplit = "220";
   RightPad = "20";
   bitmap = "./images/listMenuArray";
   canKeyFocus = true;
};

singleton GuiControlProfile(GamepadDefaultProfile)
{
   border = 0;
};

singleton GuiControlProfile(GamepadButtonTextLeft)
{
   fontType = "Arial Bold";
   fontSize = 20;
   fontColor = "40 40 40";
   justify = "left";
};

singleton GuiControlProfile(GamepadButtonTextRight : GamepadButtonTextLeft)
{
   justify = "right";
};
