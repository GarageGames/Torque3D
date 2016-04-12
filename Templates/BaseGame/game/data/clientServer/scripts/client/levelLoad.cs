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


// Whether the local client is currently running a mission.
$Client::missionRunning = false;

// Sequence number for currently running mission.
$Client::missionSeq = -1;


// Called when mission is started.
function clientStartMission()
{
   // The client recieves a mission start right before
   // being dropped into the game.
   physicsStartSimulation( "client" );
   
   // Start game audio effects channels.
   
   AudioChannelEffects.play();
   
   // Create client mission cleanup group.
      
   new SimGroup( ClientMissionCleanup );

   // Done.
      
   $Client::missionRunning = true;
}

// Called when mission is ended (either through disconnect or
// mission end client command).
function clientEndMission()
{
   // Stop physics simulation on client.
   physicsStopSimulation( "client" );

   // Stop game audio effects channels.
   
   AudioChannelEffects.stop();
   
   // Delete all the decals.
   decalManagerClear();
  
   // Delete client mission cleanup group. 
   if( isObject( ClientMissionCleanup ) )
      ClientMissionCleanup.delete();
      
   clearClientPaths();
      
   // Done.
   $Client::missionRunning = false;
}

//----------------------------------------------------------------------------
// Mission start / end events sent from the server
//----------------------------------------------------------------------------

function clientCmdMissionStart(%seq)
{
   clientStartMission();
   $Client::missionSeq = %seq;
}

function clientCmdMissionEnd( %seq )
{
   if( $Client::missionRunning && $Client::missionSeq == %seq )
   {
      clientEndMission();
      $Client::missionSeq = -1;
   }
}
