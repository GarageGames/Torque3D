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

// Load up all scripts.  This function is called when
// a server is constructed.
exec("./camera.cs");
exec("./triggers.cs");
exec("./VolumetricFog.cs");
exec("./inventory.cs");
exec("./shapeBase.cs");
exec("./item.cs");
exec("./health.cs");
exec("./projectile.cs");
exec("./radiusDamage.cs");
exec("./teleporter.cs");
exec("./physicsShape.cs");

// Load our supporting weapon script, it contains methods used by all weapons.
exec("./weapon.cs");

// Load our weapon scripts
// We only need weapon scripts for those weapons that work differently from the
// class methods defined in weapon.cs
exec("./proximityMine.cs");

// Load our default player script
exec("./player.cs");

// Load our player scripts
exec("./aiPlayer.cs");

exec("./vehicle.cs");
exec("./vehicleWheeled.cs");
exec("./cheetah.cs");

// Load turret support scripts
exec("./turret.cs");

// Load our gametypes
exec("./gameCore.cs"); // This is the 'core' of the gametype functionality.
exec("./gameDM.cs"); // Overrides GameCore with DeathMatch functionality.

//Entity/Component stuff
if(isFile("./components/game/camera.cs"))
   exec("./components/game/camera.cs");
if(isFile("./components/game/controlObject.cs"))
   exec("./components/game/controlObject.cs");
if(isFile("./components/game/itemRotate.cs"))
   exec("./components/game/itemRotate.cs");
if(isFile("./components/game/playerSpawner.cs"))
   exec("./components/game/playerSpawner.cs");
if(isFile("./components/input/fpsControls.cs"))
   exec("./components/input/fpsControls.cs");
if(isFile("./components/input/inputManager.cs"))
   exec("./components/input/inputManager.cs");
   
if(isFile("./gameObjects/GameObjectManager.cs"))
{
   exec("./gameObjects/GameObjectManager.cs");
   execGameObjects();  
}
