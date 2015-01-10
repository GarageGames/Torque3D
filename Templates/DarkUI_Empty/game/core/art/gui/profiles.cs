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
   fillColor = "21 21 21 255";
   fillColorHL ="72 72 72 255";
   fillColorSEL = "72 12 0 255";
   fillColorNA = "18 18 18 255";

   // border color
   border = 0;
   borderColor   = "100 100 100"; 
   borderColorHL = "50 50 50 50";
   borderColorNA = "75 75 75"; 

   // font
   fontType = "Arial";
   fontSize = 14;
   fontCharset = ANSI;

   fontColor = "196 196 196 255";
   fontColorHL = "255 255 255 255";
   fontColorNA = "128 128 128 255";
   fontColorSEL= "196 116 108 255";

   // bitmap information
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
   fillColor = "72 72 72";

   // border color
   borderColor   = "196 196 196 255";

   // font
   fontType = "Arial";
   fontSize = 14;
   fontColor = "255 255 255 255";

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
   fillcolor = "21 21 21 255";
   borderColor = "128 128 128";
   border = 1;
   opaque = true;
   border = true;
   category = "Core";
};

if( !isObject( GuiWindowProfile ) )
new GuiControlProfile (GuiWindowProfile)
{
   opaque = false;
   border = "0";
   fillColor = "32 32 32 255";
   fillColorHL = "72 72 72 255";
   fillColorNA = "18 18 18 255";
   fontColor = "196 196 196 255";
   fontColorHL = "255 255 255 255";
   bevelColorHL = "255 255 255";
   bevelColorLL = "0 0 0";
   text = "untitled";
   bitmap = "core/art/gui/images/window";
   textOffset = "8 4";
   hasBitmapArray = true;
   justify = "left";
   category = "Core";
   fontColors[0] = "196 196 196 255";
   fontColors[1] = "255 255 255 255";
   fontColors[2] = "128 128 128 255";
   fontColorNA = "128 128 128 255";
   fillColorSEL = "72 12 0 255";
   borderThickness = "0";
   fontColors[4] = "160 72 64 255";
   fontColors[5] = "196 116 108 255";
   fontColorLink = "160 72 64 255";
   fontColorLinkHL = "196 116 108 255";
   locked = "1";
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
   fontColor = "196 196 196 255";
   category = "Core";
   fillColor = "32 32 32 255";
   fillColorHL = "72 72 72 255";
   fillColorNA = "18 18 18 255";
   fontColors[0] = "196 196 196 255";
   fillColorSEL = "72 12 0 255";
   fontColors[1] = "255 255 255 255";
   fontColors[2] = "128 128 128 255";
   fontColorHL = "255 255 255 255";
   fontColorNA = "128 128 128 255";
   fontColors[4] = "160 72 64 255";
   fontColors[5] = "196 116 108 255";
   fontColorLink = "160 72 64 255";
   fontColorLinkHL = "196 116 108 255";
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
   fontColor = "196 196 196 255";
   autoSizeWidth = true;
   autoSizeHeight = true;   
   category = "Core";
   fontColors[0] = "196 196 196 255";
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
   fontColorLink = "160 72 64 255";
   fontColorLinkHL = "196 116 108 255";
   autoSizeWidth = true;
   autoSizeHeight = true;  
   border = false;
   category = "Core";
   fillColor = "32 32 32 255";
   fillColorHL = "72 72 72 255";
   fillColorNA = "18 18 18 255";
   fillColorSEL = "72 12 0 255";
   fontColors[4] = "160 72 64 255";
   fontColors[5] = "196 116 108 255";
   fontColors[0] = "196 196 196 255";
   fontColors[1] = "255 255 255 255";
   fontColors[2] = "128 128 128 255";
   fontColors[3] = "196 116 108 255";
   fontColor = "196 196 196 255";
   fontColorHL = "255 255 255 255";
   fontColorNA = "128 128 128 255";
   fontColorSEL = "196 116 108 255";
};

if( !isObject( GuiTextArrayProfile ) )
new GuiControlProfile( GuiTextArrayProfile : GuiTextProfile )
{
   fontColor = "196 196 196 255";
   fontColorSEL = "196 116 108 255";
   fontColorHL = "255 255 255 255";
   fontColorNA = "128 128 128 255";
   
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
   bitmap = "core/art/gui/images/textEdit";
   hasBitmapArray = true; 
   border = -2; // fix to display textEdit img
   //borderWidth = "1";  // fix to display textEdit img
   //borderColor = "100 100 100";
   fillColor = "21 21 21 255";
   fillColorHL = "72 72 72 255";
   fontColor = "196 196 196 255";
   fontColorHL = "255 255 255";
   fontColorSEL = "72 12 0 255";
   fontColorNA = "128 128 128 255";
   textOffset = "4 2";
   autoSizeWidth = false;
   autoSizeHeight = true;
   justify = "left";
   tab = true;
   canKeyFocus = true;   
   category = "Core";
   fillColorNA = "18 18 18 0";
   fillColorSEL = "72 72 72 255";
   fontColors[1] = "255 255 255 255";
   fontColors[2] = "128 128 128 255";
   fontColors[3] = "72 12 0 255";
   fontColors[0] = "196 196 196 255";
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
   fontColor = "196 196 196 255";
   justify = "center";
   category = "Core";   
};

