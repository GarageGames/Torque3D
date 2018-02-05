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
if( !isObject( ToolsGuiDefaultNonModalProfile ) )
new GuiControlProfile (ToolsGuiDefaultNonModalProfile : ToolsGuiDefaultProfile)
{
   modal = false;
};

function initializeAssetBrowser()
{
   echo(" % - Initializing Asset Browser");  
   
   exec("./guis/assetBrowser.gui");
   exec("./guis/addModuleWindow.gui");
   exec("./guis/gameObjectCreator.gui");
   exec("./guis/newAsset.gui");
   exec("./guis/newComponentAsset.gui");
   exec("./guis/editAsset.gui");
   exec("./guis/assetImport.gui");
   exec("./guis/selectModule.gui");
   exec("./guis/editModule.gui");

   exec("./scripts/assetBrowser.cs");
   exec("./scripts/popupMenus.cs");
   exec("./scripts/addModuleWindow.cs");
   exec("./scripts/assetImport.cs");
   exec("./scripts/assetImportConfig.cs");
   exec("./scripts/gameObjectCreator.cs");
   exec("./scripts/newAsset.cs");
   exec("./scripts/editAsset.cs");
   exec("./scripts/editModule.cs");   
   
   exec("./scripts/fieldTypes.cs");
   
   new ScriptObject( AssetBrowserPlugin )
   {
      superClass = "EditorPlugin";
   };
   
   Input::GetEventManager().subscribe( AssetBrowser, "BeginDropFiles" );
   Input::GetEventManager().subscribe( AssetBrowser, "DropFile" );
   Input::GetEventManager().subscribe( AssetBrowser, "EndDropFiles" );
   
   AssetBrowser.buildPopupMenus();
}

function AssetBrowserPlugin::onWorldEditorStartup( %this )
{ 
   // Add ourselves to the toolbar.
   AssetBrowser.addToolbarButton();
}