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

function initializeWorldEditor()
{
   echo(" % - Initializing World Editor");
   
   // Load GUI
   exec("./gui/profiles.ed.cs");
   exec("./scripts/cursors.ed.cs");

   exec("./gui/guiCreateNewTerrainGui.gui" );
   exec("./gui/GenericPromptDialog.ed.gui" );
   exec("./gui/guiTerrainImportGui.gui" );
   exec("./gui/guiTerrainExportGui.gui" );
   exec("./gui/EditorGui.ed.gui");
   exec("./gui/objectBuilderGui.ed.gui");
   exec("./gui/TerrainEditorVSettingsGui.ed.gui");
   exec("./gui/EditorChooseLevelGui.ed.gui");
   exec("./gui/VisibilityLayerWindow.ed.gui");
   exec("./gui/ManageBookmarksWindow.ed.gui");
   exec("./gui/ManageSFXParametersWindow.ed.gui" );
   exec("./gui/TimeAdjustGui.ed.gui");
   exec("./gui/AddFMODProjectDlg.ed.gui");
   exec("./gui/SelectObjectsWindow.ed.gui");
   exec("./gui/ProceduralTerrainPainterGui.gui" );
   exec("./gui/shadowViz.gui" );
   
   // Load Scripts.
   exec("./scripts/menus.ed.cs");
   exec("./scripts/menuHandlers.ed.cs");
   exec("./scripts/editor.ed.cs");
   exec("./scripts/editorInputCommands.cs");
   exec("./scripts/editor.keybinds.cs");
   exec("./scripts/undoManager.ed.cs");
   exec("./scripts/lighting.ed.cs");
   exec("./scripts/EditorGui.ed.cs");
   exec("./scripts/editorPrefs.ed.cs");
   exec("./scripts/editorRender.ed.cs");
   exec("./scripts/editorPlugin.ed.cs");
   exec("./scripts/EditorChooseLevelGui.ed.cs");
   exec("./scripts/visibilityLayer.ed.cs");
   exec("./scripts/cameraBookmarks.ed.cs");
   exec("./scripts/ManageSFXParametersWindow.ed.cs");
   exec("./scripts/AddFMODProjectDlg.ed.cs");
   exec("./scripts/SelectObjectsWindow.ed.cs");
   exec("./scripts/cameraCommands.ed.cs");
   exec("./scripts/lightViz.cs");
   exec("./scripts/shadowViz.cs");

   // Load Custom Editors
   loadDirectory(expandFilename("./scripts/editors"));
   loadDirectory(expandFilename("./scripts/interfaces"));
   
   // Create the default editor plugins before calling buildMenus.
      
   new ScriptObject( WorldEditorPlugin )
   {
      superClass = "EditorPlugin";
      editorGui = EWorldEditor;
   };
   
   // aka. The ObjectEditor.
   new ScriptObject( WorldEditorInspectorPlugin )
   {
      superClass = "WorldEditorPlugin";
      editorGui = EWorldEditor;
   };   
   
   new ScriptObject( TerrainEditorPlugin )
   {
      superClass = "EditorPlugin";
      editorGui = ETerrainEditor;
   };
   
   new ScriptObject( TerrainPainterPlugin )
   {
      superClass = "EditorPlugin";
      editorGui = ETerrainEditor;
   };
 
   new ScriptObject( MaterialEditorPlugin )
   {
      superClass = "WorldEditorPlugin";
      editorGui = EWorldEditor;
   };
   
   // Expose stock visibility/debug options.
   EVisibility.addOption( "Render: Zones", "$Zone::isRenderable", "" );
   EVisibility.addOption( "Render: Portals", "$Portal::isRenderable", "" );
   EVisibility.addOption( "Render: Occlusion Volumes", "$OcclusionVolume::isRenderable", "" );
   EVisibility.addOption( "Render: Triggers", "$Trigger::renderTriggers", "" );
   EVisibility.addOption( "Render: PhysicalZones", "$PhysicalZone::renderZones", "" );
   EVisibility.addOption( "Render: Sound Emitters", "$SFXEmitter::renderEmitters", "" );
   EVisibility.addOption( "Render: Mission Area", "EWorldEditor.renderMissionArea", "" );
   EVisibility.addOption( "Render: Sound Spaces", "$SFXSpace::isRenderable", "" );
   EVisibility.addOption( "Wireframe Mode", "$gfx::wireframe", "" );
   EVisibility.addOption( "Debug Render: Player Collision", "$Player::renderCollision", "" );   
   EVisibility.addOption( "Debug Render: Terrain", "TerrainBlock::debugRender", "" );
   EVisibility.addOption( "Debug Render: Decals", "$Decals::debugRender", "" );
   EVisibility.addOption( "Debug Render: Light Frustums", "$Light::renderLightFrustums", "" );
   EVisibility.addOption( "Debug Render: Bounding Boxes", "$Scene::renderBoundingBoxes", "" );
   EVisibility.addOption( "Debug Render: Physics World", "$PhysicsWorld::render", "togglePhysicsDebugViz" );
   EVisibility.addOption( "AL: Disable Shadows", "$Shadows::disable", "" );   
   EVisibility.addOption( "AL: Light Color Viz", "$AL_LightColorVisualizeVar", "toggleLightColorViz" );
   EVisibility.addOption( "AL: Light Specular Viz", "$AL_LightSpecularVisualizeVar", "toggleLightSpecularViz" );
   EVisibility.addOption( "AL: Normals Viz", "$AL_NormalsVisualizeVar", "toggleNormalsViz" );
   EVisibility.addOption( "AL: Depth Viz", "$AL_DepthVisualizeVar", "toggleDepthViz" );
   EVisibility.addOption( "AL: Color Buffer", "$AL_ColorBufferShaderVar", "toggleColorBufferViz" );
   EVisibility.addOption( "AL: Spec Map", "$AL_SpecMapShaderVar", "toggleSpecMapViz");
   EVisibility.addOption( "AL: Backbuffer", "$AL_BackbufferVisualizeVar", "toggleBackbufferViz" );
   EVisibility.addOption( "AL: Glow Buffer", "$AL_GlowVisualizeVar", "toggleGlowViz" );
   EVisibility.addOption( "AL: PSSM Cascade Viz", "$AL::PSSMDebugRender", "" );
   EVisibility.addOption( "Frustum Lock", "$Scene::lockCull", "" );
   EVisibility.addOption( "Disable Zone Culling", "$Scene::disableZoneCulling", "" );
   EVisibility.addOption( "Disable Terrain Occlusion", "$Scene::disableTerrainOcclusion", "" );

   EVisibility.addOption( "Colorblindness: Protanopia", "$CBV_Protanopia", "toggleColorBlindnessViz" );
   EVisibility.addOption( "Colorblindness: Protanomaly", "$CBV_Protanomaly", "toggleColorBlindnessViz" );
   EVisibility.addOption( "Colorblindness: Deuteranopia", "$CBV_Deuteranopia", "toggleColorBlindnessViz" );
   EVisibility.addOption( "Colorblindness: Deuteranomaly", "$CBV_Deuteranomaly", "toggleColorBlindnessViz" );
   EVisibility.addOption( "Colorblindness: Tritanopia", "$CBV_Tritanopia", "toggleColorBlindnessViz" );
   EVisibility.addOption( "Colorblindness: Tritanomaly", "$CBV_Tritanomaly", "toggleColorBlindnessViz" );
   EVisibility.addOption( "Colorblindness: Achromatopsia", "$CBV_Achromatopsia", "toggleColorBlindnessViz" );
   EVisibility.addOption( "Colorblindness: Achromatomaly", "$CBV_Achromatomaly", "toggleColorBlindnessViz" );
}

function destroyWorldEditor()
{
}
