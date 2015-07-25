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

//----------------------------------------------------------------------------
// Mission Loading
// Server download handshaking.  This produces a number of onPhaseX
// calls so the game scripts can update the game's GUI.
//
// Loading Phases:
// Phase 1: Download Datablocks
// Phase 2: Download Ghost Objects
// Phase 3: Scene Lighting
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Phase 1 
//----------------------------------------------------------------------------

function clientCmdMissionStartPhase1(%seq, %missionName, %musicTrack)
{
   // These need to come after the cls.
   echo ("*** New Mission: " @ %missionName);
   echo ("*** Phase 1: Download Datablocks & Targets");
   onMissionDownloadPhase1(%missionName, %musicTrack);
   commandToServer('MissionStartPhase1Ack', %seq);
}

function onDataBlockObjectReceived(%index, %total)
{
   onPhase1Progress(%index / %total);
}

//----------------------------------------------------------------------------
// Phase 2
//----------------------------------------------------------------------------

function clientCmdMissionStartPhase2(%seq,%missionName)
{
   onPhase1Complete();
   echo ("*** Phase 2: Download Ghost Objects");
   onMissionDownloadPhase2(%missionName);
   commandToServer('MissionStartPhase2Ack', %seq, $pref::Player:PlayerDB);
}

function onGhostAlwaysStarted(%ghostCount)
{
   $ghostCount = %ghostCount;
   $ghostsRecvd = 0;
}

function onGhostAlwaysObjectReceived()
{
   $ghostsRecvd++;
   onPhase2Progress($ghostsRecvd / $ghostCount);
}

//----------------------------------------------------------------------------
// Phase 3
//----------------------------------------------------------------------------

function clientCmdMissionStartPhase3(%seq,%missionName)
{
   onPhase2Complete();
   StartClientReplication();
   StartFoliageReplication();
   
   // Load the static mission decals.
   decalManagerLoad( %missionName @ ".decals" );
   
   echo ("*** Phase 3: Mission Lighting");
   $MSeq = %seq;
   $Client::MissionFile = %missionName;

   // Need to light the mission before we are ready.
   // The sceneLightingComplete function will complete the handshake 
   // once the scene lighting is done.
   if (lightScene("sceneLightingComplete", ""))
   {
      echo("Lighting mission....");
      schedule(1, 0, "updateLightingProgress");
      onMissionDownloadPhase3(%missionName);
      $lightingMission = true;
   }
}

function updateLightingProgress()
{
   onPhase3Progress($SceneLighting::lightingProgress);
   if ($lightingMission)
      $lightingProgressThread = schedule(1, 0, "updateLightingProgress");
}

function sceneLightingComplete()
{
   echo("Mission lighting done");
   onPhase3Complete();
   
   // The is also the end of the mission load cycle.
   onMissionDownloadComplete();
   commandToServer('MissionStartPhase3Ack', $MSeq);
}

//----------------------------------------------------------------------------
// Helper functions
//----------------------------------------------------------------------------

function connect(%server)
{
   %conn = new GameConnection(ServerConnection);
   RootGroup.add(ServerConnection);
   %conn.setConnectArgs($pref::Player::Name);
   %conn.setJoinPassword($Client::Password);
   %conn.connect(%server);
}
