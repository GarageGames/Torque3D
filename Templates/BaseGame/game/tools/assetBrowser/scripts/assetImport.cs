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

//
//
function isImageFormat(%fileExt)
{
   if( (%fileExt $= ".png") || (%fileExt $= ".jpg") || (%fileExt $= ".bmp") || (%fileExt $= ".dds") || (%fileExt $= ".tif"))
      return true;
      
   return false;
}

function isShapeFormat(%fileExt)
{
   if( (%fileExt $= ".dae") || (%fileExt $= ".dts") || (%fileExt $= ".fbx") || (%fileExt $= ".obj") || (%fileExt $= ".blend"))
      return true;
      
   return false;
}

function isSoundFormat(%fileExt)
{
   if( (%fileExt $= ".ogg") || (%fileExt $= ".wav") || (%fileExt $= ".mp3"))
      return true;
      
   return false;
}

function getImageInfo(%file)
{
   //we're going to populate a GuiTreeCtrl with info of the inbound image file
}

//This lets us go and look for a image at the importing directory as long as it matches the material name
function findImageFile(%path, %materialName, %type)
{
   
   if(isFile(%path @ "/" @ %materialName @ ".jpg"))
      return %path @ "/" @ %materialName @ ".jpg";
   else if(isFile(%path @ "/" @ %materialName @ ".png"))
      return %path @ "/" @ %materialName @ ".png";
   else if(isFile(%path @ "/" @ %materialName @ ".dds"))
      return %path @ "/" @ %materialName @ ".dds";
   else if(isFile(%path @ "/" @ %materialName @ ".tif"))
      return %path @ "/" @ %materialName @ ".tif";
}

function AssetBrowser::onBeginDropFiles( %this )
{   
   error("% DragDrop - Beginning files dropping.");
   %this.importAssetUnprocessedListArray.empty();
   %this.importAssetFinalListArray.empty();
   
   ImportAssetTree.clear();
   AssetBrowser.unprocessedAssetsCount = 0;
}

function AssetBrowser::onDropFile( %this, %filePath )
{
   if(!%this.isVisible())
      return;
      
   %fileExt = fileExt( %filePath );
   //add it to our array!
   if(isImageFormat(%fileExt))
      %this.addImportingAsset("Image", %filePath);
   else if( isShapeFormat(%fileExt))
      %this.addImportingAsset("Model", %filePath);
   else if( isSoundFormat(%fileExt))
      %this.addImportingAsset("Sound", %filePath);
   else if( %fileExt $= ".cs" || %fileExt $= ".cs.dso" )
      %this.addImportingAsset("Script", %filePath);
   else if( %fileExt $= ".gui" || %fileExt $= ".gui.dso" )
      %this.addImportingAsset("GUI", %filePath);
   else if (%fileExt $= ".zip")
      %this.onDropZipFile(%filePath);
      
   //Used to keep tabs on what files we were trying to import, used mainly in the event of
   //adjusting configs and needing to completely reprocess the import
   //ensure we're not doubling-up on files by accident
   if(%this.importingFilesArray.getIndexFromKey(%filePath) == -1)
      %this.importingFilesArray.add(%filePath);
}

function AssetBrowser::onDropZipFile(%this, %filePath)
{
   if(!%this.isVisible())
      return;
      
   %zip = new ZipObject();
   %zip.openArchive(%filePath);
   %count = %zip.getFileEntryCount();
   
   echo("Dropped in a zip file with" SPC %count SPC "files inside!");
   
   return;
   for (%i = 0; %i < %count; %i++)
   {
      %fileEntry = %zip.getFileEntry(%i);
      %fileFrom = getField(%fileEntry, 0);
      
      //First, we wanna scan to see if we have modules to contend with. If we do, we'll just plunk them in wholesale
      //and not process their contents.
      
      //If not modules, it's likely an art pack or other mixed files, so we'll import them as normal
      if( (%fileExt $= ".png") || (%fileExt $= ".jpg") || (%fileExt $= ".bmp") || (%fileExt $= ".dds") )
         %this.importAssetListArray.add("Image", %filePath);
      else if( (%fileExt $= ".dae") || (%fileExt $= ".dts"))
         %this.importAssetListArray.add("Model", %filePath);
      else if( (%fileExt $= ".ogg") || (%fileExt $= ".wav") || (%fileExt $= ".mp3"))
         %this.importAssetListArray.add("Sound", %filePath);
      else if( (%fileExt $= ".gui") || (%fileExt $= ".gui.dso"))
         %this.importAssetListArray.add("GUI", %filePath);
      //else if( (%fileExt $= ".cs") || (%fileExt $= ".dso"))
      //   %this.importAssetListArray.add("Script", %filePath);
      else if( (%fileExt $= ".mis"))
         %this.importAssetListArray.add("Level", %filePath);
         
      // For now, if it's a .cs file, we'll assume it's a behavior.
      if (fileExt(%fileFrom) !$= ".cs")
         continue;
      
      %fileTo = expandFilename("^game/behaviors/") @ fileName(%fileFrom);
      %zip.extractFile(%fileFrom, %fileTo);
      exec(%fileTo);
   }
}

function AssetBrowser::onDropImageFile(%this, %filePath)
{
   if(!%this.isVisible())
      return;
      
   // File Information madness
   %fileName         = %filePath;
   %fileOnlyName     = fileName( %fileName );
   %fileBase         = validateDatablockName(fileBase( %fileName ) @ "ImageMap");
   
   // [neo, 5/17/2007 - #3117]
   // Check if the file being dropped is already in data/images or a sub dir by checking if
   // the file path up to length of check path is the same as check path.
   %defaultPath = EditorSettings.value( "WorldEditor/defaultMaterialsPath" );

   %checkPath    = expandFilename( "^"@%defaultPath );
   %fileOnlyPath = expandFileName( %filePath ); //filePath( expandFileName( %filePath ) );
   %fileBasePath = getSubStr( %fileOnlyPath, 0, strlen( %checkPath ) );
   
   if( %checkPath !$= %fileBasePath )
   {
      // No match so file is from outside images directory and we need to copy it in
      %fileNewLocation = expandFilename("^"@%defaultPath) @ "/" @ fileBase( %fileName ) @ fileExt( %fileName );
   
      // Move to final location
      if( !pathCopy( %filePath, %fileNewLocation ) )
         return;
   }
   else 
   {  
      // Already in images path somewhere so just link to it
      %fileNewLocation = %filePath;
   }
   
   addResPath( filePath( %fileNewLocation ) );

   %matName = fileBase( %fileName );
      
   // Create Material
   %imap = new Material(%matName)
   {
	  mapTo = fileBase( %matName );
	  diffuseMap[0] = %defaultPath @ "/" @ fileBase( %fileName ) @ fileExt( %fileName );
   };
   //%imap.setName( %fileBase );
   //%imap.imageName = %fileNewLocation;
   //%imap.imageMode = "FULL";
   //%imap.filterPad = false;
   //%imap.compile();

   %diffusecheck = %imap.diffuseMap[0];
         
   // Bad Creation!
   if( !isObject( %imap ) )
      return;
      
   %this.addDatablock( %fileBase, false );
}

