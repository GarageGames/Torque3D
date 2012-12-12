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

singleton TSShapeConstructor(SoldierDAE)
{
   baseShape = "./soldier_rigged.DAE";
   loadLights = "0";
   unit = "1.0";
   upAxis = "DEFAULT";
   lodType = "TrailingNumber";
   matNamePrefix = "";
   ignoreNodeScale = "0";
   adjustCenter = "0";
   adjustFloor = "0";
   forceUpdateMaterials = "0";
};

function SoldierDAE::onLoad(%this)
{
   // BEGIN: Default (Lurker Rifle) Sequences
   %this.addSequence( "./Anims/PlayerAnim_Lurker_Back.dae Back", "Back", 0, -1);
   %this.addSequence( "./Anims/PlayerAnim_Lurker_Celebrate_01.dae Celebrate_01", "Celebrate_01", 0, -1);
   %this.addSequence( "./Anims/PlayerAnim_Lurker_Crouch_Backward.dae Crouch_Backward", "Crouch_Backward", 0, -1);
   %this.addSequence( "./Anims/PlayerAnim_Lurker_Crouch_Forward.dae Crouch_Forward", "Crouch_Forward", 0, -1);
   %this.addSequence( "./Anims/PlayerAnim_Lurker_Crouch_Side.dae Crouch_Side", "Crouch_Side", 0, -1);
   %this.addSequence( "./Anims/PlayerAnim_Lurker_Crouch_Root.dae Crouch_Root", "Crouch_Root", 0, -1);
   %this.addSequence( "./Anims/PlayerAnim_Lurker_Death1.dae Death1", "Death1", 0, -1);
   %this.addSequence( "./Anims/PlayerAnim_Lurker_Death2.dae Death2", "Death2", 0, -1);
   %this.addSequence( "./Anims/PlayerAnim_Lurker_Fall.dae Fall", "Fall", 0, -1);
   %this.addSequence( "./Anims/PlayerAnim_Lurker_Head.dae Head", "Head", 0, -1);
   %this.addSequence( "./Anims/PlayerAnim_Lurker_Jump.dae Jump", "Jump", 0, -1);
   %this.addSequence( "./Anims/PlayerAnim_Lurker_Land.dae Land", "Land", 0, -1);
   %this.addSequence( "./Anims/PlayerAnim_Lurker_Look.dae Look", "Look", 0, -1);
   %this.addSequence( "./Anims/PlayerAnim_Lurker_Reload.dae Reload", "Reload", 0, -1);
   %this.addSequence( "./Anims/PlayerAnim_Lurker_Root.dae Root", "Root", 0, -1);
   %this.addSequence( "./Anims/PlayerAnim_Lurker_Run.dae Run", "Run", 0, -1);
   %this.addSequence( "./Anims/PlayerAnim_Lurker_Side.dae Side", "Side", 0, -1);
   %this.addSequence( "./Anims/PlayerAnim_Lurker_Sitting.dae Sitting", "Sitting", 0, -1);
   %this.addSequence( "./Anims/PlayerAnim_Lurker_Swim_Backward.dae Swim_Backward", "Swim_Backward", 0, -1);
   %this.addSequence( "./Anims/PlayerAnim_Lurker_Swim_Forward.dae Swim_Forward", "Swim_Forward", 0, -1);
   %this.addSequence( "./Anims/PlayerAnim_Lurker_Swim_Root.dae Swim_Root", "Swim_Root", 0, -1);
   %this.addSequence( "./Anims/PlayerAnim_Lurker_Swim_Left.dae Swim_Left", "Swim_Left", 0, -1);
   %this.addSequence( "./Anims/PlayerAnim_Lurker_Swim_Right.dae Swim_Right", "Swim_Right", 0, -1);
   
   %this.setSequenceCyclic( "Back", true);
   %this.setSequenceCyclic( "Celebrate_01", false);
   %this.setSequenceCyclic( "Crouch_Backward", true);
   %this.setSequenceCyclic( "Crouch_Forward", true);
   %this.setSequenceCyclic( "Crouch_Side", true);
   %this.setSequenceCyclic( "Crouch_Root", true);
   %this.setSequenceCyclic( "Death1", false);
   %this.setSequenceCyclic( "Death2", false);
   %this.setSequenceCyclic( "Fall", true);
   %this.setSequenceCyclic( "Head", false);
   %this.setSequenceCyclic( "Jump", false);
   %this.setSequenceCyclic( "Land", false);
   %this.setSequenceCyclic( "Look", false);
   %this.setSequenceCyclic( "Reload", false);
   %this.setSequenceCyclic( "Root", true);
   %this.setSequenceCyclic( "Run", true);
   %this.setSequenceCyclic( "Side", true);
   %this.setSequenceCyclic( "Sitting", true);
   %this.setSequenceCyclic( "Swim_Backward", true);
   %this.setSequenceCyclic( "Swim_Forward", true);
   %this.setSequenceCyclic( "Swim_Root", true);
   %this.setSequenceCyclic( "Swim_Left", true);
   %this.setSequenceCyclic( "Swim_Right", true);

   %this.setSequenceBlend( "Head", "1", "Root", "0");
   %this.setSequenceBlend( "Look", "1", "Root", "0");
   %this.setSequenceBlend( "Reload", "1", "Root", "0");
   
   %this.setSequenceGroundSpeed( "Back", "0 -3.6 0");
   %this.setSequenceGroundSpeed( "Run", "0 5.0 0");
   %this.setSequenceGroundSpeed( "Side", "-3.6 0 0");
   %this.setSequenceGroundSpeed( "Swim_Backward", "0 -1 0");
   %this.setSequenceGroundSpeed( "Swim_Forward", "0 1 0");
   %this.setSequenceGroundSpeed( "Swim_Left", "-1 0 0");
   %this.setSequenceGroundSpeed( "Swim_Right", "1 0 0");
   %this.setSequenceGroundSpeed( "Crouch_Backward", "0 -2 0");
   %this.setSequenceGroundSpeed( "Crouch_Forward", "0 2 0");
   %this.setSequenceGroundSpeed( "Crouch_Side", "1 0 0");
   // END: Lurker Rifle Sequences

   // BEGIN: Ryder Pistol sequences
   // Extracted from Ryder
   %this.addSequence( "art/shapes/weapons/Ryder/PlayerAnims/PlayerAnim_Pistol_Back.dae Back", "Pistol_Back", 0, -1);
   %this.addSequence( "art/shapes/weapons/Ryder/PlayerAnims/PlayerAnim_Pistol_Crouch_Backward.dae Crouch_Backward", "Pistol_Crouch_Backward", 0, -1);
   %this.addSequence( "art/shapes/weapons/Ryder/PlayerAnims/PlayerAnim_Pistol_Crouch_Forward.dae Crouch_Forward", "Pistol_Crouch_Forward", 0, -1);
   %this.addSequence( "art/shapes/weapons/Ryder/PlayerAnims/PlayerAnim_Pistol_Crouch_Side.dae Crouch_Side", "Pistol_Crouch_Side", 0, -1);
   %this.addSequence( "art/shapes/weapons/Ryder/PlayerAnims/PlayerAnim_Pistol_Crouch_Root.dae Crouch_Root", "Pistol_Crouch_Root", 0, -1);
   %this.addSequence( "art/shapes/weapons/Ryder/PlayerAnims/PlayerAnim_Pistol_Death1.dae Death1", "Pistol_Death1", 0, -1);
   %this.addSequence( "art/shapes/weapons/Ryder/PlayerAnims/PlayerAnim_Pistol_Death2.dae Death2", "Pistol_Death2", 0, -1);
   %this.addSequence( "art/shapes/weapons/Ryder/PlayerAnims/PlayerAnim_Pistol_Fall.dae Fall", "Pistol_Fall", 0, -1);
   %this.addSequence( "art/shapes/weapons/Ryder/PlayerAnims/PlayerAnim_Pistol_Head.dae Head", "Pistol_Head", 0, -1);
   %this.addSequence( "art/shapes/weapons/Ryder/PlayerAnims/PlayerAnim_Pistol_Jump.dae Jump", "Pistol_Jump", 0, -1);
   %this.addSequence( "art/shapes/weapons/Ryder/PlayerAnims/PlayerAnim_Pistol_Land.dae Land", "Pistol_Land", 0, -1);
   %this.addSequence( "art/shapes/weapons/Ryder/PlayerAnims/PlayerAnim_Pistol_Look.dae Look", "Pistol_Look", 0, -1);
   %this.addSequence( "art/shapes/weapons/Ryder/PlayerAnims/PlayerAnim_Pistol_Reload.dae Reload", "Pistol_Reload", 0, -1);
   %this.addSequence( "art/shapes/weapons/Ryder/PlayerAnims/PlayerAnim_Pistol_Root.dae Root", "Pistol_Root", 0, -1);
   %this.addSequence( "art/shapes/weapons/Ryder/PlayerAnims/PlayerAnim_Pistol_Run.dae Run", "Pistol_Run", 0, -1);
   %this.addSequence( "art/shapes/weapons/Ryder/PlayerAnims/PlayerAnim_Pistol_Side.dae Side", "Pistol_Side", 0, -1);
   %this.addSequence( "art/shapes/weapons/Ryder/PlayerAnims/PlayerAnim_Pistol_Sitting.dae Sitting", "Pistol_Sitting", 0, -1);
   %this.addSequence( "art/shapes/weapons/Ryder/PlayerAnims/PlayerAnim_Pistol_Swim_Backward.dae Swim_Backward", "Pistol_Swim_Backward", 0, -1);
   %this.addSequence( "art/shapes/weapons/Ryder/PlayerAnims/PlayerAnim_Pistol_Swim_Forward.dae Swim_Forward", "Pistol_Swim_Forward", 0, -1);
   %this.addSequence( "art/shapes/weapons/Ryder/PlayerAnims/PlayerAnim_Pistol_Swim_Root.dae Swim_Root", "Pistol_Swim_Root", 0, -1);
   %this.addSequence( "art/shapes/weapons/Ryder/PlayerAnims/PlayerAnim_Pistol_Swim_Left.dae Swim_Left", "Pistol_Swim_Left", 0, -1);
   %this.addSequence( "art/shapes/weapons/Ryder/PlayerAnims/PlayerAnim_Pistol_Swim_Right.dae Swim_Right", "Pistol_Swim_Right", 0, -1);
   
   %this.setSequenceCyclic( "Pistol_Back", true);
   %this.setSequenceCyclic( "Pistol_Crouch_Backward", true);
   %this.setSequenceCyclic( "Pistol_Crouch_Forward", true);
   %this.setSequenceCyclic( "Pistol_Crouch_Side", true);
   %this.setSequenceCyclic( "Pistol_Crouch_Root", true);
   %this.setSequenceCyclic( "Pistol_Death1", false);
   %this.setSequenceCyclic( "Pistol_Death2", false);
   %this.setSequenceCyclic( "Pistol_Fall", true);
   %this.setSequenceCyclic( "Pistol_Head", false);
   %this.setSequenceCyclic( "Pistol_Jump", false);
   %this.setSequenceCyclic( "Pistol_Land", false);
   %this.setSequenceCyclic( "Pistol_Look", false);
   %this.setSequenceCyclic( "Pistol_Reload", false);
   %this.setSequenceCyclic( "Pistol_Root", true);
   %this.setSequenceCyclic( "Pistol_Run", true);
   %this.setSequenceCyclic( "Pistol_Side", true);
   %this.setSequenceCyclic( "Pistol_Sitting", true);
   %this.setSequenceCyclic( "Pistol_Swim_Backward", true);
   %this.setSequenceCyclic( "Pistol_Swim_Forward", true);
   %this.setSequenceCyclic( "Pistol_Swim_Root", true);
   %this.setSequenceCyclic( "Pistol_Swim_Left", true);
   %this.setSequenceCyclic( "Pistol_Swim_Right", true);

   %this.setSequenceBlend( "Pistol_Head", "1", "Pistol_Root", "0");
   %this.setSequenceBlend( "Pistol_Look", "1", "Pistol_Root", "0");
   %this.setSequenceBlend( "Pistol_Reload", "1", "Pistol_Root", "0");
   
   %this.setSequenceGroundSpeed( "Pistol_Back", "0 -3.6 0");
   %this.setSequenceGroundSpeed( "Pistol_Run", "0 5.0 0");
   %this.setSequenceGroundSpeed( "Pistol_Side", "3.6 0 0");
   %this.setSequenceGroundSpeed( "Pistol_Swim_Backward", "0 -1 0");
   %this.setSequenceGroundSpeed( "Pistol_Swim_Forward", "0 1 0");
   %this.setSequenceGroundSpeed( "Pistol_Swim_Left", "-1 0 0");
   %this.setSequenceGroundSpeed( "Pistol_Swim_Right", "1 0 0");
   %this.setSequenceGroundSpeed( "Pistol_Crouch_Backward", "0 -2 0");
   %this.setSequenceGroundSpeed( "Pistol_Crouch_Forward", "0 2 0");
   %this.setSequenceGroundSpeed( "Pistol_Crouch_Side", "1 0 0");
   // END: General pistol sequences

   // BEGIN: ProxMine Sequences
   %this.addSequence( "art/shapes/weapons/ProxMine/PlayerAnims/PlayerAnim_ProxMine_Back.dae Back", "ProxMine_Back", 0, -1);
   %this.addSequence( "art/shapes/weapons/ProxMine/PlayerAnims/PlayerAnim_ProxMine_Crouch_Backward.dae Crouch_Backward", "ProxMine_Crouch_Backward", 0, -1);
   %this.addSequence( "art/shapes/weapons/ProxMine/PlayerAnims/PlayerAnim_ProxMine_Crouch_Forward.dae Crouch_Forward", "ProxMine_Crouch_Forward", 0, -1);
   %this.addSequence( "art/shapes/weapons/ProxMine/PlayerAnims/PlayerAnim_ProxMine_Crouch_Side.dae Crouch_Side", "ProxMine_Crouch_Side", 0, -1);
   %this.addSequence( "art/shapes/weapons/ProxMine/PlayerAnims/PlayerAnim_ProxMine_Crouch_Root.dae Crouch_Root", "ProxMine_Crouch_Root", 0, -1);
   %this.addSequence( "art/shapes/weapons/ProxMine/PlayerAnims/PlayerAnim_ProxMine_Death1.dae Death1", "ProxMine_Death1", 0, -1);
   %this.addSequence( "art/shapes/weapons/ProxMine/PlayerAnims/PlayerAnim_ProxMine_Death2.dae Death2", "ProxMine_Death2", 0, -1);
   %this.addSequence( "art/shapes/weapons/ProxMine/PlayerAnims/PlayerAnim_ProxMine_Fall.dae Fall", "ProxMine_Fall", 0, -1);
   %this.addSequence( "art/shapes/weapons/ProxMine/PlayerAnims/PlayerAnim_ProxMine_Head.dae Head", "ProxMine_Head", 0, -1);
   %this.addSequence( "art/shapes/weapons/ProxMine/PlayerAnims/PlayerAnim_ProxMine_Jump.dae Jump", "ProxMine_Jump", 0, -1);
   %this.addSequence( "art/shapes/weapons/ProxMine/PlayerAnims/PlayerAnim_ProxMine_Land.dae Land", "ProxMine_Land", 0, -1);
   %this.addSequence( "art/shapes/weapons/ProxMine/PlayerAnims/PlayerAnim_ProxMine_Look.dae Look", "ProxMine_Look", 0, -1);
   %this.addSequence( "art/shapes/weapons/ProxMine/PlayerAnims/PlayerAnim_ProxMine_Reload.dae Reload", "ProxMine_Reload", 0, -1);
   %this.addSequence( "art/shapes/weapons/ProxMine/PlayerAnims/PlayerAnim_ProxMine_Fire.dae Fire", "ProxMine_Fire", 0, -1);
   %this.addSequence( "art/shapes/weapons/ProxMine/PlayerAnims/PlayerAnim_ProxMine_Fire_Release.dae Fire_Release", "ProxMine_Fire_Release", 0, -1);
   %this.addSequence( "art/shapes/weapons/ProxMine/PlayerAnims/PlayerAnim_ProxMine_Root.dae Root", "ProxMine_Root", 0, -1);
   %this.addSequence( "art/shapes/weapons/ProxMine/PlayerAnims/PlayerAnim_ProxMine_Run.dae Run", "ProxMine_Run", 0, -1);
   %this.addSequence( "art/shapes/weapons/ProxMine/PlayerAnims/PlayerAnim_ProxMine_Side.dae Side", "ProxMine_Side", 0, -1);
   %this.addSequence( "art/shapes/weapons/ProxMine/PlayerAnims/PlayerAnim_ProxMine_Sitting.dae Sitting", "ProxMine_Sitting", 0, -1);
   %this.addSequence( "art/shapes/weapons/ProxMine/PlayerAnims/PlayerAnim_ProxMine_Swim_Backward.dae Swim_Backward", "ProxMine_Swim_Backward", 0, -1);
   %this.addSequence( "art/shapes/weapons/ProxMine/PlayerAnims/PlayerAnim_ProxMine_Swim_Forward.dae Swim_Forward", "ProxMine_Swim_Forward", 0, -1);
   %this.addSequence( "art/shapes/weapons/ProxMine/PlayerAnims/PlayerAnim_ProxMine_Swim_Root.dae Swim_Root", "ProxMine_Swim_Root", 0, -1);
   %this.addSequence( "art/shapes/weapons/ProxMine/PlayerAnims/PlayerAnim_ProxMine_Swim_Left.dae Swim_Left", "ProxMine_Swim_Left", 0, -1);
   %this.addSequence( "art/shapes/weapons/ProxMine/PlayerAnims/PlayerAnim_ProxMine_Swim_Right.dae Swim_Right", "ProxMine_Swim_Right", 0, -1);
   
   %this.setSequenceCyclic( "ProxMine_Back", true);
   %this.setSequenceCyclic( "ProxMine_Crouch_Backward", true);
   %this.setSequenceCyclic( "ProxMine_Crouch_Forward", true);
   %this.setSequenceCyclic( "ProxMine_Crouch_Side", true);
   %this.setSequenceCyclic( "ProxMine_Crouch_Root", true);
   %this.setSequenceCyclic( "ProxMine_Death1", false);
   %this.setSequenceCyclic( "ProxMine_Death2", false);
   %this.setSequenceCyclic( "ProxMine_Fall", true);
   %this.setSequenceCyclic( "ProxMine_Head", false);
   %this.setSequenceCyclic( "ProxMine_Jump", false);
   %this.setSequenceCyclic( "ProxMine_Land", false);
   %this.setSequenceCyclic( "ProxMine_Look", false);
   %this.setSequenceCyclic( "ProxMine_Reload", false);
   %this.setSequenceCyclic( "ProxMine_Fire", false);
   %this.setSequenceCyclic( "ProxMine_Fire_Release", false);
   %this.setSequenceCyclic( "ProxMine_Root", true);
   %this.setSequenceCyclic( "ProxMine_Run", true);
   %this.setSequenceCyclic( "ProxMine_Side", true);
   %this.setSequenceCyclic( "ProxMine_Sitting", true);
   %this.setSequenceCyclic( "ProxMine_Swim_Backward", true);
   %this.setSequenceCyclic( "ProxMine_Swim_Forward", true);
   %this.setSequenceCyclic( "ProxMine_Swim_Root", true);
   %this.setSequenceCyclic( "ProxMine_Swim_Left", true);
   %this.setSequenceCyclic( "ProxMine_Swim_Right", true);

   %this.setSequenceBlend( "ProxMine_Head", "1", "ProxMine_Root", "0");
   %this.setSequenceBlend( "ProxMine_Look", "1", "ProxMine_Root", "0");
   %this.setSequenceBlend( "ProxMine_Reload", "1", "ProxMine_Root", "0");
   %this.setSequenceBlend( "ProxMine_Fire", "1", "ProxMine_Root", "0");
   %this.setSequenceBlend( "ProxMine_Fire_Release", "1", "ProxMine_Root", "0");

   %this.setSequenceGroundSpeed( "ProxMine_Back", "0 -3.6 0");
   %this.setSequenceGroundSpeed( "ProxMine_Run", "0 5.0 0");
   %this.setSequenceGroundSpeed( "ProxMine_Side", "3.6 0 0");
   %this.setSequenceGroundSpeed( "ProxMine_Swim_Backward", "0 -1 0");
   %this.setSequenceGroundSpeed( "ProxMine_Swim_Forward", "0 1 0");
   %this.setSequenceGroundSpeed( "ProxMine_Swim_Left", "-1 0 0");
   %this.setSequenceGroundSpeed( "ProxMine_Swim_Right", "1 0 0");
   %this.setSequenceGroundSpeed( "ProxMine_Crouch_Backward", "0 -2 0");
   %this.setSequenceGroundSpeed( "ProxMine_Crouch_Forward", "0 2 0");
   %this.setSequenceGroundSpeed( "ProxMine_Crouch_Side", "1 0 0");
   // END: ProxMine Sequences

   // BEGIN: Turret Sequences
   %this.addSequence( "art/shapes/weapons/Turret/PlayerAnims/PlayerAnim_Turret_Back.dae Back", "Turret_Back", 0, -1);
   %this.addSequence( "art/shapes/weapons/Turret/PlayerAnims/PlayerAnim_Turret_Crouch_Root.dae Crouch_Root", "Turret_Crouch_Root", 0, -1);
   %this.addSequence( "art/shapes/weapons/Turret/PlayerAnims/PlayerAnim_Turret_Crouch_Backward.dae Crouch_Backward", "Turret_Crouch_Backward", 0, -1);
   %this.addSequence( "art/shapes/weapons/Turret/PlayerAnims/PlayerAnim_Turret_Crouch_Forward.dae Crouch_Forward", "Turret_Crouch_Forward", 0, -1);
   %this.addSequence( "art/shapes/weapons/Turret/PlayerAnims/PlayerAnim_Turret_Crouch_Side.dae Crouch_Side", "Turret_Crouch_Side", 0, -1);
   %this.addSequence( "art/shapes/weapons/Turret/PlayerAnims/PlayerAnim_Turret_Death1.dae Death1", "Turret_Death1", 0, -1);
   %this.addSequence( "art/shapes/weapons/Turret/PlayerAnims/PlayerAnim_Turret_Death2.dae Death2", "Turret_Death2", 0, -1);
   %this.addSequence( "art/shapes/weapons/Turret/PlayerAnims/PlayerAnim_Turret_Fall.dae Fall", "Turret_Fall", 0, -1);
   %this.addSequence( "art/shapes/weapons/Turret/PlayerAnims/PlayerAnim_Turret_Run.dae Run", "Turret_Run", 0, -1);
   %this.addSequence( "art/shapes/weapons/Turret/PlayerAnims/PlayerAnim_Turret_Jump.dae Jump", "Turret_Jump", 0, -1);
   %this.addSequence( "art/shapes/weapons/Turret/PlayerAnims/PlayerAnim_Turret_Land.dae Land", "Turret_Land", 0, -1);
   %this.addSequence( "art/shapes/weapons/Turret/PlayerAnims/PlayerAnim_Turret_Look.dae Look", "Turret_Look", 0, -1);
   %this.addSequence( "art/shapes/weapons/Turret/PlayerAnims/PlayerAnim_Turret_Head.dae Head", "Turret_Head", 0, -1);
   %this.addSequence( "art/shapes/weapons/Turret/PlayerAnims/PlayerAnim_Turret_Recoil.dae Recoil", "Turret_Recoil", 0, -1);
   %this.addSequence( "art/shapes/weapons/Turret/PlayerAnims/PlayerAnim_Turret_Fire_Release.dae Fire_Release", "Turret_Fire_Release", 0, -1);
   %this.addSequence( "art/shapes/weapons/Turret/PlayerAnims/PlayerAnim_Turret_Root.dae Root", "Turret_Root", 0, -1);
   %this.addSequence( "art/shapes/weapons/Turret/PlayerAnims/PlayerAnim_Turret_Side.dae Side", "Turret_Side", 0, -1);
   %this.addSequence( "art/shapes/weapons/Turret/PlayerAnims/PlayerAnim_Turret_Sitting.dae Sitting", "Turret_Sitting", 0, -1);
   %this.addSequence( "art/shapes/weapons/Turret/PlayerAnims/PlayerAnim_Turret_Swim_Backward.dae Swim_Backward", "Turret_Swim_Backward", 0, -1);
   %this.addSequence( "art/shapes/weapons/Turret/PlayerAnims/PlayerAnim_Turret_Swim_Forward.dae Swim_Forward", "Turret_Swim_Forward", 0, -1);
   %this.addSequence( "art/shapes/weapons/Turret/PlayerAnims/PlayerAnim_Turret_Swim_Root.dae Swim_Root", "Turret_Swim_Root", 0, -1);
   %this.addSequence( "art/shapes/weapons/Turret/PlayerAnims/PlayerAnim_Turret_Swim_Left.dae Swim_Left", "Turret_Swim_Left", 0, -1);
   %this.addSequence( "art/shapes/weapons/Turret/PlayerAnims/PlayerAnim_Turret_Swim_Right.dae Swim_Right", "Turret_Swim_Right", 0, -1);
   
   %this.setSequenceCyclic( "Turret_Back", true);
   %this.setSequenceCyclic( "Turret_Crouch_Backward", true);
   %this.setSequenceCyclic( "Turret_Crouch_Forward", true);
   %this.setSequenceCyclic( "Turret_Crouch_Side", true);
   %this.setSequenceCyclic( "Turret_Death1", false);
   %this.setSequenceCyclic( "Turret_Death2", false);
   %this.setSequenceCyclic( "Turret_Fall", true);
   %this.setSequenceCyclic( "Turret_Run", true);
   %this.setSequenceCyclic( "Turret_Jump", false);
   %this.setSequenceCyclic( "Turret_Land", false);
   %this.setSequenceCyclic( "Turret_Look", false);
   %this.setSequenceCyclic( "Turret_Head", false);
   %this.setSequenceCyclic( "Turret_Recoil", false);
   %this.setSequenceCyclic( "Turret_Fire_Release", false);
   %this.setSequenceCyclic( "Turret_Root", true);
   %this.setSequenceCyclic( "Turret_Side", true);
   %this.setSequenceCyclic( "Turret_Sitting", true);
   %this.setSequenceCyclic( "Turret_Swim_Backward", true);
   %this.setSequenceCyclic( "Turret_Swim_Forward", true);
   %this.setSequenceCyclic( "Turret_Swim_Root", true);
   %this.setSequenceCyclic( "Turret_Swim_Left", true);
   %this.setSequenceCyclic( "Turret_Swim_Right", true);
   
   %this.setSequenceBlend( "Turret_Head", "1", "Turret_Root", "0");
   %this.setSequenceBlend( "Turret_Look", "1", "Turret_Root", "0");
   %this.setSequenceBlend( "Turret_Recoil", "1", "Turret_Root", "0");
   %this.setSequenceBlend( "Turret_Fire_Release", "1", "Turret_Root", "0");
   
   %this.setSequenceGroundSpeed( "Turret_Back", "0 -3.6 0");
   %this.setSequenceGroundSpeed( "Turret_Run", "0 5.0 0");
   %this.setSequenceGroundSpeed( "Turret_Side", "3.6 0 0");
   %this.setSequenceGroundSpeed( "Turret_Swim_Backward", "0 -1 0");
   %this.setSequenceGroundSpeed( "Turret_Swim_Forward", "0 1 0");
   %this.setSequenceGroundSpeed( "Turret_Swim_Left", "-1 0 0");
   %this.setSequenceGroundSpeed( "Turret_Swim_Right", "1 0 0");
   %this.setSequenceGroundSpeed( "Turret_Crouch_Backward", "0 -2 0");
   %this.setSequenceGroundSpeed( "Turret_Crouch_Forward", "0 2 0");
   %this.setSequenceGroundSpeed( "Turret_Crouch_Side", "1 0 0");
   // END: Turret Sequences
}
