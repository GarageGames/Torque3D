function AssetBrowser_editAsset::saveAsset(%this)
{
   %file = AssetDatabase.getAssetFilePath(%this.editedAssetId);
   %success = TamlWrite(AssetBrowser_editAsset.editedAsset, %file);
   
   AssetBrowser.loadFilters();

   Canvas.popDialog(AssetBrowser_editAsset);
}

function AssetBrowser::editAsset(%this, %assetDef)
{
   //Find out what type it is
   //If the passed-in definition param is blank, then we're likely called via a popup
   if(%assetDef $= "")
      %assetDef = AssetDatabase.acquireAsset(EditAssetPopup.assetId);
      
   %assetType = %assetDef.getClassName();
   
   //Build out the edit command
   %buildCommand = %this @ ".edit" @ %assetType @ "(" @ %assetDef @ ");";
   eval(%buildCommand);
}

function AssetBrowser::appendSubLevel(%this)
{
   %assetDef = AssetDatabase.acquireAsset(EditAssetPopup.assetId);
   %assetType = %assetDef.getClassName();
      
   schedule( 1, 0, "EditorOpenSceneAppend", %assetDef);
}

function AssetBrowser::editAssetInfo(%this)
{
   Canvas.pushDialog(AssetBrowser_editAsset); 
   
   %assetDef = AssetDatabase.acquireAsset(EditAssetPopup.assetId);
   
   eval("AssetBrowser.tempAsset = new " @ %assetDef.getClassName() @ "();");
   AssetBrowser.tempAsset.assignFieldsFrom(%assetDef);
   
   AssetEditInspector.inspect(AssetBrowser.tempAsset);  
   AssetBrowser_editAsset.editedAssetId = EditAssetPopup.assetId;
   AssetBrowser_editAsset.editedAsset = AssetBrowser.tempAsset;
   
   //remove some of the groups we don't need:
   for(%i=0; %i < AssetEditInspector.getCount(); %i++)
   {
      %caption = AssetEditInspector.getObject(%i).caption;
      
      if(%caption $= "Ungrouped" || %caption $= "Object" || %caption $= "Editing" 
         || %caption $= "Persistence" || %caption $= "Dynamic Fields")
      {
         AssetEditInspector.remove(AssetEditInspector.getObject(%i));
         %i--;
      }
   }
}

//------------------------------------------------------------

function AssetBrowser::refreshAsset(%this, %assetId)
{
   if(%assetId $= "")
   {
      //if we have no passed-in asset ID, we're probably going through the popup menu, so get our edit popup id  
      %assetId = EditAssetPopup.assetId;
   }
   
   AssetDatabase.refreshAsset(%assetId);
   AssetBrowser.refreshPreviews();
}

//------------------------------------------------------------

function AssetBrowser::renameAsset(%this)
{
   //Find out what type it is
   %assetDef = AssetDatabase.acquireAsset(EditAssetPopup.assetId);
   
   %curFirstResponder = AssetBrowser.getFirstResponder();
   
   if(%curFirstResponder != 0)
      %curFirstResponder.clearFirstResponder();
   
   AssetBrowser.selectedAssetPreview-->AssetNameLabel.setActive(true);
   AssetBrowser.selectedAssetPreview-->AssetNameLabel.setFirstResponder();
}

function AssetBrowser::performRenameAsset(%this, %originalAssetName, %newName)
{
   //if the name is different to the asset's original name, rename it!
   if(%originalAssetName !$= %newName)
   {
      %moduleName = AssetBrowser.selectedModule;
      
      //do a rename!
      %success = AssetDatabase.renameDeclaredAsset(%moduleName @ ":" @ %originalAssetName, %moduleName @ ":" @ %newName);
      
      if(%success)
         echo("AssetBrowser - renaming of asset " @ %moduleName @ ":" @ %originalAssetName @ " to " @ %moduleName @ ":" @ %newName @ " was a success.");
      else 
         echo("AssetBrowser - renaming of asset " @ %moduleName @ ":" @ %originalAssetName @ " to " @ %moduleName @ ":" @ %newName @ " was a failure.");
      
      if(%success)
      {
         %newAssetId = %moduleName @ ":" @ %newName;
         %assetPath = AssetDatabase.getAssetFilePath(%newAssetId);
         
         //Rename any associated files as well
         %assetDef = AssetDatabase.acquireAsset(%newAssetId);
         %assetType = %assetDef.getClassName();
         
         //rename the file to match
         %path = filePath(%assetPath);
         
         //Do the rename command
         %buildCommand = %this @ ".rename" @ %assetType @ "(" @ %assetDef @ "," @ %newAssetId @ ");";
         eval(%buildCommand);
      }
   }
   
   //Make sure everything is refreshed
   AssetBrowser.loadFilters();
}

function AssetNameField::onReturn(%this)
{
   %this.clearFirstResponder();
   %this.setActive(false);
   
   AssetBrowser.performRenameAsset(%this.originalAssetName, %this.getText());
}

//------------------------------------------------------------

function AssetBrowser::duplicateAsset(%this, %targetModule)
{
   if(%targetModule $= "")
   {
      //we need a module to duplicate to first
      Canvas.pushDialog(AssetBrowser_selectModule);
      AssetBrowser_selectModule.callback = "AssetBrowser.duplicateAsset";
      return;
   }
   
   %assetDef = AssetDatabase.acquireAsset(EditAssetPopup.assetId);
   %assetType = AssetDatabase.getAssetType(EditAssetPopup.assetId);
   
   //this acts as a redirect based on asset type and will enact the appropriate function
   //so for a GameObjectAsset, it'll become %this.duplicateGameObjectAsset(%assetDef, %targetModule);
   //and call to the tools/assetBrowser/scripts/assetTypes/gameObject.cs file for implementation
   if(%this.isMethod("duplicate"@%assetType))
      eval(%this @ ".duplicate"@%assetType@"("@%assetDef@","@%targetModule@");");
}

function AssetBrowser::deleteAsset(%this)
{
   //Find out what type it is
   %assetDef = AssetDatabase.acquireAsset(EditAssetPopup.assetId);
   %assetType = %assetDef.getClassName();
   
   MessageBoxOKCancel("Warning!", "This will delete the selected asset and the files associated to it, do you wish to continue?", 
      "AssetBrowser.confirmDeleteAsset();", "");
}

function AssetBrowser::confirmDeleteAsset(%this)
{
   %currentSelectedItem = AssetBrowserFilterTree.getSelectedItem();
   %currentItemParent = AssetBrowserFilterTree.getParentItem(%currentSelectedItem);
   
   %assetDef = AssetDatabase.acquireAsset(EditAssetPopup.assetId);
   %assetType = AssetDatabase.getAssetType(EditAssetPopup.assetId);
   
   //Do any cleanup required given the type
   if(%this.isMethod("delete"@%assetType))
      eval(%this @ ".delete"@%assetType@"("@%assetDef@");");
   
   AssetDatabase.deleteAsset(EditAssetPopup.assetId, false);

   %this.loadFilters();
   
   if(!AssetBrowserFilterTree.selectItem(%currentSelectedItem))
   {
      //if it failed, that means we deleted the last item in that category, and we need to do the parent  
      AssetBrowserFilterTree.selectItem(%currentItemParent);
      AssetBrowserFilterTree.expandItem(%currentItemParent);
   }
}
