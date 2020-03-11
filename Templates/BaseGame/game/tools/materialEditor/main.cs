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

// Material Editor Written by Dave Calabrese and Travis Vroman of Gaslight Studios

function initializeMaterialEditor()
{
   echo(" % - Initializing Material Editor");
   
   // Load Preview Window
   exec("~/materialEditor/gui/guiMaterialPreviewWindow.ed.gui");
   
   // Load Properties Window
   exec("~/materialEditor/gui/guiMaterialPropertiesWindow.ed.gui");
   
   // Load Client Scripts.
   exec("./scripts/materialEditor.ed.cs");
   exec("./scripts/materialEditorUndo.ed.cs");
   //exec("./gui/profiles.ed.cs");
   
   MaterialEditorPreviewWindow.setVisible( false );
   matEd_cubemapEditor.setVisible( false );
   matEd_addCubemapWindow.setVisible( false );
   MaterialEditorPropertiesWindow.setVisible( false );
   
   EditorGui.add( MaterialEditorPreviewWindow );
   EditorGui.add( matEd_cubemapEditor );
   EditorGui.add( matEd_addCubemapWindow );
   EditorGui.add( MaterialEditorPropertiesWindow );
}

function destroyMaterialEditor()
{
}

// Material Editor
function MaterialEditorPlugin::onWorldEditorStartup( %this )
{
   // Add ourselves to the window menu.
   %accel = EditorGui.addToEditorsMenu( "Material Editor", "", MaterialEditorPlugin );
   
   // Add ourselves to the ToolsToolbar
   %tooltip = "Material Editor (" @ %accel @ ")"; 
   EditorGui.addToToolsToolbar( "MaterialEditorPlugin", "MaterialEditorPalette", expandFilename("tools/worldEditor/images/toolbar/matterial-editor"), %tooltip );

   //connect editor windows
   GuiWindowCtrl::attach( MaterialEditorPropertiesWindow, MaterialEditorPreviewWindow);
   
   %map = new ActionMap();   
   %map.bindCmd( keyboard, "1", "EWorldEditorNoneModeBtn.performClick();", "" );  // Select
   %map.bindCmd( keyboard, "2", "EWorldEditorMoveModeBtn.performClick();", "" );  // Move
   %map.bindCmd( keyboard, "3", "EWorldEditorRotateModeBtn.performClick();", "" );  // Rotate
   %map.bindCmd( keyboard, "4", "EWorldEditorScaleModeBtn.performClick();", "" );  // Scale
   %map.bindCmd( keyboard, "f", "FitToSelectionBtn.performClick();", "" );// Fit Camera to Selection
   %map.bindCmd( keyboard, "z", "EditorGuiStatusBar.setCamera(\"Standard Camera\");", "" );// Free Camera
   %map.bindCmd( keyboard, "n", "ToggleNodeBar->renderHandleBtn.performClick();", "" );// Render Node
   %map.bindCmd( keyboard, "shift n", "ToggleNodeBar->renderTextBtn.performClick();", "" );// Render Node Text
   %map.bindCmd( keyboard, "alt s", "MaterialEditorGui.save();", "" );// Save Material
   //%map.bindCmd( keyboard, "delete", "ToggleNodeBar->renderTextBtn.performClick();", "" );// delete Material
   %map.bindCmd( keyboard, "g", "ESnapOptions-->GridSnapButton.performClick();" ); // Grid Snappping
   %map.bindCmd( keyboard, "t", "SnapToBar->objectSnapDownBtn.performClick();", "" );// Terrain Snapping
   %map.bindCmd( keyboard, "b", "SnapToBar-->objectSnapBtn.performClick();" ); // Soft Snappping
   %map.bindCmd( keyboard, "v", "EWorldEditorToolbar->boundingBoxColBtn.performClick();", "" );// Bounds Selection
   %map.bindCmd( keyboard, "o", "objectCenterDropdown->objectBoxBtn.performClick(); objectCenterDropdown.toggle();", "" );// Object Center
   %map.bindCmd( keyboard, "p", "objectCenterDropdown->objectBoundsBtn.performClick(); objectCenterDropdown.toggle();", "" );// Bounds Center
   %map.bindCmd( keyboard, "k", "objectTransformDropdown->objectTransformBtn.performClick(); objectTransformDropdown.toggle();", "" );// Object Transform
   %map.bindCmd( keyboard, "l", "objectTransformDropdown->worldTransformBtn.performClick(); objectTransformDropdown.toggle();", "" );// World Transform
   
   MaterialEditorPlugin.map = %map; 
   
   MaterialEditorGui.fileSpec = "Torque Material Files (materials.cs)|materials.cs|All Files (*.*)|*.*|";
   MaterialEditorGui.textureFormats = "Image Files (*.png, *.jpg, *.dds, *.bmp, *.gif, *.jng. *.tga)|*.png;*.jpg;*.dds;*.bmp;*.gif;*.jng;*.tga|All Files (*.*)|*.*|";
   MaterialEditorGui.modelFormats = "DTS Files (*.dts)|*.dts";
   MaterialEditorGui.lastTexturePath = "";   
   MaterialEditorGui.lastTextureFile = "";
   MaterialEditorGui.lastModelPath = "";
   MaterialEditorGui.lastModelFile = "";
   MaterialEditorGui.currentMaterial = "";
   MaterialEditorGui.lastMaterial = "";
   MaterialEditorGui.currentCubemap = "";
   MaterialEditorGui.currentObject = "";
   
   MaterialEditorGui.livePreview = "1";
   MaterialEditorGui.currentLayer = "0";
   MaterialEditorGui.currentMode = "Material";
   MaterialEditorGui.currentMeshMode = "EditorShape";
   
   new ArrayObject(UnlistedCubemaps);
   UnlistedCubemaps.add( "unlistedCubemaps", matEdCubeMapPreviewMat );
   UnlistedCubemaps.add( "unlistedCubemaps", WarnMatCubeMap );
   
   //MaterialEditor persistence manager
   new PersistenceManager(matEd_PersistMan);
}

function MaterialEditorPlugin::onActivated( %this )
{
   if($gfx::wireframe){
      $wasInWireFrameMode = true;   
      $gfx::wireframe = false;
   }else{
      $wasInWireFrameMode = false;  
   }
   advancedTextureMapsRollout.Expanded = false;
   materialAnimationPropertiesRollout.Expanded = false;
   materialAdvancedPropertiesRollout.Expanded = false;
   WorldEditorPlugin.onActivated();

   EditorGui-->MatEdPropertiesWindow.setVisible( true );
   EditorGui-->MatEdPreviewWindow.setVisible( true );
   EditorGui-->WorldEditorToolbar.setVisible( true );
   
   MaterialEditorGui.currentObject = $Tools::materialEditorList;
   // Execute the back end scripts that actually do the work.
   MaterialEditorGui.open();
   %this.map.push();
   
   Parent::onActivated(%this);
}

function MaterialEditorPlugin::onEditMenuSelect( %this, %editMenu )
{
   WorldEditorPlugin.onEditMenuSelect( %editMenu );
}

function MaterialEditorPlugin::onDeactivated( %this )
{
   if($wasInWireFrameMode)
      $gfx::wireframe = true;
      
   WorldEditorPlugin.onDeactivated();

   MaterialEditorGui.quit();
   
   EditorGui-->MatEdPropertiesWindow.setVisible( false );
   EditorGui-->MatEdPreviewWindow.setVisible( false );
   EditorGui-->WorldEditorToolbar.setVisible( false );
   %this.map.pop();
   
   Parent::onDeactivated(%this);
}