function ImportAssetConfigList::onSelect( %this, %id, %text )
{
   //Apply our settings to the assets
   echo("Changed our import config!");
   AssetBrowser.importAssetUnprocessedListArray.empty();
   AssetBrowser.importAssetUnprocessedListArray.duplicate(AssetBrowser.importAssetNewListArray);
   AssetBrowser.importAssetFinalListArray.empty();
   
   ImportAssetWindow.activeImportConfigIndex = %id;
   ImportAssetWindow.activeImportConfig = ImportAssetWindow.importConfigsList.getKey(%id);
   //ImportAssetWindow.refresh();
   
   AssetBrowser.reloadImportingFiles();
}

function ImportAssetOptionsWindow::findMissingFile(%this, %assetItem)
{
   if(%assetItem.assetType $= "Model")
      %filters = "Shape Files(*.dae, *.cached.dts)|*.dae;*.cached.dts";
   else if(%assetItem.assetType $= "Image")
      %filters = "Images Files(*.jpg,*.png,*.tga,*.bmp,*.dds)|*.jpg;*.png;*.tga;*.bmp;*.dds";
      
   %dlg = new OpenFileDialog()
   {
      Filters        = %filters;
      DefaultPath    = $Pref::WorldEditor::LastPath;
      DefaultFile    = "";
      ChangePath     = true;
      OverwritePrompt = true;
      forceRelativePath = false;
      fileName="";
      //MultipleFiles = true;
   };

   %ret = %dlg.Execute();
   
   if ( %ret )
   {
      $Pref::WorldEditor::LastPath = filePath( %dlg.FileName );
      %fullPath = %dlg.FileName;//makeRelativePath( %dlg.FileName, getMainDotCSDir() );
   }   
   
   %dlg.delete();
   
   if ( !%ret )
      return;
      
   %assetItem.filePath = %fullPath;
   %assetItem.assetName = fileBase(%assetItem.filePath);
   
   if(%assetItem.assetType $= "Image")
   {
      //See if we have anything important to update for our material parent(if we have one)
      %treeItem = ImportAssetTree.findItemByObjectId(%assetItem);
      %parentItem = ImportAssetTree.getParentItem(%treeItem);
      
      if(%parentItem != 0)
      {
         %parentAssetItem = ImportAssetTree.getItemObject(%parentItem);
         if(%parentAssetItem.assetType $= "Material")
         {
            AssetBrowser.prepareImportMaterialAsset(%parentAssetItem);              
         }
      }
   }
   
   ImportAssetWindow.refresh();
}

