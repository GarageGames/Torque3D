
new SimGroup(AssetBrowserPreviewCache);

//AssetBrowser.addToolbarButton
function AssetBrowser::addToolbarButton(%this)
{
	%filename = expandFilename("tools/gui/images/iconOpen");
	%button = new GuiBitmapButtonCtrl() {
		canSaveDynamicFields = "0";
		internalName = AssetBrowserBtn;
		Enabled = "1";
		isContainer = "0";
		Profile = "ToolsGuiButtonProfile";
		HorizSizing = "right";
		VertSizing = "bottom";
		position = "180 0";
		Extent = "25 19";
		MinExtent = "8 2";
		canSave = "1";
		Visible = "1";
		Command = "AssetBrowser.ShowDialog();";
		tooltipprofile = "ToolsGuiToolTipProfile";
		ToolTip = "Asset Browser";
		hovertime = "750";
		bitmap = %filename;
		bitmapMode = "Centered";
		buttonType = "PushButton";
		groupNum = "0";
		useMouseEvents = "0";
	};
	ToolsToolbarArray.add(%button);
	EWToolsToolbar.setExtent((25 + 8) * (ToolsToolbarArray.getCount()) + 12 SPC "33");
}
//
function AssetBrowser::onAdd(%this)
{
   %this.isReImportingAsset = false;
}

function AssetBrowser::onWake(%this)
{
   %this.importAssetNewListArray = new ArrayObject();
   %this.importAssetUnprocessedListArray = new ArrayObject();
   %this.importAssetFinalListArray = new ArrayObject();
}

//Drag-Drop functionality
function AssetBrowser::selectAsset( %this, %asset )
{
   if(AssetBrowser.selectCallback !$= "")
   {
      // The callback function should be ready to intake the returned material
      //eval("materialEd_previewMaterial." @ %propertyField @ " = " @ %value @ ";");
      if( AssetBrowser.returnType $= "name" )
      {
         eval( "" @ AssetBrowser.selectCallback @ "(" @ %name  @ ");");
      }
      else
      {
         %command = "" @ AssetBrowser.selectCallback @ "(\"" @ %asset  @ "\");";
         eval(%command);
      }
   }
   else
   {
      //try just setting the asset
      %this.changeAsset();
   }
   
   Inspector.refresh();
   
   AssetBrowser.hideDialog();
}

function AssetBrowser::showDialog( %this, %AssetTypeFilter, %selectCallback, %targetObj, %fieldName, %returnType)
{
   // Set the select callback
   AssetBrowser.selectCallback = %selectCallback;
   AssetBrowser.returnType = %returnType;
   AssetBrowser.assetTypeFilter = %AssetTypeFilter;
   AssetBrowser.fieldTargetObject = %targetObj;
   AssetBrowser.fieldTargetName = %fieldName;

   Canvas.add(AssetBrowser);
   AssetBrowser.setVisible(1);
   AssetBrowserWindow.setVisible(1);
   AssetBrowserWindow.selectWindow();
   
   if(%selectCallback $= "")
   {
      //we're not in selection mode, so just hide the select button
      %this-->SelectButton.setHidden(true);  
   }
   else
   {
      %this-->SelectButton.setHidden(false); 
   }
   
   //AssetBrowser_importAssetWindow.setVisible(0);
   //AssetBrowser_importAssetConfigWindow.setVisible(0);
   AssetBrowser.loadFilters();
}

function AssetBrowser::hideDialog( %this )
{
   AssetBrowser.setVisible(1);
   AssetBrowserWindow.setVisible(1);
   Canvas.popDialog(AssetBrowser_addModule);
   Canvas.popDialog(ImportAssetWindow);
   
   Canvas.popDialog(AssetBrowser);
}

