function AssetBrowser::createMaterialAsset(%this)
{
   %assetName = AssetBrowser.newAssetSettings.assetName;
   
   %moduleName = AssetBrowser.newAssetSettings.moduleName;
   %modulePath = "data/" @ %moduleName;
   
   %tamlpath = %modulePath @ "/materials/" @ %assetName @ ".asset.taml";
   %sgfPath = %modulePath @ "/materials/" @ %assetName @ ".sgf";
   
   %asset = new MaterialAsset()
   {
      AssetName = %assetName;
      versionId = 1;
      shaderData = "";
      shaderGraph = %sgfPath;
   };
   
   TamlWrite(%asset, %tamlpath);
   
   %moduleDef = ModuleDatabase.findModule(%moduleName, 1);
	AssetDatabase.addDeclaredAsset(%moduleDef, %tamlpath);

	AssetBrowser.loadFilters();
	
	%treeItemId = AssetBrowserFilterTree.findItemByName(%moduleName);
	%smItem = AssetBrowserFilterTree.findChildItemByName(%treeItemId, "Materials");
	
	AssetBrowserFilterTree.onSelect(%smItem);
   
	return %tamlpath;
}

function AssetBrowser::editMaterialAsset(%this, %assetDef)
{
   //if(EditorSettings.materialEditMode $= "MaterialEditor")
   //{
      %assetDef.materialDefinitionName.reload();
      
      EditorGui.setEditor(MaterialEditorPlugin);
      
      MaterialEditorGui.currentMaterial = %assetDef.materialDefinitionName;
      MaterialEditorGui.setActiveMaterial( %assetDef.materialDefinitionName );
      
      AssetBrowser.hideDialog();
   /*}
   else
   {
      Canvas.pushDialog(ShaderEditor); 
      ShaderEditorGraph.loadGraph(%assetDef.shaderGraph);
      $ShaderGen::targetShaderFile = filePath(%assetDef.shaderGraph) @"/"@fileBase(%assetDef.shaderGraph);
   //}   */
}

