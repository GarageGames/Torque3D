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

function RoadEditorGui::onWake( %this )
{   
   $DecalRoad::EditorOpen = true;      
   
   %count = EWorldEditor.getSelectionSize();
   for ( %i = 0; %i < %count; %i++ )
   {
      %obj = EWorldEditor.getSelectedObject(%i);
      if ( %obj.getClassName() !$= "DecalRoad" )
         EWorldEditor.unselectObject();
      else
         %this.setSelectedRoad( %obj );
   }        
   
   %this.onNodeSelected(-1);
}

function RoadEditorGui::onSleep( %this )
{
   $DecalRoad::EditorOpen = false;   
}

function RoadEditorGui::paletteSync( %this, %mode )
{
   %evalShortcut = "ToolsPaletteArray-->" @ %mode @ ".setStateOn(1);";
   eval(%evalShortcut);
}  

function RoadEditorGui::onDeleteKey( %this )
{
   %road = %this.getSelectedRoad();
   %node = %this.getSelectedNode();
   
   if ( !isObject( %road ) )
      return;
      
   if ( %node != -1 )
   {
      %this.deleteNode();
   }
   else
   {
      MessageBoxOKCancel( "Notice", "Delete selected DecalRoad?", "RoadEditorGui.deleteRoad();", "" );
   }   
}

function RoadEditorGui::onEscapePressed( %this )
{
   if( %this.getMode() $= "RoadEditorAddNodeMode" )
   {
      %this.prepSelectionMode();
      return true;
   }
   return false;
}

//just in case we need it later
function RoadEditorGui::onRoadCreation( %this )
{
}

function RoadEditorGui::onRoadSelected( %this, %road )
{      
   %this.road = %road;
   
   // Update the materialEditorList
   if(isObject( %road ))
      $Tools::materialEditorList = %road.getId();
   
   RoadInspector.inspect( %road );  
   RoadTreeView.buildVisibleTree(true);
   if( RoadTreeView.getSelectedObject() != %road )
   {
      RoadTreeView.clearSelection();
      %treeId = RoadTreeView.findItemByObjectId( %road );
      RoadTreeView.selectItem( %treeId );  
   }
}

function RoadEditorGui::onNodeSelected( %this, %nodeIdx )
{
   
   if ( %nodeIdx == -1 )
   {
      RoadEditorProperties-->position.setActive( false );
      RoadEditorProperties-->position.setValue( "" );    
      
      RoadEditorProperties-->width.setActive( false );
      RoadEditorProperties-->width.setValue( "" );  
   }
   else
   {
      RoadEditorProperties-->position.setActive( true );
      RoadEditorProperties-->position.setValue( %this.getNodePosition() );    
      
      RoadEditorProperties-->width.setActive( true );
      RoadEditorProperties-->width.setValue( %this.getNodeWidth() );  
   }
   
}

function RoadEditorGui::onNodeModified( %this, %nodeIdx )
{
   
   RoadEditorProperties-->position.setValue( %this.getNodePosition() );    
   RoadEditorProperties-->width.setValue( %this.getNodeWidth() );  

}

function RoadEditorGui::editNodeDetails( %this )
{
   
   %this.setNodePosition( RoadEditorProperties-->position.getText() );
   %this.setNodeWidth( RoadEditorProperties-->width.getText() );
}

function RoadEditorGui::onBrowseClicked( %this )
{
   //%filename = RETextureFileCtrl.getText();
         
   %dlg = new OpenFileDialog()
   {
      Filters        = "All Files (*.*)|*.*|";
      DefaultPath    = RoadEditorGui.lastPath;
      DefaultFile    = %filename;
      ChangePath     = false;
      MustExist      = true;
   };
         
   %ret = %dlg.Execute();
   if(%ret)
   {
      RoadEditorGui.lastPath = filePath( %dlg.FileName );
      %filename = %dlg.FileName;
      RoadEditorGui.setTextureFile( %filename );
      RETextureFileCtrl.setText( %filename );
   }
   
   %dlg.delete();
}

function RoadInspector::inspect( %this, %obj )
{
   %name = "";
   if ( isObject( %obj ) )
      %name = %obj.getName();   
   else
      RoadFieldInfoControl.setText( "" );
   
   //RoadInspectorNameEdit.setValue( %name );
   Parent::inspect( %this, %obj );  
}

function RoadInspector::onInspectorFieldModified( %this, %object, %fieldName, %arrayIndex, %oldValue, %newValue )
{
   // Same work to do as for the regular WorldEditor Inspector.
   Inspector::onInspectorFieldModified( %this, %object, %fieldName, %arrayIndex, %oldValue, %newValue );   
}

function RoadInspector::onFieldSelected( %this, %fieldName, %fieldTypeStr, %fieldDoc )
{
   RoadFieldInfoControl.setText( "<font:ArialBold:14>" @ %fieldName @ "<font:ArialItalic:14> (" @ %fieldTypeStr @ ") " NL "<font:Arial:14>" @ %fieldDoc );
}

function RoadTreeView::onInspect(%this, %obj)
{
   RoadInspector.inspect(%obj);   
}

function RoadTreeView::onSelect(%this, %obj)
{
   RoadEditorGui.road = %obj; 
   RoadInspector.inspect( %obj );
   if(%obj != RoadEditorGui.getSelectedRoad())
   {
      RoadEditorGui.setSelectedRoad( %obj );
   }
}

function RoadEditorGui::prepSelectionMode( %this )
{
   %mode = %this.getMode();
   
   if ( %mode $= "RoadEditorAddNodeMode"  )
   {
      if ( isObject( %this.getSelectedRoad() ) )
         %this.deleteNode();
   }
   
   %this.setMode( "RoadEditorSelectMode" );
   ToolsPaletteArray-->RoadEditorSelectMode.setStateOn(1);
}
//------------------------------------------------------------------------------
function ERoadEditorSelectModeBtn::onClick(%this)
{
   EditorGuiStatusBar.setInfo(%this.ToolTip);
}

function ERoadEditorAddModeBtn::onClick(%this)
{
   EditorGuiStatusBar.setInfo(%this.ToolTip);
}

function ERoadEditorMoveModeBtn::onClick(%this)
{
   EditorGuiStatusBar.setInfo(%this.ToolTip);
}

function ERoadEditorScaleModeBtn::onClick(%this)
{
   EditorGuiStatusBar.setInfo(%this.ToolTip);
}

function ERoadEditorInsertModeBtn::onClick(%this)
{
   EditorGuiStatusBar.setInfo(%this.ToolTip);
}

function ERoadEditorRemoveModeBtn::onClick(%this)
{
   EditorGuiStatusBar.setInfo(%this.ToolTip);
}

function RoadDefaultWidthSliderCtrlContainer::onWake(%this)
{
   RoadDefaultWidthSliderCtrlContainer-->slider.setValue(RoadDefaultWidthTextEditContainer-->textEdit.getText());
}