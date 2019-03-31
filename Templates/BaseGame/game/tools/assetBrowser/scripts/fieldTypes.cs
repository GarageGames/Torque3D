function GuiVariableInspector::onInspectorFieldModified(%this, %targetObj, %fieldName, %index, %oldValue, %newValue)
{
   echo("FIELD CHANGED: " @ %fieldName @ " from " @ %oldValue @ " to " @ %newValue);
}

function GuiInspectorVariableGroup::onConstructField(%this, %fieldName, %fieldLabel, %fieldTypeName, %fieldDesc, %fieldDefaultVal, %fieldDataVals, %ownerObj)
{
   %makeCommand = %this @ ".build" @ %fieldTypeName @ "Field(\""@ %fieldName @ "\",\"" @ %fieldLabel @ "\",\"" @ %fieldDesc @ "\",\"" @ 
            %fieldDefaultVal @ "\",\"" @ %fieldDataVals @ "\",\"" @ %ownerObj @"\");";
   eval(%makeCommand);
}

function GuiInspectorVariableGroup::buildListField(%this, %fieldName, %fieldLabel, %fieldDesc, %fieldDefaultVal, %fieldDataVals, %ownerObj)
{
   %extent = 200;
   
   %fieldCtrl = %this.createInspectorField();
   
   %extent = %this.stack.getExtent();
   
   %width = mRound(%extent/2);
   %height = 20;
   %inset = 10;
      
   /*%container = new GuiControl() {
      canSaveDynamicFields = "0";
      Profile = "EditorContainerProfile";
      HorizSizing = "right";
      VertSizing = "bottom";
      Position = "0 0";
      Extent = %extent.x SPC %height;
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
      Position = %inset SPC "0";
      Extent = %width + %inset SPC %height;
      MinExtent = "8 2";
      canSave = "0";
      Visible = "1";
      hovertime = "100";
      tooltip = %tooltip;
      tooltipProfile = "EditorToolTipProfile";
      text = %fieldLabel;
      maxLength = "1024";
   };*/
   
   %editControl = new GuiPopUpMenuCtrl() {
      class = "guiInspectorListField";
      maxPopupHeight = "200";
      sbUsesNAColor = "0";
      reverseTextList = "0";
      bitmapBounds = "16 16";
      maxLength = "1024";
      Margin = "0 0 0 0";
      Padding = "0 0 0 0";
      AnchorTop = "1";
      AnchorBottom = "0";
      AnchorLeft = "1";
      AnchorRight = "0";
      isContainer = "0";
      Profile = "ToolsGuiPopUpMenuProfile";
      HorizSizing = "right";
      VertSizing = "bottom";
      Position = %fieldCtrl.edit.position;
      Extent = %fieldCtrl.edit.extent;
      MinExtent = "8 2";
      canSave = "1";
      Visible = "1";
      tooltipprofile = "ToolsGuiToolTipProfile";
      tooltip = %tooltip;
      text = %fieldDefaultVal;
      hovertime = "1000";
      ownerObject = %ownerObj;
      fieldName = %fieldName;
   };
   
   //set the field value
   if(getSubStr(%this.fieldName, 0, 1) $= "$")
   {
      if(%fieldName $= "")
         %editControl.setText(%fieldName);
   }
   else
   {
      //regular variable
      %setCommand = %editControl @ ".setText(" @ %ownerObj @ "." @ %fieldName @ ");";
      eval(%setCommand);
   }
   
   %listCount = getTokenCount(%fieldDataVals, ",");
   
   for(%i=0; %i < %listCount; %i++)
   {
      %entryText = getToken(%fieldDataVals, ",", %i);
      %editControl.add(%entryText); 
   }

   %fieldCtrl.setCaption(%fieldLabel);
   %fieldCtrl.setEditControl(%editControl);

   %this.addInspectorField(%fieldCtrl);
}

function guiInspectorListField::onSelect( %this, %id, %text )
{   
   if(getSubStr(%this.fieldName, 0, 1) $= "$")
   {
      //ah, a global var, just do it straight, then
      %setCommand = %this.fieldName @ " = \"" @ %text @ "\";";
   }
   else
   {
      //regular variable
      %setCommand = %this.ownerObject @ "." @ %this.fieldName @ " = \"" @ %text @ "\";";
   }
   eval(%setCommand);
}