function AssetBrowser::prepareImportMaterialAsset(%this, %assetItem)
{
   //Iterate over to find appropriate images for
         
   //Fetch just the fileBase name
   %fileDir = filePath(%assetItem.filePath);
   %fileName = fileBase(%assetItem.filePath);
   %fileExt = fileExt(%assetItem.filePath);
   
   //Check if we need to filter this material out or not
   if(ImportAssetWindow.activeImportConfig.IgnoreMaterials !$= "")
   {
      %ignoredMatNamesCount = getTokenCount(ImportAssetWindow.activeImportConfig.IgnoreMaterials, ",;");
      for(%i=0; %i < %ignoredMatNamesCount; %i++)
      {
        %ignoreName = getToken(ImportAssetWindow.activeImportConfig.IgnoreMaterials, ".;", %i);
        
        if(strIsMatchExpr(%ignoreName, %fileName))
        {
            //We fit the bill, ignore this material and skip it
            %assetItem.skip = true;
            return;  
        }
      }
   }
   
   if(ImportAssetWindow.activeImportConfig.PopulateMaterialMaps == 1)
   {
      %materialItemId = ImportAssetTree.findItemByObjectId(%assetItem);
      
      if(%assetItem.diffuseImageAsset $= "")
      {
         //First, load our diffuse map, as set to the material in the shape
         //We're going to presume(for now) that the specifically-mentioned file for a given material is the diffuse/albedo
         %diffuseImagePath = %fileDir @ "/" @ %filename @ %fileExt;
         
         %diffuseImageSuffix = ImportAssetWindow.parseImagePathSuffixes(%diffuseImagePath);
         
         
         
         if(ImportAssetWindow.activeImportConfig.UseDiffuseSuffixOnOriginImg == 1 && %diffuseImageSuffix $= "")
         {
            %diffuseToken = getToken(ImportAssetWindow.activeImportConfig.DiffuseTypeSuffixes, ",;", 0);
            
            %diffuseAsset = AssetBrowser.addImportingAsset("Image", %diffuseImagePath, %materialItemId, %filename @ %diffuseToken);
         }
         else
         {
            %diffuseAsset = AssetBrowser.addImportingAsset("Image", %diffuseImagePath, %materialItemId);
         }
         
         %assetItem.diffuseImageAsset = %diffuseAsset;
      }
      
      //Now, iterate over our comma-delimited suffixes to see if we have any matches. We'll use the first match in each case, if any.
      if(%assetItem.normalImageAsset $= "")
      {
         //First, normal map
         %targetFilePath = %this.findMaterialMapFileWSuffix(%fileDir, %fileName, %fileExt, ImportAssetWindow.activeImportConfig.NormalTypeSuffixes);
         
         if(%targetFilePath $= "")
         {
            //Didn't find it for the presumed file path, so lets angle it from the diffuse map's texture name, if it has one
            if(isObject(%assetItem.diffuseImageAsset))
            {
               if(isFile(%assetItem.diffuseImageAsset.filePath))
               {
                  %diffFileDir = filePath(%assetItem.diffuseImageAsset.filePath);
                  %diffFileName = fileBase(%assetItem.diffuseImageAsset.filePath);
                  %diffFileExt = fileExt(%assetItem.diffuseImageAsset.filePath);
                  
                  %suffixCount = getTokenCount(ImportAssetWindow.activeImportConfig.DiffuseTypeSuffixes, ",;");
                  for(%sfx = 0; %sfx < %suffixCount; %sfx++)
                  {
                     %suffixToken = getToken(ImportAssetWindow.activeImportConfig.DiffuseTypeSuffixes, ",;", %sfx);
                     if(strIsMatchExpr("*"@%suffixToken, %diffFileName))
                     {
                        %diffFileName = strreplace(%diffFileName, %suffixToken, "");
                        break;
                     }
                  }
                  
                  %targetFilePath = %this.findMaterialMapFileWSuffix(%diffFileDir, %diffFileName, %diffFileExt, ImportAssetWindow.activeImportConfig.NormalTypeSuffixes);
               }
            }
         }
         
         if(%targetFilePath !$= "")
         {
            %normalAsset = AssetBrowser.addImportingAsset("Image", %targetFilePath, %materialItemId);
            %assetItem.normalImageAsset = %normalAsset;
         }
      }
      if(%assetItem.specularImageAsset $= "")
      {
         //Specular
         %listCount = getTokenCount(ImportAssetWindow.activeImportConfig.SpecularTypeSuffixes, ",;");
   
         %foundFile = 0;
         for(%i=0; %i < %listCount; %i++)
         {
            %entryText = getToken(ImportAssetWindow.activeImportConfig.SpecularTypeSuffixes, ",;", %i);
            
            %targetFilePath = %fileDir @ "/" @ %filename @ %entryText @ %fileExt;
            %foundFile = isFile(%targetFilePath);
            
            if(%foundFile)
            {
               %specularAsset = AssetBrowser.addImportingAsset("Image", %targetFilePath, %materialItemId);
               %assetItem.specularImageAsset = %specularAsset;
               break;  
            }
         }
      }
      
      if(%assetItem.metalImageAsset $= "")
      {
         //Metal
         %listCount = getTokenCount(ImportAssetWindow.activeImportConfig.MetalnessTypeSuffixes, ",;");
   
         %foundFile = 0;
         for(%i=0; %i < %listCount; %i++)
         {
            %entryText = getToken(ImportAssetWindow.activeImportConfig.MetalnessTypeSuffixes, ",;", %i);
            
            %targetFilePath = %fileDir @ "/" @ %filename @ %entryText @ %fileExt;
            %foundFile = isFile(%targetFilePath);
            
            if(%foundFile)
            {
               %metalAsset = AssetBrowser.addImportingAsset("Image", %targetFilePath, %materialItemId);
               %assetItem.metalImageAsset = %metalAsset;
               break;  
            }
         }
      }
      
      if(%assetItem.roughnessImageAsset $= "")
      {
         //Roughness
         %listCount = getTokenCount(ImportAssetWindow.activeImportConfig.RoughnessTypeSuffixes, ",;");
   
         %foundFile = 0;
         for(%i=0; %i < %listCount; %i++)
         {
            %entryText = getToken(ImportAssetWindow.activeImportConfig.RoughnessTypeSuffixes, ",;", %i);
            
            %targetFilePath = %fileDir @ "/" @ %filename @ %entryText @ %fileExt;
            %foundFile = isFile(%targetFilePath);
            
            if(%foundFile)
            {
               %roughnessAsset = AssetBrowser.addImportingAsset("Image", %targetFilePath, %materialItemId);
               %assetItem.roughnessImageAsset = %roughnessAsset;
               break;  
            }
         }
      }
      
      if(%assetItem.smoothnessImageAsset $= "")
      {
         //Smoothness
         %listCount = getTokenCount(ImportAssetWindow.activeImportConfig.SmoothnessTypeSuffixes, ",;");
   
         %foundFile = 0;
         for(%i=0; %i < %listCount; %i++)
         {
            %entryText = getToken(ImportAssetWindow.activeImportConfig.SmoothnessTypeSuffixes, ",;", %i);
            
            %targetFilePath = %fileDir @ "/" @ %filename @ %entryText @ %fileExt;
            %foundFile = isFile(%targetFilePath);
            
            if(%foundFile)
            {
               %smoothnessAsset = AssetBrowser.addImportingAsset("Image", %targetFilePath, %materialItemId);
               %assetItem.SmoothnessImageAsset = %smoothnessAsset;
               break;  
            }
         }
      }
      
      if(%assetItem.AOImageAsset $= "")
      {
         //AO
         %listCount = getTokenCount(ImportAssetWindow.activeImportConfig.AOTypeSuffixes, ",;");
   
         %foundFile = 0;
         for(%i=0; %i < %listCount; %i++)
         {
            %entryText = getToken(ImportAssetWindow.activeImportConfig.AOTypeSuffixes, ",;", %i);
            
            %targetFilePath = %fileDir @ "/" @ %filename @ %entryText @ %fileExt;
            %foundFile = isFile(%targetFilePath);
            
            if(%foundFile)
            {
               %AOAsset = AssetBrowser.addImportingAsset("Image", %targetFilePath, %materialItemId);
               %assetItem.AOImageAsset = %AOAsset;
               break;  
            }
         }
      }
      
      if(%assetItem.compositeImageAsset $= "")
      {
         //Composite
         %listCount = getTokenCount(ImportAssetWindow.activeImportConfig.CompositeTypeSuffixes, ",;");
   
         %foundFile = 0;
         for(%i=0; %i < %listCount; %i++)
         {
            %entryText = getToken(ImportAssetWindow.activeImportConfig.CompositeTypeSuffixes, ",;", %i);
            
            %targetFilePath = %fileDir @ "/" @ %filename @ %entryText @ %fileExt;
            %foundFile = isFile(%targetFilePath);
            
            if(%foundFile)
            {
               %compositeAsset = AssetBrowser.addImportingAsset("Image", %targetFilePath, %materialItemId);
               %assetItem.compositeImageAsset = %compositeAsset;
               break;  
            }
         }
      }
   }
}