function AssetBrowser::buildPreviewArray( %this, %asset, %moduleName )
{
   %assetDesc = AssetDatabase.acquireAsset(%asset);
   %assetName = AssetDatabase.getAssetName(%asset);
   %previewImage = "core/art/warnmat";
   
   AssetPreviewArray.empty();
      
   // it may seem goofy why the checkbox can't be instanciated inside the container
   // reason being its because we need to store the checkbox ctrl in order to make changes
   // on it later in the function. 

   %previewSize = "80 80";
   %previewBounds = 20;
   
   %assetType = AssetDatabase.getAssetType(%asset);
   
   %container = new GuiControl(){
      profile = "ToolsGuiDefaultProfile";
      Position = "0 0";
      Extent = %previewSize.x + %previewBounds SPC %previewSize.y + %previewBounds + 24;
      HorizSizing = "right";
      VertSizing = "bottom";
      isContainer = "1";
      assetName = %assetName;
      moduleName = %moduleName;
      assetType = %assetType;
   };

   %tooltip = %assetName;
   
   if(%assetType $= "ShapeAsset")
   {
      %previewButton = new GuiObjectView()
      {
         className = "AssetPreviewControl";
         internalName = %matName;
         HorizSizing = "right";
         VertSizing = "bottom";
         Profile = "ToolsGuiDefaultProfile";
         position = "7 4";
         extent = %previewSize;
         MinExtent = "8 8";
         canSave = "1";
         Visible = "1";
         tooltipprofile = "ToolsGuiToolTipProfile";
         hovertime = "1000";
         Margin = "0 0 0 0";
         Padding = "0 0 0 0";
         AnchorTop = "1";
         AnchorBottom = "0";
         AnchorLeft = "1";
         AnchorRight = "0";
         renderMissionArea = "0";
         GizmoProfile = "GlobalGizmoProfile";
         cameraZRot = "0";
         forceFOV = "0";
         gridColor = "0 0 0 0";
         renderNodes = "0";
         renderObjBox = "0";
         renderMounts = "0";
         renderColMeshes = "0";
         selectedNode = "-1";
         sunDiffuse = "255 255 255 255";
         sunAmbient = "180 180 180 255";
         timeScale = "1.0";
         fixedDetail = "0";
         orbitNode = "0";
         
         new GuiBitmapButtonCtrl()
         {
            HorizSizing = "right";
            VertSizing = "bottom";
            profile = "ToolsGuiButtonProfile";
            position = "0 0";
            extent = %previewSize;
            Variable = "";
            buttonType = "ToggleButton";
            bitmap = "tools/materialEditor/gui/cubemapBtnBorder";
            groupNum = "0";
            text = "";
         }; 
      };
      
      %assetQuery = new AssetQuery();
      %numAssetsFound = AssetDatabase.findAllAssets(%assetQuery);
      
      for( %i=0; %i < %numAssetsFound; %i++)
      {
          %assetId = %assetQuery.getAsset(%i);
          %name = AssetDatabase.getAssetName(%assetId);
          
          if(%name $= %assetName)
          {
            %asset = AssetDatabase.acquireAsset(%assetId);
            
            %previewButton.setModel(%asset.fileName);
            //%previewButton.refreshShape();
            //%previewButton.currentDL = 0;
            //%previewButton.fitToShape();
            
            break;
          }
      }
   }
   else
   {
      if(%assetType $= "ComponentAsset")
      {
         %assetPath = "data/" @ %moduleName @ "/components/" @ %assetName @ ".cs";
         %doubleClickCommand = "EditorOpenFileInTorsion( "@%assetPath@", 0 );";
         
         %previewImage = "tools/assetBrowser/art/componentIcon";
         
         %assetFriendlyName = %assetDesc.friendlyName;
         %assetDesc = %assetDesc.description;
         %tooltip = %assetFriendlyName @ "\n" @ %assetDesc;
      }
      else if(%assetType $= "GameObjectAsset")
      {
         %assetPath = "data/" @ %moduleName @ "/gameObjects/" @ %assetName @ ".cs";
         %doubleClickCommand = "EditorOpenFileInTorsion( "@%assetPath@", 0 );";
         
         %previewImage = "tools/assetBrowser/art/gameObjectIcon";
         
         %tooltip = %assetDesc.gameObjectName;
      }
      else if(%assetType $= "ImageAsset")
      {
         //nab the image and use it for the preview
         %assetQuery = new AssetQuery();
         %numAssetsFound = AssetDatabase.findAllAssets(%assetQuery);
         
         for( %i=0; %i < %numAssetsFound; %i++)
         {
             %assetId = %assetQuery.getAsset(%i);
             %name = AssetDatabase.getAssetName(%assetId);
             
             if(%name $= %assetName)
             {
               %asset = AssetDatabase.acquireAsset(%assetId);
               %previewImage = %asset.imageFile;
               break;
             }
         }
      }
      else if(%assetType $= "StateMachineAsset")
      {
         %previewImage = "tools/assetBrowser/art/stateMachineIcon";
      }
      else if(%assetType $= "SoundAsset")
      {
         %previewImage = "tools/assetBrowser/art/soundIcon";
      }
      else if(%assetType $= "LevelAsset")
      {
         %previewImage = "tools/assetBrowser/art/levelIcon";
      }
      else if(%assetType $= "PostEffectAsset")
      {
         %previewImage = "tools/assetBrowser/art/postEffectIcon";
      }
      else if(%assetType $= "GUIAsset")
      {
         %previewImage = "tools/assetBrowser/art/guiIcon";
      }
      else if(%assetType $= "ScriptAsset")
      {
         if(%assetDesc.isServerSide)
            %previewImage = "tools/assetBrowser/art/serverScriptIcon";
         else
            %previewImage = "tools/assetBrowser/art/clientScriptIcon";
      }
      else if(%assetType $= "MaterialAsset")
      {
         %previewImage = "";
         //nab the image and use it for the preview
         %assetQuery = new AssetQuery();
         %numAssetsFound = AssetDatabase.findAllAssets(%assetQuery);
         
         for( %i=0; %i < %numAssetsFound; %i++)
         {
             %assetId = %assetQuery.getAsset(%i);
             %name = AssetDatabase.getAssetName(%assetId);
             
             if(%name $= %assetName)
             {
               %asset = AssetDatabase.acquireAsset(%assetId);
               %previewImage = %asset.materialDefinitionName.diffuseMap[0];
               break;
             }
         }
         
         if(%previewImage $= "")
            %previewImage = "tools/assetBrowser/art/materialIcon";
      }
      if(%assetType $= "ShapeAnimationAsset")
      {
         %previewImage = "tools/assetBrowser/art/animationIcon";
      }
      
      %previewButton = new GuiBitmapButtonCtrl()
      {
         className = "AssetPreviewControl";
         internalName = %assetName;
         HorizSizing = "right";
         VertSizing = "bottom";
         profile = "ToolsGuiButtonProfile";
         position = "10 4";
         extent = %previewSize;
         buttonType = "PushButton";
         bitmap = %previewImage;
         Command = "";
         text = "";
         useStates = false;
         
         new GuiBitmapButtonCtrl()
         {
               HorizSizing = "right";
               VertSizing = "bottom";
               profile = "ToolsGuiButtonProfile";
               position = "0 0";
               extent = %previewSize;
               Variable = "";
               buttonType = "toggleButton";
               bitmap = "tools/materialEditor/gui/cubemapBtnBorder";
               groupNum = "0";
               text = "";
            }; 
      }; 
   }
   
   %previewBorder = new GuiButtonCtrl(){
         class = "AssetPreviewButton";
         internalName = %assetName@"Border";
         HorizSizing = "right";
         VertSizing = "bottom";
         profile = "ToolsGuiThumbHighlightButtonProfile";
         position = "0 0";
         extent = %previewSize.x + %previewBounds SPC %previewSize.y + 24;
         Variable = "";
         buttonType = "radioButton";
         tooltip = %tooltip;
         Command = "AssetBrowser.updateSelection( $ThisControl.getParent().assetName, $ThisControl.getParent().moduleName );"; 
		   altCommand = %doubleClickCommand;
         groupNum = "0";
         useMouseEvents = true;
         text = "";
         icon = %previewImage;
   };
   
   %previewNameCtrl = new GuiTextEditCtrl(){
         position = 0 SPC %previewSize.y + %previewBounds - 16;
         profile = ToolsGuiTextEditCenterProfile;
         extent = %previewSize.x + %previewBounds SPC 16;
         text = %assetName;
         originalAssetName = %assetName; //special internal field used in renaming assets
         internalName = "AssetNameLabel";
         class = "AssetNameField";
         active = false;
      };
   
   %container.add(%previewButton);  
   %container.add(%previewBorder); 
   %container.add(%previewNameCtrl);
   
   // add to the gui control array
   AssetBrowser-->materialSelection.add(%container);
   
   // add to the array object for reference later
   AssetPreviewArray.add( %previewButton, %previewImage );
}