function AssetBrowser::onDropSoundFile(%this, %filePath)
{
   if(!%this.isVisible())
      return;
      
   // File Information madness
   %fileName         = %filePath;
   %fileOnlyName     = fileName( %fileName );
   %fileBase         = validateDatablockName(fileBase( %fileName ) @ "ImageMap");
   
   // [neo, 5/17/2007 - #3117]
   // Check if the file being dropped is already in data/images or a sub dir by checking if
   // the file path up to length of check path is the same as check path.
   %defaultPath = EditorSettings.value( "WorldEditor/defaultMaterialsPath" );

   %checkPath    = expandFilename( "^"@%defaultPath );
   %fileOnlyPath = expandFileName( %filePath ); //filePath( expandFileName( %filePath ) );
   %fileBasePath = getSubStr( %fileOnlyPath, 0, strlen( %checkPath ) );
   
   if( %checkPath !$= %fileBasePath )
   {
      // No match so file is from outside images directory and we need to copy it in
      %fileNewLocation = expandFilename("^"@%defaultPath) @ "/" @ fileBase( %fileName ) @ fileExt( %fileName );
   
      // Move to final location
      if( !pathCopy( %filePath, %fileNewLocation ) )
         return;
   }
   else 
   {  
      // Already in images path somewhere so just link to it
      %fileNewLocation = %filePath;
   }
   
   addResPath( filePath( %fileNewLocation ) );

   %matName = fileBase( %fileName );
      
   // Create Material
   %imap = new Material(%matName)
   {
	  mapTo = fileBase( %matName );
	  diffuseMap[0] = %defaultPath @ "/" @ fileBase( %fileName ) @ fileExt( %fileName );
   };
   //%imap.setName( %fileBase );
   //%imap.imageName = %fileNewLocation;
   //%imap.imageMode = "FULL";
   //%imap.filterPad = false;
   //%imap.compile();

   %diffusecheck = %imap.diffuseMap[0];
         
   // Bad Creation!
   if( !isObject( %imap ) )
      return;
      
   %this.addDatablock( %fileBase, false );
}

function AssetBrowser::onEndDropFiles( %this )
{
   if(!%this.isVisible())
      return;
   
   //we have assets to import, so go ahead and display the window for that now
   Canvas.pushDialog(AssetImportCtrl);
   ImportAssetWindow.visible = true;
   //ImportAssetWindow.validateAssets();
   ImportAssetWindow.refresh();
   ImportAssetWindow.selectWindow();
   
   // Update object library
   GuiFormManager::SendContentMessage($LBCreateSiderBar, %this, "refreshAll 1");
   
   if(ImportAssetWindow.importConfigsList.count() == 0)
   {
      MessageBoxOK( "Warning", "No base import config. Please create an import configuration set to simplify asset importing.");
   }
}
//
//
//

function AssetBrowser::reloadImportingFiles(%this)
{
   //Effectively, we re-import the files we were trying to originally. We'd only usually do this in the event we change our import config
   %this.onBeginDropFiles();
   
   for(%i=0; %i < %this.importingFilesArray.count(); %i++)
   {
      %this.onDropFile(%this.importingFilesArray.getKey(%i));
   }
    
   %this.onEndDropFiles();  
}

function AssetBrowser::ImportTemplateModules(%this)
{
   //AssetBrowser_ImportModule
   Canvas.pushDialog(AssetBrowser_ImportModuleTemplate);
   AssetBrowser_ImportModuleTemplateWindow.visible = true;   
   
   AssetBrowser_ImportModuleTemplateList.clear();
   
   //ModuleDatabase.scanModules("../../../../../../Templates/Modules/");
   
   %pattern = "../../../../../../Templates/Modules//*//*.module";   
   %file = findFirstFile( %pattern );

   while( %file !$= "" )
   {      
      echo("FOUND A TEMPLATE MODULE! " @ %file);
      %file = findNextFile( %pattern );
   }
   
   /*%moduleCheckbox = new GuiCheckBoxCtrl()
   {
      text = "Testadoo";
      moduleId = "";
   };
   
   AssetBrowser_ImportModuleTemplateList.addRow("0", "Testaroooooo");
   AssetBrowser_ImportModuleTemplateList.addRow("1", "Testadoooooo");*/
}

function AssetBrowser_ImportModuleTemplateList::onSelect(%this, %selectedRowIdx, %text)
{
   echo("Selected row: " @ %selectedRowIdx @ " " @ %text);
}

function AssetBrowser::addImportingAsset( %this, %assetType, %filePath, %parentAssetItem, %assetNameOverride )
{
   //In some cases(usually generated assets on import, like materials) we'll want to specifically define the asset name instead of peeled from the filePath
   if(%assetNameOverride !$= "")
      %assetName = %assetNameOverride;
   else
      %assetName = fileBase(%filePath);
      
   //We don't get a file path at all if we're a generated entry, like materials
   //if we have a file path, though, then sanitize it
   if(%filePath !$= "")
      %filePath = filePath(%filePath) @ "/" @ fileBase(%filePath) @ fileExt(%filePath);
   
   %moduleName = AssetBrowser.SelectedModule;
   ImportAssetModuleList.text = %moduleName;
   
   //Add to our main list
   %assetItem = new ScriptObject()
   {
      assetType = %assetType;
      filePath = %filePath;
      assetName = %assetName;
      cleanAssetName = %assetName; 
      moduleName = %moduleName;
      dirty  = true;
      parentAssetItem = %parentAssetItem;
      status = "";
      statusType = "";
      statusInfo = "";
      skip = false;
      processed = false;
   };

   //little bit of interception here
   if(%assetItem.assetType $= "Model")
   {
      %fileExt = fileExt(%assetItem.filePath);
      if(%fileExt $= ".dae")
      {
         %shapeInfo = new GuiTreeViewCtrl();
         enumColladaForImport(%assetItem.filePath, %shapeInfo, false);  
      }
      else
      {
         %shapeInfo = GetShapeInfo(%assetItem.filePath);
      }
      
      %assetItem.shapeInfo = %shapeInfo;
      
      %shapeItem = %assetItem.shapeInfo.findItemByName("Shape");
      %shapeCount = %assetItem.shapeInfo.getItemValue(%shapeItem);
      
      %animItem = %assetItem.shapeInfo.findItemByName("Animations");
      %animCount = %assetItem.shapeInfo.getItemValue(%animItem);
      
      //If the model has shapes AND animations, then it's a normal shape with embedded animations
      //if it has shapes and no animations it's a regular static mesh
      //if it has no shapes and animations, it's a special case. This means it's a shape animation only file so it gets flagged as special
      if(%shapeCount == 0 && %animCount != 0)
      {
         %assetItem.assetType = "Animation";
      }
      else if(%shapeCount == 0 && %animCount == 0)
      {
         //either it imported wrong or it's a bad file we can't read. Either way, don't try importing it
         error("Error - attempted to import a model file with no shapes or animations! Model in question was: " @ %filePath);
         
         %assetItem.delete();
         return 0;
      }
   }
   
   if(%parentAssetItem $= "")
   {
      ImportAssetTree.insertObject(0, %assetItem);
      
      //%assetItem.parentDepth = 0;
      //%this.importAssetNewListArray.add(%assetItem);
      //%this.importAssetUnprocessedListArray.add(%assetItem);
   }
   else
   {
      %parentid = ImportAssetTree.findItemByObjectId(%parentAssetItem);
      ImportAssetTree.insertObject(%parentid, %assetItem);
      
      //%assetItem.parentDepth = %parentAssetItem.parentDepth + 1;  
      //%parentIndex = %this.importAssetUnprocessedListArray.getIndexFromKey(%parentAssetItem);
      
      //%parentAssetItem.dependencies = %parentAssetItem.dependencies SPC %assetItem;
      //trim(%parentAssetItem.dependencies);
      
      //%this.importAssetUnprocessedListArray.insert(%assetItem, "", %parentIndex + 1);
   }
   
   %this.unprocessedAssetsCount++;
   
   return %assetItem;
}

function AssetBrowser::importLegacyGame(%this)
{
   
}

