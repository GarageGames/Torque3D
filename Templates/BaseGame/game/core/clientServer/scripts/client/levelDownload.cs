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

//-----------------------------------------------------------------------------
// Mission Loading
// The client portion of the client/server mission loading process
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------
// Loading Phases:
// Phase 1: Transmit Datablocks
//          Transmit targets
// Phase 2: Transmit Ghost Objects
// Phase 3: Start Game
//
// The server invokes the client MissionStartPhase[1-3] function to request
// permission to start each phase.  When a client is ready for a phase,
// it responds with MissionStartPhase[1-3]Ack.

//----------------------------------------------------------------------------
// Phase 1 
//----------------------------------------------------------------------------
function clientCmdMissionStartPhase1(%seq, %missionName)
{
   // These need to come after the cls.
   echo ("*** New Mission: " @ %missionName);
   echo ("*** Phase 1: Download Datablocks & Targets");
   
   //Prep the postFX stuff
   // Load the post effect presets for this mission.
   %path = filePath( %missionName ) @ "/" @ fileBase( %missionName ) @ $PostFXManager::fileExtension;

   if ( isScriptFile( %path ) )
   {
      postFXManager::loadPresetHandler( %path ); 
   }
   else
   {
      PostFXManager::settingsApplyDefaultPreset();
   }
   
   onMissionDownloadPhase("LOADING DATABLOCKS");
   
   commandToServer('MissionStartPhase1Ack', %seq);
}

function onDataBlockObjectReceived(%index, %total)
{
   onMissionDownloadProgress(%index / %total);
}

//----------------------------------------------------------------------------
// Phase 2
//----------------------------------------------------------------------------
function clientCmdMissionStartPhase2(%seq,%missionName)
{
   onPhaseComplete();
   echo ("*** Phase 2: Download Ghost Objects");
   
   onMissionDownloadPhase("LOADING OBJECTS");
   
   commandToServer('MissionStartPhase2Ack', %seq);
}

function onGhostAlwaysStarted(%ghostCount)
{
   $ghostCount = %ghostCount;
   $ghostsRecvd = 0;
}

function onGhostAlwaysObjectReceived()
{
   $ghostsRecvd++;
   onMissionDownloadProgress($ghostsRecvd / $ghostCount);
}  

//----------------------------------------------------------------------------
// Phase 3
//----------------------------------------------------------------------------
function clientCmdMissionStartPhase3(%seq,%missionName)
{
   onPhaseComplete();
   StartClientReplication();
   StartFoliageReplication();
   
   // Load the static mission decals.
   if(isFile(%missionName @ ".decals"))
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
      
      onMissionDownloadPhase("LIGHTING MISSION");
      
      $lightingMission = true;
   }
}

function updateLightingProgress()
{
   onMissionDownloadProgress($SceneLighting::lightingProgress);
   if ($lightingMission)
      $lightingProgressThread = schedule(1, 0, "updateLightingProgress");
}

function sceneLightingComplete()
{
   echo("Mission lighting done");
   $lightingMission = false;
   
   onPhaseComplete("STARTING MISSION");
   
   // The is also the end of the mission load cycle.
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

function onMissionDownloadPhase(%phase)
{
   if ( !isObject( LoadingProgress ) )
      return;
      
   LoadingProgress.setValue(0);
   LoadingProgressTxt.setValue(%phase);
   Canvas.repaint();
}

function onMissionDownloadProgress(%progress)
{
   if ( !isObject( LoadingProgress ) )
      return;
      
   LoadingProgress.setValue(%progress);
   Canvas.repaint(33);
}

function onPhaseComplete(%text)
{
   if ( !isObject( LoadingProgress ) )
      return;
	  
   if(%text !$= "")
      LoadingProgressTxt.setValue(%text);
      
   LoadingProgress.setValue( 1 );
   Canvas.repaint();
}