function AssetBrowser::loadImages( %this, %materialNum )
{
   // this will save us from spinning our wheels in case we don't exist
   /*if( !AssetBrowser.visible )
      return;
   
   // this schedule is here to dynamically load images
   %previewButton = AssetPreviewArray.getKey(%materialNum);
   %previewImage = AssetPreviewArray.getValue(%materialNum);
   
   if(%previewButton.getClassName() !$= "GuiObjectView")
   {
      %previewButton.setBitmap(%previewImage);
      %previewButton.setText("");
   }
   
   %materialNum++;
   
   /*if( %materialNum < AssetPreviewArray.count() )
   {
      %tempSchedule = %this.schedule(64, "loadImages", %materialNum);
      MatEdScheduleArray.add( %tempSchedule, %materialNum );
   }*/
}

function AssetBrowser::loadFilters( %this )
{
   AssetBrowser-->filterTree.clear();

   AssetBrowser-->filterTree.buildIconTable(":tools/classIcons/prefab");

   AssetBrowser-->filterTree.insertItem(0, "Assets");
   
   //First, build our our list of active modules
   %modulesList = ModuleDatabase.findModules(true);
   
   for(%i=0; %i < getWordCount(%modulesList); %i++)
   {
      %moduleName = getWord(%modulesList, %i).ModuleId;
      
      %moduleItemId = AssetBrowser-->filterTree.findItemByName(%moduleName);
		
		if(%moduleItemId == 0)
		   %moduleItemId = AssetBrowser-->filterTree.insertItem(1, %moduleName, "", "", 1, 1); 
   }

   //Next, go through and list the asset categories
   %assetQuery = new AssetQuery();
   %numAssetsFound = AssetDatabase.findAllAssets(%assetQuery);
   
   for( %i=0; %i < %numAssetsFound; %i++)
   {
	    %assetId = %assetQuery.getAsset(%i);
	    
		//first, get the asset's module, as our major categories
		%module = AssetDatabase.getAssetModule(%assetId);
		
		%moduleName = %module.moduleId;
		
		//These are core, native-level components, so we're not going to be messing with this module at all, skip it
		if(%moduleName $= "CoreComponentsModule")
		   continue;
		
		//first, see if this module Module is listed already
		%moduleItemId = AssetBrowser-->filterTree.findItemByName(%moduleName);
		
		if(%moduleItemId == 0)
		   %moduleItemId = AssetBrowser-->filterTree.insertItem(1, %moduleName, "", "", 1, 1);
		   
      %assetType = AssetDatabase.getAssetCategory(%assetId);
		
		if(%assetType $= "")
		{
		   %assetType = AssetDatabase.getAssetType(%assetId);
		   if(%assetType $= "")
			   %assetType = "Misc";
		}
		
		if(AssetBrowser.assetTypeFilter !$= "" && AssetBrowser.assetTypeFilter !$= %assetType)
		   continue;
		
		%assetTypeId = AssetBrowser-->filterTree.findChildItemByName(%moduleItemId, %assetType);
		
		if(%assetTypeId == 0)
		   %assetTypeId = AssetBrowser-->filterTree.insertItem(%moduleItemId, %assetType);
   }

   AssetBrowser-->filterTree.buildVisibleTree(true);
   
   //special handling for selections
   if(AssetBrowser.newModuleId !$= "")
   {
      AssetBrowser-->filterTree.clearSelection();
      %newModuleItem = AssetBrowser-->filterTree.findItemByName(AssetBrowser.newModuleId);
      AssetBrowser-->filterTree.selectItem(%newModuleItem);
      AssetBrowser.newModuleId = ""; 
   }
   
   %selectedItem = AssetBrowser-->filterTree.getSelectedItem();
   AssetBrowser-->filterTree.scrollVisibleByObjectId(%selectedItem);
}

