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
function CreateAssetButton::onClick(%this)
{
   AddNewAssetPopup.showPopup(Canvas);
}

function AssetBrowser_newAsset::onWake(%this)
{
   NewAssetModuleList.refresh();
   //NewComponentParentClass.setText("Component");
}

function AssetBrowser_newAssetWindow::onClose(%this)
{
   NewAssetPropertiesInspector.clearFields(); 
   Canvas.popDialog(AssetBrowser_newAsset);
}

function NewAssetTypeList::onWake(%this)
{
   %this.refresh();
}

function NewAssetTypeList::refresh(%this)
{
   %this.clear();
   
   //TODO: make this more automated
   //%this.add("GameObject", 0);
   %this.add("Component", 0);
   %this.add("Image", 1);
   %this.add("Material", 2);
   %this.add("Shape", 3);  
   %this.add("Sound", 4);
   %this.add("State Machine", 5);
}

function NewAssetTypeList::onSelected(%this)
{
   %assetType = %this.getText();
   
   if(%assetType $= "Component")
   {
      NewComponentAssetSettings.hidden = false;
   }
}

function NewAssetModuleBtn::onClick(%this)
{
   Canvas.pushDialog(AssetBrowser_AddModule);
   AssetBrowser_addModuleWindow.selectWindow();
}

function AssetBrowser::setupCreateNewAsset(%this, %assetType, %moduleName)
{
   Canvas.pushDialog(AssetBrowser_newAsset);
   
   AssetBrowser_newAssetWindow.text = "New" SPC %assetType SPC "Asset";
   
   NewAssetPropertiesInspector.clear();
   
   NewAssetModuleList.setText(%moduleName);
   
   //get rid of the old one if we had one.
   if(isObject(%this.newAssetSettings))
      %this.newAssetSettings.delete();
   
   %this.newAssetSettings = new ScriptObject();
   
   %this.newAssetSettings.assetType = %assetType;
   %this.newAssetSettings.moduleName = %moduleName;
   
   %shortAssetTypeName = strreplace(%assetType, "Asset", "");
   
   NewAssetPropertiesInspector.startGroup("General");
   NewAssetPropertiesInspector.addField("assetName", "New Asset Name", "String",  "Name of the new asset", "New" @ %shortAssetTypeName, "", %this.newAssetSettings);
   //NewAssetPropertiesInspector.addField("AssetType", "New Asset Type", "List",  "Type of the new asset", %assetType, "Component,Image,Material,Shape,Sound,State Machine", %newAssetSettings);
   
   NewAssetPropertiesInspector.addField("friendlyName", "Friendly Name", "String",  "Human-readable name of new asset", "", "", %this.newAssetSettings);
      
   NewAssetPropertiesInspector.addField("description", "Description", "Command",  "Description of the new asset", "", "", %this.newAssetSettings);   
   NewAssetPropertiesInspector.endGroup();
   
   if(%assetType $= "ComponentAsset")
   {
      NewAssetPropertiesInspector.startGroup("Components");
      NewAssetPropertiesInspector.addField("parentClass", "New Asset Parent Class", "String",  "Name of the new asset's parent class", "Component", "", %this.newAssetSettings);
      NewAssetPropertiesInspector.addField("componentGroup", "Component Group", "String",  "Name of the group of components this component asset belongs to", "", "", %this.newAssetSettings);
      //NewAssetPropertiesInspector.addField("componentName", "Component Name", "String",  "Name of the new component", "", "", %this.newAssetSettings);
      NewAssetPropertiesInspector.endGroup();
   }
   else if(%assetType $= "LevelAsset")
   {
      NewAssetPropertiesInspector.startGroup("Level");
      NewAssetPropertiesInspector.addField("levelPreviewImage", "LevePreviewImage", "Image",  "Preview Image for the level", "", "", %this.newAssetSettings);
      NewAssetPropertiesInspector.endGroup();
   }
   else if(%assetType $= "ScriptAsset")
   {
      NewAssetPropertiesInspector.startGroup("Script");
      NewAssetPropertiesInspector.addField("isServerScript", "Is Server Script", "bool",  "Is this script used on the server?", "1", "", %this.newAssetSettings);
      NewAssetPropertiesInspector.endGroup();
   }
   /*else if(%assetType $= "ShapeAnimationAsset")
   {
      NewAssetPropertiesInspector.startGroup("Animation");
      NewAssetPropertiesInspector.addField("sourceFile", "Source File", "filename",  "Source file this animation will pull from", "", "", %this.newAssetSettings);
      NewAssetPropertiesInspector.addField("animationName", "Animation Name", "string",  "Name of the animation clip when used in a shape", "", "", %this.newAssetSettings);
      
      NewAssetPropertiesInspector.addField("startFrame", "Starting Frame", "int",  "Source file this animation will pull from", "", "", %this.newAssetSettings);
      NewAssetPropertiesInspector.addField("endFrame", "Ending Frame", "int",  "Source file this animation will pull from", "", "", %this.newAssetSettings);
      
      NewAssetPropertiesInspector.addField("padRotation", "Pad Rotations", "bool",  "Source file this animation will pull from", "0", "", %this.newAssetSettings);
      NewAssetPropertiesInspector.addField("padTransforms", "Pad Transforms", "bool",  "Source file this animation will pull from", "0", "", %this.newAssetSettings);
      NewAssetPropertiesInspector.endGroup();
   }*/
   
   return;
   
   if(%moduleName $= "")
   {
      Canvas.pushDialog(AssetBrowser_selectModule);
   }
   else
   {
      AssetBrowser.SelectedModule = %moduleName;
      
      if(%assetType $= "MaterialAsset")
      {
         createNewMaterialAsset("NewMaterial", %moduleName);
      }
      else if(%assetType $= "StateMachineAsset")
      {
         createNewStateMachineAsset("NewStateMachine", %moduleName);
      }
      else if(%assetType $= "ScriptAsset")
      {
         createNewScriptAsset("NewScriptAsset", %moduleName);
      }
   }
}

