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

// Provide default values for all World Editor settings.  These make use of
// the EditorSettings instance of the Settings class, as defined in the Tools
// package onStart().

EditorSettings.beginGroup( "WorldEditor", true );
EditorSettings.setDefaultValue(  "currentEditor",           "WorldEditorInspectorPlugin"  );
EditorSettings.setDefaultValue(  "dropType",                "screenCenter"    );
EditorSettings.setDefaultValue(  "undoLimit",               "40"              );
EditorSettings.setDefaultValue(  "forceLoadDAE",            "0"               );
EditorSettings.setDefaultValue(  "displayType",             $EditTsCtrl::DisplayTypePerspective );
EditorSettings.setDefaultValue(  "orthoFOV",                "50" );
EditorSettings.setDefaultValue(  "orthoShowGrid",           "1" );
EditorSettings.setDefaultValue(  "currentEditor",           "WorldEditorInspectorPlugin" );
EditorSettings.setDefaultValue(  "newLevelFile",            "tools/levels/BlankRoom.mis" );
EditorSettings.setDefaultValue(  "newGameObjectDir",        "scripts/server/gameObjects" );

if( isFile( "C:/Program Files/Torsion/Torsion.exe" ) )
   EditorSettings.setDefaultValue(  "torsionPath",          "C:/Program Files/Torsion/Torsion.exe" );
else if( isFile( "C:/Program Files (x86)/Torsion/Torsion.exe" ) )
   EditorSettings.setDefaultValue(  "torsionPath",          "C:/Program Files (x86)/Torsion/Torsion.exe" );
else
   EditorSettings.setDefaultValue(  "torsionPath",          "" );

EditorSettings.beginGroup( "ObjectIcons" );
EditorSettings.setDefaultValue(  "fadeIcons",               "1"               );
EditorSettings.setDefaultValue(  "fadeIconsStartDist",      "8"               );
EditorSettings.setDefaultValue(  "fadeIconsEndDist",        "20"              );
EditorSettings.setDefaultValue(  "fadeIconsStartAlpha",     "255"             );
EditorSettings.setDefaultValue(  "fadeIconsEndAlpha",       "0"               );
EditorSettings.endGroup();

EditorSettings.beginGroup( "Grid" );
EditorSettings.setDefaultValue(  "gridSize",                "1"               );
EditorSettings.setDefaultValue(  "gridSnap",                "0"               );
EditorSettings.setDefaultValue(  "gridColor",               "102 102 102 100" );
EditorSettings.setDefaultValue(  "gridOriginColor",         "255 255 255 100" );
EditorSettings.setDefaultValue(  "gridMinorColor",          "51 51 51 100"    );
EditorSettings.endGroup();

EditorSettings.beginGroup( "Tools" );
EditorSettings.setDefaultValue(  "snapGround",              "0"               );
EditorSettings.setDefaultValue(  "snapSoft",                "0"               );
EditorSettings.setDefaultValue(  "snapSoftSize",            "2.0"             );
EditorSettings.setDefaultValue(  "boundingBoxCollision",    "0"               );
EditorSettings.setDefaultValue(  "objectsUseBoxCenter",     "1"               );
EditorSettings.setDefaultValue(  "dropAtScreenCenterScalar","1.0"             );
EditorSettings.setDefaultValue(  "dropAtScreenCenterMax",   "100.0"           );
EditorSettings.endGroup();

EditorSettings.beginGroup( "Render" );
EditorSettings.setDefaultValue(  "renderObjHandle",         "1"               );
EditorSettings.setDefaultValue(  "renderObjText",           "1"               );
EditorSettings.setDefaultValue(  "renderPopupBackground",   "1"               );
EditorSettings.setDefaultValue(  "renderSelectionBox",      "1"               ); //<-- Does not currently render
EditorSettings.setDefaultValue(  "showMousePopupInfo",      "1"               );
//EditorSettings.setDefaultValue(  "visibleDistanceScale",    "1"               );
EditorSettings.endGroup();