// create category and update current material if there is one
function AssetBrowser::createFilter( %this, %filter )
{
   if( %filter $= %existingFilters )
   {
      MessageBoxOK( "Error", "Can not create blank filter.");
      return;
   }
      
   for( %i = AssetBrowser.staticFilterObjects; %i < AssetBrowser-->filterArray.getCount() ; %i++ )
   {
      %existingFilters = AssetBrowser-->filterArray.getObject(%i).getObject(0).filter;
      if( %filter $= %existingFilters )
      {
         MessageBoxOK( "Error", "Can not create two filters of the same name.");
         return;
      }
   }
   %container = new GuiControl(){
      profile = "ToolsGuiDefaultProfile";
      Position = "0 0";
      Extent = "128 18";
      HorizSizing = "right";
      VertSizing = "bottom";
      isContainer = "1";
         
      new GuiCheckBoxCtrl(){
         Profile = "ToolsGuiCheckBoxListProfile";
         position = "5 1";
         Extent = "118 18";
         Command = "";
         groupNum = "0";
         buttonType = "ToggleButton";
         text = %filter @ " ( " @ MaterialFilterAllArray.countKey(%filter) @ " )";
         filter = %filter;
         Command = "AssetBrowser.preloadFilter();";
      };
   };
   
   AssetBrowser-->filterArray.add( %container );
   
   // if selection exists, lets reselect it to refresh it
   if( isObject(AssetBrowser.selectedMaterial) )
      AssetBrowser.updateSelection( AssetBrowser.selectedMaterial, AssetBrowser.selectedPreviewImagePath );
   
   // material category text field to blank
   AssetBrowser_addFilterWindow-->tagName.setText("");
}

function AssetBrowser::updateSelection( %this, %asset, %moduleName )
{
   // the material selector will visually update per material information
   // after we move away from the material. eg: if we remove a field from the material,
   // the empty checkbox will still be there until you move fro and to the material again
   
   %isAssetBorder = 0;
   eval("%isAssetBorder = isObject(AssetBrowser-->"@%asset@"Border);");
   if( %isAssetBorder )
   {
      eval( "AssetBrowser-->"@%asset@"Border.setStateOn(1);");
   }
      
   %isAssetBorderPrevious = 0;
   eval("%isAssetBorderPrevious = isObject(AssetBrowser-->"@%this.prevSelectedMaterialHL@"Border);");
   if( %isAssetBorderPrevious )
   {
      eval( "AssetBrowser-->"@%this.prevSelectedMaterialHL@"Border.setStateOn(0);");
   }
   
   AssetBrowser.selectedMaterial = %asset;
   AssetBrowser.selectedAsset = %moduleName@":"@%asset;
   AssetBrowser.selectedAssetDef = AssetDatabase.acquireAsset(AssetBrowser.selectedAsset);
   //AssetBrowser.selectedPreviewImagePath = %previewImagePath;
   
   %this.prevSelectedMaterialHL = %asset;
}

//
//needs to be deleted with the persistence manager and needs to be blanked out of the matmanager
//also need to update instances... i guess which is the tricky part....
function AssetBrowser::showDeleteDialog( %this )
{
   %material = AssetBrowser.selectedMaterial;
   %secondFilter = "MaterialFilterMappedArray";
   %secondFilterName = "Mapped";
   
   for( %i = 0; %i < MaterialFilterUnmappedArray.count(); %i++ )
   {
      if( MaterialFilterUnmappedArray.getValue(%i) $= %material )
      {
         %secondFilter = "MaterialFilterUnmappedArray";
         %secondFilterName = "Unmapped";
         break;
      }
   }
   
   if( isObject( %material ) )
   {
      MessageBoxYesNoCancel("Delete Material?", 
         "Are you sure you want to delete<br><br>" @ %material.getName() @ "<br><br> Material deletion won't take affect until the engine is quit.", 
         "AssetBrowser.deleteMaterial( " @ %material @ ", " @ %secondFilter @ ", " @ %secondFilterName @" );", 
         "", 
         "" );
   }
}

function AssetBrowser::deleteMaterial( %this, %materialName, %secondFilter, %secondFilterName )
{
   if( !isObject( %materialName ) )
      return;
   
   for( %i = 0; %i <= MaterialFilterAllArray.countValue( %materialName ); %i++)
   {
      %index = MaterialFilterAllArray.getIndexFromValue( %materialName );
      MaterialFilterAllArray.erase( %index );
   }
   MaterialFilterAllArrayCheckbox.setText("All ( " @ MaterialFilterAllArray.count() - 1 @ " ) ");
   
   %checkbox = %secondFilter @ "Checkbox";
   for( %k = 0; %k <= %secondFilter.countValue( %materialName ); %k++)
   {
      %index = %secondFilter.getIndexFromValue( %materialName );
      %secondFilter.erase( %index );
   }
   %checkbox.setText( %secondFilterName @ " ( " @ %secondFilter.count() - 1 @ " ) ");
   
   for( %i = 0; %materialName.getFieldValue("materialTag" @ %i) !$= ""; %i++ )
   {
      %materialTag = %materialName.getFieldValue("materialTag" @ %i);
         
         for( %j = AssetBrowser.staticFilterObjects; %j < AssetBrowser-->filterArray.getCount() ; %j++ )
         {
            if( %materialTag $= AssetBrowser-->filterArray.getObject(%j).getObject(0).filter )
            {
               %count = getWord( AssetBrowser-->filterArray.getObject(%j).getObject(0).getText(), 2 );
               %count--;
               AssetBrowser-->filterArray.getObject(%j).getObject(0).setText( %materialTag @ " ( "@ %count @ " )");
            }
         }
      
   }
   
   UnlistedMaterials.add( "unlistedMaterials", %materialName );
   
   if( %materialName.getFilename() !$= "" && 
         %materialName.getFilename() !$= "tools/gui/AssetBrowser.ed.gui" &&
         %materialName.getFilename() !$= "tools/materialEditor/scripts/materialEditor.ed.cs" )
   {
      AssetBrowserPerMan.removeObjectFromFile(%materialName);
      AssetBrowserPerMan.saveDirty();
   }
      
   AssetBrowser.preloadFilter();
}