//We do a quick validation that mandatory fields are filled in before passing along to the asset-type specific function
function CreateNewAsset()
{
   %assetName = AssetBrowser.newAssetSettings.assetName;
   
   if(%assetName $= "")
	{
	   MessageBoxOK( "Error", "Attempted to make a new asset with no name!");
		return;
	}
	
	//get the selected module data
   %moduleName = NewAssetModuleList.getText();
   
   if(%moduleName $= "")
	{
	   MessageBoxOK( "Error", "Attempted to make a new asset with no module!");
		return;
	}
	
	AssetBrowser.newAssetSettings.moduleName = %moduleName;
	
   %assetType = AssetBrowser.newAssetSettings.assetType;
	if(%assetType $= "")
	{
	   MessageBoxOK( "Error", "Attempted to make a new asset with no type!");
		return;
	}
	
	if(%assetType $= "ComponentAsset")
	{
	   //Canvas.popDialog(AssetBrowser_newComponentAsset); 
	   //AssetBrowser_newComponentAsset-->AssetBrowserModuleList.setText(AssetBrowser.selectedModule);
	   %assetFilePath = createNewComponentAsset(%assetName, %path);
	}
	else if(%assetType $= "MaterialAsset")
	{
	   %assetFilePath = createNewMaterialAsset();
	}
	else if(%assetType $= "StateMachineAsset")
	{
	   %assetFilePath = createNewStateMachineAsset();
	}
	else if(%assetType $= "GUIAsset")
	{
	   %assetFilePath = createNewGUIAsset();
	}
	else if(%assetType $= "LevelAsset")
	{
	   %assetFilePath = createNewLevelAsset();
	}
	else if(%assetType $= "ScriptAsset")
	{
	   %assetFilePath = createNewScriptAsset();
	}
	else if(%assetType $= "ShapeAnimationAsset")
	{
	   %assetFilePath = createShapeAnimationAsset();
	}
	
	Canvas.popDialog(AssetBrowser_newAsset);
	
	//Load it
	%moduleDef = ModuleDatabase.findModule(%moduleName,1);
	AssetDatabase.addDeclaredAsset(%moduleDef, %assetFilePath);
	
	AssetBrowser.loadFilters();
}

