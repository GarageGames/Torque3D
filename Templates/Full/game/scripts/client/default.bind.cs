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

if ( isObject( moveMap ) )
   moveMap.delete();
new ActionMap(moveMap);


//------------------------------------------------------------------------------
// Non-remapable binds
//------------------------------------------------------------------------------

function escapeFromGame()
{
   if ( $Server::ServerType $= "SinglePlayer" )
      MessageBoxYesNo( "Exit", "Exit from this Mission?", "disconnect();", "");
   else
      MessageBoxYesNo( "Disconnect", "Disconnect from the server?", "disconnect();", "");
}

moveMap.bindCmd(keyboard, "escape", "", "handleEscape();");

function showPlayerList(%val)
{
   if (%val)
      PlayerListGui.toggle();
}

moveMap.bind( keyboard, F2, showPlayerList );

function hideHUDs(%val)
{
   if (%val)
      HudlessPlayGui.toggle();
}

moveMap.bind(keyboard, "ctrl h", hideHUDs);

function doScreenShotHudless(%val)
{
   if(%val)
   {
      canvas.setContent(HudlessPlayGui);
      //doScreenshot(%val);
      schedule(10, 0, "doScreenShot", %val);
   }
   else
      canvas.setContent(PlayGui);
}

moveMap.bind(keyboard, "alt p", doScreenShotHudless);

//------------------------------------------------------------------------------
// Movement Keys
//------------------------------------------------------------------------------

$movementSpeed = 1; // m/s

function setSpeed(%speed)
{
   if(%speed)
      $movementSpeed = %speed;
}

function moveleft(%val)
{
   $mvLeftAction = %val * $movementSpeed;
}

function moveright(%val)
{
   $mvRightAction = %val * $movementSpeed;
}

function moveforward(%val)
{
   $mvForwardAction = %val * $movementSpeed;
}

function movebackward(%val)
{
   $mvBackwardAction = %val * $movementSpeed;
}

function moveup(%val)
{
   %object = ServerConnection.getControlObject();
   
   if(%object.isInNamespaceHierarchy("Camera"))
      $mvUpAction = %val * $movementSpeed;
}

function movedown(%val)
{
   %object = ServerConnection.getControlObject();
   
   if(%object.isInNamespaceHierarchy("Camera"))
      $mvDownAction = %val * $movementSpeed;
}

function turnLeft( %val )
{
   $mvYawRightSpeed = %val ? $Pref::Input::KeyboardTurnSpeed : 0;
}

function turnRight( %val )
{
   $mvYawLeftSpeed = %val ? $Pref::Input::KeyboardTurnSpeed : 0;
}

function panUp( %val )
{
   $mvPitchDownSpeed = %val ? $Pref::Input::KeyboardTurnSpeed : 0;
}

function panDown( %val )
{
   $mvPitchUpSpeed = %val ? $Pref::Input::KeyboardTurnSpeed : 0;
}

function getMouseAdjustAmount(%val)
{
   // based on a default camera FOV of 90'
   return(%val * ($cameraFov / 90) * 0.01) * $pref::Input::LinkMouseSensitivity;
}

function getGamepadAdjustAmount(%val)
{
   // based on a default camera FOV of 90'
   return(%val * ($cameraFov / 90) * 0.01) * 10.0;
}

function yaw(%val)
{
   %yawAdj = getMouseAdjustAmount(%val);
   if(ServerConnection.isControlObjectRotDampedCamera())
   {
      // Clamp and scale
      %yawAdj = mClamp(%yawAdj, -m2Pi()+0.01, m2Pi()-0.01);
      %yawAdj *= 0.5;
   }

   $mvYaw += %yawAdj;
}

function pitch(%val)
{
   %pitchAdj = getMouseAdjustAmount(%val);
   if(ServerConnection.isControlObjectRotDampedCamera())
   {
      // Clamp and scale
      %pitchAdj = mClamp(%pitchAdj, -m2Pi()+0.01, m2Pi()-0.01);
      %pitchAdj *= 0.5;
   }

   $mvPitch += %pitchAdj;
}

function jump(%val)
{
   $mvTriggerCount2++;
}

function gamePadMoveX( %val )
{
   if(%val > 0)
   {
      $mvRightAction = %val * $movementSpeed;
      $mvLeftAction = 0;
   }
   else
   {
      $mvRightAction = 0;
      $mvLeftAction = -%val * $movementSpeed;
   }
}

