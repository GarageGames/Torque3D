function AssetBrowser_selectPackage::onWake(%this)
{
   AssetBrowser_SelectPackageWindow-->packageList.refresh();
}

function SelectPackage_NewAssetPackageBtn::onClick(%this)
{
   Canvas.pushDialog(AssetBrowser_AddPackage);
   AssetBrowser_addPackageWindow.selectWindow();
   
   AssetBrowser_AddPackage.callback = "AssetBrowser_selectPackage.newPackageAdded();";
}

function AssetBrowser_selectPackage::newPackageAdded(%this)
{
   AssetBrowser_SelectPackageWindow-->packageList.refresh();     
}