function createNewComponentAsset()
{
   %moduleName = AssetBrowser.newAssetSettings.moduleName;
   %modulePath = "data/" @ %moduleName;
      
   %assetName = AssetBrowser.newAssetSettings.assetName;
   
   %tamlpath = %modulePath @ "/components/" @ %assetName @ ".asset.taml";
   %scriptPath = %modulePath @ "/components/" @ %assetName @ ".cs";
   
   %asset = new ComponentAsset()
   {
      AssetName = %assetName;
      versionId = 1;
      componentName = %assetName;
      componentClass = AssetBrowser.newAssetSettings.parentClass;
      friendlyName = AssetBrowser.newAssetSettings.friendlyName;
      componentType = AssetBrowser.newAssetSettings.componentGroup;
      description = AssetBrowser.newAssetSettings.description;
      scriptFile = %scriptPath;
   };
   
   TamlWrite(%asset, %tamlpath);
   
   %file = new FileObject();
	
	if(%file.openForWrite(%scriptPath))
	{
		//TODO: enable ability to auto-embed a header for copyright or whatnot
	   %file.writeline("//onAdd is called when the component is created and then added to it's owner entity.\n");
	   %file.writeline("//You would also add any script-defined component fields via addComponentField().\n");
		%file.writeline("function " @ %assetName @ "::onAdd(%this)\n{\n\n}\n");
		%file.writeline("//onAdd is called when the component is removed and deleted from it's owner entity.");
		%file.writeline("function " @ %assetName @ "::onRemove(%this)\n{\n\n}\n");
		%file.writeline("//onClientConnect is called any time a new client connects to the server.");
		%file.writeline("function " @ %assetName @ "::onClientConnect(%this, %client)\n{\n\n}\n");
		%file.writeline("//onClientDisconnect is called any time a client disconnects from the server.");
		%file.writeline("function " @ %assetName @ "::onClientDisonnect(%this, %client)\n{\n\n}\n");
		%file.writeline("//update is called when the component does an update tick.\n");
		%file.writeline("function " @ %assetName @ "::Update(%this)\n{\n\n}\n");
		
		%file.close();
	}
	
	Canvas.popDialog(AssetBrowser_newComponentAsset);
	
	%moduleDef = ModuleDatabase.findModule(%moduleName, 1);
	AssetDatabase.addDeclaredAsset(%moduleDef, %tamlpath);

	AssetBrowser.loadFilters();
	
	%treeItemId = AssetBrowserFilterTree.findItemByName(%moduleName);
	%smItem = AssetBrowserFilterTree.findChildItemByName(%treeItemId, "Components");
	
	AssetBrowserFilterTree.onSelect(%smItem);
	
	return %tamlpath;
}

function createNewMaterialAsset()
{
   %assetName = AssetBrowser.newAssetSettings.assetName;
   
   %moduleName = AssetBrowser.newAssetSettings.moduleName;
   %modulePath = "data/" @ %moduleName;
   
   %tamlpath = %modulePath @ "/materials/" @ %assetName @ ".asset.taml";
   %sgfPath = %modulePath @ "/materials/" @ %assetName @ ".sgf";
   
   %asset = new MaterialAsset()
   {
      AssetName = %assetName;
      versionId = 1;
      shaderData = "";
      shaderGraph = %sgfPath;
   };
   
   TamlWrite(%asset, %tamlpath);
   
   %moduleDef = ModuleDatabase.findModule(%moduleName, 1);
	AssetDatabase.addDeclaredAsset(%moduleDef, %tamlpath);

	AssetBrowser.loadFilters();
	
	%treeItemId = AssetBrowserFilterTree.findItemByName(%moduleName);
	%smItem = AssetBrowserFilterTree.findChildItemByName(%treeItemId, "Materials");
	
	AssetBrowserFilterTree.onSelect(%smItem);
   
	return %tamlpath;
}

function createNewScriptAsset()
{
   %moduleName = AssetBrowser.newAssetSettings.moduleName;
   %modulePath = "data/" @ %moduleName;
      
   %assetName = AssetBrowser.newAssetSettings.assetName;      
   
   %tamlpath = %modulePath @ "/scripts/" @ %assetName @ ".asset.taml";
   %scriptPath = %modulePath @ "/scripts/" @ %assetName @ ".cs";
   
   %asset = new ScriptAsset()
   {
      AssetName = %assetName;
      versionId = 1;
      scriptFilePath = %scriptPath;
   };
   
   TamlWrite(%asset, %tamlpath);
   
   %moduleDef = ModuleDatabase.findModule(%moduleName, 1);
	AssetDatabase.addDeclaredAsset(%moduleDef, %tamlpath);

	AssetBrowser.loadFilters();
	
	%treeItemId = AssetBrowserFilterTree.findItemByName(%moduleName);
	%smItem = AssetBrowserFilterTree.findChildItemByName(%treeItemId, "Scripts");
	
	AssetBrowserFilterTree.onSelect(%smItem);
	
	%file = new FileObject();
   
   if(%file.openForWrite(%scriptPath))
	{
		%file.close();
	}
   
	return %tamlpath;
}