function gamePadMoveY( %val )
{
   if(%val > 0)
   {
      $mvForwardAction = %val * $movementSpeed;
      $mvBackwardAction = 0;
   }
   else
   {
      $mvForwardAction = 0;
      $mvBackwardAction = -%val * $movementSpeed;
   }
}

function gamepadYaw(%val)
{
   %yawAdj = getGamepadAdjustAmount(%val);
   if(ServerConnection.isControlObjectRotDampedCamera())
   {
      // Clamp and scale
      %yawAdj = mClamp(%yawAdj, -m2Pi()+0.01, m2Pi()-0.01);
      %yawAdj *= 0.5;
   }

   if(%yawAdj > 0)
   {
      $mvYawLeftSpeed = %yawAdj;
      $mvYawRightSpeed = 0;
   }
   else
   {
      $mvYawLeftSpeed = 0;
      $mvYawRightSpeed = -%yawAdj;
   }
}

function gamepadPitch(%val)
{
   %pitchAdj = getGamepadAdjustAmount(%val);
   if(ServerConnection.isControlObjectRotDampedCamera())
   {
      // Clamp and scale
      %pitchAdj = mClamp(%pitchAdj, -m2Pi()+0.01, m2Pi()-0.01);
      %pitchAdj *= 0.5;
   }

   if(%pitchAdj > 0)
   {
      $mvPitchDownSpeed = %pitchAdj;
      $mvPitchUpSpeed = 0;
   }
   else
   {
      $mvPitchDownSpeed = 0;
      $mvPitchUpSpeed = -%pitchAdj;
   }
}

moveMap.bind( keyboard, a, moveleft );
moveMap.bind( keyboard, d, moveright );
moveMap.bind( keyboard, left, moveleft );
moveMap.bind( keyboard, right, moveright );

moveMap.bind( keyboard, w, moveforward );
moveMap.bind( keyboard, s, movebackward );
moveMap.bind( keyboard, up, moveforward );
moveMap.bind( keyboard, down, movebackward );

moveMap.bind( keyboard, e, moveup );
moveMap.bind( keyboard, c, movedown );

moveMap.bind( keyboard, space, jump );
moveMap.bind( mouse, xaxis, yaw );
moveMap.bind( mouse, yaxis, pitch );

moveMap.bind( gamepad, thumbrx, "D", "-0.23 0.23", gamepadYaw );
moveMap.bind( gamepad, thumbry, "D", "-0.23 0.23", gamepadPitch );
moveMap.bind( gamepad, thumblx, "D", "-0.23 0.23", gamePadMoveX );
moveMap.bind( gamepad, thumbly, "D", "-0.23 0.23", gamePadMoveY );

moveMap.bind( gamepad, btn_a, jump );
moveMap.bindCmd( gamepad, btn_back, "disconnect();", "" );

moveMap.bindCmd(gamepad, dpadl, "toggleLightColorViz();", "");
moveMap.bindCmd(gamepad, dpadu, "toggleDepthViz();", "");
moveMap.bindCmd(gamepad, dpadd, "toggleNormalsViz();", "");
moveMap.bindCmd(gamepad, dpadr, "toggleLightSpecularViz();", "");

// ----------------------------------------------------------------------------
// Stance/pose
// ----------------------------------------------------------------------------

function doCrouch(%val)
{
   $mvTriggerCount3++;
}

moveMap.bind(keyboard, lcontrol, doCrouch);
moveMap.bind(gamepad, btn_b, doCrouch);

function doSprint(%val)
{
   $mvTriggerCount5++;
}

moveMap.bind(keyboard, lshift, doSprint);

//------------------------------------------------------------------------------
// Mouse Trigger
//------------------------------------------------------------------------------

function mouseFire(%val)
{
   $mvTriggerCount0++;
}

//function altTrigger(%val)
//{
   //$mvTriggerCount1++;
//}

moveMap.bind( mouse, button0, mouseFire );
//moveMap.bind( mouse, button1, altTrigger );

//------------------------------------------------------------------------------
// Gamepad Trigger
//------------------------------------------------------------------------------

function gamepadFire(%val)
{
   if(%val > 0.1 && !$gamepadFireTriggered)
   {
      $gamepadFireTriggered = true;
      $mvTriggerCount0++;
   }
   else if(%val <= 0.1 && $gamepadFireTriggered)
   {
      $gamepadFireTriggered = false;
      $mvTriggerCount0++;
   }
}

