function AssetBrowser_selectModule::onWake(%this)
{
   AssetBrowser_SelectModuleWindow-->ModuleList.refresh();
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