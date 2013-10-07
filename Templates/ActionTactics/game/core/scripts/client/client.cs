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

function initBaseClient()
{
   // Base client functionality
   exec( "./message.cs" );
   exec( "./mission.cs" );
   exec( "./missionDownload.cs" );
   exec( "./actionMap.cs" );
   exec( "./renderManager.cs" );
   exec( "./lighting.cs" );
   
   initRenderManager();
   initLightingSystems();   
}

/// A helper function which will return the ghosted client object
/// from a server object when connected to a local server.
function serverToClientObject( %serverObject )
{
   assert( isObject( LocalClientConnection ), "serverToClientObject() - No local client connection found!" );
   assert( isObject( ServerConnection ), "serverToClientObject() - No server connection found!" );      
         
   %ghostId = LocalClientConnection.getGhostId( %serverObject );
   if ( %ghostId == -1 )
      return 0;
                
   return ServerConnection.resolveGhostID( %ghostId );   
}

//----------------------------------------------------------------------------
// Debug commands
//----------------------------------------------------------------------------

function netSimulateLag( %msDelay, %packetLossPercent )
{
   if ( %packetLossPercent $= "" )
      %packetLossPercent = 0;
                  
   commandToServer( 'NetSimulateLag', %msDelay, %packetLossPercent );
}