function AssetBrowser::importNewAssetFile(%this)
{
   %dlg = new OpenFileDialog()
   {
      Filters        = "Shape Files(*.dae, *.cached.dts)|*.dae;*.cached.dts|Images Files(*.jpg,*.png,*.tga,*.bmp,*.dds)|*.jpg;*.png;*.tga;*.bmp;*.dds|Any Files (*.*)|*.*|";
      DefaultPath    = $Pref::WorldEditor::LastPath;
      DefaultFile    = "";
      ChangePath     = false;
      OverwritePrompt = true;
      forceRelativePath = false;
      //MultipleFiles = true;
   };

   %ret = %dlg.Execute();
   
   if ( %ret )
   {
      $Pref::WorldEditor::LastPath = filePath( %dlg.FileName );
      %fullPath = %dlg.FileName;
      %file = fileBase( %fullPath );
   }   
   
   %dlg.delete();
   
   if ( !%ret )
      return;
      
   AssetBrowser.onBeginDropFiles();
   AssetBrowser.onDropFile(%fullPath);
   AssetBrowser.onEndDropFiles();
}

//
function ImportAssetButton::onClick(%this)
{
   ImportAssetsPopup.showPopup(Canvas);
}
//

//
function ImportAssetWindow::onWake(%this)
{
   //We've woken, meaning we're trying to import assets
   //Lets refresh our list
   if(!ImportAssetWindow.isVisible())
      return;
      
   $AssetBrowser::importConfigsFile = "tools/assetBrowser/assetImportConfigs.xml";
   
   %this.reloadImportOptionConfigs();
}

function ImportAssetWindow::reloadImportOptionConfigs(%this)
{
   ImportAssetWindow.importConfigsList = new ArrayObject();
   ImportAssetConfigList.clear();
   
   %xmlDoc = new SimXMLDocument();
   if(%xmlDoc.loadFile($AssetBrowser::importConfigsFile))
   {
      //StateMachine element
      %xmlDoc.pushFirstChildElement("AssetImportConfigs");
      
      //Configs
      %configCount = 0;
      while(%xmlDoc.pushChildElement(%configCount))
      {
         %configObj = new ScriptObject(){};
         
         %configObj.Name = %xmlDoc.attribute("Name");

         %xmlDoc.pushFirstChildElement("Mesh");
            %configObj.ImportMesh = %xmlDoc.attribute("ImportMesh");
            %configObj.DoUpAxisOverride = %xmlDoc.attribute("DoUpAxisOverride");
            %configObj.UpAxisOverride = %xmlDoc.attribute("UpAxisOverride");
            %configObj.DoScaleOverride = %xmlDoc.attribute("DoScaleOverride");
            %configObj.ScaleOverride = %xmlDoc.attribute("ScaleOverride");
            %configObj.IgnoreNodeScale = %xmlDoc.attribute("IgnoreNodeScale");
            %configObj.AdjustCenter = %xmlDoc.attribute("AdjustCenter");
            %configObj.AdjustFloor = %xmlDoc.attribute("AdjustFloor");
            %configObj.CollapseSubmeshes = %xmlDoc.attribute("CollapseSubmeshes");       
            %configObj.LODType = %xmlDoc.attribute("LODType");
            %configObj.ImportedNodes = %xmlDoc.attribute("ImportedNodes");
            %configObj.IgnoreNodes = %xmlDoc.attribute("IgnoreNodes");
            %configObj.ImportMeshes = %xmlDoc.attribute("ImportMeshes");
            %configObj.IgnoreMeshes = %xmlDoc.attribute("IgnoreMeshes");
         %xmlDoc.popElement();
         
         %xmlDoc.pushFirstChildElement("Materials");
            %configObj.ImportMaterials = %xmlDoc.attribute("ImportMaterials");
            %configObj.IgnoreMaterials = %xmlDoc.attribute("IgnoreMaterials");
            %configObj.CreateComposites = %xmlDoc.attribute("CreateComposites");
            %configObj.UseDiffuseSuffixOnOriginImg = %xmlDoc.attribute("UseDiffuseSuffixOnOriginImg");
            %configObj.UseExistingMaterials = %xmlDoc.attribute("UseExistingMaterials");
         %xmlDoc.popElement();
         
         %xmlDoc.pushFirstChildElement("Animations");
            %configObj.ImportAnimations = %xmlDoc.attribute("ImportAnimations");
            %configObj.SeparateAnimations = %xmlDoc.attribute("SeparateAnimations");
            %configObj.SeparateAnimationPrefix = %xmlDoc.attribute("SeparateAnimationPrefix");
         %xmlDoc.popElement();
         
         %xmlDoc.pushFirstChildElement("Collisions");
            %configObj.GenerateCollisions = %xmlDoc.attribute("GenerateCollisions");
            %configObj.GenCollisionType = %xmlDoc.attribute("GenCollisionType");
            %configObj.CollisionMeshPrefix = %xmlDoc.attribute("CollisionMeshPrefix");
            %configObj.GenerateLOSCollisions = %xmlDoc.attribute("GenerateLOSCollisions");
            %configObj.GenLOSCollisionType = %xmlDoc.attribute("GenLOSCollisionType");
            %configObj.LOSCollisionMeshPrefix = %xmlDoc.attribute("LOSCollisionMeshPrefix");
         %xmlDoc.popElement();
         
         %xmlDoc.pushFirstChildElement("Images");
            %configObj.ImageType = %xmlDoc.attribute("ImageType");
            %configObj.DiffuseTypeSuffixes = %xmlDoc.attribute("DiffuseTypeSuffixes");
            %configObj.NormalTypeSuffixes = %xmlDoc.attribute("NormalTypeSuffixes");
            %configObj.SpecularTypeSuffixes = %xmlDoc.attribute("SpecularTypeSuffixes");
            %configObj.MetalnessTypeSuffixes = %xmlDoc.attribute("MetalnessTypeSuffixes");
            %configObj.RoughnessTypeSuffixes = %xmlDoc.attribute("RoughnessTypeSuffixes");
            %configObj.SmoothnessTypeSuffixes = %xmlDoc.attribute("SmoothnessTypeSuffixes");
            %configObj.AOTypeSuffixes = %xmlDoc.attribute("AOTypeSuffixes");
            %configObj.CompositeTypeSuffixes = %xmlDoc.attribute("CompositeTypeSuffixes");
            %configObj.TextureFilteringMode = %xmlDoc.attribute("TextureFilteringMode");
            %configObj.UseMips = %xmlDoc.attribute("UseMips");
            %configObj.IsHDR = %xmlDoc.attribute("IsHDR");
            %configObj.Scaling = %xmlDoc.attribute("Scaling");
            %configObj.Compressed = %xmlDoc.attribute("Compressed");
            %configObj.GenerateMaterialOnImport = %xmlDoc.attribute("GenerateMaterialOnImport");
            %configObj.PopulateMaterialMaps = %xmlDoc.attribute("PopulateMaterialMaps");
         %xmlDoc.popElement();
         
         %xmlDoc.pushFirstChildElement("Sounds");
            %configObj.VolumeAdjust = %xmlDoc.attribute("VolumeAdjust");
            %configObj.PitchAdjust = %xmlDoc.attribute("PitchAdjust");
            %configObj.Compressed = %xmlDoc.attribute("Compressed");
         %xmlDoc.popElement();
         
         %xmlDoc.popElement();
         %configCount++;
         
         ImportAssetWindow.importConfigsList.add(%configObj);
      }
      
      %xmlDoc.popElement();
   }
   
   for(%i = 0; %i < ImportAssetWindow.importConfigsList.count(); %i++)
   {
      %configObj = ImportAssetWindow.importConfigsList.getKey(%i);
      ImportAssetConfigList.add(%configObj.Name);
   }
   
   %importConfigIdx = ImportAssetWindow.activeImportConfigIndex;
   if(%importConfigIdx $= "")
      %importConfigIdx = 0;
      
   ImportAssetConfigList.setSelected(%importConfigIdx);
}

