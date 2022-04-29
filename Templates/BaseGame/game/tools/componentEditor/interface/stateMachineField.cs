function BehaviorFieldStack::createStateMachineEditor(%this, %behavior, %fieldIndex)
{
   %fieldInfo = %behavior.template.getBehaviorField(%fieldIndex);
   %name = getField(%fieldInfo, 0);
   
   %button = new GuiButtonCtrl()
   {
      class = EditStateMachineBtn;
      HorizSizing = "right";
      VertSizing = "bottom";
      Position = "0 2";
      Extent = (%this.extent.x - 8) SPC 13;
      text = "Edit States";
      tooltip = "Open window to edit the state machine";
      behavior = %behavior;
   };
   %this.add(%button);
}

function EditStateMachineBtn::onClick(%this)
{
   Canvas.pushDialog(StateMachineEditor);
   StateMachineEditor.behavior = %this.behavior;
   
   StateMachineEditor.open();
}

function StateMachineEditor::open(%this)
{
   //check our behavior and see if we have any existing state/field info to work with
   //if we do, load those up first
   for(%i = 0; %i < %this.behavior.stateMachine.count(); %i++)
   {
      %stateName = %this.behavior.stateMachine.getKey(%i);
      
      %this.addState(%stateName);
   }    
}

function StateMachineEditor::addState(%this, %stateName)
{
   if(%stateName $= "")
      %stateName = "New State";
      
   %state = new GuiControl() {
      position = "0 0";
      extent = "285 50";
      horizSizing = "horizResizeWidth";
      vertSizing = "vertResizeTop";
      isContainer = "1";
      
      new GuiTextEditCtrl() {
         position = "0 0";
         extent = "100 15";
         text = %stateName;
      };
      
      new GuiButtonCtrl() {
         //buttonMargin = "4 4";
         text = "Remove State";
         position = "184 0";
         extent = "100 15";
         //profile = "GuiButtonProfile";
         command = "ScriptEditorGui.save();";
      };
      
      new GuiSeparatorCtrl() {
         position = "0 15";
         extent = %this.extent.x SPC "10";
         type = "horizontal";
      };
      
      new GuiStackControl(%stateName@"StateStack")
      {
         //Profile = "EditorContainerProfile";
         HorizSizing = "right";
         VertSizing = "bottom";
         Position = "0 25";
         Extent = "285 20";
         padding = 4;
         
         new GuiButtonCtrl() {
            text = "Add field";
            position = "3 0";
            extent = "280 20";
            horizSizing = "left";
            vertSizing = "top";
            command = "StateMachineEditor.addField("@%stateName@");";
         };
      };
   };
   
   %this-->Stack.add(%state);
   //%this-->stateStackScroll.computeSizes();
}

function StateMachineEditor::addField(%this, %stateName)
{
   %index = %this.behavior.stateMachine.count();
   %field = new GuiControl() {
      position = "0 0";
      extent = "285 20";
      horizSizing = "width";
      vertSizing = "height";
      isContainer = "1";
      fieldValueCtrl = "";
      fieldID = %index++;
   };
      
   %fieldList = new GuiPopUpMenuCtrlEx() 
   {
      class = "stateMachineFieldList";
      Profile = "GuiPopupMenuProfile";
      HorizSizing = "width";
      VertSizing = "bottom";
      position = "0 1";
      Extent = "120 18";
      behavior = %this.behavior;
   };
   
   %field.add(%fieldList);
   %fieldList.refresh();
   
    (%stateName@"StateStack").addToStack(%field);
      
   %this-->Stack.updateStack();
   %this-->stateStackScroll.computeSizes();
   
   %this.behavior.addStateField(%stateName, "", "");
   
   return %field;
}

//==============================================================================
function stateMachineFieldList::refresh(%this)
{
   %this.clear();
   
   // Find all the types.
   %count = getWordCount(%this.behavior.stateFields);   
   %index = 0;
   for (%j = 0; %j < %count; %j++)
   {
      %item = getWord(%this.behavior.stateFields, %j);
      %this.add(%item, %index);
      %this.fieldType[%index] = %item;
      %index++;
   }
}

function stateMachineFieldList::onSelect(%this)
{
   //if(%this.getParent().fieldValueCtrl $= "")
   %this.fieldType = %this.fieldType[%this.getSelected()];
   
   if(%this.fieldType $= "transitionOnAnimEnd" || %this.fieldType $= "transitionOnAnimTrigger" 
      || %this.fieldType $= "transitionOnTimeout")
   {
      %fieldCtrl = new GuiPopUpMenuCtrlEx() 
      {
         class = "stateMachineFieldList";
         Profile = "GuiPopupMenuProfile";
         HorizSizing = "width";
         VertSizing = "bottom";
         position = "124 1";
         Extent = "120 18";
      };
   }
   else if(%this.fieldType $= "animation")
   {
      %fieldCtrl = new GuiPopUpMenuCtrlEx() 
      {
         class = "stateMachineFieldList";
         Profile = "GuiPopupMenuProfile";
         HorizSizing = "width";
         VertSizing = "bottom";
         position = "124 1";
         Extent = "120 18";
      };
      
      %index = 0;
      %animBhvr = %this.behavior.owner.getBehavior("AnimationController");
      for(%i = 0; %i < %animBhvr.getAnimationCount(); %i++)
      {
         %item = %animBhvr.getAnimationName(%i);
         %fieldCtrl.add(%item, %index);
         %fieldCtrl.fieldValue[%index] = %item;
         %index++;
      }
   }
   else
   {
      %fieldCtrl = new GuiTextEditCtrl() {
         position = "124 1";
         extent = "120 10";
         text = "";
      };
   }
   
   //get the state machine entry
   %index = %this.getParent().fieldID;
   
   %oldValue = %this.behavior.stateMachine.getValue(%index);
   %this.behavior.stateMachine.setValue(%fieldType SPC %oldValue.y);
   
   %this.getParent().add(%fieldCtrl);
}

//==============================================================================

//Now for the unique field types
/*function stateMachineFieldList::refresh(%this)
{
   
}*/