//
function ImportAssetOptionsWindow::editImportSettings(%this, %assetItem)
{
   ImportAssetOptionsWindow.setVisible(1);
   ImportAssetOptionsWindow.selectWindow();
   
   ImportOptionsList.clearFields();
   
   %assetType = %assetItem.assetType;
   %filePath = %assetItem.filePath;
   %assetName = %assetItem.assetName;
   %assetConfigObj = %assetItem.importConfig;
   
   ImportOptionsList.startGroup("Asset");
   ImportOptionsList.addField("AssetName", "Asset Name", "string", "", "NewAsset", "", %assetItem);
   ImportOptionsList.endGroup();
   
   if(%assetType $= "Model")
   {
      //Get the shape info, so we know what we're doing with the mesh
      %shapeInfo = GetShapeInfo(%filePath);
      %meshItem = %shapeInfo.findItemByName("Meshes");
      %matItem = %shapeInfo.findItemByName("Materials");
      
      %meshCount = %shapeInfo.getItemValue(%meshItem);
      %matCount = %shapeInfo.getItemValue(%matItem);
      
      %firstMat = %shapeInfo.getChild(%matItem);
      echo("Mesh's first material texture path is: " @ %shapeInfo.getItemValue(%firstMat));
            
      if(%meshCount > 0)
      {
         ImportOptionsList.startGroup("Mesh");
         ImportOptionsList.addField("AutogenCollisions", "Auto-gen Collisions", "bool", "", "0", "", %assetConfigObj);
         ImportOptionsList.addField("CollapseSubmeshes", "Collapse Submeshes", "bool", "", "0", "", %assetConfigObj);
         ImportOptionsList.addField("UpAxisOverride", "Up-Axis Override", "list", "", "Z_AXIS", "Z_AXIS,Y_AXIS,X_AXIS", %assetConfigObj);
         ImportOptionsList.addField("OverrideScale", "Override Scale", "float", "", "1.0", "", %assetConfigObj);
         ImportOptionsList.addField("IgnoreNodeScale", "IgnoreNodeScaling", "bool", "", "0", "", %assetConfigObj);
         ImportOptionsList.addField("AdjustCenter", "Adjust Center", "bool", "", "0", "", %assetConfigObj);
         ImportOptionsList.addField("CollapseSubmeshes", "Collapse Submeshes", "bool", "", "0", "", %assetConfigObj);
         ImportOptionsList.addField("AdjustFloor", "Adjust Floor", "bool", "", "0", "", %assetConfigObj);
         ImportOptionsList.addField("LODType", "LOD Type", "list", "", "TrailingNumber", "TrailingNumber,DetectDTS", %assetConfigObj);
         ImportOptionsList.endGroup();
      }
      
      if(%matItem > 0)
      {
         ImportOptionsList.startGroup("Material");
         ImportOptionsList.addCallbackField("ImportMaterials", "Import Materials", "bool", "", "1", "", "ImportMaterialsChanged", %assetConfigObj);
         ImportOptionsList.addField("UseExistingMaterials", "Use Existing Materials", "bool", "", "1", "", %assetConfigObj);
         ImportOptionsList.endGroup();
      }
   }
   else if(%assetType $= "Material")
   {
      ImportOptionsList.startGroup("Material");
      ImportOptionsList.addField("CreateComposites", "Create Composite Textures", "bool", "", "1", "", %assetConfigObj);
      ImportOptionsList.endGroup();
   }
   else if(%assetType $= "Image")
   {
      ImportOptionsList.startGroup("Formatting");
      ImportOptionsList.addField("ImageType", "Image Type", "string", "", "Diffuse", "", %assetConfigObj);
      ImportOptionsList.addField("TextureFiltering", "Texture Filtering", "list", "", "Bilinear", "None,Bilinear,Trilinear", %assetConfigObj);
      ImportOptionsList.addField("UseMips", "Use Mips", "bool", "", "1", "", %assetConfigObj);
      ImportOptionsList.addField("IsHDR", "Is HDR", "bool", "", "0", "", %assetConfigObj);
      ImportOptionsList.endGroup();
      
      ImportOptionsList.startGroup("Scaling");
      ImportOptionsList.addField("Scaling", "Scaling", "float", "", "1.0", "", %assetConfigObj);
      ImportOptionsList.endGroup();
      
      ImportOptionsList.startGroup("Compression");
      ImportOptionsList.addField("IsCompressed", "Is Compressed", "bool", "", "1", "", %assetConfigObj);
      ImportOptionsList.endGroup();
      
      ImportOptionsList.startGroup("Material");
      ImportOptionsList.addField("GenerateMaterialOnImport", "Generate Material On Import", "bool", "", "1", "", %optionsObj);
      ImportOptionsList.addField("PopulateMaterialMaps", "Populate Material Maps", "bool", "", "1", "", %optionsObj);
      ImportOptionsList.addField("UseDiffuseSuffixOnOriginImg", "Use Diffuse Suffix for Origin Image", "bool", "", "1", "", %optionsObj);
      ImportOptionsList.addField("UseExistingMaterials", "Use Existing Materials", "bool", "", "1", "", %optionsObj);
      ImportOptionsList.addField("IgnoreMaterials", "Ignore Importing Materials that fit these naming convention.", "command", "", "1", "", %optionsObj);
      ImportOptionsList.endGroup();
   }
   else if(%assetType $= "Sound")
   {
      ImportOptionsList.startGroup("Adjustment");
      ImportOptionsList.addField("VolumeAdjust", "VolumeAdjustment", "float", "", "1.0", "", %assetConfigObj);
      ImportOptionsList.addField("PitchAdjust", "PitchAdjustment", "float", "", "1.0", "", %assetConfigObj);
      ImportOptionsList.endGroup();
      
      ImportOptionsList.startGroup("Compression");
      ImportOptionsList.addField("IsCompressed", "Is Compressed", "bool", "", "1", "", %assetConfigObj);
      ImportOptionsList.endGroup();
   }
}

