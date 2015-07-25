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

function execEditorProfilesCS()
{
   exec("./profiles.ed.cs");   
}

$Gui::clipboardFile = expandFilename("./clipboard.gui");


if( !isObject( ToolsGuiDefaultProfile ) )
new GuiControlProfile (ToolsGuiDefaultProfile)
{
   tab = true;
   canKeyFocus = false;
   hasBitmapArray = false;
   mouseOverSelected = false;

   // fill color
   opaque = false;
   fillColor = "32 32 32 255";
   fillColorHL ="72 72 72 255";
   fillColorSEL = "18 18 18 255";
   fillColorNA = "72 12 0  255";

   // border color
   border = 0;
   borderColor   = "21 21 21 255"; 
   borderColorHL = "72 72 72 255";
   borderColorNA = "72 72 72 255"; 

   // font
   fontType = "Arial";
   fontSize = 14;
   fontCharset = ANSI;

   fontColor = "196 196 196 255";
   fontColorHL = "255 255 255 255";
   fontColorNA = "128 128 128 255";
   fontColorSEL= "196 116 108 255";

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

if( !isObject( ToolsGuiEditorProfile ) )
new GuiControlProfile (ToolsGuiEditorProfile)
{
   // +++ new for Dark UI
   tab = true;
   canKeyFocus = false;
   hasBitmapArray = false;
   mouseOverSelected = false;
   
   // don't go transparent when it's a background
   opaque = true;
   fillColor = "32 32 32 255";
   fillColorHL ="72 72 72 255";
   fillColorSEL = "18 18 18 255";
   fillColorNA = "72 12 0  255";
   border = 0;
   borderColor   = "21 21 21 255"; 
   borderColorHL = "72 72 72 255";
   borderColorNA = "72 72 72 255"; 
   fontType = "Arial";
   fontSize = 14;
   fontCharset = ANSI;
   fontColor = "196 196 196 255";
   fontColorHL = "255 255 255 255";
   fontColorNA = "128 128 128 255";
   fontColorSEL= "196 116 108 255";
   bitmap = "";
   bitmapBase = "";
   textOffset = "0 0";
   modal = true;
   justify = "left";
   autoSizeWidth = false;
   autoSizeHeight = false;
   returnTab = false;
   numbersOnly = false;
   cursorColor = "0 0 0 255";
};

if( !isObject( ToolsGuiSolidDefaultProfile ) )
new GuiControlProfile (ToolsGuiSolidDefaultProfile)
{
   opaque = true;
   border = false;
   category = "Tools";
};

if( !isObject( ToolsGuiTransparentProfile ) )
new GuiControlProfile (ToolsGuiTransparentProfile)
{
   opaque = false;
   border = false;
   category = "Tools";
};

if( !isObject( ToolsGuiGroupBorderProfile ) )
new GuiControlProfile( ToolsGuiGroupBorderProfile )
{
   border = false;
   opaque = false;
   hasBitmapArray = true;
   bitmap = "./images/group-border";
   category = "Tools";
};

if( !isObject( ToolsGuiTabBorderProfile ) )
new GuiControlProfile( ToolsGuiTabBorderProfile )
{
   border = false;
   opaque = false;
   hasBitmapArray = true;
   bitmap = "./images/tab-border";
   category = "Tools";
};

if( !isObject( ToolsGuiToolTipProfile ) )
new GuiControlProfile (ToolsGuiToolTipProfile)
{
   // fill color
   fillColor = "72 72 72";

   // border color
   borderColor   = "196 196 196 255";

   // font
   fontType = "Arial";
   fontSize = 14;
   fontColor = "255 255 255 255";

   category = "Tools";
};

if( !isObject( ToolsGuiModelessDialogProfile ) )
new GuiControlProfile( ToolsGuiModelessDialogProfile )
{
   modal = false;
   category = "Tools";
};

if( !isObject( ToolsGuiFrameSetProfile ) )
new GuiControlProfile (ToolsGuiFrameSetProfile)
{
   fillcolor = "32 32 32 255";
   borderColor = "72 72 72";
   opaque = "1"; // +++ don't go transparent here!
   border = false;
   category = "Tools";
};

if( !isObject( ToolsGuiWindowProfile ) )
new GuiControlProfile (ToolsGuiWindowProfile)
{
   opaque = false;
   border = 2;
   fillColor = "32 32 32 255";
   fillColorHL = "72 72 72 255";
   fillColorNA = "18 18 18 255";
   fontColor = "196 196 196 255";
   fontColorHL = "255 255 255 255";
   bevelColorHL = "255 255 255";
   bevelColorLL = "0 0 0";
   text = "untitled";
   bitmap = "tools/gui/images/window";
   textOffset = "8 4";
   hasBitmapArray = true;
   justify = "left";
   category = "Tools";
   fontColors[0] = "196 196 196 255";
   fillColorSEL = "72 12 0 255";
   fontColors[1] = "255 255 255 255";
   fontColors[2] = "18 18 18 255";
   fontColors[3] = "196 116 108 255";
   fontColors[4] = "160 72 64 255";
   fontColors[5] = "196 116 108 255";
   fontColorNA = "18 18 18 255";
   fontColorSEL = "196 116 108 255";
   fontColorLink = "160 72 64 255";
   fontColorLinkHL = "196 116 108 255";
};

if( !isObject( ToolsGuiToolbarWindowProfile ) )
new GuiControlProfile(ToolsGuiToolbarWindowProfile : ToolsGuiWindowProfile)
{
      bitmap = "./images/toolbar-window";
      text = "";
      category = "Tools";
}; 

if( !isObject( ToolsGuiWindowCollapseProfile ) )
new GuiControlProfile (ToolsGuiWindowCollapseProfile : ToolsGuiWindowProfile)
{
   category = "Tools";
};

if( !isObject( ToolsGuiTextProfile ) )
new GuiControlProfile (ToolsGuiTextProfile)
{
   justify = "left";
   fontColor = "196 196 196 255";
   category = "Tools";
   fontColors[0] = "196 196 196 255";
   fontColors[1] = "255 255 255 255";
   fontColors[2] = "128 128 128 255";
   fontColors[3] = "196 116 108 255";
   fontColors[4] = "160 72 64 255";
   fontColors[5] = "196 116 108 255";
   fontColorHL = "255 255 255 255";
   fontColorNA = "128 128 128 255";
   fontColorSEL = "196 116 108 255";
   fontColorLink = "160 72 64 255";
   fontColorLinkHL = "196 116 108 255";
   fillColor = "32 32 32 255";
   fillColorHL = "72 72 72 255";
   fillColorNA = "18 18 18 255";
   fillColorSEL = "72 12 0 255";
};

if( !isObject( ToolsGuiTextBoldCenterProfile ) )
new GuiControlProfile (ToolsGuiTextBoldCenterProfile : ToolsGuiTextProfile)
{
   // fontColor = "50 50 50"; // +++ already defined in ToolsGuiTextProfile
   fontType = "Arial Bold";
   fontSize = 16;
   justify = "center";
   category = "Tools";
};

if( !isObject( ToolsGuiTextRightProfile ) )
new GuiControlProfile (ToolsGuiTextRightProfile : ToolsGuiTextProfile)
{
   justify = "right";
   category = "Tools";
};

if( !isObject( ToolsGuiTextCenterProfile ) )
new GuiControlProfile (ToolsGuiTextCenterProfile : ToolsGuiTextProfile)
{
   justify = "center";
   category = "Tools";
};

if( !isObject( ToolsGuiInspectorTitleTextProfile ) )
new GuiControlProfile (ToolsGuiInspectorTitleTextProfile)
{
   fontColor = "100 100 100";
   category = "Tools";
};

if( !isObject( ToolsGuiAutoSizeTextProfile ) )
new GuiControlProfile (ToolsGuiAutoSizeTextProfile)
{
   fontColor = "0 0 0";
   autoSizeWidth = true;
   autoSizeHeight = true;   
   category = "Tools";
   fontColors[0] = "0 0 0 255";
};

if( !isObject( ToolsGuiMLTextProfile ) )
new GuiControlProfile( ToolsGuiMLTextProfile )
{
   fontColorLink = "160 72 64 255";
   fontColorLinkHL = "196 116 108 255";
   autoSizeWidth = true;
   autoSizeHeight = true;  
   border = false;
   category = "Tools";
   fillColor = "32 32 32 255";
   fontColors[0] = "196 196 196 255";
   fontColors[1] = "255 255 255 255";
   fontColors[2] = "128 128 128 255";
   fontColors[3] = "196 116 108 255";
   fontColors[4] = "160 72 64 255";
   fontColors[5] = "196 116 108 255";
   fontColor = "196 196 196 255";
   fontColorHL = "255 255 255 255";
   fontColorNA = "128 128 128 255";
   fontColorSEL = "196 116 108 255";
   fillColorHL = "72 72 72 255";
   fillColorNA = "18 18 18 255";
   fillColorSEL = "72 12 0 255";
};

if( !isObject( ToolsGuiTextArrayProfile ) )
new GuiControlProfile( ToolsGuiTextArrayProfile : ToolsGuiTextProfile )
{
   fontColor = "50 50 50";
   fontColorHL = " 0 0 0";
   fontColorSEL = "0 0 0";
   fillColor ="200 200 200";
   fillColorHL = "228 228 235";
   fillColorSEL = "200 200 200";
   border = false;
   category = "Tools";
};

if( !isObject( ToolsGuiTextListProfile ) )
new GuiControlProfile( ToolsGuiTextListProfile : ToolsGuiTextProfile ) 
{
   tab = true;
   canKeyFocus = true;
   category = "Tools";
};

if( !isObject( ToolsGuiTextEditProfile ) )
new GuiControlProfile( ToolsGuiTextEditProfile )
{
   opaque = true;
   bitmap = "tools/gui/images/textEditFrame";
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
   category = "Tools";
   fontColors[1] = "255 255 255 255";
   fontColors[2] = "128 128 128 255";
   fontColors[3] = "72 12 0 255";
   fillColorNA = "18 18 18 0";
   fillColorSEL = "72 72 72 255";
   fontColors[0] = "196 196 196 255";
};

if( !isObject( ToolsGuiNumericTextEditProfile ) )
new GuiControlProfile( ToolsGuiNumericTextEditProfile : ToolsGuiTextEditProfile )
{
   numbersOnly = true;
   category = "Tools";
};

if( !isObject( ToolsGuiNumericDropSliderTextProfile ) )
new GuiControlProfile( ToolsGuiNumericDropSliderTextProfile : ToolsGuiTextEditProfile )
{
   bitmap = "./images/textEditSliderBox";
   category = "Tools";
};

if( !isObject( ToolsGuiRLProgressBitmapProfile ) )
new GuiControlProfile( ToolsGuiRLProgressBitmapProfile )
{
   border = false;
   hasBitmapArray = true;
   bitmap = "./images/rl-loadingbar";
   category = "Tools";
};

if( !isObject( ToolsGuiProgressTextProfile ) )
new GuiControlProfile( ToolsGuiProgressTextProfile )
{
   fontSize = "14";
	fontType = "Arial";
   fontColor = "196 196 196 255";
   justify = "center";
   category = "Tools";   
};

if( !isObject( ToolsGuiButtonProfile ) )
new GuiControlProfile( ToolsGuiButtonProfile )
{
   opaque = "0";
   border = "1";
   fontColor = "224 224 224 255";
   fontColorHL = "255 255 255 255";
   fontColorNA = "200 200 200";
   fixedExtent = 0;
   justify = "center";
   canKeyFocus = "1";
	bitmap = "tools/gui/images/button";
   hasBitmapArray = false;
   category = "Tools";
   fillColor = "21 21 21 255";
   fillColorHL = "128 128 128 255";
   fillColorNA = "24 24 24 255";
   fillColorSEL = "90 90 90 255";
   fontColors[0] = "224 224 224 255";
   fontColors[2] = "200 200 200 255";
   fontColors[1] = "255 255 255 255";
   fontColors[4] = "160 72 64 255";
   fontColorLink = "160 72 64 255";
   fontColors[5] = "196 116 108 255";
   fontColorLinkHL = "196 116 108 255";
};

if( !isObject( ToolsGuiThumbHighlightButtonProfile ) )
new GuiControlProfile( ToolsGuiThumbHighlightButtonProfile : ToolsGuiButtonProfile )
{
   bitmap = "./images/thumbHightlightButton";
   category = "Tools";
};

if( !isObject( ToolsGuiIconButtonProfile ) )
new GuiControlProfile( ToolsGuiIconButtonProfile )
{
   opaque = true;
   border = true;
   fontColor = "196 196 196";
   fontColorHL = "255 255 255";
   fontColorNA = "128 128 128";
   fixedExtent = false;
   justify = "center";
   canKeyFocus = false;
	bitmap = "./images/iconbutton";
   hasBitmapArray = true;
   category = "Tools";
};

if( !isObject( ToolsGuiIconButtonSmallProfile ) )
new GuiControlProfile( ToolsGuiIconButtonSmallProfile : ToolsGuiIconButtonProfile )
{
   bitmap = "./images/iconbuttonsmall";
   category = "Tools";
};

if( !isObject( ToolsGuiEditorTabPage ) )
new GuiControlProfile(ToolsGuiEditorTabPage)
{
   opaque = true;
   border = false;
   fontColor = "196 196 196 255";
   fontColorHL = "255 255 255 255";
   fixedExtent = 0;
   justify = "center";
   canKeyFocus = false;
   bitmap = "tools/gui/images/tab";
   hasBitmapArray = true;
   category = "Tools";
   fontColors[1] = "255 255 255 255";
};

if( !isObject( ToolsGuiCheckBoxProfile ) )
new GuiControlProfile( ToolsGuiCheckBoxProfile )
{
   opaque = false;
   fillColor = "21 21 21 255";
   border = false;
   borderColor = "100 100 100";
   fontSize = 14;
   fontColor = "196 196 196 255";
   fontColorHL = "196 196 196 255";
	fontColorNA = "128 128 128 255";
   fixedExtent = 1;
   justify = "left";
   bitmap = "tools/gui/images/checkbox";
   hasBitmapArray = true;
   category = "Tools";
   fontColors[0] = "196 196 196 255";
   fontColors[1] = "196 196 196 255";
   fontColors[2] = "128 128 128 255";
};

if( !isObject( ToolsGuiCheckBoxListProfile ) )
new GuiControlProfile( ToolsGuiCheckBoxListProfile : ToolsGuiCheckBoxProfile)
{
   bitmap = "tools/gui/images/checkbox-list";
   category = "Tools";
   fillColor = "21 21 21 255";
};

if( !isObject( ToolsGuiCheckBoxListFlipedProfile ) )
new GuiControlProfile( ToolsGuiCheckBoxListFlipedProfile : ToolsGuiCheckBoxProfile)
{
   bitmap = "tools/gui/images/checkbox-list_fliped";
   category = "Tools";
   fillColor = "21 21 21 255";
};

if( !isObject( ToolsGuiInspectorCheckBoxTitleProfile ) )
new GuiControlProfile( ToolsGuiInspectorCheckBoxTitleProfile : ToolsGuiCheckBoxProfile ){
   fontColor = "196 196 196 255";
   category = "Tools";
   fillColor = "32 32 32 255";
   fontColors[0] = "196 196 196 255";
};

if( !isObject( ToolsGuiRadioProfile ) )
new GuiControlProfile( ToolsGuiRadioProfile )
{
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

if( !isObject( ToolsGuiScrollProfile ) )
new GuiControlProfile( ToolsGuiScrollProfile )
{
   opaque = true;
   fillcolor = "21 21 21 255";
   fontColor = "0 0 0";
   fontColorHL = "150 150 150";
   border = "1";
   bitmap = "tools/gui/images/scrollBar";
   hasBitmapArray = true;
   category = "Tools";
   fontColors[0] = "0 0 0 255";
   fontColors[1] = "150 150 150 255";
};

if( !isObject( ToolsGuiOverlayProfile ) )
new GuiControlProfile( ToolsGuiPopupMenuItemBorder )
{
   opaque = true;
   fillcolor = "21 21 21 255";
   fontColor = "196 196 196 255";
   fontColorHL = "255 255 255 255";
	fillColor = "0 0 0 100";
   category = "Tools";
   canKeyFocus = "0";
   fillColorHL = "72 72 72 255";
   fillColorNA = "18 18 18 255";
   fillColorSEL = "116 116 116 255";
   fontColors[0] = "196 196 196 255";
   fontColors[1] = "255 255 255 255";
   fontColors[2] = "128 128 128 255";
   fontColors[4] = "160 72 64 255";
   fontColors[5] = "196 116 108 255";
   fontColorNA = "128 128 128 255";
   fontColorLink = "160 72 64 255";
   fontColorLinkHL = "196 116 108 255";
   fontColors[3] = "255 255 255 255";
   fontColorSEL = "255 255 255 255";
};

if( !isObject( ToolsGuiSliderProfile ) )
new GuiControlProfile( ToolsGuiPopUpMenuEditProfile )
{
   bitmap = "tools/gui/images/dropDown";
   category = "Tools";
   canKeyFocus = "1";
   fillColor = "21 21 21 255";
   fillColorHL = "72 72 72 255";
   fillColorNA = "18 18 18 255";
   fillColorSEL = "116 116 116 255";
   border = "1";
   textOffset = "6 4";
   profileForChildren = "ToolsGuiPopUpMenuDefault";
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

if( !isObject( ToolsGuiSliderBoxProfile ) )
new GuiControlProfile( ToolsGuiPopUpMenuDefault )
{
   bitmap = "tools/gui/images/scrollBar";
   category = "Tools";
   mouseOverSelected = "1";
   opaque = "1";
   borderThickness = "0";
   textOffset = "3 3";
   hasBitmapArray = "1";
   profileForChildren = "ToolsGuiPopupMenuItemBorder";
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

if( !isObject( ToolsGuiPopupMenuItemBorder ) )
new GuiControlProfile( ToolsGuiPopupMenuItemBorder : ToolsGuiButtonProfile )
{
   opaque = true;
   border = true;
   fontColor = "196 196 196 255";
   fontColorHL = "255 255 255 255";
   fontColorNA = "128 128 128 255";
   fontColorLink = "160 72 64 255";
   fontColorLinkHL = "196 116 108 255";
   fixedExtent = false;
   justify = "center";
   canKeyFocus = false;
   bitmap = "./images/button";
   category = "Tools";
};

if( !isObject( ToolsGuiPopUpMenuDefault ) )
new GuiControlProfile( ToolsGuiPopUpMenuTabProfile : ToolsGuiDefaultProfile )
{
   opaque = true;
   mouseOverSelected = true;
   textOffset = "6 4";
   border = "0";
   borderThickness = 0;
   bitmap = "tools/gui/images/dropDown-tab";
   hasBitmapArray = true;
   profileForChildren = "ToolsGuiPopUpMenuDefault";
   fillColor = "21 21 21 255";
   fillColorHL = "72 72 72 255";
   fillColorSEL = "116 116 116 255";
   borderColor = "100 100 100";
   category = "Tools";
   canKeyFocus = "1";
   fillColorNA = "18 18 18 255";
   fontColors[0] = "196 196 196 255";
   fontColors[1] = "255 255 255 255";
   fontColors[2] = "255 255 255 255";
   fontColor = "196 196 196 255";
   fontColorNA = "255 255 255 255";
   fontColorHL = "255 255 255 255";
   fontColorSEL = "255 255 255";
};

if( !isObject( ToolsGuiPopUpMenuProfile ) )
new GuiControlProfile( ToolsGuiPopUpMenuProfile : ToolsGuiPopUpMenuDefault )
{
   textOffset         = "6 4";
   bitmap             = "tools/gui/images/dropDown";
   hasBitmapArray     = true;
   border             = 1;
   profileForChildren = ToolsGuiPopUpMenuDefault;
   category = "Tools";
   fillColor = "21 21 21 255";
   fillColorHL = "72 72 72 255";
   fillColorNA = "18 18 18 255";
   fillColorSEL = "116 116 116 255";
   borderColor = "128 128 128 255";
   fontColors[0] = "196 196 196 255";
   fontColors[1] = "255 255 255 255";
   fontColors[2] = "128 128 128 255";
   fontColors[3] = "255 255 255 255";
   fontColors[4] = "160 72 64 255";
   fontColors[5] = "196 116 108 255";
   fontColor = "196 196 196 255";
   fontColorHL = "255 255 255 255";
   fontColorNA = "128 128 128 255";
   fontColorSEL = "255 255 255 255";
   fontColorLink = "160 72 64 255";
   fontColorLinkHL = "196 116 108 255";
};

if( !isObject( ToolsGuiPopUpMenuTabProfile ) )
new GuiControlProfile( ToolsGuiPopUpMenuTabProfile : ToolsGuiPopUpMenuDefault )
{
   bitmap             = "./images/dropDown-tab";
   textOffset         = "6 4";
   canKeyFocus        = true;
   hasBitmapArray     = true;
   border             = 1;
   profileForChildren = ToolsGuiPopUpMenuDefault;
   category = "Tools";
   
   fontColor = "255 255 255 255";
   fontColorHL = "255 255 255 255";
   fontColorNA = "255 255 255 255";
   fontColorSEL = "255 255 255 255";
};

if( !isObject( ToolsGuiPopUpMenuEditProfile ) )
new GuiControlProfile( ToolsGuiPopUpMenuEditProfile : ToolsGuiPopUpMenuDefault )
{
   textOffset         = "6 4";
   canKeyFocus        = true;
   bitmap             = "./images/dropDown";
   hasBitmapArray     = true;
   border             = 1;
   profileForChildren = ToolsGuiPopUpMenuDefault;
   category = "Tools";
};

if( !isObject( ToolsGuiListBoxProfile ) )
new GuiControlProfile( ToolsGuiListBoxProfile )
{
   tab = true;
   canKeyFocus = true;
   category = "Tools";
};

if( !isObject( ToolsGuiTabBookProfile ) )
new GuiControlProfile( ToolsGuiTabBookProfile )
{
   fillColorHL = "100 100 100";
   fillColorNA = "150 150 150";
   fontColor = "224 224 224 255";
   fontColorHL = "255 255 255 255";
   fontColorNA = "64 64 64 255";
   fontType = "Arial";
   fontSize = 14;
   justify = "center";
   bitmap = "tools/gui/images/tab";
   tabWidth = 64;
   tabHeight = 24;
   tabPosition = "Top";
   tabRotation = "Horizontal";
   textOffset = "0 -3";
   tab = true;
   cankeyfocus = true;
   category = "Tools";
   fontColors[0] = "224 224 224 255";
   fontColors[2] = "64 64 64 255";
   fontColors[3] = "255 255 255 255";
   fontColorSEL = "255 255 255 255";
};

if( !isObject( ToolsGuiTabBookNoBitmapProfile ) )
new GuiControlProfile( ToolsGuiTabBookNoBitmapProfile : ToolsGuiTabBookProfile )
{
   bitmap = "";
   category = "Tools";
};

if( !isObject( ToolsGuiTabPageProfile ) )
new GuiControlProfile( ToolsGuiTabPageProfile : ToolsGuiDefaultProfile )
{
   fontType = "Arial";
   fontSize = 10;
   justify = "center";
   bitmap = "tools/gui/images/tab";
   opaque = false;
   fillColor = "21 21 21 255";
   category = "Tools";
};

if( !isObject( ToolsGuiTreeViewProfile ) )
new GuiControlProfile( ToolsGuiTreeViewProfile )
{  
   bitmap = "tools/gui/images/treeView";
   autoSizeHeight = true;
   canKeyFocus = true;
   fillColor = "21 21 21 255"; 
   fillColorHL = "72 72 72 255";
   fillColorSEL = "128 128 128 255";
   fillColorNA = "21 21 21 255";
   fontColor = "196 196 196 255";
   fontColorHL = "255 255 255 255";   
   fontColorSEL= "255 255 255 255";
   fontColorNA = "128 128 128 255";
   borderColor = "128 000 000";
   borderColorHL = "255 228 235";
   fontSize = 14;   
   opaque = false;
   border = false;
   category = "Tools";
   fontColors[0] = "196 196 196 255";
   fontColors[1] = "255 255 255 255";
   fontColors[2] = "128 128 128 255";
   fontColors[3] = "255 255 255 255";
};

if( !isObject( ToolsGuiTextPadProfile ) )
new GuiControlProfile( ToolsGuiTextPadProfile )
{
   fontType ="Lucida Console";
   fontSize ="12";
   tab = true;
   canKeyFocus = true;
   
   // Deviate from the Default
   opaque=true;  
   fillColor = "21 21 21 255";   
   border = 0;
   category = "Tools";
};

if( !isObject( ToolsGuiFormProfile ) )
new GuiControlProfile( ToolsGuiFormProfile : ToolsGuiTextProfile )
{
   opaque = false;
   border = 5;
   justify = "center";
   profileForChildren = ToolsGuiButtonProfile;
   opaque = false;
   hasBitmapArray = true;
   bitmap = "./images/button";
   category = "Tools";
};

// ----------------------------------------------------------------------------

singleton GuiControlProfile( GuiEditorClassProfile )
{
   opaque = true;
   fillColor = "32 32 32 255";
   border = 0;
   borderColor   = "40 40 40 140";
   borderColorHL = "127 127 127";
   fontColor = "196 196 196 255";
   fontColorHL = "255 255 255 255";
   fixedExtent = true;
   justify = "center";
   bitmap = "tools/gui/images/scrollBar";
   hasBitmapArray = true;
   category = "Editor";
};

singleton GuiControlProfile( GuiBackFillProfile )
{
   opaque = true;
   fillColor = "36 36 36 255";
   border = false;
   borderColor = "255 128 128";
   fontType = "Arial";
   fontSize = 12;
   fontColor = "196 196 196 255";
   fontColorHL = "255 255 255 255";
   fixedExtent = true;
   justify = "center";
   category = "Editor";
};

singleton GuiControlProfile( GuiControlListPopupProfile )
{
   opaque = true;
   fillColor = "32 32 32 255";
   fillColorHL = "72 72 72 255";
   border = false;
   //borderColor = "0 0 0";
   fontColor = "196 196 196 255";
   fontColorHL = "255 255 255 255";
   fontColorNA = "128 128 128 255";
   textOffset = "0 2";
   autoSizeWidth = false;
   autoSizeHeight = true;
   tab = true;
   canKeyFocus = true;
   bitmap = "tools/gui/images/scrollBar";
   hasBitmapArray = true;
   category = "Editor";
};

singleton GuiControlProfile( GuiSceneGraphEditProfile )
{
   canKeyFocus = true;
   tab = true;
   category = "Editor";
};

singleton GuiControlProfile( GuiPreviewBackgroundProfile )
{
   borderColor = "21 21 21 255";
   borderColorNA = "72 72 72 255";
   fillColorNA = "36 36 36 0";
   borderColorHL = "0 0 0 255";
   bitmap = "tools/gui/images/preview_grid";
   category = "Editor";
};

singleton GuiControlProfile( GuiInspectorButtonProfile : ToolsGuiButtonProfile )
{
   //border = 1;
   justify = "Center";
   category = "Editor";
};

singleton GuiControlProfile( GuiInspectorSwatchButtonProfile )
{
   borderColor = "100 100 100 255";
   borderColorNA = "200 200 200 255";
   fillColorNA = "255 255 255 0";
   borderColorHL = "0 0 0 255";
   category = "Editor";
};

singleton GuiControlProfile( GuiInspectorTextEditProfile )
{
   // Transparent Background
   opaque = true;
   fillColor = "21 21 21 0";
   fillColorHL = "72 72 72 255";

   // No Border (Rendered by field control)
   border = false;

   tab = true;
   canKeyFocus = true;

   // font
   fontType = "Arial";
   fontSize = 14;

   fontColor = "196 196 196 255";
   fontColorSEL = "196 116 108 255";
   fontColorHL = "255 255 255 255";
   fontColorNA = "128 128 128 255";
   category = "Editor";
};
singleton GuiControlProfile( GuiDropdownTextEditProfile :  ToolsGuiTextEditProfile )
{
   bitmap = "tools/gui/images/dropdown-textEdit";
   category = "Editor";
};
singleton GuiControlProfile( GuiInspectorTextEditRightProfile : GuiInspectorTextEditProfile )
{
   justify = "right";
   category = "Editor";
};

singleton GuiControlProfile( GuiInspectorGroupProfile )
{
   fontType    = "Arial";
   fontSize    = "14";
   
   fillColor = "32 32 32 255";
   fillColorHL = "72 72 72 255";
   
   fontColor = "196 196 196 255";
   fontColorSEL = "196 116 108 255";
   fontColorHL = "255 255 255 255";
   fontColorNA = "128 128 128 255";
   
   justify = "left";
   opaque = false;
   border = false;
  
   bitmap = "tools/editorclasses/gui/images/rollout";
   
   textOffset = "20 0";

   category = "Editor";
};

singleton GuiControlProfile( GuiInspectorFieldProfile)
{
   // fill color
   opaque = false;
   fillColor = "21 21 21 255";
   fillColorHL = "64 64 64 255";
   fillColorNA = "128 128 128 255";

   // border color
   border = false;
   borderColor   = "190 190 190";
   borderColorHL = "156 156 156";
   borderColorNA = "200 200 200";
   
   //bevelColorHL = "255 255 255";
   //bevelColorLL = "0 0 0";

   // font
   fontType = "Arial";
   fontSize = 14;

   fontColor = "196 196 196 255";
   fontColorSEL = "196 116 108 255";
   fontColorHL = "255 255 255 255";
   fontColorNA = "128 128 128 255";
   textOffset = "10 0";

   tab = true;
   canKeyFocus = true;
   category = "Editor";
};

/*
singleton GuiControlProfile( GuiInspectorMultiFieldProfile : GuiInspectorFieldProfile )
{
   opaque = true;
   fillColor = "50 50 230 30";
};
*/

singleton GuiControlProfile( GuiInspectorMultiFieldDifferentProfile : GuiInspectorFieldProfile )
{
   border = false;
   borderColor = "21 21 21 255";
};

singleton GuiControlProfile( GuiInspectorDynamicFieldProfile : GuiInspectorFieldProfile )
{
   // Transparent Background
   opaque = true;
   fillColor = "21 21 21 0";
   fillColorHL = "72 72 72 255";

   // No Border (Rendered by field control)
   border = false;

   tab = true;
   canKeyFocus = true;

   // font
   fontType = "Arial";
   fontSize = 14;

   fontColor = "196 196 196 255";
   fontColorSEL = "196 116 108 255";
   fontColorHL = "255 255 255 255";
   fontColorNA = "128 128 128 255";
   category = "Editor";
};

singleton GuiControlProfile( GuiRolloutProfile )
{
   border = 0;
   borderColor = "200 200 200";
   
   hasBitmapArray = true;
   bitmap = "tools/editorClasses/gui/images/rollout";
   
   textoffset = "17 0";
   category = "Editor";
};

singleton GuiControlProfile( GuiInspectorRolloutProfile0 )
{
   // font
   fontType = "Arial";
   fontSize = 14;
   
   fillColor = "21 21 21 255";
   fillColorHL = "72 72 72 255";

   fontColor = "196 196 196 255";
   fontColorSEL = "196 116 108 255";
   fontColorHL = "255 255 255 255";
   fontColorNA = "128 128 128 255";
   
   justify = "left";
   opaque = false;
   
   border = 0;
   borderColor   = "72 72 72";
   borderColorHL = "72 72 72";
   borderColorNA = "72 72 72";
  
   bitmap = "tools/editorclasses/gui/images/rollout_plusminus_header";
   
   textOffset = "20 0";
   category = "Editor";
};

singleton GuiControlProfile( GuiInspectorStackProfile )
{
   opaque = false;
   border = false;
   category = "Editor";
};

singleton GuiControlProfile( GuiInspectorProfile  : GuiInspectorFieldProfile )
{
   opaque = true;
   fillColor = "21 21 21 255";
   border = 0;
   cankeyfocus = true;
   tab = true;
   category = "Editor";
};
singleton GuiControlProfile( GuiInspectorInfoProfile  : GuiInspectorFieldProfile )
{
   opaque = true;
   fillColor = "21 21 21 255";
   border = 0;
   cankeyfocus = true;
   tab = true;
   category = "Editor";
};

singleton GuiControlProfile( GuiInspectorBackgroundProfile : GuiInspectorFieldProfile )
{
   border = 0;
   cankeyfocus=true;
   tab = true;
   category = "Editor";
};

singleton GuiControlProfile( GuiInspectorTypeFileNameProfile )
{
   // Transparent Background
   opaque = false;

   // No Border (Rendered by field control)
   border = 0;

   tab = true;
   canKeyFocus = true;

   // font
   fontType = "Arial";
   fontSize = 14;
   
   // Center text
   justify = "center";

   fillColor = "21 21 21 255";
   fillColorHL = "72 72 72 255";

   fontColor = "196 196 196 255";
   fontColorSEL = "196 116 108 255";
   fontColorHL = "255 255 255 255";
   fontColorNA = "128 128 128 255";

   borderColor   = "72 72 72";
   borderColorHL = "72 72 72";
   borderColorNA = "72 72 72";
   category = "Editor";
};

singleton GuiControlProfile( GuiInspectorColumnCtrlProfile : GuiInspectorFieldProfile )
{
   opaque = true;
   fillColor = "210 210 210 255"; 
   border = 0;
   category = "Editor";
};

singleton GuiControlProfile( InspectorTypeEnumProfile : GuiInspectorFieldProfile )
{
   mouseOverSelected = true;
   bitmap = "tools/gui/images/scrollBar";
   hasBitmapArray = true;
   opaque=true;
   border=false;
   textOffset = "4 0";
   category = "Editor";
};

singleton GuiControlProfile( InspectorTypeCheckboxProfile : GuiInspectorFieldProfile )
{
   bitmap = "tools/gui/images/checkBox";
   hasBitmapArray = true;
   opaque=false;
   border=false;
   textOffset = "4 0";
   category = "Editor";
};

singleton GuiControlProfile( GuiToolboxButtonProfile : ToolsGuiButtonProfile )
{
   justify = "center";
   fontColor = "196 196 196 255";
   fontColorHL = "255 255 255 255";
   fontColorNA = "128 128 128 255";
   fontColorLink = "160 72 64 255";
   fontColorLinkHL = "196 116 108 255";
   border = 0;
   textOffset = "0 0";   
   category = "Editor";
};

singleton GuiControlProfile( GuiDirectoryTreeProfile : ToolsGuiTreeViewProfile )
{
   fillColor = "21 21 21 255";
   fillColorHL = "72 72 72 255";

   fontColor = "196 196 196 255";
   fontColorSEL = "196 116 108 255";
   fontColorHL = "255 255 255 255";
   fontColorNA = "128 128 128 255";
   
   fontType = "Arial";
   fontSize = 14;
   category = "Editor";
};

singleton GuiControlProfile( GuiDirectoryFileListProfile )
{
   fillColor = "21 21 21 255";
   fillColorHL = "72 72 72 255";

   fontColor = "196 196 196 255";
   fontColorSEL = "196 116 108 255";
   fontColorHL = "255 255 255 255";
   fontColorNA = "128 128 128 255";
   
   fontType = "Arial";
   fontSize = 14;
   category = "Editor";
};

singleton GuiControlProfile( GuiDragAndDropProfile )
{
   category = "Editor";
};

singleton GuiControlProfile( GuiInspectorFieldInfoPaneProfile )
{
   opaque = false;
   fillcolor = GuiInspectorBackgroundProfile.fillColor;
   borderColor = ToolsGuiDefaultProfile.borderColor;
   border = 0;
   category = "Editor";
};

singleton GuiControlProfile( GuiInspectorFieldInfoMLTextProfile : ToolsGuiMLTextProfile )
{
   opaque = false;   
   border = 0;   
   textOffset = "5 0";
   category = "Editor";
};

singleton GuiControlProfile( GuiEditorScrollProfile )
{
   opaque = true;
   fillcolor = GuiInspectorBackgroundProfile.fillColor;
   borderColor = ToolsGuiDefaultProfile.borderColor;
   border = 0;
   bitmap = "tools/gui/images/scrollBar";
   hasBitmapArray = true;
   category = "Editor";
};

singleton GuiControlProfile( GuiCreatorIconButtonProfile )
{
   opaque = true;       
   fillColor = "64 64 64 255";
   fillColorHL = "128 128 128 255";
   fillColorNA = "21 21 21 255";
   fillColorSEL = "72 72 72 255";
      
   //tab = true;
   //canKeyFocus = true;

   fontType = "Arial";
   fontSize = 14;

   fontColor = "196 196 196 255";
   fontColorSEL = "212 212 212 255";
   fontColorHL = "255 255 255 255";
   fontColorNA = "128 128 128 255";
   
   border = false;
   borderColor   = "72 72 72 0";
   borderColorHL = "72 72 72 0";
   borderColorNA = "72 72 72 0";
   
   category = "Editor";
};
