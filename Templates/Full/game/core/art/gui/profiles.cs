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

// If we got back no prefs path modification
if( $Gui::fontCacheDirectory $= "")
{
   $Gui::fontCacheDirectory = expandFilename( "~/fonts" );
}

// ----------------------------------------------------------------------------
// GuiDefaultProfile is a special profile that all other profiles inherit
// defaults from. It must exist.
// ----------------------------------------------------------------------------

if( !isObject( GuiDefaultProfile ) )
new GuiControlProfile (GuiDefaultProfile)
{
   tab = false;
   canKeyFocus = false;
   hasBitmapArray = false;
   mouseOverSelected = false;

   // fill color
   opaque = false;
   fillColor = "242 241 240";
   fillColorHL ="228 228 235";
   fillColorSEL = "98 100 137";
   fillColorNA = "255 255 255 ";

   // border color
   border = 0;
   borderColor   = "100 100 100"; 
   borderColorHL = "50 50 50 50";
   borderColorNA = "75 75 75"; 

   // font
   fontType = "Arial";
   fontSize = 14;
   fontCharset = ANSI;

   fontColor = "0 0 0";
   fontColorHL = "0 0 0";
   fontColorNA = "0 0 0";
   fontColorSEL= "255 255 255";

   // bitmap information
   bitmap = "";
   bitmapBase = "";
   textOffset = "0 0";

   // used by guiTextControl
   modal = true;
   justify = "left";
   autoSizeWidth = false;
   autoSizeHeight = false;
   returnTab = false;
   numbersOnly = false;
   cursorColor = "0 0 0 255";

   // sounds
   //soundButtonDown = "";
   //soundButtonOver = "";
};

if( !isObject( GuiSolidDefaultProfile ) )
new GuiControlProfile (GuiSolidDefaultProfile)
{
   opaque = true;
   border = true;
   category = "Core";
};

if( !isObject( GuiTransparentProfile ) )
new GuiControlProfile (GuiTransparentProfile)
{
   opaque = false;
   border = false;
   category = "Core";
};

if( !isObject( GuiGroupBorderProfile ) )
new GuiControlProfile( GuiGroupBorderProfile )
{
   border = false;
   opaque = false;
   hasBitmapArray = true;
   bitmap = "./images/group-border";
   category = "Core";
};

if( !isObject( GuiTabBorderProfile ) )
new GuiControlProfile( GuiTabBorderProfile )
{
   border = false;
   opaque = false;
   hasBitmapArray = true;
   bitmap = "./images/tab-border";
   category = "Core";
};

if( !isObject( GuiToolTipProfile ) )
new GuiControlProfile (GuiToolTipProfile)
{
   // fill color
   fillColor = "239 237 222";

   // border color
   borderColor   = "138 134 122";

   // font
   fontType = "Arial";
   fontSize = 14;
   fontColor = "0 0 0";

   category = "Core";
};

if( !isObject( GuiModelessDialogProfile ) )
new GuiControlProfile( GuiModelessDialogProfile )
{
   modal = false;
   category = "Core";
};

if( !isObject( GuiFrameSetProfile ) )
new GuiControlProfile (GuiFrameSetProfile)
{
   fillcolor = "255 255 255";
   borderColor = "246 245 244";
   border = 1;
   opaque = true;
   border = true;
   category = "Core";
};

if( !isObject( GuiWindowProfile ) )
new GuiControlProfile (GuiWindowProfile)
{
   opaque = false;
   border = 2;
   fillColor = "242 241 240";
   fillColorHL = "221 221 221";
   fillColorNA = "200 200 200";
   fontColor = "50 50 50";
   fontColorHL = "0 0 0";
   bevelColorHL = "255 255 255";
   bevelColorLL = "0 0 0";
   text = "untitled";
   bitmap = "./images/window";
   textOffset = "8 4";
   hasBitmapArray = true;
   justify = "left";
   category = "Core";
};

if( !isObject( GuiInputCtrlProfile ) )
new GuiControlProfile( GuiInputCtrlProfile )
{
   tab = true;
   canKeyFocus = true;
   category = "Core";
};

if( !isObject( GuiTextProfile ) )
new GuiControlProfile (GuiTextProfile)
{
   justify = "left";
   fontColor = "20 20 20";
   category = "Core";
};

if( !isObject( GuiTextRightProfile ) )
new GuiControlProfile (GuiTextRightProfile : GuiTextProfile)
{
   justify = "right";
   category = "Core";
};

if( !isObject( GuiAutoSizeTextProfile ) )
new GuiControlProfile (GuiAutoSizeTextProfile)
{
   fontColor = "0 0 0";
   autoSizeWidth = true;
   autoSizeHeight = true;   
   category = "Core";
};

if( !isObject( GuiMediumTextProfile ) )
new GuiControlProfile( GuiMediumTextProfile : GuiTextProfile )
{
   fontSize = 24;
   category = "Core";
};

if( !isObject( GuiBigTextProfile ) )
new GuiControlProfile( GuiBigTextProfile : GuiTextProfile )
{
   fontSize = 36;
   category = "Core";
};