function AssetBrowser::findMaterialMapFileWSuffix(%this, %fileDir, %filename, %fileExt, %suffixList)
{
   //Now, iterate over our comma-delimited suffixes to see if we have any matches. We'll use the first match in each case, if any.
   //First, normal map
   %listCount = getTokenCount(%suffixList, ",;");
   
   %foundFile = 0;
   for(%i=0; %i < %listCount; %i++)
   {
      %entryText = getToken(%suffixList, ",;", %i);
      
      %targetFilePath = %fileDir @ "/" @ %filename @ %entryText @ %fileExt;
      %foundFile = isFile(%targetFilePath);
      
      if(%foundFile)
      {
         return %targetFilePath;
         break;  
      }
   }  
   
   return "";
}

function AssetBrowser::importMaterialAsset(%this, %assetItem)
{
   %moduleName = ImportAssetModuleList.getText();
   
   %assetType = %assetItem.AssetType;
   %filePath = %assetItem.filePath;
   %assetName = %assetItem.assetName;
   %assetImportSuccessful = false;
   %assetId = %moduleName@":"@%assetName;
   
   %assetPath = "data/" @ %moduleName @ "/materials";
   %tamlpath = %assetPath @ "/" @ %assetName @ ".asset.taml";
   %sgfPath = %assetPath @ "/" @ %assetName @ ".sgf";
   %scriptPath = %assetPath @ "/" @ %assetName @ ".cs";
   
   %newAsset = new MaterialAsset()
   {
      assetName = %assetName;
      versionId = 1;
      shaderGraph = %sgfPath;
      scriptFile = %scriptPath;
      originalFilePath = %filePath;
      materialDefinitionName = %assetName;
   };
   
   //check dependencies
   %importItem = ImportAssetTree.findItemByObjectId(%assetItem);
   if(ImportAssetTree.isParentItem(%importItem))
   {
        %imageSlot = 0;
        %childId = ImportAssetTree.getChild(%importItem);
        while(%childId > 0)
        {
            %dependencyAssetItem = ImportAssetTree.getItemObject(%childId);
            
            %depAssetType = %dependencyAssetItem.assetType;
            if(%depAssetType $= "Image")
            {
               %matSet = "%newAsset.imageMap"@%imageSlot@"=\"@Asset="@%moduleName@":"@%dependencyAssetItem.assetName@"\";";
               eval(%matSet);
            }
            
            %childId = ImportAssetTree.getNextSibling(%childId);  
            %imageSlot++;
        }
   }
   
   %assetImportSuccessful = TamlWrite(%newAsset, %tamlpath);
   
   %file = new FileObject();

   if(%file.openForWrite(%scriptPath))
   {
      %file.writeline("//--- OBJECT WRITE BEGIN ---");
      %file.writeline("singleton Material(" @ %assetName @ ") {");
      
      //TODO: pass along the shape's target material for this just to be sure
      %file.writeLine("   mapTo = \"" @ %assetName @ "\";"); 
      
      if(%assetItem.diffuseImageAsset !$= "")
      {
         %diffuseAssetPath = "data/" @ %moduleName @ "/Images/" @ fileName(%assetItem.diffuseImageAsset.filePath);
         %file.writeline("   DiffuseMap[0] = \"" @ %diffuseAssetPath @"\";");
         %file.writeline("   DiffuseMapAsset[0] = \"" @ %moduleName @ ":" @ %assetItem.diffuseImageAsset.assetName @"\";");
      }
      if(%assetItem.normalImageAsset)
      {
         %normalAssetPath = "data/" @ %moduleName @ "/Images/" @ fileName(%assetItem.normalImageAsset.filePath);
         %file.writeline("   NormalMap[0] = \"" @ %normalAssetPath @"\";");
         %file.writeline("   NormalMapAsset[0] = \"" @ %moduleName @ ":" @ %assetItem.normalImageAsset.assetName @"\";");
      }
      /*if(%assetItem.specularImageAsset)
      {
         %file.writeline("   SpecularMap[0] = \"" @ %assetItem.specularImageAsset.filePath @"\";");
         %file.writeline("   SpecularMapAsset[0] = \"" @ %moduleName @ ":" @ %assetItem.specularImageAsset.assetName @"\";");
      }*/
      if(%assetItem.roughnessImageAsset)
      {
         %file.writeline("   RoughMap[0] = \"" @ %assetItem.roughnessImageAsset.filePath @"\";");
         %file.writeline("   RoughMapAsset[0] = \"" @ %moduleName @ ":" @ %assetItem.roughnessImageAsset.assetName @"\";");
      }
      if(%assetItem.smoothnessImageAsset)
      {
         %file.writeline("   SmoothnessMap[0] = \"" @ %assetItem.smoothnessImageAsset.filePath @"\";");
         %file.writeline("   SmoothnessMapAsset[0] = \"" @ %moduleName @ ":" @ %assetItem.smoothnessImageAsset.assetName @"\";");
      }
      if(%assetItem.metalnessImageAsset)
      {
         %file.writeline("   MetalMap[0] = \"" @ %assetItem.metalnessImageAsset.filePath @"\";");
         %file.writeline("   MetalMapAsset[0] = \"" @ %moduleName @ ":" @ %assetItem.metalnessImageAsset.assetName @"\";");
      }
      if(%assetItem.AOImageAsset)
      {
         %file.writeline("   AOMap[0] = \"" @ %assetItem.AOImageAsset.filePath @"\";");
         %file.writeline("   AOMapAsset[0] = \"" @ %moduleName @ ":" @ %assetItem.AOImageAsset.assetName @"\";");
      }
      if(%assetItem.compositeImageAsset)
      {
         %file.writeline("   CompositeMap[0] = \"" @ %assetItem.compositeImageAsset.filePath @"\";");
         %file.writeline("   CompositeMapAsset[0] = \"" @ %moduleName @ ":" @ %assetItem.compositeImageAsset.assetName @"\";");
      }
      %file.writeline("};");
      %file.writeline("//--- OBJECT WRITE END ---");
      
      %file.close();
   }
   
   %moduleDef = ModuleDatabase.findModule(%moduleName,1);
         
   if(!AssetBrowser.isAssetReImport)
      AssetDatabase.addDeclaredAsset(%moduleDef, %assetPath @ "/" @ %assetName @ ".asset.taml");
   else
      AssetDatabase.refreshAsset(%assetId);
}

