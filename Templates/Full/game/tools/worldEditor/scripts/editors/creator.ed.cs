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


function EWCreatorWindow::init( %this )
{
   // Just so we can recall this method for testing changes
   // without restarting.
   if ( isObject( %this.array ) )
      %this.array.delete();
      
   %this.array = new ArrayObject();
   %this.array.caseSensitive = true; 
   %this.setListView( true );
   
   %this.beginGroup( "Environment" );
   
      // Removed Prefab as there doesn't really seem to be a point in creating a blank one
      //%this.registerMissionObject( "Prefab",              "Prefab" );
      %this.registerMissionObject( "SkyBox",              "Sky Box" );
      %this.registerMissionObject( "CloudLayer",          "Cloud Layer" );
      %this.registerMissionObject( "BasicClouds",         "Basic Clouds" );
      %this.registerMissionObject( "ScatterSky",          "Scatter Sky" );
      %this.registerMissionObject( "Sun",                 "Basic Sun" );
      %this.registerMissionObject( "Lightning" );
      %this.registerMissionObject( "WaterBlock",          "Water Block" );
      %this.registerMissionObject( "SFXEmitter",          "Sound Emitter" );
      %this.registerMissionObject( "Precipitation" );
      %this.registerMissionObject( "ParticleEmitterNode", "Particle Emitter" );
      %this.registerMissionObject( "VolumetricFog", "Volumetric Fog" );
      %this.registerMissionObject( "RibbonNode", "Ribbon" );
      
      // Legacy features. Users should use Ground Cover and the Forest Editor.   
      //%this.registerMissionObject( "fxShapeReplicator",   "Shape Replicator" );
      //%this.registerMissionObject( "fxFoliageReplicator", "Foliage Replicator" );
      
      %this.registerMissionObject( "PointLight",          "Point Light" );
      %this.registerMissionObject( "SpotLight",           "Spot Light" );
      %this.registerMissionObject( "GroundCover",         "Ground Cover" );
      %this.registerMissionObject( "TerrainBlock",        "Terrain Block" );
      %this.registerMissionObject( "GroundPlane",         "Ground Plane" );
      %this.registerMissionObject( "WaterPlane",          "Water Plane" );
      %this.registerMissionObject( "PxCloth",             "Cloth" );
      %this.registerMissionObject( "ForestWindEmitter",   "Wind Emitter" );
               
      %this.registerMissionObject( "DustEmitter", "Dust Emitter" );
      %this.registerMissionObject( "DustSimulation", "Dust Simulation" );
      %this.registerMissionObject( "DustEffecter", "Dust Effecter" );
      
   %this.endGroup();

   %this.beginGroup( "Level" );
   
      %this.registerMissionObject( "MissionArea",  "Mission Area" );
      %this.registerMissionObject( "Path" );
      %this.registerMissionObject( "Marker",       "Path Node" );
      %this.registerMissionObject( "Trigger" );
      %this.registerMissionObject( "PhysicalZone", "Physical Zone" );
      %this.registerMissionObject( "Camera" );
      %this.registerMissionObject( "LevelInfo",    "Level Info" );
      %this.registerMissionObject( "TimeOfDay",    "Time of Day" );
      %this.registerMissionObject( "Zone",         "Zone" );
      %this.registerMissionObject( "Portal",       "Zone Portal" );
      %this.registerMissionObject( "SpawnSphere",  "Player Spawn Sphere", "PlayerDropPoint" );
      %this.registerMissionObject( "SpawnSphere",  "Observer Spawn Sphere", "ObserverDropPoint" );
      %this.registerMissionObject( "SFXSpace",      "Sound Space" );
      %this.registerMissionObject( "OcclusionVolume", "Occlusion Volume" );
      %this.registerMissionObject( "AccumulationVolume", "Accumulation Volume" );
      
   %this.endGroup();
   
   %this.beginGroup( "System" );
   
      %this.registerMissionObject( "SimGroup" );
      
   %this.endGroup();  

   %this.beginGroup( "ExampleObjects" );
   
      %this.registerMissionObject( "RenderObjectExample" );
      %this.registerMissionObject( "RenderMeshExample" );
      %this.registerMissionObject( "RenderShapeExample" );
      
   %this.endGroup(); 
}

