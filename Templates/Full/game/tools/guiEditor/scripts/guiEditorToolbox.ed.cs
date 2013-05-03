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

// Code for the toolbox tab of the Gui Editor sidebar.


//---------------------------------------------------------------------------------------------

function GuiEditorToolbox::initialize( %this )
{   
   // Set up contents.
   
   %viewType = %this.currentViewType;
   if( %viewType $= "" )
      %viewType = "Categorized";
      
   %this.currentViewType = "";
   %this.setViewType( %viewType );
      
   %this.isInitialized = true;
}

//---------------------------------------------------------------------------------------------

function GuiEditorToolbox::getViewType( %this )
{
   return %this.currentViewType;
}

//---------------------------------------------------------------------------------------------

function GuiEditorToolbox::setViewType( %this, %viewType )
{
   if( %this.currentViewType $= %viewType
       || !%this.isMethod( "setViewType" @ %viewType ) )
      return;

   %this.clear();
   eval( %this @ ".setViewType" @ %viewType @ "();" );
}

//---------------------------------------------------------------------------------------------

function GuiEditorToolbox::setViewTypeAlphabetical( %this )
{
   %controls = enumerateConsoleClassesByCategory( "Gui" );
   %classes = new ArrayObject();
   
   // Collect relevant classes.
   
   foreach$( %className in %controls )
   {
      if( GuiEditor.isFilteredClass( %className )
          || !isMemberOfClass( %className, "GuiControl" ) )
         continue;
         
      %classes.push_back( %className );
   }
   
   // Sort classes alphabetically.
   
   %classes.sortk( true );
   
   // Add toolbox buttons.

   %numClasses = %classes.count();
   for( %i = 0; %i < %numClasses; %i ++ )
   {
      %className = %classes.getKey( %i );
      %ctrl = new GuiIconButtonCtrl()
      {
         profile = "ToolsGuiIconButtonSmallProfile";
         extent = "128 18";
         text = %className;
         iconBitmap = EditorIconRegistry::findIconByClassName( %className );
         buttonMargin = "2 2";
         iconLocation = "left";
         textLocation = "left";
         textMargin = "24";
         AutoSize = true;

         command = "GuiEditor.createControl( " @ %className @ " );";
         useMouseEvents = true;
         className = "GuiEditorToolboxButton";
         tooltip = %className NL "\n" @ getDescriptionOfClass( %className );
         tooltipProfile = "ToolsGuiToolTipProfile";
      };

      %this.add( %ctrl );
   }

   %classes.delete();
   %this.currentViewType = "Alphabetical";
}

//---------------------------------------------------------------------------------------------

function GuiEditorToolbox::setViewTypeCategorized( %this )
{
   // Create rollouts for each class category we have and
   // record the classes in each category in a temporary array
   // on the rollout so we can later sort the class names before
   // creating the actual controls in the toolbox.

   %controls = enumerateConsoleClassesByCategory( "Gui" );
   foreach$( %className in %controls )
   {
      if( GuiEditor.isFilteredClass( %className )
          || !isMemberOfClass( %className, "GuiControl" ) )
         continue;

      // Get the class's next category under Gui.

      %category = getWord( getCategoryOfClass( %className ), 1 );
      if( %category $= "" )
         continue;

      // Find or create the rollout for the category.

      %rollout = %this.getOrCreateRolloutForCategory( %category );

      // Insert the item.

      if( !%rollout.classes )
         %rollout.classes = new ArrayObject();

      %rollout.classes.push_back( %className );
   }

   // Go through the rollouts, sort the class names, and
   // create the toolbox controls.

   foreach( %rollout in %this )
   {
      if( !%rollout.isMemberOfClass( "GuiRolloutCtrl" ) )
         continue;

      // Get the array with the class names and sort it.
      // Sort in descending order to counter reversal of order
      // when we later add the controls to the stack.

      %classes = %rollout.classes;
      %classes.sortk( true );

      // Add a control for each of the classes to the
      // rollout's stack control.

      %stack = %rollout-->array;
      %numClasses = %classes.count();
      for( %n = 0; %n < %numClasses; %n ++ )
      {
         %className = %classes.getKey( %n );
         %ctrl = new GuiIconButtonCtrl()
         {
            profile = "ToolsGuiIconButtonSmallProfile";
            extent = "128 18";
            text = %className;
            iconBitmap = EditorIconRegistry::findIconByClassName( %className );
            buttonMargin = "2 2";
            iconLocation = "left";
            textLocation = "left";
            textMargin = "24";
            AutoSize = true;

            command = "GuiEditor.createControl( " @ %className @ " );";
            useMouseEvents = true;
            className = "GuiEditorToolboxButton";
            tooltip = %className NL "\n" @ getDescriptionOfClass( %className );
            tooltipProfile = "ToolsGuiToolTipProfile";
         };

         %stack.add( %ctrl );
      }

      // Delete the temporary array.

      %rollout.classes = "";
      %classes.delete();
   }
   
   %this.currentViewType = "Categorized";
}

//---------------------------------------------------------------------------------------------

