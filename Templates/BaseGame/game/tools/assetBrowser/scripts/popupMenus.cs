function AssetBrowser::buildPopupMenus(%this)
{
   if( !isObject( AddNewModulePopup ) )
   {
      new PopupMenu( AddNewModulePopup )
      {
         superClass = "MenuBuilder";
         class = "EditorWorldMenu";
         isPopup = true;
         
         item[ 0 ] = "Create New Module" TAB "" TAB "AssetBrowser.CreateNewModule();";
         item[ 1 ] = "Refresh Module Dependencies" TAB "" TAB "AssetBrowser.RefreshModuleDependencies();";
      };
   }
      
   if( !isObject( EditAssetPopup ) )
   {
      new PopupMenu( EditAssetPopup )
      {
         superClass = "MenuBuilder";
         class = "EditorWorldMenu";
         //isPopup = true;

         item[ 0 ] = "Edit Asset" TAB "" TAB "AssetBrowser.editAsset();";
         item[ 1 ] = "Rename Asset" TAB "" TAB "AssetBrowser.renameAsset();";
         item[ 2 ] = "Refresh Asset" TAB "" TAB "AssetBrowser.refreshAsset();";
         item[ 3 ] = "Asset Properties" TAB "" TAB "AssetBrowser.editAssetInfo();";
         item[ 4 ] = "-";
         Item[ 5 ] = "Duplicate Asset" TAB "" TAB "AssetBrowser.duplicateAsset();";
         item[ 6 ] = "-";
         item[ 7 ] = "Re-Import Asset" TAB "" TAB "AssetBrowser.reImportAsset();";
         item[ 8 ] = "-";
         item[ 9 ] = "Delete Asset" TAB "" TAB "AssetBrowser.deleteAsset();";

         jumpFileName = "";
         jumpLineNumber = "";
      };
   }
   
   if( !isObject( EditLevelAssetPopup ) )
   {
      new PopupMenu( EditLevelAssetPopup )
      {
         superClass = "MenuBuilder";
         class = "EditorWorldMenu";
         //isPopup = true;

         item[ 0 ] = "Edit Level" TAB "" TAB "AssetBrowser.editAsset();";
         item[ 1 ] = "Append as Sublevel" TAB "" TAB "AssetBrowser.appendSublevel();";
         item[ 2 ] = "Rename Asset" TAB "" TAB "AssetBrowser.renameAsset();";
         item[ 3 ] = "Refresh Asset" TAB "" TAB "AssetBrowser.refreshAsset();";
         item[ 4 ] = "Asset Properties" TAB "" TAB "AssetBrowser.editAssetInfo();";
         item[ 5 ] = "-";
         Item[ 6 ] = "Duplicate Asset" TAB "" TAB "AssetBrowser.duplicateAsset();";
         item[ 7 ] = "-";
         item[ 8 ] = "Re-Import Asset" TAB "" TAB "AssetBrowser.reImportAsset();";
         item[ 9 ] = "-";
         item[ 10 ] = "Delete Asset" TAB "" TAB "AssetBrowser.deleteAsset();";

         jumpFileName = "";
         jumpLineNumber = "";
      };
   }
   
   if( !isObject( AddNewComponentAssetPopup ) )
   {
      new PopupMenu( AddNewComponentAssetPopup )
      {
         superClass = "MenuBuilder";
         class = "EditorWorldMenu";
         //isPopup = true;

         //item[ 0 ] = "Create Component" TAB "" TAB "Canvas.pushDialog(AssetBrowser_newComponentAsset); AssetBrowser_newComponentAsset-->NewComponentPackageList.setText(AssetBrowser.selectedModule);";
         item[ 0 ] = "Component" TAB "" TAB "AssetBrowser.setupCreateNewAsset(\"ComponentAsset\", AssetBrowser.selectedModule);";
         
         //list other common component types here to shortcut the creation process
      };
   }
   
   if( !isObject( AddNewScriptAssetPopup ) )
   {
      %this.AddNewScriptAssetPopup = new PopupMenu( AddNewScriptAssetPopup )
      {
         superClass = "MenuBuilder";
         class = "EditorWorldMenu";
         //isPopup = true;

         item[ 0 ] = "Create Component" TAB AddNewComponentAssetPopup;
         item[ 1 ] = "Create Script" TAB "" TAB "AssetBrowser.setupCreateNewAsset(\"ScriptAsset\", AssetBrowser.selectedModule);";
         item[ 2 ] = "Create State Machine" TAB "" TAB "AssetBrowser.setupCreateNewAsset(\"StateMachineAsset\", AssetBrowser.selectedModule);";
         //item[ 3 ] = "-";
         //item[ 3 ] = "Create Game Object" TAB "" TAB "AssetBrowser.createNewGameObjectAsset(\"NewGameObject\", AssetBrowser.selectedModule);";
      };
      //%this.AddNewScriptAssetPopup.insertSubMenu(0, "Create Component", AddNewComponentAssetPopup);
   }
   
   if( !isObject( AddNewArtAssetPopup ) )
   {
      %this.AddNewArtAssetPopup = new PopupMenu( AddNewArtAssetPopup )
      {
         superClass = "MenuBuilder";
         class = "EditorWorldMenu";
         //isPopup = true;

         item[ 0 ] = "Create Material" TAB "" TAB "AssetBrowser.setupCreateNewAsset(\"MaterialAsset\", AssetBrowser.selectedModule);";//"createNewMaterialAsset(\"NewMaterial\", AssetBrowser.selectedModule);";
         item[ 1 ] = "Create Image" TAB "" TAB "AssetBrowser.setupCreateNewAsset(\"ImageAsset\", AssetBrowser.selectedModule);";//"AssetBrowser.createNewImageAsset(\"NewImage\", AssetBrowser.selectedModule);";
         item[ 2 ] = "-";         
         item[ 3 ] = "Create Shape" TAB "" TAB "AssetBrowser.setupCreateNewAsset(\"Shape\", AssetBrowser.selectedModule);";
         item[ 4 ] = "Create Shape Animation" TAB "" TAB "AssetBrowser.setupCreateNewAsset(\"ShapeAnimationAsset\", AssetBrowser.selectedModule);";//"AssetBrowser.createNewShapeAnimationAsset(\"NewShapeAnimation\", AssetBrowser.selectedModule);";
         item[ 5 ] = "-";
         item[ 6 ] = "Create GUI" TAB "" TAB "AssetBrowser.setupCreateNewAsset(\"GUIAsset\", AssetBrowser.selectedModule);";//"AssetBrowser.createNewGUIAsset(\"NewGUI\", AssetBrowser.selectedModule);";
         item[ 7 ] = "-";
         item[ 8 ] = "Create Post Effect" TAB "" TAB "AssetBrowser.setupCreateNewAsset(\"PostEffectAsset\", AssetBrowser.selectedModule);";//"AssetBrowser.createNewPostEffectAsset(\"NewPostEffect\", AssetBrowser.selectedModule);";
         item[ 9 ] = "-";
         item[ 10 ] = "Create Sound" TAB "" TAB "AssetBrowser.setupCreateNewAsset(\"SoundAsset\", AssetBrowser.selectedModule);";//"AssetBrowser.createNewSoundAsset(\"NewSound\", AssetBrowser.selectedModule);";
         item[ 11 ] = "-";
         item[ 12 ] = "Create Particle Effect" TAB "" TAB "AssetBrowser.setupCreateNewAsset(\"ParticleEffectAsset\", AssetBrowser.selectedModule);";//"AssetBrowser.createNewParticleEffectAsset(\"NewParticleEffect\", AssetBrowser.selectedModule);";
      };
   }
   
   if( !isObject( AddNewCppAssetPopup ) )
   {
      %this.AddNewCppAssetPopup = new PopupMenu( AddNewCppAssetPopup )
      {
         superClass = "MenuBuilder";
         class = "EditorWorldMenu";
         //isPopup = true;

         /*item[ 0 ] = "Create Static Class" TAB "" TAB "AssetBrowser.setupCreateNewAsset(\"CppStaticClassAsset\", AssetBrowser.selectedModule);";
         item[ 1 ] = "Create Regular Class" TAB "" TAB "AssetBrowser.setupCreateNewAsset(\"CppRegularClassAsset\", AssetBrowser.selectedModule);";
         item[ 2 ] = "Create GameObject Class" TAB "" TAB "AssetBrowser.setupCreateNewAsset(\"CppGameObjectAsset\", AssetBrowser.selectedModule);";
         item[ 3 ] = "Create Component Class" TAB "" TAB "AssetBrowser.setupCreateNewAsset(\"CppComponentAsset\", AssetBrowser.selectedModule);";
         item[ 4 ] = "Create Script Class" TAB "" TAB "AssetBrowser.setupCreateNewAsset(\"CppScriptClass\", AssetBrowser.selectedModule);";*/
         
         item[ 0 ] = "Create C++ Class" TAB "" TAB "AssetBrowser.setupCreateNewAsset(\"CppAsset\", AssetBrowser.selectedModule);";
      };
      //%this.AddNewScriptAssetPopup.insertSubMenu(0, "Create Component", AddNewComponentAssetPopup);
   }
      
   if( !isObject( AddNewAssetPopup ) )
   {
      %this.AddNewAssetPopup = new PopupMenu( AddNewAssetPopup )
      {
         superClass = "MenuBuilder";
         class = "EditorWorldMenu";
         
         item[0] = "Create Code Asset" TAB AddNewScriptAssetPopup;
         item[1] = "-";
         item[2] = "Create Art Asset" TAB AddNewArtAssetPopup;
         item[3] = "-";
         item[4] = "Create Level" TAB "" TAB "AssetBrowser.setupCreateNewAsset(\"LevelAsset\", AssetBrowser.selectedModule);";//"AssetBrowser.createNewLevelAsset(\"NewLevel\", AssetBrowser.selectedModule);";
         item[5] = "-";
         item[6] = "Create C++ Asset" TAB AddNewCppAssetPopup;
      
      };
   }
   
   if( !isObject( EditModulePopup ) )
   {
      new PopupMenu( EditModulePopup )
      {
         superClass = "MenuBuilder";
         class = "EditorWorldMenu";
         //isPopup = true;
         
         item[ 0 ] = "Create New Asset" TAB AddNewAssetPopup;
         item[ 1 ] = "Reload Module" TAB "" TAB "AssetBrowser.reloadModule();";
         Item[ 2 ] = "-";
         Item[ 3 ] = "Edit Module" TAB "" TAB "AssetBrowser.editModuleInfo();";
         Item[ 4 ] = "-";
         Item[ 5 ] = "Duplicate Module" TAB "" TAB "AssetBrowser.copyModule();";
         Item[ 6 ] = "-";
         Item[ 7 ] = "Delete Module" TAB "" TAB "AssetBrowser.deleteModule();";
      };
   }
   
   //Some assets are not yet ready/implemented, so disable their creation here
   AddNewArtAssetPopup.enableItem(3, false); //shape
   AddNewArtAssetPopup.enableItem(4, false); //shape animation
   AddNewArtAssetPopup.enableItem(10, false); //sound asset
   AddNewArtAssetPopup.enableItem(12, false); //particle effect
   
   if( !isObject( EditAssetCategoryPopup ) )
   {
      new PopupMenu( EditAssetCategoryPopup )
      {
         superClass = "MenuBuilder";
         class = "EditorWorldMenu";
         //isPopup = true;
         
         item[ 0 ] = "Toggle Autoloading of Script Assets" TAB "" TAB "AssetBrowser.toggleAutoloadAsset(\"Script\");";
      };
   }
   
   //Browser visibility menu
   if( !isObject( BrowserVisibilityPopup ) )
   {
      new PopupMenu( BrowserVisibilityPopup )
      {
         superClass = "MenuBuilder";
         class = "EditorWorldMenu";
         //isPopup = true;
         
         item[ 0 ] = "Toggle Show Core Modules" TAB "" TAB "AssetBrowser.viewCoreModulesFilter();";
         item[ 1 ] = "Toggle Only Show Modules with Assets" TAB "" TAB "AssetBrowser.viewPopulatedModulesFilter();";
         Item[ 2 ] = "-";
         item[ 3 ] = "Show Assets as list" TAB "" TAB "AssetBrowser.viewListFilter();";
         Item[ 4 ] = "Show Assets with tags" TAB "" TAB "AssetBrowser.viewTagsFilter();";
      };
   }
   
   //Import Legacy menus
   if( !isObject( ImportAssetsPopup ) )
   {
      new PopupMenu( ImportAssetsPopup )
      {
         superClass = "MenuBuilder";
         class = "EditorWorldMenu";
         
         item[ 0 ] = "Import Legacy Game" TAB "" TAB "AssetBrowser.importLegacyGame();";
         Item[ 1 ] = "-";
         item[ 2 ] = "Import new assets" TAB "" TAB "AssetBrowser.importNewAssetFile();";
      };
   }
   
   if( !isObject( EditGameObjectAssetPopup ) )
   {
      new PopupMenu( EditGameObjectAssetPopup )
      {
         superClass = "MenuBuilder";
         class = "EditorWorldMenu";
         //isPopup = true;
         
         item[ 0 ] = "Open GameObject Editor" TAB "" TAB "echo(\"Not yet implemented.\");";
         item[ 1 ] = "Edit GameObject Script" TAB "" TAB "AssetBrowser.editGameObjectAssetScript(AssetDatabase.acquireAsset(EditGameObjectAssetPopup.assetId));";
         item[ 2 ] = "-";
         item[ 3 ] = "Apply Instance to GameObject" TAB "" TAB "AssetBrowser.applyInstanceToGameObject(AssetDatabase.acquireAsset(EditGameObjectAssetPopup.assetId));";
         item[ 4 ] = "Reset Instance to GameObject" TAB "" TAB "echo(\"Not yet implemented.\");";
         item[ 5 ] = "-";
         item[ 6 ] = "Create Child GameObject" TAB "" TAB "echo(\"Not yet implemented.\");";
      };
   }
   
   //Asset Import Resolution menus
   if( !isObject( ImportAssetResolutionsPopup ) )
   {
      %this.ImportAssetResolutionsPopup = new PopupMenu( ImportAssetResolutionsPopup )
      {
         superClass = "MenuBuilder";
         class = "EditorWorldMenu";
         
         item[0] = "Use original Asset for duplicates" TAB "" TAB "";
         item[1] = "Override duplicate with new Asset" TAB "" TAB "";
         item[2] = "-";
         item[3] = "Rename Asset" TAB "" TAB "";
         item[4] = "-";
         item[5] = "Find missing file" TAB "" TAB "ImportAssetWindow.findMissingFile(ImportAssetResolutionsPopup.assetItem);";
         item[6] = "-";
         item[7] = "Edit Asset properties" TAB "" TAB "";
      
      };
   }
}

function AddNewScriptAssetPopupMenu::onSelectItem(%this, %id, %text)
{   
   return true;
}
function AddNewScriptAssetPopupMenu::setupDefaultState(%this)
{
   // Setup camera speed gui's. Both menu and editorgui
   %this.setupGuiControls();
   
   Parent::setupDefaultState(%this);
}

function AddNewScriptAssetPopupMenu::setupGuiControls(%this)
{
}
