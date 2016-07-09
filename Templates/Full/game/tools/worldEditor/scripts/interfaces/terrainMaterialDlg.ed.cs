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

function TerrainMaterialDlg::show( %this, %matIndex, %terrMat, %onApplyCallback )
{
   Canvas.pushDialog( %this );
   
   %this.matIndex = %matIndex; 
   %this.onApplyCallback = %onApplyCallback;

   %matLibTree = %this-->matLibTree;
   %item = %matLibTree.findItemByObjectId( %terrMat );
   if ( %item != -1 )
   {
      %matLibTree.selectItem( %item );
      %matLibTree.scrollVisible( %item );
   }
   else
   {
      for( %i = 1; %i < %matLibTree.getItemCount(); %i++ )
      {
         %terrMat = TerrainMaterialDlg-->matLibTree.getItemValue(%i);
         if( %terrMat.getClassName() $= "TerrainMaterial" )
         {
            %matLibTree.selectItem( %i, true );
            %matLibTree.scrollVisible( %i );
            break;
         }
      }
   }
}

//-----------------------------------------------------------------------------

function TerrainMaterialDlg::showByObjectId( %this, %matObjectId, %onApplyCallback )
{
   Canvas.pushDialog( %this );
     
   %this.matIndex = -1;
   %this.onApplyCallback = %onApplyCallback;
                 
   %matLibTree = %this-->matLibTree;
   %matLibTree.clearSelection();   
   %item = %matLibTree.findItemByObjectId( %matObjectId );
   if ( %item != -1 )
   {
      %matLibTree.selectItem( %item );
      %matLibTree.scrollVisible( %item );
   }
}

//-----------------------------------------------------------------------------

function TerrainMaterialDlg::onWake( %this )
{
   if( !isObject( ETerrainMaterialPersistMan ) )
      new PersistenceManager( ETerrainMaterialPersistMan );
      
   if( !isObject( TerrainMaterialDlgNewGroup ) )
      new SimGroup( TerrainMaterialDlgNewGroup );
   if( !isObject( TerrainMaterialDlgDeleteGroup ) )
      new SimGroup( TerrainMaterialDlgDeleteGroup );
      
   // Snapshot the materials.
   %this.snapshotMaterials();

   // Refresh the material list.
   %matLibTree = %this-->matLibTree;
   %matLibTree.clear();
   
   %matLibTree.open( TerrainMaterialSet, false );  
   
   %matLibTree.buildVisibleTree( true );   
   %item = %matLibTree.getFirstRootItem();
   %matLibTree.expandItem( %item );
   
   %this.activateMaterialCtrls( true );      
}

//-----------------------------------------------------------------------------

function TerrainMaterialDlg::onSleep( %this )
{
   if( isObject( TerrainMaterialDlgSnapshot ) )
      TerrainMaterialDlgSnapshot.delete();
      
   %this-->matLibTree.clear();
}

//-----------------------------------------------------------------------------

function TerrainMaterialDlg::dialogApply( %this )
{
   // Move all new materials we have created to the root group.
   
   %newCount = TerrainMaterialDlgNewGroup.getCount();
   for( %i = 0; %i < %newCount; %i ++ )
      RootGroup.add( TerrainMaterialDlgNewGroup.getObject( %i ) );
      
   // Finalize deletion of all removed materials.
   
   %deletedCount = TerrainMaterialDlgDeleteGroup.getCount();
   for( %i = 0; %i < %deletedCount; %i ++ )
   {
      %mat = TerrainMaterialDlgDeleteGroup.getObject( %i );
      ETerrainMaterialPersistMan.removeObjectFromFile( %mat );
      
      %matIndex = ETerrainEditor.getMaterialIndex( %mat.internalName );
      if( %matIndex != -1 )
      {
         ETerrainEditor.removeMaterial( %matIndex );
         EPainter.updateLayers();
      }
      
      %mat.delete();
   }

   // Make sure we save any changes to the current selection.
   %this.saveDirtyMaterial( %this.activeMat );
   
   // Save all changes.
   ETerrainMaterialPersistMan.saveDirty();
   
   // Delete the snapshot.
   TerrainMaterialDlgSnapshot.delete();

   // Remove ourselves from the canvas.
   Canvas.popDialog( TerrainMaterialDlg ); 
                            
   call( %this.onApplyCallback, %this.activeMat, %this.matIndex );
}

