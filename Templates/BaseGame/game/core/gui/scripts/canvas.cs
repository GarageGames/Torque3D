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

function createCanvas(%windowTitle)
{
   if ($isDedicated)
   {
      GFXInit::createNullDevice();
      return true;
   }
   
   // Create the Canvas
   $GameCanvas = new GuiCanvas(Canvas)
   {
      displayWindow = $platform !$= "windows";
   };

   // Set the window title
   if (isObject(Canvas)) 
   {
      Canvas.setWindowTitle(%windowTitle @ " - " @ $pref::Video::displayDevice);
      configureCanvas();
   } 
   else 
   {
      error("Canvas creation failed. Shutting down.");
      quit();
   }
}

// Constants for referencing video resolution preferences
$WORD::RES_X = 0;
$WORD::RES_Y = 1;
$WORD::FULLSCREEN = 2;
$WORD::BITDEPTH = 3;
$WORD::REFRESH = 4;
$WORD::AA = 5;

function configureCanvas()
{
   // Setup a good default if we don't have one already.
   if ($pref::Video::Resolution $= "")
      $pref::Video::Resolution = "800 600";
   if ($pref::Video::FullScreen $= "")
      $pref::Video::FullScreen = false;
   if ($pref::Video::BitDepth $= "")
      $pref::Video::BitDepth = "32";
   if ($pref::Video::RefreshRate $= "")
      $pref::Video::RefreshRate = "60";
   if ($pref::Video::AA $= "")
      $pref::Video::AA = "4";

   %resX = $pref::Video::Resolution.x;
   %resY = $pref::Video::Resolution.y;
   %fs = $pref::Video::FullScreen;
   %bpp = $pref::Video::BitDepth;
   %rate = $pref::Video::RefreshRate;
   %aa = $pref::Video::AA;
   
   if($cliFullscreen !$= "") {
      %fs = $cliFullscreen;
      $cliFullscreen = "";
   }
   
   echo("--------------");
   echo("Attempting to set resolution to \"" @ %resX SPC %resY SPC %fs SPC %bpp SPC %rate SPC %aa @ "\"");
   
   %deskRes    = getDesktopResolution();      
   %deskResX   = getWord(%deskRes, $WORD::RES_X);
   %deskResY   = getWord(%deskRes, $WORD::RES_Y);
   %deskResBPP = getWord(%deskRes, 2);
   
   // We shouldn't be getting this any more but just in case...
   if (%bpp $= "Default")
      %bpp = %deskResBPP;
      
   // Make sure we are running at a valid resolution
   if (%fs $= "0" || %fs $= "false")
   {
      // Windowed mode has to use the same bit depth as the desktop
      %bpp = %deskResBPP;
      
      // Windowed mode also has to run at a smaller resolution than the desktop
      if ((%resX >= %deskResX) || (%resY >= %deskResY))
      {
         warn("Warning: The requested windowed resolution is equal to or larger than the current desktop resolution. Attempting to find a better resolution");
      
         %resCount = Canvas.getModeCount();
         for (%i = (%resCount - 1); %i >= 0; %i--)
         {
            %testRes = Canvas.getMode(%i);
            %testResX = getWord(%testRes, $WORD::RES_X);
            %testResY = getWord(%testRes, $WORD::RES_Y);
            %testBPP  = getWord(%testRes, $WORD::BITDEPTH);

            if (%testBPP != %bpp)
               continue;
            
            if ((%testResX < %deskResX) && (%testResY < %deskResY))
            {
               // This will work as our new resolution
               %resX = %testResX;
               %resY = %testResY;
               
               warn("Warning: Switching to \"" @ %resX SPC %resY SPC %bpp @ "\"");
               
               break;
            }
         }
      }
   }
   
   $pref::Video::Resolution = %resX SPC %resY;
   $pref::Video::FullScreen = %fs;
   $pref::Video::BitDepth = %bpp;
   $pref::Video::RefreshRate = %rate;
   $pref::Video::AA = %aa;
   
   if (%fs == 1 || %fs $= "true")
      %fsLabel = "Yes";
   else
      %fsLabel = "No";

   echo("Accepted Mode: " NL
      "--Resolution : " @  %resX SPC %resY NL 
      "--Full Screen : " @ %fsLabel NL
      "--Bits Per Pixel : " @ %bpp NL
      "--Refresh Rate : " @ %rate NL
      "--AA TypeXLevel : " @ %aa NL
      "--------------");

   // Actually set the new video mode
   Canvas.setVideoMode(%resX, %resY, %fs, %bpp, %rate, %aa);

   commandToServer('setClientAspectRatio', %resX, %resY);

   // AA piggybacks on the AA setting in $pref::Video::mode.
   // We need to parse the setting between AA modes, and then it's level
   // It's formatted as AATypexAALevel
   // So, FXAAx4 or MLAAx2
   if ( isObject( FXAA_PostEffect ) )
      FXAA_PostEffect.isEnabled = ( %aa > 0 ) ? true : false;
}
