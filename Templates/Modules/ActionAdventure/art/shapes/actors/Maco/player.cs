//-----------------------------------------------------------------------------
// 3D Action Adventure Kit for T3D
// Copyright (C) Ubiq Visuals, Inc.
// http://www.ubiqvisuals.com/
//-----------------------------------------------------------------------------

singleton TSShapeConstructor(PlayerDts)
{
   baseShape = "./player.dts";
};         

function PlayerDts::onLoad(%this)
{
   %this.addSequence("./player_root.dsq", "root", "0", "-1", "1", "0");
   %this.addSequence("./player_forward.dsq", "run", "0", "-1", "1", "0");
   %this.addSequence("./player_walk.dsq", "walk", "0", "-1", "1", "0");
   %this.addSequence("./player_standjump.dsq", "standjump", "0", "-1", "1", "0");
   %this.addSequence("./player_land.dsq", "land", "0", "-1", "1", "0");
   %this.addSequence("./player_jump.dsq", "jump", "0", "-1", "1", "0");
   %this.addSequence("./player_jumpland.dsq", "jumpland", "0", "-1", "1", "0");
   %this.addSequence("./player_fall.dsq", "fall", "0", "-1", "1", "0");
   %this.addSequence("./player_climbidle.dsq", "climbidle", "0", "-1", "1", "0");
   %this.addSequence("./player_climbright.dsq", "climbright", "0", "-1", "1", "0");
   %this.addSequence("./player_climbleft.dsq", "climbleft", "0", "-1", "1", "0");
   %this.addSequence("./player_climbup.dsq", "climbup", "0", "-1", "1", "0");
   %this.addSequence("./player_climbdown.dsq", "climbdown", "0", "-1", "1", "0");
   %this.addSequence("./player_ledgeidle.dsq", "ledgeidle", "0", "-1", "1", "0");
   %this.addSequence("./player_ledgeright.dsq", "ledgeright", "0", "-1", "1", "0");
   %this.addSequence("./player_ledgeleft.dsq", "ledgeleft", "0", "-1", "1", "0");
   %this.addSequence("./player_wallleft.dsq", "wallleft", "0", "-1", "1", "0");
   %this.addSequence("./player_wallright.dsq", "wallright", "0", "-1", "1", "0");
   %this.addSequence("./player_stop.dsq", "stop", "0", "-1", "1", "0");
   %this.addSequence("./player_slidefront.dsq", "slidefront", "0", "-1", "1", "0");
   %this.addSequence("./player_slideback.dsq", "slideback", "0", "-1", "1", "0");
   %this.addSequence("./player_ledgeup.dsq", "ledgeup", "0", "-1", "1", "0");
   %this.addSequence("./player_death.dsq", "death1", "0", "-1", "1", "0");
   %this.addSequence("./player_wallidle.dsq", "wallidle", "0", "-1", "1", "0");
   %this.addSequence("./player_root.dsq", "ambient", "0", "-1", "1", "0");
   
   //Ubiq: Fixing up some triggers (these should really be changed in the .dsq)
   %this.removeTrigger("jumpland", "3", "1");
   %this.removeTrigger("jumpland", "3", "2");
   %this.removeTrigger("jumpland", "4", "1");
   %this.removeTrigger("jumpland", "4", "2");
   %this.removeTrigger("jumpland", "5", "1");
   %this.removeTrigger("jumpland", "5", "2");
   %this.removeTrigger("jumpland", "6", "1");
   %this.removeTrigger("jumpland", "6", "2");
   
   %this.addTrigger("stop", "1", "1");
   %this.addTrigger("stop", "1", "2");
   %this.removeTrigger("stop", "3", "1");
   %this.removeTrigger("stop", "3", "2");
   %this.removeTrigger("stop", "5", "1");
   %this.removeTrigger("stop", "5", "2");
   %this.removeTrigger("stop", "7", "1");
   %this.removeTrigger("stop", "7", "2");
   %this.removeTrigger("stop", "9", "1");
   %this.removeTrigger("stop", "9", "2");
   %this.removeTrigger("stop", "11", "2");
   %this.removeTrigger("stop", "11", "1");
   %this.removeTrigger("stop", "13", "1");
   %this.removeTrigger("stop", "13", "2");
   %this.addTrigger("stop", "16", "2");
}
