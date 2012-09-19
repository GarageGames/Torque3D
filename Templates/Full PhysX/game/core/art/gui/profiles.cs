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

//---------------------------------------------------------------------------------------------
// GuiDefaultProfile is a special profile that all other profiles inherit defaults from. It
// must exist.
//---------------------------------------------------------------------------------------------
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

if( !isObject( GuiTransparentwbProfile ) )
new GuiControlProfile (GuiTransparentwbProfile)
{
   opaque = false;
   border = true;
   //fillcolor ="255 255 255";
   borderColor   = "100 100 100";
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

if( !isObject( GuiGroupTitleProfile ) )
new GuiControlProfile( GuiGroupTitleProfile )
{
   fillColor = "242 241 240";
   fillColorHL ="242 241 240";
   fillColorNA = "242 241 240";
   fontColor = "0 0 0";
   opaque = true;
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
   fillcolor = "255 255 255";//GuiDefaultProfile.fillColor;
   borderColor = "246 245 244";
   border = 1;
   //fillColor = "240 239 238";
   //borderColor = "50 50 50";//"204 203 202";
   //fillColor = GuiDefaultProfile.fillColorNA;
   //borderColor   = GuiDefaultProfile.borderColorNA;
   opaque = true;
   border = true;
   category = "Core";
};


if ($platform $= "macos")
{
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
   		justify = "center";
         category = "Core";
   };
}
else {

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
}

if( !isObject( GuiToolbarWindowProfile ) )
new GuiControlProfile(GuiToolbarWindowProfile : GuiWindowProfile)
{
      bitmap = "./images/toolbar-window";
      text = "";
      category = "Core";
}; 

if( !isObject( GuiHToolbarWindowProfile ) ) 
new GuiControlProfile (GuiHToolbarWindowProfile : GuiWindowProfile)
{
      bitmap = "./images/htoolbar-window";
      text = "";
      category = "Core";
};

if( !isObject( GuiMenubarWindowProfile ) ) 
new GuiControlProfile (GuiMenubarWindowProfile : GuiWindowProfile)
{
      bitmap = "./images/menubar-window";
      text = "";
      category = "Core";
}; 

if( !isObject( GuiWindowCollapseProfile ) )
new GuiControlProfile (GuiWindowCollapseProfile : GuiWindowProfile)
{
   category = "Core";
};

if( !isObject( GuiControlProfile ) )
new GuiControlProfile (GuiContentProfile)
{
   opaque = true;
   fillColor = "255 255 255";
   category = "Core";
};