if( !isObject( GuiButtonProfile ) )
new GuiControlProfile( GuiButtonProfile )
{
   opaque = true;
   border = "0";
	 
   fontColor = "224 224 224 255";
   fontColorHL = "255 255 255 255";
	 fontColorNA = "64 64 64 255";
	 //fontColorSEL ="0 0 0";
   fixedExtent = 0;
   justify = "center";
   canKeyFocus = false;
	bitmap = "core/art/gui/images/button";
   hasBitmapArray = false;
   category = "Core";
   fillColor = "116 116 116 255";
   fontColors[0] = "224 224 224 255";
   fontColors[2] = "64 64 64 255";
   fillColorHL = "128 128 128 255";
   fillColorNA = "24 24 24 255";
   fillColorSEL = "90 90 90 255";
   borderThickness = "0";
   fontColors[1] = "255 255 255 255";
};

if( !isObject( GuiMenuButtonProfile ) )
new GuiControlProfile( GuiMenuButtonProfile )
{
   opaque = true;
   border = false;
   fontSize = 18;
   fontType = "Arial Bold";
   fontColor = "224 224 224 255";
   fontColorHL = "255 255 255 255";
   fontColorNA = "64 64 64 255";
   //fontColorSEL ="0 0 0";
   fixedExtent = 0;
   justify = "center";
   canKeyFocus = false;
	bitmap = "core/art/gui/images/selector-button";
   hasBitmapArray = false;
   category = "Core";
   fillColor = "116 116 116 255";
   fillColorHL = "128 128 128 255";
   fillColorNA = "24 24 24 255";
   fillColorSEL = "90 90 90 255";
   fontColors[0] = "224 224 224 255";
   fontColors[2] = "64 64 64 255";
   fontColors[1] = "255 255 255 255";
};

if( !isObject( GuiButtonTabProfile ) )
new GuiControlProfile( GuiButtonTabProfile )
{
   opaque = true;
   border = "0";
   fontColor = "224 224 224 255";
   fontColorHL = "255 255 255 255";
   fontColorNA = "64 64 64 255";
   fixedExtent = 0;
   justify = "center";
   canKeyFocus = false;
   bitmap = "core/art/gui/images/buttontab";
   category = "Core";
   fillColor = "72 72 72 255";
   fillColorHL = "116 116 116 255";
   fillColorNA = "90 90 90 255";
   fillColorSEL = "116 116 116 255";
   fontColors[0] = "224 224 224 255";
   borderThickness = "0";
   fontColors[1] = "255 255 255 255";
   fontColors[2] = "64 64 64 255";
   fontColors[4] = "160 72 64 255";
   fontColors[5] = "196 116 108 255";
   fontColorLink = "160 72 64 255";
   fontColorLinkHL = "196 116 108 255";
};

if( !isObject( GuiCheckBoxProfile ) )
new GuiControlProfile( GuiCheckBoxProfile )
{
   opaque = false;
   fillColor = "116 116 116 255";
   border = false;
   borderColor = "100 100 100";
   fontSize = 14;
   fontColor = "196 196 196 255";
   fontColorHL = "255 255 255 255";
	fontColorNA = "128 128 128 255";
   fixedExtent = 1;
   justify = "left";
   bitmap = "core/art/gui/images/checkbox";
   hasBitmapArray = true;
   category = "Core";
   fillColorHL = "128 128 128 255";
   fillColorNA = "24 24 24 255";
   fillColorSEL = "90 90 90 255";
   fontColors[0] = "196 196 196 255";
   fontColors[1] = "255 255 255 255";
   fontColors[2] = "128 128 128 255";
   fontColors[3] = "196 116 108 255";
   fontColors[4] = "160 72 64 255";
   fontColorSEL = "196 116 108 255";
   fontColorLink = "160 72 64 255";
   fontColors[5] = "196 116 108 255";
   fontColorLinkHL = "196 116 108 255";
};

if( !isObject( GuiScrollProfile ) )
new GuiControlProfile( GuiScrollProfile )
{
   opaque = true;
   fillcolor = "21 21 21 255";
   fontColor = "180 180 180 255";
   fontColorHL = "255 255 255 255";
   //borderColor = GuiDefaultProfile.borderColor;
   border = "1";
   bitmap = "core/art/gui/images/scrollBar";
   hasBitmapArray = true;
   category = "Core";
   fontColors[0] = "180 180 180 255";
   fontColors[1] = "255 255 255 255";
};

if( !isObject( GuiOverlayProfile ) )
new GuiControlProfile( GuiOverlayProfile )
{
   opaque = true;
   fillcolor = "0 0 0 100";
   fontColor = "0 0 0";
   fontColorHL = "255 255 255";
	fillColor = "0 0 0 100";
   category = "Core";
   fontColors[0] = "0 0 0 255";
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
   border = "1";
   fontColor = "196 196 196 255";
   fontColorHL = "128 128 128 255";
   fontColorNA = "24 24 24 255";
   justify = "center";
   canKeyFocus = false;
   bitmap = "core/art/gui/images/button";
   category = "Core";
   borderColor = "128 128 128 255";
   fontColors[0] = "196 196 196 255";
   fontColors[1] = "128 128 128 255";
   fontColors[2] = "24 24 24 255";
};

