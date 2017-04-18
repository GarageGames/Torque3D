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

/// The texture filename filter used with OpenFileDialog.
$TerrainEditor::TextureFileSpec = "Image Files (*.png, *.jpg, *.dds)|*.png;*.jpg;*.dds|All Files (*.*)|*.*|";

function TerrainEditor::init( %this )
{
   %this.attachTerrain();
   %this.setBrushSize( 9, 9 );

   new PersistenceManager( ETerrainPersistMan );
}

///
function EPainter_TerrainMaterialUpdateCallback( %mat, %matIndex )
{
   // Skip over a bad selection.
   if ( %matIndex == -1 || !isObject( %mat ) )
      return;

   // Update the material and the UI.
   ETerrainEditor.updateMaterial( %matIndex, %mat.getInternalName() );
   EPainter.setup( %matIndex );
}

function EPainter_TerrainMaterialAddCallback( %mat, %matIndex )
{
   // Ignore bad materials.
   if ( !isObject( %mat ) )
      return;

   // Add it and update the UI.
   ETerrainEditor.addMaterial( %mat.getInternalName() );
   EPainter.setup( %matIndex );
}

function TerrainEditor::setPaintMaterial( %this, %matIndex, %terrainMat )
{
   assert( isObject( %terrainMat ), "TerrainEditor::setPaintMaterial - Got bad material!" );

   ETerrainEditor.paintIndex = %matIndex;
   ETerrainMaterialSelected.selectedMatIndex = %matIndex;
   ETerrainMaterialSelected.selectedMat = %terrainMat;
   ETerrainMaterialSelected.bitmap = %terrainMat.diffuseMap;
   ETerrainMaterialSelectedEdit.Visible = isObject(%terrainMat);
   TerrainTextureText.text = %terrainMat.getInternalName();
   ProceduralTerrainPainterDescription.text = "Generate "@ %terrainMat.getInternalName() @" layer";
}

function TerrainEditor::setup( %this )
{
   %action = %this.savedAction;
   %desc = %this.savedActionDesc;
   if ( %this.savedAction $= "" )
   {
      %action = brushAdjustHeight;
   }

   %this.switchAction( %action );
}

function EPainter::updateLayers( %this, %matIndex )
{
   // Default to whatever was selected before.
   if ( %matIndex $= "" )
      %matIndex = ETerrainEditor.paintIndex;

   // The material string is a newline seperated string of
   // TerrainMaterial internal names which we can use to find
   // the actual material data in TerrainMaterialSet.

   %mats = ETerrainEditor.getMaterials();

   %matList = %this-->theMaterialList;
   %matList.deleteAllObjects();
   %listWidth = getWord( %matList.getExtent(), 0 );

   for( %i = 0; %i < getRecordCount( %mats ); %i++ )
   {
      %matInternalName = getRecord( %mats, %i );
      %mat = TerrainMaterialSet.findObjectByInternalName( %matInternalName );

      // Is there no material info for this slot?
      if ( !isObject( %mat ) )
         continue;

      %index = %matList.getCount();
      %command = "ETerrainEditor.setPaintMaterial( " @ %index @ ", " @ %mat @ " );";
      %altCommand = "TerrainMaterialDlg.show( " @ %index @ ", " @ %mat @ ", EPainter_TerrainMaterialUpdateCallback );";

      %ctrl = new GuiIconButtonCtrl()
      {
         class = "EPainterIconBtn";
         internalName = "EPainterMaterialButton" @ %i;
         profile = "GuiCreatorIconButtonProfile";
         iconLocation = "Left";
         textLocation = "Right";
         extent = %listWidth SPC "46";
         textMargin = 5;
         buttonMargin = "4 4";
         buttonType = "RadioButton";
         sizeIconToButton = true;
         makeIconSquare = true;
         tooltipprofile = "ToolsGuiToolTipProfile";
         command = %command;
         altCommand = %altCommand;
         useMouseEvents = true;

         new GuiBitmapButtonCtrl()
         {
            bitmap = "tools/gui/images/delete";
            buttonType = "PushButton";
            HorizSizing = "left";
            VertSizing = "bottom";
            position = ( %listwidth - 20 ) SPC "26";
            Extent = "17 17";
            command = "EPainter.showMaterialDeleteDlg( " @ %matInternalName @ " );";
         };
      };

      %ctrl.setText( %matInternalName );
      %ctrl.setBitmap( %mat.diffuseMap );

      %tooltip = %matInternalName;
      if(%i < 9)
         %tooltip = %tooltip @ " (" @ (%i+1) @ ")";
      else if(%i == 9)
         %tooltip = %tooltip @ " (0)";
      %ctrl.tooltip = %tooltip;

      %matList.add( %ctrl );
   }

   %matCount = %matList.getCount();

   // Add one more layer as the 'add new' layer.
   %ctrl = new GuiIconButtonCtrl()
   {
      profile = "GuiCreatorIconButtonProfile";
      iconBitmap = "~/worldEditor/images/terrainpainter/new_layer_icon";
      iconLocation = "Left";
      textLocation = "Right";
      extent = %listWidth SPC "46";
      textMargin = 5;
      buttonMargin = "4 4";
      buttonType = "PushButton";
      sizeIconToButton = true;
      makeIconSquare = true;
      tooltipprofile = "ToolsGuiToolTipProfile";
      text = "New Layer";
      tooltip = "New Layer";
      command = "TerrainMaterialDlg.show( " @ %matCount @ ", 0, EPainter_TerrainMaterialAddCallback );";
   };
   %matList.add( %ctrl );

   // Make sure our selection is valid and that we're
   // not selecting the 'New Layer' button.

   if( %matIndex < 0 )
      return;
   if( %matIndex >= %matCount )
      %matIndex = 0;

   // To make things simple... click the paint material button to
   // active it and initialize other state.
   %ctrl = %matList.getObject( %matIndex );
   %ctrl.performClick();
}

