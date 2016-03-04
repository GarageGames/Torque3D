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

// Only load these functions if an Oculus VR device is present
if(!isFunction(isOculusVRDeviceActive))
   return;

function setupOculusActionMaps()
{
   if (isObject(OculusWarningMap))
      return;

   new ActionMap(OculusWarningMap);
   new ActionMap(OculusCanvasMap);

   OculusWarningMap.bind(keyboard, space, dismissOculusVRWarnings);

   OculusCanvasMap.bind( mouse, xaxis, oculusYaw );
   OculusCanvasMap.bind( mouse, yaxis, oculusPitch );
   OculusCanvasMap.bind( mouse, button0, oculusClick );
}

function oculusYaw(%val)
{
   OculusCanvas.cursorNudge(%val * 0.10, 0);
}

function oculusPitch(%val)
{
   OculusCanvas.cursorNudge(0, %val * 0.10);
}

function oculusClick(%active)
{
   OculusCanvas.cursorClick(0, %active);  
}

function GuiOffscreenCanvas::checkCursor(%this)
{
   %count = %this.getCount();
   for(%i = 0; %i < %count; %i++)
   {
      %control = %this.getObject(%i);
      if ((%control.noCursor $= "") || !%control.noCursor)
      {
         %this.cursorOn();
         return true;
      }
   }
   // If we get here, every control requested a hidden cursor, so we oblige.

   %this.cursorOff();
   return false;
}

function GuiOffscreenCanvas::pushDialog(%this, %ctrl, %layer, %center)
{
   Parent::pushDialog(%this, %ctrl, %layer, %center);
   %cursorVisible = %this.checkCursor();

   if (%cursorVisible)
   {
      echo("OffscreenCanvas visible");
      OculusCanvasMap.pop();
      OculusCanvasMap.push();
   }
   else
   {
      echo("OffscreenCanvas not visible");
      OculusCanvasMap.pop();
   }
}

function GuiOffscreenCanvas::popDialog(%this, %ctrl)
{
   Parent::popDialog(%this, %ctrl);
   %cursorVisible = %this.checkCursor();

   if (%cursorVisible)
   {
      echo("OffscreenCanvas visible");
      OculusCanvasMap.pop();
      OculusCanvasMap.push();
   }
   else
   {
      echo("OffscreenCanvas not visible");
      OculusCanvasMap.pop();
   }
}


//-----------------------------------------------------------------------------

function oculusSensorMetricsCallback()
{
   return ovrDumpMetrics(0);
}


//-----------------------------------------------------------------------------
function onOculusStatusUpdate(%status)
{
   $LastOculusTrackingState = %status;
}

//-----------------------------------------------------------------------------

// Call this function from createCanvas() to have the Canvas attach itself
// to the Rift's display.  The Canvas' window will still open on the primary
// display if that is different from the Rift, but it will move to the Rift
// when it goes full screen.  If the Rift is not connected then nothing
// will happen.
function pointCanvasToOculusVRDisplay()
{
   $pref::Video::displayOutputDevice = getOVRHMDDisplayDeviceName(0);
}

//-----------------------------------------------------------------------------

// Call this function from GameConnection::initialControlSet() just before
// your "Canvas.setContent(PlayGui);" call, or at any time you wish to switch
// to a side-by-side rendering and the appropriate barrel distortion.  This
// will turn on side-by-side rendering and tell the GameConnection to use the
// Rift as its display device.
// Parameters:
// %gameConnection - The client GameConnection instance
// %trueStereoRendering - If true will enable stereo rendering with an eye
// offset for each viewport.  This will render each frame twice.  If false
// then a pseudo stereo rendering is done with only a single render per frame.
function enableOculusVRDisplay(%gameConnection, %trueStereoRendering)
{
   setOVRHMDAsGameConnectionDisplayDevice(%gameConnection);
   PlayGui.renderStyle = "stereo side by side";
   setOptimalOVRCanvasSize(Canvas);

   if (!isObject(OculusCanvas))
   {
      new GuiOffscreenCanvas(OculusCanvas) {
         targetSize = "512 512";
         targetName = "oculusCanvas";
         dynamicTarget = true;
      };
   }

   if (!isObject(OculusVROverlay))
   {
      exec("./oculusVROverlay.gui");
   }

   OculusCanvas.setContent(OculusVROverlay);
   OculusCanvas.setCursor(DefaultCursor);
   PlayGui.setStereoGui(OculusCanvas);
   OculusCanvas.setCursorPos("128 128");
   OculusCanvas.cursorOff();
   $GameCanvas = OculusCanvas;

   %ext = Canvas.getExtent();
   $OculusMouseScaleX = 512.0 / 1920.0;
   $OculusMouseScaleY = 512.0 / 1060.0;
   
   //$gfx::wireframe = true;
   // Reset all sensors
   ovrResetAllSensors();
}

// Call this function when ever you wish to turn off the stereo rendering
// and barrel distortion for the Rift.
function disableOculusVRDisplay(%gameConnection)
{
   OculusCanvas.popDialog();
   OculusWarningMap.pop();
   $GameCanvas = Canvas;

   if (isObject(gameConnection))
   {
      %gameConnection.clearDisplayDevice();
   }
   PlayGui.renderStyle = "standard";
}

// Helper function to set the standard Rift control scheme.  You could place
// this function in GameConnection::initialControlSet() at the same time
// you call enableOculusVRDisplay().
function setStandardOculusVRControlScheme(%gameConnection)
{
   if($OculusVR::SimulateInput)
   {
      // We are simulating a HMD so allow the mouse and gamepad to control
      // both yaw and pitch.
      %gameConnection.setControlSchemeParameters(true, true, true);
   }
   else
   {
      // A HMD is connected so have the mouse and gamepad only add to yaw
      %gameConnection.setControlSchemeParameters(true, true, false);
   }
}

//-----------------------------------------------------------------------------

// Helper function to set the resolution for the Rift.
// Parameters:
// %fullscreen - If true then the display will be forced to full screen.  If
// pointCanvasToOculusVRDisplay() was called before the Canvas was created, then
// the full screen display will appear on the Rift.
function setVideoModeForOculusVRDisplay(%fullscreen)
{
   %res = getOVRHMDResolution(0);
   Canvas.setVideoMode(%res.x, %res.y, %fullscreen, 32, 4);
}

//-----------------------------------------------------------------------------

// Reset all Oculus Rift sensors.  This will make the Rift's current heading
// be considered the origin.
function resetOculusVRSensors()
{
   ovrResetAllSensors();
}

function dismissOculusVRWarnings(%value)
{
   //if (%value)
   //{
      ovrDismissWarnings();
      OculusWarningMap.pop();
   //}
}