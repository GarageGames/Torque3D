//---------------------------------------------------------------------------------------------------------------
// Copyright (C) 2013 WinterLeaf Entertainment LLC.
// 
// THE SOFTWARE IS PROVIDED ON AN “AS IS” BASIS, WITHOUT WARRANTY OF ANY KIND,
// INCLUDING WITHOUT LIMIT ATION THE WARRANTIES OF MERCHANT ABILITY, FITNESS
// FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT . THE ENTIRE RISK AS TO THE
// QUALITY AND PERFORMANCE OF THE SOFTWARE IS THE RESPONSIBILITY OF LICENSEE.
// SHOULD THE SOFTWARE PROVE DEFECTIVE IN ANY RESPECT , LICENSEE AND NOT LICENSOR 
// OR ITS SUPPLIERS OR RESELLERS ASSUMES THE ENTIRE COST OF ANY SERVICE AND
// REPAIR. THIS DISCLAIMER OF WARRANTY CONSTITUTES AN ESSENTIAL PART OF THIS 
// AGREEMENT. NO USE OF THE SOFTWARE IS AUTHORIZED HEREUNDER EXCEPT UNDER
// THIS DISCLAIMER.
// 
// The use of the WinterLeaf Entertainment LLC Advanced UI System
// is governed by this license agreement (“Agreement”).
// 
// R E S T R I C T I O N S
// 
// (a) Licensee may not: (i) create any derivative works, including but not
// limited to translations, localizations or technology add-ons of the Advanced UI System;
// (ii) reverse engineer , or otherwise attempt to derive the algorithms
// for Advanced UI System (iii) redistribute, encumber , sell, rent, lease, sublicense, or otherwise
// transfer rights to Advanced UI System; or (iv) remove or alter any trademark, logo, copyright
// or other proprietary notices, legends, symbols or labels in Advanced UI System; or (iiv) use
// the Software to develop or distribute any software that competes with the Software
// without WinterLeaf Entertainment’s prior written consent; or (iiiv) use the Software for
// any illegal purpose.
// (b) Licensee may not distribute the Advanced UI System in any manner.
// 
// 
// Please visit http://www.winterleafentertainment.com for more information about the project and latest updates.
// 
// Last updated: 09/16/2013
//---------------------------------------------------------------------------------------------------------------

function IngameContext::onAdd( %this )
{
   %filename = $appName;
   $XMLOutput = strreplace( %filename, " ", "") @ "_XML";
}

function IngameContext::onRightMouseUp( %this, %ctrl)
{
   %popup = new PopupMenu( )
   {
      superClass = "MenuBuilder";
      isPopup = true;
      
      object = -1;
   };
      
   %popup.removeAllItems();
   %popup.object = %ctrl;
   
   if( %ctrl.getName() !$= "" )
         %popup.appendItem( %ctrl.getName() TAB "" TAB "");
   else
         %popup.appendItem ( %ctrl.getClassName() TAB "" TAB "");
         
   %popup.enableItem( 0, false);
         
   if( %popup.object.lockControl)
   {
         %lockPos = %popup.appendItem("Locked" TAB "" TAB "%this.object.setControlLock(!%this.object.getControlLock());");
         %popup.checkItem(%lockPos, %ctrl.getControlLock());
   }
   
   if(%popup.object.contextFontColor)
         %fontColorPos = %popup.appendItem("Font Color..." TAB "" TAB "getColorI(  %this.object.controlFontColor , %this.object @ \".setControlFontColor\", %this.object.getRoot(), %this.object @ \".setControlFontColor\", %this.object @ \".setControlFontColor\" );");
   
   if(%popup.object.contextBackColor)
         %backgroundColorPos = %popup.appendItem("Background Color..." TAB "" TAB "IngameContext.transparentControlCheck( %this.object, \"backColor\");");
         
   if(%popup.object.contextFillColor)
         %fillColorPos = %popup.appendItem("Fill Color..." TAB "" TAB "IngameContext.transparentControlCheck( %this.object, \"fillColor\");");
         
   if(%popup.object.contextFontSize)
         %fillColorPos = %popup.appendItem("Font Size..." TAB "" TAB "ChangeFontSize.loadDialog(%this.object);");

   if( %popup.object.windowSettings)
      %winSettingsPos = %popup.appendItem("Window Settings" TAB "" TAB "WindowSettings.loadDialog(%this.object);");
      
   if(%ctrl.isMemberOfClass( "GuiBitmapCtrl"))
   {
      if(%popup.object.setBitmap)
         %bitmapPos = %popup.appendItem("Set Bitmap..." TAB "" TAB "getLoadFilename( \"All Image Files|*.png;*.jpeg;*.jpg;*.tga;*.jng;*.mng;*.gif;*.bmp;*.dds|png|*.png|jpeg|*.jpeg|jpg|*.jpg|tga|*.tga|jng|*.jng|mng|*.mng|gif|*.gif|bmp|*.bmp|dds|*.dds\", %this.object @ \".setBitmap\", \"\" );");
   }
   
   if(%ctrl.isMemberOfClass( "GuiWindowCtrl"))
   {
      if(%popup.object.setTitle)
         %bitmapPos = %popup.appendItem("Set Title..." TAB "" TAB "setTitle.loadDialog(%this.object);");
   }
   
   %popup.showPopup( Canvas );
   
   
}

function IngameContext::loadXML()
{
   if( !isObject($XMLOutput) )
      new ReadXML($XMLOutput) { fileName = $XMLOutput @ ".xml"; };
   %output = $XMLOutput.readFile();
   if( %output )
      echo("Read successful.");
   else
      echo("Read error.");
}

function IngameContext::xmlOutput(%this, %ctrl)
{
      %currentObject = %ctrl.getRootControl();
      %saved = %currentObject.saveToXML( $XMLOutput, $XMLOutput @ ".xml");
      if( %saved )
         echo (" Saved " @ $XMLOutput @ ".xml" );
      else
         echo( "Saved failed" );
}

function IngameContext::transparentControlCheck( %this, %ctrl, %colorName )
{
   %transparent = %ctrl.transparentControlCheck();
   if( %transparent )
      return;
   
   if( %colorName $= "fillColor" )
      getColorI(  %ctrl.controlFillColor , %ctrl @ ".setControlFillColor", %ctrl.getRoot(), %ctrl @ ".setControlFillColor", %ctrl @ ".setControlFillColor" );
   else if( %colorName $= "backColor" )
      getColorI(  %ctrl.backgroundColor , %ctrl @ ".setControlBackgroundColor", %ctrl.getRoot(), %ctrl @ ".setControlBackgroundColor", %ctrl @ ".setControlBackgroundColor" );
}