function EPainter::showMaterialDeleteDlg( %this, %matInternalName )
{
   MessageBoxYesNo( "Confirmation",
      "Really remove material '" @ %matInternalName @ "' from the terrain?",
      %this @ ".removeMaterial( " @ %matInternalName @ " );", "" );
}

function EPainter::removeMaterial( %this, %matInternalName )
{
   %selIndex = ETerrainEditor.paintIndex - 1;

   // Remove the material from the terrain.

   %index = ETerrainEditor.getMaterialIndex( %matInternalName );
   if( %index != -1 )
      ETerrainEditor.removeMaterial( %index );

   // Update the material list.

   %this.updateLayers( %selIndex );
}

function EPainter::setup( %this, %matIndex )
{
   // Update the layer listing.
   %this.updateLayers( %matIndex );

   // Automagically put us into material paint mode.
   ETerrainEditor.currentMode = "paint";
   ETerrainEditor.selectionHidden = true;
   ETerrainEditor.currentAction = paintMaterial;
   ETerrainEditor.currentActionDesc = "Paint material on terrain";
   ETerrainEditor.setAction( ETerrainEditor.currentAction );
   EditorGuiStatusBar.setInfo(ETerrainEditor.currentActionDesc);
   ETerrainEditor.renderVertexSelection = true;
}

function onNeedRelight()
{
   if( RelightMessage.visible == false )
      RelightMessage.visible = true;
}

function TerrainEditor::onGuiUpdate(%this, %text)
{
   %minHeight = getWord(%text, 1);
   %avgHeight = getWord(%text, 2);
   %maxHeight = getWord(%text, 3);

   %mouseBrushInfo = " (Mouse) #: " @ getWord(%text, 0) @ "  avg: " @ %avgHeight @ " " @ ETerrainEditor.currentAction;
   %selectionInfo = "     (Selected) #: " @ getWord(%text, 4) @ "  avg: " @ getWord(%text, 5);

   TEMouseBrushInfo.setValue(%mouseBrushInfo);
   TEMouseBrushInfo1.setValue(%mouseBrushInfo);
   TESelectionInfo.setValue(%selectionInfo);
   TESelectionInfo1.setValue(%selectionInfo);

   EditorGuiStatusBar.setSelection("min: " @ %minHeight @ "  avg: " @ %avgHeight @ "  max: " @ %maxHeight);
}

function TerrainEditor::onBrushChanged( %this )
{
   EditorGui.currentEditor.syncBrushInfo();
}

function TerrainEditor::toggleBrushType( %this, %brush )
{
   %this.setBrushType( %brush.internalName );
}

function TerrainEditor::offsetBrush(%this, %x, %y)
{
   %curPos = %this.getBrushPos();
   %this.setBrushPos(getWord(%curPos, 0) + %x, getWord(%curPos, 1) + %y);
}

function TerrainEditor::onActiveTerrainChange(%this, %newTerrain)
{
   // Need to refresh the terrain painter.
   if ( EditorGui.currentEditor.getId() == TerrainPainterPlugin.getId() )
      EPainter.setup(ETerrainEditor.paintIndex);
}

function TerrainEditor::getActionDescription( %this, %action )
{
   switch$( %action )
   {
      case "brushAdjustHeight":
         return "Adjust terrain height up or down.";

      case "raiseHeight":
         return "Raise terrain height.";

      case "lowerHeight":
         return "Lower terrain height.";

      case "smoothHeight":
         return "Smooth terrain.";

      case "paintNoise":
         return "Modify terrain height using noise.";

      case "flattenHeight":
         return "Flatten terrain.";

      case "setHeight":
         return "Set terrain height to defined value.";

      case "setEmpty":
         return "Remove terrain collision.";

      case "clearEmpty":
         return "Add back terrain collision.";

      default:
         return "";
   }
}