if( !isObject( GuiMLTextProfile ) )
new GuiControlProfile( GuiMLTextProfile )
{
   fontColorLink = "100 100 100";
   fontColorLinkHL = "255 255 255";
   autoSizeWidth = true;
   autoSizeHeight = true;  
   border = false;
   category = "Core";
};

if( !isObject( GuiTextArrayProfile ) )
new GuiControlProfile( GuiTextArrayProfile : GuiTextProfile )
{
   fontColor = "50 50 50";
   fontColorHL = " 0 0 0";
   fontColorSEL = "0 0 0";
   fillColor ="200 200 200";
   fillColorHL = "228 228 235";
   fillColorSEL = "200 200 200";
   border = false;
   category = "Core";
};

if( !isObject( GuiTextEditProfile ) )
new GuiControlProfile( GuiTextEditProfile )
{
   opaque = true;
   bitmap = "./images/textEdit";
   hasBitmapArray = true; 
   border = -2; // fix to display textEdit img
   //borderWidth = "1";  // fix to display textEdit img
   //borderColor = "100 100 100";
   fillColor = "242 241 240 0";
   fillColorHL = "255 255 255";
   fontColor = "0 0 0";
   fontColorHL = "255 255 255";
   fontColorSEL = "98 100 137";
   fontColorNA = "200 200 200";
   textOffset = "4 2";
   autoSizeWidth = false;
   autoSizeHeight = true;
   justify = "left";
   tab = true;
   canKeyFocus = true;   
   category = "Core";
};

if( !isObject( GuiProgressProfile ) )
new GuiControlProfile( GuiProgressProfile )
{
   opaque = false;
   fillColor = "0 162 255 200";
   border = true;
   borderColor   = "50 50 50 200";
   category = "Core";
};

if( !isObject( GuiProgressBitmapProfile ) )
new GuiControlProfile( GuiProgressBitmapProfile )
{
   border = false;
   hasBitmapArray = true;
   bitmap = "./images/loadingbar";
   category = "Core";
};

if( !isObject( GuiProgressTextProfile ) )
new GuiControlProfile( GuiProgressTextProfile )
{
   fontSize = "14";
	fontType = "Arial";
   fontColor = "0 0 0";
   justify = "center";
   category = "Core";   
};

if( !isObject( GuiButtonProfile ) )
new GuiControlProfile( GuiButtonProfile )
{
   opaque = true;
   border = true;
	 
   fontColor = "50 50 50";
   fontColorHL = "0 0 0";
	 fontColorNA = "200 200 200";
	 //fontColorSEL ="0 0 0";
   fixedExtent = false;
   justify = "center";
   canKeyFocus = false;
	bitmap = "./images/button";
   hasBitmapArray = false;
   category = "Core";
};

if( !isObject( GuiMenuButtonProfile ) )
new GuiControlProfile( GuiMenuButtonProfile )
{
   opaque = true;
   border = false;
   fontSize = 18;
   fontType = "Arial Bold";
   fontColor = "50 50 50";
   fontColorHL = "0 0 0";
   fontColorNA = "200 200 200";
   //fontColorSEL ="0 0 0";
   fixedExtent = false;
   justify = "center";
   canKeyFocus = false;
	bitmap = "./images/selector-button";
   hasBitmapArray = false;
   category = "Core";
};

if( !isObject( GuiButtonTabProfile ) )
new GuiControlProfile( GuiButtonTabProfile )
{
   opaque = true;
   border = true;
   fontColor = "50 50 50";
   fontColorHL = "0 0 0";
   fontColorNA = "0 0 0";
   fixedExtent = false;
   justify = "center";
   canKeyFocus = false;
   bitmap = "./images/buttontab";
   category = "Core";
};

if( !isObject( GuiCheckBoxProfile ) )
new GuiControlProfile( GuiCheckBoxProfile )
{
   opaque = false;
   fillColor = "232 232 232";
   border = false;
   borderColor = "100 100 100";
   fontSize = 14;
   fontColor = "20 20 20";
   fontColorHL = "80 80 80";
	fontColorNA = "200 200 200";
   fixedExtent = true;
   justify = "left";
   bitmap = "./images/checkbox";
   hasBitmapArray = true;
   category = "Core";
};

if( !isObject( GuiScrollProfile ) )
new GuiControlProfile( GuiScrollProfile )
{
   opaque = true;
   fillcolor = "255 255 255";
   fontColor = "0 0 0";
   fontColorHL = "150 150 150";
   //borderColor = GuiDefaultProfile.borderColor;
   border = true;
   bitmap = "./images/scrollBar";
   hasBitmapArray = true;
   category = "Core";
};

if( !isObject( GuiOverlayProfile ) )
new GuiControlProfile( GuiOverlayProfile )
{
   opaque = true;
   fillcolor = "255 255 255";
   fontColor = "0 0 0";
   fontColorHL = "255 255 255";
	fillColor = "0 0 0 100";
   category = "Core";
};

