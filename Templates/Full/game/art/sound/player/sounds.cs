//======================================================================
// Footsteps
//======================================================================

new SFXProfile(playerFootstepSand01)
{
   filename = "art/sound/player/player_footstep_sand01.ogg";
   description = AudioClose3D;
   preload = true;
};
new SFXProfile(playerFootstepSand02)
{
   filename = "art/sound/player/player_footstep_sand02.ogg";
   description = AudioClose3D;
   preload = true;
};
new SFXProfile(playerFootstepSand03)
{
   filename = "art/sound/player/player_footstep_sand03.ogg";
   description = AudioClose3D;
   preload = true;
};
new SFXPlayList(playerFootstepSand)
{
   description = AudioEffect;
   numSlotsToPlay = 1;
   random = "OrderedRandom";
   track[0] = playerFootstepSand01;
   track[1] = playerFootstepSand02;
   track[2] = playerFootstepSand03;
   pitchScaleVariance[0] = "-0.1 0.1";
   pitchScaleVariance[1] = "-0.1 0.1";
   pitchScaleVariance[2] = "-0.1 0.1";
};

//----------------------------------------------------------------------

new SFXProfile(playerFootstepStone01)
{
   filename = "art/sound/player/player_footstep_stone01.ogg";
   description = AudioClose3D;
   preload = true;
};
new SFXProfile(playerFootstepStone02)
{
   filename = "art/sound/player/player_footstep_stone02.ogg";
   description = AudioClose3D;
   preload = true;
};
new SFXProfile(playerFootstepStone03)
{
   filename = "art/sound/player/player_footstep_stone03.ogg";
   description = AudioClose3D;
   preload = true;
};
new SFXPlayList(playerFootstepStone)
{
   description = AudioEffect;
   numSlotsToPlay = 1;
   random = "OrderedRandom";
   track[0] = playerFootstepStone01;
   track[1] = playerFootstepStone02;
   track[2] = playerFootstepStone03;
   pitchScaleVariance[0] = "-0.1 0.1";
   pitchScaleVariance[1] = "-0.1 0.1";
   pitchScaleVariance[2] = "-0.1 0.1";
};

//======================================================================
// Stopping
//======================================================================

new SFXProfile(playerStop)
{
   filename = "art/sound/player/player_stop.ogg";
   description = AudioClose3D;
   preload = true;
};

//======================================================================
// Jumping
//======================================================================

new SFXProfile(playerJumpCrouch)
{
   filename = "art/sound/player/player_jump_crouch.ogg";
   description = AudioClose3D;
   preload = true;
};

//----------------------------------------------------------------------

new SFXProfile(playerJump01)
{
   filename = "art/sound/player/player_jump01.ogg";
   description = AudioClose3D;
   preload = true;
};
new SFXProfile(playerJump02)
{
   filename = "art/sound/player/player_jump02.ogg";
   description = AudioClose3D;
   preload = true;
};
new SFXProfile(playerJump03)
{
   filename = "art/sound/player/player_jump03.ogg";
   description = AudioClose3D;
   preload = true;
};
new SFXPlayList(playerJump)
{
   description = AudioEffect;
   numSlotsToPlay = 1;
   random = "OrderedRandom";
   track[0] = playerJump01;
   track[1] = playerJump02;
   track[2] = playerJump03;
   pitchScaleVariance[0] = "-0.1 0.1";
   pitchScaleVariance[1] = "-0.1 0.1";
   pitchScaleVariance[2] = "-0.1 0.1";
};

//======================================================================
// Landing
//======================================================================

new SFXProfile(playerLand01)
{
   filename = "art/sound/player/player_land01.ogg";
   description = AudioClose3D;
   preload = true;
};
new SFXProfile(playerLand02)
{
   filename = "art/sound/player/player_land02.ogg";
   description = AudioClose3D;
   preload = true;
};
new SFXPlayList(playerLand)
{
   description = AudioEffect;
   numSlotsToPlay = 1;
   random = "OrderedRandom";
   track[0] = playerLand01;
   track[1] = playerLand02;
   pitchScaleVariance[0] = "-0.1 0.1";
   pitchScaleVariance[1] = "-0.1 0.1";
};

