function AssetBrowser::createLevelAsset(%this)
{
   %moduleName = AssetBrowser.newAssetSettings.moduleName;
   %modulePath = "data/" @ %moduleName;
   
   %assetName = AssetBrowser.newAssetSettings.assetName;
   
   %tamlpath = %modulePath @ "/levels/" @ %assetName @ ".asset.taml";
   %levelPath = %modulePath @ "/levels/" @ %assetName @ ".mis";
   
   %asset = new LevelAsset()
   {
      AssetName = %assetName;
      versionId = 1;
      LevelFile = %assetName @ ".mis";
      LevelName = AssetBrowser.newAssetSettings.levelName;
      AssetDescription = AssetBrowser.newAssetSettings.description;
      PreviewImage = AssetBrowser.newAssetSettings.levelPreviewImage;
   };
   
   TamlWrite(%asset, %tamlpath);
   
   if(!pathCopy("tools/levels/BlankRoom.mis", %levelPath, false))
   {
      echo("Unable to copy template level file!");
   }

	%moduleDef = ModuleDatabase.findModule(%moduleName, 1);
	AssetDatabase.addDeclaredAsset(%moduleDef, %tamlpath);

	AssetBrowser.loadFilters();
	
	%treeItemId = AssetBrowserFilterTree.findItemByName(%moduleName);
	%smItem = AssetBrowserFilterTree.findChildItemByName(%treeItemId, "Levels");
	
	AssetBrowserFilterTree.onSelect(%smItem);
	
	return %tamlpath;  
}

function AssetBrowser::editLevelAsset(%this, %assetDef)
{
   schedule( 1, 0, "EditorOpenMission", %assetDef);
}
   
function AssetBrowser::buildLevelAssetPreview(%this, %assetDef, %previewData)
{
   %previewData.assetName = %assetDef.assetName;
   %previewData.assetPath = %assetDef.levelFile;
   %previewData.doubleClickCommand = "schedule( 1, 0, \"EditorOpenMission\", "@%assetDef@");";
   
   %levelPreviewImage = %assetDesc.PreviewImage;
         
   if(isFile(%levelPreviewImage))
      %previewData.previewImage = %levelPreviewImage;
   else
      %previewData.previewImage = "tools/assetBrowser/art/levelIcon";
   
   %previewData.assetFriendlyName = %assetDef.assetName;
   %previewData.assetDesc = %assetDef.description;
   %previewData.tooltip = %assetDef.assetName;
}