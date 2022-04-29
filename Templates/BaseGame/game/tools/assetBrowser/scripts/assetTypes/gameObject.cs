function AssetBrowser::createGameObjectAsset(%this)
{
   GameObjectCreatorObjectName.text = "";
   
   %activeSelection = EWorldEditor.getActiveSelection();
   if( %activeSelection.getCount() == 0 )
      return;
      
   GameObjectCreator.selectedEntity = %activeSelection.getObject( 0 );

   Canvas.pushDialog(GameObjectCreator);
}

function AssetBrowser::editGameObjectAsset(%this, %assetDef)
{
   //We have no dedicated GO editor for now, so just defer to the script editing aspect
   %this.editGameObjectAssetScript(%assetDef);
}

function AssetBrowser::editGameObjectAssetScript(%this, %assetDef)
{
   %scriptFile = %assetDef.scriptFile;
   
   EditorOpenFileInTorsion(makeFullPath(%scriptFile), 0);  
}

function AssetBrowser::applyInstanceToGameObject(%this, %assetDef)
{
   %obj = EditGameObjectAssetPopup.object;
   
   //TODO add proper validation against the original GO asset
   %obj.dirtyGameObject = true;
   
   TamlWrite(%obj, %assetDef.TAMLFilePath);
}

function AssetBrowser::duplicateGameObjectAsset(%this, %assetDef, %targetModule)
{
   //Check if we have a target module, if not we need to select one
   if(%targetModule $= "")
   {
      error("AssetBrowser::duplicateGameObjectAsset - No target module selected!");
      return;
   }
   
   %assetId = %assetDef.getAssetId();
   %assetName = AssetDatabase.getAssetName(%assetId);
   
   //First step, copy the files
   %modulePath = "data/" @ %targetModule @ "/gameObjects/";
   
   if(!isDirectory(%modulePath))
      createPath(%modulePath);
   
   %assetFile = AssetDatabase.getAssetFilePath(%assetId);
   %scriptFile = %assetDef.scriptFile;
   %gameObjectFile = %assetDef.TAMLFile;
   
   echo("AssetBrowser::duplicateGameObjectAsset - duplicating! " @ %assetId @ " to " @ %targetModule);
   
   %tamlPath = %modulePath @ fileName(%assetFile);
   
   pathCopy(%assetFile, %tamlPath);
   pathCopy(%scriptFile, %modulePath @ fileName(%scriptFile));
   pathCopy(%gameObjectFile, %modulePath @ fileName(%gameObjectFile));
   
   echo("AssetBrowser::duplicateGameObjectAsset - duplicated!");
   
   //Register the asset
   %moduleDef = ModuleDatabase.findModule(%targetModule, 1);
   
	AssetDatabase.addDeclaredAsset(%moduleDef, %tamlPath);

   //Refresh the browser
	AssetBrowser.loadFilters();
	
	//Ensure our context is set
	%treeItemId = AssetBrowserFilterTree.findItemByName(%targetModule);
	%smItem = AssetBrowserFilterTree.findChildItemByName(%treeItemId, "GameObjectAsset");
	
	AssetBrowserFilterTree.selectItem(%smItem);
	
	//Rename it for convenience
	AssetBrowser.performRenameAsset(%assetName, "New" @ %assetName);
	
	//Expand and refresh the target module
   AssetBrowserFilterTree.expandItem(%treeItemId,true);
	
	AssetBrowserFilterTree.buildVisibleTree();
}

//not used
function AssetBrowser::importGameObjectAsset(%this, %assetId)
{
   
}

function AssetBrowser::dragAndDropGameObjectAsset(%this, %assetDef, %dropTarget)
{
   if(!isObject(%dropTarget))
      return;
      
   if(%dropTarget.getId() == EWorldEditor.getId())
   {
      if(isObject(%assetDef))
      {
         %gameObject = %assetDef.createObject(); 
         
         getScene(0).add(%gameObject);
         
         %pos = EWCreatorWindow.getCreateObjectPosition(); //LocalClientConnection.camera.position; 
      
         %gameObject.position = %pos;
         
         EWorldEditor.clearSelection();
         EWorldEditor.selectObject(%gameObject); 
      }
      else
      {
         error("WorldEditor::onControlDropped - unable to create GameObject"); 
      }
   }
}