function AssetBrowser::thumbnailCountUpdate(%this)
{
   $Pref::AssetBrowser::ThumbnailCountIndex = AssetBrowser-->materialPreviewCountPopup.getSelected();
   AssetBrowser.LoadFilter( AssetBrowser.currentFilter, AssetBrowser.currentStaticFilter );
}

function AssetBrowser::toggleTagFilterPopup(%this)
{
	if(TagFilterWindow.visible)
		TagFilterWindow.visible = false;
	else
		TagFilterWindow.visible = true;
		
	return;
   %assetQuery = new AssetQuery();
   %numAssetsFound = AssetDatabase.findAllAssets(%assetQuery);
   
   for( %i=0; %i < %numAssetsFound; %i++)
   {
	    %assetId = %assetQuery.getAsset(%i);
		
		//first, get the asset's module, as our major categories
		%module = AssetDatabase.getAssetModule(%assetId);
		
		%moduleName = %module.moduleId;
		
		//check that we don't re-add it
		%moduleItemId = AssetBrowser-->filterTree.findItemByName(%moduleName);
		
		if(%moduleItemId == -1 || %moduleItemId == 0)
			%moduleItemId = AssetBrowser-->filterTree.insertItem(1, %module.moduleId, "", "", 1, 1);
			
		//now, add the asset's category
		%assetType = AssetDatabase.getAssetCategory(%assetId);
		
		%checkBox = new GuiCheckBoxCtrl()
		{
			canSaveDynamicFields = "0";
			isContainer = "0";
			Profile = "ToolsGuiCheckBoxListProfile";
			HorizSizing = "right";
			VertSizing = "bottom";
			Position = "0 0";
			Extent = (%textLength * 4) @ " 18";
			MinExtent = "8 2";
			canSave = "1";
			Visible = "1";
			Variable = %var;
			tooltipprofile = "ToolsGuiToolTipProfile";
			hovertime = "1000";
			text = %text;
			groupNum = "-1";
			buttonType = "ToggleButton";
			useMouseEvents = "0";
			useInactiveState = "0";
			Command = %cmd;
		};
		
		TagFilterList.add(%checkBox);
   }	
}

function AssetBrowser::changeAsset(%this)
{
   //alright, we've selectd an asset for a field, so time to set it!
   %cmd = %this.fieldTargetObject @ "." @ %this.fieldTargetName @ "=\"" @ %this.selectedAsset @ "\";";
   echo("Changing asset via the " @ %cmd @ " command");
   eval(%cmd);
}

function AssetBrowser::reImportAsset(%this)
{
   //Find out what type it is
   %assetDef = AssetDatabase.acquireAsset(EditAssetPopup.assetId);
   %assetType = AssetDatabase.getAssetType(EditAssetPopup.assetId);
      
   if(%assetType $= "ShapeAsset" || %assetType $= "ImageAsset" || %assetType $= "SoundAsset")
   {
      AssetBrowser.isAssetReImport = true;
      AssetBrowser.reImportingAssetId = EditAssetPopup.assetId;
      
      AssetBrowser.onBeginDropFiles();
      AssetBrowser.onDropFile(%assetDef.originalFilePath);
      AssetBrowser.onEndDropFiles();
      
      %module = AssetDatabase.getAssetModule(EditAssetPopup.assetId);
      
      //get the selected module data
      ImportAssetModuleList.setText(%module.ModuleId);
   }
}

//------------------------------------------------------------------------------
function AssetPreviewButton::onRightClick(%this)
{
   AssetBrowser.selectedAssetPreview = %this.getParent();
   EditAssetPopup.assetId = %this.getParent().moduleName @ ":" @ %this.getParent().assetName;
   %assetType = %this.getParent().assetType;
   
   //Do some enabling/disabling of options depending on asset type
   EditAssetPopup.enableItem(0, true);
   EditAssetPopup.enableItem(7, true);
   
   //Is it an editable type?
   if(%assetType $= "ImageAsset" || %assetType $= "GameObjectAsset" || %assetType $= "SoundAsset")
   {
      EditAssetPopup.enableItem(0, false);
   }
   
   //Is it an importable type?
   if(%assetType $= "GameObjectAsset" || %assetType $= "ComponentAsset" || %assetType $= "GUIAsset" || %assetType $= "LevelAsset"
       || %assetType $= "MaterialAsset" || %assetType $= "ParticleAsset"  || %assetType $= "PostEffectAsset" || %assetType $= "ScriptAsset"
       || %assetType $= "StateMachineAsset")
   {
      EditAssetPopup.enableItem(7, false);
   }
   
   EditAssetPopup.showPopup(Canvas);  
}

function AssetListPanel::onRightMouseDown(%this)
{
   AddNewAssetPopup.showPopup(Canvas);
}

//------------------------------------------------------------------------------
function AssetBrowser::refreshPreviews(%this)
{
   AssetBrowserFilterTree.onSelect(AssetBrowser.selectedItem);
}

