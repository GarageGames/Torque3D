//-----------------------------------------------------------------------------
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
//
// Much of this script was taken directly from the PathShape resource:
// http://www.garagegames.com/community/resource/view/20385/1
// With additional improvements by Azaezel:
// http://www.garagegames.com/community/forums/viewthread/137195
// ----------------------------------------------------------------------------

// TSPathShape callback functions. You can override any of these functions for
// an individual shape by copying the function and replacing 'TSPathShape::'
// with 'YourShapeName::' 
function TSPathShape::onAdd(%this)
{
   if ( isObject(%this.path) )
   {
      %this.pushPath(%this.path);
      %this.setPathPosition(0);
   }
}

function TSPathShape::onPathChange(%this)
{
   // Save the current position and move state before resetting the path
   %pos = %this.getPathPosition();
   %moveState = %this.movementState;

   %this.reset(0, false); // Stop and reset the shapes path, don't create first node

   if ( !isObject(%this.path) )
      return;

   %this.pushPath(%this.path);
   %this.setPathPosition(%pos);
   %this.setMoveState(%moveState);
}

function TSPathShape::onNode(%this, %node)
{
   //echo("TSPathShape::onNode(" @ %this @ ", " @ %node @ ")");
}

function TSPathShape::pushPath(%this, %path)
{
   // Install the new path
   if ( %path.speed $= "" )
      %path.speed = 2;

   for (%i = 0; %i < %path.getCount(); %i++)
      %this.pushNode(%path.getObject(%i));

   // If looping, push the starting node onto the end to make this a loop
   if( %this.path.isLooping )
      %this.pushNode(%path.getObject(0));
   %this.setLooping(%this.path.isLooping);
}

function TSPathShape::pushNode(%this, %node)
{
   %speed = (%node.speed !$= "") ? %node.speed : %this.path.speed;
   if ((%type = %node.type) $= "")
      %type = "Normal";
   if ((%smoothing = %node.smoothingType) $= "")
      %smoothing = "Linear";
   %this.pushBack(%node.getTransform(), %speed, %type, %smoothing);
   %this.nodeCount = %this.getNodeCount();
}