//-----------------------------------------------------------------------------

function TerrainMaterialDlg::dialogCancel( %this )
{
   // Restore material properties we have changed.
   
   %this.restoreMaterials();
   
   // Clear the persistence manager state.
   
   ETerrainMaterialPersistMan.clearAll();
   
   // Delete all new object we have created.
   
   TerrainMaterialDlgNewGroup.clear();
   
   // Restore materials we have marked for deletion.
   
   %deletedCount = TerrainMaterialDlgDeleteGroup.getCount();
   for( %i = 0; %i < %deletedCount; %i ++ )
   {
      %mat = TerrainMaterialDlgDeleteGroup.getObject( %i );
      %mat.parentGroup = RootGroup;
      TerrainMaterialSet.add( %mat );
   }
   
   Canvas.popDialog( TerrainMaterialDlg );  
}

//-----------------------------------------------------------------------------

function TerrainMaterialDlg::setMaterialName( %this, %newName )
{
   %mat = %this.activeMat;
   
   if( %mat.internalName !$= %newName )
   {
      %existingMat = TerrainMaterialSet.findObjectByInternalName( %newName );
      if( isObject( %existingMat ) )
      {
         MessageBoxOK( "Error",
            "There already is a terrain material called '" @ %newName @ "'.", "", "" );
      }
      else
      {
         %mat.setInternalName( %newName );
         %this-->matLibTree.buildVisibleTree( false );
      }
   }
}

//-----------------------------------------------------------------------------

function TerrainMaterialDlg::changeBase( %this )
{   
   %ctrl = %this-->baseTexCtrl;
   %file = %ctrl.bitmap;
   if( getSubStr( %file, 0 , 6 ) $= "tools/" )
      %file = "";
      
   %file = TerrainMaterialDlg._selectTextureFileDialog( %file );      
   if( %file $= "" )
   {
      if( %ctrl.bitmap !$= "" )
         %file = %ctrl.bitmap;
      else
         %file = "tools/materialEditor/gui/unknownImage";
   }
   
   %file = makeRelativePath( %file, getMainDotCsDir() );
   %ctrl.setBitmap( %file );  
}

//-----------------------------------------------------------------------------

function TerrainMaterialDlg::changeDetail( %this )
{
   %ctrl = %this-->detailTexCtrl;
   %file = %ctrl.bitmap;
   if( getSubStr( %file, 0 , 6 ) $= "tools/" )
      %file = "";

   %file = TerrainMaterialDlg._selectTextureFileDialog( %file );  
   if( %file $= "" )
   {
      if( %ctrl.bitmap !$= "" )
         %file = %ctrl.bitmap;
      else
         %file = "tools/materialEditor/gui/unknownImage";
   }
   
   %file = makeRelativePath( %file, getMainDotCsDir() );
   %ctrl.setBitmap( %file );  
}

//----------------------------------------------------------------------------

function TerrainMaterialDlg::changeMacro( %this )
{
   %ctrl = %this-->macroTexCtrl;
   %file = %ctrl.bitmap;
   if( getSubStr( %file, 0 , 6 ) $= "tools/" )
      %file = "";

   %file = TerrainMaterialDlg._selectTextureFileDialog( %file );  
   if( %file $= "" )
   {
      if( %ctrl.bitmap !$= "" )
         %file = %ctrl.bitmap;
      else
         %file = "tools/materialEditor/gui/unknownImage";
   }
   
   %file = makeRelativePath( %file, getMainDotCsDir() );
   %ctrl.setBitmap( %file );  
}


//-----------------------------------------------------------------------------

function TerrainMaterialDlg::changeNormal( %this )
{   
   %ctrl = %this-->normTexCtrl;
   %file = %ctrl.bitmap;
   if( getSubStr( %file, 0 , 6 ) $= "tools/" )
      %file = "";

   %file = TerrainMaterialDlg._selectTextureFileDialog( %file );  
   if( %file $= "" )
   {
      if( %ctrl.bitmap !$= "" )
         %file = %ctrl.bitmap;
      else
         %file = "tools/materialEditor/gui/unknownImage";
   }

   %file = makeRelativePath( %file, getMainDotCsDir() );
   %ctrl.setBitmap( %file );   
}