//======================================================================
// Climbing
//======================================================================

new SFXProfile(playerClimbIdle)
{
   filename = "art/sound/player/player_climb_idle.ogg";
   description = AudioClose3D;
   preload = true;
};

//----------------------------------------------------------------------

new SFXProfile(playerClimbUp01)
{
   filename = "art/sound/player/player_climb_up01.ogg";
   description = AudioClose3D;
   preload = true;
};
new SFXProfile(playerClimbUp02)
{
   filename = "art/sound/player/player_climb_up02.ogg";
   description = AudioClose3D;
   preload = true;
};
new SFXProfile(playerClimbUp03)
{
   filename = "art/sound/player/player_climb_up03.ogg";
   description = AudioClose3D;
   preload = true;
};
new SFXPlayList(playerClimbUp)
{
   description = AudioEffect;
   numSlotsToPlay = 1;
   random = "OrderedRandom";
   track[0] = playerClimbUp01;
   track[1] = playerClimbUp02;
   track[2] = playerClimbUp03;
   pitchScaleVariance[0] = "-0.1 0.1";
   pitchScaleVariance[1] = "-0.1 0.1";
   pitchScaleVariance[2] = "-0.1 0.1";
};

//----------------------------------------------------------------------

new SFXProfile(playerClimbDown01)
{
   filename = "art/sound/player/player_climb_down01.ogg";
   description = AudioClose3D;
   preload = true;
};
new SFXProfile(playerClimbDown02)
{
   filename = "art/sound/player/player_climb_down02.ogg";
   description = AudioClose3D;
   preload = true;
};
new SFXPlayList(playerClimbDown)
{
   description = AudioEffect;
   numSlotsToPlay = 1;
   random = "OrderedRandom";
   track[0] = playerClimbDown01;
   track[1] = playerClimbDown02;
   pitchScaleVariance[0] = "-0.1 0.1";
   pitchScaleVariance[1] = "-0.1 0.1";
};

//----------------------------------------------------------------------

new SFXProfile(playerClimbLeftRight01)
{
   filename = "art/sound/player/player_climb_leftright01.ogg";
   description = AudioClose3D;
   preload = true;
};
new SFXProfile(playerClimbLeftRight02)
{
   filename = "art/sound/player/player_climb_leftright02.ogg";
   description = AudioClose3D;
   preload = true;
};
new SFXPlayList(playerClimbLeftRight)
{
   description = AudioEffect;
   numSlotsToPlay = 1;
   random = "OrderedRandom";
   track[0] = playerClimbLeftRight01;
   track[1] = playerClimbLeftRight02;
   pitchScaleVariance[0] = "-0.1 0.1";
   pitchScaleVariance[1] = "-0.1 0.1";
};

//======================================================================
// Ledge Grabbing
//======================================================================

new SFXProfile(playerLedgeIdle)
{
   filename = "art/sound/player/player_ledge_idle.ogg";
   description = AudioClose3D;
   preload = true;
};

//----------------------------------------------------------------------

new SFXProfile(playerLedgeUp)
{
   filename = "art/sound/player/player_ledgeup.ogg";
   description = AudioClose3D;
   preload = true;
};

//----------------------------------------------------------------------

new SFXProfile(playerLedgeLeftRight01)
{
   filename = "art/sound/player/player_ledge_leftright01.ogg";
   description = AudioClose3D;
   preload = true;
};
new SFXProfile(playerLedgeLeftRight02)
{
   filename = "art/sound/player/player_ledge_leftright02.ogg";
   description = AudioClose3D;
   preload = true;
};
new SFXPlayList(playerLedgeLeftRight)
{
   description = AudioEffect;
   numSlotsToPlay = 1;
   random = "OrderedRandom";
   track[0] = playerLedgeLeftRight01;
   track[1] = playerLedgeLeftRight02;
   pitchScaleVariance[0] = "-0.1 0.1";
   pitchScaleVariance[1] = "-0.1 0.1";
};

//======================================================================
// Sliding
//======================================================================

new SFXProfile(playerSlide)
{
   filename = "art/sound/player/player_slide.ogg";
   description = AudioCloseLoop3D_FadeIn;
   preload = true;
};