function EWCreatorWindow::onWake( %this )
{
   CreatorTabBook.selectPage( 0 );
   CreatorTabBook.onTabSelected( "Scripted" );
}

function EWCreatorWindow::beginGroup( %this, %group )
{
   %this.currentGroup = %group;   
}

function EWCreatorWindow::endGroup( %this, %group )
{
   %this.currentGroup = "";
}

function EWCreatorWindow::getCreateObjectPosition()
{
   %focusPoint = LocalClientConnection.getControlObject().getLookAtPoint();
   if( %focusPoint $= "" )
      return "0 0 0";
   else
      return getWord( %focusPoint, 1 ) SPC getWord( %focusPoint, 2 ) SPC getWord( %focusPoint, 3 );
}

function EWCreatorWindow::registerMissionObject( %this, %class, %name, %buildfunc, %group )
{
   if( !isClass(%class) )
      return;
      
   if ( %name $= "" )
      %name = %class;
   if ( %this.currentGroup !$= "" && %group $= "" )
      %group = %this.currentGroup;
   
   if ( %class $= "" || %group $= "" )
   {
      warn( "EWCreatorWindow::registerMissionObject, invalid parameters!" );
      return;  
   }

   %args = new ScriptObject();
   %args.val[0] = %class;
   %args.val[1] = %name;
   %args.val[2] = %buildfunc;
   
   %this.array.push_back( %group, %args );
}

function EWCreatorWindow::getNewObjectGroup( %this )
{
   return %this.objectGroup;
}

function EWCreatorWindow::setNewObjectGroup( %this, %group )
{
   if( %this.objectGroup )
   {
      %oldItemId = EditorTree.findItemByObjectId( %this.objectGroup );
      if( %oldItemId > 0 )
         EditorTree.markItem( %oldItemId, false );
   }

   %group = %group.getID();
   %this.objectGroup = %group;
   %itemId = EditorTree.findItemByObjectId( %group );
   EditorTree.markItem( %itemId );
}

function EWCreatorWindow::createStatic( %this, %file )
{
   if ( !$missionRunning )
      return;

   if( !isObject(%this.objectGroup) )
      %this.setNewObjectGroup( MissionGroup );

   %objId = new TSStatic()
   {
      shapeName = %file;
      position = %this.getCreateObjectPosition();
      parentGroup = %this.objectGroup;
   };
   
   %this.onObjectCreated( %objId );
}

function EWCreatorWindow::createPrefab( %this, %file )
{
   if ( !$missionRunning )
      return;

   if( !isObject(%this.objectGroup) )
      %this.setNewObjectGroup( MissionGroup );

   %objId = new Prefab()
   {
      filename = %file;
      position = %this.getCreateObjectPosition();
      parentGroup = %this.objectGroup;
   };
   
   %this.onObjectCreated( %objId );
}

function EWCreatorWindow::createObject( %this, %cmd )
{
   if ( !$missionRunning )
      return;

   if( !isObject(%this.objectGroup) )
      %this.setNewObjectGroup( MissionGroup );

   pushInstantGroup();
   %objId = eval(%cmd);
   popInstantGroup();
   
   if( isObject( %objId ) )
      %this.onFinishCreateObject( %objId );
      
   return %objId;
}

function EWCreatorWindow::onFinishCreateObject( %this, %objId )
{
   %this.objectGroup.add( %objId );

   if( %objId.isMemberOfClass( "SceneObject" ) )
   {
      %objId.position = %this.getCreateObjectPosition();

      //flush new position
      %objId.setTransform( %objId.getTransform() );
   }

   %this.onObjectCreated( %objId );
}

function EWCreatorWindow::onObjectCreated( %this, %objId )
{
   // Can we submit an undo action?
   if ( isObject( %objId ) )
      MECreateUndoAction::submit( %objId );
            
   EditorTree.clearSelection();
   EWorldEditor.clearSelection();      
   EWorldEditor.selectObject( %objId );
   
   // When we drop the selection don't store undo
   // state for it... the creation deals with it.
   EWorldEditor.dropSelection( true );
}

