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

beginModule( 'physX3' );

   // Look for the optional global from the project.conf.
   global $PHYSX3_SDK_PATH;
   if (!$PHYSX3_SDK_PATH)
   {
      // First look for an environment var.
      $PHYSX3_SDK_PATH = getenv( "TORQUE_PHYSX3_PATH" );

      if (strlen($PHYSX3_SDK_PATH) == 0 || !file_exists($PHYSX3_SDK_PATH))
      {
         // Sometimes users get confused and use this var.
         $PHYSX3_SDK_PATH = getenv( "PHYSX3_SDK_PATH" );

         if (strlen($PHYSX3_SDK_PATH) == 0 || !file_exists($PHYSX3_SDK_PATH))
         {
            // Since neither environment var worked try for 
            $PHYSX3_SDK_PATH = getenv("ProgramFiles") . "/NVIDIA Corporation/NVIDIA PhysX SDK/v3.3.0_win";

            // Last channce... try the x86 program files
            if (!file_exists($PHYSX_SDK_PATH))
               $PHYSX3_SDK_PATH = getenv("ProgramFiles(x86)") . "/NVIDIA Corporation/NVIDIA PhysX SDK/v3.3.0_win";
         }
      }

      // We need forward slashes for paths.
      $PHYSX3_SDK_PATH = str_replace( "\\", "/", $PHYSX3_SDK_PATH);

      // Remove trailing slashes.
      $PHYSX3_SDK_PATH = rtrim($PHYSX3_SDK_PATH, " /");
   }

   // If we still don't have the SDK path then let the user know.
   if (!file_exists($PHYSX3_SDK_PATH))
   {
      trigger_error( 
            "\n*******************************************************************".
            "\n".
            "\n  We were not able to find a valid path to the PhysX 3 SDK!".
            "\n".
            "\n  You must install the latest PhysX3 SDK and set the path via a".
            "\n  \$PHYSX3_SDK_PATH variable in your buildFiles/project.conf file".
            "\n  or by setting the TORQUE_PHYSX3_PATH system environment variable".
            "\n  (may require a reboot).".
            "\n".
            "\n*******************************************************************".
            "\n", E_USER_ERROR );
   }

   addProjectDefine( "TORQUE_PHYSICS_PHYSX3" );
   addProjectDefine( "TORQUE_PHYSICS_ENABLED" );

   // Source
   addEngineSrcDir( "T3D/physics/physx3" );
   addEngineSrcDir( "T3D/physics/physx3/Cloth" );
   addEngineSrcDir( "T3D/physics/physx3/Particles" );

   // Includes
   addIncludePath( $PHYSX3_SDK_PATH . "/Include" );
   addIncludePath( $PHYSX3_SDK_PATH . "/Include/extensions" );
   addIncludePath( $PHYSX3_SDK_PATH . "/Include/foundation" );
   addIncludePath( $PHYSX3_SDK_PATH . "/Include/characterkinematic" );
   addIncludePath( $PHYSX3_SDK_PATH . "/Include/common" );
      
   // Libs
   addProjectLibDir( $PHYSX3_SDK_PATH . "/Lib/win32" );
   addProjectLibInput( "PhysX3_x86.lib","PhysX3CHECKED_x86.lib" );
   addProjectLibInput( "PhysX3Common_x86.lib","PhysX3CommonCHECKED_x86.lib");
   addProjectLibInput( "PhysX3Extensions.lib","PhysX3ExtensionsCHECKED.lib" );
   addProjectLibInput( "PhysX3Cooking_x86.lib","PhysX3CookingCHECKED_x86.lib");
   addProjectLibInput( "PxTask.lib","PxTaskCHECKED.lib" );
   addProjectLibInput( "PhysX3CharacterKinematic_x86.lib","PhysX3CharacterKinematicCHECKED_x86.lib" );
   addProjectLibInput( "PhysXVisualDebuggerSDK.lib", "PhysXVisualDebuggerSDKCHECKED.lib");
   addProjectLibInput( "PhysXProfileSDK.lib", "PhysXProfileSDKCHECKED.lib");
          
   // File Copy
   copyFileToProject( $PHYSX3_SDK_PATH . "/Bin/win32/nvToolsExt32_1.dll", "/game/nvToolsExt32_1.dll" );
   copyFileToProject( $PHYSX3_SDK_PATH . "/Bin/win32/PhysX3_x86.dll",  "/game/PhysX3_x86.dll" );
   copyFileToProject( $PHYSX3_SDK_PATH . "/Bin/win32/PhysX3CharacterKinematic_x86.dll",     "/game/PhysX3CharacterKinematic_x86.dll" );
   copyFileToProject( $PHYSX3_SDK_PATH . "/Bin/win32/PhysX3CharacterKinematicCHECKED_x86.dll",   "/game/PhysX3CharacterKinematicCHECKED_x86.dll" );
   copyFileToProject( $PHYSX3_SDK_PATH . "/Bin/win32/PhysX3CHECKED_x86.dll",   "/game/PhysX3CHECKED_x86.dll" );
   copyFileToProject( $PHYSX3_SDK_PATH . "/Bin/win32/PhysX3Common_x86.dll",   "/game/PhysX3Common_x86.dll" );
   copyFileToProject( $PHYSX3_SDK_PATH . "/Bin/win32/PhysX3CommonCHECKED_x86.dll",   "/game/PhysX3CommonCHECKED_x86.dll" );
   copyFileToProject( $PHYSX3_SDK_PATH . "/Bin/win32/PhysX3Cooking_x86.dll",   "/game/PhysX3Cooking_x86.dll" );
   copyFileToProject( $PHYSX3_SDK_PATH . "/Bin/win32/PhysX3CookingCHECKED_x86.dll",   "/game/PhysX3CookingCHECKED_x86.dll" );

endModule();

?>
