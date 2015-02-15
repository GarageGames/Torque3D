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



//=============================================================================
//    AddFMODProjectDlg.
//=============================================================================

//-----------------------------------------------------------------------------

function AddFMODProjectDlg::show( %this )
{
   if( $platform $= "macos" )
   {
      %fmodex = "libfmodex.dylib";
      %fmodevent = "libfmodevent.dylib";
   }
   else
   {
      %fmodex = "fmodex.dll";
      %fmodevent = "fmod_event.dll";
   }

   // Make sure we have FMOD running.
   
   if( getField( sfxGetDeviceInfo(), $SFX::DEVICE_INFO_PROVIDER ) !$= "FMOD" )
   {
      MessageBoxOK( "Error",
         "You do not currently have FMOD selected as your sound system." NL
         "" NL
         "To install FMOD, place the FMOD DLLs (" @ %fmodex @ " and " @ %fmodevent @ ")" SPC
         "in your game/ folder alongside your game executable" SPC
         "and restart Torque." NL
         "" NL
         "To select FMOD as your sound system, choose it as the sound provider in" SPC
         "the audio tab of the Game Options dialog."
      );
      
      return;
   }
   
   // Make sure we have the FMOD Event DLL loaded.
   
   %deviceCaps = getField( sfxGetDeviceInfo(), $SFX::DEVICE_INFO_CAPS );
   if( !( %deviceCaps & $SFX::DEVICE_CAPS_FMODDESIGNER ) )
   {
      MessageBoxOK( "Error",
         "You do not have the requisite FMOD Event DLL in place." NL
         "" NL
         "Please copy " @ %fmodevent @ " into your game/ folder and restart Torque."
      );
      return;
   }
   
   // Show it.
      
   Canvas.pushDialog( %this, 0, true );
}

//-----------------------------------------------------------------------------

function AddFMODProjectDlg::onWake( %this )
{
   %this.persistenceMgr = new PersistenceManager();
}

//-----------------------------------------------------------------------------

function AddFMODProjectDlg::onSleep( %this )
{
   %this.persistenceMgr.delete();
}

//-----------------------------------------------------------------------------

function AddFMODProjectDlg::onCancel( %this )
{
   Canvas.popDialog( %this );
}

//-----------------------------------------------------------------------------

function AddFMODProjectDlg::onOK( %this )
{
   %objName    = %this-->projectNameField.getText();
   %fileName   = %this-->fileNameField.getText();
   %mediaPath  = %this-->mediaPathField.getText();
   
   // Make sure the object name is valid.
   if( !Editor::validateObjectName( %objName, true ))
      return;

   // Make sure the .fev file exists.
   
   if( %fileName $= "" )
   {
      MessageBoxOK( "Error",
         "Please enter a project file name."
      );
      return;
   }
   if( !isFile( %fileName ) )
   {
      MessageBoxOK( "Error",
         "'" @ %fileName @ "' is not a valid file."
      );
      return;
   }
   
   // Make sure the media path exists.
   
   if( !isDirectory( %mediaPath ) )
   {
      MessageBoxOK( "Error",
         "'" @ %mediaPath @ "' is not a valid directory."
      );
      return;
   }
   
   // If an event script exists from a previous instantiation,
   // delete it first.
   
   %eventFileName = %fileName @ ".cs";
   if( isFile( %eventFileName ) )
      fileDelete( %eventFileName );
   
   // Create the FMOD project object.
   
   pushInstantGroup();
   eval( "new SFXFMODProject( " @ %objName @ ") {" NL
      "fileName = \"" @ %fileName @ "\";" NL
      "mediaPath = \"" @ %mediaPath @ "\";" NL
   "};" );
   popInstantGroup();
   
   if( !isObject( %objName ) )
   {
      MessageBoxOK( "Error",
         "Failed to create the object.  Please take a look at the log for details."
      );
      return;
   }
   else
   {
      // Save the object.

      %objName.setFileName( "scripts/client/audioData.cs" );
      %this.persistenceMgr.setDirty( %objName );
      %this.persistenceMgr.saveDirty();
   }
      
   Canvas.popDialog( %this );
   
   // Trigger a reinit on the datablock editor, just in case.
   
   if( isObject( DatablockEditorPlugin ) )
      DatablockEditorPlugin.populateTrees();
}

//-----------------------------------------------------------------------------

function AddFMODProjectDlg::onSelectFile( %this )
{
   if( $pref::WorldEditor::AddFMODProjectDlg::lastPath $= "" )
      $pref::WorldEditor::AddFMODProjectDlg::lastPath = getMainDotCsDir();

   %dlg = new OpenFileDialog()
   {
      Title       = "Select Compiled FMOD Designer Event File...";
      Filters     = "Compiled Event Files (*.fev)|*.fev|All Files (*.*)|*.*|";
      DefaultPath = $pref::WorldEditor::AddFMODProjectDlg::lastPath;
      DefaultFile = fileName( %this-->fileNameField.getText() );
      MustExit    = true;
      ChangePath  = false;
   };
   
   %ret = %dlg.execute();
   if( %ret )
   {
      %file = %dlg.fileName;
      $pref::WorldEditor::AddFMODProjectDlg::lastPath = filePath( %file );
   }
   
   %dlg.delete();
   
   if( !%ret )
      return;
      
   %file = makeRelativePath( %file, getMainDotCsDir() );
   %this-->fileNameField.setText( %file );
   
   if( %this-->projectNameField.getText() $= "" )
   {
      %projectName = "fmod" @ fileBase( %file );
      if( isValidObjectName( %projectName ) )
         %this-->projectNameField.setText( %projectName );
   }
}

//-----------------------------------------------------------------------------

function AddFMODProjectDlg::onSelectMediaPath( %this )
{
   %defaultPath = %this-->mediaPathField.getText();
   if( %defaultPath $= "" )
   {
      %defaultPath = filePath( %this-->fileNameField.getText() );
      if( %defaultPath $= "" )
         %defaultPath = getMainDotCsDir();
      else
         %defaultPath = makeFullPath( %defaultPath );
   }

   %dlg = new OpenFolderDialog()
   {
      Title       = "Select Media Path...";
      DefaultPath = %defaultPath;
      MustExit    = true;
      ChangePath  = false;
   };
   
   %ret = %dlg.execute();
   if( %ret )
      %file = %dlg.fileName;
   
   %dlg.delete();
   
   if( !%ret )
      return;
      
   %file = makeRelativePath( %file, getMainDotCsDir() );
   %this-->mediaPathField.setText( %file );
}
