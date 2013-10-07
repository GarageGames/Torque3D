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


//-----------------------------------------------------------------------------
// Register a Content Library Layout
//
// Returns : Layout Object ID or 0.
//-----------------------------------------------------------------------------
function GuiFormManager::InitLayouts( %libraryName, %layoutName, %layoutObj )
{
   // Retrieve Library Object
   %libraryObj = GuiFormManager::FindLibrary( %libraryName );
   if( %libraryObj == 0 )
   {
      error("GuiFormManager::RegisterLayout - Unable to find Library" SPC %libraryName );
      return 0;
   }

   // Load up all Layouts in the layout base path.
   loadDirectory( %libraryObj.basePath, "cs", "dso" );

}

//-----------------------------------------------------------------------------
// Register a Content Library Layout
//
// Returns : True or False.
//-----------------------------------------------------------------------------
function GuiFormManager::RegisterLayout( %libraryName, %layoutName, %layoutObj )
{
   // Retrieve Library Object
   %libraryObj = GuiFormManager::FindLibrary( %libraryName );
   if( %libraryObj == 0 )
   {
      error("GuiFormManager::RegisterLayout - Unable to find Library" SPC %libraryName );
      return false;
   }

   // Retrieve Layout Group
   %layoutGroup = %libraryObj.getObject( 1 );
   if( !isObject( %layoutGroup ) )
   {
      error("GuiFormManager::RegisterLayout - Unable to locate layout group!");
      return false;
   }

   // See if a layout with this name already exists.
   if( GuiFormManager::FindLayout( %libraryName, %layoutName ) != 0 )
   {
      error("GuiFormManager::RegisterLayout - Layout with name" SPC %layoutName SPC "already exists!");
      return false;
   }

   %layoutRef = new ScriptObject()
   {
      layoutGroup   = %layoutGroup;
      layoutName    = %layoutName;
      layoutLibrary = %libraryObj;
      layoutObj     = %layoutObj;
      layoutFile    = %libraryObj.basePath @ %layoutName @ ".cs";     
   };

   // Tag Layout Object Properly so it can reset itself.
   %layoutObj.layoutRef   = %layoutRef;

   // Add Layout Object to group.
   %layoutGroup.add( %layoutObj );

   // Add Layout Object Ref to group.
   %layoutGroup.add( %layoutRef );

   // Return Success.
   return true;
}

//-----------------------------------------------------------------------------
// Unregister a Content Library Layout
//
// Returns : True or False.
//-----------------------------------------------------------------------------
function GuiFormManager::UnregisterLayout( %libraryName, %layoutName, %deleteFile )
{
   %libraryObj = GuiFormManager::FindLibrary( %libraryName );
   if( %libraryObj == 0 )
   {
      error("GuiFormManager::UnregisterLayout - Unable to find Library" SPC %libraryName );
      return false;
   }

   // See if the layout exists.
   %layoutObjRef = GuiFormManager::FindLayout( %libraryObj, %layoutName );

   if( %layoutObjRef == 0 )
      return true;

   // Remove Layout File.
   if( ( %deleteFile == true ) && isFile( %layoutObjRef.layoutFile ) )
      fileDelete( %layoutObjRef.layoutFile );

   // Delete the Object.
   if( isObject( %layoutObjRef.layoutObj ) )
      %layoutObjRef.layoutObj.delete();  

   // Delete the Reference
   %layoutObjRef.delete();

   // Layout Unregistered.
   return true;
      
}