function createNewStateMachineAsset()
{
   %assetName = AssetBrowser.newAssetSettings.assetName;
      
   %assetQuery = new AssetQuery();
   
   %matchingAssetCount = AssetDatabase.findAssetName(%assetQuery, %assetName);
   
   %i=1;
   while(%matchingAssetCount > 0)
   {
      %newAssetName = %assetName @ %i;
      %i++;
      
      %matchingAssetCount = AssetDatabase.findAssetName(%assetQuery, %newAssetName);
   }
   
   %assetName = %newAssetName;
   
   %assetQuery.delete();
   
   %tamlpath = "data/" @ %moduleName @ "/stateMachines/" @ %assetName @ ".asset.taml";
   %smFilePath = "data/" @ %moduleName @ "/stateMachines/" @ %assetName @ ".xml";
   
   %asset = new StateMachineAsset()
   {
      AssetName = %assetName;
      versionId = 1;
      stateMachineFile = %smFilePath;
   };
   
   %xmlDoc = new SimXMLDocument();
   %xmlDoc.saveFile(%smFilePath);
   %xmlDoc.delete();
   
   TamlWrite(%asset, %tamlpath);
   
   //Now write our XML file
   %xmlFile = new FileObject();
	%xmlFile.openForWrite(%smFilePath);
	%xmlFile.writeLine("<StateMachine>");
	%xmlFile.writeLine("</StateMachine>");
	%xmlFile.close();
   
   %moduleDef = ModuleDatabase.findModule(%moduleName, 1);
	AssetDatabase.addDeclaredAsset(%moduleDef, %tamlpath);

	AssetBrowser.loadFilters();
	
	%treeItemId = AssetBrowserFilterTree.findItemByName(%moduleName);
	%smItem = AssetBrowserFilterTree.findChildItemByName(%treeItemId, "StateMachines");
	
	AssetBrowserFilterTree.onSelect(%smItem);
   
	return %tamlpath;
}

function createNewGUIAsset()
{
   %moduleName = AssetBrowser.newAssetSettings.moduleName;
   %modulePath = "data/" @ %moduleName;
      
   %assetName = AssetBrowser.newAssetSettings.assetName;
   
   %tamlpath = %modulePath @ "/GUIs/" @ %assetName @ ".asset.taml";
   %guipath = %modulePath @ "/GUIs/" @ %assetName @ ".gui";
   %scriptPath = %modulePath @ "/GUIs/" @ %assetName @ ".cs";
   
   %asset = new GUIAsset()
   {
      AssetName = %assetName;
      versionId = 1;
      scriptFilePath = %scriptPath;
      guiFilePath = %guipath;
   };
   
   TamlWrite(%asset, %tamlpath);
   
   %file = new FileObject();
   
   if(%file.openForWrite(%guipath))
	{
	   %file.writeline("//--- OBJECT WRITE BEGIN ---");
		%file.writeline("%guiContent = new GuiControl(" @ %assetName @ ") {");
		%file.writeline("   position = \"0 0\";");
		%file.writeline("   extent = \"100 100\";");
		%file.writeline("};");
		%file.writeline("//--- OBJECT WRITE END ---");
		
		%file.close();
	}
	
	if(%file.openForWrite(%scriptPath))
	{
		%file.writeline("function " @ %assetName @ "::onWake(%this)\n{\n\n}\n");
		%file.writeline("function " @ %assetName @ "::onSleep(%this)\n{\n\n}\n");
		
		%file.close();
	}
	
	//load the gui
	exec(%guipath);
	exec(%scriptPath);
	
	%moduleDef = ModuleDatabase.findModule(%moduleName, 1);
	AssetDatabase.addDeclaredAsset(%moduleDef, %tamlpath);

	AssetBrowser.loadFilters();
	
	%treeItemId = AssetBrowserFilterTree.findItemByName(%moduleName);
	%smItem = AssetBrowserFilterTree.findChildItemByName(%treeItemId, "GUIs");
	
	AssetBrowserFilterTree.onSelect(%smItem);
	
	return %tamlpath;
}