function ImportAssetOptionsWindow::deleteImportingAsset(%this, %assetItem)
{
   %item = ImportAssetTree.findItemByObjectId(%assetItem);
   
   ImportAssetTree.removeAllChildren(%item);
   ImportAssetTree.removeItem(%item);

   schedule(10, 0, "refreshImportAssetWindow");
   //ImportAssetWindow.refresh();
   ImportAssetOptionsWindow.setVisible(0);
}

function ImportAssetOptionsWindow::saveAssetOptions(%this)
{
   ImportAssetWindow.refresh();
   ImportAssetOptionsWindow.setVisible(0);   
}

function ImportOptionsList::ImportMaterialsChanged(%this, %fieldName, %newValue, %ownerObject)
{
   echo("CHANGED IF OUR IMPORTED MATERIALS WERE HAPPENING!");
}

function ImportAssetConfigEditorWindow::populateConfigList(%this, %optionsObj)
{
   AssetImportConfigName.setText(%optionsObj.Name);
   
   ImportOptionsConfigList.clear();
   
   ImportOptionsConfigList.startGroup("Mesh");
   ImportOptionsConfigList.addCallbackField("ImportMesh", "Import Mesh", "bool", "", "1", "", "ToggleImportMesh", %optionsObj);
   ImportOptionsConfigList.addField("DoUpAxisOverride", "Do Up-axis Override", "bool", "", "0", "", %optionsObj);
   ImportOptionsConfigList.addField("UpAxisOverride", "Up-axis Override", "list", "", "Z_AXIS", "X_AXIS,Y_AXIS,Z_AXIS", %optionsObj);
   ImportOptionsConfigList.addField("DoScaleOverride", "Do Scale Override", "bool", "", "0", "", %optionsObj);
   ImportOptionsConfigList.addField("ScaleOverride", "Scale Override", "float", "", "1", "", %optionsObj);
   ImportOptionsConfigList.addField("IgnoreNodeScale", "Ignore Node Scale", "bool", "", "0", "", %optionsObj);
   ImportOptionsConfigList.addField("AdjustCenter", "Adjust Center", "bool", "", "0", "", %optionsObj);
   ImportOptionsConfigList.addField("AdjustFloor", "Adjust Floor", "bool", "", "0", "", %optionsObj);
   ImportOptionsConfigList.addField("CollapseSubmeshes", "Collapse Submeshes", "bool", "", "0", "", %optionsObj);
   ImportOptionsConfigList.addField("LODType", "LOD Type", "list", "", "TrailingNumber", "TrailingNumber,DetectDTS", %optionsObj);
   //ImportOptionsConfigList.addField("TrailingNumber", "Trailing Number", "float", "", "2", "", %optionsObj, "Mesh");
   ImportOptionsConfigList.addField("ImportedNodes", "Imported Nodes", "command", "", "", "", %optionsObj);
   ImportOptionsConfigList.addField("IgnoreNodes", "Ignore Nodes", "command", "", "", "", %optionsObj);
   ImportOptionsConfigList.addField("ImportMeshes", "Import Meshes", "command", "", "", "", %optionsObj);
   ImportOptionsConfigList.addField("IgnoreMeshes", "Imported Meshes", "command", "", "", "", %optionsObj);
   ImportOptionsConfigList.endGroup();
   
   //Materials
   ImportOptionsConfigList.startGroup("Material");
   ImportOptionsConfigList.addField("ImportMaterials", "Import Materials", "bool", "", "1", "", %optionsObj);
   ImportOptionsConfigList.addField("CreateComposites", "Create Composites", "bool", "", "1", "", %optionsObj);
   ImportOptionsConfigList.addField("UseDiffuseSuffixOnOriginImg", "Use Diffuse Suffix for Origin Image", "bool", "", "1", "", %optionsObj);
   ImportOptionsConfigList.addField("UseExistingMaterials", "Use Existing Materials", "bool", "", "1", "", %optionsObj);
   ImportOptionsConfigList.addField("IgnoreMaterials", "Ignore Materials", "command", "", "", "", %optionsObj);
   ImportOptionsConfigList.endGroup();
   
   //Animations
   ImportOptionsConfigList.startGroup("Animations");
   ImportOptionsConfigList.addField("ImportAnimations", "Import Animations", "bool", "", "1", "", %optionsObj);
   ImportOptionsConfigList.addField("SeparateAnimations", "Separate Animations", "bool", "", "1", "", %optionsObj);
   ImportOptionsConfigList.addField("SeparateAnimationPrefix", "Separate Animation Prefix", "string", "", "", "", %optionsObj);
   ImportOptionsConfigList.endGroup();
   
   //Collision
   ImportOptionsConfigList.startGroup("Collision");
   ImportOptionsConfigList.addField("GenerateCollisions", "Generate Collisions", "bool", "", "1", "", %optionsObj);
   ImportOptionsConfigList.addField("GenCollisionType", "Generate Collision Type", "list", "", "CollisionMesh", "CollisionMesh,ConvexHull", %optionsObj);
   ImportOptionsConfigList.addField("CollisionMeshPrefix", "CollisionMesh Prefix", "string", "", "Col", "", %optionsObj);
   ImportOptionsConfigList.addField("GenerateLOSCollisions", "Generate LOS Collisions", "bool", "", "1", "", %optionsObj);
   ImportOptionsConfigList.addField("GenLOSCollisionType", "Generate LOS Collision Type", "list", "", "CollisionMesh", "CollisionMesh,ConvexHull", %optionsObj);
   ImportOptionsConfigList.addField("LOSCollisionMeshPrefix", "LOS CollisionMesh Prefix", "string", "", "LOS", "", %optionsObj);
   ImportOptionsConfigList.endGroup();
   
   //Images
   ImportOptionsConfigList.startGroup("Image");
   ImportOptionsConfigList.addField("ImageType", "Image Type", "list", "", "N/A", "N/A,Diffuse,Normal,Specular,Metalness,Roughness,AO,Composite,GUI", %optionsObj);
   ImportOptionsConfigList.addField("DiffuseTypeSuffixes", "Diffuse Type Suffixes", "command", "", "_ALBEDO,_DIFFUSE,_ALB,_DIF,_COLOR,_COL", "", %optionsObj);
   ImportOptionsConfigList.addField("NormalTypeSuffixes", "Normal Type Suffixes", "command", "", "_NORMAL,_NORM", "", %optionsObj);
   
   if(EditorSettings.lightingModel $= "Legacy")
   {
      ImportOptionsConfigList.addField("SpecularTypeSuffixes", "Specular Type Suffixes", "command", "", "_SPECULAR,_SPEC", "", %optionsObj);
   }
   else
   {
      ImportOptionsConfigList.addField("MetalnessTypeSuffixes", "Metalness Type Suffixes", "command", "", "_METAL,_MET,_METALNESS,_METALLIC", "", %optionsObj);
      ImportOptionsConfigList.addField("RoughnessTypeSuffixes", "Roughness Type Suffixes", "command", "", "_ROUGH,_ROUGHNESS", "", %optionsObj);
      ImportOptionsConfigList.addField("SmoothnessTypeSuffixes", "Smoothness Type Suffixes", "command", "", "_SMOOTH,_SMOOTHNESS", "", %optionsObj);
      ImportOptionsConfigList.addField("AOTypeSuffixes", "AO Type Suffixes", "command", "", "_AO,_AMBIENT,_AMBIENTOCCLUSION", "", %optionsObj);
      ImportOptionsConfigList.addField("CompositeTypeSuffixes", "Composite Type Suffixes", "command", "", "_COMP,_COMPOSITE", "", %optionsObj);
   }
   
   ImportOptionsConfigList.addField("TextureFilteringMode", "Texture Filtering Mode", "list", "", "Bilinear", "None,Bilinear,Trilinear", %optionsObj);
   ImportOptionsConfigList.addField("UseMips", "Use Mipmaps", "bool", "", "1", "", %optionsObj);
   ImportOptionsConfigList.addField("IsHDR", "Is HDR", "bool", "", "0", "", %optionsObj);
   ImportOptionsConfigList.addField("Scaling", "Scaling", "float", "", "1.0", "", %optionsObj);
   ImportOptionsConfigList.addField("Compressed", "Is Compressed", "bool", "", "1", "", %optionsObj);
   ImportOptionsConfigList.addField("GenerateMaterialOnImport", "Generate Material On Import", "bool", "", "1", "", %optionsObj);
   ImportOptionsConfigList.addField("PopulateMaterialMaps", "Populate Material Maps", "bool", "", "1", "", %optionsObj);
   ImportOptionsConfigList.endGroup();
   
   //Sounds
   ImportOptionsConfigList.startGroup("Sound");
   ImportOptionsConfigList.addField("VolumeAdjust", "Volume Adjustment", "float", "", "1.0", "", %optionsObj);
   ImportOptionsConfigList.addField("PitchAdjust", "Pitch Adjustment", "float", "", "1.0", "", %optionsObj);
   ImportOptionsConfigList.addField("Compressed", "Is Compressed", "bool", "", "0", "", %optionsObj);
   ImportOptionsConfigList.endGroup();
}