//-----------------------------------------------------------------------------

function TerrainMaterialDlg::newMat( %this )
{
   // Create a unique material name.
   %matName = getUniqueInternalName( "newMaterial", TerrainMaterialSet, true );

   // Create the new material.
   %newMat = new TerrainMaterial()
   {
      internalName = %matName;
      parentGroup = TerrainMaterialDlgNewGroup;
   };
   %newMat.setFileName( "art/terrains/materials.cs" );
   
   // Mark it as dirty and to be saved in the default location.
   ETerrainMaterialPersistMan.setDirty( %newMat, "art/terrains/materials.cs" );
            
   %matLibTree = %this-->matLibTree;
   %matLibTree.buildVisibleTree( true );
   %item = %matLibTree.findItemByObjectId( %newMat );
   %matLibTree.selectItem( %item );   
}

//-----------------------------------------------------------------------------

function TerrainMaterialDlg::deleteMat( %this )
{
   if( !isObject( %this.activeMat ) )
      return;

   // Cannot delete this material if it is the only one left on the Terrain
   if ( ( ETerrainEditor.getMaterialCount() == 1 ) &&
        ( ETerrainEditor.getMaterialIndex( %this.activeMat.internalName ) != -1 ) )
   {
      MessageBoxOK( "Error", "Cannot delete this Material, it is the only " @
         "Material still in use by the active Terrain." );
      return;
   }

   TerrainMaterialSet.remove( %this.activeMat );
   TerrainMaterialDlgDeleteGroup.add( %this.activeMat );
   
   %matLibTree = %this-->matLibTree;
   %matLibTree.open( TerrainMaterialSet, false );
   %matLibTree.selectItem( 1 );
}

//-----------------------------------------------------------------------------

function TerrainMaterialDlg::activateMaterialCtrls( %this, %active )
{  
   %parent = %this-->matSettingsParent;
   %count = %parent.getCount();
   for ( %i = 0; %i < %count; %i++ )
      %parent.getObject( %i ).setActive( %active );      
}

//-----------------------------------------------------------------------------

function TerrainMaterialTreeCtrl::onSelect( %this, %item )
{
   TerrainMaterialDlg.setActiveMaterial( %item );
}

//-----------------------------------------------------------------------------

function TerrainMaterialTreeCtrl::onUnSelect( %this, %item )
{
   TerrainMaterialDlg.saveDirtyMaterial( %item );   
   TerrainMaterialDlg.setActiveMaterial( 0 );   
}

//-----------------------------------------------------------------------------

function TerrainMaterialDlg::setActiveMaterial( %this, %mat )
{  
   if (  isObject( %mat ) && 
         %mat.isMemberOfClass( TerrainMaterial ) )
   {
      %this.activeMat = %mat;
      
      %this-->matNameCtrl.setText( %mat.internalName );
      if (%mat.diffuseMap $= ""){
         %this-->baseTexCtrl.setBitmap( "tools/materialEditor/gui/unknownImage" );
      }else{
         %this-->baseTexCtrl.setBitmap( %mat.diffuseMap ); 
      }
      if (%mat.detailMap $= ""){
         %this-->detailTexCtrl.setBitmap( "tools/materialEditor/gui/unknownImage" );
      }else{
         %this-->detailTexCtrl.setBitmap( %mat.detailMap );
      }
      if (%mat.macroMap $= ""){
         %this-->macroTexCtrl.setBitmap( "tools/materialEditor/gui/unknownImage" );
      }else{
         %this-->macroTexCtrl.setBitmap( %mat.macroMap );
      }      
      if (%mat.normalMap $= ""){
         %this-->normTexCtrl.setBitmap( "tools/materialEditor/gui/unknownImage" );
      }else{
         %this-->normTexCtrl.setBitmap( %mat.normalMap ); 
      }
      %this-->detSizeCtrl.setText( %mat.detailSize );
      %this-->baseSizeCtrl.setText( %mat.diffuseSize );
      %this-->detStrengthCtrl.setText( %mat.detailStrength );
      %this-->detDistanceCtrl.setText( %mat.detailDistance );      
      %this-->sideProjectionCtrl.setValue( %mat.useSideProjection );
      %this-->parallaxScaleCtrl.setText( %mat.parallaxScale );

      %this-->macroSizeCtrl.setText( %mat.macroSize );
      %this-->macroStrengthCtrl.setText( %mat.macroStrength );
      %this-->macroDistanceCtrl.setText( %mat.macroDistance );      
            
      %this.activateMaterialCtrls( true );      
   }
   else
   {
      %this.activeMat = 0;
      %this.activateMaterialCtrls( false );        
   }
}