if( !isObject( GuiSliderProfile ) )
new GuiControlProfile( GuiSliderProfile )
{
   bitmap = "./images/slider";
   category = "Core";
};

if( !isObject( GuiSliderBoxProfile ) )
new GuiControlProfile( GuiSliderBoxProfile )
{
   bitmap = "./images/slider-w-box";
   category = "Core";
};

// ----------------------------------------------------------------------------
// TODO: Revisit Popupmenu
// ----------------------------------------------------------------------------

if( !isObject( GuiPopupMenuItemBorder ) )
new GuiControlProfile( GuiPopupMenuItemBorder : GuiButtonProfile )
{
   opaque = true;
   border = true;
   fontColor = "0 0 0";
   fontColorHL = "0 0 0";
   fontColorNA = "255 255 255";
   fixedExtent = false;
   justify = "center";
   canKeyFocus = false;
   bitmap = "./images/button";
   category = "Core";
};

if( !isObject( GuiPopUpMenuDefault ) )
new GuiControlProfile( GuiPopUpMenuDefault : GuiDefaultProfile )
{
   opaque = true;
   mouseOverSelected = true;
   textOffset = "3 3";
   border = 0;
   borderThickness = 0;
   fixedExtent = true;
   bitmap = "./images/scrollbar";
   hasBitmapArray = true;
   profileForChildren = GuiPopupMenuItemBorder;
   fillColor = "242 241 240 ";//"255 255 255";//100
   fillColorHL = "228 228 235 ";//"204 203 202";
   fillColorSEL = "98 100 137 ";//"204 203 202";
   // font color is black
   fontColorHL = "0 0 0 ";//"0 0 0";
   fontColorSEL = "255 255 255";//"0 0 0";
   borderColor = "100 100 100";
   category = "Core";
};

if( !isObject( GuiPopUpMenuProfile ) )
new GuiControlProfile( GuiPopUpMenuProfile : GuiPopUpMenuDefault )
{
   textOffset         = "6 4";
   bitmap             = "./images/dropDown";
   hasBitmapArray     = true;
   border             = 1;
   profileForChildren = GuiPopUpMenuDefault;
   category = "Core";
};

if( !isObject( GuiTabBookProfile ) )
new GuiControlProfile( GuiTabBookProfile )
{
   fillColorHL = "100 100 100";
   fillColorNA = "150 150 150";
   fontColor = "30 30 30";
   fontColorHL = "0 0 0";
   fontColorNA = "50 50 50";
   fontType = "Arial";
   fontSize = 14;
   justify = "center";
   bitmap = "./images/tab";
   tabWidth = 64;
   tabHeight = 24;
   tabPosition = "Top";
   tabRotation = "Horizontal";
   textOffset = "0 -3";
   tab = true;
   cankeyfocus = true;
   category = "Core";
};

if( !isObject( GuiTabPageProfile ) )
new GuiControlProfile( GuiTabPageProfile : GuiDefaultProfile )
{
   fontType = "Arial";
   fontSize = 10;
   justify = "center";
   bitmap = "./images/tab";
   opaque = false;
   fillColor = "240 239 238";
   category = "Core";
};

if( !isObject( GuiConsoleProfile ) )
new GuiControlProfile( GuiConsoleProfile )
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

if( !isObject( GuiConsoleTextProfile ) )
new GuiControlProfile( GuiConsoleTextProfile )
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

$ConsoleDefaultFillColor = "0 0 0 175";

if( !isObject( ConsoleScrollProfile ) )
new GuiControlProfile( ConsoleScrollProfile : GuiScrollProfile )
{
	opaque = true;
	fillColor = $ConsoleDefaultFillColor;
	border = 1;
	//borderThickness = 0;
	borderColor = "0 0 0";
   category = "Core";
};

if( !isObject( ConsoleTextEditProfile ) )
new GuiControlProfile( ConsoleTextEditProfile : GuiTextEditProfile )
{
   fillColor = "242 241 240 255";
   fillColorHL = "255 255 255";   
   category = "Core";
};

//-----------------------------------------------------------------------------
// Center and bottom print
//-----------------------------------------------------------------------------

if( !isObject( CenterPrintProfile ) )
new GuiControlProfile ( CenterPrintProfile )
{
   opaque = false;
   fillColor = "128 128 128";
   fontColor = "0 255 0";
   border = true;
   borderColor = "0 255 0";
   category = "Core";
};

if( !isObject( CenterPrintTextProfile ) )
new GuiControlProfile ( CenterPrintTextProfile )
{
   opaque = false;
   fontType = "Arial";
   fontSize = 12;
   fontColor = "0 255 0";
   category = "Core";
};

// ----------------------------------------------------------------------------
// Radio button control
// ----------------------------------------------------------------------------

if( !isObject( GuiRadioProfile ) )
new GuiControlProfile( GuiRadioProfile )
{
   fontSize = 14;
   fillColor = "232 232 232";
   fontColor = "20 20 20";
   fontColorHL = "80 80 80";
   fixedExtent = true;
   bitmap = "./images/radioButton";
   hasBitmapArray = true;
   category = "Core";
};