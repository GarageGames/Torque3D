function AssetBrowser::createScriptAsset(%this)
{
   %moduleName = AssetBrowser.newAssetSettings.moduleName;
   %modulePath = "data/" @ %moduleName;
      
   %assetName = AssetBrowser.newAssetSettings.assetName;      
   
   %tamlpath = %modulePath @ "/scripts/" @ %assetName @ ".asset.taml";
   %scriptPath = %modulePath @ "/scripts/" @ %assetName @ ".cs";
   
   %asset = new ScriptAsset()
   {
      AssetName = %assetName;
      versionId = 1;
      scriptFile = %assetName @ ".cs";
   };
   
   TamlWrite(%asset, %tamlpath);
   
   %moduleDef = ModuleDatabase.findModule(%moduleName, 1);
	AssetDatabase.addDeclaredAsset(%moduleDef, %tamlpath);

	AssetBrowser.loadFilters();
	
	%treeItemId = AssetBrowserFilterTree.findItemByName(%moduleName);
	%smItem = AssetBrowserFilterTree.findChildItemByName(%treeItemId, "ScriptAsset");
	
	AssetBrowserFilterTree.onSelect(%smItem);
	
	%file = new FileObject();
   
   if(%file.openForWrite(%scriptPath))
	{
		%file.close();
	}
   
	return %tamlpath;
}

function AssetBrowser::editScriptAsset(%this, %assetDef)
{
   %scriptFile = %assetDef.scriptFile;
   
   EditorOpenFileInTorsion(makeFullPath(%scriptFile), 0);
}

function AssetBrowser::duplicateScriptAsset(%this, %assetDef, %targetModule)
{
}

function AssetBrowser::importScriptAsset(%this, %assetId)
{
}

function AssetBrowser::dragAndDropScriptAsset(%this, %assetDef, %dropTarget)
{
   if(!isObject(%dropTarget))
      return;
}

function AssetBrowser::renameScriptAsset(%this, %assetDef, %originalName, %newName)
{
}

function AssetBrowser::deleteScriptAsset(%this, %assetDef)
{
}

function AssetBrowser::buildScriptAssetPreview(%this, %assetDef, %previewData)
{
   %previewData.assetName = %assetDef.assetName;
   %previewData.assetPath = %assetDef.scriptFile;
   %previewData.doubleClickCommand = "EditorOpenFileInTorsion( \""@%previewData.assetPath@"\", 0 );";
   
   if(%assetDef.isServerSide)
      %previewData.previewImage = "tools/assetBrowser/art/serverScriptIcon";
   else
      %previewData.previewImage = "tools/assetBrowser/art/clientScriptIcon";
   
   %previewData.assetFriendlyName = %assetDef.assetName;
   %previewData.assetDesc = %assetDef.description;
   %previewData.tooltip = %assetDef.assetName;
}

function GuiInspectorTypeScriptAssetPtr::onClick( %this, %fieldName )
{
   //Get our data
   %obj = %this.getInspector().getInspectObject(0);
}