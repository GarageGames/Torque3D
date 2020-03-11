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

// This inventory system is totally scripted, no C++ code involved.
// It uses object datablock names to track inventory and is generally
// object type, or class, agnostic.  In other words, it will inventory
// any kind of ShapeBase object, though the throw method does assume
// that the objects are small enough to throw :)
//
// For a ShapeBase object to support inventory, it must have an array
// of inventory max values:
//    %this.maxInv[GunAmmo] = 100;
//    %this.maxInv[SpeedGun] = 1;
// where the names "SpeedGun" and "GunAmmo" are datablocks.
//
// For objects to be inventoriable, they must provide a set of inventory
// callback methods, mainly:
//    onUse
//    onThrow
//    onPickup
//
// Example methods are given further down.  The item.cs file also contains
// example inventory items.

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Inventory server commands
//-----------------------------------------------------------------------------

function serverCmdUse(%client, %data)
{
   %client.getControlObject().use(%data);
}

//-----------------------------------------------------------------------------
// ShapeBase inventory support
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

function ShapeBase::use(%this, %data)
{
   // Use an object in the inventory.

   // Need to prevent weapon changing when zooming, but only shapes
   // that have a connection.
   %conn = %this.getControllingClient();
   if (%conn)
   {
      %defaultFov = %conn.getControlCameraDefaultFov();
      %fov = %conn.getControlCameraFov();
      if (%fov != %defaultFov)
         return false;
   }

   if (%this.getInventory(%data) > 0)
      return %data.onUse(%this);

   return false;
}

function ShapeBase::throw(%this, %data, %amount)
{
   // Throw objects from inventory. The onThrow method is
   // responsible for decrementing the inventory.

   if (%this.getInventory(%data) > 0)
   {
      %obj = %data.onThrow(%this, %amount);
      if (%obj)
      {
         %this.throwObject(%obj);
         serverPlay3D(ThrowSnd, %this.getTransform());
         return true;
      }
   }
   return false;
}

function ShapeBase::pickup(%this, %obj, %amount)
{
   // This method is called to pickup an object and add it to the inventory.
   // The datablock onPickup method is actually responsible for doing all the
   // work, including incrementing the inventory.

   %data = %obj.getDatablock();

   // Try and pickup the max if no value was specified
   if (%amount $= "")
      %amount = %this.maxInventory(%data) - %this.getInventory(%data);

   // The datablock does the work...
   if (%amount < 0)
      %amount = 0;
   if (%amount)
      return %data.onPickup(%obj, %this, %amount);
   return false;
}

//-----------------------------------------------------------------------------

function ShapeBase::hasInventory(%this, %data)
{
   return (%this.inv[%data] > 0);
}

function ShapeBase::hasAmmo(%this, %weapon)
{
   if (%weapon.image.ammo $= "")
      return(true);
   else
      return(%this.getInventory(%weapon.image.ammo) > 0);
}

function ShapeBase::maxInventory(%this, %data)
{
   if (%data.isField("clip"))
   {
      // Use the clip system which uses the maxInventory
      // field on the ammo itself.
      return %data.maxInventory;
   }
   else
   {
      // Use the ammo pool system which uses the maxInv[]
      // array on the object's datablock.
      // If there is no limit defined, we assume 0
      return %this.getDatablock().maxInv[%data.getName()];
   }
}

function ShapeBase::incInventory(%this, %data, %amount)
{
   // Increment the inventory by the given amount.  The return value
   // is the amount actually added, which may be less than the
   // requested amount due to inventory restrictions.

   %max = %this.maxInventory(%data);
   %total = %this.inv[%data.getName()];
   if (%total < %max)
   {
      if (%total + %amount > %max)
         %amount = %max - %total;
      %this.setInventory(%data, %total + %amount);
      return %amount;
   }
   return 0;
}

function ShapeBase::decInventory(%this, %data, %amount)
{
   // Decrement the inventory by the given amount. The return value
   // is the amount actually removed.

   %total = %this.inv[%data.getName()];
   if (%total > 0)
   {
      if (%total < %amount)
         %amount = %total;
      %this.setInventory(%data, %total - %amount);
      return %amount;
   }
   return 0;
}

//-----------------------------------------------------------------------------

function ShapeBase::getInventory(%this, %data)
{
   // Return the current inventory amount
   return %this.inv[%data.getName()];
}

function ShapeBase::setInventory(%this, %data, %value)
{
   // Set the inventory amount for this datablock and invoke inventory
   // callbacks.  All changes to inventory go through this single method.

   // Impose inventory limits
   if (%value < 0)
      %value = 0;
   else
   {
      %max = %this.maxInventory(%data);
      if (%value > %max)
         %value = %max;
   }

   // Set the value and invoke object callbacks
   %name = %data.getName();
   if (%this.inv[%name] != %value)
   {
      %this.inv[%name] = %value;
      %data.onInventory(%this, %value);
      %this.getDataBlock().onInventory(%data, %value);
   }
   return %value;
}

//-----------------------------------------------------------------------------

function ShapeBase::clearInventory(%this)
{
   // To be filled in...
}

//-----------------------------------------------------------------------------

function ShapeBase::throwObject(%this, %obj)
{
   // Throw the given object in the direction the shape is looking.
   // The force value is hardcoded according to the current default
   // object mass and mission gravity (20m/s^2).

   %throwForce = %this.getDataBlock().throwForce;
   if (!%throwForce)
   %throwForce = 20;

   // Start with the shape's eye vector...
   %eye = %this.getEyeVector();
   %vec = vectorScale(%eye, %throwForce);

   // Add a vertical component to give the object a better arc
   %verticalForce = %throwForce / 2;
   %dot = vectorDot("0 0 1", %eye);
   if (%dot < 0)
      %dot = -%dot;
   %vec = vectorAdd(%vec, vectorScale("0 0 "@%verticalForce, 1 - %dot));

   // Add the shape's velocity
   %vec = vectorAdd(%vec, %this.getVelocity());

   // Set the object's position and initial velocity
   %pos = getBoxCenter(%this.getWorldBox());
   %obj.setTransform(%pos);
   %obj.applyImpulse(%pos, %vec);

   // Since the object is thrown from the center of the shape,
   // the object needs to avoid colliding with it's thrower.
   %obj.setCollisionTimeout(%this);
}

//-----------------------------------------------------------------------------
// Callback hooks invoked by the inventory system
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ShapeBase object callbacks invoked by the inventory system

function ShapeBase::onInventory(%this, %data, %value)
{
   // Invoked on ShapeBase objects whenever their inventory changes
   // for the given datablock.
}

//-----------------------------------------------------------------------------
// ShapeBase datablock callback invoked by the inventory system.

function ShapeBaseData::onUse(%this, %user)
{
   // Invoked when the object uses this datablock, should return
   // true if the item was used.

   return false;
}

function ShapeBaseData::onThrow(%this, %user, %amount)
{
   // Invoked when the object is thrown.  This method should
   // construct and return the actual mission object to be
   // physically thrown.  This method is also responsible for
   // decrementing the user's inventory.

   return 0;
}

function ShapeBaseData::onPickup(%this, %obj, %user, %amount)
{
   // Invoked when the user attempts to pickup this datablock object.
   // The %amount argument is the space in the user's inventory for
   // this type of datablock.  This method is responsible for
   // incrementing the user's inventory is something is addded.
   // Should return true if something was added to the inventory.

   return false;
}

function ShapeBaseData::onInventory(%this, %user, %value)
{
   // Invoked whenever an user's inventory total changes for
   // this datablock.
}