if( !isObject( GuiBlackContentProfile ) )
new GuiControlProfile (GuiBlackContentProfile)
{
   opaque = true;
   fillColor = "0 0 0";
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

if( !isObject( GuiTextBoldProfile ) )
new GuiControlProfile (GuiTextBoldProfile : GuiTextProfile)
{
   fontType = "Arial Bold";
   fontSize = 16;
   category = "Core";
};

if( !isObject( GuiTextBoldCenterProfile ) )
new GuiControlProfile (GuiTextBoldCenterProfile : GuiTextProfile)
{
   fontColor = "50 50 50";
   fontType = "Arial Bold";
   fontSize = 16;
   justify = "center";
   category = "Core";
};

if( !isObject( GuiTextRightProfile ) )
new GuiControlProfile (GuiTextRightProfile : GuiTextProfile)
{
   justify = "right";
   category = "Core";
};

if( !isObject( GuiTextCenterProfile ) )
new GuiControlProfile (GuiTextCenterProfile : GuiTextProfile)
{
   justify = "center";
   category = "Core";
};

if( !isObject( GuiTextSolidProfile ) )
new GuiControlProfile (GuiTextSolidProfile : GuiTextProfile)
{
   opaque = true;
   border = 5;
   borderColor = GuiDefaultProfile.fillColor;
   category = "Core";
};

if( !isObject( InspectorTitleTextProfile ) )
new GuiControlProfile (InspectorTitleTextProfile)
{
   fontColor = "100 100 100";
   category = "Core";
};

if( !isObject( GuiTextProfileLight ) )
new GuiControlProfile (GuiTextProfileLight)
{
   fontColor = "220 220 220";
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

if( !isObject( GuiTextRightProfile ) )
new GuiControlProfile (GuiTextRightProfile : GuiTextProfile)
{
   justify = "right";
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

if( !isObject( GuiTextListProfile ) )
new GuiControlProfile( GuiTextListProfile : GuiTextProfile ) 
{
   tab = true;
   canKeyFocus = true;
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

if( !isObject( GuiTreeViewRenameCtrlProfile ) )
new GuiControlProfile( GuiTreeViewRenameCtrlProfile : GuiTextEditProfile )
{
   returnTab = true;
   category = "Core";
};

if( !isObject( GuiTextEditProfileNumbersOnly ) )
new GuiControlProfile( GuiTextEditProfileNumbersOnly : GuiTextEditProfile )
{
   numbersOnly = true;
   category = "Core";
};

if( !isObject( GuiTextEditNumericProfile ) )
new GuiControlProfile( GuiTextEditNumericProfile : GuiTextEditProfile )
{
   numbersOnly = true;
   category = "Core";
};

if( !isObject( GuiNumericDropSliderTextProfile ) )
new GuiControlProfile( GuiNumericDropSliderTextProfile : GuiTextEditProfile )
{
   bitmap = "./images/textEditSliderBox";
   category = "Core";
};

if( !isObject( GuiTextEditDropSliderNumbersOnly ) )
new GuiControlProfile( GuiTextEditDropSliderNumbersOnly :  GuiTextEditProfile )
{
   numbersOnly = true;
   bitmap = "./images/textEditSliderBox";
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

if( !isObject( GuiRLProgressBitmapProfile ) )
new GuiControlProfile( GuiRLProgressBitmapProfile )
{
   border = false;
   hasBitmapArray = true;
   bitmap = "./images/rl-loadingbar";
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

if( !isObject( GuiThumbHighlightButtonProfile ) )
new GuiControlProfile( GuiThumbHighlightButtonProfile : GuiButtonProfile )
{
   bitmap = "./images/thumbHightlightButton";
   category = "Core";
};

if( !isObject( InspectorDynamicFieldButton ) )
new GuiControlProfile( InspectorDynamicFieldButton : GuiButtonProfile )
{
   canKeyFocus = true;
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

if( !isObject( GuiIconButtonProfile ) )
new GuiControlProfile( GuiIconButtonProfile )
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
	bitmap = "./images/iconbutton";
   hasBitmapArray = true;
   category = "Core";
};

if( !isObject( GuiIconButtonSolidProfile ) )
new GuiControlProfile( GuiIconButtonSolidProfile : GuiIconButtonProfile )
{
   bitmap = "./images/iconbuttonsolid";
   category = "Core";
};

if( !isObject( GuiIconButtonSmallProfile ) )
new GuiControlProfile( GuiIconButtonSmallProfile : GuiIconButtonProfile )
{
   bitmap = "./images/iconbuttonsmall";
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
	 //fontColorSEL ="0 0 0";
   fixedExtent = false;
   justify = "center";
   canKeyFocus = false;
   bitmap = "./images/buttontab";
  // hasBitmapArray = false;
  category = "Core";
};

if( !isObject( EditorTabPage ) )
new GuiControlProfile(EditorTabPage)
{
   opaque = true;
   border = false;
   //fontSize = 18;
   //fontType = "Arial";
   fontColor = "0 0 0";
   fontColorHL = "0 0 0";
   fixedExtent = false;
   justify = "center";
   canKeyFocus = false;
   bitmap = "./images/tab";
   hasBitmapArray = true; //false;
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

if( !isObject( GuiCheckBoxListProfile ) )
new GuiControlProfile( GuiCheckBoxListProfile : GuiCheckBoxProfile)
{
   bitmap = "./images/checkbox-list";
   category = "Core";
};

if( !isObject( GuiCheckBoxListFlipedProfile ) )
new GuiControlProfile( GuiCheckBoxListFlipedProfile : GuiCheckBoxProfile)
{
   bitmap = "./images/checkbox-list_fliped";
   category = "Core";
};

if( !isObject( InspectorCheckBoxTitleProfile ) )
new GuiControlProfile( InspectorCheckBoxTitleProfile : GuiCheckBoxProfile ){
   fontColor = "100 100 100";
   category = "Core";
};

if( !isObject( GuiRadioProfile ) )
new GuiControlProfile( GuiRadioProfile )
{
   fontSize = 14;
   fillColor = "232 232 232";
	/*fontColor = "200 200 200";
   fontColorHL = "255 255 255";*/
   fontColor = "20 20 20";
   fontColorHL = "80 80 80";
   fixedExtent = true;
   bitmap = "./images/radioButton";
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

if( !isObject( GuiTransparentScrollProfile ) )
new GuiControlProfile( GuiTransparentScrollProfile )
{
   opaque = false;
   fillColor = "255 255 255";
	 fontColor = "0 0 0";
   border = false;
   borderThickness = 2;
   borderColor = "100 100 100";
   bitmap = "./images/scrollBar";
   hasBitmapArray = true;
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

if( !isObject( GuiPaneProfile ) )
new GuiControlProfile( GuiPaneProfile )
{
   bitmap = "./images/popupMenu";
   hasBitmapArray = true;
   category = "Core";
};

if( !isObject( GuiPopupMenuItemBorder ) )
new GuiControlProfile( GuiPopupMenuItemBorder : GuiButtonProfile )
{
  // borderThickness = 1;
   //borderColor = "100 100 100 220"; //200
   //borderColorHL = "51 51 51 220"; //200
   
    opaque = true;
   border = true;
	 
   fontColor = "0 0 0";
   fontColorHL = "0 0 0";
   fontColorNA = "255 255 255";
   fixedExtent = false;
   justify = "center";
   canKeyFocus = false;
	 bitmap = "./images/button";
  // hasBitmapArray = false;

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

if( !isObject( GuiPopupBackgroundProfile ) )
new GuiControlProfile( GuiPopupBackgroundProfile )
{
   modal = true;
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

if( !isObject( GuiPopUpMenuTabProfile ) )
new GuiControlProfile( GuiPopUpMenuTabProfile : GuiPopUpMenuDefault )
{
   bitmap             = "./images/dropDown-tab";
   textOffset         = "6 4";
   canKeyFocus        = true;
   hasBitmapArray     = true;
   border             = 1;
   profileForChildren = GuiPopUpMenuDefault;
   category = "Core";
};

if( !isObject( GuiPopUpMenuEditProfile ) )
new GuiControlProfile( GuiPopUpMenuEditProfile : GuiPopUpMenuDefault )
{
   textOffset         = "6 4";
   canKeyFocus        = true;
   bitmap             = "./images/dropDown";
   hasBitmapArray     = true;
   border             = 1;
   profileForChildren = GuiPopUpMenuDefault;
   category = "Core";
};

if( !isObject( GuiListBoxProfile ) )
new GuiControlProfile( GuiListBoxProfile )
{
   tab = true;
   canKeyFocus = true;
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
   //border = false;
   //opaque = false;
   category = "Core";
};

if( !isObject( GuiTabBookNoBitmapProfile ) )
new GuiControlProfile( GuiTabBookNoBitmapProfile : GuiTabBookProfile )
{
   bitmap = "";
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

if( !isObject( GuiMenuBarProfile ) )
new GuiControlProfile( GuiMenuBarProfile )
{
   fontType = "Arial";
   fontSize = 14;
   opaque = true;
   fillColor = "240 239 238";
   fillColorHL = "202 201 200";
   fillColorSEL = "202 0 0";
   
   borderColorNA = "202 201 200";
   borderColorHL = "50 50 50";
   border = 0;
   fontColor = "20 20 20";
   fontColorHL = "0 0 0";
   fontColorNA = "255 255 255";
   //fixedExtent = true;
   justify = "center";
   canKeyFocus = false;
   mouseOverSelected = true;
   bitmap = "./images/menu";
   hasBitmapArray = true;
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

if( !isObject( GuiConsoleTextEditProfile ) )
new GuiControlProfile( GuiConsoleTextEditProfile : GuiTextEditProfile )
{
   fontType = ($platform $= "macos") ? "Monaco" : "Lucida Console";
   fontSize = ($platform $= "macos") ? 13 : 12;
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

if( !isObject( GuiTreeViewProfile ) )
new GuiControlProfile( GuiTreeViewProfile )
{  
   bitmap            = "./images/treeView";
   autoSizeHeight    = true;
   canKeyFocus       = true;
   
   fillColor = "255 255 255"; //GuiDefaultProfile.fillColor;
   fillColorHL = "228 228 235";//GuiDefaultProfile.fillColorHL;
   fillColorSEL = "98 100 137";
   fillColorNA = "255 255 255";//GuiDefaultProfile.fillColorNA;
   fontColor = "0 0 0";//GuiDefaultProfile.fontColor;
   fontColorHL = "0 0 0";//GuiDefaultProfile.fontColorHL;   
   fontColorSEL= "255 255 255";//GuiDefaultProfile.fontColorSEL;
   fontColorNA = "200 200 200";//GuiDefaultProfile.fontColorNA;
   borderColor = "128 000 000";
   borderColorHL = "255 228 235";

   fontSize = 14;
   
   opaque = false;
   border = false;
   category = "Core";
};

if( !isObject( GuiSimpleTreeProfile ) )
new GuiControlProfile( GuiSimpleTreeProfile : GuiTreeViewProfile )
{
   opaque = true;
   fillColor = "255 255 255 255";
   border = true;
   category = "Core";
};

if( !isObject( GuiText24Profile ) )
new GuiControlProfile( GuiText24Profile : GuiTextProfile )
{
   fontSize = 24;
   category = "Core";
};

if( !isObject( GuiRSSFeedMLTextProfile ) )
new GuiControlProfile( GuiRSSFeedMLTextProfile )
{
   fontColorLink = "55 55 255";
   fontColorLinkHL = "255 55 55";
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

if( !isObject( GuiTextPadProfile ) )
new GuiControlProfile( GuiTextPadProfile )
{
   fontType = ($platform $= "macos") ? "Monaco" : "Lucida Console";
   fontSize = ($platform $= "macos") ? 13 : 12;
   tab = true;
   canKeyFocus = true;
   
   // Deviate from the Default
   opaque=true;  
   fillColor = "255 255 255";
   
   border = 0;
   category = "Core";
};

if( !isObject( GuiTransparentProfileModeless ) )
new GuiControlProfile( GuiTransparentProfileModeless : GuiTransparentProfile )  
{
   modal = false;
   category = "Core";
};

if( !isObject( GuiFormProfile ) )
new GuiControlProfile( GuiFormProfile : GuiTextProfile )
{
   opaque = false;
   border = 5;
   
   bitmap = "./images/form";
   hasBitmapArray = true;

   justify = "center";
   
   profileForChildren = GuiButtonProfile;
   
   // border color
	 opaque = false;
   //border = 5;
	 bitmap = "./images/button";
  // borderColor   = "153 153 153"; 
 //  borderColorHL = "230 230 230";
 //  borderColorNA = "126 79 37";
 category = "Core";
};

if( !isObject( GuiNumericTextEditSliderProfile ) )
new GuiControlProfile( GuiNumericTextEditSliderProfile )
{
   // Transparent Background
   opaque = true;
   fillColor = "0 0 0 0";
   fillColorHL = "255 255 255";
   
   border = true;   

   tab = false;
   canKeyFocus = true;

   // font
   fontType = "Arial";
   fontSize = 14;

   fontColor = "0 0 0";
   fontColorSEL = "43 107 206";
   fontColorHL = "244 244 244";
   fontColorNA = "100 100 100";
   
   numbersOnly = true;
   category = "Core";
};

if( !isObject( GuiNumericTextEditSliderBitmapProfile ) )
new GuiControlProfile( GuiNumericTextEditSliderBitmapProfile )
{
   // Transparent Background
   opaque = true;
   
   border = true;   
   borderColor = "100 100 100";
   
   tab = false;
   canKeyFocus = true;

   // font
   fontType = "Arial";
   fontSize = 14;

   fillColor = "242 241 240";//"255 255 255";
   fillColorHL = "255 255 255";//"222 222 222";
   fontColor = "0 0 0";//"0 0 0";
   fontColorHL = "255 255 255";//"0 0 0";
   fontColorSEL = "98 100 137";//"230 230 230";
   fontColorNA = "200 200 200";//"0 0 0";
   
   numbersOnly = true;
   
   hasBitmapArray = true;
   bitmap = "./images/numericslider";
   category = "Core";
};

if( !isObject( GuiMultiFieldTextEditProfile ) )
new GuiControlProfile( GuiMultiFieldTextEditProfile : GuiTextEditProfile )
{
   category = "Core";
};

if( !isObject( GuiModalDialogBackgroundProfile ) )
new GuiControlProfile( GuiModalDialogBackgroundProfile )
{
   opaque = true;
   fillColor = "221 221 221 150";
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