EditorSettings.beginGroup( "Color" );
EditorSettings.setDefaultValue(  "dragRectColor",           "255 255 0 255"   );
EditorSettings.setDefaultValue(  "objectTextColor",         "255 255 255 255" );
EditorSettings.setDefaultValue(  "objMouseOverColor",       "0 255 0 255"     ); //<-- Currently ignored by editor (always white)
EditorSettings.setDefaultValue(  "objMouseOverSelectColor", "0 0 255 255"     ); //<-- Currently ignored by editor (always white)
EditorSettings.setDefaultValue(  "objSelectColor",          "255 0 0 255"     ); //<-- Currently ignored by editor (always white)
EditorSettings.setDefaultValue(  "popupBackgroundColor",    "100 100 100 255" );
EditorSettings.setDefaultValue(  "popupTextColor",          "255 255 0 255"   );
EditorSettings.setDefaultValue(  "raceSelectColor",         "0 0 100 100"     ); //<-- What is this used for?
EditorSettings.setDefaultValue(  "selectionBoxColor",       "255 255 0 255"   ); //<-- Does not currently render
EditorSettings.setDefaultValue(  "uvEditorHandleColor",     "1"               ); //<-- Index into color popup
EditorSettings.endGroup();

EditorSettings.beginGroup( "Images" );
EditorSettings.setDefaultValue(  "defaultHandle",           "tools/worldEditor/images/DefaultHandle" );
EditorSettings.setDefaultValue(  "lockedHandle",            "tools/worldEditor/images/LockedHandle"  );
EditorSettings.setDefaultValue(  "selectHandle",            "tools/worldEditor/images/SelectHandle"  );
EditorSettings.endGroup();

EditorSettings.beginGroup( "Docs" );
EditorSettings.setDefaultValue(  "documentationLocal",      "../../../Documentation/Official Documentation.html"  );
EditorSettings.setDefaultValue(  "documentationReference",  "../../../Documentation/Torque 3D - Script Manual.chm");
EditorSettings.setDefaultValue(  "documentationURL",        "http://www.garagegames.com/products/torque-3d/documentation/user"           );
EditorSettings.setDefaultValue(  "forumURL",                "http://www.garagegames.com/products/torque-3d/forums"      );
EditorSettings.endGroup();

EditorSettings.endGroup(); // WorldEditor

//-------------------------------------

// After setting up the default value, this field should be altered immediately
// after successfully using such functionality such as Open... or Save As...
EditorSettings.beginGroup( "LevelInformation" );
EditorSettings.setDefaultValue(  "levelsDirectory",         "levels"         );
EditorSettings.endGroup();

//-------------------------------------

EditorSettings.beginGroup( "AxisGizmo", true );

EditorSettings.setDefaultValue(  "axisGizmoMaxScreenLen",   "100"             ); //<-- What is this used for?
EditorSettings.setDefaultValue(  "rotationSnap",            "15"              ); //<-- Not currently used
EditorSettings.setDefaultValue(  "snapRotations",           "0"               ); //<-- Not currently used
EditorSettings.setDefaultValue(  "mouseRotateScalar",       "0.8"             );
EditorSettings.setDefaultValue(  "mouseScaleScalar",        "0.8"             );
EditorSettings.setDefaultValue(  "renderWhenUsed",          "0"               );
EditorSettings.setDefaultValue(  "renderInfoText",          "1"               );

EditorSettings.beginGroup( "Grid" );
EditorSettings.setDefaultValue(  "gridColor",               "255 255 255 20"  );
EditorSettings.setDefaultValue(  "gridSize",                "10 10 10"        );
EditorSettings.setDefaultValue(  "snapToGrid",              "0"               ); //<-- Not currently used
EditorSettings.setDefaultValue(  "renderPlane",             "0"               );
EditorSettings.setDefaultValue(  "renderPlaneHashes",       "0"               );
EditorSettings.setDefaultValue(  "planeDim",                "500"             );
EditorSettings.endGroup();