function gamepadAltTrigger(%val)
{
   if(%val > 0.1 && !$gamepadAltTriggerTriggered)
   {
      $gamepadAltTriggerTriggered = true;
      $mvTriggerCount1++;
   }
   else if(%val <= 0.1 && $gamepadAltTriggerTriggered)
   {
      $gamepadAltTriggerTriggered = false;
      $mvTriggerCount1++;
   }
}

moveMap.bind(gamepad, triggerr, gamepadFire);
moveMap.bind(gamepad, triggerl, gamepadAltTrigger);

//------------------------------------------------------------------------------
// Zoom and FOV functions
//------------------------------------------------------------------------------

if($Player::CurrentFOV $= "")
   $Player::CurrentFOV = $pref::Player::DefaultFOV / 2;

// toggleZoomFOV() works by dividing the CurrentFOV by 2.  Each time that this
// toggle is hit it simply divides the CurrentFOV by 2 once again.  If the
// FOV is reduced below a certain threshold then it resets to equal half of the
// DefaultFOV value.  This gives us 4 zoom levels to cycle through.

function toggleZoomFOV()
{
    $Player::CurrentFOV = $Player::CurrentFOV / 2;

    if($Player::CurrentFOV < 5)
        resetCurrentFOV();

    if(ServerConnection.zoomed)
      setFOV($Player::CurrentFOV);
    else
    {
      setFov(ServerConnection.getControlCameraDefaultFov());
    }
}

function resetCurrentFOV()
{
   $Player::CurrentFOV = ServerConnection.getControlCameraDefaultFov() / 2;
}

function turnOffZoom()
{
   ServerConnection.zoomed = false;
   setFov(ServerConnection.getControlCameraDefaultFov());
   Reticle.setVisible(true);
   zoomReticle.setVisible(false);

   // Rather than just disable the DOF effect, we want to set it to the level's
   // preset values.
   //DOFPostEffect.disable();
   ppOptionsUpdateDOFSettings();
}

function setZoomFOV(%val)
{
   if(%val)
      toggleZoomFOV();
}

function toggleZoom(%val)
{
   if (%val)
   {
      ServerConnection.zoomed = true;
      setFov($Player::CurrentFOV);
      Reticle.setVisible(false);
      zoomReticle.setVisible(true);

      DOFPostEffect.setAutoFocus( true );
      DOFPostEffect.setFocusParams( 0.5, 0.5, 50, 500, -5, 5 );
      DOFPostEffect.enable();
   }
   else
   {
      turnOffZoom();
   }
}

function mouseButtonZoom(%val)
{
   toggleZoom(%val);
}

moveMap.bind(keyboard, f, setZoomFOV); // f for field of view
moveMap.bind(keyboard, z, toggleZoom); // z for zoom
moveMap.bind( mouse, button1, mouseButtonZoom );

//------------------------------------------------------------------------------
// Camera & View functions
//------------------------------------------------------------------------------

function toggleFreeLook( %val )
{
   if ( %val )
      $mvFreeLook = true;
   else
      $mvFreeLook = false;
}

function toggleFirstPerson(%val)
{
   if (%val)
   {
      ServerConnection.setFirstPerson(!ServerConnection.isFirstPerson());
   }
}

function toggleCamera(%val)
{
   if (%val)
      commandToServer('ToggleCamera');
}

moveMap.bind( keyboard, v, toggleFreeLook ); // v for vanity
moveMap.bind(keyboard, tab, toggleFirstPerson );
moveMap.bind(keyboard, "alt c", toggleCamera);

moveMap.bind( gamepad, btn_start, toggleCamera );
moveMap.bind( gamepad, btn_x, toggleFirstPerson );

// ----------------------------------------------------------------------------
// Misc. Player stuff
// ----------------------------------------------------------------------------

// Gideon does not have these animations, so the player does not need access to
// them.  Commenting instead of removing so as to retain an example for those
// who will want to use a player model that has these animations and wishes to
// use them.

//moveMap.bindCmd(keyboard, "ctrl w", "commandToServer('playCel',\"wave\");", "");
//moveMap.bindCmd(keyboard, "ctrl s", "commandToServer('playCel',\"salute\");", "");

moveMap.bindCmd(keyboard, "ctrl k", "commandToServer('suicide');", "");

