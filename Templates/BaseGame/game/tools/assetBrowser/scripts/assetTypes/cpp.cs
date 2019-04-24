function AssetBrowser::createCppAsset(%this)
{
   %moduleName = AssetBrowser.newAssetSettings.moduleName;
   %modulePath = "data/" @ %moduleName;
      
   %assetName = AssetBrowser.newAssetSettings.assetName;      
   
   %tamlpath = %modulePath @ "/source/" @ %assetName @ ".asset.taml";
   %codePath = %modulePath @ "/source/" @ %assetName @ ".cpp";
   %headerPath = %modulePath @ "/source/" @ %assetName @ ".h";
   
   //Do the work here
   %assetType = AssetBrowser.newAssetSettings.assetType;
   
   /*if(%assetType $= "CppStaticClassAsset" 
	   || %assetType $= "CppRegularClassAsset" 
	   || %assetType $= "CppGameObjectAsset"
	   || %assetType $= "CppComponentAsset"
      || %assetType $= "CppScriptClass")
   {
      
   }*/
   
   %asset = new CppAsset()
   {
      AssetName = %assetName;
      versionId = 1;
      codeFile = %codePath;
      headerFile = %headerPath;
   };
   
   TamlWrite(%asset, %tamlpath);
   
   %moduleDef = ModuleDatabase.findModule(%moduleName, 1);
	AssetDatabase.addDeclaredAsset(%moduleDef, %tamlpath);

	AssetBrowser.loadFilters();
	
	%treeItemId = AssetBrowserFilterTree.findItemByName(%moduleName);
	%smItem = AssetBrowserFilterTree.findChildItemByName(%treeItemId, "CppAsset");
	
	AssetBrowserFilterTree.onSelect(%smItem);
	
	%templateFilesPath = "tools/assetBrowser/scripts/templateFiles/";
	
	%file = new FileObject();
	%templateFile = new FileObject();
	
	if(%assetType $= "CppStaticClassAsset")
	{
	   %cppTemplateCodeFilePath = %templateFilesPath @ "CppStaticClassFile.cpp";
	   %cppTemplateHeaderFilePath = %templateFilesPath @ "CppStaticClassFile.h";
	}
   
   if(%file.openForWrite(%codePath) && %templateFile.openForRead(%cppTemplateCodeFilePath))
   {
      while( !%templateFile.isEOF() )
      {
         %line = %templateFile.readline();
         %line = strreplace( %line, "@", %assetName );
         
         %file.writeline(%line);
         echo(%line);
      }
      
      %file.close();
      %templateFile.close();
   }
   else
   {
      %file.close();
      %templateFile.close();
      
      warnf("CreateNewCppAsset - Something went wrong and we couldn't write the C++ code file!");
   }
   
   if(%file.openForWrite(%headerPath) && %templateFile.openForRead(%cppTemplateHeaderFilePath))
   {
      while( !%templateFile.isEOF() )
      {
         %line = %templateFile.readline();
         %line = strreplace( %line, "@", %assetName );
         
         %file.writeline(%line);
         echo(%line);
      }
      
      %file.close();
      %templateFile.close();
   }
   else
   {
      %file.close();
      %templateFile.close();
      
      warnf("CreateNewCppAsset - Something went wrong and we couldn't write the C++ header file!");
   }
	
	//Last, check that we have a C++ Module definition. If not, make one so anything important can be initialized on startup there
	%cppModuleFilePath = %modulePath @ "/source/" @ %moduleName @ ".cpp";
	if(!isFile(%cppModuleFilePath))
	{
	   %file = new FileObject();
	   %templateFile = new FileObject();
   
      if(%file.openForWrite(%cppModuleFilePath) && %templateFile.openForRead(%templateFilesPath @ "CppModuleFile.cpp"))
      {
         while( !%templateFile.isEOF() )
         {
            %line = %templateFile.readline();
            %line = strreplace( %line, "@", %moduleName );
            
            %file.writeline(%line);
            echo(%line);
         }
         
         %file.close();
         %templateFile.close();
      }
      else
      {
         %file.close();
         %templateFile.close();
         
         warnf("CreateNewCppAsset - Something went wrong and we couldn't write the C++ module file!");
      }
	}
   
	return %tamlpath;
}

function AssetBrowser::buildCppAssetPreview(%this, %assetDef, %previewData)
{
   %previewData.assetName = %assetDef.assetName;
   %previewData.assetPath = %assetDef.codeFilePath;
   %previewData.doubleClickCommand = "echo(\"Not yet implemented to edit C++ files from the editor\");";//"EditorOpenFileInTorsion( "@%previewData.assetPath@", 0 );";
   
   %previewData.previewImage = "tools/assetBrowser/art/cppIcon";
   
   %previewData.assetFriendlyName = %assetDef.assetName;
   %previewData.assetDesc = %assetDef.description;
   %previewData.tooltip = %assetDef.assetName;
}