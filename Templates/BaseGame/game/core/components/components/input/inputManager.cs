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

function SetInput(%client, %device, %key, %command, %bindMap, %behav)
{
   commandToClient(%client, 'SetInput', %device, %key, %command, %bindMap, %behav);  
}

function RemoveInput(%client, %device, %key, %command, %bindMap)
{
   commandToClient(%client, 'removeInput', %device, %key, %command, %bindMap);  
}

function clientCmdSetInput(%device, %key, %command, %bindMap, %behav)
{
   //if we're requesting a custom bind map, set that up
   if(%bindMap $= "")
      %bindMap = moveMap;
   
   if (!isObject(%bindMap)){
      new ActionMap(moveMap);
      moveMap.push();
   }
      
   //get our local
   //%localID = ServerConnection.resolveGhostID(%behav); 
   
   //%tmpl = %localID.getTemplate();
   //%tmpl.insantiateNamespace(%tmpl.getName());
      
   //first, check if we have an existing command
   %oldBind = %bindMap.getBinding(%command);
   if(%oldBind !$= "")
      %bindMap.unbind(getField(%oldBind, 0), getField(%oldBind, 1));
    
   //now, set the requested bind   
   %bindMap.bind(%device, %key, %command);
}

function clientCmdRemoveSpecCtrlInput(%device, %key, %bindMap)
{
   //if we're requesting a custom bind map, set that up
   if(%bindMap $= "")
      %bindMap = moveMap;
   
   if (!isObject(%bindMap))
      return;
      
   %bindMap.unbind(%device, %key);
}

function clientCmdSetupClientBehavior(%bhvrGstID)
{
   %localID = ServerConnection.resolveGhostID(%bhvrGstID); 
   %tmpl = %localID.getTemplate();
   %tmpl.insantiateNamespace(%tmpl.getName());
}

function getMouseAdjustAmount(%val)
{
   // based on a default camera FOV of 90'
   return(%val * ($cameraFov / 90) * 0.01) * $pref::Input::LinkMouseSensitivity;
}