function ImportAssetWindow::setImportOptions(%this, %optionsObj)
{
   //Todo, editor + load from files for preconfigs
   
   //General
   %optionsObj.treatWarningsAsErrors = false;
   %optionsObj.ignoreDuplicateAssets = false;
   
   //Meshes
   %optionsObj.ImportMesh = true;
   %optionsObj.UpAxisOverride = "Z_AXIS";
   %optionsObj.OverrideScale = 1.0;
   %optionsObj.IgnoreNodeScale = false;
   %optionsObj.AdjustCenter = false;
   %optionsObj.AdjustFloor = false;
   %optionsObj.CollapseSubmeshes = false;
   %optionsObj.LODType = "TrailingNumber";
   %optionsObj.TrailingNumber = 2;
   %optionsObj.ImportedNodes = "";
   %optionsObj.IgnoreNodes = "";
   %optionsObj.ImportMeshes = "";
   %optionsObj.IgnoreMeshes = "";
   
   //Materials
   %optionsObj.ImportMaterials = true;
   %optionsObj.CreateComposites = true;
   
   //Animations
   %optionsObj.ImportAnimations = true;
   %optionsObj.SeparateAnimations = true;
   %optionsObj.SeparateAnimationPrefix = "";
   
   //Collision
   %optionsObj.GenerateCollisions = true;
   %optionsObj.GenCollisionType = "CollisionMesh";
   %optionsObj.CollisionMeshPrefix = "Collision";
   %optionsObj.GenerateLOSCollisions = true;
   %optionsObj.GenLOSCollisionType = "CollisionMesh";
   %optionsObj.LOSCollisionMeshPrefix = "LOS";
   
   //Images
   %optionsObj.ImageType = "Diffuse";
   %optionsObj.DiffuseTypeSuffixes = "_ALBEDO,_DIFFUSE,_ALB,_DIF,_COLOR,_COL";
   %optionsObj.NormalTypeSuffixes = "_NORMAL,_NORM";
   %optionsObj.SpecularTypeSuffixes = "_SPECULAR,_SPEC";
   %optionsObj.MetalnessTypeSuffixes = "_METAL,_MET,_METALNESS,_METALLIC";
   %optionsObj.RoughnessTypeSuffixes = "_ROUGH,_ROUGHNESS";
   %optionsObj.SmoothnessTypeSuffixes = "_SMOOTH,_SMOOTHNESS";
   %optionsObj.AOTypeSuffixes = "_AO,_AMBIENT,_AMBIENTOCCLUSION";
   %optionsObj.CompositeTypeSuffixes = "_COMP,_COMPOSITE";
   %optionsObj.TextureFilteringMode = "Bilinear";
   %optionsObj.UseMips = true;
   %optionsObj.IsHDR = false;
   %optionsObj.Scaling = 1.0;
   %optionsObj.Compressed = true;
   
   //Sounds
   %optionsObj.VolumeAdjust = 1.0;
   %optionsObj.PitchAdjust = 1.0;
   %optionsObj.Compressed = false;
}

//
function ImportAssetWindow::processNewImportAssets(%this, %id)
{
   while(%id > 0)
   {
      %assetItem = ImportAssetTree.getItemObject(%id);
      
      if(%assetItem.processed == false)
      {
         %assetConfigObj = ImportAssetWindow.activeImportConfig.clone();
         %assetConfigObj.assetIndex = %i;

         //sanetize before modifying our asset name(suffix additions, etc)      
         if(%assetItem.assetName !$= %assetItem.cleanAssetName)
            %assetItem.assetName = %assetItem.cleanAssetName;
            
         %assetConfigObj.assetName = %assetItem.assetName;
         
         if(%assetItem.assetType $= "Model")
         {
            %fileExt = fileExt(%assetItem.filePath);
            if(%fileExt $= ".dae")
            {
               %shapeInfo = new GuiTreeViewCtrl();
               enumColladaForImport(%assetItem.filePath, %shapeInfo, false);  
            }
            else
            {
               %shapeInfo = GetShapeInfo(%assetItem.filePath);
            }
            
            %assetItem.shapeInfo = %shapeInfo;
         
            %shapeItem = %assetItem.shapeInfo.findItemByName("Shape");
            %shapeCount = %assetItem.shapeInfo.getItemValue(%shapeItem);
            
            if(%assetConfigObj.ImportMesh == 1 && %shapeCount > 0)
            {
               
            }
            
            %animItem = %assetItem.shapeInfo.findItemByName("Animations");
            %animCount = %assetItem.shapeInfo.getItemValue(%animItem);
            
            if(%assetConfigObj.ImportAnimations == 1 && %animCount > 0)
            {
               %animationItem = %assetItem.shapeInfo.getChild(%animItem);
               
               %animName = %assetItem.shapeInfo.getItemText(%animationItem);
               //%animName = %assetItem.shapeInfo.getItemValue(%animationItem);
               
               AssetBrowser.addImportingAsset("Animation", %animName, %assetItem);
               
               %animationItem = %assetItem.shapeInfo.getNextSibling(%animationItem);
               while(%animationItem != 0)
               {
                  %animName = %assetItem.shapeInfo.getItemText(%animationItem);
                  //%animName = %assetItem.shapeInfo.getItemValue(%animationItem);
                  
                  AssetBrowser.addImportingAsset("Animation", %animName, %assetItem);
                     
                  %animationItem = %shapeInfo.getNextSibling(%animationItem);
               }
            }
            
            %matItem = %assetItem.shapeInfo.findItemByName("Materials");
            %matCount = %assetItem.shapeInfo.getItemValue(%matItem);
            
            if(%assetConfigObj.importMaterials == 1 && %matCount > 0)
            {
               %materialItem = %assetItem.shapeInfo.getChild(%matItem);
               
               %matName = %assetItem.shapeInfo.getItemText(%materialItem);
               
               %filePath = %assetItem.shapeInfo.getItemValue(%materialItem);
               if(%filePath !$= "")
               {
                  AssetBrowser.addImportingAsset("Material", %filePath, %assetItem);
               }
               else
               {
                  //we need to try and find our material, since the shapeInfo wasn't able to find it automatically
                  %filePath = findImageFile(filePath(%assetItem.filePath), %matName);
                  if(%filePath !$= "")
                     AssetBrowser.addImportingAsset("Material", %filePath, %assetItem);
                  else
                     AssetBrowser.addImportingAsset("Material", %matName, %assetItem);
               }
               
               %materialItem = %assetItem.shapeInfo.getNextSibling(%materialItem);
               while(%materialItem != 0)
               {
                  %matName = %assetItem.shapeInfo.getItemText(%materialItem);
                  %filePath = %assetItem.shapeInfo.getItemValue(%materialItem);
                  if(%filePath !$= "")
                  {
                     AssetBrowser.addImportingAsset("Material", %filePath, %assetItem);
                  }
                  else
                  {
                     //we need to try and find our material, since the shapeInfo wasn't able to find it automatically
                     %filePath = findImageFile(filePath(%assetItem.filePath), %matName);
                     if(%filePath !$= "")
                        AssetBrowser.addImportingAsset("Material", %filePath, %assetItem);
                     else
                        AssetBrowser.addImportingAsset("Material", %matName, %assetItem);
                  }
                     
                  %materialItem = %shapeInfo.getNextSibling(%materialItem);
               }
            }
         }
         else if(%assetItem.assetType $= "Animation")
         {
            //if we don't have our own file, that means we're gunna be using our parent shape's file so reference that
            if(!isFile(%assetItem.filePath))
            {
               %assetItem.filePath = %assetItem.parentAssetItem.filePath;
            }
         }
         else if(%assetItem.assetType $= "Material")
         {
            AssetBrowser.prepareImportMaterialAsset(%assetItem);
         } 
         else if(%assetItem.assetType $= "Image")
         {
            AssetBrowser.prepareImportImageAsset(%assetItem);
         }
         
         %assetItem.processed = true;
      }
      
      //AssetBrowser.importAssetUnprocessedListArray.erase(0);    
      //Been processed, so add it to our final list
      //AssetBrowser.importAssetFinalListArray.add(%assetItem);
      
      if(ImportAssetTree.isParentItem(%id))
      {
         %childItem = ImportAssetTree.getChild(%id);
         
         //recurse!
         %this.processNewImportAssets(%childItem); 
      }

      %id = ImportAssetTree.getNextSibling(%id);
   }
}