function AssetBrowser::renameGameObjectAsset(%this, %assetDef, %newAssetId, %originalName, %newName)
{
   %oldScriptFilePath = %assetDef.scriptFile;
   %scriptFilePath = filePath(%assetDef.scriptFile);
   %scriptExt = fileExt(%assetDef.scriptFile);
   
   %oldGOFilePath = %assetDef.TAMLFile;
   
   %filepath = AssetDatabase.getAssetFilePath(%assetDef.getAssetId());
   %path = makeRelativePath(filePath(%filepath));
   
   %newScriptFileName = %path @ "/" @ %newName @ %scriptExt;
   %newAssetFile = %path @ "/" @ %newName @ ".asset.taml";
   %newGOFile = %path @ "/" @ %newName @ ".taml";
   
   %assetDef.gameObjectName = %newName;
   %assetDef.scriptFile = %newScriptFileName;
   %assetDef.TAMLFile = %newGOFile;
   
   TamlWrite(%assetDef, %newAssetFile);
   fileDelete(%filepath);
   
   //Quick check, if we duplicated the asset to a new module, the old path may not line up so we'll want to update that to be relevent
   if(filePath(%oldScriptFilePath) !$= %path)
   {
     %oldFileBase = fileName(%oldScriptFilePath);
     %oldScriptFilePath = %path @ "/" @ %oldFileBase;
   }
   
   %scriptFileCopySuccess = pathCopy(%oldScriptFilePath, %newScriptFileName);
   fileDelete(%oldScriptFilePath);
   
   if(!%scriptFileCopySuccess)
      error("AssetBrowser::renameGameObjectAsset - unable to copy scriptFile");
   
   if(filePath(%oldGOFilePath) !$= %path)
   {
     %oldFileBase = fileName(%oldGOFilePath);
     %oldGOFilePath = %path @ "/" @ %oldFileBase;
   }
   
   %goFileCopySuccess = pathCopy(%oldGOFilePath, %newGOFile);
   fileDelete(%oldGOFilePath);
   
   if(!%scriptFileCopySuccess)
      error("AssetBrowser::renameGameObjectAsset - unable to copy gameObject");
   
   //Go through our scriptfile and replace the old namespace with the new
   %editedFileContents = "";
   
   %file = new FileObject();
   if ( %file.openForRead( %newScriptFileName ) ) 
   {
      while ( !%file.isEOF() ) 
      {
         %line = %file.readLine();
         %line = trim( %line );
         
         %editedFileContents = %editedFileContents @ strreplace(%line, %originalName, %newName) @ "\n";
      }
      
      %file.close();
   }
   
   if(%editedFileContents !$= "")
   {
      %file.openForWrite(%newScriptFileName);
      
      %file.writeline(%editedFileContents);
      
      %file.close();
   }
   
   exec(%newScriptFileName);
   
   %gameObj = TAMLRead(%newGOFile);
   
   %gameObj.className = %newName;
   %gameObj.GameObject = %assetDef.getAssetId();
   
   TAMLWrite(%gameObj, %newGOFile);
}

function AssetBrowser::buildGameObjectAssetPreview(%this, %assetDef, %previewData)
{
   %previewData.assetName = %assetDef.assetName;
   %previewData.assetPath = %assetDef.scriptFile;
   %previewData.doubleClickCommand = "EditorOpenFileInTorsion( "@%previewData.assetPath@", 0 );";
   
   %previewData.previewImage = "tools/assetBrowser/art/gameObjectIcon";
   
   %previewData.assetFriendlyName = %assetDef.gameObjectName;
   %previewData.assetDesc = %assetDef.description;
   %previewData.tooltip = %assetDef.gameObjectName;
}

function GuiInspectorTypeGameObjectAssetPtr::onClick( %this, %fieldName )
{
   //Get our data
   %obj = %this.getInspector().getInspectObject(0);
   
   EditGameObjectAssetPopup.object = %obj;
   
   %assetId = %obj.getFieldValue(%fieldName);
   
   if(%assetId !$= "")
   {
      EditGameObjectAssetPopup.assetId = %assetId;
      
      
      EditGameObjectAssetPopup.showPopup(Canvas);
   }
   else
   {
      //We've gotta be trying to create a GameObject, so kick that off  
      AssetBrowser.createGameObjectAsset();
   }
}