function GuiEditorToolbox::getOrCreateRolloutForCategory( %this, %category )
{
   // Try to find an existing rollout.
   
   %ctrl = %this.getRolloutForCategory( %category );
   if( %ctrl != 0 )
      return %ctrl;
      
   // None there.  Create a new one.
   
   %ctrl = new GuiRolloutCtrl() {
      Margin = "0 0 0 0";
      DefaultHeight = "40";
      Expanded = "1";
      ClickCollapse = "1";
      HideHeader = "0";
      isContainer = "1";
      Profile = "GuiRolloutProfile";
      HorizSizing = "right";
      VertSizing = "bottom";
      position = "0 0";
      Extent = "421 114";
      MinExtent = "8 2";
      canSave = "1";
      Visible = "1";
      tooltipprofile = "ToolsGuiToolTipProfile";
      hovertime = "1000";
      canSaveDynamicFields = "0";
      autoCollapseSiblings = true;
      caption = %category;
      class = "GuiEditorToolboxRolloutCtrl";

      new GuiDynamicCtrlArrayControl() {
         isContainer = "1";
         Profile = "ToolsGuiDefaultProfile";
         HorizSizing = "right";
         VertSizing = "bottom";
         position = "0 0";
         Extent = "421 64";
         MinExtent = "64 64";
         canSave = "1";
         Visible = "1";
         tooltipprofile = "ToolsGuiToolTipProfile";
         hovertime = "1000";
         canSaveDynamicFields = "0";
         padding = "6 2 4 0";
         colSpacing = "1";
         rowSpacing = "9";
         dynamicSize = true;
         autoCellSize = true;
         internalName = "array";
      };
   };
   
   %this.add( %ctrl );
   %ctrl.collapse();
   
   // Sort the rollouts by their caption.
   
   %this.sort( "_GuiEditorToolboxSortRollouts" );
   
   return %ctrl;
}

//---------------------------------------------------------------------------------------------

function GuiEditorToolbox::getRolloutForCategory( %this, %category )
{
   foreach( %obj in %this )
   {
      if( !%obj.isMemberOfClass( "GuiRolloutCtrl" ) )
         continue;
         
      if( stricmp( %obj.caption, %category ) == 0 )
         return %obj;
   }
   
   return 0;
}

//---------------------------------------------------------------------------------------------

function GuiEditorToolbox::startGuiControlDrag( %this, %class )
{
   // Create a new control of the given class.
   
   %payload = eval( "return new " @ %class @ "();" );
   if( !isObject( %payload ) )
      return;
   
   // this offset puts the cursor in the middle of the dragged object.
   %xOffset = getWord( %payload.extent, 0 ) / 2;
   %yOffset = getWord( %payload.extent, 1 ) / 2;  
   
   // position where the drag will start, to prevent visible jumping.
   %cursorpos = Canvas.getCursorPos();
   %xPos = getWord( %cursorpos, 0 ) - %xOffset;
   %yPos = getWord( %cursorpos, 1 ) - %yOffset;
   
   // Create drag&drop control.
   
   %dragCtrl = new GuiDragAndDropControl()
   {
      canSaveDynamicFields    = "0";
      Profile                 = "ToolsGuiSolidDefaultProfile";
      HorizSizing             = "right";
      VertSizing              = "bottom";
      Position                = %xPos SPC %yPos;
      extent                  = %payload.extent;
      MinExtent               = "32 32";
      canSave                 = "1";
      Visible                 = "1";
      hovertime               = "1000";
      deleteOnMouseUp         = true;
      class                   = "GuiDragAndDropControlType_GuiControl";
   };
   
   %dragCtrl.add( %payload );
   Canvas.getContent().add( %dragCtrl );
   
   // Start drag.

   %dragCtrl.startDragging( %xOffset, %yOffset );
}

//=============================================================================================
//    GuiEditorToolboxRolloutCtrl.
//=============================================================================================

//---------------------------------------------------------------------------------------------

function GuiEditorToolboxRolloutCtrl::onHeaderRightClick( %this )
{
   if( !isObject( GuiEditorToolboxRolloutCtrlMenu ) )
      new PopupMenu( GuiEditorToolboxRolloutCtrlMenu )
      {
         superClass = "MenuBuilder";
         isPopup = true;
         
         item[ 0 ] = "Expand All" TAB "" TAB %this @ ".expandAll();";
         item[ 1 ] = "Collapse All" TAB "" TAB %this @ ".collapseAll();";
      };
      
   GuiEditorToolboxRolloutCtrlMenu.showPopup( %this.getRoot() );
}

//---------------------------------------------------------------------------------------------

function GuiEditorToolboxRolloutCtrl::expandAll( %this )
{
   foreach( %ctrl in %this.parentGroup )
   {
      if( %ctrl.isMemberOfClass( "GuiRolloutCtrl" ) )
         %ctrl.instantExpand();
   }
}

//---------------------------------------------------------------------------------------------

function GuiEditorToolboxRolloutCtrl::collapseAll( %this )
{
   foreach( %ctrl in %this.parentGroup )
   {
      if( %ctrl.isMemberOfClass( "GuiRolloutCtrl" ) )
         %ctrl.instantCollapse();
   }
}

//=============================================================================================
//    GuiEditorToolboxButton.
//=============================================================================================

//---------------------------------------------------------------------------------------------

function GuiEditorToolboxButton::onMouseDragged( %this )
{
   GuiEditorToolbox.startGuiControlDrag( %this.text );
}

//=============================================================================================
//    Misc.
//=============================================================================================

//---------------------------------------------------------------------------------------------

/// Utility function to sort rollouts by their caption.
function _GuiEditorToolboxSortRollouts( %a, %b )
{
   return strinatcmp( %a.caption, %b.caption );
}
