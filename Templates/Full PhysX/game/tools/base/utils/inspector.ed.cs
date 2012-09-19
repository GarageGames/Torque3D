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

// Functionality that allows all editor inspectors to share certain functionality.



//---------------------------------------------------------------------------------------------

function EditorInspectorBase::onAdd( %this )
{
   if( !isObject( EditorInspectorBaseDatablockFieldPopup ) )
      new PopupMenu( EditorInspectorBaseDatablockFieldPopup )
      {
         superClass = "MenuBuilder";
         isPopup = true;
         
         item[ 0 ] = "Edit Datablock" TAB "" TAB "DatablockEditorPlugin.openDatablock( %this.inspectorField.getData() );";
         Item[ 1 ] = "Jump to Definition in Torsion" TAB "" TAB "EditorOpenDeclarationInTorsion( %this.inspectorField.getData() );";
         item[ 2 ] = "Inspect Object" TAB "" TAB "inspectObject( %this.inspectorField.getData() );";
         item[ 3 ] = "-";
         item[ 4 ] = "Copy Value" TAB "" TAB "setClipboard( %this.inspectorField.getData() );";
         item[ 5 ] = "Paste Value" TAB "" TAB "%this.inspectorField.apply( getClipboard() );";
         item[ 6 ] = "Reset to Default" TAB "" TAB "%this.inspectorField.reset();";
         
         inspectorField = -1;
      };

   if( !isObject( EditorInspectorBaseFieldPopup ) )
      new PopupMenu( EditorInspectorBaseFieldPopup )
      {
         superClass = "MenuBuilder";
         isPopup = true;

         item[ 0 ] = "Inspect Object" TAB "" TAB "inspectObject( %this.inspectorField.getData() );";
         Item[ 1 ] = "Jump to Definition in Torsion" TAB "" TAB "EditorOpenDeclarationInTorsion( %this.inspectorField.getData() );";
         item[ 2 ] = "-";
         item[ 3 ] = "Copy Value" TAB "" TAB "setClipboard( %this.inspectorField.getData() );";
         item[ 4 ] = "Paste Value" TAB "" TAB "%this.inspectorField.apply( getClipboard() );";
         item[ 5 ] = "Reset to Default" TAB "" TAB "%this.inspectorField.reset();";

         inspectorField = -1;
      };

   if( !isObject( EditorInspectorBaseFileFieldPopup ) )
      new PopupMenu( EditorInspectorBaseFileFieldPopup )
      {
         superClass = "MenuBuilder";
         isPopup = true;

         item[ 0 ] = "Open File" TAB "" TAB "openFile( %this.filePath );";
         item[ 1 ] = "Open Folder" TAB "" TAB "openFolder( %this.folderPath );";
         item[ 2 ] = "-";
         item[ 3 ] = "Copy Value" TAB "" TAB "setClipboard( %this.inspectorField.getData() );";
         item[ 4 ] = "Paste Value" TAB "" TAB "%this.inspectorField.apply( getClipboard() );";
         item[ 5 ] = "Reset to Default" TAB "" TAB "%this.inspectorField.reset();";

         inspectorField = -1;
         folderPath = "";
         filePath = "";
      };

   if( !isObject( EditorInspectorBaseShapeFieldPopup ) )
      new PopupMenu( EditorInspectorBaseShapeFieldPopup )
      {
         superClass = "MenuBuilder";
         isPopup = true;

         item[ 0 ] = "Edit Shape" TAB "" TAB "ShapeEditorPlugin.openShape( %this.inspectorField.getData() );";
         item[ 1 ] = "-";
         item[ 2 ] = "Open File" TAB "" TAB "openFile( %this.filePath );";
         item[ 3 ] = "Open Folder" TAB "" TAB "openFolder( %this.folderPath );";
         item[ 4 ] = "-";
         item[ 5 ] = "Copy Value" TAB "" TAB "setClipboard( %this.inspectorField.getData() );";
         item[ 6 ] = "Paste Value" TAB "" TAB "%this.inspectorField.apply( getClipboard() );";
         item[ 7 ] = "Reset to Default" TAB "" TAB "%this.inspectorField.reset();";

         inspectorField = -1;
         folderPath = "";
         filePath = "";
      };

   if( !isObject( EditorInspectorBaseProfileFieldPopup ) )
      new PopupMenu( EditorInspectorBaseProfileFieldPopup )
      {
         superClass = "MenuBuilder";
         isPopup = true;

         item[ 0 ] = "Edit Profile" TAB "" TAB "if( !$InGuiEditor ) toggleGuiEditor( true ); GuiEditor.editProfile( %this.inspectorField.getData() );";
         item[ 1 ] = "Jump to Definition in Torsion" TAB "" TAB "EditorOpenDeclarationInTorsion( %this.inspectorField.getData() );";
         item[ 2 ] = "Inspect Object" TAB "" TAB "inspectObject( %this.inspectorField.getData() );";
         item[ 3 ] = "-";
         item[ 4 ] = "Copy Value" TAB "" TAB "setClipboard( %this.inspectorField.getData() );";
         item[ 5 ] = "Paste Value" TAB "" TAB "%this.inspectorField.apply( getClipboard() );";
         item[ 6 ] = "Reset to Default" TAB "" TAB "%this.inspectorField.reset();";

         inspectorField = -1;
         folderPath = "";
         filePath = "";
      };
}

//---------------------------------------------------------------------------------------------

