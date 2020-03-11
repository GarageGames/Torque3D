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

$MeshRoad::wireframe = true;
$MeshRoad::showSpline = true;
$MeshRoad::showReflectPlane = false;
$MeshRoad::showRoad = true;
$MeshRoad::breakAngle = 3.0;
   
function MeshRoadEditorGui::onWake( %this )
{   
   $MeshRoad::EditorOpen = true; 
   
   %count = EWorldEditor.getSelectionSize();
   for ( %i = 0; %i < %count; %i++ )
   {
      %obj = EWorldEditor.getSelectedObject(%i);
      if ( %obj.getClassName() !$= "MeshRoad" )
         EWorldEditor.unselectObject();
      else
         %this.setSelectedRoad( %obj );
   }      
   
   //%this-->TabBook.selectPage(0);
     
   %this.onNodeSelected(-1);
}

function MeshRoadEditorGui::onSleep( %this )
{
   $MeshRoad::EditorOpen = false;    
}

function MeshRoadEditorGui::paletteSync( %this, %mode )
{
   %evalShortcut = "ToolsPaletteArray-->" @ %mode @ ".setStateOn(1);";
   eval(%evalShortcut);
}   
function MeshRoadEditorGui::onEscapePressed( %this )
{
   if( %this.getMode() $= "MeshRoadEditorAddNodeMode" )
   {
      %this.prepSelectionMode();
      return true;
   }
   return false;
}
function MeshRoadEditorGui::onRoadSelected( %this, %road )
{
   %this.road = %road;
   
   // Update the materialEditorList
   if( isObject( %road ) )
      $Tools::materialEditorList = %road.getId();
   
   MeshRoadInspector.inspect( %road );  
   MeshRoadTreeView.buildVisibleTree(true);
   if( MeshRoadTreeView.getSelectedObject() != %road )
   {
      MeshRoadTreeView.clearSelection();
      %treeId = MeshRoadTreeView.findItemByObjectId( %road );
      MeshRoadTreeView.selectItem( %treeId );  
   }
}

function MeshRoadEditorGui::onNodeSelected( %this, %nodeIdx )
{
   if ( %nodeIdx == -1 )
   {
      MeshRoadEditorOptionsWindow-->position.setActive( false );
      MeshRoadEditorOptionsWindow-->position.setValue( "" );    
      
      MeshRoadEditorOptionsWindow-->rotation.setActive( false );
      MeshRoadEditorOptionsWindow-->rotation.setValue( "" );
      
      MeshRoadEditorOptionsWindow-->width.setActive( false );
      MeshRoadEditorOptionsWindow-->width.setValue( "" ); 
      
      MeshRoadEditorOptionsWindow-->depth.setActive( false );
      MeshRoadEditorOptionsWindow-->depth.setValue( "" );  
   }
   else
   {
      MeshRoadEditorOptionsWindow-->position.setActive( true );
      MeshRoadEditorOptionsWindow-->position.setValue( %this.getNodePosition() );    
      
      MeshRoadEditorOptionsWindow-->rotation.setActive( true );
      MeshRoadEditorOptionsWindow-->rotation.setValue( %this.getNodeNormal() );
      
      MeshRoadEditorOptionsWindow-->width.setActive( true );
      MeshRoadEditorOptionsWindow-->width.setValue( %this.getNodeWidth() ); 
      
      MeshRoadEditorOptionsWindow-->depth.setActive( true );
      MeshRoadEditorOptionsWindow-->depth.setValue( %this.getNodeDepth() );  
   }
}


function MeshRoadEditorGui::onNodeModified( %this, %nodeIdx )
{
   MeshRoadEditorOptionsWindow-->position.setValue( %this.getNodePosition() );    
   MeshRoadEditorOptionsWindow-->rotation.setValue( %this.getNodeNormal() );
   MeshRoadEditorOptionsWindow-->width.setValue( %this.getNodeWidth() ); 
   MeshRoadEditorOptionsWindow-->depth.setValue( %this.getNodeDepth() );   
}

function MeshRoadEditorGui::editNodeDetails( %this )
{
   
   %this.setNodePosition( MeshRoadEditorOptionsWindow-->position.getText() );
   %this.setNodeNormal( MeshRoadEditorOptionsWindow-->rotation.getText() );
   %this.setNodeWidth( MeshRoadEditorOptionsWindow-->width.getText() );
   %this.setNodeDepth( MeshRoadEditorOptionsWindow-->depth.getText() );
}

