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

// Material Editor Written by Dave Calabrese of Gaslight Studios

singleton GuiControlProfile (GuiMatEdSliderProfile)
{
   bitmap = "./matEdSlider";
   category = "Editor";
};

singleton GuiControlProfile (GuiMatEdRightJustifyProfile)
{
   // font
   fontType = "Arial";
   fontSize = 14;
   fontCharset = ANSI;

   fontColor = "0 0 0";
   
   justify = "right";
   category = "Editor";
};

singleton GuiControlProfile(GuiMatEdPopUpMenuProfile)
{
   opaque = false;
   mouseOverSelected = true;
   textOffset = "3 3";
   border = 1;
   /*borderThickness = 1;*/
   fixedExtent = true;
   //bitmap = "./images/scrollbar";
   bitmap = "tools/editorClasses/gui/images/scroll";
   hasBitmapArray = true;
   profileForChildren = GuiControlListPopupProfile;
   fillColor = "255 0 0 255";
   fontColor = "255 255 255 255";
   fillColorHL = "50 50 50";
   fontColorHL = "220 220 220";
   borderColor = "100 100 108";
   category = "Editor";
};

singleton GuiControlProfile (MatEdCenteredTextProfile)
{
   fontColor = "0 0 0";
   justify = "center";
   category = "Editor";
};
