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

singleton GuiControlProfile( GuiEditorClassProfile )
{
   opaque = true;
   fillColor = "232 232 232";
   border = 1;
   borderColor   = "40 40 40 140";
   borderColorHL = "127 127 127";
   fontColor = "0 0 0";
   fontColorHL = "50 50 50";
   fixedExtent = true;
   justify = "center";
   bitmap = "core/art/gui/images/scrollBar";
   hasBitmapArray = true;
   category = "Editor";
};

singleton GuiControlProfile( GuiBackFillProfile )
{
   opaque = true;
   fillColor = "0 94 94";
   border = true;
   borderColor = "255 128 128";
   fontType = "Arial";
   fontSize = 12;
   fontColor = "0 0 0";
   fontColorHL = "50 50 50";
   fixedExtent = true;
   justify = "center";
   category = "Editor";
};

singleton GuiControlProfile( GuiControlListPopupProfile )
{
   opaque = true;
   fillColor = "255 255 255";
   fillColorHL = "204 203 202";
   border = false;
   //borderColor = "0 0 0";
   fontColor = "0 0 0";
   fontColorHL = "0 0 0";
   fontColorNA = "50 50 50";
   textOffset = "0 2";
   autoSizeWidth = false;
   autoSizeHeight = true;
   tab = true;
   canKeyFocus = true;
   bitmap = "core/art/gui/images/scrollBar";
   hasBitmapArray = true;
   category = "Editor";
};

singleton GuiControlProfile( GuiSceneGraphEditProfile )
{
   canKeyFocus = true;
   tab = true;
   category = "Editor";
};

singleton GuiControlProfile( GuiInspectorButtonProfile : GuiButtonProfile )
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
   fillColor = "0 0 0 0";
   fillColorHL = "255 255 255";

   // No Border (Rendered by field control)
   border = false;

   tab = true;
   canKeyFocus = true;

   // font
   fontType = "Arial";
   fontSize = 14;

   fontColor = "0 0 0";
   fontColorSEL = "43 107 206";
   fontColorHL = "244 244 244";
   fontColorNA = "100 100 100";
   category = "Editor";
};
singleton GuiControlProfile( GuiDropdownTextEditProfile :  GuiTextEditProfile )
{
   bitmap = "core/art/gui/images/dropdown-textEdit";
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
   
   fontColor = "0 0 0 150";
   fontColorHL = "25 25 25 220";
   fontColorNA = "128 128 128";
   
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
   fillColor = "255 255 255";
   fillColorHL = "204 203 202";
   fillColorNA = "244 244 244";

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

   fontColor = "32 32 32";
   fontColorHL = "50 50 50";
   fontColorNA = "190 190 190";
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
   border = true;
   borderColor = "190 100 100";
};

singleton GuiControlProfile( GuiInspectorDynamicFieldProfile : GuiInspectorFieldProfile )
{
   // Transparent Background
   opaque = true;
   fillColor = "0 0 0 0";
   fillColorHL = "255 255 255";

   // No Border (Rendered by field control)
   border = false;

   tab = true;
   canKeyFocus = true;

   // font
   fontType = "Arial";
   fontSize = 14;

   fontColor = "0 0 0";
   fontColorSEL = "43 107 206";
   fontColorHL = "244 244 244";
   fontColorNA = "100 100 100";
   category = "Editor";
};

singleton GuiControlProfile( GuiRolloutProfile )
{
   border = 1;
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

   fontColor = "32 32 32";
   fontColorHL = "32 100 100";
   fontColorNA = "0 0 0";
   
   justify = "left";
   opaque = false;
   
   border = 0;
   borderColor   = "190 190 190";
   borderColorHL = "156 156 156";
   borderColorNA = "64 64 64";
  
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
   fillColor = "255 255 255 255";
   border = 0;
   cankeyfocus = true;
   tab = true;
   category = "Editor";
};
singleton GuiControlProfile( GuiInspectorInfoProfile  : GuiInspectorFieldProfile )
{
   opaque = true;
   fillColor = "242 241 240";
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

   fontColor = "32 32 32";
   fontColorHL = "50 50 50";
   fontColorNA = "0 0 0";

   fillColor = "255 255 255";
   fillColorHL = "204 203 202";
   fillColorNA = "244 244 244";

   borderColor   = "190 190 190";
   borderColorHL = "156 156 156";
   borderColorNA = "64 64 64";
   category = "Editor";
};

singleton GuiControlProfile( GuiInspectorColumnCtrlProfile : GuiInspectorFieldProfile )
{
   opaque = true;
   fillColor = "210 210 210"; 
   border = 0;
   category = "Editor";
};

singleton GuiControlProfile( InspectorTypeEnumProfile : GuiInspectorFieldProfile )
{
   mouseOverSelected = true;
   bitmap = "core/art/gui/images/scrollBar";
   hasBitmapArray = true;
   opaque=true;
   border=true;
   textOffset = "4 0";
   category = "Editor";
};

singleton GuiControlProfile( InspectorTypeCheckboxProfile : GuiInspectorFieldProfile )
{
   bitmap = "core/art/gui/images/checkBox";
   hasBitmapArray = true;
   opaque=false;
   border=false;
   textOffset = "4 0";
   category = "Editor";
};

singleton GuiControlProfile( GuiToolboxButtonProfile : GuiButtonProfile )
{
   justify = "center";
   fontColor = "0 0 0";
   border = 0;
   textOffset = "0 0";   
   category = "Editor";
};

singleton GuiControlProfile( T2DDatablockDropDownProfile : GuiPopUpMenuProfile )
{
   category = "Editor";
};

singleton GuiControlProfile( GuiDirectoryTreeProfile : GuiTreeViewProfile )
{
   fontColor = "40 40 40";
   fontColorSEL= "250 250 250 175"; 
   fillColorHL = "0 60 150";
   fontColorNA = "240 240 240";
   fontType = "Arial";
   fontSize = 14;
   category = "Editor";
};

singleton GuiControlProfile( GuiDirectoryFileListProfile )
{
   fontColor = "40 40 40";
   fontColorSEL= "250 250 250 175"; 
   fillColorHL = "0 60 150";
   fontColorNA = "240 240 240";
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
   borderColor = GuiDefaultProfile.borderColor;
   border = 1;
   category = "Editor";
};

singleton GuiControlProfile( GuiInspectorFieldInfoMLTextProfile : GuiMLTextProfile )
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
   borderColor = GuiDefaultProfile.borderColor;
   border = 1;
   bitmap = "core/art/gui/images/scrollBar";
   hasBitmapArray = true;
   category = "Editor";
};

singleton GuiControlProfile( GuiCreatorIconButtonProfile )
{
   opaque = true;       
   fillColor = "225 243 252 255";
   fillColorHL = "225 243 252 0";
   fillColorNA = "225 243 252 0";
   fillColorSEL = "225 243 252 0";
      
   //tab = true;
   //canKeyFocus = true;

   fontType = "Arial";
   fontSize = 14;

   fontColor = "0 0 0";
   fontColorSEL = "43 107 206";
   fontColorHL = "244 244 244";
   fontColorNA = "100 100 100";
   
   border = 1;
   borderColor   = "153 222 253 255";
   borderColorHL = "156 156 156";
   borderColorNA = "153 222 253 0";
   
   //bevelColorHL = "255 255 255";
   //bevelColorLL = "0 0 0";
   category = "Editor";
};
