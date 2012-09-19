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

// Inventory items.  These objects rely on the item & inventory support
// system defined in item.cs and inventory.cs

//-----------------------------------------------------------------------------
// Health Patches cannot be picked up and are not meant to be added to inventory.
// Health is applied automatically when an objects collides with a patch.
//-----------------------------------------------------------------------------

function HealthPatch::onCollision(%this, %obj, %col)
{
   // Apply health to colliding object if it needs it.
   // Works for all shapebase objects.
   if (%col.getDamageLevel() != 0 && %col.getState() !$= "Dead")
   {
      %col.applyRepair(%this.repairAmount);

      // Update the Health GUI while repairing
      %this.doHealthUpdate(%col);

      %obj.respawn();
      if (%col.client)
         messageClient(%col.client, 'MsgHealthPatchUsed', '\c2Health Patch Applied');
      serverPlay3D(HealthUseSound, %obj.getTransform());
   }
}

function HealthPatch::doHealthUpdate(%this, %obj)
{
   // Would be better to add a onRepair() callback to shapeBase.cpp in order to
   // prevent any excess/unneccesary schedules from this.  But for the time
   // being....

   // This is just a rough timer to update the Health HUD every 250 ms.  From
   // my tests a large health pack will fully heal a player from 10 health in
   // 36 iterations (ie. 9 seconds).  If either the scheduling time, the repair
   // amount, or the repair rate is changed then the healthTimer counter should
   // be changed also.

   if (%obj.healthTimer < 40) // 40 = 10 seconds at 1 iteration per 250 ms.
   {
      %obj.UpdateHealth();
      %this.schedule(250, doHealthUpdate, %obj);
      %obj.healthTimer++;
   }
   else
      %obj.healthTimer = 0;
}

function ShapeBase::tossPatch(%this)
{
   //error("ShapeBase::tossPatch(" SPC %this.client.nameBase SPC ")");
   if(!isObject(%this))
      return;

   %item = ItemData::createItem(HealthKitPatch);
   %item.sourceObject = %this;
   %item.static = false;
   MissionCleanup.add(%item);

   %vec = (-1.0 + getRandom() * 2.0) SPC (-1.0 + getRandom() * 2.0) SPC getRandom();
   %vec = vectorScale(%vec, 10);
   %eye = %this.getEyeVector();
   %dot = vectorDot("0 0 1", %eye);
   if (%dot < 0)
      %dot = -%dot;
   %vec = vectorAdd(%vec, vectorScale("0 0 8", 1 - %dot));
   %vec = vectorAdd(%vec, %this.getVelocity());
   %pos = getBoxCenter(%this.getWorldBox());

   %item.setTransform(%pos);
   %item.applyImpulse(%pos, %vec);
   %item.setCollisionTimeout(%this);
   //serverPlay3D(%item.getDataBlock().throwSound, %item.getTransform());
   %item.schedulePop();

   return %item;
}
