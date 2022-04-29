function AssetBrowser_SelectModule::onWake(%this)
{
   AssetBrowser_SelectModuleWindow-->ModuleList.refresh();
}

function AssetBrowser_SelectModule::moduleSelected(%this)
{
   Canvas.popDialog(AssetBrowser_SelectModule);
   
   %module = AssetBrowser_SelectModuleWindow-->ModuleList.getText();
   echo("Module Selected: " @ %module);
   
   if(%this.callback !$= "")
      eval(%this.callback @ "(" @ %module @ ");");
   else
      error("AssetBrowser_SelectModule - Invalid callback");
}

function SelectModule_NewAssetModuleBtn::onClick(%this)
{
   Canvas.pushDialog(AssetBrowser_AddModule);
   AssetBrowser_addModuleWindow.selectWindow();
   
   AssetBrowser_AddModule.callback = "AssetBrowser_selectModule.newModuleAdded();";
}

function AssetBrowser_selectModule::newModuleAdded(%this)
{
   AssetBrowser_SelectModuleWindow-->ModuleList.refresh();     
}