function ImportAssetConfigEditorWindow::addNewConfig(%this)
{
   ImportAssetConfigEditorWindow.setVisible(1);
   ImportAssetConfigEditorWindow.selectWindow();
   
   %optionsObj = new ScriptObject(){};
   
   ImportAssetWindow.importConfigsList.add(%optionsObj); 
   
   //Initial, blank configuration
   %optionsObj.ImportMesh = true;
   %optionsObj.DoUpAxisOverride = false;
   %optionsObj.UpAxisOverride = "Z_AXIS";
   %optionsObj.DoScaleOverride = false;
   %optionsObj.ScaleOverride = 1.0;
   %optionsObj.IgnoreNodeScale = false;
   %optionsObj.AdjustCenter = false;
   %optionsObj.AdjustFloor = false;
   %optionsObj.CollapseSubmeshes = false;
   %optionsObj.LODType = "TrailingNumber";
   //%optionsObj.TrailingNumber = 2;
   %optionsObj.ImportedNodes = "";
   %optionsObj.IgnoreNodes = "";
   %optionsObj.ImportMeshes = "";
   %optionsObj.IgnoreMeshes = "";
   
   //Materials
   %optionsObj.ImportMaterials = true;
   %optionsObj.IgnoreMaterials = "";
   %optionsObj.CreateComposites = true;
   %optionsObj.UseDiffuseSuffixOnOriginImg = true;
   %optionsObj.UseExistingMaterials = true;
   
   //Animations
   %optionsObj.ImportAnimations = true;
   %optionsObj.SeparateAnimations = true;
   %optionsObj.SeparateAnimationPrefix = "";
   
   //Collision
   %optionsObj.GenerateCollisions = true;
   %optionsObj.GenCollisionType = "CollisionMesh";
   %optionsObj.CollisionMeshPrefix = "Col";
   %optionsObj.GenerateLOSCollisions = true;
   %optionsObj.GenLOSCollisionType = "CollisionMesh";
   %optionsObj.LOSCollisionMeshPrefix = "LOS";
   
   //Images
   %optionsObj.ImageType = "N/A";
   %optionsObj.DiffuseTypeSuffixes = "_ALBEDO;_DIFFUSE;_ALB;_DIF;_COLOR;_COL;_BASECOLOR;_BASE_COLOR";
   %optionsObj.NormalTypeSuffixes = "_NORMAL;_NORM";
   %optionsObj.SpecularTypeSuffixes = "_SPECULAR;_SPEC";
   %optionsObj.MetalnessTypeSuffixes = "_METAL;_MET;_METALNESS;_METALLIC";
   %optionsObj.RoughnessTypeSuffixes = "_ROUGH;_ROUGHNESS";
   %optionsObj.SmoothnessTypeSuffixes = "_SMOOTH;_SMOOTHNESS";
   %optionsObj.AOTypeSuffixes = "_AO;_AMBIENT;_AMBIENTOCCLUSION";
   %optionsObj.CompositeTypeSuffixes = "_COMP;_COMPOSITE";
   %optionsObj.TextureFilteringMode = "Bilinear";
   %optionsObj.UseMips = true;
   %optionsObj.IsHDR = false;
   %optionsObj.Scaling = 1.0;
   %optionsObj.Compressed = true;
   %optionsObj.GenerateMaterialOnImport = true;
   %optionsObj.PopulateMaterialMaps = true;
   
   //Sounds
   %optionsObj.VolumeAdjust = 1.0;
   %optionsObj.PitchAdjust = 1.0;
   %optionsObj.Compressed = false;
   
   //Hook in the UI
   %this.populateConfigList(%optionsObj);
}

