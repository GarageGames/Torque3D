//
function AssetBrowser::CreateNewModule(%this)
{
   Canvas.pushDialog(AssetBrowser_AddModule); 
   AssetBrowser_addModuleWindow.selectWindow();  
   
   AssetBrowser_addModuleWindow.callbackFunction = "AssetBrowser.loadFilters();";
}

function AssetBrowser_editModule::saveModule(%this)
{
   //Check what special actions we may need to do, such as renames
   %moduleDef = ModuleDatabase.findModule(AssetBrowser.selectedModule, 1);
   
   %oldModuleName = %moduleDef.ModuleID;
   
   if(%oldModuleName !$= AssetBrowser.tempModule.ModuleID)
   {      
      //rename the script file and script namespaces
      %oldScriptFilePath = "data/" @ %oldModuleName @ "/" @ %moduleDef.scriptFile;
      %newscriptFilePath = "data/" @ AssetBrowser.tempModule.ModuleID @ "/";
      %scriptExt = fileExt(%moduleDef.scriptFile);
      
      %newScriptFileName = %newscriptFilePath @ "/" @ AssetBrowser.tempModule.ModuleID @ %scriptExt;
      %newScriptFileOldName = %newscriptFilePath @ "/" @ %oldModuleName @ %scriptExt;
      
      %moduleDef.ModuleId = AssetBrowser.tempModule.ModuleID;
      %moduleDef.scriptFile = AssetBrowser.tempModule.ModuleID @ %scriptExt;
      
      ModuleDatabase.copyModule(%moduleDef, AssetBrowser.tempModule.ModuleID, "data/" @ AssetBrowser.tempModule.ModuleID);
      
      //Go through our scriptfile and replace the old namespace with the new
      %editedFileContents = "";
      
      %file = new FileObject();
      if ( %file.openForRead( %newScriptFileOldName ) ) 
      {
         while ( !%file.isEOF() ) 
         {
            %line = %file.readLine();
            %line = trim( %line );
            
            %editedFileContents = %editedFileContents @ strreplace(%line, %oldModuleName, AssetBrowser.tempModule.ModuleID) @ "\n";
         }
         
         %file.close();
      }
      
      if(%editedFileContents !$= "")
      {
         %file.openForWrite(%newScriptFileName);
         
         %file.writeline(%editedFileContents);
         
         %file.close();
      }
      
      %success = fileDelete(%newScriptFileOldName);
      
      ModuleDatabase.unloadExplicit(%oldModuleName);
      
      %success = fileDelete("data/" @ %oldModuleName);
      
      ModuleDatabase.loadExplicit(AssetBrowser.tempModule.ModuleID);
   }
   
   //Now, update the module file itself
   //%file = ModuleDatabase.getAssetFilePath(%moduleDef.ModuleID);
   //%success = TamlWrite(AssetBrowser_editAsset.editedAsset, %file);
   
   AssetBrowser.loadFilters();

   Canvas.popDialog(AssetBrowser_editModule);
}

function AssetBrowser::editModuleInfo(%this)
{
   Canvas.pushDialog(AssetBrowser_editModule); 
   
   %moduleDef = ModuleDatabase.findModule(AssetBrowser.selectedModule, 1);
   
   AssetBrowser.tempModule = new ModuleDefinition();
   AssetBrowser.tempModule.assignFieldsFrom(%moduleDef);
   
   ModuleEditInspector.inspect(AssetBrowser.tempModule);  
   AssetBrowser_editModule.editedModuleId = AssetBrowser.selectedModule;
   AssetBrowser_editModule.editedModule = AssetBrowser.tempModule;
   
   //remove some of the groups we don't need:
   for(%i=0; %i < ModuleEditInspector.getCount(); %i++)
   {
      %caption = ModuleEditInspector.getObject(%i).caption;
      
      if(%caption $= "BuildId" || %caption $= "type" || %caption $= "Dependencies" || %caption $= "scriptFile" 
         || %caption $= "AssetTagsManifest" || %caption $= "ScopeSet" || %caption $= "ModulePath" 
         || %caption $= "ModuleFile" || %caption $= "ModuleFilePath" || %caption $= "ModuleScriptFilePath"  )
      {
         ModuleEditInspector.remove(ModuleEditInspector.getObject(%i));
         %i--;
      }
   }
}

function AssetBrowser::renameModule(%this)
{
   
}

function AssetBrowser::reloadModule(%this)
{
   ModuleDatabase.unregisterModule(AssetBrowser.SelectedModule, 1);
   ModuleDatabase.loadExplicit(AssetBrowser.SelectedModule);
}

function AssetBrowser::deleteModule(%this)
{
   
}

function AssetBrowser::RefreshModuleDependencies(%this)
{
   //Iterate through all our modules
   
   //then, iterate through the module's assets
   
   //if an asset has a module that isn't us, queue that into the dependencies list  
}