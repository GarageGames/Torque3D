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

//---------------------------------------------------------------------------------------------
// Path to the folder that contains the editors we will load.
//---------------------------------------------------------------------------------------------
$Tools::resourcePath = "tools/";

// These must be loaded first, in this order, before anything else is loaded
$Tools::loadFirst = "editorClasses base worldEditor";

//---------------------------------------------------------------------------------------------
// Object that holds the simObject id that the materialEditor uses to interpret its material list
//---------------------------------------------------------------------------------------------
$Tools::materialEditorList = "";

//---------------------------------------------------------------------------------------------
// Tools Package.
//---------------------------------------------------------------------------------------------
package Tools
{
   function loadKeybindings()
   {
      Parent::loadKeybindings();
      

   }
   
   // Start-up.
   function onStart()
   {
      Parent::onStart();            
      
      new Settings(EditorSettings) { file = "tools/settings.xml"; };
      EditorSettings.read();
            
      echo( " % - Initializing Tools" );         
      
      // Default file path when saving from the editor (such as prefabs)
      if ($Pref::WorldEditor::LastPath $= "")
      {
         $Pref::WorldEditor::LastPath = getMainDotCsDir();
      }
      
      // Common GUI stuff.
      exec( "./gui/cursors.ed.cs" );
      exec( "./gui/profiles.ed.cs" );
      exec( "./editorClasses/gui/panels/navPanelProfiles.ed.cs" );
      
      // Make sure we get editor profiles before any GUI's
      // BUG: these dialogs are needed earlier in the init sequence, and should be moved to
      // common, along with the guiProfiles they depend on.
      exec( "./gui/guiDialogs.ed.cs" );
      
      //%toggle = $Scripts::ignoreDSOs;
      //$Scripts::ignoreDSOs = true;
      
      $ignoredDatablockSet = new SimSet();
   
      // fill the list of editors
      $editors[count] = getWordCount( $Tools::loadFirst );
      for ( %i = 0; %i < $editors[count]; %i++ )
      {
         $editors[%i] = getWord( $Tools::loadFirst, %i );
      }
      
      %pattern = $Tools::resourcePath @ "/*/main.cs";
      %folder = findFirstFile( %pattern );
      if ( %folder $= "")
      {
         // if we have absolutely no matches for main.cs, we look for main.cs.dso
         %pattern = $Tools::resourcePath @ "/*/main.cs.dso";
         %folder = findFirstFile( %pattern );
      }
      while ( %folder !$= "" )
      {
         if( filePath( %folder ) !$= "tools" ) // Skip the actual 'tools' folder...we want the children
         {
            %folder = filePath( %folder );
            %editor = fileName( %folder );
            if ( IsDirectory( %folder ) )
            {
               // Yes, this sucks and should be done better
               if ( strstr( $Tools::loadFirst, %editor ) == -1 )
               {
                  $editors[$editors[count]] = %editor;
                  $editors[count]++;
               }
            }
         }
         %folder = findNextFile( %pattern );
      }

      // initialize every editor
      new SimSet( EditorPluginSet );      
      %count = $editors[count];
      for ( %i = 0; %i < %count; %i++ )
      {
         exec( "./" @ $editors[%i] @ "/main.cs" );
         
         %initializeFunction = "initialize" @ $editors[%i];
         if( isFunction( %initializeFunction ) )
            call( %initializeFunction );
      }

      // Popuplate the default SimObject icons that 
      // are used by the various editors.
      EditorIconRegistry::loadFromPath( "tools/classIcons/" );
      
      // Load up the tools resources. All the editors are initialized at this point, so
      // resources can override, redefine, or add functionality.
      Tools::LoadResources( $Tools::resourcePath );
      
      //$Scripts::ignoreDSOs = %toggle;
   }
   
   function startToolTime(%tool)
   {
      if($toolDataToolCount $= "")
         $toolDataToolCount = 0;
      
      if($toolDataToolEntry[%tool] !$= "true")
      {
         $toolDataToolEntry[%tool] = "true";
         $toolDataToolList[$toolDataToolCount] = %tool;
         $toolDataToolCount++;
         $toolDataClickCount[%tool] = 0;
      }
      
      $toolDataStartTime[%tool] = getSimTime();
      $toolDataClickCount[%tool]++;
   }
   
   function endToolTime(%tool)
   {
      %startTime = 0;
      
      if($toolDataStartTime[%tool] !$= "")
         %startTime = $toolDataStartTime[%tool];
      
      if($toolDataTotalTime[%tool] $= "")
         $toolDataTotalTime[%tool] = 0;
         
      $toolDataTotalTime[%tool] += getSimTime() - %startTime;
   }
   
   function dumpToolData()
   {
      %count = $toolDataToolCount;
      for(%i=0; %i<%count; %i++)
      {
         %tool = $toolDataToolList[%i];
         %totalTime = $toolDataTotalTime[%tool];
         if(%totalTime $= "")
            %totalTime = 0;
         %clickCount = $toolDataClickCount[%tool];
         echo("---");
         echo("Tool: " @ %tool);
         echo("Time (seconds): " @ %totalTime / 1000);
         echo("Activated: " @ %clickCount);
         echo("---");
      }
   }
   
   // Shutdown.
   function onExit()
   {
      if( EditorGui.isInitialized )
         EditorGui.shutdown();
      
      // Free all the icon images in the registry.
      EditorIconRegistry::clear();
                  
      // Save any Layouts we might be using
      //GuiFormManager::SaveLayout(LevelBuilder, Default, User);
            
      %count = $editors[count];
      for (%i = 0; %i < %count; %i++)
      {
         %destroyFunction = "destroy" @ $editors[%i];
         if( isFunction( %destroyFunction ) )
            call( %destroyFunction );
      }
      	
      // Call Parent.
      Parent::onExit();

      // write out our settings xml file
      EditorSettings.write();
   }
};

function Tools::LoadResources( %path )
{
   %resourcesPath = %path @ "resources/";
   %resourcesList = getDirectoryList( %resourcesPath );
   
   %wordCount = getFieldCount( %resourcesList );
   for( %i = 0; %i < %wordCount; %i++ )
   {
      %resource = GetField( %resourcesList, %i );
      if( isFile( %resourcesPath @ %resource @ "/resourceDatabase.cs") )
         ResourceObject::load( %path, %resource );
   }
}

//-----------------------------------------------------------------------------
// Activate Package.
//-----------------------------------------------------------------------------
activatePackage(Tools);