/// This is only ment for terrain editing actions and not
/// processed actions or the terrain material painting action.
function TerrainEditor::switchAction( %this, %action )
{
   %actionDesc = %this.getActionDescription(%action);

   %this.currentMode = "paint";
   %this.selectionHidden = true;
   %this.currentAction = %action;
   %this.currentActionDesc = %actionDesc;
   %this.savedAction = %action;
   %this.savedActionDesc = %actionDesc;

   if (  %action $= "setEmpty" ||
         %action $= "clearEmpty" ||
          %action $= "setHeight" )
      %this.renderSolidBrush = true;
   else
      %this.renderSolidBrush = false;

   EditorGuiStatusBar.setInfo(%actionDesc);

   %this.setAction( %this.currentAction );
}

function TerrainEditor::onSmoothHeightmap( %this )
{
   if ( !%this.getActiveTerrain() )
      return;

   // Show the dialog first and let the user
   // set the smoothing parameters.



   // Now create the terrain smoothing action to
   // get the work done and perform later undos.
   %action = new TerrainSmoothAction();
   %action.smooth( %this.getActiveTerrain(), 1.0, 1 );
   %action.addToManager( Editor.getUndoManager() );
}

function TerrainEditor::onMaterialUndo( %this )
{
   // Update the gui to reflect the current materials.
   EPainter.updateLayers();
}

//------------------------------------------------------------------------------
// Functions
//------------------------------------------------------------------------------

function TerrainEditorSettingsGui::onWake(%this)
{
   TESoftSelectFilter.setValue(ETerrainEditor.softSelectFilter);
}

function TerrainEditorSettingsGui::onSleep(%this)
{
   ETerrainEditor.softSelectFilter = TESoftSelectFilter.getValue();
}

function TESettingsApplyButton::onAction(%this)
{
   ETerrainEditor.softSelectFilter = TESoftSelectFilter.getValue();
   ETerrainEditor.resetSelWeights(true);
   ETerrainEditor.processAction("softSelect");
}

function getPrefSetting(%pref, %default)
{
   //
   if(%pref $= "")
      return(%default);
   else
      return(%pref);
}

function TerrainEditorPlugin::setEditorFunction(%this)
{
   %terrainExists = parseMissionGroup( "TerrainBlock" );

   if( %terrainExists == false )
      MessageBoxYesNoCancel("No Terrain","Would you like to create a New Terrain?", "Canvas.pushDialog(CreateNewTerrainGui);");

   return %terrainExists;
}

function TerrainPainterPlugin::setEditorFunction(%this, %overrideGroup)
{
   %terrainExists = parseMissionGroup( "TerrainBlock" );

   if( %terrainExists == false )
      MessageBoxYesNoCancel("No Terrain","Would you like to create a New Terrain?", "Canvas.pushDialog(CreateNewTerrainGui);");

   return %terrainExists;
}

function EPainterIconBtn::onMouseDragged( %this )
{
   %payload = new GuiControl()
   {
      profile = GuiCreatorIconButtonProfile;
      position = "0 0";
      extent = %this.extent.x SPC "5";
      dragSourceControl = %this;
   };

   %xOffset = getWord( %payload.extent, 0 ) / 2;
   %yOffset = getWord( %payload.extent, 1 ) / 2;
   %cursorpos = Canvas.getCursorPos();
   %xPos = getWord( %cursorpos, 0 ) - %xOffset;
   %yPos = getWord( %cursorpos, 1 ) - %yOffset;

   // Create the drag control.

   %ctrl = new GuiDragAndDropControl()
   {
      canSaveDynamicFields    = "0";
      Profile                 = EPainterDragDropProfile;
      HorizSizing             = "right";
      VertSizing              = "bottom";
      Position                = %xPos SPC %yPos;
      extent                  = %payload.extent;
      MinExtent               = "4 4";
      canSave                 = "1";
      Visible                 = "1";
      hovertime               = "1000";
      deleteOnMouseUp         = true;
   };

   %ctrl.add( %payload );

   Canvas.getContent().add( %ctrl );
   %ctrl.startDragging( %xOffset, %yOffset );
}

function EPainterIconBtn::onControlDragged( %this, %payload )
{
   %payload.getParent().position = %this.getGlobalPosition();
}

function EPainterIconBtn::onControlDropped( %this, %payload )
{
   %srcBtn = %payload.dragSourceControl;
   %dstBtn = %this;
   %stack = %this.getParent();

   // Not dropped on a valid Button.
   // Really this shouldnt happen since we are in a callback on our specialized
   // EPainterIconBtn namespace.
   if ( %stack != %dstBtn.getParent() || %stack != EPainterStack.getId() )
   {
      echo( "Not dropped on valid control" );
      return;
   }

   // Dropped on the original control, no order change.
   // Simulate a click on the control, instead of a drag/drop.
   if ( %srcBtn == %dstBtn )
   {
      %dstBtn.performClick();
      return;
   }

   %dstIndex = %stack.getObjectIndex( %dstBtn );
   ETerrainEditor.reorderMaterial( %stack.getObjectIndex( %srcBtn ), %dstIndex );

   // select the button/material we just reordered.
   %stack.getObject( %dstIndex ).performClick();
}