EditorSettings.endGroup();

//-------------------------------------

EditorSettings.beginGroup( "TerrainEditor", true );

EditorSettings.setDefaultValue(  "currentAction",           "raiseHeight"     );

EditorSettings.beginGroup( "Brush" );
EditorSettings.setDefaultValue(  "maxBrushSize",            "40 40"           );
EditorSettings.setDefaultValue(  "brushSize",               "1 1"             );
EditorSettings.setDefaultValue(  "brushType",               "box"             );
EditorSettings.setDefaultValue(  "brushPressure",           "1"               );
EditorSettings.setDefaultValue(  "brushSoftness",           "1"               );
EditorSettings.endGroup();

EditorSettings.beginGroup( "ActionValues" );
EditorSettings.setDefaultValue(  "adjustHeightVal",         "10"              );
EditorSettings.setDefaultValue(  "setHeightVal",            "100"             );
EditorSettings.setDefaultValue(  "scaleVal",                "1"               ); //<-- Tool not currently implemented
EditorSettings.setDefaultValue(  "smoothFactor",            "0.1"             );
EditorSettings.setDefaultValue(  "noiseFactor",             "1.0"             );
EditorSettings.setDefaultValue(  "softSelectRadius",        "50"              );
EditorSettings.setDefaultValue(  "softSelectFilter",        "1.000000 0.833333 0.666667 0.500000 0.333333 0.166667 0.000000" );
EditorSettings.setDefaultValue(  "softSelectDefaultFilter", "1.000000 0.833333 0.666667 0.500000 0.333333 0.166667 0.000000" );
EditorSettings.setDefaultValue(  "slopeMinAngle",           "0"               );
EditorSettings.setDefaultValue(  "slopeMaxAngle",           "90"              );
EditorSettings.endGroup();

EditorSettings.endGroup();

//-------------------------------------

EditorSettings.beginGroup( "TerrainPainter", true );
EditorSettings.endGroup();

//-------------------------------------

//TODO: this doesn't belong here
function setDefault( %name, %value )
{
   if( !isDefined( %name ) )
      eval( %name SPC "=" SPC "\"" @ %value @ "\";" );
}

setDefault( "$pref::WorldEditor::visibleDistanceScale", "1" ); // DAW: Keep this around for now as is used by EditTSCtrl

// JCF: Couldn't some or all of these be exposed 
// from WorldEditor::ConsoleInit via Con::AddVariable()
// and do away with this file?

