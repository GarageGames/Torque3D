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
function AssetBrowser_addModuleWindow::onGainFirstResponder(%this)
{
   %this-->moduleName.setFirstResponder();
}

function AssetBrowser_addModuleWindow::close()
{
   Canvas.popDialog(AssetBrowser_addModule);  
   eval(AssetBrowser_addModuleWindow.callbackFunction);
}

function AssetBrowser_addModuleWindow::CreateNewModule(%this)
{
   %newModuleName = %this-->moduleName.getText();
   
   if(%newModuleName $= "")
      return;
      
   echo("Creating a new Module named: " @ %newModuleName);
   
   %moduleFilePath = "data/" @ %newModuleName;
   %moduleDefinitionFilePath = %moduleFilePath @ "/" @ %newModuleName @ ".module";
   %moduleScriptFilePath = %moduleFilePath @ "/" @ %newModuleName @ ".cs";
   
   %newModule = new ModuleDefinition()
   {
      ModuleId = %newModuleName;
      versionId = 1;
      ScriptFile = %newModuleName @ ".cs";
      CreateFunction="onCreate";
	   DestroyFunction="onDestroy";
	   Group = "Game";
      
      new DeclaredAssets()
      {
         Extension = "asset.taml";
         Recurse = true;
      };
      
      //Autoload the usual suspects
      new AutoloadAssets()
      {
         AssetType = "ComponentAsset";
         Recurse = true;
      };
      new AutoloadAssets()
      {
         AssetType = "GUIAsset";
         Recurse = true;
      };
   };
   
   TAMLWrite(%newModule, %moduleDefinitionFilePath); 
   
   //Now generate the script file for it
   %file = new FileObject();
	
	if(%file.openForWrite(%moduleScriptFilePath))
	{
		%file.writeline("function " @ %newModuleName @ "::onCreate(%this)\n{\n\n}\n");
		%file.writeline("function " @ %newModuleName @ "::onDestroy(%this)\n{\n\n}\n");
		
		//todo, pre-write any event functions of interest
		
		%file.close();
	}
   
   //force a refresh of our modules list
   ModuleDatabase.ignoreLoadedGroups(true);
   ModuleDatabase.scanModules();
   %success = ModuleDatabase.loadExplicit(%newModuleName, 1);
   ModuleDatabase.ignoreLoadedGroups(false);
   
   //force a reload of the Module lists
   NewAssetModuleList.refresh();
   GameObjectModuleList.refresh();
   ImportAssetModuleList.refresh();
   
   AssetBrowser.newModuleId = %newModuleName;
   
   Canvas.popDialog(AssetBrowser_addModule);
   eval(AssetBrowser_addModuleWindow.callbackFunction);
}

function AssetBrowserModuleList::onWake(%this)
{
   %this.refresh();
}

function AssetBrowserModuleList::refresh(%this)
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