function CreatorTabBook::onTabSelected( %this, %text, %idx )
{
   if ( %this.isAwake() )
   {
      EWCreatorWindow.tab = %text;      
      EWCreatorWindow.navigate( "" );
   }
}

function EWCreatorWindow::navigate( %this, %address )
{
   CreatorIconArray.frozen = true;
   CreatorIconArray.clear();  
   CreatorPopupMenu.clear();       
        
   if ( %this.tab $= "Scripted" )
   {
      %category = getWord( %address, 1 );                  
      %dataGroup = "DataBlockGroup";
      
      for ( %i = 0; %i < %dataGroup.getCount(); %i++ )
      {
         %obj = %dataGroup.getObject(%i);
         // echo ("Obj: " @ %obj.getName() @ " - " @ %obj.category );
         
         if ( %obj.category $= "" && %obj.category == 0 )
            continue;
            
         // Add category to popup menu if not there already
         if ( CreatorPopupMenu.findText( %obj.category ) == -1 )
            CreatorPopupMenu.add( %obj.category );
         
         if ( %address $= "" )
         {         
            %ctrl = %this.findIconCtrl( %obj.category );
            if ( %ctrl == -1 )
            {
               %this.addFolderIcon( %obj.category );
            }    
         }
         else if ( %address $= %obj.category )
         {            
            %ctrl = %this.findIconCtrl( %obj.getName() );
            if ( %ctrl == -1 )
               %this.addShapeIcon( %obj );
         }
      }
   }
   
   if ( %this.tab $= "Meshes" )
   {      
      %fullPath = findFirstFileMultiExpr( getFormatExtensions() );
      
      while ( %fullPath !$= "" )
      {
         if (strstr(%fullPath, "cached.dts") != -1)
         {
            %fullPath = findNextFileMultiExpr( getFormatExtensions() );
            continue;
         }

         %fullPath = makeRelativePath( %fullPath, getMainDotCSDir() );                                  
         %splitPath = strreplace( %fullPath, "/", " " );     
         if( getWord(%splitPath, 0) $= "tools" )
         {
            %fullPath = findNextFileMultiExpr( getFormatExtensions() );
            continue;
         }
                      
         %dirCount = getWordCount( %splitPath ) - 1;
         
         %pathFolders = getWords( %splitPath, 0, %dirCount - 1 );         
         
         // Add this file's path (parent folders) to the
         // popup menu if it isn't there yet.
         %temp = strreplace( %pathFolders, " ", "/" );         
         %r = CreatorPopupMenu.findText( %temp );
         if ( %r == -1 )
         {
            CreatorPopupMenu.add( %temp );
         }
         
         // Is this file in the current folder?        
         if ( stricmp( %pathFolders, %address ) == 0 )
         {
            %this.addStaticIcon( %fullPath );
         }
         // Then is this file in a subfolder we need to add
         // a folder icon for?
         else
         {
            %wordIdx = 0;
            %add = false;
            
            if ( %address $= "" )
            {
               %add = true;
               %wordIdx = 0;
            }
            else
            {
               for ( ; %wordIdx < %dirCount; %wordIdx++ )
               {
                  %temp = getWords( %splitPath, 0, %wordIdx );
                  if ( stricmp( %temp, %address ) == 0 )
                  {                  
                     %add = true;
                     %wordIdx++;
                     break;  
                  }
               }
            }
            
            if ( %add == true )
            {               
               %folder = getWord( %splitPath, %wordIdx );
               
               %ctrl = %this.findIconCtrl( %folder );
               if ( %ctrl == -1 )
                  %this.addFolderIcon( %folder );
            }
         }         

         %fullPath = findNextFileMultiExpr( getFormatExtensions() );
      }
   }
   
   if ( %this.tab $= "Level" )
   {         
      // Add groups to popup menu
      %array = %this.array;
      %array.sortk();
      
      %count = %array.count();
      
      if ( %count > 0 )
      {
         %lastGroup = "";
         
         for ( %i = 0; %i < %count; %i++ )
         {
            %group = %array.getKey( %i );

            if ( %group !$= %lastGroup )
            {
               CreatorPopupMenu.add( %group );
               
               if ( %address $= "" )
                  %this.addFolderIcon( %group );                                             
            }               
            
            if ( %address $= %group )
            {
               %args = %array.getValue( %i );
               %class = %args.val[0];
               %name = %args.val[1];
               %func = %args.val[2];

               %this.addMissionObjectIcon( %class, %name, %func );
            }
            
            %lastGroup = %group;
         }
      }
   }   
   
   if ( %this.tab $= "Prefabs" )
   {      
      %expr = "*.prefab";
      %fullPath = findFirstFile( %expr );
      
      while ( %fullPath !$= "" )
      {         
         %fullPath = makeRelativePath( %fullPath, getMainDotCSDir() );                                  
         %splitPath = strreplace( %fullPath, "/", " " );     
         if( getWord(%splitPath, 0) $= "tools" )
         {
            %fullPath = findNextFile( %expr );
            continue;
         }
                      
         %dirCount = getWordCount( %splitPath ) - 1;
         
         %pathFolders = getWords( %splitPath, 0, %dirCount - 1 );         
         
         // Add this file's path (parent folders) to the
         // popup menu if it isn't there yet.
         %temp = strreplace( %pathFolders, " ", "/" );         
         %r = CreatorPopupMenu.findText( %temp );
         if ( %r == -1 )
         {
            CreatorPopupMenu.add( %temp );
         }
         
         // Is this file in the current folder?        
         if ( stricmp( %pathFolders, %address ) == 0 )
         {
            %this.addPrefabIcon( %fullPath );            
         }
         // Then is this file in a subfolder we need to add
         // a folder icon for?
         else
         {
            %wordIdx = 0;
            %add = false;
            
            if ( %address $= "" )
            {
               %add = true;
               %wordIdx = 0;
            }
            else
            {
               for ( ; %wordIdx < %dirCount; %wordIdx++ )
               {
                  %temp = getWords( %splitPath, 0, %wordIdx );
                  if ( stricmp( %temp, %address ) == 0 )
                  {                  
                     %add = true;
                     %wordIdx++;
                     break;  
                  }
               }
            }
            
            if ( %add == true )
            {               
               %folder = getWord( %splitPath, %wordIdx );
               
               %ctrl = %this.findIconCtrl( %folder );
               if ( %ctrl == -1 )
                  %this.addFolderIcon( %folder );
            }
         }         

         %fullPath = findNextFile( %expr );
      }
   } 
   
   CreatorIconArray.sort( "alphaIconCompare" );
   
   for ( %i = 0; %i < CreatorIconArray.getCount(); %i++ )
   {
      CreatorIconArray.getObject(%i).autoSize = false;         
   }
   
   CreatorIconArray.frozen = false;
   CreatorIconArray.refresh();
   
   // Recalculate the array for the parent guiScrollCtrl
   CreatorIconArray.getParent().computeSizes();  
   
   %this.address = %address;

   CreatorPopupMenu.sort();

   %str = strreplace( %address, " ", "/" );
   %r = CreatorPopupMenu.findText( %str );
   if ( %r != -1 )
      CreatorPopupMenu.setSelected( %r, false );
   else
      CreatorPopupMenu.setText( %str );
   CreatorPopupMenu.tooltip = %str;
}