function EditorGui::readWorldEditorSettings(%this)
{
   EditorSettings.beginGroup( "WorldEditor", true );
   EWorldEditor.dropType                  = EditorSettings.value( "dropType" );                   //$pref::WorldEditor::dropType;
   EWorldEditor.undoLimit                 = EditorSettings.value( "undoLimit" );                  //$pref::WorldEditor::undoLimit;
   EWorldEditor.forceLoadDAE              = EditorSettings.value( "forceLoadDAE" );               //$pref::WorldEditor::forceLoadDAE;
   %this.currentDisplayType               = EditorSettings.value( "displayType" );
   %this.currentOrthoFOV                  = EditorSettings.value( "orthoFOV" );
   EWorldEditor.renderOrthoGrid           = EditorSettings.value( "orthoShowGrid" );
   %this.currentEditor                    = EditorSettings.value( "currentEditor" );
   %this.torsionPath                      = EditorSettings.value( "torsionPath" );
   
   EditorSettings.beginGroup( "ObjectIcons" );
   EWorldEditor.fadeIcons                 = EditorSettings.value( "fadeIcons" );
   EWorldEditor.fadeIconsStartDist        = EditorSettings.value( "fadeIconsStartDist" );
   EWorldEditor.fadeIconsEndDist          = EditorSettings.value( "fadeIconsEndDist" );
   EWorldEditor.fadeIconsStartAlpha       = EditorSettings.value( "fadeIconsStartAlpha" );
   EWorldEditor.fadeIconsEndAlpha         = EditorSettings.value( "fadeIconsEndAlpha" );
   EditorSettings.endGroup();
   
   EditorSettings.beginGroup( "Grid" );
   EWorldEditor.gridSize                  = EditorSettings.value( "gridSize" );
   EWorldEditor.gridSnap                  = EditorSettings.value( "gridSnap" );
   EWorldEditor.gridColor                 = EditorSettings.value( "gridColor" );
   EWorldEditor.gridOriginColor           = EditorSettings.value( "gridOriginColor" );
   EWorldEditor.gridMinorColor            = EditorSettings.value( "gridMinorColor" );
   EditorSettings.endGroup();
   
   EditorSettings.beginGroup( "Tools" );
   EWorldEditor.stickToGround             = EditorSettings.value("snapGround");                 //$pref::WorldEditor::snapGround;
   EWorldEditor.setSoftSnap( EditorSettings.value("snapSoft") );                                //$pref::WorldEditor::snapSoft
   EWorldEditor.setSoftSnapSize( EditorSettings.value("snapSoftSize") );                        //$pref::WorldEditor::snapSoftSize
   EWorldEditor.boundingBoxCollision      = EditorSettings.value("boundingBoxCollision");       //$pref::WorldEditor::boundingBoxCollision;
   EWorldEditor.objectsUseBoxCenter       = EditorSettings.value("objectsUseBoxCenter");        //$pref::WorldEditor::objectsUseBoxCenter;
   EWorldEditor.dropAtScreenCenterScalar  = EditorSettings.value("dropAtScreenCenterScalar");
   EWorldEditor.dropAtScreenCenterMax     = EditorSettings.value("dropAtScreenCenterMax");
   EditorSettings.endGroup();

   EditorSettings.beginGroup( "Render" );
   EWorldEditor.renderObjHandle           = EditorSettings.value("renderObjHandle");            //$pref::WorldEditor::renderObjHandle;
   EWorldEditor.renderObjText             = EditorSettings.value("renderObjText");              //$pref::WorldEditor::renderObjText;
   EWorldEditor.renderPopupBackground     = EditorSettings.value("renderPopupBackground");      //$pref::WorldEditor::renderPopupBackground;
   EWorldEditor.renderSelectionBox        = EditorSettings.value("renderSelectionBox");         //$pref::WorldEditor::renderSelectionBox;   
   EWorldEditor.showMousePopupInfo        = EditorSettings.value("showMousePopupInfo");         //$pref::WorldEditor::showMousePopupInfo;   
   EditorSettings.endGroup();

   EditorSettings.beginGroup( "Color" );
   EWorldEditor.dragRectColor             = EditorSettings.value("dragRectColor");              //$pref::WorldEditor::dragRectColor;
   EWorldEditor.objectTextColor           = EditorSettings.value("objectTextColor");            //$pref::WorldEditor::objectTextColor;
   EWorldEditor.objMouseOverColor         = EditorSettings.value("objMouseOverColor");          //$pref::WorldEditor::objMouseOverColor;
   EWorldEditor.objMouseOverSelectColor   = EditorSettings.value("objMouseOverSelectColor");    //$pref::WorldEditor::objMouseOverSelectColor;
   EWorldEditor.objSelectColor            = EditorSettings.value("objSelectColor");             //$pref::WorldEditor::objSelectColor;
   EWorldEditor.popupBackgroundColor      = EditorSettings.value("popupBackgroundColor");       //$pref::WorldEditor::popupBackgroundColor;
   EWorldEditor.popupTextColor            = EditorSettings.value("popupTextColor");             //$pref::WorldEditor::popupTextColor;   
   EWorldEditor.selectionBoxColor         = EditorSettings.value("selectionBoxColor");          //$pref::WorldEditor::selectionBoxColor;
   EditorSettings.endGroup();

   EditorSettings.beginGroup( "Images" );
   EWorldEditor.defaultHandle             = EditorSettings.value("defaultHandle");              //$pref::WorldEditor::defaultHandle;
   EWorldEditor.lockedHandle              = EditorSettings.value("lockedHandle");               //$pref::WorldEditor::lockedHandle;
   EWorldEditor.selectHandle              = EditorSettings.value("selectHandle");               //$pref::WorldEditor::selectHandle;
   EditorSettings.endGroup();

   EditorSettings.beginGroup( "Docs" );
   EWorldEditor.documentationLocal        = EditorSettings.value( "documentationLocal" );
   EWorldEditor.documentationURL          = EditorSettings.value( "documentationURL" );
   EWorldEditor.documentationReference    = EditorSettings.value( "documentationReference" );
   EWorldEditor.forumURL                  = EditorSettings.value( "forumURL" );
   EditorSettings.endGroup();

   //EWorldEditor.planarMovement            = $pref::WorldEditor::planarMovement;   //<-- What is this used for?

   EditorSettings.endGroup(); // WorldEditor
   
   EditorSettings.beginGroup( "AxisGizmo", true );
   GlobalGizmoProfile.screenLength        = EditorSettings.value("axisGizmoMaxScreenLen");      //$pref::WorldEditor::axisGizmoMaxScreenLen;
   GlobalGizmoProfile.rotationSnap        = EditorSettings.value("rotationSnap");               //$pref::WorldEditor::rotationSnap;
   GlobalGizmoProfile.snapRotations       = EditorSettings.value("snapRotations");              //$pref::WorldEditor::snapRotations;
   GlobalGizmoProfile.rotateScalar        = EditorSettings.value("mouseRotateScalar");          //$pref::WorldEditor::mouseRotateScalar;
   GlobalGizmoProfile.scaleScalar         = EditorSettings.value("mouseScaleScalar");           //$pref::WorldEditor::mouseScaleScalar;
   GlobalGizmoProfile.renderWhenUsed      = EditorSettings.value("renderWhenUsed");
   GlobalGizmoProfile.renderInfoText      = EditorSettings.value("renderInfoText");

   EditorSettings.beginGroup( "Grid" );
   GlobalGizmoProfile.gridColor           = EditorSettings.value("gridColor");                  //$pref::WorldEditor::gridColor;
   GlobalGizmoProfile.gridSize            = EditorSettings.value("gridSize");                   //$pref::WorldEditor::gridSize;
   GlobalGizmoProfile.snapToGrid          = EditorSettings.value("snapToGrid");                 //$pref::WorldEditor::snapToGrid;
   GlobalGizmoProfile.renderPlane         = EditorSettings.value("renderPlane");                //$pref::WorldEditor::renderPlane;
   GlobalGizmoProfile.renderPlaneHashes   = EditorSettings.value("renderPlaneHashes");          //$pref::WorldEditor::renderPlaneHashes;   
   GlobalGizmoProfile.planeDim            = EditorSettings.value("planeDim");                   //$pref::WorldEditor::planeDim;
   EditorSettings.endGroup();

   EditorSettings.endGroup(); // AxisGizmo
}