function MeshRoadEditorGui::onBrowseClicked( %this )
{
   //%filename = RETextureFileCtrl.getText();

   %dlg = new OpenFileDialog()
   {
      Filters        = "All Files (*.*)|*.*|";
      DefaultPath    = MeshRoadEditorGui.lastPath;
      DefaultFile    = %filename;
      ChangePath     = false;
      MustExist      = true;
   };
         
   %ret = %dlg.Execute();
   if(%ret)
   {
      MeshRoadEditorGui.lastPath = filePath( %dlg.FileName );
      %filename = %dlg.FileName;
      MeshRoadEditorGui.setTextureFile( %filename );
      MeshRoadEditorTextureFileCtrl.setText( %filename );
   }
   
   %dlg.delete();
}

function MeshRoadInspector::inspect( %this, %obj )
{
   %name = "";
   if ( isObject( %obj ) )
      %name = %obj.getName();   
   else
      MeshFieldInfoControl.setText( "" );
   
   //RiverInspectorNameEdit.setValue( %name );
   Parent::inspect( %this, %obj );  
}

function MeshRoadInspector::onInspectorFieldModified( %this, %object, %fieldName, %arrayIndex, %oldValue, %newValue )
{
   // Same work to do as for the regular WorldEditor Inspector.
   Inspector::onInspectorFieldModified( %this, %object, %fieldName, %arrayIndex, %oldValue, %newValue );   
}

function MeshRoadInspector::onFieldSelected( %this, %fieldName, %fieldTypeStr, %fieldDoc )
{
   MeshFieldInfoControl.setText( "<font:ArialBold:14>" @ %fieldName @ "<font:ArialItalic:14> (" @ %fieldTypeStr @ ") " NL "<font:Arial:14>" @ %fieldDoc );
}

function MeshRoadTreeView::onInspect(%this, %obj)
{
   MeshRoadInspector.inspect(%obj);   
}

function MeshRoadTreeView::onSelect(%this, %obj)
{
   MeshRoadEditorGui.road = %obj; 
   MeshRoadInspector.inspect( %obj );
   if(%obj != MeshRoadEditorGui.getSelectedRoad())
   {
      MeshRoadEditorGui.setSelectedRoad( %obj );
   }
}

function MeshRoadEditorGui::prepSelectionMode( %this )
{
   %mode = %this.getMode();
   
   if ( %mode $= "MeshRoadEditorAddNodeMode"  )
   {
      if ( isObject( %this.getSelectedRoad() ) )
         %this.deleteNode();
   }
   
   %this.setMode( "MeshRoadEditorSelectMode" );
   ToolsPaletteArray-->MeshRoadEditorSelectMode.setStateOn(1);
}

//------------------------------------------------------------------------------
function EMeshRoadEditorSelectModeBtn::onClick(%this)
{
   EditorGuiStatusBar.setInfo(%this.ToolTip);
}

function EMeshRoadEditorAddModeBtn::onClick(%this)
{
   EditorGuiStatusBar.setInfo(%this.ToolTip);
}

function EMeshRoadEditorMoveModeBtn::onClick(%this)
{
   EditorGuiStatusBar.setInfo(%this.ToolTip);
}

function EMeshRoadEditorRotateModeBtn::onClick(%this)
{
   EditorGuiStatusBar.setInfo(%this.ToolTip);
}

function EMeshRoadEditorScaleModeBtn::onClick(%this)
{
   EditorGuiStatusBar.setInfo(%this.ToolTip);
}

function EMeshRoadEditorInsertModeBtn::onClick(%this)
{
   EditorGuiStatusBar.setInfo(%this.ToolTip);
}

function EMeshRoadEditorRemoveModeBtn::onClick(%this)
{
   EditorGuiStatusBar.setInfo(%this.ToolTip);
}

function MeshRoadDefaultWidthSliderCtrlContainer::onWake(%this)
{
   MeshRoadDefaultWidthSliderCtrlContainer-->slider.setValue(MeshRoadDefaultWidthTextEditContainer-->textEdit.getText());
}

function MeshRoadDefaultDepthSliderCtrlContainer::onWake(%this)
{
   MeshRoadDefaultDepthSliderCtrlContainer-->slider.setValue(MeshRoadDefaultDepthTextEditContainer-->textEdit.getText());
}