function EWCreatorWindow::navigateDown( %this, %folder )
{
   if ( %this.address $= "" )
      %address = %folder;
   else   
      %address = %this.address SPC %folder;

   // Because this is called from an IconButton::onClick command
   // we have to wait a tick before actually calling navigate, else
   // we would delete the button out from under itself.
   %this.schedule( 1, "navigate", %address );
}

function EWCreatorWindow::navigateUp( %this )
{
   %count = getWordCount( %this.address );
   
   if ( %count == 0 )
      return;
      
   if ( %count == 1 )
      %address = "";
   else      
      %address = getWords( %this.address, 0, %count - 2 );
      
   %this.navigate( %address );
}

function EWCreatorWindow::setListView( %this, %noupdate )
{
   //CreatorIconArray.clear();
   //CreatorIconArray.setVisible( false );
   
   CreatorIconArray.setVisible( true );
   %this.contentCtrl = CreatorIconArray;   
   %this.isList = true;
   
   if ( %noupdate == true )
      %this.navigate( %this.address );
}

//function EWCreatorWindow::setIconView( %this )
//{
   //echo( "setIconView" );
   //
   //CreatorIconStack.clear();
   //CreatorIconStack.setVisible( false );
   //
   //CreatorIconArray.setVisible( true );
   //%this.contentCtrl = CreatorIconArray;
   //%this.isList = false;
   //
   //%this.navigate( %this.address );