function EditorGui::writeWorldEditorSettings(%this)
{
   EditorSettings.beginGroup( "WorldEditor", true );
   EditorSettings.setValue( "dropType",               EWorldEditor.dropType );               //$pref::WorldEditor::dropType
   EditorSettings.setValue( "undoLimit",              EWorldEditor.undoLimit );              //$pref::WorldEditor::undoLimit
   EditorSettings.setValue( "forceLoadDAE",           EWorldEditor.forceLoadDAE );           //$pref::WorldEditor::forceLoadDAE
   EditorSettings.setValue( "displayType",            %this.currentDisplayType );
   EditorSettings.setValue( "orthoFOV",               %this.currentOrthoFOV );
   EditorSettings.setValue( "orthoShowGrid",          EWorldEditor.renderOrthoGrid );
   EditorSettings.setValue( "currentEditor",          %this.currentEditor );
   EditorSettings.setvalue( "torsionPath",            %this.torsionPath );
   
   EditorSettings.beginGroup( "ObjectIcons" );
   EditorSettings.setValue( "fadeIcons",              EWorldEditor.fadeIcons );
   EditorSettings.setValue( "fadeIconsStartDist",     EWorldEditor.fadeIconsStartDist );
   EditorSettings.setValue( "fadeIconsEndDist",       EWorldEditor.fadeIconsEndDist );
   EditorSettings.setValue( "fadeIconsStartAlpha",    EWorldEditor.fadeIconsStartAlpha );
   EditorSettings.setValue( "fadeIconsEndAlpha",      EWorldEditor.fadeIconsEndAlpha );
   EditorSettings.endGroup();

   EditorSettings.beginGroup( "Grid" );
   EditorSettings.setValue( "gridSize",               EWorldEditor.gridSize );
   EditorSettings.setValue( "gridSnap",               EWorldEditor.gridSnap );
   EditorSettings.setValue( "gridColor",              EWorldEditor.gridColor );
   EditorSettings.setValue( "gridOriginColor",        EWorldEditor.gridOriginColor );
   EditorSettings.setValue( "gridMinorColor",         EWorldEditor.gridMinorColor );
   EditorSettings.endGroup();

   EditorSettings.beginGroup( "Tools" );
   EditorSettings.setValue( "snapGround",             EWorldEditor.stickToGround );          //$Pref::WorldEditor::snapGround
   EditorSettings.setValue( "snapSoft",               EWorldEditor.getSoftSnap() );          //$Pref::WorldEditor::snapSoft
   EditorSettings.setValue( "snapSoftSize",           EWorldEditor.getSoftSnapSize() );      //$Pref::WorldEditor::snapSoftSize
   EditorSettings.setValue( "boundingBoxCollision",   EWorldEditor.boundingBoxCollision );   //$Pref::WorldEditor::boundingBoxCollision
   EditorSettings.setValue( "objectsUseBoxCenter",    EWorldEditor.objectsUseBoxCenter );    //$Pref::WorldEditor::objectsUseBoxCenter
   EditorSettings.setValue( "dropAtScreenCenterScalar",  EWorldEditor.dropAtScreenCenterScalar );
   EditorSettings.setValue( "dropAtScreenCenterMax",  EWorldEditor.dropAtScreenCenterMax );
   EditorSettings.endGroup();

   EditorSettings.beginGroup( "Render" );
   EditorSettings.setValue( "renderObjHandle",        EWorldEditor.renderObjHandle );        //$Pref::WorldEditor::renderObjHandle
   EditorSettings.setValue( "renderObjText",          EWorldEditor.renderObjText );          //$Pref::WorldEditor::renderObjText
   EditorSettings.setValue( "renderPopupBackground",  EWorldEditor.renderPopupBackground );  //$Pref::WorldEditor::renderPopupBackground
   EditorSettings.setValue( "renderSelectionBox",     EWorldEditor.renderSelectionBox );     //$Pref::WorldEditor::renderSelectionBox
   EditorSettings.setValue( "showMousePopupInfo",     EWorldEditor.showMousePopupInfo );     //$Pref::WorldEditor::showMousePopupInfo
   EditorSettings.endGroup();

   EditorSettings.beginGroup( "Color" );
   EditorSettings.setValue( "dragRectColor",          EWorldEditor.dragRectColor );          //$Pref::WorldEditor::dragRectColor
   EditorSettings.setValue( "objectTextColor",        EWorldEditor.objectTextColor );        //$Pref::WorldEditor::objectTextColor
   EditorSettings.setValue( "objMouseOverColor",      EWorldEditor.objMouseOverColor );      //$Pref::WorldEditor::objMouseOverColor
   EditorSettings.setValue( "objMouseOverSelectColor",EWorldEditor.objMouseOverSelectColor );//$Pref::WorldEditor::objMouseOverSelectColor
   EditorSettings.setValue( "objSelectColor",         EWorldEditor.objSelectColor );         //$Pref::WorldEditor::objSelectColor
   EditorSettings.setValue( "popupBackgroundColor",   EWorldEditor.popupBackgroundColor );   //$Pref::WorldEditor::popupBackgroundColor
   EditorSettings.setValue( "selectionBoxColor",      EWorldEditor.selectionBoxColor );      //$Pref::WorldEditor::selectionBoxColor
   EditorSettings.endGroup();

   EditorSettings.beginGroup( "Images" );
   EditorSettings.setValue( "defaultHandle",          EWorldEditor.defaultHandle );          //$Pref::WorldEditor::defaultHandle
   EditorSettings.setValue( "selectHandle",           EWorldEditor.selectHandle );           //$Pref::WorldEditor::selectHandle
   EditorSettings.setValue( "lockedHandle",           EWorldEditor.lockedHandle );           //$Pref::WorldEditor::lockedHandle
   EditorSettings.endGroup();
   
   EditorSettings.beginGroup( "Docs" );
   EditorSettings.setValue(  "documentationLocal",    EWorldEditor.documentationLocal );
   EditorSettings.setValue(  "documentationReference",    EWorldEditor.documentationReference );
   EditorSettings.setValue(  "documentationURL",      EWorldEditor.documentationURL );
   EditorSettings.setValue(  "forumURL",              EWorldEditor.forumURL );
   EditorSettings.endGroup();

   EditorSettings.endGroup(); // WorldEditor
   
   EditorSettings.beginGroup( "AxisGizmo", true );

   EditorSettings.setValue( "axisGizmoMaxScreenLen",  GlobalGizmoProfile.screenLength );     //$Pref::WorldEditor::axisGizmoMaxScreenLen
   EditorSettings.setValue( "rotationSnap",           GlobalGizmoProfile.rotationSnap );     //$Pref::WorldEditor::rotationSnap
   EditorSettings.setValue( "snapRotations",          GlobalGizmoProfile.snapRotations );    //$Pref::WorldEditor::snapRotations
   EditorSettings.setValue( "mouseRotateScalar",      GlobalGizmoProfile.rotateScalar );     //$Pref::WorldEditor::mouseRotateScalar
   EditorSettings.setValue( "mouseScaleScalar",       GlobalGizmoProfile.scaleScalar );      //$Pref::WorldEditor::mouseScaleScalar
   EditorSettings.setValue( "renderWhenUsed",         GlobalGizmoProfile.renderWhenUsed );
   EditorSettings.setValue( "renderInfoText",         GlobalGizmoProfile.renderInfoText );

   EditorSettings.beginGroup( "Grid" );
   EditorSettings.setValue( "gridColor",              GlobalGizmoProfile.gridColor );        //$Pref::WorldEditor::gridColor
   EditorSettings.setValue( "gridSize",               GlobalGizmoProfile.gridSize );         //$Pref::WorldEditor::gridSize
   EditorSettings.setValue( "snapToGrid",             GlobalGizmoProfile.snapToGrid );       //$Pref::WorldEditor::snapToGrid
   EditorSettings.setValue( "renderPlane",            GlobalGizmoProfile.renderPlane );      //$Pref::WorldEditor::renderPlane
   EditorSettings.setValue( "renderPlaneHashes",      GlobalGizmoProfile.renderPlaneHashes );//$Pref::WorldEditor::renderPlaneHashes
   EditorSettings.setValue( "planeDim",               GlobalGizmoProfile.planeDim );         //$Pref::WorldEditor::planeDim
   EditorSettings.endGroup();

   EditorSettings.endGroup(); // AxisGizmo
}