//------------------------------------------------------------------------------
// Item manipulation
//------------------------------------------------------------------------------
moveMap.bindCmd(keyboard, "1", "commandToServer('use',\"Ryder\");", "");
moveMap.bindCmd(keyboard, "2", "commandToServer('use',\"Lurker\");", "");
moveMap.bindCmd(keyboard, "3", "commandToServer('use',\"LurkerGrenadeLauncher\");", "");
moveMap.bindCmd(keyboard, "4", "commandToServer('use',\"ProxMine\");", "");
moveMap.bindCmd(keyboard, "5", "commandToServer('use',\"DeployableTurret\");", "");

moveMap.bindCmd(keyboard, "r", "commandToServer('reloadWeapon');", "");

function unmountWeapon(%val)
{
   if (%val)
      commandToServer('unmountWeapon');
}

moveMap.bind(keyboard, 0, unmountWeapon);

function throwWeapon(%val)
{
   if (%val)
      commandToServer('Throw', "Weapon");
}
function tossAmmo(%val)
{
   if (%val)
      commandToServer('Throw', "Ammo");
}

moveMap.bind(keyboard, "alt w", throwWeapon);
moveMap.bind(keyboard, "alt a", tossAmmo);

function nextWeapon(%val)
{
   if (%val)
      commandToServer('cycleWeapon', "next");
}

function prevWeapon(%val)
{
   if (%val)
      commandToServer('cycleWeapon', "prev");
}

function mouseWheelWeaponCycle(%val)
{
   if (%val < 0)
      commandToServer('cycleWeapon', "next");
   else if (%val > 0)
      commandToServer('cycleWeapon', "prev");
}

moveMap.bind(keyboard, q, nextWeapon);
moveMap.bind(keyboard, "ctrl q", prevWeapon);
moveMap.bind(mouse, "zaxis", mouseWheelWeaponCycle);

//------------------------------------------------------------------------------
// Message HUD functions
//------------------------------------------------------------------------------

function pageMessageHudUp( %val )
{
   if ( %val )
      pageUpMessageHud();
}

function pageMessageHudDown( %val )
{
   if ( %val )
      pageDownMessageHud();
}

function resizeMessageHud( %val )
{
   if ( %val )
      cycleMessageHudSize();
}

moveMap.bind(keyboard, u, toggleMessageHud );
//moveMap.bind(keyboard, y, teamMessageHud );
moveMap.bind(keyboard, "pageUp", pageMessageHudUp );
moveMap.bind(keyboard, "pageDown", pageMessageHudDown );
moveMap.bind(keyboard, "p", resizeMessageHud );

//------------------------------------------------------------------------------
// Demo recording functions
//------------------------------------------------------------------------------

function startRecordingDemo( %val )
{
   if ( %val )
      startDemoRecord();
}

function stopRecordingDemo( %val )
{
   if ( %val )
      stopDemoRecord();
}

moveMap.bind( keyboard, F3, startRecordingDemo );
moveMap.bind( keyboard, F4, stopRecordingDemo );

//------------------------------------------------------------------------------
// Theora Video Capture (Records a movie file)  
//------------------------------------------------------------------------------

function toggleMovieRecording(%val)
{
   if (!%val)
      return;

   %movieEncodingType = "THEORA";  // Valid encoder values are "PNG" and "THEORA" (default). 
   %movieFPS = 30;  // video capture frame rate.
   
   if (!$RecordingMovie)
   {
       // locate a non-existent filename to use
       for(%i = 0; %i < 1000; %i++)
       {
          %num = %i;
          if(%num < 10)
             %num = "0" @ %num;
          if(%num < 100)
             %num = "0" @ %num;
       
          %filePath = "movies/movie" @ %num;
          if(!isfile(%filePath))
             break;
       }
       if(%i == 1000)
          return;

      // Start the movie recording
      recordMovie(%filePath, %movieFPS, %movieEncodingType);
      
   }
   else
   {
      // Stop the current recording
      stopMovie();
   }
}

// Key binding works at any time and not just while in a game.
GlobalActionMap.bind(keyboard, "alt m", toggleMovieRecording);

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

moveMap.bind(keyboard, "F8", dropCameraAtPlayer);
moveMap.bind(keyboard, "F7", dropPlayerAtCamera);

function bringUpOptions(%val)
{
   if (%val)
      Canvas.pushDialog(OptionsDlg);
}

GlobalActionMap.bind(keyboard, "ctrl o", bringUpOptions);


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
GlobalActionMap.bind(keyboard, "ctrl F2", showMetrics);

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

GlobalActionMap.bind(keyboard, "ctrl F3", doProfile);

