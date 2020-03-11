if( !isObject( GuiMenuButtonProfile ) )
new GuiControlProfile( GuiMenuButtonProfile )
{
   opaque = true;
   border = false;
   fontSize = 18;
   fontType = "Arial Bold";
   fontColor = "240 240 240";
   fontColorHL = "0 0 0";
   fontColorNA = "125 125 125";
   //fontColorSEL ="0 0 0";
   fixedExtent = false;
   justify = "center";
   canKeyFocus = false;
   bitmap = "data/ui/art/menu-button";
   hasBitmapArray = false;
   soundButtonDown = menuButtonPressed;
   soundButtonOver = menuButtonHover;
   category = "Core";
};

if( !isObject( GuiHighlightMenuButtonProfile ) )
new GuiControlProfile( GuiHighlightMenuButtonProfile )
{
   opaque = true;
   border = false;
   fontSize = 18;
   fontType = "Arial Bold";
   fontColor = "240 240 240";
   fontColorHL = "0 0 0";
   fontColorNA = "125 125 125";
   //fontColorSEL ="0 0 0";
   fixedExtent = false;
   justify = "center";
   canKeyFocus = false;
   bitmap = "data/ui/art/selector-button-highlight-only";
   hasBitmapArray = false;
   category = "Core";
};

if( !isObject( GuiBlankMenuButtonProfile ) )
new GuiControlProfile( GuiBlankMenuButtonProfile )
{
   opaque = true;
   border = false;
   fontSize = 18;
   fontType = "Arial Bold";
   fontColor = "200 200 200";
   fontColorHL = "255 255 255";
   fontColorNA = "200 200 200";
   //fontColorSEL ="0 0 0";
   fixedExtent = false;
   justify = "center";
   canKeyFocus = false;
   bitmap = "data/ui/art/selector-button-blank";
   hasBitmapArray = false;
   soundButtonDown = menuButtonPressed;
   soundButtonOver = menuButtonHover;
   category = "Core";
};

if( !isObject( GuiMenuTextProfile ) )
new GuiControlProfile( GuiMenuTextProfile )
{
   opaque = true;
   border = false;
   fontSize = 18;
   fontType = "Arial Bold";
   fontColor = "240 240 240";
   fontColorHL = "0 0 0";
   fontColorNA = "125 125 125";
   fixedExtent = false;
   justify = "center";
   category = "Core";
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
   bitmap = "data/ui/art/group-border";
   category = "Core";
};

if( !isObject( GuiTabBorderProfile ) )
new GuiControlProfile( GuiTabBorderProfile )
{
   border = false;
   opaque = false;
   hasBitmapArray = true;
   bitmap = "data/ui/art/tab-border";
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

if( !isObject( GuiMLWhiteTextProfile ) )
new GuiControlProfile( GuiMLWhiteTextProfile )
{
   fontColor = "220 220 220";
   fontColorHL = "255 255 255";
   autoSizeWidth = true;
   autoSizeHeight = true;  
   border = false;
   category = "Core";
};

if( !isObject( GuiTextArrayProfile ) )
new GuiControlProfile( GuiTextArrayProfile : GuiTextProfile )
{
   fontColor = "250 250 250";
   fontColorHL = " 0 0 0";
   fontColorSEL = "0 0 0";
   fillColor ="50 50 50";
   fillColorHL = "125 125 125";
   fillColorSEL = "180 180 180";
   border = false;
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
   bitmap = "data/ui/art/button";
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
   bitmap = "data/ui/art/scrollBar";
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
   bitmap             = "data/ui/art/dropDown";
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
   bitmap = "data/ui/art/tab";
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
   bitmap = "data/ui/art/tab";
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
   bitmap = "data/ui/art/radioButton";
   hasBitmapArray = true;
   category = "Core";
};

// ---------------------------------------------------------------------------
// Slider control
// ---------------------------------------------------------------------------
if( !isObject( GuiSliderProfile ) )
new GuiControlProfile( GuiSliderProfile )
{
   bitmap = "data/ui/art/slider";
   category = "Core";
};

//
// Scroll Profile
//
if(!isObject(GuiMenuScrollProfile))
new GuiControlProfile(GuiMenuScrollProfile)
{
   opaque = true;
   fillcolor = "50 50 50";
   fontColor = "200 200 200";
   fontColorHL = "250 250 250";
   border = true;
   bitmap = "data/ui/art/scrollBar";
   hasBitmapArray = true;
   category = "Core";
};

// Scroll
if(!isObject(GuiMenuScrollProfile))
new GuiControlProfile(GuiMenuScrollProfile)
{
   opaque = true;
   fillcolor = "128 128 128";
   fontColor = "0 0 0";
   fontColorHL = "150 150 150";
   border = true;
   bitmap = "./images/scrollBar";
   hasBitmapArray = true;
   category = "Core";
};