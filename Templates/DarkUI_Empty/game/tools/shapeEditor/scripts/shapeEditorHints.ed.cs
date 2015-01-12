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

// The Shape Editor tool can provide some simple hints about which nodes and
// sequences are required/desired for a particular type of shape to work with
// Torque. The following objects define the node and sequence hints, and the
// Shape Editor gui marks them as present or not-present in the current shape.

new SimGroup (ShapeHintControls)
{
};

new SimGroup (ShapeHintGroup)
{
   new ScriptObject()
   {
      objectType = "ShapeBase";
      node0 = "cam" TAB "This node is used as the 3rd person camera position.";
      node1 = "eye" TAB "This node is used as the 1st person camera position.";
      node2 = "ear" TAB "This node is where the SFX listener is mounted on (if missing, eye is used).";
      node3 = "mount0-31" TAB "Nodes used for mounting ShapeBaseImages to this object";
      node4 = "AIRepairNode" TAB "unused";
      sequence0 = "Visibility" TAB "2 frame sequence used to show object damage (start=no damage, end=fully damaged)";
      sequence1 = "Damage" TAB "Sequence used to show object damage (start=no damage, end=fully damaged)";
   };

   new ScriptObject()
   {
      objectType = "ShapeBaseImageData";
      node0 = "ejectPoint" TAB "Node from which shell casings are emitted (for weapon ShapeImages)";
      node1 = "muzzlePoint" TAB "Node used to fire projectiles and particles (for weapon ShapeImages)";
      node2 = "retractionPoint" TAB "Nearest point to use as muzzle when up against a wall (and muzzle node is inside wall)";
      node3 = "mountPoint" TAB "Where to attach to on this object";
      sequence0 = "ambient" TAB "Cyclic sequence to play while image is mounted";
      sequence1 = "spin" TAB "Cyclic sequence to play while image is mounted";
   };

   new ScriptObject()
   {
      objectType = "Player";
      node0 = "Bip01 Pelvis" TAB "";
      node1 = "Bip01 Spine" TAB "";
      node2 = "Bip01 Spine1" TAB "";
      node3 = "Bip01 Spine2" TAB "";
      node4 = "Bip01 Neck" TAB "";
      node5 = "Bip01 Head" TAB "";

      sequence0 = "head" TAB "Vertical head movement (for looking) (start=full up, end=full down)";
      sequence1 = "headside" TAB "Horizontal head movement (for looking) (start=full left, end=full right)";
      sequence2 = "look" TAB "Vertical arm movement (for looking) (start=full up, end=full down)";
      sequence3 = "light_recoil" TAB "Player has been hit lightly";
      sequence4 = "medium_recoil" TAB "Player has been hit moderately hard";
      sequence5 = "heavy_recoil" TAB "Player has been hit hard";

      sequence6 = "root" TAB "Player is not moving";
      sequence7 = "run" TAB "Player is running forward";
      sequence8 = "back" TAB "Player is running backward";
      sequence9 = "side" TAB "Player is running sideways left (strafing)";
      sequence9 = "side_right" TAB "Player is running sideways right (strafing)";

      sequence10 = "crouch_root" TAB "Player is crouched and not moving";
      sequence11 = "crouch_forward" TAB "Player is crouched and moving forward";
      sequence11 = "crouch_backward" TAB "Player is crouched and moving backward";
      sequence11 = "crouch_side" TAB "Player is crouched and moving sideways left";
      sequence11 = "crouch_right" TAB "Player is crouched and moving sideways right";

      sequence12 = "prone_root" TAB "Player is lying down and not moving";
      sequence13 = "prone_forward" TAB "Player is lying down and moving forward";
      sequence13 = "prone_backward" TAB "Player is lying down and moving backward";

      sequence14 = "swim_root" TAB "Player is swimming and not moving";
      sequence15 = "swim_forward" TAB "Player is swimming and moving forward";
      sequence16 = "swim_backward" TAB "Player is swimming and moving backward";
      sequence17 = "swim_left" TAB "Player is swimming and moving left";
      sequence18 = "swim_right" TAB "Player is swimming and moving right";

      sequence19 = "fall" TAB "Player is falling";
      sequence20 = "jump" TAB "Player has jumped from a moving start";
      sequence21 = "standjump" TAB "Player has jumped from a standing start";
      sequence22 = "land" TAB "Player has landed after falling";
      sequence23 = "jet" TAB "Player is jetting";

      sequence24 = "death1-11" TAB "Player has been killed (only one of these will play)";
   };

   new ScriptObject()
   {
      objectType = "WheeledVehicle";
      node0 = "hub0-7" TAB "Placement node for wheel X";
      sequence0 = "spring0-7" TAB "Spring suspension for wheel X (start=fully compressed, end=fully extended)";
      sequence1 = "steering" TAB "Steering mechanism (start=full left, end=full right)";
      sequence2 = "brakelight" TAB "Sequence to play when braking (start=brakes off, end=brakes on)";
   };

   new ScriptObject()
   {
      objectType = "HoverVehicle";
      node0 = "JetNozzle0-3" TAB "Nodes for jet engine exhaust particle emission";
      node1 = "JetNozzleX" TAB "Nodes for jet engine exhaust particle emission";
      sequence0 = "activateBack" TAB "Non-cyclic sequence to play when vehicle first starts moving backwards";
      sequence1 = "maintainBack" TAB "Cyclic sequence to play when vehicle continues moving backwards";
   };

   new ScriptObject()
   {
      objectType = "FlyingVehicle";
      node0 = "JetNozzle0-3" TAB "Nodes for jet engine exhaust particle emission";
      node1 = "JetNozzleX" TAB "Nodes for jet engine exhaust particle emission";
      node2 = "JetNozzleY" TAB "Nodes for jet engine exhaust particle emission";
      node3 = "contrail0-3" TAB "Nodes for contrail particle emission";
      sequence0 = "activateBack" TAB "Sequence to play when vehicle first starts thrusting backwards";
      sequence1 = "maintainBack" TAB "Cyclic sequence to play when vehicle continues thrusting backwards";
      sequence2 = "activateBot" TAB "Non-cyclic sequence to play when vehicle first starts thrusting upwards";
      sequence3 = "maintainBot" TAB "Cyclic sequence to play when vehicle continues thrusting upwards";
   };

   new ScriptObject()
   {
      objectType = "Projectile";
      sequence0 = "activate" TAB "Non-cyclic sequence to play when projectile is first created";
      sequence1 = "maintain" TAB "Cyclic sequence to play for remainder of projectile lifetime";
   };
};
