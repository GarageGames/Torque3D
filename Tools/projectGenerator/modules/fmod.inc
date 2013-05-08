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

beginModule( 'fmod' );
  $allgood = true;

	// Additional includes
	switch( Generator::$platform )
   {
      case "win32":
      case "mac":
        //addLibIncludePath( 'fmod/inc' );
        // Look for the optional global from the project.conf.
        global $FMOD_SDK_PATH;
        if (!$FMOD_SDK_PATH)
        {
          // First look for an environment var.
          $FMOD_SDK_PATH = getenv( "TORQUE_FMOD_PATH" );
          
          if (strlen($FMOD_SDK_PATH) == 0 || !file_exists($FMOD_SDK_PATH))
          {
            // Check for another environment variable.  There is no standard one for FMOD.
            $FMOD_SDK_PATH = getenv( "FMOD_SDK_PATH" );

            if (strlen($FMOD_SDK_PATH) == 0 || !file_exists($FMOD_SDK_PATH))
            {
              // No environment variables present so check the FMOD SDK install location
              $FMOD_SDK_PATH = getenv("ProgramFiles") . "/FMOD SoundSystem/FMOD Programmers API Windows";
                
              // Last channce... try the x86 default install path.
              if (!file_exists($FMOD_SDK_PATH))
                $FMOD_SDK_PATH = getenv("ProgramFiles(x86)") . "/FMOD SoundSystem/FMOD Programmers API Windows";
            }
          }

          // We need forward slashes for paths.
          $FMOD_SDK_PATH = str_replace( "\\", "/", $FMOD_SDK_PATH);

          // Remove trailing slashes.
          $FMOD_SDK_PATH = rtrim($FMOD_SDK_PATH, " /");
        }
        
        if (file_exists($FMOD_SDK_PATH))
        {
          addIncludePath( $FMOD_SDK_PATH . "/api/inc" );
          addIncludePath( $FMOD_SDK_PATH . "/fmoddesignerapi/api/inc" );
          
          // File Copy
          copyFileToProject( $FMOD_SDK_PATH . "/api/fmodex.dll",                      "/game/fmodex.dll" );
          copyFileToProject( $FMOD_SDK_PATH . "/fmoddesignerapi/api/fmod_event.dll",  "/game/fmod_event.dll" );
        }
        else
        {
echo "\n\n*** FMOD PATH NOT VALID \n\n";
          $allgood = false;
        }

      break;
        
      case "360":
        // Assume 32-bit system
        $FMOD_PATH = "C:/Program Files/FMOD SoundSystem/FMOD Programmers API Xbox 360/";
        
        // Ohnoes! Try 64-bit!
        if (!file_exists($FMOD_PATH))
          $FMOD_PATH = "C:/Program Files (x86)/FMOD SoundSystem/FMOD Programmers API Xbox 360/";
        
        if (!file_exists($FMOD_PATH))
        {
           $allgood = false;
           echo "\n\n FMOD integration failed to find Xbox360 installation!\n";
           echo "   - FMOD_PATH is set to $FMOD_PATH\n\n"; 
        }
        else
        {
          echo "\n\nFMOD integration for Torque3D on the Xbox360\n";
          echo "   - FMOD_PATH is set to $FMOD_PATH\n\n";
          
          // Fmod API
          addIncludePath($FMOD_PATH . 'api/inc');
          addProjectLibDir($FMOD_PATH . 'api/lib');
          
          // Fmod Designer API
          addIncludePath($FMOD_PATH . 'fmoddesignerapi/api/inc');
          addProjectLibDir($FMOD_PATH . 'fmoddesignerapi/api/lib');
          
          addProjectLibInput('fmodxbox360.lib');
          
          addProjectLibInput('xmahal.lib');
          addProjectLibInput('xmic.lib');
          
          // Prior to June 2009 XDK, FMOD requires xaudio.lib
          //addProjectLibInput('xaudio.lib');
        }
        break;
   }
   
  // Source
  if($allgood)
	  addEngineSrcDir('sfx/fmod');
endModule();
?>