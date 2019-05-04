function AssetBrowser::createGUIAsset(%this)
{
   %moduleName = AssetBrowser.newAssetSettings.moduleName;
   %modulePath = "data/" @ %moduleName;
      
   %assetName = AssetBrowser.newAssetSettings.assetName;
   
   %tamlpath = %modulePath @ "/GUIs/" @ %assetName @ ".asset.taml";
   %guipath = %modulePath @ "/GUIs/" @ %assetName @ ".gui";
   %scriptPath = %modulePath @ "/GUIs/" @ %assetName @ ".cs";
   
   %asset = new GUIAsset()
   {
      AssetName = %assetName;
      versionId = 1;
      scriptFile = %assetName @ ".cs";
      guiFile = %assetName @ ".gui";
   };
   
   TamlWrite(%asset, %tamlpath);
   
   %file = new FileObject();
   
   if(%file.openForWrite(%guipath))
	{
	   %file.writeline("//--- OBJECT WRITE BEGIN ---");
		%file.writeline("%guiContent = new GuiControl(" @ %assetName @ ") {");
		%file.writeline("   position = \"0 0\";");
		%file.writeline("   extent = \"100 100\";");
		%file.writeline("};");
		%file.writeline("//--- OBJECT WRITE END ---");
		
		%file.close();
	}
	
	if(%file.openForWrite(%scriptPath))
	{
		%file.writeline("function " @ %assetName @ "::onWake(%this)\n{\n\n}\n");
		%file.writeline("function " @ %assetName @ "::onSleep(%this)\n{\n\n}\n");
		
		%file.close();
	}
	
	//load the gui
	exec(%guipath);
	exec(%scriptPath);
	
	%moduleDef = ModuleDatabase.findModule(%moduleName, 1);
	AssetDatabase.addDeclaredAsset(%moduleDef, %tamlpath);

	AssetBrowser.loadFilters();
	
	%treeItemId = AssetBrowserFilterTree.findItemByName(%moduleName);
	%smItem = AssetBrowserFilterTree.findChildItemByName(%treeItemId, "GUIs");
	
	AssetBrowserFilterTree.onSelect(%smItem);
	
	return %tamlpath;  
}

function AssetBrowser::editGUIAsset(%this, %assetDef)
{
   if(!isObject(%assetDef.assetName))
   {
      exec(%assetDef.GUIFilePath);
      exec(%assetDef.mScriptFilePath);
   }
   
   GuiEditContent(%assetDef.assetName);  
}

function AssetBrowser::buildGUIAssetPreview(%this, %assetDef, %previewData)
{
   %previewData.assetName = %assetDef.assetName;
   %previewData.assetPath = %assetDef.GUIFilePath;
   %previewData.doubleClickCommand = "";
   
   %previewData.previewImage = "tools/assetBrowser/art/guiIcon";
   
   %previewData.assetFriendlyName = %assetDef.assetName;
   %previewData.assetDesc = %assetDef.description;
   %previewData.tooltip = %assetDef.assetName;
}