//}

function EWCreatorWindow::findIconCtrl( %this, %name )
{
   for ( %i = 0; %i < %this.contentCtrl.getCount(); %i++ )
   {
      %ctrl = %this.contentCtrl.getObject( %i );
      if ( %ctrl.text $= %name )
         return %ctrl;
   }
   
   return -1;
}

function EWCreatorWindow::createIcon( %this )
{
   %ctrl = new GuiIconButtonCtrl()
   {            
      profile = "GuiCreatorIconButtonProfile";     
      buttonType = "radioButton";
      groupNum = "-1";    
   };
      
   if ( %this.isList )
   {
      %ctrl.iconLocation = "Left";
      %ctrl.textLocation = "Right";
      %ctrl.extent = "348 19";
      %ctrl.textMargin = 8;
      %ctrl.buttonMargin = "2 2";
      %ctrl.autoSize = true;
   }
   else
   {
      %ctrl.iconLocation = "Center";         
      %ctrl.textLocation = "Bottom";
      %ctrl.extent = "40 40";    
   }
         
   return %ctrl;
}

function EWCreatorWindow::addFolderIcon( %this, %text )
{
   %ctrl = %this.createIcon();
      
   %ctrl.altCommand = "EWCreatorWindow.navigateDown(\"" @ %text @ "\");";
   %ctrl.iconBitmap = "tools/gui/images/folder.png";   
   %ctrl.text = %text;
   %ctrl.tooltip = %text;     
   %ctrl.class = "CreatorFolderIconBtn";
   
   %ctrl.buttonType = "radioButton";
   %ctrl.groupNum = "-1";   
   
   %this.contentCtrl.addGuiControl( %ctrl );   
}

function EWCreatorWindow::addMissionObjectIcon( %this, %class, %name, %buildfunc )
{
   %ctrl = %this.createIcon();      

   // If we don't find a specific function for building an
   // object then fall back to the stock one
   %method = "build" @ %buildfunc;
   if( !ObjectBuilderGui.isMethod( %method ) )
      %method = "build" @ %class;

   if( !ObjectBuilderGui.isMethod( %method ) )
      %cmd = "return new " @ %class @ "();";
   else
      %cmd = "ObjectBuilderGui." @ %method @ "();";

   %ctrl.altCommand = "ObjectBuilderGui.newObjectCallback = \"EWCreatorWindow.onFinishCreateObject\"; EWCreatorWindow.createObject( \"" @ %cmd @ "\" );";
   %ctrl.iconBitmap = EditorIconRegistry::findIconByClassName( %class );
   %ctrl.text = %name;
   %ctrl.class = "CreatorMissionObjectIconBtn";   
   %ctrl.tooltip = %class; 
   
   %ctrl.buttonType = "radioButton";
   %ctrl.groupNum = "-1";   
   
   %this.contentCtrl.addGuiControl( %ctrl );
}

