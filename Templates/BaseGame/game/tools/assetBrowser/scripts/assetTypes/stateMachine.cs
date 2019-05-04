function AssetBrowser::createStateMachineAsset(%this)
{
   %assetName = AssetBrowser.newAssetSettings.assetName;
   %moduleName = AssetBrowser.selectedModule;
      
   %assetQuery = new AssetQuery();
   
   %matchingAssetCount = AssetDatabase.findAssetName(%assetQuery, %assetName);
   
   %i=1;
   while(%matchingAssetCount > 0)
   {
      %newAssetName = %assetName @ %i;
      %i++;
      
      %matchingAssetCount = AssetDatabase.findAssetName(%assetQuery, %newAssetName);
   }
   
   %assetName = %newAssetName;
   
   %assetQuery.delete();
   
   %tamlpath = "data/" @ %moduleName @ "/stateMachines/" @ %assetName @ ".asset.taml";
   %smFilePath = "data/" @ %moduleName @ "/stateMachines/" @ %assetName @ ".xml";
   
   %asset = new StateMachineAsset()
   {
      AssetName = %assetName;
      versionId = 1;
      stateMachineFile = %assetName @ ".xml";
   };
   
   %xmlDoc = new SimXMLDocument();
   %xmlDoc.saveFile(%smFilePath);
   %xmlDoc.delete();
   
   TamlWrite(%asset, %tamlpath);
   
   //Now write our XML file
   %xmlFile = new FileObject();
	%xmlFile.openForWrite(%smFilePath);
	%xmlFile.writeLine("<StateMachine>");
	%xmlFile.writeLine("</StateMachine>");
	%xmlFile.close();
   
   %moduleDef = ModuleDatabase.findModule(%moduleName, 1);
	AssetDatabase.addDeclaredAsset(%moduleDef, %tamlpath);

	AssetBrowser.loadFilters();
	
	%treeItemId = AssetBrowserFilterTree.findItemByName(%moduleName);
	%smItem = AssetBrowserFilterTree.findChildItemByName(%treeItemId, "StateMachines");
	
	AssetBrowserFilterTree.onSelect(%smItem);
   
	return %tamlpath;
}

function AssetBrowser::editStateMachineAsset(%this, %assetDef)
{
   eval("AssetBrowser.tempAsset = new " @ %assetDef.getClassName() @ "();");
   AssetBrowser.tempAsset.assignFieldsFrom(%assetDef);
   
   SMAssetEditInspector.inspect(AssetBrowser.tempAsset);  
   AssetBrowser_editAsset.editedAssetId = EditAssetPopup.assetId;
   AssetBrowser_editAsset.editedAsset = AssetBrowser.tempAsset;
   
   //remove some of the groups we don't need:
   for(%i=0; %i < SMAssetEditInspector.getCount(); %i++)
   {
      %caption = SMAssetEditInspector.getObject(%i).caption;
      
      if(%caption $= "Ungrouped" || %caption $= "Object" || %caption $= "Editing" 
         || %caption $= "Persistence" || %caption $= "Dynamic Fields")
      {
         SMAssetEditInspector.remove(SMAssetEditInspector.getObject(%i));
         %i--;
      }
   }

   Canvas.pushDialog(StateMachineEditor);
   StateMachineEditor.loadStateMachineAsset(EditAssetPopup.assetId);
   StateMachineEditor-->Window.text = "State Machine Editor ("@EditAssetPopup.assetId@")";
}

function AssetBrowser::duplicateStateMachineAsset(%this, %assetDef)
{
   //Check if we have a target module, if not we need to select one
   if(%targetModule $= "")
   {
      error("AssetBrowser::duplicateStateMachineAsset - No target module selected!");
      return;
   }
   
   %assetId = %assetDef.getAssetId();
   %assetName = AssetDatabase.getAssetName(%assetId);
   
   //First step, copy the files
   %modulePath = "data/" @ %targetModule @ "/stateMachines/";
   
   if(!isDirectory(%modulePath))
      createPath(%modulePath);
   
   %assetFile = AssetDatabase.getAssetFilePath(%assetId);
   %stateMachineFile = %assetDef.stateMachineFile;
    
   echo("AssetBrowser::duplicateGameObjectAsset - duplicating! " @ %assetId @ " to " @ %targetModule);
   
   %tamlPath = %modulePath @ fileName(%assetFile);
   
   pathCopy(%assetFile, %tamlPath);
   pathCopy(%stateMachineFile, %modulePath @ fileName(%stateMachineFile));
   
   echo("AssetBrowser::duplicateStateMachineAsset - duplicated!");
   
   //Register the asset
   %moduleDef = ModuleDatabase.findModule(%targetModule, 1);
   
	AssetDatabase.addDeclaredAsset(%moduleDef, %tamlPath);

   //Refresh the browser
	AssetBrowser.loadFilters();
	
	//Ensure our context is set
	%treeItemId = AssetBrowserFilterTree.findItemByName(%targetModule);
	%smItem = AssetBrowserFilterTree.findChildItemByName(%treeItemId, "StateMachineAsset");
	
	AssetBrowserFilterTree.selectItem(%smItem);
	
	//Rename it for convenience
	AssetBrowser.performRenameAsset(%assetName, "New" @ %assetName);
	
	//Expand and refresh the target module
   AssetBrowserFilterTree.expandItem(%treeItemId,true);
	
	AssetBrowserFilterTree.buildVisibleTree();
}

function AssetBrowser::renameGameObjectAsset(%this, %assetDef, %newAssetId, %originalName, %newName)
{
   %assetPath = AssetDatabase.getAssetFilePath(%newAssetId);
   
   //rename the file to match
   %path = filePath(%assetPath);
         
   %oldScriptFilePath = %assetDef.stateMachineFile;
   %scriptFilePath = filePath(%assetDef.stateMachineFile);
   %scriptExt = fileExt(%assetDef.stateMachineFile);
   
   %newScriptFileName = %scriptFilePath @ "/" @ %newName @ %scriptExt;
   %newAssetFile = %path @ "/" @ %newName @ ".asset.taml";
   
   %assetDef.stateMachineFile = %newScriptFileName;
   
   TamlWrite(%assetDef, %newAssetFile);
   fileDelete(%assetPath);
   
   pathCopy(%oldScriptFilePath, %newScriptFileName);
   fileDelete(%oldScriptFilePath); 
}

function AssetBrowser::buildStateMachineAssetPreview(%this, %assetDef, %previewData)
{
   %previewData.assetName = %assetDef.assetName;
   %previewData.assetPath = %assetDef.scriptFile;
   %previewData.doubleClickCommand = "AssetBrowser.editStateMachineAsset( "@%assetDef@" );";
   
   %previewData.previewImage = "tools/assetBrowser/art/stateMachineIcon";
   
   %previewData.assetFriendlyName = %assetDef.friendlyName;
   %previewData.assetDesc = %assetDef.description;
   %previewData.tooltip = %assetDef.friendlyName @ "\n" @ %assetDef;
}