function ImportAssetWindow::findImportingAssetByName(%this, %assetName)
{
   %id = ImportAssetTree.getFirstRootItem();
   
   return %this._findImportingAssetByName(%id, %assetName);
}

function ImportAssetWindow::_findImportingAssetByName(%this, %id, %assetName)
{
   while(%id > 0)
   {
      %assetItem = ImportAssetTree.getItemObject(%id);
      
      if(%assetItem.cleanAssetName $= %assetName)
      {
         return %asset;
      }
      
      if(ImportAssetTree.isParentItem(%id))
      {
         %childItem = ImportAssetTree.getChild(%id);
         
         //recurse!
         %ret = %this._findImportingAssetByName(%childItem, %assetName);
         if(%ret != 0)
            return %ret;
      }

      %id = ImportAssetTree.getNextSibling(%id);
   }
   
   return 0;
}

function ImportAssetWindow::parseImageSuffixes(%this, %assetItem)
{
   //diffuse
   %suffixCount = getTokenCount(ImportAssetWindow.activeImportConfig.DiffuseTypeSuffixes, ",;");
   for(%sfx = 0; %sfx < %suffixCount; %sfx++)
   {
      %suffixToken = getToken(ImportAssetWindow.activeImportConfig.DiffuseTypeSuffixes, ",;", %sfx);
      if(strIsMatchExpr("*"@%suffixToken, %assetItem.AssetName))
      {
         %assetItem.imageSuffixType = %suffixToken;
         return "diffuse";
      }
   }
   
   //normal
   %suffixCount = getTokenCount(ImportAssetWindow.activeImportConfig.NormalTypeSuffixes, ",;");
   for(%sfx = 0; %sfx < %suffixCount; %sfx++)
   {
      %suffixToken = getToken(ImportAssetWindow.activeImportConfig.NormalTypeSuffixes, ",;", %sfx);
      if(strIsMatchExpr("*"@%suffixToken, %assetItem.AssetName))
      {
         %assetItem.imageSuffixType = %suffixToken;
         return "normal";
      }
   }
   
   //roughness
   %suffixCount = getTokenCount(ImportAssetWindow.activeImportConfig.RoughnessTypeSuffixes, ",;");
   for(%sfx = 0; %sfx < %suffixCount; %sfx++)
   {
      %suffixToken = getToken(ImportAssetWindow.activeImportConfig.RoughnessTypeSuffixes, ",;", %sfx);
      if(strIsMatchExpr("*"@%suffixToken, %assetItem.AssetName))
      {
         %assetItem.imageSuffixType = %suffixToken;
         return "roughness";
      }
   }
   
   //Ambient Occlusion
   %suffixCount = getTokenCount(ImportAssetWindow.activeImportConfig.AOTypeSuffixes, ",;");
   for(%sfx = 0; %sfx < %suffixCount; %sfx++)
   {
      %suffixToken = getToken(ImportAssetWindow.activeImportConfig.AOTypeSuffixes, ",;", %sfx);
      if(strIsMatchExpr("*"@%suffixToken, %assetItem.AssetName))
      {
         %assetItem.imageSuffixType = %suffixToken;
         return "AO";
      }
   }
   
   //metalness
   %suffixCount = getTokenCount(ImportAssetWindow.activeImportConfig.MetalnessTypeSuffixes, ",;");
   for(%sfx = 0; %sfx < %suffixCount; %sfx++)
   {
      %suffixToken = getToken(ImportAssetWindow.activeImportConfig.MetalnessTypeSuffixes, ",;", %sfx);
      if(strIsMatchExpr("*"@%suffixToken, %assetItem.AssetName))
      {
         %assetItem.imageSuffixType = %suffixToken;
         return "metalness";
      }
   }
   
   //composite
   %suffixCount = getTokenCount(ImportAssetWindow.activeImportConfig.CompositeTypeSuffixes, ",;");
   for(%sfx = 0; %sfx < %suffixCount; %sfx++)
   {
      %suffixToken = getToken(ImportAssetWindow.activeImportConfig.CompositeTypeSuffixes, ",;", %sfx);
      if(strIsMatchExpr("*"@%suffixToken, %assetItem.AssetName))
      {
         %assetItem.imageSuffixType = %suffixToken;
         return "composite";
      }
   }
   
   //specular
   %suffixCount = getTokenCount(ImportAssetWindow.activeImportConfig.SpecularTypeSuffixes, ",;");
   for(%sfx = 0; %sfx < %suffixCount; %sfx++)
   {
      %suffixToken = getToken(ImportAssetWindow.activeImportConfig.SpecularTypeSuffixes, ",;", %sfx);
      if(strIsMatchExpr("*"@%suffixToken, %assetItem.AssetName))
      {
         %assetItem.imageSuffixType = %suffixToken;
         return "specular";
      }
   }
   
   return "";
}

function ImportAssetWindow::parseImagePathSuffixes(%this, %filePath)
{
   //diffuse
   %suffixCount = getTokenCount(ImportAssetWindow.activeImportConfig.DiffuseTypeSuffixes, ",");
   for(%sfx = 0; %sfx < %suffixCount; %sfx++)
   {
      %suffixToken = getToken(ImportAssetWindow.activeImportConfig.DiffuseTypeSuffixes, ",", %sfx);
      if(strIsMatchExpr("*"@%suffixToken, %filePath))
      {
         return "diffuse";
      }
   }
   
   //normal
   %suffixCount = getTokenCount(ImportAssetWindow.activeImportConfig.NormalTypeSuffixes, ",");
   for(%sfx = 0; %sfx < %suffixCount; %sfx++)
   {
      %suffixToken = getToken(ImportAssetWindow.activeImportConfig.NormalTypeSuffixes, ",", %sfx);
      if(strIsMatchExpr("*"@%suffixToken, %filePath))
      {
         return "normal";
      }
   }
   
   //roughness
   %suffixCount = getTokenCount(ImportAssetWindow.activeImportConfig.RoughnessTypeSuffixes, ",");
   for(%sfx = 0; %sfx < %suffixCount; %sfx++)
   {
      %suffixToken = getToken(ImportAssetWindow.activeImportConfig.RoughnessTypeSuffixes, ",", %sfx);
      if(strIsMatchExpr("*"@%suffixToken, %filePath))
      {
         return "roughness";
      }
   }
   
   //Ambient Occlusion
   %suffixCount = getTokenCount(ImportAssetWindow.activeImportConfig.AOTypeSuffixes, ",");
   for(%sfx = 0; %sfx < %suffixCount; %sfx++)
   {
      %suffixToken = getToken(ImportAssetWindow.activeImportConfig.AOTypeSuffixes, ",", %sfx);
      if(strIsMatchExpr("*"@%suffixToken, %filePath))
      {
         return "AO";
      }
   }
   
   //metalness
   %suffixCount = getTokenCount(ImportAssetWindow.activeImportConfig.MetalnessTypeSuffixes, ",");
   for(%sfx = 0; %sfx < %suffixCount; %sfx++)
   {
      %suffixToken = getToken(ImportAssetWindow.activeImportConfig.MetalnessTypeSuffixes, ",", %sfx);
      if(strIsMatchExpr("*"@%suffixToken, %filePath))
      {
         return "metalness";
      }
   }
   
   //composite
   %suffixCount = getTokenCount(ImportAssetWindow.activeImportConfig.CompositeTypeSuffixes, ",");
   for(%sfx = 0; %sfx < %suffixCount; %sfx++)
   {
      %suffixToken = getToken(ImportAssetWindow.activeImportConfig.CompositeTypeSuffixes, ",", %sfx);
      if(strIsMatchExpr("*"@%suffixToken, %filePath))
      {
         return "composite";
      }
   }
   
   //specular
   %suffixCount = getTokenCount(ImportAssetWindow.activeImportConfig.SpecularTypeSuffixes, ",");
   for(%sfx = 0; %sfx < %suffixCount; %sfx++)
   {
      %suffixToken = getToken(ImportAssetWindow.activeImportConfig.SpecularTypeSuffixes, ",", %sfx);
      if(strIsMatchExpr("*"@%suffixToken, %filePath))
      {
         return "specular";
      }
   }
   
   return "";
}