//-----------------------------------------------------------------------------
// Find a Content Library Layout
//
// Returns : Layout Object ID or 0.
//-----------------------------------------------------------------------------
function GuiFormManager::FindLayout( %libraryName, %layoutName )
{
   // Fetch Library Object.
   if( isObject( %libraryName ) && %libraryName.Name !$= "" )
      %libraryName = %libraryName.Name;

   %libraryObj = GuiFormManager::FindLibrary( %libraryName );

   if( %libraryObj == 0 )
   {
      error("GuiFormManager::FindLayout - Unable to find Library" SPC %libraryName );
      return 0;
   }

   // Retrieve Layout Group
   %layoutGroup = %libraryObj.getObject( 1 );
   if( !isObject( %layoutGroup ) )
   {
      error("GuiFormManager::FindLayout - Unable to locate layout group!");
      return 0;
   }

   // Find Layout Object.
   for( %i = 0; %i < %layoutGroup.getCount(); %i++ )
   {
      %layoutGroupIter = %layoutGroup.getObject( %i );
      if( %layoutGroupIter.getClassName() $= "ScriptObject" && %layoutGroupIter.layoutName $= %layoutName )
         return %layoutGroupIter;
   }

   // Not Found
   return 0;
}

//-----------------------------------------------------------------------------
// Save a Content Library Layout
//
// Returns : True or False
//-----------------------------------------------------------------------------
function GuiFormManager::SaveLayout( %library, %layoutName, %newName )
{
   %libraryObj = GuiFormManager::FindLibrary( %library );
   if( %libraryObj == 0 )
   {
      error("GuiFormManager::SaveLayout - Unable to find Library" SPC %library );
      return false;
   }

   %layoutObjRef = GuiFormManager::FindLayout( %library, %layoutName );
   if( %layoutObjRef == 0 )
   {
      error("GuiFormManager::SaveLayout - Cannot find layout" SPC %layoutName );
      return false;
   }  

   // Do any form layout specifics saving.
   GuiFormManager::SaveLayoutContent( %layoutObjRef.layoutObj );

   %newFile = %libraryObj.basePath @  "/" @ %newName @ ".cs";
   if( %newName $= "" )
   {
      %newName = %layoutObjRef.layoutName;
      %newFile = %layoutObjRef.layoutFile;
   }

	// Open Layout File Object.
	%layoutFile = new FileObject();
   if( !%layoutFile.openForWrite( %newFile ) )
   {
      error("GuiFormManager::SaveLayout - Unable to open" SPC %newFile SPC "for writing!");
      %layoutFile.delete();
      return false;
   }

   // Get Layout Object
   %layoutObj = %layoutObjRef.layoutObj;

   // Write Layout Object to File
   %layoutFile.writeObject( %layoutObj, "%layoutObj = " );
   %layoutFile.writeLine("GuiFormManager::RegisterLayout(\"" @ %libraryObj.name @ "\",\"" @ %newName @ "\",%layoutObj);" );
   %layoutFile.close();
   %layoutFile.delete();
    
   // Layout Saved
   return true;

}

//-----------------------------------------------------------------------------
// Reload The Current Layout from the version last stored on disk.
//
// Returns : True or False
//-----------------------------------------------------------------------------
function GuiFormManager::ReloadLayout( %libraryName, %layoutName, %parent )
{
   %layoutObj = GuiFormManager::FindLayout( %libraryName, %layoutName );
   if( %layoutObj == 0 || !isObject( %layoutObj ) )
   {
      error("GuiFormManager::ReloadLayout - Unable to locate layout" SPC %layoutName SPC "in library" SPC %libraryName );
      return 0;
   }

   // Store necessary layout info before the object is destroyed in UnregisterLayout.
   %layoutFile = %layoutObj.layoutFile;

   // Unregister Layout but don't delete the layout file from disk.
   if( !GuiFormManager::UnregisterLayout( %libraryName, %layoutName, false ) )
   {
      error("GuiFormManager::ReloadLayout - Unable to unregister layout file" SPC %layoutFile );
      return 0;
   }

   // Load the layout from disk.
   exec( %layoutFile );

   // Set it active.
   GuiFormManager::ActivateLayout( %libraryName, %layoutName, %parent );

   return true;
}


