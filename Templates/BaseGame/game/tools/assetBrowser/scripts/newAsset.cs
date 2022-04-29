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

function AssetBrowser::setupCreateNewAsset(%this, %assetType, %moduleName, %callback)
{
   Canvas.pushDialog(AssetBrowser_newAsset);
   
   AssetBrowser_newAssetWindow.text = "New" SPC %assetType SPC "Asset";
   
   NewAssetPropertiesInspector.clearFields();
   
   NewAssetModuleList.setText(%moduleName);
   
   AssetBrowser_newAsset.callbackFunc = %callback;
   
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
   
   //NewAssetPropertiesInspector.addField("friendlyName", "Friendly Name", "String",  "Human-readable name of new asset", "", "", %this.newAssetSettings);
      
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
      NewAssetPropertiesInspector.addField("levelName", "Level Name", "String",  "Human-readable name of new level", "", "", %this.newAssetSettings);
      NewAssetPropertiesInspector.addField("levelPreviewImage", "Level Preview Image", "Image",  "Preview Image for the level", "", "", %this.newAssetSettings);
      NewAssetPropertiesInspector.endGroup();
   }
   else if(%assetType $= "ScriptAsset")
   {
      NewAssetPropertiesInspector.startGroup("Script");
      NewAssetPropertiesInspector.addField("isServerScript", "Is Server Script", "bool",  "Is this script used on the server?", "1", "", %this.newAssetSettings);
      NewAssetPropertiesInspector.endGroup();
   }
   //Special case, we only do this via internal means like baking
   /*else if(%assetType $= "ShapeAsset")
   {
      NewAssetPropertiesInspector.startGroup("Shape");
      NewAssetPropertiesInspector.addField("isServerScript", "Is Server Script", "bool",  "Is this script used on the server?", "1", "", %this.newAssetSettings);
      NewAssetPropertiesInspector.endGroup();
   }*/
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
	
	%assetFilePath = eval(AssetBrowser @ ".create"@%assetType@"();");
	
	Canvas.popDialog(AssetBrowser_newAsset);
	
	//Load it
	%moduleDef = ModuleDatabase.findModule(%moduleName,1);
	AssetDatabase.addDeclaredAsset(%moduleDef, %assetFilePath);
	
	AssetBrowser.loadFilters();
	
	if(AssetBrowser_newAsset.callbackFunc !$= "")
	{
      %callbackCommand = "" @ AssetBrowser_newAsset.callbackFunc @ "(\"" @ %moduleName @ ":" @ %assetName @ "\");";
      eval(%callbackCommand);
	}
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
   AssetBrowser.createGameObjectAsset();
}