new GuiControlProfile( GuiPopUpMenuDefault )
{
   bitmap = "tools/gui/images/scrollBar";
   category = "Tools";
   mouseOverSelected = "1";
   opaque = "1";
   borderThickness = "0";
   textOffset = "3 3";
   hasBitmapArray = "1";
   profileForChildren = "GuiPopupMenuItemBorder";
   fixedExtent = "1";
   fillColorHL = "72 72 72 255";
   fillColorNA = "18 18 18 255";
   fillColorSEL = "116 116 116 255";
   fontColors[0] = "196 196 196 255";
   fontColors[1] = "255 255 255 255";
   fontColors[2] = "128 128 128 255";
   fontColors[4] = "160 72 64 255";
   fontColors[5] = "196 116 108 255";
   fontColor = "196 196 196 255";
   fontColorHL = "255 255 255 255";
   fontColorNA = "128 128 128 255";
   fontColorLink = "160 72 64 255";
   fontColorLinkHL = "196 116 108 255";
};


if( !isObject( GuiPopUpMenuDefault ) )
new GuiControlProfile( GuiTabBookProfile : GuiDefaultProfile )
{
   opaque = "0";
   mouseOverSelected = "0";
   textOffset = "0 -3";
   border = 0;
   borderThickness = "1";
   fixedExtent = 1;
   bitmap = "core/art/gui/images/tab";
   hasBitmapArray = "0";
   profileForChildren = GuiPopupMenuItemBorder;
   fillColor = "21 21 21 255";
   fillColorHL = "100 100 100 255";
   fillColorSEL = "72 12 0 255";
   fontColorHL = "255 255 255 255";
   fontColorSEL = "255 255 255 255";
   borderColor = "100 100 100";
   category = "Core";
   fillColorNA = "150 150 150 255";
   fontColors[0] = "240 240 240 255";
   fontColors[1] = "255 255 255 255";
   fontColors[2] = "128 128 128 255";
   fontColors[3] = "255 255 255 255";
   fontColors[4] = "255 0 255 255";
   fontColors[5] = "255 0 255 255";
   fontColor = "240 240 240 255";
   fontColorNA = "128 128 128 255";
   fontColorLink = "255 0 255 255";
   fontColorLinkHL = "255 0 255 255";
   tab = "1";
   canKeyFocus = "1";
   justify = "Center";
   tabRotation = "Horizontal";
   tabHeight = "24";
   tabPosition = "Top";
   tabWidth = "64";
};

if( !isObject( GuiPopUpMenuProfile ) )
new GuiControlProfile( GuiPopUpMenuProfile : GuiPopUpMenuDefault )
{
   textOffset         = "6 4";
   bitmap             = "core/art/gui/images/dropDown";
   hasBitmapArray     = true;
   border             = "0";
   profileForChildren = GuiPopUpMenuDefault;
   category = "Core";
   fontColors[0] = "196 196 196 255";
   fontColors[1] = "255 255 255 255";
   fontColor = "196 196 196 255";
   fontColorHL = "255 255 255 255";
   fontColors[2] = "128 128 128 255";
   fontColorNA = "128 128 128 255";
   fontCharset = "ANSI";
   fontColors[3] = "255 255 255 255";
   fontColorSEL = "255 255 255 255";
   fontColors[4] = "160 72 64 255";
   fontColors[5] = "196 116 108 255";
   fontColorLink = "160 72 64 255";
   fontColorLinkHL = "196 116 108 255";
   fillColor = "21 21 21 255";
   fillColorHL = "72 72 72 255";
   fillColorNA = "18 18 18 255";
   fillColorSEL = "116 116 116 255";
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
   bitmap = "core/art/gui/images/tab";
   opaque = false;
   fillColor = "21 21 21 255";
   category = "Core";
   fontColors[3] = "255 255 255 255";
   fontColorSEL = "255 255 255 255";
   fontColors[0] = "196 196 196 255";
   fontColor = "196 196 196 255";
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
   fontColor = "196 196 196";
   autoSizeWidth = true;
   autoSizeHeight = true;   
   textOffset = "2 2";
   opaque = true;   
   fillColor = "21 21 21 255";
   border = "1";
   borderThickness = 1;
   borderColor = "128 128 128";
   category = "Core";
   fontColors[0] = "196 196 196 255";
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
   // +++ changed Dark UI 1.3
   fontSize = 14;
   fillColor = "32 32 32 255";
   fontColor = "196 196 196 255";
   fontColorHL = "255 255 255 255";
   fixedExtent = 1;
   bitmap = "tools/gui/images/radioButton";
   hasBitmapArray = true;
   category = "Tools";
   modal = "1";
   fontColors[0] = "196 196 196 255";
   fontColors[1] = "255 255 255 255";
};
