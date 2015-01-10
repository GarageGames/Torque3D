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

function initializeEditorClasses()
{
   echo(" % - Initializing Tools Base");


   $EditorClassesGroup = "EditorClassesCleanup";
   if( !isObject( $EditorClassesGroup ) )
      new SimGroup( $EditorClassesGroup );


   //-----------------------------------------------------------------------------
   // Load Editor Profiles
   //-----------------------------------------------------------------------------
   
   exec("./scripts/fileLoader.ed.cs");   
   
   loadDirectory( expandFilename("./gui/panels") );
   

   //-----------------------------------------------------------------------------
   // Setup Preferences Manager
   //-----------------------------------------------------------------------------
   
   exec("./scripts/preferencesManager.ed.cs");
   initPreferencesManager();
   
   //-----------------------------------------------------------------------------
   // Load Form Managers
   //-----------------------------------------------------------------------------
   
   exec("./scripts/guiFormLibraryManager.ed.cs");
   exec("./scripts/guiFormContentManager.ed.cs");
   exec("./scripts/guiFormReferenceManager.ed.cs");
   exec("./scripts/guiFormLayoutManager.ed.cs");
   exec("./scripts/guiFormMessageManager.ed.cs");
   exec("./scripts/expandos.ed.cs");
   exec("./scripts/utility.ed.cs");
   setupBaseExpandos();

   // User Display
   exec("./scripts/contextPopup.ed.cs");

   // Project Support   
   exec("./scripts/projects/projectEvents.ed.cs");
   exec("./scripts/projects/projectInternalInterface.ed.cs");
   
   // Input
   exec("./scripts/input/inputEvents.ed.cs");
   exec("./scripts/input/dragDropEvents.ed.cs");
   exec("./scripts/input/applicationEvents.ed.cs");
   
   // Form Class
   exec("./scripts/guiFormClass.ed.cs");
   exec("./scripts/guiClasses/guiThumbnailPopup.ed.cs");
   exec("./scripts/guiClasses/guiThumbnail.ed.cs");
   exec("./scripts/RSSNews/RSSFeedScript.ed.cs");

   loadDirectory( expandFilename("./scripts/core") );
   loadDirectory( expandFilename("./scripts/platform") );
}

function destroyEditorClasses()
{
}
