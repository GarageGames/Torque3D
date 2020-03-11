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

// These scripts make use of dynamic attribute values on Item datablocks,
// these are as follows:
//
//    maxInventory      Max inventory per object (100 bullets per box, etc.)
//    pickupName        Name to display when client pickups item
//
// Item objects can have:
//
//    count             The # of inventory items in the object.  This
//                      defaults to maxInventory if not set.

// Respawntime is the amount of time it takes for a static "auto-respawn"
// object, such as an ammo box or weapon, to re-appear after it's been
// picked up.  Any item marked as "static" is automaticlly respawned.
$Item::RespawnTime = 30 * 1000;

// Poptime represents how long dynamic items (those that are thrown or
// dropped) will last in the world before being deleted.
$Item::PopTime = 30 * 1000;

//-----------------------------------------------------------------------------
// ItemData base class methods used by all items
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

function Item::respawn(%this)
{
   // This method is used to respawn static ammo and weapon items
   // and is usually called when the item is picked up.
   // Instant fade...
   %this.startFade(0, 0, true);
   %this.setHidden(true);

   // Shedule a reapearance
   %this.schedule($Item::RespawnTime, "setHidden", false);
   %this.schedule($Item::RespawnTime + 100, "startFade", 1000, 0, false);
}

function Item::schedulePop(%this)
{
   // This method deletes the object after a default duration. Dynamic
   // items such as thrown or drop weapons are usually popped to avoid
   // world clutter.
   %this.schedule($Item::PopTime - 1000, "startFade", 1000, 0, true);
   %this.schedule($Item::PopTime, "delete");
}

//-----------------------------------------------------------------------------
// Callbacks to hook items into the inventory system

function ItemData::onThrow(%this, %user, %amount)
{
   // Remove the object from the inventory
   if (%amount $= "")
      %amount = 1;
   if (%this.maxInventory !$= "")
      if (%amount > %this.maxInventory)
         %amount = %this.maxInventory;
   if (!%amount)
      return 0;
   %user.decInventory(%this,%amount);

   // Construct the actual object in the world, and add it to
   // the mission group so it's cleaned up when the mission is
   // done.  The object is given a random z rotation.
   %obj = new Item()
   {
      datablock = %this;
      rotation = "0 0 1 "@ (getRandom() * 360);
      count = %amount;
   };
   MissionGroup.add(%obj);
   %obj.schedulePop();
   return %obj;
}

function ItemData::onPickup(%this, %obj, %user, %amount)
{
    // Add it to the inventory, this currently ignores the request
    // amount, you get what you get.  If the object doesn't have
    // a count or the datablock doesn't have maxIventory set, the
    // object cannot be picked up.

    // See if the object has a count
    %count = %obj.count;
    if (%count $= "")
    {
       // No, so check the datablock
       %count = %this.count;
       if (%count $= "")
       {
          // No, so attempt to provide the maximum amount
          if (%this.maxInventory !$= "")
          {
             if (!(%count = %this.maxInventory))
                return;
          }
          else
             %count = 1;
       }
    }
    
    %user.incInventory(%this, %count);

    // Inform the client what they got.
    if (%user.client)
       messageClient(%user.client, 'MsgItemPickup', '\c0You picked up %1', %this.pickupName);

    // If the item is a static respawn item, then go ahead and
    // respawn it, otherwise remove it from the world.
    // Anything not taken up by inventory is lost.
    if (%obj.isStatic())
       %obj.respawn();
    else
       %obj.delete();
    return true;
}

function ItemData::createItem(%data)
{
   %obj = new Item()
   {
      dataBlock = %data;
      static = true;
      rotate = true;
   };
   return %obj;
}
