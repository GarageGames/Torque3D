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

// Common code for object selection dialogs.


//=============================================================================================
//    Initialization.
//=============================================================================================

//---------------------------------------------------------------------------------------------

function EObjectSelection::init( %this )
{
   // Initialize the class list.
   
   %classList = %this-->classList;
   if( isObject( %classList ) )
      %this.initClassList();
      
   // Initialize the filter list.
   
   %filterList = %this-->filterList;
   if( isObject( %filterList ) )
      %this.initFilterList();
      
   // Initialize the group list.
   
   %groupList = %this-->groupList;
   if( isObject( %groupList ) )
      %this.initGroupList();
}

//---------------------------------------------------------------------------------------------

function EObjectSelection::cleanup( %this )
{      
   // Clear the class list.
      
   %classList = %this-->classList;
   if( isObject( %classList ) )
      %classList.clear();
      
   // Clear the filter list.
      
   %filterList = %this-->filterList;
   if( isObject( %filterList ) )
      %filterList.clear();
      
   // Clear the group list.
   
   %groupList = %this-->groupList;
   if( isObject( %groupList ) )
      %groupList.clear();

   // Delete the class array.

   if( isObject( %this.classArray ) )
      %this.classArray.delete();
}

//=============================================================================================
//    Methods to override in a subclass.
//=============================================================================================

//---------------------------------------------------------------------------------------------

/// Return the group object where onSelectObjects should begin searching for objects.
function EObjectSelection::getRootGroup( %this )
{
   return RootGroup;
}

//---------------------------------------------------------------------------------------------

/// Return a set that contains all filter objects to include in the filter list.
/// Returning 0 will leave the filter list empty.
function EObjectSelection::getFilterSet( %this )
{
   return 0;
}

//---------------------------------------------------------------------------------------------

/// Return true if the given class should be included in the class list.
function EObjectSelection::includeClass( %this, %className )
{
   return true;
}

//---------------------------------------------------------------------------------------------

/// The object has met the given criteria.  Select or deselect it depending on %val.
function EObjectSelection::selectObject( %this, %object, %val )
{
}

//=============================================================================================
//    Events.
//=============================================================================================

//---------------------------------------------------------------------------------------------

function EObjectSelection::onSelectObjects( %this, %val )
{
   // Get the root group to search in.
   
   %groupList = %this-->groupList;
   if( !isObject( %groupList ) )
      %root = %this.getRootGroup();
   else
      %root = %groupList.getSelected();
      
   if( !isObject( %root ) )
      return;
      
   // Fetch the object name pattern.
   
   %namePatternField = %this-->namePattern;
   if( isObject( %namePatternField ) )
      %this.namePattern = %namePatternField.getText();
   else
      %this.namePattern = "";
      
   // Clear current selection first, if need be.
   
   if( %val )
   {
      %retainSelectionBox = %this-->retainSelection;
      if( isObject( %retainSelectionBox ) && !%retainSelectionBox.isStateOn() )
         %this.clearSelection();
   }
      
   // (De)Select all matching objects in it.
      
   %this.selectObjectsIn( %root, %val, true );
}

//=============================================================================================
//    Selection.
//=============================================================================================

//---------------------------------------------------------------------------------------------

function EObjectSelection::selectObjectsIn( %this, %group, %val, %excludeGroup )
{
   // Match to the group itself.
   
   if( !%excludeGroup && %this.objectMatchesCriteria( %group ) )
      %this.selectObject( %group, %val );
      
   // Recursively match all children.
      
   foreach( %obj in %group )
   {
      if( %obj.isMemberOfClass( "SimSet" ) )
         %this.selectObjectsIn( %obj, %val );
      else if( %this.objectMatchesCriteria( %obj ) )
         %this.selectObject( %obj, %val );
   }
}

//---------------------------------------------------------------------------------------------

function EObjectSelection::objectMatchesCriteria( %this, %object )
{
   // Check name.
   
   if( %this.namePattern !$= "" && !strIsMatchExpr( %this.namePattern, %object.getName() ) )
      return false;
      
   // Check class.
   
   if( !%this.isClassEnabled( %object.getClassName() ) )
      return false;
      
   return true;
}

//=============================================================================================
//    Groups.
//=============================================================================================