function EditorGui::readTerrainEditorSettings(%this)
{
   EditorSettings.beginGroup( "TerrainEditor", true );

   ETerrainEditor.savedAction             = EditorSettings.value("currentAction");

   EditorSettings.beginGroup( "Brush" );
   ETerrainEditor.maxBrushSize = EditorSettings.value("maxBrushSize");
   ETerrainEditor.setBrushSize( EditorSettings.value("brushSize") );
   ETerrainEditor.setBrushType( EditorSettings.value("brushType") );
   ETerrainEditor.setBrushPressure( EditorSettings.value("brushPressure") );
   ETerrainEditor.setBrushSoftness( EditorSettings.value("brushSoftness") );
   EditorSettings.endGroup();

   EditorSettings.beginGroup( "ActionValues" );
   ETerrainEditor.adjustHeightVal         = EditorSettings.value("adjustHeightVal");
   ETerrainEditor.setHeightVal            = EditorSettings.value("setHeightVal");
   ETerrainEditor.scaleVal                = EditorSettings.value("scaleVal");
   ETerrainEditor.smoothFactor            = EditorSettings.value("smoothFactor");
   ETerrainEditor.noiseFactor             = EditorSettings.value("noiseFactor");
   ETerrainEditor.softSelectRadius        = EditorSettings.value("softSelectRadius");
   ETerrainEditor.softSelectFilter        = EditorSettings.value("softSelectFilter");
   ETerrainEditor.softSelectDefaultFilter = EditorSettings.value("softSelectDefaultFilter");
   ETerrainEditor.setSlopeLimitMinAngle( EditorSettings.value("slopeMinAngle") );
   ETerrainEditor.setSlopeLimitMaxAngle( EditorSettings.value("slopeMaxAngle") );
   EditorSettings.endGroup();

   EditorSettings.endGroup();
}