function EditorInspectorBase::onFieldRightClick( %this, %field )
{
   %obj = %this.getInspectObject();
   %fieldValue = %field.getData();

   %inspectIndex = -1;
   %openFileIndex = -1;
   %openFolderIndex = -1;

   // Find out if this is a TypeFilename field referring to a shape file.
   
   %isShapeFilenameField = false;
   if( %field.getInspectedFieldName() $= "shapeName" )
   {
      %isShapeFilenameField =
         %obj.isMemberOfClass( "PhysicsShape" ) ||
         %obj.isMemberOfClass( "TSStatic" );
   }
   else if( %field.getInspectedFieldName() $= "shapeFile" )
   {
      %isShapeFilenameField =
         %obj.isMemberOfClass( "ShapeBaseData" ) ||
         %obj.isMemberOfClass( "ShapeBaseImageData" ) ||
         %obj.isMemberOfClass( "ForestItemData" ) ||
         %obj.isMemberOfClass( "WheeledVehicleTire" ) ||
         %obj.isMemberOfClass( "fxShapeReplicator" ) ||
         %obj.isMemberOfClass( "RenderShapeExample" ) ||
         %obj.isMemberOfClass( "DebrisData" );
   }
   
   // Select the popup.
   
   if( %isShapeFilenameField )
   {
      %popup = EditorInspectorBaseShapeFieldPopup;
      
      %openFileIndex = 2;
      %openFolderIndex = 3;
   }
   else if( EditorInspectorBase::isFileTypeField( %field ) )
   {
      %popup = EditorInspectorBaseFileFieldPopup;
      %openFileIndex = 0;
      %openFolderIndex = 1;      
   }
   else
   {
      switch$( %field.getClassName() )
      {
         case "GuiInspectorCustomField":
            if( %field.getInspectedFieldName() !$= "parentGroup" )
               return;
               
         case "GuiInspectorTypeGuiProfile":
            
            %popup = EditorInspectorBaseProfileFieldPopup;
            
            %popup.enableItem( 0, isObject( %fieldValue ) );
            %inspectIndex = 2;
            %jumpToIndex = 1;
         
         case "GuiInspectorDatablockField" or
              "GuiInspectorTypeSFXDescriptionName" or
              "GuiInspectorTypeSFXEnvironmentName" or
              "GuiInspectorTypeSFXTrackName" or
              "GuiInspectorTypeSFXAmbienceName" or
              "GuiInspectorTypeSFXSourceName":
           
            %popup = EditorInspectorBaseDatablockFieldPopup;
            %popup.enableItem( 0, isObject( %fieldValue ) );
            %inspectIndex = 2;
            %jumpToIndex = 1;
         
         default:
      
            %popup = EditorInspectorBaseFieldPopup;
            %inspectIndex = 0;
            %jumpToIndex = 1;
      }
   }
      
   if( %inspectIndex != -1 )
   {
      %isObject = false;
      if( EditorInspectorBase::isObjectTypeField( %field ) )
         %isObject = isObject( %fieldValue );

      %popup.enableItem( %inspectIndex, %isObject );
      %popup.enableItem( %jumpToIndex, %isObject );
   }
      
   if( %openFileIndex != -1 || %openFolderIndex != -1 )
   {
      %fullPath = EditorInspectorBase::getFullFilePath( %field );
      %popup.filePath = %fullPath;
      %popup.folderPath = filePath( %fullPath );
   
      if( %openFileIndex != -1 )
         %popup.enableItem( 0, isFile( %fullPath ) );
         
      if( %openFolderIndex != -1 )
         %popup.enableItem( 1, isDirectory( %popup.folderPath ) );
   }

   %popup.inspectorField = %field;
   %popup.showPopup( Canvas );
}

//---------------------------------------------------------------------------------------------

function EditorInspectorBase::isObjectTypeField( %field )
{
   // Inspector field types that refer to objects.
   
   switch$( %field.getClassName() )
   {
      case "GuiInspectorDatablockField" or
           "GuiInspectorTypeSFXDescriptionName" or
           "GuiInspectorTypeSFXEnvironmentName" or
           "GuiInspectorTypeSFXTrackName" or
           "GuiInspectorTypeSFXAmbienceName" or
           "GuiInspectorTypeSFXSourceName" or
           "GuiInspectorTypeGuiProfile":
         return true;
   }
   
   // Other console types that refer to objects.
   
   switch$( %field.getInspectedFieldType() )
   {
      case "TypeSimObject" or
           "TypeSimObjectName" or
           "TypeMaterialName" or
           "TypeCubemapName" or
           "TypeGuiProfile":
         return true;
   }
   
   return false;
}

//---------------------------------------------------------------------------------------------

function EditorInspectorBase::isFileTypeField( %field )
{
   return %field.isMemberOfClass( "GuiInspectorTypeFileName" );
}

//---------------------------------------------------------------------------------------------

function EditorInspectorBase::getFullFilePath( %field )
{
   %fileName = %field.getData();
   %inspector = %field.getInspector();
   %object = %inspector.getInspectObject();
   
   if( %object.isMemberOfClass( "Material" ) )
   {
      // Image filenames in materials are relative to the material's file.
      
      %objectPath = filePath( makeFullPath( %object.getFilename(), getMainDotCsDir() ) );
      return makeFullPath( %fileName, %objectPath );
   }
   else
      return makeFullPath( %fileName, getMainDotCsDir() );
}