function ImportAssetConfigEditorWindow::editConfig(%this)
{
   ImportAssetConfigEditorWindow.setVisible(1);
   ImportAssetConfigEditorWindow.selectWindow();
   
   %this.populateConfigList(ImportAssetWindow.activeImportConfig);
}

function ImportAssetConfigEditorWindow::deleteConfig(%this)
{
   ImportAssetWindow.importConfigsList.erase(ImportAssetWindow.activeImportConfigIndex);
   ImportAssetConfigList.setSelected(0); //update it
   
   ImportAssetConfigEditorWindow.saveAssetOptionsConfig();
}

function ImportAssetConfigEditorWindow::saveAssetOptionsConfig(%this)
{
   %xmlDoc = new SimXMLDocument();
   
   %xmlDoc.pushNewElement("AssetImportConfigs");
      
      for(%i = 0; %i < ImportAssetWindow.importConfigsList.count(); %i++)
      {
         %configObj = ImportAssetWindow.importConfigsList.getKey(%i);         
         
         %xmlDoc.pushNewElement("Config");
         
         if(%configObj.Name $= "")
            %configObj.Name = AssetImportConfigName.getText();
            
         %xmlDoc.setAttribute("Name", %configObj.Name); 
         
            %xmlDoc.pushNewElement("Mesh");
               %xmlDoc.setAttribute("ImportMesh", %configObj.ImportMesh);
               %xmlDoc.setAttribute("DoUpAxisOverride", %configObj.DoUpAxisOverride);
               %xmlDoc.setAttribute("UpAxisOverride", %configObj.UpAxisOverride);
               %xmlDoc.setAttribute("DoScaleOverride", %configObj.DoScaleOverride);
               %xmlDoc.setAttribute("ScaleOverride", %configObj.ScaleOverride);
               %xmlDoc.setAttribute("IgnoreNodeScale", %configObj.IgnoreNodeScale);
               %xmlDoc.setAttribute("AdjustCenter", %configObj.AdjustCenter);
               %xmlDoc.setAttribute("AdjustFloor", %configObj.AdjustFloor);
               %xmlDoc.setAttribute("CollapseSubmeshes", %configObj.CollapseSubmeshes);         
               %xmlDoc.setAttribute("LODType", %configObj.LODType);
               %xmlDoc.setAttribute("ImportedNodes", %configObj.ImportedNodes);
               %xmlDoc.setAttribute("IgnoreNodes", %configObj.IgnoreNodes);
               %xmlDoc.setAttribute("ImportMeshes", %configObj.ImportMeshes);
               %xmlDoc.setAttribute("IgnoreMeshes", %configObj.IgnoreMeshes);
            %xmlDoc.popElement();
            
            %xmlDoc.pushNewElement("Materials");
               %xmlDoc.setAttribute("ImportMaterials", %configObj.ImportMaterials);
               %xmlDoc.setAttribute("IgnoreMaterials", %configObj.IgnoreMaterials);
               %xmlDoc.setAttribute("CreateComposites", %configObj.CreateComposites);
               %xmlDoc.setAttribute("UseDiffuseSuffixOnOriginImg", %configObj.UseDiffuseSuffixOnOriginImg);
               %xmlDoc.setAttribute("UseExistingMaterials", %configObj.UseExistingMaterials);
            %xmlDoc.popElement();
            
            %xmlDoc.pushNewElement("Animations");
               %xmlDoc.setAttribute("ImportAnimations", %configObj.ImportAnimations);
               %xmlDoc.setAttribute("SeparateAnimations", %configObj.SeparateAnimations);
               %xmlDoc.setAttribute("SeparateAnimationPrefix", %configObj.SeparateAnimationPrefix);
            %xmlDoc.popElement();
            
            %xmlDoc.pushNewElement("Collisions");
               %xmlDoc.setAttribute("GenerateCollisions", %configObj.GenerateCollisions);
               %xmlDoc.setAttribute("GenCollisionType", %configObj.GenCollisionType);
               %xmlDoc.setAttribute("CollisionMeshPrefix", %configObj.CollisionMeshPrefix);
               %xmlDoc.setAttribute("GenerateLOSCollisions", %configObj.GenerateLOSCollisions);
               %xmlDoc.setAttribute("GenLOSCollisionType", %configObj.GenLOSCollisionType);
               %xmlDoc.setAttribute("LOSCollisionMeshPrefix", %configObj.LOSCollisionMeshPrefix);
            %xmlDoc.popElement();
            
            %xmlDoc.pushNewElement("Images");
               %xmlDoc.setAttribute("ImageType", %configObj.ImageType);
               %xmlDoc.setAttribute("DiffuseTypeSuffixes", %configObj.DiffuseTypeSuffixes);
               %xmlDoc.setAttribute("NormalTypeSuffixes", %configObj.NormalTypeSuffixes);
               %xmlDoc.setAttribute("SpecularTypeSuffixes", %configObj.SpecularTypeSuffixes);
               %xmlDoc.setAttribute("MetalnessTypeSuffixes", %configObj.MetalnessTypeSuffixes);
               %xmlDoc.setAttribute("RoughnessTypeSuffixes", %configObj.RoughnessTypeSuffixes);
               %xmlDoc.setAttribute("SmoothnessTypeSuffixes", %configObj.SmoothnessTypeSuffixes);
               %xmlDoc.setAttribute("AOTypeSuffixes", %configObj.AOTypeSuffixes);
               %xmlDoc.setAttribute("CompositeTypeSuffixes", %configObj.CompositeTypeSuffixes);
               %xmlDoc.setAttribute("TextureFilteringMode", %configObj.TextureFilteringMode);
               %xmlDoc.setAttribute("UseMips", %configObj.UseMips);
               %xmlDoc.setAttribute("IsHDR", %configObj.IsHDR);
               %xmlDoc.setAttribute("Scaling", %configObj.Scaling);
               %xmlDoc.setAttribute("Compressed", %configObj.Compressed);
               %xmlDoc.setAttribute("GenerateMaterialOnImport", %configObj.GenerateMaterialOnImport);
               %xmlDoc.setAttribute("PopulateMaterialMaps", %configObj.PopulateMaterialMaps);
            %xmlDoc.popElement();
            
            %xmlDoc.pushNewElement("Sounds");
               %xmlDoc.setAttribute("VolumeAdjust", %configObj.VolumeAdjust);
               %xmlDoc.setAttribute("PitchAdjust", %configObj.PitchAdjust);
               %xmlDoc.setAttribute("Compressed", %configObj.Compressed);
            %xmlDoc.popElement();
         
         %xmlDoc.popElement();
      }
      
   %xmlDoc.popElement();
   
   %xmlDoc.saveFile($AssetBrowser::importConfigsFile);
   
   ImportAssetConfigEditorWindow.setVisible(0);
   ImportAssetWindow.reloadImportOptionConfigs();
}

function ImportOptionsConfigList::ToggleImportMesh(%this, %fieldName, %newValue, %ownerObject)
{
   %this.setFieldEnabled("DoUpAxisOverride", %newValue);
   %this.setFieldEnabled("UpAxisOverride", %newValue);
   %this.setFieldEnabled("DoScaleOverride", %newValue);
   %this.setFieldEnabled("ScaleOverride", %newValue);
   %this.setFieldEnabled("IgnoreNodeScale", %newValue);
   %this.setFieldEnabled("AdjustCenter", %newValue);
   %this.setFieldEnabled("AdjustFloor", %newValue);
   %this.setFieldEnabled("CollapseSubmeshes", %newValue);
   %this.setFieldEnabled("LODType", %newValue);   
   %this.setFieldEnabled("ImportedNodes", %newValue);
   %this.setFieldEnabled("IgnoreNodes", %newValue);
   %this.setFieldEnabled("ImportMeshes", %newValue);
   %this.setFieldEnabled("IgnoreMeshes", %newValue);
}