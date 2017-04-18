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

// The "Object Inspector" is a useful little window for browsing and editing SimObject
// hierarchies.  Be aware that there is no undo in the inspector.

//---------------------------------------------------------------------------------------------

/// Bring up a new inspector window on the given object.
function inspectObject( %object )
{
   if( !isObject( %object ) )
   {
      error( "inspectObject: no object '" @ %object @ "'" );
      return;
   }
      
   // Create a new object inspector window.
   exec( "./guiObjectInspector.ed.gui" );
   
   if( !isObject( %guiContent) )
   {
      error( "InspectObject: failed to create GUI from 'guiObjectInspector.ed.gui'" );
      return;
   }
   
   // Initialize the inspector.
   
   %guiContent.init( %object );
      
   Canvas.getContent().add( %guiContent );
}

//=============================================================================================
//    GuiObjectInspector
//=============================================================================================

//---------------------------------------------------------------------------------------------

function GuiObjectInspector::init( %this, %object )
{   
   if( !%object.isMemberOfClass( "SimSet" ) )
   {
      // Complete deletely the splitter and the left-side part of the inspector
      // leaving only the field inspector.
      
      %this.add( %this-->panel2 );
      %this-->splitter.delete();
      %this-->inspector.inspect( %object );
      %this-->methodList.init( %object );
   }
   else
   {
      %treeView = %this-->treeView;
      %treeView.inspectorCtrl = %this-->inspector;
      %treeView.methodList = %this-->methodList;

      %treeView.open( %object );
   }
   
   // Set window caption.
   
   %caption = "Object Inspector - " @ %object.getId() @ " : " @ %object.getClassName();
   
   %name = %object.getName();
   if( %name !$= "" )
      %caption = %caption @ " - " @ %name;
      
   %this.text = %caption;
}

//---------------------------------------------------------------------------------------------

function GuiObjectInspector::onClose( %this )
{
   // Delete us.
   %this.schedule( 1, "delete" );
}

//=============================================================================================
//    GuiObjectInspectorTree
//=============================================================================================

//---------------------------------------------------------------------------------------------

function GuiObjectInspectorTree::onSelect( %this, %object )
{
   if( isObject( %object ) )
   {
      %this.inspectorCtrl.inspect( %object );
      %this.methodList.init( %object );
   }
}

//---------------------------------------------------------------------------------------------

function GuiObjectInspectorTree::onRightMouseUp( %this, %itemId, %mousePos, %object )
{
   if( !isObject( GuiObjectInspectorTreePopup ) )
      new PopupMenu( GuiObjectInspectorTreePopup )
      {
         superClass = "MenuBuilder";
         isPopup = true;

         item[ 0 ] = "Jump to Definition in Torsion" TAB "" TAB "EditorOpenDeclarationInTorsion( %this.object );";

         object = "";
      };
      
   GuiObjectInspectorTreePopup.object = %object;
   GuiObjectInspectorTreePopup.showPopup( Canvas );
}

//=============================================================================================
//    GuiObjectInspectorMethodList
//=============================================================================================

//---------------------------------------------------------------------------------------------

function GuiObjectInspectorMethodList::init( %this, %object )
{
   %this.clear();
   
   %methods = %object.dumpMethods();
   %count = %methods.count();
   %methodsGroup = %this.insertItem( 0, "Methods" );
   %parentScripted = %this.insertItem( %methodsGroup, "Scripted" );
   %parentNative = %this.insertItem( %methodsGroup, "Native" );
   
   for( %i = 0; %i < %count; %i ++ )
   {
      %name = %methods.getKey( %i );
      %value = %methods.getValue( %i );
      %prototype = getRecord( %value, 2 );
      %fileName = getRecord( %value, 3 );
      %lineNumber = getRecord( %value, 4 );
      %usage = getRecords( %value, 5 );
            
      %tooltip = %prototype;
      if( isFile( %fileName ) )
      {
         %parent = %parentScripted;
         %tooltip = %tooltip NL "Declared in: " @ %fileName @ ":" @ %lineNumber;
      }
      else
         %parent = %parentNative;
         
      %tooltip = %tooltip @ "\n\n" @ %usage;

      %id = %this.insertItem( %parent, %prototype, %fileName NL %lineNumber );
      %this.setItemTooltip( %id, %tooltip );
   }
      
   %methods.delete();
   
   if( %object.isMethod( "getDebugInfo" ) )
   {
      %debugInfo = %object.getDebugInfo();      
      %count = %debugInfo.count();
      %parent = %this.insertItem( 0, "Debug Info" );
      
      for( %i = 0; %i < %count; %i ++ )
         %id = %this.insertItem( %parent, %debugInfo.getKey( %i ) @ ": " @ %debugInfo.getValue( %i ) );
      
      %debugInfo.delete();
   }

   %this.sort( 0, true );
}

//---------------------------------------------------------------------------------------------

function GuiObjectInspectorMethodList::onRightMouseUp( %this, %item, %mousePos )
{
   %value = %this.getItemValue( %item );
   if( %value $= "" )
      return;
      
   %fileName = getRecord( %value, 0 );
   %lineNumber = getRecord( %value, 1 );

   if( isFile( %fileName ) )
   {
      if( !isObject( GuiInspectorMethodListPopup ) )
         new PopupMenu( GuiInspectorMethodListPopup )
         {
            superClass = "MenuBuilder";
            isPopup = true;

            item[ 0 ] = "Jump to Definition in Torsion" TAB "" TAB "EditorOpenFileInTorsion( %this.jumpFileName, %this.jumpLineNumber );";

            jumpFileName = "";
            jumpLineNumber = "";
         };
         
      GuiInspectorMethodListPopup.jumpFileName = %fileName;
      GuiInspectorMethodListPopup.jumpLineNumber = %lineNumber;
      
      GuiInspectorMethodListPopup.showPopup( Canvas );
   }
}

//=============================================================================================
//    GuiObjectInspectorTreeFilter
//=============================================================================================

//---------------------------------------------------------------------------------------------

function GuiObjectInspectorTreeFilter::onWake( %this )
{
   %treeView = %this.getParent()-->TreeView;
   if( isObject( %treeView ) )
      %this.treeView = %treeView;
      
   Parent::onWake( %this );
}

//=============================================================================================
//    GuiObjectInspectorTreeFilter
//=============================================================================================

//---------------------------------------------------------------------------------------------

function GuiObjectInspectorTreeFilterClearButton::onWake( %this )
{
   %filterText = %this.getParent()-->FilterText;
   if( isObject( %filterText ) )
      %this.textCtrl = %filterText;
}
