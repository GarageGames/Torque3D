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
moveMap.bind( keyboard, F2, showPlayerList );

moveMap.bind(keyboard, "ctrl h", hideHUDs);

moveMap.bind(keyboard, "alt p", doScreenShotHudless);

moveMap.bindCmd(keyboard, "escape", "", "Canvas.pushDialog(PauseMenu);");

//------------------------------------------------------------------------------
// Movement Keys
//------------------------------------------------------------------------------
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
moveMap.bind(keyboard, lcontrol, doCrouch);
moveMap.bind(gamepad, btn_b, doCrouch);

moveMap.bind(keyboard, lshift, doSprint);

//------------------------------------------------------------------------------
// Mouse Trigger
//------------------------------------------------------------------------------
//function altTrigger(%val)
//{
   //$mvTriggerCount1++;
//}

moveMap.bind( mouse, button0, mouseFire );
//moveMap.bind( mouse, button1, altTrigger );

//------------------------------------------------------------------------------
// Gamepad Trigger
//------------------------------------------------------------------------------
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
moveMap.bind(keyboard, f, setZoomFOV); // f for field of view
moveMap.bind(keyboard, z, toggleZoom); // z for zoom
moveMap.bind( mouse, button1, mouseButtonZoom );

//------------------------------------------------------------------------------
// Camera & View functions
//------------------------------------------------------------------------------
moveMap.bind( keyboard, v, toggleFreeLook ); // v for vanity
moveMap.bind(keyboard, tab, toggleFirstPerson );

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

moveMap.bind(keyboard, 0, unmountWeapon);

moveMap.bind(keyboard, "alt w", throwWeapon);
moveMap.bind(keyboard, "alt a", tossAmmo);

moveMap.bind(keyboard, q, nextWeapon);
moveMap.bind(keyboard, "ctrl q", prevWeapon);
moveMap.bind(mouse, "zaxis", mouseWheelWeaponCycle);

//------------------------------------------------------------------------------
// Message HUD functions
//------------------------------------------------------------------------------
moveMap.bind(keyboard, u, toggleMessageHud );
//moveMap.bind(keyboard, y, teamMessageHud );
moveMap.bind(keyboard, "pageUp", pageMessageHudUp );
moveMap.bind(keyboard, "pageDown", pageMessageHudDown );
moveMap.bind(keyboard, "p", resizeMessageHud );

//------------------------------------------------------------------------------
// Demo recording functions
//------------------------------------------------------------------------------
moveMap.bind( keyboard, F3, startRecordingDemo );
moveMap.bind( keyboard, F4, stopRecordingDemo );

//------------------------------------------------------------------------------
// Helper Functions
//------------------------------------------------------------------------------
moveMap.bind(keyboard, "F8", dropCameraAtPlayer);
moveMap.bind(keyboard, "F7", dropPlayerAtCamera);

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
// Bind the keys to the carjack command
moveMap.bindCmd(keyboard, "ctrl z", "carjack();", "");

// Starting vehicle action map code
if ( isObject( vehicleMap ) )
   vehicleMap.delete();
new ActionMap(vehicleMap);

//------------------------------------------------------------------------------
// Non-remapable binds
//------------------------------------------------------------------------------
vehicleMap.bindCmd(keyboard, "escape", "", "Canvas.pushDialog(PauseMenu);");

// The key command for flipping the car
vehicleMap.bindCmd(keyboard, "ctrl x", "commandToServer(\'flipCar\');", "");

vehicleMap.bind( keyboard, w, moveforward );
vehicleMap.bind( keyboard, s, movebackward );
vehicleMap.bind( keyboard, up, moveforward );
vehicleMap.bind( keyboard, down, movebackward );
vehicleMap.bind( mouse, xaxis, yaw );
vehicleMap.bind( mouse, yaxis, pitch );
vehicleMap.bind( mouse, button0, mouseFire );
vehicleMap.bind( mouse, button1, altTrigger );
vehicleMap.bindCmd(keyboard, "f","getout();","");
vehicleMap.bind(keyboard, space, brake);
vehicleMap.bindCmd(keyboard, "l", "brakeLights();", "");
vehicleMap.bind( keyboard, v, toggleFreeLook ); // v for vanity
//vehicleMap.bind(keyboard, tab, toggleFirstPerson );
// bind the left thumbstick for steering
vehicleMap.bind( gamepad, thumblx, "D", "-0.23 0.23", gamepadYaw );
// bind the gas, break, and reverse buttons
vehicleMap.bind( gamepad, btn_a, moveforward );
vehicleMap.bind( gamepad, btn_b, brake );
vehicleMap.bind( gamepad, btn_x, movebackward );
// bind exiting the vehicle to a button
vehicleMap.bindCmd(gamepad, btn_y,"getout();","");