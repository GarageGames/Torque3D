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

if(!isObject(GuiConsoleProfile))
new GuiControlProfile(GuiConsoleProfile)
{
   fontType = ($platform $= "macos") ? "Monaco" : "Lucida Console";
   fontSize = ($platform $= "macos") ? 13 : 12;
   fontColor = "255 255 255";
   fontColorHL = "0 255 255";
   fontColorNA = "255 0 0";
   fontColors[6] = "100 100 100";
   fontColors[7] = "100 100 0";
   fontColors[8] = "0 0 100";
   fontColors[9] = "0 100 0";
   category = "Core";
};

if(!isObject(GuiConsoleTextProfile))
new GuiControlProfile(GuiConsoleTextProfile)
{   
   fontColor = "0 0 0";
   autoSizeWidth = true;
   autoSizeHeight = true;   
   textOffset = "2 2";
   opaque = true;   
   fillColor = "255 255 255";
   border = true;
   borderThickness = 1;
   borderColor = "0 0 0";
   category = "Core";
};

if(!isObject(ConsoleScrollProfile))
new GuiControlProfile(ConsoleScrollProfile : GuiScrollProfile)
{
	opaque = true;
	fillColor = "0 0 0 175";
	border = 1;
	//borderThickness = 0;
	borderColor = "0 0 0";
   category = "Core";
};

if(!isObject(ConsoleTextEditProfile))
new GuiControlProfile(ConsoleTextEditProfile : GuiTextEditProfile)
{
   fillColor = "242 241 240 255";
   fillColorHL = "255 255 255";   
   category = "Core";
};