function AssetBrowserFilterTree::onSelect(%this, %itemId)
{
	if(%itemId == 1)
		//can't select root
		return;
		
   //Make sure we have an actual module selected!
   %parentId = %this.getParentItem(%itemId);
      
   if(%parentId != 1)
      AssetBrowser.selectedModule = %this.getItemText(%parentId);//looks like we have one of the categories selected, not the module. Nab the parent so we have the correct thing!
   else
      AssetBrowser.selectedModule = %this.getItemText(%itemId);
      
   AssetBrowser.selectedItem = %itemId;
	
	//alright, we have a module or sub-filter selected, so now build our asset list based on that filter!
	echo("Asset Browser Filter Tree selected filter #:" @ %itemId);
	
	// manage schedule array properly
   if(!isObject(MatEdScheduleArray))
      new ArrayObject(MatEdScheduleArray);
	
	// if we select another list... delete all schedules that were created by 
   // previous load
   for( %i = 0; %i < MatEdScheduleArray.count(); %i++ )
      cancel(MatEdScheduleArray.getKey(%i));
	
	// we have to empty out the list; so when we create new schedules, these dont linger
   MatEdScheduleArray.empty();
   
   // manage preview array
   if(!isObject(AssetPreviewArray))
      new ArrayObject(AssetPreviewArray);
      
   // we have to empty out the list; so when we create new guicontrols, these dont linger
   AssetPreviewArray.empty();
   AssetBrowser-->materialSelection.deleteAllObjects();
   //AssetBrowser-->materialPreviewPagesStack.deleteAllObjects();

   %assetArray = new ArrayObject();

   //First, Query for our assets
   %assetQuery = new AssetQuery();
   %numAssetsFound = AssetDatabase.findAllAssets(%assetQuery);
   
	//module name per our selected filter:
	%moduleItemId = %this.getParentItem(%itemId);
	
	//check if we've selected a Module
	if(%moduleItemId == 1)
	{
	   %FilterModuleName = %this.getItemText(%itemId);
	}
	else
	{
	   %FilterModuleName = %this.getItemText(%moduleItemId);
	}
   
    //now, we'll iterate through, and find the assets that are in this module, and this category
    for( %i=0; %i < %numAssetsFound; %i++)
    {
	    %assetId = %assetQuery.getAsset(%i);
		
		//first, get the asset's module, as our major categories
		%module = AssetDatabase.getAssetModule(%assetId);
		
		%moduleName = %module.moduleId;
		
		if(%FilterModuleName $= %moduleName)
		{
			//it's good, so test that the category is right!
			%assetType = AssetDatabase.getAssetCategory(%assetId);
			if(%assetType $= "")
			{
			   %assetType = AssetDatabase.getAssetType(%assetId);
			}
			
			if(%this.getItemText(%itemId) $= %assetType || (%assetType $= "" && %this.getItemText(%itemId) $= "Misc")
			   || %moduleItemId == 1)
			{
				//stop adding after previewsPerPage is hit
				%assetName = AssetDatabase.getAssetName(%assetId);
				
				%searchText = AssetBrowserSearchFilter.getText();
				if(%searchText !$= "\c2Filter...")
				{
					if(strstr(strlwr(%assetName), strlwr(%searchText)) != -1)
						%assetArray.add( %moduleName, %assetId);
				}
				else
				{
					//got it.	
					%assetArray.add( %moduleName, %assetId );
				}
			}
		}
   }

	AssetBrowser.currentPreviewPage = 0;
	AssetBrowser.totalPages = 1;
	
	for(%i=0; %i < %assetArray.count(); %i++)
		AssetBrowser.buildPreviewArray( %assetArray.getValue(%i), %assetArray.getKey(%i) );
   
   AssetBrowser.loadImages( 0 );
}

function AssetBrowserFilterTree::onRightMouseDown(%this, %itemId)
{
   if( %this.getSelectedItemsCount() > 0 && %itemId != 1)
   {
      //AddNewAssetPopup.showPopup(Canvas);  
      
      //We have something clicked, so figure out if it's a sub-filter or a module filter, then push the correct
      //popup menu
      if(%this.getParentItem(%itemId) == 1)
      {
         //yep, module, push the all-inclusive popup  
         EditModulePopup.showPopup(Canvas); 
         //also set the module value for creation info
         AssetBrowser.selectedModule = %this.getItemText(%itemId);
      }
      else
      {
         //get the parent, and thus our module
         %moduleId = %this.getParentItem(%itemId);
         
         //set the module value for creation info
         AssetBrowser.selectedModule = %this.getItemText(%moduleId);
         
         if(%this.getItemText(%itemId) $= "ComponentAsset")
         {
            AddNewComponentAssetPopup.showPopup(Canvas);
            //Canvas.popDialog(AssetBrowser_newComponentAsset); 
	         //AssetBrowser_newComponentAsset-->AssetBrowserModuleList.setText(AssetBrowser.selectedModule);
         }
         else
         {
            
         }
      }
   }
   else if( %this.getSelectedItemsCount() > 0 && %itemId == 1)
   {
      AddNewModulePopup.showPopup(Canvas); 
   }
}

//
//
function AssetBrowserSearchFilterText::onWake( %this )
{
   /*%filter = %this.treeView.getFilterText();
   if( %filter $= "" )
      %this.setText( "\c2Filter..." );
   else
      %this.setText( %filter );*/
}

//------------------------------------------------------------------------------

function AssetBrowserSearchFilterText::onGainFirstResponder( %this )
{
   %this.selectAllText();
}

//---------------------------------------------------------------------------------------------

// When Enter is pressed in the filter text control, pass along the text of the control
// as the treeview's filter.
function AssetBrowserSearchFilterText::onReturn( %this )
{
   %text = %this.getText();
   if( %text $= "" )
      %this.reset();
   else
   {
      //%this.treeView.setFilterText( %text );
	  %curItem = AssetBrowserFilterTree.getSelectedItem();
	  AssetBrowserFilterTree.onSelect(%curItem);
   }
}

//---------------------------------------------------------------------------------------------

function AssetBrowserSearchFilterText::reset( %this )
{
   %this.setText( "\c2Filter..." );
   //%this.treeView.clearFilterText();
   %curItem = AssetBrowserFilterTree.getSelectedItem();
   AssetBrowserFilterTree.onSelect(%curItem);
}

