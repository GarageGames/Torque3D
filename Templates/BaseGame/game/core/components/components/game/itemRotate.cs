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

//registerComponent("ItemRotationComponent", "Component", "Item Rotation", "Game", false, "Rotates the entity around the z axis, like an item pickup.");

function ItemRotationComponent::onAdd(%this)
{
   %this.addComponentField(rotationsPerMinute, "Number of rotations per minute", "float", "5", "");
   %this.addComponentField(forward, "Rotate forward or backwards", "bool", "1", "");
   %this.addComponentField(horizontal, "Rotate horizontal or verticle, true for horizontal", "bool", "1", "");
}

function ItemRotationComponent::Update(%this)
{
    %tickRate = 0.032;
    
	//Rotations per second is calculated based on a standard update tick being 32ms. So we scale by the tick speed, then add that to our rotation to 
	//get a nice rotation speed.
	if(%this.horizontal)
	{
		if(%this.forward)
			%this.owner.rotation.z += ( ( 360 * %this.rotationsPerMinute ) / 60 ) * %tickRate;
		else
			%this.owner.rotation.z -= ( ( 360 * %this.rotationsPerMinute ) / 60 ) * %tickRate;
	}
	else
	{
		%this.owner.rotation.x += ( ( 360 * %this.rotationsPerMinute ) / 60 ) * %tickRate;
	}
}