function AssetBrowser::buildMaterialAssetPreview(%this, %assetDef, %previewData)
{
   %previewData.assetName = %assetDef.materialDefinitionName;
   %previewData.assetPath = %assetDef.scriptFile;

   //Lotta prepwork
   %previewData.doubleClickCommand = %assetDef@".materialDefinitionName.reload(); "
                                   @ "$Tools::materialEditorList = \"\";"
                                   @ "EWorldEditor.clearSelection();"
                                   @ "MaterialEditorGui.currentObject = 0;"
                                   @ "MaterialEditorGui.currentMode = \"asset\";"
                                   @ "MaterialEditorGui.currentMaterial = "@%assetDef@".materialDefinitionName;"
                                   @ "MaterialEditorGui.setActiveMaterial( "@%assetDef@".materialDefinitionName );"
                                   @ "EditorGui.setEditor(MaterialEditorPlugin); "
                                   @ "AssetBrowser.hideDialog();";
   
   if(isFile(%assetDef.materialDefinitionName.diffuseMap[0]))
      %previewData.previewImage = %assetDef.materialDefinitionName.diffuseMap[0];
   else
      %previewData.previewImage = "tools/assetBrowser/art/materialIcon";
   
   %previewData.assetFriendlyName = %assetDef.assetName;
   %previewData.assetDesc = %assetDef.description;
   %previewData.tooltip = %assetDef.friendlyName @ "\n" @ %assetDef;
}

function GuiInspectorTypeMaterialAssetPtr::onControlDropped( %this, %payload, %position )
{
   Canvas.popDialog(EditorDragAndDropLayer);
   
   // Make sure this is a color swatch drag operation.
   if( !%payload.parentGroup.isInNamespaceHierarchy( "AssetPreviewControlType_AssetDrop" ) )
      return;

   %assetType = %payload.dragSourceControl.parentGroup.assetType;
   %module = %payload.dragSourceControl.parentGroup.moduleName;
   %assetName = %payload.dragSourceControl.parentGroup.assetName;
   
   if(%assetType $= "MaterialAsset")
   {
      echo("DROPPED A MATERIAL ON A MATERIAL ASSET COMPONENT FIELD!");  
      //%assetDef = AssetDatabase.acquireAsset(%module @ ":" @ %assetName);
      
      %this.setMaterialAsset(%module @ ":" @ %assetName);
   }
   
   EWorldEditor.isDirty = true;
}