//-----------------------------------------------------------------------------
// Activate a Layout on a Given Parent.
//
// Returns : True or False
//-----------------------------------------------------------------------------
function GuiFormManager::ActivateLayout( %library, %layoutName, %parent )
{
   %libraryObj = GuiFormManager::FindLibrary( %library );
   if( %libraryObj == 0 )
   {
      error("GuiFormManager::FindLayout - Unable to find Library" SPC %library );
      return 0;
   }

   %layoutObjRef = GuiFormManager::FindLayout( %library, %layoutName );
   if( %layoutObjRef == 0 )
   {
      error("GuiFormManager::ActivateLayout - Cannot find layout" SPC %layoutName );
      return false;
   }

   // Clear parent for new layout.
   %parent.clear();

   %layoutObj = %layoutObjRef.layoutObj;

   // Size to fit parent container.
   %extent = %parent.getExtent();
   %layoutObj.setExtent( GetWord(%extent, 0), GetWord(%extent, 1) );

   // Add to parent.
   %parent.add( %layoutObj );
 
   // Not Found
   return true;
}

//-----------------------------------------------------------------------------
// Deactivate a given layout.
//
// Returns : True or False
//-----------------------------------------------------------------------------
function GuiFormManager::DeactivateLayout( %library, %layoutName )
{
   %libraryObj = GuiFormManager::FindLibrary( %library );
   if( %libraryObj == 0 )
   {
      error("GuiFormManager::DeactivateLayout - Unable to find Library" SPC %library );
      return 0;
   }

   %layoutObjRef = GuiFormManager::FindLayout( %library, %layoutName );
   if( %layoutObjRef == 0 )
   {
      error("GuiFormManager::DeactivateLayout - Cannot find layout" SPC %layoutName );
      return false;
   }

   // Retrieve Layout Group
   %layoutGroup = %libraryObj.getObject( 1 );
   if( !isObject( %layoutGroup ) )
   {
      error("GuiFormManager::RegisterLayout - Unable to locate layout group!");
      return 0;
   }

   // Fetch Layout Object
   %layoutObj = %layoutObjRef.layoutObj;

   // Clear all forms content.
   GuiFormManager::ClearLayoutContent( %layoutObj );

   // Return layout to it's home.
   %layoutGroup.add( %layoutObj );   
   
   // Not Found
   return true;
}

//-----------------------------------------------------------------------------
// Recursively Remove Form Content
//
// Returns : None.
//-----------------------------------------------------------------------------
function GuiFormManager::SaveLayoutContent( %layoutObj )
{
   for( %i = 0; %i < %layoutObj.getCount(); %i++ )
   {
      %object = %layoutObj.getObject( %i );
      if( %object.isMemberOfClass( "SimGroup" ) )
      {
         %formContent = 0;
         if (%object.getCount() > 0)
            %formContent = %object.getObject( 1 );
         
         if( isObject( %formContent ) && %object.ContentLibrary !$= "" && %object.Content !$= "" )
         {
            %contentObj = GuiFormManager::FindFormContent( %object.ContentLibrary, %object.Content );
            if( %contentObj == 0 )
            {
               error("GuiFormManager::SaveLayoutContent - Content Library Specified But Content Not Found!" );
               return;
            }

            if( %contentObj.SaveFunction !$= "" )
               eval( %contentObj.SaveFunction @ "(" @ %object @ "," @ %formContent @ ");" );
         }
      }
      else
         GuiFormManager::SaveLayoutContent( %object );
   }
}

//-----------------------------------------------------------------------------
// Recursively Remove Form Content
//
// Returns : None.
//-----------------------------------------------------------------------------
function GuiFormManager::ClearLayoutContent( %layoutObj )
{
   for( %i = 0; %i < %layoutObj.getCount(); %i++ )
   {
      %object = %layoutObj.getObject( %i );
      if( %object.getClassName() $= "GuiFormCtrl" )
      {
         // Clear Content ID So that onWake recreates the content.
         %object.ContentID = "";

         %formContent = %object.getObject( 1 );
         if( isObject( %formContent ) )
            %formContent.delete();         
      }
      else
         GuiFormManager::ClearLayoutContent( %object );
   }
}