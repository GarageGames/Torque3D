function GuiInspectorComponentGroup::buildMaterialField(%this, %component, %fieldName)
{
   %extent = 200;

   %currentMaterial = %component.getFieldValue(%fieldName);

   //if we don't have a new material set on this slot, just use the default
   if(%currentMaterial $= "" || %currentMaterial == 0)
	  %currentMaterial = %material;
      
   %container = new GuiControl() {
      canSaveDynamicFields = "0";
      Profile = "EditorContainerProfile";
      HorizSizing = "right";
      VertSizing = "bottom";
      Position = "0 0";
      Extent = "300 110";
      MinExtent = "8 2";
      canSave = "0";
      Visible = "1";
      hovertime = "100";
      tooltip = %tooltip;
      tooltipProfile = "EditorToolTipProfile";
   };

   %labelControl = new GuiTextCtrl() {
      canSaveDynamicFields = "0";
      Profile = "EditorFontHLBold";
      HorizSizing = "right";
      VertSizing = "bottom";
      Position = "16 3";
      Extent = "100 18";
      MinExtent = "8 2";
      canSave = "0";
      Visible = "1";
      hovertime = "100";
      tooltip = %tooltip;
      tooltipProfile = "EditorToolTipProfile";
      text = %fieldName;
      maxLength = "1024";
   };

   //
   %imageContainer = new GuiControl(){
      profile = "ToolsGuiDefaultProfile";
      Position = "20 25";
      Extent = "74 87";
      HorizSizing = "right";
      VertSizing = "bottom";
      isContainer = "1";
      
      new GuiTextCtrl(){
         position = "7 71";
         profile = "ToolsGuiTextCenterProfile";
         extent = "64 16";
         text = %matName;
      };
   };
   
   %previewButton = new GuiBitmapButtonCtrl(){
      internalName = %matName;
      HorizSizing = "right";
      VertSizing = "bottom";
      profile = "ToolsGuiButtonProfile";
      position = "7 4";
      extent = "64 64";
      buttonType = "PushButton";
      bitmap = "";
      Command = "";
      text = "Loading...";
      useStates = false;
	  tooltip = "Change material";
      
      new GuiBitmapButtonCtrl(){
            HorizSizing = "right";
            VertSizing = "bottom";
            profile = "ToolsGuiButtonProfile";
            position = "0 0";
            extent = "64 64";
            Variable = "";
            buttonType = "toggleButton";
            bitmap = "tools/materialEditor/gui/cubemapBtnBorder";
            groupNum = "0";
            text = "";
			tooltip = "Change material";
         }; 
   }; 
   
   %previewBorder = new GuiButtonCtrl(){
	     className = "materialFieldBtn";
         internalName = %matName@"Border";
         HorizSizing = "right";
         VertSizing = "bottom";
         profile = "ToolsGuiThumbHighlightButtonProfile";
         position = "3 0";
         extent = "72 88";
         Variable = "";
         buttonType = "toggleButton";
         tooltip = %matName;
         groupNum = "0";
         text = "";
		 Object = %component;
		 targetField = %fieldName;
   };
   %previewBorder.Command = %previewBorder @ ".getMaterialName();"; 
   
   %imageContainer.add(%previewButton);  
   %imageContainer.add(%previewBorder);
   %container.add(%imageContainer);	
   //
   
   %mapToLabel = new GuiTextCtrl() {
      canSaveDynamicFields = "0";
      Profile = "EditorFontHLBold";
      HorizSizing = "right";
      VertSizing = "bottom";
      Position = "100 26";
      Extent = %extent SPC "18";
      MinExtent = "8 2";
      canSave = "0";
      Visible = "1";
      hovertime = "100";
      tooltip = %tooltip;
      tooltipProfile = "EditorToolTipProfile";
      text = "Mapped to:" SPC %material.mapTo;
      maxLength = "1024";
   };

   %editControl = new GuiTextEditCtrl() {
      class = "BehaviorEdTextField";
      internalName = %accessor @ "File";
      canSaveDynamicFields = "0";
      Profile = "EditorTextEdit";
      HorizSizing = "right";
      VertSizing = "bottom";
      Position = "100 50";
      Extent = %extent SPC "22";
      MinExtent = "8 2";
      canSave = "0";
      Visible = "1";
      hovertime = "100";
      tooltip = %tooltip;
      tooltipProfile = "EditorToolTipProfile";
      maxLength = "1024";
      historySize = "0";
      password = "0";
      text = %currentMaterial;
      
      tabComplete = "0";
      sinkAllKeyEvents = "0";
      password = "0";
      passwordMask = "*";
      precision = %precision;
      accessor = %accessor;
      isProperty = true;
      undoLabel = %fieldName;
      object = %this.object;
      useWords = false;
   };
   
   %resetButton = new GuiButtonCtrl() {
      canSaveDynamicFields = "0";
      className = "materialFieldBtn";
      Profile = "GuiButtonProfile";
      HorizSizing = "right";
      VertSizing = "bottom";
      Position = "100 75";
      Extent = (%extent * 0.3)-3 SPC "22";
      MinExtent = "8 2";
      canSave = "0";
      Visible = "1";
      hovertime = "100";
      tooltip = "Reset to default material";
      tooltipProfile = "EditorToolTipProfile";
      text = "Reset";
      pathField = %editControl;
   };

   %editMatButton = new GuiButtonCtrl() {
      canSaveDynamicFields = "0";
      className = "materialFieldBtn";
      Profile = "GuiButtonProfile";
      HorizSizing = "right";
      VertSizing = "bottom";
      Position = %resetButton.position.x + (%extent * 0.3) + 6 SPC "75";
      Extent = (%extent * 0.6)-3 SPC "22";
      MinExtent = "8 2";
      canSave = "0";
      Visible = "1";
      hovertime = "100";
      tooltip = "Edit in Material Editor";
      tooltipProfile = "EditorToolTipProfile";
      text = "Open in Editor";
      pathField = %editControl;
   };
   
   %container.add(%mapToLabel);
   %container.add(%labelControl);
   %container.add(%editControl);
   %container.add(%resetButton);
   %container.add(%editMatButton);

   //load the material
   %matName = "";

   // CustomMaterials are not available for selection
   /*if ( !isObject( %currentMaterial ) || %currentMaterial.isMemberOfClass( "CustomMaterial" ) )
      return;*/
   %assetDef = AssetDatabase.acquireAsset(%currentMaterial);
   
   %materialDef = %assetDef.materialDefinitionName;

   if( %materialDef.isMemberOfClass("TerrainMaterial") )
   {
      %matName = %currentMaterial.getInternalName();
      
      if( %materialDef.diffuseMap $= "")
         %previewImage = "core/art/warnmat";
      else
         %previewImage = %materialDef.diffuseMap;
   }
   else if( %materialDef.toneMap[0] $= "" && %materialDef.diffuseMap[0] $= "" && !isObject(%materialDef.cubemap) )
   {
      %matName = %materialDef.name;
      %previewImage = "core/art/warnmat";
   }
   else
   {
      %matName = %materialDef.name;
      
      if( %materialDef.toneMap[0] !$= "" )
         %previewImage = %materialDef.toneMap[0];
      else if( %materialDef.diffuseMap[0] !$= "" )
         %previewImage = %materialDef.diffuseMap[0];
      else if( %materialDef.cubemap.cubeFace[0] !$= "" )
         %previewImage = %materialDef.cubemap.cubeFace[0];
      
      //%previewImage = MaterialEditorGui.searchForTexture( %material,  %previewImage );
      
      // were going to use a couple of string commands in order to properly
      // find out what the img src path is 
      // **NEW** this needs to be updated with the above, but has some timing issues
      %materialDiffuse =  %previewImage;
      %materialPath = %materialDef.getFilename();
      
      if( strchr( %materialDiffuse, "/") $= "" )
      {
         %k = 0;
         while( strpos( %materialPath, "/", %k ) != -1 )
         {
            %foo = strpos( %materialPath, "/", %k );
            %k = %foo + 1;
         }
      
         %foobar = getSubStr( %materialPath , %k , 99 );
         %previewImage =  strreplace( %materialPath, %foobar, %previewImage );
      }
      else
         %previewImage =  strreplace( %materialPath, %materialPath, %previewImage );
   }

   %previewButton.setBitmap(%previewImage);
   %previewButton.setText("");
   
   %this.stack.add(%container);
}

function materialFieldBtn::onClick(%this)
{
   AssetBrowser.showDialog("MaterialAsset", "", %this.Object, %this.targetField, "");  
}

function materialFieldBtn::setMaterial(%this, %matAssetName)
{
   
}