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

beginModule( 'physX' );

   // Look for the optional global from the project.conf.
   global $PHYSX_SDK_PATH;
   if (!$PHYSX_SDK_PATH)
   {
      // First look for an environment var.
      $PHYSX_SDK_PATH = getenv( "TORQUE_PHYSX_PATH" );

      if (strlen($PHYSX_SDK_PATH) == 0 || !file_exists($PHYSX_SDK_PATH))
      {
         // Sometimes users get confused and use this var.
         $PHYSX_SDK_PATH = getenv( "PHYSX_SDK_PATH" );

         if (strlen($PHYSX_SDK_PATH) == 0 || !file_exists($PHYSX_SDK_PATH))
         {
            // Since neither environment var worked try for 
            // the default PhysX SDK install location.
            $PHYSX_SDK_PATH = getenv("ProgramFiles") . "/NVIDIA Corporation/NVIDIA PhysX SDK/v2.8.4_win";

            // Last channce... try the x86 default install path.
            if (!file_exists($PHYSX_SDK_PATH))
               $PHYSX_SDK_PATH = getenv("ProgramFiles(x86)") . "/NVIDIA Corporation/NVIDIA PhysX SDK/v2.8.4_win";
         }
      }

      // We need forward slashes for paths.
      $PHYSX_SDK_PATH = str_replace( "\\", "/", $PHYSX_SDK_PATH);

      // Remove trailing slashes.
      $PHYSX_SDK_PATH = rtrim($PHYSX_SDK_PATH, " /");
   }

   // If we still don't have the SDK path then let the user know.
   if (!file_exists($PHYSX_SDK_PATH))
   {
      trigger_error( 
            "\n*******************************************************************".
            "\n".
            "\n  We were not able to find a valid path to the PhysX SDK!".
            "\n".
            "\n  You must install the latest PhysX SDK and set the path via a".
            "\n  \$PHYSX_SDK_PATH variable in your buildFiles/project.conf file".
            "\n  or by setting the TORQUE_PHYSX_PATH system environment variable".
            "\n  (may require a reboot).".
            "\n".
            "\n*******************************************************************".
            "\n", E_USER_ERROR );
   }

   addProjectDefine( "TORQUE_PHYSICS_PHYSX" );
   addProjectDefine( "TORQUE_PHYSICS_ENABLED" );

   // Source
   addEngineSrcDir( "T3D/physics/physX" );

   // Includes
   addIncludePath( $PHYSX_SDK_PATH . "/SDKs/Physics/include" );
   addIncludePath( $PHYSX_SDK_PATH . "/SDKs/Foundation/include" );
   addIncludePath( $PHYSX_SDK_PATH . "/SDKs/PhysXLoader/include" );
   addIncludePath( $PHYSX_SDK_PATH . "/SDKs/Cooking/include" );
   addIncludePath( $PHYSX_SDK_PATH . "/SDKs/NxCharacter/include" );
   addIncludePath( $PHYSX_SDK_PATH . "/Tools/NxuStream2" );

   // Libs
   addProjectLibDir( $PHYSX_SDK_PATH . "/SDKs/lib/Win32" );
   addProjectLibInput( "PhysXCooking.lib" );
   addProjectLibInput( "PhysXLoader.lib" );
          
   // File Copy
   copyFileToProject( $PHYSX_SDK_PATH . "/Bin/win32/cudart32_30_9.dll", "/game/cudart32_30_9.dll" );
   copyFileToProject( $PHYSX_SDK_PATH . "/Bin/win32/PhysXCooking.dll",  "/game/PhysXCooking.dll" );
   copyFileToProject( $PHYSX_SDK_PATH . "/Bin/win32/PhysXCore.dll",     "/game/PhysXCore.dll" );
   copyFileToProject( $PHYSX_SDK_PATH . "/Bin/win32/PhysXDevice.dll",   "/game/PhysXDevice.dll" );
   copyFileToProject( $PHYSX_SDK_PATH . "/Bin/win32/PhysXLoader.dll",   "/game/PhysXLoader.dll" );

   // For PhysX support.
   includeLib( 'nxCharacter' );
   includeLib( 'nxuStream' );

   if (inProjectConfig())
   {
      addProjectDependency( 'nxCharacter' );
      addProjectDependency( 'nxuStream' );
      addSolutionProjectRef( 'nxCharacter' );
      addSolutionProjectRef( 'nxuStream' );
   }

endModule();

?>