//---------------------------------------------------------------------------------------------

function AssetBrowserSearchFilterText::onClick( %this )
{
   %this.textCtrl.reset();
}

//
//
//
function AssetBrowser::reloadModules(%this)
{
   ModuleDatabase.unloadGroup("Game");
   
   %modulesList = ModuleDatabase.findModules();
   
   %count = getWordCount(%modulesList);
   
   for(%i=0; %i < %count; %i++)
   {
      %moduleId = getWord(%modulesList, %i).ModuleId;
      ModuleDatabase.unloadExplicit(%moduleId);
   }

   ModuleDatabase.scanModules();
   
   %modulesList = ModuleDatabase.findModules();
   
   %count = getWordCount(%modulesList);
   
   for(%i=0; %i < %count; %i++)
   {
      %moduleId = getWord(%modulesList, %i).ModuleId;
      ModuleDatabase.loadExplicit(%moduleId);
   }
   
   //ModuleDatabase.loadGroup("Game");
}
//

function AssetPreviewButton::onMouseDragged(%this)
{
   %payload = new GuiBitmapButtonCtrl();
   %payload.assignFieldsFrom( %this );
   %payload.className = "AssetPreviewControl";
   %payload.position = "0 0";
   %payload.dragSourceControl = %this;
   %payload.bitmap = %this.icon;
   %payload.extent.x /= 2;
   %payload.extent.y /= 2;
   
   %xOffset = getWord( %payload.extent, 0 ) / 2;
   %yOffset = getWord( %payload.extent, 1 ) / 2;
   
   // Compute the initial position of the GuiDragAndDrop control on the cavas based on the current
   // mouse cursor position.
   
   %cursorpos = Canvas.getCursorPos();
   %xPos = getWord( %cursorpos, 0 ) - %xOffset;
   %yPos = getWord( %cursorpos, 1 ) - %yOffset;
   
   if(!isObject(EditorDragAndDropLayer))
   {
      new GuiControl(EditorDragAndDropLayer)
      {
         position = "0 0";
         extent = Canvas.extent;
      };
   }
   
   // Create the drag control.
   %ctrl = new GuiDragAndDropControl()
   {
      canSaveDynamicFields    = "0";
      Profile                 = "GuiSolidDefaultProfile";
      HorizSizing             = "right";
      VertSizing              = "bottom";
      Position                = %xPos SPC %yPos;
      extent                  = %payload.extent;
      MinExtent               = "4 4";
      canSave                 = "1";
      Visible                 = "1";
      hovertime               = "1000";

      // Let the GuiDragAndDropControl delete itself on mouse-up.  When the drag is aborted,
      // this not only deletes the drag control but also our payload.
      deleteOnMouseUp         = true;
      
      useWholeCanvas = true;

      // To differentiate drags, use the namespace hierarchy to classify them.
      // This will allow a color swatch drag to tell itself apart from a file drag, for example.
      class                   = "AssetPreviewControlType_AssetDrop";
   };
   
   // Add the temporary color swatch to the drag control as the payload.
   %ctrl.add( %payload );
   
   // Start drag by adding the drag control to the canvas and then calling startDragging().
   //Canvas.getContent().add( %ctrl );
   EditorDragAndDropLayer.add(%ctrl);
   Canvas.pushDialog(EditorDragAndDropLayer);
   
   %ctrl.startDragging( %xOffset, %yOffset );
}

function AssetPreviewButton::onControlDragCancelled(%this)
{
   Canvas.popDialog(EditorDragAndDropLayer);
}

function AssetPreviewButton::onControlDropped( %this, %payload, %position )
{
   Canvas.popDialog(EditorDragAndDropLayer);
   
   // Make sure this is a color swatch drag operation.
   if( !%payload.parentGroup.isInNamespaceHierarchy( "AssetPreviewControlType_AssetDrop" ) )
      return;

   // If dropped on same button whence we came from,
   // do nothing.

   if( %payload.dragSourceControl == %this )
      return;

   // If a swatch button control is dropped onto this control,
   // copy it's color.

   if( %payload.isMemberOfClass( "AssetPreviewButton" ) )
   {
      // If the swatch button is part of a color-type inspector field,
      // remember the inspector field so we can later set the color
      // through it.

      if( %this.parentGroup.isMemberOfClass( "GuiInspectorTypeColorI" ) )
         %this.parentGroup.apply( ColorFloatToInt( %payload.color ) );
      else if( %this.parentGroup.isMemberOfClass( "GuiInspectorTypeColorF" ) )
         %this.parentGroup.apply( %payload.color );
      else
         %this.setColor( %payload.color );
   }
}

