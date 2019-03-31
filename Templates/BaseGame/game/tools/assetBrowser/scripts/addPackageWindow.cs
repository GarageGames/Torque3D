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
function AssetBrowser_addPackageWindow::onGainFirstResponder(%this)
{
   %this-->packageName.setFirstResponder();
}

function AssetBrowser_addPackageWindow::close()
{
   Canvas.popDialog(AssetBrowser_addPackage);  
   eval(AssetBrowser_addPackageWindow.callbackFunction);
}

function AssetBrowser_addPackageWindow::CreateNewPackage(%this)
{
   %newPackageName = %this-->packageName.getText();
   
   if(%newPackageName $= "")
      return;
      
   echo("Creating a new package named: " @ %newPackageName);
   
   %moduleFilePath = "data/" @ %newPackageName;
   %moduleDefinitionFilePath = %moduleFilePath @ "/" @ %newPackageName @ ".module";
   %moduleScriptFilePath = %moduleFilePath @ "/" @ %newPackageName @ ".cs";
   
   %newPackage = new ModuleDefinition()
   {
      ModuleId = %newPackageName;
      versionId = 1;
      ScriptFile = %newPackageName @ ".cs";
      CreateFunction="onCreate";
	   DestroyFunction="onDestroy";
	   Group = "Game";
      
      new DeclaredAssets()
      {
         Extension = "asset.taml";
         Recurse = true;
      };
   };
   
   TAMLWrite(%newPackage, %moduleDefinitionFilePath); 
   
   //Now generate the script file for it
   %file = new FileObject();
	
	if(%file.openForWrite(%moduleScriptFilePath))
	{
		%file.writeline("function " @ %newPackageName @ "::onCreate(%this)\n{\n\n}\n");
		%file.writeline("function " @ %newPackageName @ "::onDestroy(%this)\n{\n\n}\n");
		
		//todo, pre-write any event functions of interest
		
		%file.close();
	}
   
   //force a refresh of our modules list
   ModuleDatabase.ignoreLoadedGroups(true);
   ModuleDatabase.scanModules();
   %success = ModuleDatabase.loadExplicit(%newPackageName, 1);
   ModuleDatabase.ignoreLoadedGroups(false);
   
   //force a reload of the package lists
   NewAssetPackageList.refresh();
   GameObjectPackageList.refresh();
   ImportAssetPackageList.refresh();
   
   Canvas.popDialog(AssetBrowser_addPackage);
   eval(AssetBrowser_addPackageWindow.callbackFunction);
}

function AssetBrowserPackageList::onWake(%this)
{
   %this.refresh();
}

function AssetBrowserPackageList::refresh(%this)
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