function EditorGui::writeTerrainEditorSettings(%this)
{
   EditorSettings.beginGroup( "TerrainEditor", true );
   
   EditorSettings.setValue( "currentAction",          ETerrainEditor.savedAction );

   EditorSettings.beginGroup( "Brush" );
   EditorSettings.setValue( "maxBrushSize",           ETerrainEditor.maxBrushSize );
   EditorSettings.setValue( "brushSize",              ETerrainEditor.getBrushSize() );
   EditorSettings.setValue( "brushType",              ETerrainEditor.getBrushType() );
   EditorSettings.setValue( "brushPressure",          ETerrainEditor.getBrushPressure() );
   EditorSettings.setValue( "brushSoftness",          ETerrainEditor.getBrushSoftness() );
   EditorSettings.endGroup();

   EditorSettings.beginGroup( "ActionValues" );
   EditorSettings.setValue( "adjustHeightVal",        ETerrainEditor.adjustHeightVal );
   EditorSettings.setValue( "setHeightVal",           ETerrainEditor.setHeightVal );
   EditorSettings.setValue( "scaleVal",               ETerrainEditor.scaleVal );
   EditorSettings.setValue( "smoothFactor",           ETerrainEditor.smoothFactor );
   EditorSettings.setValue( "noiseFactor",            ETerrainEditor.noiseFactor );
   EditorSettings.setValue( "softSelectRadius",       ETerrainEditor.softSelectRadius );
   EditorSettings.setValue( "softSelectFilter",       ETerrainEditor.softSelectFilter );
   EditorSettings.setValue( "softSelectDefaultFilter",ETerrainEditor.softSelectDefaultFilter );
   EditorSettings.setValue( "slopeMinAngle",          ETerrainEditor.getSlopeLimitMinAngle() );
   EditorSettings.setValue( "slopeMaxAngle",          ETerrainEditor.getSlopeLimitMaxAngle() );
   EditorSettings.endGroup();

   EditorSettings.endGroup();
}
