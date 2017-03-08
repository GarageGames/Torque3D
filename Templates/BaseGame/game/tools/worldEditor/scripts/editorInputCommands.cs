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

//------------------------------------------------------------------------------
// Non-remapable binds
//------------------------------------------------------------------------------
function escapeFromGame()
{
   disconnect();
}

//------------------------------------------------------------------------------
// Movement Keys
//------------------------------------------------------------------------------
function editorMoveleft(%val)
{
   $mvLeftAction = %val * $Camera::movementSpeed;
}

function editorMoveright(%val)
{
   $mvRightAction = %val * $Camera::movementSpeed;
}

function editorMoveforward(%val)
{
   $mvForwardAction = %val * $Camera::movementSpeed;
}

function editorMovebackward(%val)
{
   $mvBackwardAction = %val * $Camera::movementSpeed;
}

function editorMoveup(%val)
{
   %object = ServerConnection.getControlObject();
   
   if(%object.isInNamespaceHierarchy("Camera"))
      $mvUpAction = %val * $Camera::movementSpeed;
}

function editorMovedown(%val)
{
   %object = ServerConnection.getControlObject();
   
   if(%object.isInNamespaceHierarchy("Camera"))
      $mvDownAction = %val * $Camera::movementSpeed;
}

function getEditorMouseAdjustAmount(%val)
{
   return(%val * ($cameraFov / 90) * 0.01);
}

function editorYaw(%val)
{
   %yawAdj = getEditorMouseAdjustAmount(%val);
   if(ServerConnection.isControlObjectRotDampedCamera())
   {
      // Clamp and scale
      %yawAdj = mClamp(%yawAdj, -m2Pi()+0.01, m2Pi()-0.01);
      %yawAdj *= 0.5;
   }

   $mvYaw += %yawAdj;
}

function editorPitch(%val)
{
   %pitchAdj = getEditorMouseAdjustAmount(%val);
   if(ServerConnection.isControlObjectRotDampedCamera())
   {
      // Clamp and scale
      %pitchAdj = mClamp(%pitchAdj, -m2Pi()+0.01, m2Pi()-0.01);
      %pitchAdj *= 0.5;
   }

   $mvPitch += %pitchAdj;
}

//------------------------------------------------------------------------------
// Mouse Trigger
//------------------------------------------------------------------------------
function editorClick(%val)
{
   $mvTriggerCount0++;
}

function editorRClick(%val)
{
   $mvTriggerCount1++;
}

//------------------------------------------------------------------------------
// Camera & View functions
//------------------------------------------------------------------------------
function toggleCamera(%val)
{
   if (%val)
      commandToServer('ToggleCamera');
}

//------------------------------------------------------------------------------
// Helper Functions
//------------------------------------------------------------------------------
function dropCameraAtPlayer(%val)
{
   if (%val)
      commandToServer('dropCameraAtPlayer');
}

function dropPlayerAtCamera(%val)
{
   if (%val)
      commandToServer('DropPlayerAtCamera');
}

//------------------------------------------------------------------------------
// Debugging Functions
//------------------------------------------------------------------------------
function showMetrics(%val)
{
   if(%val)
   {
      if(!Canvas.isMember(FrameOverlayGui))
         metrics("fps gfx shadow sfx terrain groundcover forest net");
      else
         metrics("");
   }
}

//------------------------------------------------------------------------------
//
// Start profiler by pressing ctrl f3
// ctrl f3 - starts profile that will dump to console and file
//
function doProfile(%val)
{
   if (%val)
   {
      // key down -- start profile
      echo("Starting profile session...");
      profilerReset();
      profilerEnable(true);
   }
   else
   {
      // key up -- finish off profile
      echo("Ending profile session...");

      profilerDumpToFile("profilerDumpToFile" @ getSimTime() @ ".txt");
      profilerEnable(false);
   }
}

function editorWheelFadeScroll( %val )
{
   EWorldEditor.fadeIconsDist += %val * 0.1;
   if( EWorldEditor.fadeIconsDist < 0 )
      EWorldEditor.fadeIconsDist = 0;
}