function createNewLevelAsset()
{
   %moduleName = AssetBrowser.newAssetSettings.moduleName;
   %modulePath = "data/" @ %moduleName;
   
   %assetName = AssetBrowser.newAssetSettings.assetName;
   
   %tamlpath = %modulePath @ "/levels/" @ %assetName @ ".asset.taml";
   %levelPath = %modulePath @ "/levels/" @ %assetName @ ".mis";
   
   %asset = new LevelAsset()
   {
      AssetName = %assetName;
      versionId = 1;
      LevelFile = %levelPath;
      LevelDescription = AssetBrowser.newAssetSettings.levelDescription;
      PreviewImage = AssetBrowser.newAssetSettings.levelPreviewImage;
   };
   
   TamlWrite(%asset, %tamlpath);
   
   if(!pathCopy("tools/levels/BlankRoom.mis", %levelPath, false))
   {
      echo("Unable to copy template level file!");
   }

	%moduleDef = ModuleDatabase.findModule(%moduleName, 1);
	AssetDatabase.addDeclaredAsset(%moduleDef, %tamlpath);

	AssetBrowser.loadFilters();
	
	%treeItemId = AssetBrowserFilterTree.findItemByName(%moduleName);
	%smItem = AssetBrowserFilterTree.findChildItemByName(%treeItemId, "Levels");
	
	AssetBrowserFilterTree.onSelect(%smItem);
	
	return %tamlpath;
}

function createNewShapeAnimationAsset()
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
      
   /*%moduleName = AssetBrowser.newAssetSettings.moduleName;
   %modulePath = "data/" @ %moduleName;
   
   %assetName = AssetBrowser.newAssetSettings.assetName;
   
   %tamlpath = %modulePath @ "/levels/" @ %assetName @ ".asset.taml";
   %levelPath = %modulePath @ "/levels/" @ %assetName @ ".mis";
   
   %asset = new ShapeAnimationAsset()
   {
      AssetName = %assetName;
      versionId = 1;
      LevelFile = %levelPath;
      LevelDescription = AssetBrowser.newAssetSettings.levelDescription;
      PreviewImage = AssetBrowser.newAssetSettings.levelPreviewImage;
   };
   
   TamlWrite(%asset, %tamlpath);
   
   if(!pathCopy("tools/levels/BlankRoom.mis", %levelPath, false))
   {
      echo("Unable to copy template level file!");
   }

	%moduleDef = ModuleDatabase.findModule(%moduleName, 1);
	AssetDatabase.addDeclaredAsset(%moduleDef, %tamlpath);

	AssetBrowser.loadFilters();
	
	%treeItemId = AssetBrowserFilterTree.findItemByName(%moduleName);
	%smItem = AssetBrowserFilterTree.findChildItemByName(%treeItemId, "Levels");
	
	AssetBrowserFilterTree.onSelect(%smItem);
	
	return %tamlpath;*/
}

function ParentComponentList::onWake(%this)
{
   %this.refresh();
}

function ParentComponentList::refresh(%this)
{
   %this.clear();
   
   %assetQuery = new AssetQuery();
   if(!AssetDatabase.findAssetType(%assetQuery, "ComponentAsset"))
      return; //if we didn't find ANY, just exit
   
   // Find all the types.
   %count = %assetQuery.getCount();

   /*%categories = "";
   for (%i = 0; %i < %count; %i++)
   {
      %assetId = %assetQuery.getAsset(%i);
      
      %componentAsset = AssetDatabase.acquireAsset(%assetId);
      %componentName = %componentAsset.componentName;
      
      if(%componentName $= "")
         %componentName = %componentAsset.componentClass;
      
      %this.add(%componentName, %i);
   }*/
   
   %categories = "";
   for (%i = 0; %i < %count; %i++)
   {
      %assetId = %assetQuery.getAsset(%i);
      
      %componentAsset = AssetDatabase.acquireAsset(%assetId);
      %componentClass = %componentAsset.componentClass;
      if (!isInList(%componentClass, %categories))
         %categories = %categories TAB %componentClass;
   }
   
   %categories = trim(%categories);
   
   %index = 0;
   %categoryCount = getFieldCount(%categories);
   for (%i = 0; %i < %categoryCount; %i++)
   {
      %category = getField(%categories, %i);
      %this.addCategory(%category);
      
      for (%j = 0; %j < %count; %j++)
      {
         %assetId = %assetQuery.getAsset(%j);
      
         %componentAsset = AssetDatabase.acquireAsset(%assetId);
         %componentName = %componentAsset.componentName;
         %componentClass = %componentAsset.componentClass;
      
         if (%componentClass $= %category)
         {
            if(%componentName !$= "")
               %this.add("   "@%componentName, %i);
         }
      }
   }
}

//----------------------------------------------------------
// Game Object creation
//----------------------------------------------------------
function EWorldEditor::createGameObject( %this )
{
   GameObjectCreatorObjectName.text = "";
   
   %activeSelection = %this.getActiveSelection();
   if( %activeSelection.getCount() == 0 )
      return;
      
   GameObjectCreator.selectedEntity = %activeSelection.getObject( 0 );

   Canvas.pushDialog(GameObjectCreator);
}