//------------------------------------------------------------------------------
// Misc.
//------------------------------------------------------------------------------

GlobalActionMap.bind(keyboard, "tilde", toggleConsole);
GlobalActionMap.bindCmd(keyboard, "alt k", "cls();","");
GlobalActionMap.bindCmd(keyboard, "alt enter", "", "Canvas.attemptFullscreenToggle();");
GlobalActionMap.bindCmd(keyboard, "F1", "", "contextHelp();");
moveMap.bindCmd(keyboard, "n", "toggleNetGraph();", "");

// ----------------------------------------------------------------------------
// Useful vehicle stuff
// ----------------------------------------------------------------------------

// Trace a line along the direction the crosshair is pointing
// If you find a car with a player in it...eject them
function carjack()
{
   %player = LocalClientConnection.getControlObject();

   if (%player.getClassName() $= "Player")
   {
      %eyeVec = %player.getEyeVector();

      %startPos = %player.getEyePoint();
      %endPos = VectorAdd(%startPos, VectorScale(%eyeVec, 1000));

      %target = ContainerRayCast(%startPos, %endPos, $TypeMasks::VehicleObjectType);

      if (%target)
      {
         // See if anyone is mounted in the car's driver seat
         %mount = %target.getMountNodeObject(0);

         // Can only carjack bots
         // remove '&& %mount.getClassName() $= "AIPlayer"' to allow you
         // to carjack anyone/anything
         if (%mount && %mount.getClassName() $= "AIPlayer")
         {
            commandToServer('carUnmountObj', %mount);
         }
      }
   }
}

// Bind the keys to the carjack command
moveMap.bindCmd(keyboard, "ctrl z", "carjack();", "");


// Starting vehicle action map code
if ( isObject( vehicleMap ) )
   vehicleMap.delete();
new ActionMap(vehicleMap);

// The key command for flipping the car
vehicleMap.bindCmd(keyboard, "ctrl x", "commandToServer(\'flipCar\');", "");

function getOut()
{
   vehicleMap.pop();
   moveMap.push();
   commandToServer('dismountVehicle');
}

function brakeLights()
{
   // Turn on/off the Cheetah's head lights.
   commandToServer('toggleBrakeLights');
}

function brake(%val)
{
   commandToServer('toggleBrakeLights');
   $mvTriggerCount2++;
}

vehicleMap.bind( keyboard, w, moveforward );
vehicleMap.bind( keyboard, s, movebackward );
vehicleMap.bind( keyboard, up, moveforward );
vehicleMap.bind( keyboard, down, movebackward );
vehicleMap.bind( mouse, xaxis, yaw );
vehicleMap.bind( mouse, yaxis, pitch );
vehicleMap.bind( mouse, button0, mouseFire );
vehicleMap.bind( mouse, button1, altTrigger );
vehicleMap.bindCmd(keyboard, "ctrl f","getout();","");
vehicleMap.bind(keyboard, space, brake);
vehicleMap.bindCmd(keyboard, "l", "brakeLights();", "");
vehicleMap.bindCmd(keyboard, "escape", "", "handleEscape();");
vehicleMap.bind( keyboard, v, toggleFreeLook ); // v for vanity
//vehicleMap.bind(keyboard, tab, toggleFirstPerson );
vehicleMap.bind(keyboard, "alt c", toggleCamera);
// bind the left thumbstick for steering
vehicleMap.bind( gamepad, thumblx, "D", "-0.23 0.23", gamepadYaw );
// bind the gas, break, and reverse buttons
vehicleMap.bind( gamepad, btn_a, moveforward );
vehicleMap.bind( gamepad, btn_b, brake );
vehicleMap.bind( gamepad, btn_x, movebackward );
// bind exiting the vehicle to a button
vehicleMap.bindCmd(gamepad, btn_y,"getout();","");


// ----------------------------------------------------------------------------
// Oculus Rift
// ----------------------------------------------------------------------------

function OVRSensorRotEuler(%pitch, %roll, %yaw)
{
   //echo("Sensor euler: " @ %pitch SPC %roll SPC %yaw);
   $mvRotZ0 = %yaw;
   $mvRotX0 = %pitch;
   $mvRotY0 = %roll;
}

$mvRotIsEuler0 = true;
$OculusVR::GenerateAngleAxisRotationEvents = false;
$OculusVR::GenerateEulerRotationEvents = true;
moveMap.bind( oculusvr, ovr_sensorrotang0, OVRSensorRotEuler );