function EWCreatorWindow::addShapeIcon( %this, %datablock )
{
   %ctrl = %this.createIcon();
   
   %name = %datablock.getName();
   %class = %datablock.getClassName();
   %cmd = %class @ "::create(" @ %name @ ");";
      
   %shapePath = ( %datablock.shapeFile !$= "" ) ? %datablock.shapeFile : %datablock.shapeName;
   
   %createCmd = "EWCreatorWindow.createObject( \\\"" @ %cmd @ "\\\" );";
   %ctrl.altCommand = "ColladaImportDlg.showDialog( \"" @ %shapePath @ "\", \"" @ %createCmd @ "\" );";

   %ctrl.iconBitmap = EditorIconRegistry::findIconByClassName( %class );
   %ctrl.text = %name;
   %ctrl.class = "CreatorShapeIconBtn";
   %ctrl.tooltip = %name;
   
   %ctrl.buttonType = "radioButton";
   %ctrl.groupNum = "-1";   
   
   %this.contentCtrl.addGuiControl( %ctrl );   
}

function EWCreatorWindow::addStaticIcon( %this, %fullPath )
{
   %ctrl = %this.createIcon();
   
   %ext = fileExt( %fullPath );
   %file = fileBase( %fullPath );
   %fileLong = %file @ %ext;
   %tip = %fileLong NL
          "Size: " @ fileSize( %fullPath ) / 1000.0 SPC "KB" NL
          "Date Created: " @ fileCreatedTime( %fullPath ) NL
          "Last Modified: " @ fileModifiedTime( %fullPath );

   %createCmd = "EWCreatorWindow.createStatic( \\\"" @ %fullPath @ "\\\" );";
   %ctrl.altCommand = "ColladaImportDlg.showDialog( \"" @ %fullPath @ "\", \"" @ %createCmd @ "\" );";

   %ctrl.iconBitmap = ( ( %ext $= ".dts" ) ? EditorIconRegistry::findIconByClassName( "TSStatic" ) : "tools/gui/images/iconCollada" );
   %ctrl.text = %file;
   %ctrl.class = "CreatorStaticIconBtn";
   %ctrl.tooltip = %tip;
   
   %ctrl.buttonType = "radioButton";
   %ctrl.groupNum = "-1";   
   
   %this.contentCtrl.addGuiControl( %ctrl );   
}

function EWCreatorWindow::addPrefabIcon( %this, %fullPath )
{
   %ctrl = %this.createIcon();
   
   %ext = fileExt( %fullPath );
   %file = fileBase( %fullPath );
   %fileLong = %file @ %ext;
   %tip = %fileLong NL
          "Size: " @ fileSize( %fullPath ) / 1000.0 SPC "KB" NL
          "Date Created: " @ fileCreatedTime( %fullPath ) NL
          "Last Modified: " @ fileModifiedTime( %fullPath );

   %ctrl.altCommand = "EWCreatorWindow.createPrefab( \"" @ %fullPath @ "\" );";
   %ctrl.iconBitmap = EditorIconRegistry::findIconByClassName( "Prefab" );
   %ctrl.text = %file;
   %ctrl.class = "CreatorPrefabIconBtn";
   %ctrl.tooltip = %tip;
   
   %ctrl.buttonType = "radioButton";
   %ctrl.groupNum = "-1";   
   
   %this.contentCtrl.addGuiControl( %ctrl );   
}

function CreatorPopupMenu::onSelect( %this, %id, %text )
{   
   %split = strreplace( %text, "/", " " );
   EWCreatorWindow.navigate( %split );  
}

function alphaIconCompare( %a, %b )
{
   if ( %a.class $= "CreatorFolderIconBtn" )   
      if ( %b.class !$= "CreatorFolderIconBtn" )
         return -1;
   
   if ( %b.class $= "CreatorFolderIconBtn" )
      if ( %a.class !$= "CreatorFolderIconBtn" )
         return 1;         
   
   %result = stricmp( %a.text, %b.text );
   return %result;
}

// Generic create object helper for use from the console.

function genericCreateObject( %class )
{
   if ( !isClass( %class ) )
   {
      warn( "createObject( " @ %class @ " ) - Was not a valid class." );
      return;
   }
   
   %cmd = "return new " @ %class @ "();";
   
   %obj = EWCreatorWindow.createObject( %cmd );   
   
   // In case the caller wants it.
   return %obj;   
}
