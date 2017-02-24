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

function GuiInspectorEntityGroup::CreateContent(%this)
{
}

function GuiInspectorEntityGroup::InspectObject( %this, %targetObject )
{
   %this.stack.clear();
   %this.stack.addGuiControl(%this.createAddComponentList());
}

function GuiInspectorEntityGroup::createAddComponentList(%this)
{
   %extent = %this.getExtent();
   
   %container = new GuiControl()
   {
      Profile = "EditorContainerProfile";
      HorizSizing = "width";
      VertSizing = "bottom";
      Position = "0 0";
      Extent = %extent.x SPC "25";
   };

   %componentList = new GuiPopUpMenuCtrlEx(QuickEditComponentList) 
   {
      Profile = "GuiPopupMenuProfile";
      HorizSizing = "width";
      VertSizing = "bottom";
      position = "28 4";
      Extent = (%extent.x - 28) SPC "18";
      hovertime = "100";
      tooltip = "The component to add to the object";
      tooltipProfile = "EditorToolTipProfile";
   };

   %addButton = new GuiIconButtonCtrl() {
      class = AddComponentQuickEditButton;
      Profile = "EditorButton";
      HorizSizing = "right";
      VertSizing = "bottom";
      Position = "2 0";
      Extent = "24 24";
      buttonMargin = "4 4";
      iconLocation = "Left";
      sizeIconToButton = "0";
      iconBitmap = "tools/gui/images/iconAdd.png";
      hovertime = "100";
      tooltip = "Add the selected component to the object";
      tooltipProfile = "EditorToolTipProfile";
      componentList = %componentList;
   };
   
   %componentList.refresh();
   
   %container.add(%componentList);
   %container.add(%addButton);
   
   if(!isObject("componentTooltipTheme"))
   {
      %theme = createsupertooltiptheme("componentTooltipTheme");
      %theme.addstyle("headerstyle", "<just:left><font:arial bold:16><color:000000>");
      %theme.addstyle("headertwostyle", "<font:arial bold:14><color:000000>");
      %theme.addstyle("basictextstyle", "<font:arial:14><color:000000>");
      %theme.setdefaultstyle("title", "headerstyle");
      %theme.setdefaultstyle("paramtitle", "headertwostyle");
      %theme.setdefaultstyle("param", "basictextstyle");
      %theme.setspacing(3, 0);
   }
   
   return %container;
}

function QuickEditComponentList::refresh(%this)
{
   %this.clear();
   
   //find all ComponentAssets
   %assetQuery = new AssetQuery();
   if(!AssetDatabase.findAssetType(%assetQuery, "ComponentAsset"))
      return; //if we didn't find ANY, just exit
   
   // Find all the types.
   %count = %assetQuery.getCount();

   %categories = "";
   for (%i = 0; %i < %count; %i++)
   {
      %assetId = %assetQuery.getAsset(%i);
      
      %componentAsset = AssetDatabase.acquireAsset(%assetId);
      %componentType = %componentAsset.componentType;
      if (!isInList(%componentType, %categories))
         %categories = %categories TAB %componentType;
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
         %componentType = %componentAsset.componentType;
         %friendlyName = %componentAsset.friendlyName;
		
         if (%componentType $= %category)
         {
            //TODO: Haven't worked out getting categories to look distinct
            //from entries in the drop-down so for now just indent them for the visual distinction
            %spacedName = "    " @ %friendlyName;
            %this.add(%spacedName, %index);
            %this.component[%index] = %componentAsset;
            %index++;
         }
      }
   }
}

function QuickEditComponentList::onHotTrackItem( %this, %itemID )
{
   %componentObj = %this.component[%itemID];
   if( isObject( %componentObj ) && %this.componentDesc != %componentObj )
   {
      SuperTooltipDlg.init("componentTooltipTheme");
      SuperTooltipDlg.setTitle(%componentObj.friendlyName);
      SuperTooltipDlg.addParam("", %componentObj.description @ "\n");
      
      %fieldCount = %componentObj.getComponentFieldCount();
      for (%i = 0; %i < %fieldCount; %i++)
      {
         %name = getField(%componentObj.getComponentField(%i), 0);

         SuperTooltipDlg.addParam(%name, %description @ "\n");
      }
      %position = %this.getGlobalPosition();
      SuperTooltipDlg.processTooltip( %position,0,1 );
      %this.opened = true;    
      %this.componentDesc = %componentObj;
   }
   else if( !isObject( %componentObj ) )
   {
      if( %this.opened == true )
         SuperTooltipDlg.hide();
      %this.componentDesc = "";
   }      
}

function QuickEditComponentList::setProperty(%this, %object)
{
   %this.objectToAdd = %object;
}

function QuickEditComponentList::onSelect(%this)
{
   if( %this.opened == true )
      SuperTooltipDlg.hide();
   
   %this.componentToAdd = %this.component[%this.getSelected()];
}

function QuickEditComponentList::onCancel( %this )
{
   if( %this.opened == true )
      SuperTooltipDlg.hide();
}

function AddComponentQuickEditButton::onClick(%this)
{
   %component = %this.componentList.componentToAdd;
	
	%componentName = %this.componentList.componentToAdd.componentName;
	%componentClass = %this.componentList.componentToAdd.componentClass;
	
	%command = "$ComponentEditor::newComponent = new" SPC %componentClass SPC "(){ class = \"" 
	@ %componentName @ "\"; };";
	
	eval(%command);
   
   %instance = $ComponentEditor::newComponent;
   %undo = new UndoScriptAction()
   {
      actionName = "Added Component";
      class = UndoAddComponent;
      object = %this.componentList.objectToAdd;
      component = %instance;
   };

   %undo.addToManager(LevelBuilderUndoManager);
   
   %instance.owner = Inspector.getInspectObject(0);
   %instance.owner.add(%instance);
   
   Inspector.schedule( 50, "refresh" );
   EWorldEditor.isDirty = true;
}

function addComponent(%obj, %instance)
{
   echo("Adding the component!");
   %obj.addComponent(%instance);
   Inspector.schedule( 50, "refresh" );
   EWorldEditor.isDirty = true;
}