//---------------------------------------------------------------------------------------------

function EObjectSelection::initGroupList( %this )
{
   %groupList = %this-->groupList;

   %selected = 0;
   if( %groupList.size() > 0 )
      %selected = %groupList.getSelected();
      
   %groupList.clear();
   
   %root = %this.getRootGroup();
   if( !isObject( %root ) )
      return;
      
   // Add all non-empty groups.
      
   %this.scanGroup( %root, %groupList, 0 );
   
   // Select initial group.
   
   if( %selected != 0 && isObject( %selected ) )
      %groupList.setSelected( %selected );
   else
      %groupList.setSelected( %root.getId() );
}

//---------------------------------------------------------------------------------------------

function EObjectSelection::scanGroup( %this, %group, %list, %indentLevel )
{
   // Create a display name for the group.
   
   %text = %group.getName();
   if( %text $= "" )
      %text = %group.getClassName(); 
      
   %internalName = %group.getInternalName();
   if( %internalName !$= "" )
      %text = %text @ " [" @ %internalName @ "]";
   
   // Indent the name according to the depth in the hierarchy.
   
   if( %indentLevel > 0 )
      %text = strrepeat( "  ", %indentLevel ) @ %text;
      
   // Add it to the list.
      
   %list.add( %text, %group.getId() );
   
   // Recurse into SimSets with at least one child.
   
   foreach ( %obj in %group )
   {
      if(    !%obj.isMemberOfClass( "SimSet" )
          || %obj.getCount() == 0 )
         continue;
         
      %this.scanGroup( %obj, %list, %indentLevel + 1 );
   }
}

//=============================================================================================
//    Filters.
//=============================================================================================

//---------------------------------------------------------------------------------------------

function EObjectSelection::initFilterList( %this )
{
   %filterList = %this-->filterList;
}

//=============================================================================================
//    Classes.
//=============================================================================================

//---------------------------------------------------------------------------------------------

/// Initialize the list of class toggles.
function EObjectSelection::initClassList( %this )
{   
   %classArray = new ArrayObject();
   %this.classArray = %classArray;
      
   // Add all classes to the array.
   
   %classes = enumerateConsoleClasses();
   foreach$( %className in %classes )
   {
      if( !%this.includeClass( %className ) )
         continue;

      %classArray.push_back( %className, true );
   }
   
   // Sort the class list.
   
   %classArray.sortk( true );
   
   // Add checkboxes for all classes to the list.
   
   %classList = %this-->classList;
   %count = %classArray.count();
   for( %i = 0; %i < %count; %i ++ )
   {
      %className = %classArray.getKey( %i );
      %textLength = strlen( %className );
      %text = " " @ %className;

      %checkBox = new GuiCheckBoxCtrl()
      {
         canSaveDynamicFields = "0";
         isContainer = "0";
         Profile = "ToolsGuiCheckBoxListFlipedProfile";
         HorizSizing = "right";
         VertSizing = "bottom";
         Position = "0 0";
         Extent = ( %textLength * 4 ) @ " 18";
         MinExtent = "8 2";
         canSave = "0";
         Visible = "1";
         tooltipprofile = "ToolsGuiToolTipProfile";
         hovertime = "1000";
         tooltip = "Include/exclude all " @ %className @ " objects.";
         text = %text;
         groupNum = "-1";
         buttonType = "ToggleButton";
         useMouseEvents = "0";
         useInactiveState = "0";
         command = %classArray @ ".setValue( $ThisControl.getValue(), " @ %i @ " );";
      };

      %checkBox.setStateOn( true );
      %classList.addGuiControl( %checkBox );
   }
}

//---------------------------------------------------------------------------------------------

function EObjectSelection::selectAllInClassList( %this, %state )
{
   %classList = %this-->classList;
   
   foreach( %ctrl in %classList )
   {
      if( %ctrl.getValue() == %state )
         %ctrl.performClick();
   }
}

//---------------------------------------------------------------------------------------------

function EObjectSelection::isClassEnabled( %this, %className )
{
   // Look up the class entry in the array.
   
   %index = %this.classArray.getIndexFromKey( %className );
   if( %index == -1 )
      return false;
      
   // Return the flag.
      
   return %this.classArray.getValue( %index );
}
