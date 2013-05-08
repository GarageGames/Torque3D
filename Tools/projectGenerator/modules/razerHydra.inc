<?php
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

beginModule( 'razerHydra' );

   // Look for the optional global from the project.conf.
   global $RAZERHYDRA_SDK_PATH;
   if (!$RAZERHYDRA_SDK_PATH)
   {
      // First look for an environment var.
      $RAZERHYDRA_SDK_PATH = getenv( "TORQUE_RAZERHYDRA_PATH" );

      if (strlen($RAZERHYDRA_SDK_PATH) == 0 || !file_exists($RAZERHYDRA_SDK_PATH))
      {
         // Sometimes users get confused and use this var.
         $RAZERHYDRA_SDK_PATH = getenv( "RAZERHYDRA_SDK_PATH" );
      }

      // We need forward slashes for paths.
      $RAZERHYDRA_SDK_PATH = str_replace( "\\", "/", $RAZERHYDRA_SDK_PATH);

      // Remove trailing slashes.
      $RAZERHYDRA_SDK_PATH = rtrim($RAZERHYDRA_SDK_PATH, " /");
   }

   // If we still don't have the SDK path then let the user know.
   if (!file_exists($RAZERHYDRA_SDK_PATH))
   {
      trigger_error( 
            "\n*******************************************************************".
            "\n".
            "\n  We were not able to find a valid path to the Razer Hydra SDK!".
            "\n".
            "\n  You must install the latest Sixense SDK and set the path via a".
            "\n  \$RAZERHYDRA_SDK_PATH variable in your buildFiles/project.conf file".
            "\n  or by setting the TORQUE_RAZERHYDRA_PATH system environment variable".
            "\n  (may require a reboot).".
            "\n".
            "\n*******************************************************************".
            "\n", E_USER_ERROR );
   }

   // Only Windows is supported at this time
   if ( Generator::$platform == "win32" )
   {
      // Source
      addEngineSrcDir( "platform/input/razerHydra" );

      // Includes
      addIncludePath( $RAZERHYDRA_SDK_PATH . "/include" );
      
      // File Copy for Release
      copyFileToProject( $RAZERHYDRA_SDK_PATH . "/bin/win32/release_dll/sixense.dll",  "/game/sixense.dll" );

      // File Copy for Debug
      copyFileToProject( $RAZERHYDRA_SDK_PATH . "/bin/win32/debug_dll/sixensed.dll",   "/game/sixensed.dll" );
      copyFileToProject( $RAZERHYDRA_SDK_PATH . "/samples/win32/sixense_simple3d/DeviceDLL.dll", "/game/DeviceDLL.dll" ); // Only needed by the debug sixense library
   }

endModule();

?>
