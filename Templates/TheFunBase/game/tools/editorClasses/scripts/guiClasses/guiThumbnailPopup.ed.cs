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

//-----------------------------------------------------------------------------
// ContextDialogContainer Class - Example Use
//-----------------------------------------------------------------------------
//
//%MyContextDialog = new ScriptObject() 
//{
//   class           = ContextDialogContainer;
//   superClass      = GuiThumbnailPopup;
//   dialog          = %myContainedDialog.getID();
//   thumbType       = "GuiThumbnailClass";
//   listType        = "StaticSpriteThumbs";
//};
// %MyContextDialog.show( %positionX, %positionY );
// %MyContextDialog.hide();
//
// NOTES
//
// - thumbType describes a script namespace that will be linked to the creation 
//  of the actual thumbs in the list.  This allows you to override their display
// - listType describes a script namespace that will be linked to the creation
//  of the list and will have refresh and destroy called on it when you need
//  to add objects to the list.  to add an object, call %this.AddObject on you
//  get a refresh call.
//
//
//function MyCallbackNamespace::onContextActivate( %this ) 
//{
//    echo("Dialog has been pushed onto canvas, clicking outside of it will pop it!");
//}
//function MyCallbackNamespace::onContextDeactivate( %this )
//{
//    echo("Dialog has lost it's context and has thus been popped from the canvas!");
//}
//
//
// Object Hierarchy
// [%scriptObject] ScriptObject (GuiThumbnailPopup)
//   .superClass (ContextDialogContainer)
// (%scriptObject)->[%dialogCtrl] 
//                 | GuiScrollCtrl [%scrollCtrl] (GuiThumbnailArray)
//                 | GuiDynamicCtrlArrayCtrl [%objectList] (GuiThumbnailCreator)
//                   .superClass ( listType )
//                   .thumbType = %thumbType
//                   .base = %this
// 
function GuiThumbnailPopup::CreateThumbPopup( %this, %parent, %thumbType, %label )
{
   %base = new GuiWindowCtrl()
    {
      profile = "ToolsGuiWindowProfile";
      horizSizing = "right";
      vertSizing = "bottom";
      position = "0 0";
      extent = "260 200";
      minExtent = "140 200";
      visible = "1";
      text = %label;
      maxLength = "255";
      resizeWidth = "1";
      resizeHeight = "1";
      canMove = "1";
      canClose = "0";
      canMinimize = "0";
      canMaximize = "0";
    };
   %scroll = new GuiScrollCtrl() 
   {
      canSaveDynamicFields = "0";
      Profile = "ToolsGuiScrollProfile";
      class = "GuiThumbnailArray";
      internalName = "thumbnailScroll";
      HorizSizing = "width";
      VertSizing = "height";
      Position = "5 13";
      Extent = "250 178";
      MinExtent = "8 2";
      canSave = "1";
      Visible = "1";
      hovertime = "1000";
      hScrollBar = "alwaysOff";
      vScrollBar = "alwaysOn";
      lockHorizScroll = "true";
      lockVertScroll = "false";
      constantThumbHeight = "1";
      Margin = "6 2";
      thumbType = %thumbType; // Special Tag - Class of thumbObject
   };
   %base.add(%scroll);
   %objectList = new GuiDynamicCtrlArrayControl() 
   {
            canSaveDynamicFields = "0";
            Profile = "ToolsGuiScrollProfile";
            class = %this.listType;
            superClass = "GuiThumbnailCreator";
            HorizSizing = "width";
            VertSizing = "height";
            Position = "0 0";
            Extent = "250 178";
            MinExtent = "64 64";
            canSave = "1";
            Visible = "1";
            internalName = "objectList";
            hovertime = "1000";
            colCount = "0";
            colSize = %this.thumbSizeX;
            rowSize = %this.thumbSizeY;
            rowSpacing = "2";
            colSpacing = "2";
            thumbType = %thumbType; // Special Tag - Class of thumbObject
            base = %this; // Special Tag - Link to base class for hiding of dlg
   };
   %scroll.add(%objectList);
   %parent.add(%base);
  
   return %base;
   
}

function GuiThumbnailPopup::onAdd(%this)
{
   // Call parent.
   if( !Parent::onAdd( %this ) )
      return false;
   
   if( %this.thumbType $= "" )
      %this.thumbType = "GuiDefaultThumbnail";
      
   %this.Dialog = %this.createThumbPopup( %this.base, %this.thumbType, %this.label );
         
   if( !isObject( %this.Dialog ) )
   {
      warn("GuiThumbnailPopup::onAdd - Invalid Context Dialog Specified!");
      return false;
   }
}



function GuiThumbnailArray::onRemove(%this)
{
   %this.destroy();      
}

function GuiThumbnailArray::onWake( %this )
{ 
   // Find objectList
   %objectList = %this.findObjectByInternalName("ObjectList");

   if( !isObject( %objectList ) )
      return;
   
   %objectList.refreshList();
}

function GuiThumbnailArray::refreshList(%this)
{
   // Find objectList
   %objectList = %this.findObjectByInternalName("ObjectList");

   if( !isObject( %objectList ) )
      return;
   
   // Parent will clear 
   %objectList.destroy();
   
}

function GuiThumbnailArray::destroy(%this)
{     
   // Find objectList
   %objectList = %this.findObjectByInternalName("ObjectList");

   if( !isObject( %objectList ) )
      return;

   while( %objectList.getCount() > 0 )
   {
      %object = %objectList.getObject( 0 );
      if( isObject( %object ) )
         %object.delete();
      else
         %objectList.remove( %object );
   }
}

//-----------------------------------------------------------------------------
// Add a T2D Object to the Object List
//-----------------------------------------------------------------------------
function GuiThumbnailCreator::AddObject( %this, %object, %data, %tooltip )
{
   // Add to group
   $LB::ObjectLibraryGroup.add( %object );
    
   // Build Object Container
   %container = new GuiControl() { profile = ToolsGuiButtonProfile; };
          
   // Add to list.
   %this.add( %container );
   
   // Return Container
   return %container;
}