function refreshImportAssetWindow()
{
   ImportAssetWindow.refresh();  
}

function ImportAssetWindow::refresh(%this)
{
   //Go through and process any newly, unprocessed assets
   %id = ImportAssetTree.getFirstRootItem();
   
   %this.processNewImportAssets(%id);
   
   %this.indentCount = 0;
   
   ImportingAssetList.clear();
   
   if(AssetBrowser.importAssetUnprocessedListArray.count() == 0)
   {
      //We've processed them all, prep the assets for actual importing
      //Initial set of assets
      %id = ImportAssetTree.getFirstRootItem();
      
      //recurse!
      %this.refreshChildItem(%id);   
   }
   else
   {
      //Continue processing
      %this.refresh();  
   }
}

function ImportAssetWindow::refreshChildItem(%this, %id)
{
   while (%id > 0)
   {
      %assetItem = ImportAssetTree.getItemObject(%id);
      
      if(%assetItem.skip)
      {
         %id = ImportAssetTree.getNextSibling(%id);
         continue;  
      }
      
      %assetType = %assetItem.assetType;
      %filePath = %assetItem.filePath;
      %assetName = %assetItem.assetName;
      
      //validate
      %this.validateAsset(%assetItem);
      
      //Once validated, attempt any fixes for issues
      %this.resolveIssue(%assetItem);
      
      //Make sure we size correctly
      ImportingAssetList.extent.x = ImportingAssetList.getParent().extent.x - 15;
      
      //create!
      %width = mRound(mRound(ImportingAssetList.extent.x) / 2);
      %height = 20;
      %indent = %this.indentCount * 16;
      %toolTip = "";
      
      %iconPath = "tools/gui/images/iconInformation";
      %configCommand = "ImportAssetOptionsWindow.editImportSettings(" @ %assetItem @ ");";
      
      if(%assetType $= "Model" || %assetType $= "Animation" || %assetType $= "Image" || %assetType $= "Sound")
      {
         /*if(%assetItem.status $= "Error")
         {
            %iconPath = "tools/gui/images/iconError";
            %configCommand = "ImportAssetOptionsWindow.findMissingFile(" @ %assetItem @ ");";
         }
         else*/
         if(%assetItem.status $= "Warning")
         {
            %iconPath = "tools/gui/images/iconWarn";
            %configCommand = "ImportAssetOptionsWindow.fixIssues(" @ %assetItem @ ");";
            
            if(%assetItem.statusType $= "DuplicateAsset" || %assetItem.statusType $= "DuplicateImportAsset")
               %assetName = %assetItem.assetName @ " <Duplicate Asset>";
         }
         
         %toolTip = %assetItem.statusInfo;
      }
      else
      {
         if(%assetItem.status $= "Error")
         {
            %iconPath = "tools/gui/images/iconError";
            %configCommand = "";//"ImportAssetOptionsWindow.findMissingFile(" @ %assetItem @ ");";
         }
         else if(%assetItem.status $= "Warning")
         {
            %iconPath = "tools/gui/images/iconWarn";
            %configCommand = "";//"ImportAssetOptionsWindow.fixIssues(" @ %assetItem @ ");";
            
            if(%assetItem.statusType $= "DuplicateAsset" || %assetItem.statusType $= "DuplicateImportAsset")
               %assetName = %assetItem.assetName @ " <Duplicate Asset>";
         }
      }
      
      %inputCellPos = %indent;
      %inputCellWidth = (ImportingAssetList.extent.x * 0.3) - %indent;
      
      %filePathBtnPos = %inputCellPos + %inputCellWidth - %height;
      
      %assetNameCellPos = %inputCellPos + %inputCellWidth;
      %assetNameCellWidth = ImportingAssetList.extent.x * 0.3;
      
      %assetTypeCellPos = %assetNameCellPos + %assetNameCellWidth;
      %assetTypeCellWidth = ImportingAssetList.extent.x * 0.3;
      
      %configBtnPos = %assetTypeCellPos + %assetTypeCellWidth - (%height * 2);
      %configBtnWidth = %height;
      
      %delBtnPos = %assetTypeCellPos + %assetTypeCellWidth - %height;
      %delBtnWidth = %height;
      
      %inputField = %filePath;
      
      //Check if it's a generated type, like materials
      %inputPathProfile = ToolsGuiTextEditProfile;
      %generatedField = false;
      if(%assetType $= "Material")
      {
         %inputField = "(Generated)";
         %generatedField = true;
      }
      else
      {
         //nope, so check that it's a valid file path. If not, flag it as such
         if(%assetItem.status $= "Error")
         {
            if(!isFile(%filePath))
            {
               %inputField = "File not found!";
               %inputPathProfile = ToolsGuiTextEditErrorProfile;
            }
         }
      }
      
      %importEntry = new GuiControl()
      {
         position = "0 0";
         extent = ImportingAssetList.extent.x SPC %height;
         horzSizing = "width";
         vertSizing = "bottom";
         
         new GuiTextEditCtrl()
         {
            Text = %inputField; 
            position = %inputCellPos SPC "0";
            extent = %inputCellWidth SPC %height;
            internalName = "InputPath";
            active = false;
            profile = %inputPathProfile;
            horzSizing = "width";
            vertSizing = "bottom";
         };
         
         new GuiButtonCtrl()
         {
            position = %filePathBtnPos SPC "0";
            extent = %height SPC %height;
            command = "ImportAssetOptionsWindow.findMissingFile(" @ %assetItem @ ");";
            text = "...";
            internalName = "InputPathButton";
            tooltip = %toolTip;
            visible = !%generatedField;
            horzSizing = "width";
            vertSizing = "bottom";
         };
         
         new GuiTextEditCtrl()
         {
           Text = %assetName; 
           position = %assetNameCellPos SPC "0";
           extent = %assetNameCellWidth SPC %height;
           internalName = "AssetName";
           horzSizing = "width";
            vertSizing = "bottom";
         };
         
         new GuiTextEditCtrl()
         {
           Text = %assetType; 
           position = %assetTypeCellPos SPC "0";
           extent = %assetTypeCellWidth SPC %height;
           active = false;
           internalName = "AssetType";
           horzSizing = "width";
            vertSizing = "bottom";
         };
         
         new GuiBitmapButtonCtrl()
         {
            position = %configBtnPos SPC "0";
            extent = %height SPC %height;
            command = "ImportAssetWindow.importResolution(" @ %assetItem @ ");";
            bitmap = %iconPath;
            tooltip = %toolTip;
            horzSizing = "width";
            vertSizing = "bottom";
         };
         new GuiBitmapButtonCtrl()
         {
            position = %delBtnPos SPC "0";
            extent = %height SPC %height;
            command = "ImportAssetOptionsWindow.deleteImportingAsset(" @ %assetItem @ ");";
            bitmap = "tools/gui/images/iconDelete";
            horzSizing = "width";
            vertSizing = "bottom";
         };
      };
      
      ImportingAssetList.add(%importEntry);
      
      if(ImportAssetTree.isParentItem(%id))
      {
         %this.indentCount++;  
         
         %childItem = ImportAssetTree.getChild(%id);
         
         //recurse!
         %this.refreshChildItem(%childItem); 
      }

      %id = ImportAssetTree.getNextSibling(%id);
   }
   
   %this.indentCount--;
}
//