//-----------------------------------------------------------------------------

function TerrainMaterialDlg::saveDirtyMaterial( %this, %mat )
{
   // Skip over obviously bad cases.
   if (  !isObject( %mat ) || 
         !%mat.isMemberOfClass( TerrainMaterial ) )
      return;
            
   // Read out properties from the dialog.
   
   %newName = %this-->matNameCtrl.getText(); 
   
   if (%this-->baseTexCtrl.bitmap $= "tools/materialEditor/gui/unknownImage"){
      %newDiffuse = "";
   }else{
      %newDiffuse = %this-->baseTexCtrl.bitmap;  
   }
   if (%this-->normTexCtrl.bitmap $= "tools/materialEditor/gui/unknownImage"){
      %newNormal = "";
   }else{
      %newNormal = %this-->normTexCtrl.bitmap;  
   }
   if (%this-->detailTexCtrl.bitmap $= "tools/materialEditor/gui/unknownImage"){
      %newDetail = "";
   }else{
      %newDetail = %this-->detailTexCtrl.bitmap;  
   }
   if (%this-->macroTexCtrl.bitmap $= "tools/materialEditor/gui/unknownImage"){
      %newMacro = "";
   }else{
      %newMacro = %this-->macroTexCtrl.bitmap;  
   }
   %detailSize = %this-->detSizeCtrl.getText();      
   %diffuseSize = %this-->baseSizeCtrl.getText();     
   %detailStrength = %this-->detStrengthCtrl.getText();
   %detailDistance = %this-->detDistanceCtrl.getText();   
   %useSideProjection = %this-->sideProjectionCtrl.getValue();   
   %parallaxScale = %this-->parallaxScaleCtrl.getText();

   %macroSize = %this-->macroSizeCtrl.getText();      
   %macroStrength = %this-->macroStrengthCtrl.getText();
   %macroDistance = %this-->macroDistanceCtrl.getText();   
   
   // If no properties of this materials have changed,
   // return.

   if (  %mat.internalName $= %newName &&
         %mat.diffuseMap $= %newDiffuse &&
         %mat.normalMap $= %newNormal &&
         %mat.detailMap $= %newDetail &&
         %mat.macroMap $= %newMacro &&
         %mat.detailSize == %detailSize &&
         %mat.diffuseSize == %diffuseSize &&
         %mat.detailStrength == %detailStrength &&
         %mat.detailDistance == %detailDistance &&         
         %mat.useSideProjection == %useSideProjection &&
         %mat.macroSize == %macroSize &&
         %mat.macroStrength == %macroStrength &&
         %mat.macroDistance == %macroDistance &&         
         %mat.parallaxScale == %parallaxScale )               
      return;
      
   // Make sure the material name is unique.
   
   if( %mat.internalName !$= %newName )
   {
      %existingMat = TerrainMaterialSet.findObjectByInternalName( %newName );
      if( isObject( %existingMat ) )
      {
         MessageBoxOK( "Error",
            "There already is a terrain material called '" @ %newName @ "'.", "", "" );
            
         // Reset the name edit control to the old name.
            
         %this-->matNameCtrl.setText( %mat.internalName );
      }
      else
         %mat.setInternalName( %newName );    
   }
   
   %mat.diffuseMap = %newDiffuse;    
   %mat.normalMap = %newNormal;    
   %mat.detailMap = %newDetail;    
   %mat.macroMap = %newMacro;
   %mat.detailSize = %detailSize;  
   %mat.diffuseSize = %diffuseSize;
   %mat.detailStrength = %detailStrength;    
   %mat.detailDistance = %detailDistance;    
   %mat.macroSize = %macroSize;  
   %mat.macroStrength = %macroStrength;    
   %mat.macroDistance = %macroDistance;    
   %mat.useSideProjection = %useSideProjection;
   %mat.parallaxScale = %parallaxScale;
   
   // Mark the material as dirty and needing saving.
   
   %fileName = %mat.getFileName();
   if( %fileName $= "" )
      %fileName = "art/terrains/materials.cs";
      
   ETerrainMaterialPersistMan.setDirty( %mat, %fileName ); 
}

