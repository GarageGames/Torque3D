function AssetBrowser::createShapeAnimationAsset(%this)
{
   %dlg = new OpenFileDialog()
   {
      Filters        = "Animation Files(*.dae, *.cached.dts)|*.dae;*.cached.dts";
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
   }   
   
   %dlg.delete();
   
   if ( !%ret )
      return;  
}

function AssetBrowser::editShapeAnimationAsset(%this, %assetDef)
{
   %this.hideDialog();
   ShapeEditorPlugin.openShapeAsset(%assetDef);   
}

function AssetBrowser::buildShapeAnimationAssetPreview(%this, %assetDef, %previewData)
{
   %previewData.assetName = %assetDef.animationName;
   %previewData.assetPath = %assetDef.scriptFile;

   //Lotta prepwork
   /*%previewData.doubleClickCommand = %assetDef@".materialDefinitionName.reload(); "
                                   @ "$Tools::materialEditorList = \"\";"
                                   @ "EWorldEditor.clearSelection();"
                                   @ "MaterialEditorGui.currentObject = 0;"
                                   @ "MaterialEditorGui.currentMode = \"asset\";"
                                   @ "MaterialEditorGui.currentMaterial = "@%assetDef@".materialDefinitionName;"
                                   @ "MaterialEditorGui.setActiveMaterial( "@%assetDef@".materialDefinitionName );"
                                   @ "EditorGui.setEditor(MaterialEditorPlugin); "
                                   @ "AssetBrowser.hideDialog();";*/
   
   %previewData.previewImage = "tools/assetBrowser/art/animationIcon";
   
   %previewData.assetFriendlyName = %assetDef.assetName;
   %previewData.assetDesc = %assetDef.description;
   %previewData.tooltip = %assetDef.friendlyName @ "\n" @ %assetDef;
}