function ImportAssetWindow::importResolution(%this, %assetItem)
{
   if(%assetItem.status !$= "Error" && %assetItem.status !$= "Warning")
   {
      //If nothing's wrong, we just edit it
      ImportAssetOptionsWindow.editImportSettings(%assetItem);
      return;
   }
   else
   {
      ImportAssetResolutionsPopup.assetItem = %assetItem;
      if(%assetItem.statusType $= "DuplicateAsset" || %assetItem.statusType $= "DuplicateImportAsset")
      {
         ImportAssetResolutionsPopup.enableItem(3, false); //Rename
         ImportAssetResolutionsPopup.enableItem(5, false); //Find Missing
      }
      else if(%assetItem.statusType $= "MissingFile")
      {
         ImportAssetResolutionsPopup.enableItem(0, false); //Use Orig
         ImportAssetResolutionsPopup.enableItem(1, false); //Use Dupe
         ImportAssetResolutionsPopup.enableItem(3, false); //Rename
      }
   }
   
   ImportAssetResolutionsPopup.showPopup(Canvas);  
}

function ImportAssetWindow::validateAssets(%this)
{
   %assetCount = AssetBrowser.importAssetFinalListArray.count();
   %moduleName = ImportAssetModuleList.getText();
   %assetQuery = new AssetQuery();
   
   %hasIssues = false;
   
   //First, check the obvious: name collisions. We should have no asset that shares a similar name.
   //If we do, prompt for it be renamed first before continuing
   
   for(%i=0; %i < %assetCount; %i++)
   {
      %assetItemA = AssetBrowser.importAssetFinalListArray.getKey(%i);
      
      //First, check our importing assets for name collisions
      for(%j=0; %j < %assetCount; %j++)
      {
         %assetItemB = AssetBrowser.importAssetFinalListArray.getKey(%j);
         if( (%assetItemA.assetName $= %assetItemB.assetName) && (%i != %j) )
         {
            //yup, a collision, prompt for the change and bail out
            /*MessageBoxOK( "Error!", "Duplicate asset names found with importing assets!\nAsset \"" @ 
               %assetItemA.assetName @ "\" of type \"" @ %assetItemA.assetType @ "\" and \"" @
               %assetItemB.assetName @ "\" of type \"" @ %assetItemB.assetType @ "\" have matching names.\nPlease rename one of them and try again!");*/
               
            %assetItemA.status = "Warning";
            %assetItemA.statusType = "DuplicateImportAsset";
            %assetItemA.statusInfo = "Duplicate asset names found with importing assets!\nAsset \"" @ 
               %assetItemA.assetName @ "\" of type \"" @ %assetItemA.assetType @ "\" and \"" @
               %assetItemB.assetName @ "\" of type \"" @ %assetItemB.assetType @ "\" have matching names.\nPlease rename one of them and try again!";
               
            %hasIssues = true;
         }
      }
      
      //No collisions of for this name in the importing assets. Now, check against the existing assets in the target module
      if(!AssetBrowser.isAssetReImport)
      {
         %numAssetsFound = AssetDatabase.findAllAssets(%assetQuery);

         %foundCollision = false;
         for( %f=0; %f < %numAssetsFound; %f++)
         {
            %assetId = %assetQuery.getAsset(%f);
             
            //first, get the asset's module, as our major categories
            %module = AssetDatabase.getAssetModule(%assetId);
            
            %testModuleName = %module.moduleId;
            
            //These are core, native-level components, so we're not going to be messing with this module at all, skip it
            if(%moduleName !$= %testModuleName)
               continue;

            %testAssetName = AssetDatabase.getAssetName(%assetId);
            
            if(%testAssetName $= %assetItemA.assetName)
            {
               %foundCollision = true;
               
               %assetItemA.status = "Warning";
               %assetItemA.statusType = "DuplicateAsset";
               %assetItemA.statusInfo = "Duplicate asset names found with the target module!\nAsset \"" @ 
                  %assetItemA.assetName @ "\" of type \"" @ %assetItemA.assetType @ "\" has a matching name.\nPlease rename it and try again!";
                  
               break;            
            }
         }
         
         if(%foundCollision == true)
         {
            %hasIssues = true;
            
            //yup, a collision, prompt for the change and bail out
            /*MessageBoxOK( "Error!", "Duplicate asset names found with the target module!\nAsset \"" @ 
               %assetItemA.assetName @ "\" of type \"" @ %assetItemA.assetType @ "\" has a matching name.\nPlease rename it and try again!");*/
               
            //%assetQuery.delete();
            //return false;
         }
      }
      
      //Check if we were given a file path(so not generated) but somehow isn't a valid file
      if(%assetItemA.filePath !$= "" && !isFile(%assetItemA.filePath))
      {
         %hasIssues = true;  
         %assetItemA.status = "error";
         %assetItemA.statusType = "MissingFile";
         %assetItemA.statusInfo = "Unable to find file to be imported. Please select asset file.";
      }
   }
   
   //Clean up our queries
   %assetQuery.delete();
   
   if(%hasIssues)
      return false;
   else
      return true;
}

function ImportAssetWindow::ImportAssets(%this)
{
   //do the actual importing, now!
   %assetCount = AssetBrowser.importAssetFinalListArray.count();
   
   //get the selected module data
   %moduleName = ImportAssetModuleList.getText();
   
   %module = ModuleDatabase.findModule(%moduleName, 1);
   
   if(!isObject(%module))
   {
      MessageBoxOK( "Error!", "No module selected. You must select or create a module for the assets to be added to.");
      return;
   }
   
   %id = ImportAssetTree.getFirstRootItem();
   
   %this.doImportAssets(%id);
   
   //force an update of any and all modules so we have an up-to-date asset list
   AssetBrowser.loadFilters();
   AssetBrowser.refreshPreviews();
   Canvas.popDialog(AssetImportCtrl);
   AssetBrowser.isAssetReImport = false;
}