//-----------------------------------------------------------------------------

function TerrainMaterialDlg::snapshotMaterials( %this )
{
   if( !isObject( TerrainMaterialDlgSnapshot ) )
      new SimGroup( TerrainMaterialDlgSnapshot );
      
   %group = TerrainMaterialDlgSnapshot;
   %group.clear();
   
   %matCount = TerrainMaterialSet.getCount();
   for( %i = 0; %i < %matCount; %i ++ )
   {
      %mat = TerrainMaterialSet.getObject( %i );
      if( !isMemberOfClass( %mat.getClassName(), "TerrainMaterial" ) )
         continue;
         
      %snapshot = new ScriptObject()
      {
         parentGroup = %group;
         material = %mat;
         internalName = %mat.internalName;
         diffuseMap = %mat.diffuseMap;
         normalMap = %mat.normalMap;
         detailMap = %mat.detailMap;
         macroMap = %mat.macroMap;
         detailSize = %mat.detailSize;
         diffuseSize = %mat.diffuseSize;
         detailStrength = %mat.detailStrength;
         detailDistance = %mat.detailDistance;
         macroSize = %mat.macroSize;
         macroStrength = %mat.macroStrength;
         macroDistance = %mat.macroDistance;
         useSideProjection = %mat.useSideProjection;
         parallaxScale = %mat.parallaxScale;
      };
   }
}

//-----------------------------------------------------------------------------

function TerrainMaterialDlg::restoreMaterials( %this )
{
   if( !isObject( TerrainMaterialDlgSnapshot ) )
   {
      error( "TerrainMaterial::restoreMaterials - no snapshot present" );
      return;
   }
   
   %count = TerrainMaterialDlgSnapshot.getCount();
   for( %i = 0; %i < %count; %i ++ )
   {
      %obj = TerrainMaterialDlgSnapshot.getObject( %i );      
      %mat = %obj.material;

      %mat.setInternalName( %obj.internalName );
      %mat.diffuseMap = %obj.diffuseMap;
      %mat.normalMap = %obj.normalMap;
      %mat.detailMap = %obj.detailMap;
      %mat.macroMap = %obj.macroMap;
      %mat.detailSize = %obj.detailSize;
      %mat.diffuseSize = %obj.diffuseSize;
      %mat.detailStrength = %obj.detailStrength;
      %mat.detailDistance = %obj.detailDistance;
      %mat.macroSize = %obj.macroSize;
      %mat.macroStrength = %obj.macroStrength;
      %mat.macroDistance = %obj.macroDistance;
      %mat.useSideProjection = %obj.useSideProjection;
      %mat.parallaxScale = %obj.parallaxScale;
   }
}

//-----------------------------------------------------------------------------

function TerrainMaterialDlg::_selectTextureFileDialog( %this, %defaultFileName )
{
   if( $Pref::TerrainEditor::LastPath $= "" )
      $Pref::TerrainEditor::LastPath = "art/terrains";

   %dlg = new OpenFileDialog()
   {
      Filters        = $TerrainEditor::TextureFileSpec;
      DefaultPath    = $Pref::TerrainEditor::LastPath;
      DefaultFile    = %defaultFileName;
      ChangePath     = false;
      MustExist      = true;
   };
            
   %ret = %dlg.Execute();
   if ( %ret )
   {
      $Pref::TerrainEditor::LastPath = filePath( %dlg.FileName );
      %file = %dlg.FileName;
   }
      
   %dlg.delete();
   
   if ( !%ret )
      return; 
      
   %file = filePath(%file) @ "/" @ fileBase(%file);
      
   return %file;
}