function EWorldEditor::onControlDropped( %this, %payload, %position )
{
   Canvas.popDialog(EditorDragAndDropLayer);
   
   // Make sure this is a color swatch drag operation.
   if( !%payload.parentGroup.isInNamespaceHierarchy( "AssetPreviewControlType_AssetDrop" ) )
      return;

   // If dropped on same button whence we came from,
   // do nothing.

   if( %payload.dragSourceControl == %this )
      return;

   %assetType = %payload.dragSourceControl.parentGroup.assetType;
   
   %pos = EWCreatorWindow.getCreateObjectPosition(); //LocalClientConnection.camera.position; 
   %module = %payload.dragSourceControl.parentGroup.moduleName;
   %asset = %payload.dragSourceControl.parentGroup.assetName;
   
   if(%assetType $= "ImageAsset")
   {
      echo("DROPPED AN IMAGE ON THE EDITOR WINDOW!");  
   }
   else if(%assetType $= "ShapeAsset")
   {
      echo("DROPPED A SHAPE ON THE EDITOR WINDOW!"); 
      
      %newEntity = new Entity()
      {
         position = %pos;
         
         new MeshComponent()
         {
            MeshAsset = %module @ ":" @ %asset;
         };
         
         //new CollisionComponent(){};
      };
      
      MissionGroup.add(%newEntity);
      
      EWorldEditor.clearSelection();
      EWorldEditor.selectObject(%newEntity);
   }
   else if(%assetType $= "MaterialAsset")
   {
      echo("DROPPED A MATERIAL ON THE EDITOR WINDOW!");  
   }
   else if(%assetType $= "GameObjectAsset")
   {
      echo("DROPPED A GAME OBJECT ON THE EDITOR WINDOW!");  
      
      %GO = spawnGameObject(%asset, true);
      
      %pos = EWCreatorWindow.getCreateObjectPosition(); //LocalClientConnection.camera.position; 
      
      %GO.position = %pos;
      
      EWorldEditor.clearSelection();
      EWorldEditor.selectObject(%GO);
   }
   else if(%assetType $= "ComponentAsset")
   {
      %newEntity = new Entity()
      {
         position = %pos;
      };
      
      %assetDef = AssetDatabase.acquireAsset(%module @ ":" @ %asset);
      
      if(%assetDef.componentClass $= "Component")
         eval("$tmpVar = new " @ %assetDef.componentClass @ "() { class = " @ %assetDef.componentName @ "; }; %newEntity.add($tmpVar);");
      else
         eval("$tmpVar = new " @ %assetDef.componentClass @ "() {}; %newEntity.add($tmpVar);");
         
      MissionGroup.add(%newEntity);
      
      EWorldEditor.clearSelection();
      EWorldEditor.selectObject(%newEntity);
   }
   else if(%assetType $= "ScriptAsset") //do we want to do it this way?
   {
      %newEntity = new Entity()
      {
         position = %pos;
         class = %asset;
      };

      MissionGroup.add(%newEntity);
      
      EWorldEditor.clearSelection();
      EWorldEditor.selectObject(%newEntity);
   }
   
   EWorldEditor.isDirty = true;
}

function GuiInspectorTypeShapeAssetPtr::onControlDropped( %this, %payload, %position )
{
   Canvas.popDialog(EditorDragAndDropLayer);
   
   // Make sure this is a color swatch drag operation.
   if( !%payload.parentGroup.isInNamespaceHierarchy( "AssetPreviewControlType_AssetDrop" ) )
      return;

   %assetType = %payload.dragSourceControl.parentGroup.assetType;
   
   if(%assetType $= "ShapeAsset")
   {
      echo("DROPPED A SHAPE ON A SHAPE ASSET COMPONENT FIELD!");  
      
      %module = %payload.dragSourceControl.parentGroup.moduleName;
      %asset = %payload.dragSourceControl.parentGroup.assetName;
      
      %targetComponent = %this.ComponentOwner;
      %targetComponent.MeshAsset = %module @ ":" @ %asset;
      
      //Inspector.refresh();
   }
   
   EWorldEditor.isDirty= true;
}

function GuiInspectorTypeImageAssetPtr::onControlDropped( %this, %payload, %position )
{
   Canvas.popDialog(EditorDragAndDropLayer);
   
   // Make sure this is a color swatch drag operation.
   if( !%payload.parentGroup.isInNamespaceHierarchy( "AssetPreviewControlType_AssetDrop" ) )
      return;

   %assetType = %payload.dragSourceControl.parentGroup.assetType;
   
   if(%assetType $= "ImageAsset")
   {
      echo("DROPPED A IMAGE ON AN IMAGE ASSET COMPONENT FIELD!");  
   }
   
   EWorldEditor.isDirty = true;
}

function GuiInspectorTypeMaterialAssetPtr::onControlDropped( %this, %payload, %position )
{
   Canvas.popDialog(EditorDragAndDropLayer);
   
   // Make sure this is a color swatch drag operation.
   if( !%payload.parentGroup.isInNamespaceHierarchy( "AssetPreviewControlType_AssetDrop" ) )
      return;

   %assetType = %payload.dragSourceControl.parentGroup.assetType;
   
   if(%assetType $= "MaterialAsset")
   {
      echo("DROPPED A MATERIAL ON A MATERIAL ASSET COMPONENT FIELD!");  
   }
   
   EWorldEditor.isDirty = true;
}

function AssetBrowserFilterTree::onControlDropped( %this, %payload, %position )
{
   Canvas.popDialog(EditorDragAndDropLayer);
   
   if( !%payload.parentGroup.isInNamespaceHierarchy( "AssetPreviewControlType_AssetDrop" ) )
      return;
      
   %assetType = %payload.dragSourceControl.parentGroup.assetType;
   %assetName = %payload.dragSourceControl.parentGroup.assetName;
   %moduleName = %payload.dragSourceControl.parentGroup.moduleName;
   
   echo("DROPPED A " @ %assetType @ " ON THE ASSET BROWSER NAVIGATION TREE!");
   
   %item = %this.getItemAtPosition(%position);
   
   echo("DROPPED IT ON ITEM " @ %item);
   
   %parent = %this.getParentItem(%item);
   
   if(%parent == 1)
   {
      //we're a module entry, cool
      %targetModuleName = %this.getItemText(%item);
      echo("DROPPED IT ON MODULE " @ %targetModuleName);   
      
      if(%moduleName !$= %targetModuleName)
      {
         //we're trying to move the asset to a different module!
         MessageBoxYesNo( "Move Asset", "Do you wish to move asset " @ %assetName @ " to module " @ %targetModuleName @ "?", 
               "AssetBrowser.moveAsset("@%assetName@", "@%targetModuleName@");", "");  
      }
   }
}