function ImportAssetWindow::doImportAssets(%this, %id)
{
   while(%id > 0)
   {
      %assetItem = ImportAssetTree.getItemObject(%id);
      
      if(%assetItem.skip)
      {
         %id = ImportAssetTree.getNextSibling(%id);
         continue;  
      }
      
      %assetType = %assetItem.AssetType;
      %filePath = %assetItem.filePath;
      %assetName = %assetItem.assetName;
      %assetImportSuccessful = false;
      %assetId = %moduleName@":"@%assetName;
      
      if(%assetType $= "Image")
      {
         AssetBrowser.importImageAsset(%assetItem);
      }
      else if(%assetType $= "Model")
      {
         AssetBrowser.importShapeAsset(%assetItem);
      }
      else if(%assetType $= "Animation")
      {
         %assetPath = "data/" @ %moduleName @ "/ShapeAnimations";
         %assetFullPath = %assetPath @ "/" @ fileName(%filePath);
         
         %newAsset = new ShapeAnimationAsset()
         {
            assetName = %assetName;
            versionId = 1;
            fileName = %assetFullPath;
            originalFilePath = %filePath;
            animationFile = %assetFullPath;
            animationName = %assetName;
            startFrame = 0;
            endFrame = -1;
            padRotation = false;
            padTransforms = false;
         };

         %assetImportSuccessful = TAMLWrite(%newAsset, %assetPath @ "/" @ %assetName @ ".asset.taml"); 
         
         //and copy the file into the relevent directory
         %doOverwrite = !AssetBrowser.isAssetReImport;
         if(!pathCopy(%filePath, %assetFullPath, %doOverwrite))
         {
            error("Unable to import asset: " @ %filePath);
         }
      }
      else if(%assetType $= "Sound")
      {
         %assetPath = "data/" @ %moduleName @ "/Sounds";
         %assetFullPath = %assetPath @ "/" @ fileName(%filePath);
         
         %newAsset = new SoundAsset()
         {
            assetName = %assetName;
            versionId = 1;
            fileName = %assetFullPath;
            originalFilePath = %filePath;
         };
         
         %assetImportSuccessful = TAMLWrite(%newAsset, %assetPath @ "/" @ %assetName @ ".asset.taml"); 
         
         //and copy the file into the relevent directory
         %doOverwrite = !AssetBrowser.isAssetReImport;
         if(!pathCopy(%filePath, %assetFullPath, %doOverwrite))
         {
            error("Unable to import asset: " @ %filePath);
         }
      }
      else if(%assetType $= "Material")
      {
         AssetBrowser.importMaterialAsset(%assetItem);
      }
      else if(%assetType $= "Script")
      {
         %assetPath = "data/" @ %moduleName @ "/Scripts";
         %assetFullPath = %assetPath @ "/" @ fileName(%filePath);
         
         %newAsset = new ScriptAsset()
         {
            assetName = %assetName;
            versionId = 1;
            scriptFilePath = %assetFullPath;
            isServerSide = true;
            originalFilePath = %filePath;
         };
         
         %assetImportSuccessful = TAMLWrite(%newAsset, %assetPath @ "/" @ %assetName @ ".asset.taml"); 
         
         //and copy the file into the relevent directory
         %doOverwrite = !AssetBrowser.isAssetReImport;
         if(!pathCopy(%filePath, %assetFullPath, %doOverwrite))
         {
            error("Unable to import asset: " @ %filePath);
         }
      }
      else if(%assetType $= "GUI")
      {
         %assetPath = "data/" @ %moduleName @ "/GUIs";
         %assetFullPath = %assetPath @ "/" @ fileName(%filePath);
         
         %newAsset = new GUIAsset()
         {
            assetName = %assetName;
            versionId = 1;
            GUIFilePath = %assetFullPath;
            scriptFilePath = "";
            originalFilePath = %filePath;
         };
         
         %assetImportSuccessful = TAMLWrite(%newAsset, %assetPath @ "/" @ %assetName @ ".asset.taml"); 
         
         //and copy the file into the relevent directory
         %doOverwrite = !AssetBrowser.isAssetReImport;
         if(!pathCopy(%filePath, %assetFullPath, %doOverwrite))
         {
            error("Unable to import asset: " @ %filePath);
         }
      }
      
      if(%assetImportSuccessful)
      {
         %moduleDef = ModuleDatabase.findModule(%moduleName,1);
         
         if(!AssetBrowser.isAssetReImport)
            AssetDatabase.addDeclaredAsset(%moduleDef, %assetPath @ "/" @ %assetName @ ".asset.taml");
         else
            AssetDatabase.refreshAsset(%assetId);
      }
      
      if(ImportAssetTree.isParentItem(%id))
      {
         %childItem = ImportAssetTree.getChild(%id);
         
         //recurse!
         %this.doImportAssets(%childItem); 
      }

      %id = ImportAssetTree.getNextSibling(%id);
   }
}

function ImportAssetWindow::Close(%this)
{
   //Some cleanup
   AssetBrowser.importingFilesArray.clear();
   
   Canvas.popDialog();  
}
//
function ImportAssetWindow::validateAsset(%this, %assetItem)
{
   %assetCount = AssetBrowser.importAssetFinalListArray.count();
   %moduleName = ImportAssetModuleList.getText();
   
   %hasIssues = false;
   
   //First, check the obvious: name collisions. We should have no asset that shares a similar name.
   //If we do, prompt for it be renamed first before continuing
   
   for(%i=0; %i < %assetCount; %i++)
   {
      %assetItemA = AssetBrowser.importAssetFinalListArray.getKey(%i);
      
      if( (%assetItemA.assetName $= %assetItem.assetName) && (%assetItemA.getId() != %assetItem.getId()) )
      {
         //yup, a collision, prompt for the change and bail out
         /*MessageBoxOK( "Error!", "Duplicate asset names found with importing assets!\nAsset \"" @ 
            %assetItemA.assetName @ "\" of type \"" @ %assetItemA.assetType @ "\" and \"" @
            %assetItemB.assetName @ "\" of type \"" @ %assetItemB.assetType @ "\" have matching names.\nPlease rename one of them and try again!");*/
            
         %assetItem.status = "Warning";
         %assetItem.statusType = "DuplicateImportAsset";
         %assetItem.statusInfo = "Duplicate asset names found with importing assets!\nAsset \"" @ 
            %assetItemA.assetName @ "\" of type \"" @ %assetItemA.assetType @ "\" and \"" @
            %assetItem.assetName @ "\" of type \"" @ %assetItem.assetType @ "\" have matching names.\nPlease rename one of them and try again!";
            
         %hasIssues = true;
         return false;
      }
   }

   //No collisions of for this name in the importing assets. Now, check against the existing assets in the target module
   if(!AssetBrowser.isAssetReImport)
   {
      %assetQuery = new AssetQuery();
      
      %numAssetsFound = AssetDatabase.findAllAssets(%assetQuery);

      %foundCollision = false;
      for( %f=0; %f < %numAssetsFound; %f++)
      {
         %assetId = %assetQuery.getAsset(%f);
          
         //first, get the asset's module, as our major categories
         %module = AssetDatabase.getAssetModule(%assetId);
         
         %testModuleName = %module.moduleId;
         
         //These are core, native-level components, so we're not going to be messing with this module at all, skip it
         if(%moduleName !$= %testModuleName)
            continue;

         %testAssetName = AssetDatabase.getAssetName(%assetId);
         
         if(%testAssetName $= %assetItem.assetName)
         {
            %foundCollision = true;
            
            %assetItem.status = "Warning";
            %assetItem.statusType = "DuplicateAsset";
            %assetItem.statusInfo = "Duplicate asset names found with the target module!\nAsset \"" @ 
               %assetItem.assetName @ "\" of type \"" @ %assetItem.assetType @ "\" has a matching name.\nPlease rename it and try again!";
               
            //Clean up our queries
            %assetQuery.delete();
      
            return false;            
         }
      }
      
      if(%foundCollision == true)
      {
         %hasIssues = true;
         
         //yup, a collision, prompt for the change and bail out
         /*MessageBoxOK( "Error!", "Duplicate asset names found with the target module!\nAsset \"" @ 
            %assetItemA.assetName @ "\" of type \"" @ %assetItemA.assetType @ "\" has a matching name.\nPlease rename it and try again!");*/
            
         //%assetQuery.delete();
         //return false;
      }
      
      //Clean up our queries
      %assetQuery.delete();
   }
      
   //Check if we were given a file path(so not generated) but somehow isn't a valid file
   if(%assetItem.filePath !$= "" && !isFile(%assetItem.filePath))
   {
      %hasIssues = true;  
      %assetItem.status = "error";
      %assetItem.statusType = "MissingFile";
      %assetItem.statusInfo = "Unable to find file to be imported. Please select asset file.";
      
      return false;
   }
   
   return true;
}

function ImportAssetWindow::resolveIssue(%this, %assetItem)
{
   if(%assetItem.status !$= "Warning")
      return;
      
   //Ok, we actually have a warning, so lets resolve
   if(%assetItem.statusType $= "DuplicateImportAsset" || %assetItem.statusType $= "DuplicateAsset")
   {
      
   }
   else if(%assetItem.statusType $= "MissingFile")
   {
      %this.findMissingFile(%assetItem);
   }
}
//

//
function ImportAssetModuleList::onWake(%this)
{
   %this.refresh();
}

function ImportAssetModuleList::refresh(%this)
{
   %this.clear();
   
   //First, get our list of modules
   %moduleList = ModuleDatabase.findModules();
   
   %count = getWordCount(%moduleList);
   for(%i=0; %i < %count; %i++)
   {
      %moduleName = getWord(%moduleList, %i);
      %this.add(%moduleName.ModuleId, %i);  
   }
}
//
