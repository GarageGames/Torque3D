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

// Parenting is in place for WheeledVehicleData to VehicleData.  This should
// make it easier for people to simply drop in new (generic) vehicles.  All that
// the user needs to create is a set of datablocks for the new wheeled vehicle
// to use.  This means that no (or little) scripting should be necessary.

// Special, or unique vehicles however will still require some scripting.  They
// may need to override the onAdd() function in order to mount weapons,
// differing tires/springs, etc., almost everything else is taken care of in the
// WheeledVehicleData and VehicleData methods.  This helps us by not having to
// duplicate the same code for every new vehicle.

// In theory this would work for HoverVehicles and FlyingVehicles also, but
// hasn't been tested or fully implemented for those classes -- yet.

function VehicleData::onAdd(%this, %obj)
{
   %obj.setRechargeRate(%this.rechargeRate);
   %obj.setEnergyLevel(%this.MaxEnergy);
   %obj.setRepairRate(0);

   if (%obj.mountable || %obj.mountable $= "")
      %this.isMountable(%obj, true);
   else
      %this.isMountable(%obj, false);

   if (%this.nameTag !$= "")
      %obj.setShapeName(%this.nameTag);
}

function VehicleData::onRemove(%this, %obj)
{
   //echo("\c4VehicleData::onRemove("@ %this.getName() @", "@ %obj.getClassName() @")");

   // if there are passengers/driver, kick them out
   for(%i = 0; %i < %obj.getDatablock().numMountPoints; %i++)
   {
      if (%obj.getMountNodeObject(%i))
      {
         %passenger = %obj.getMountNodeObject(%i);
         %passenger.getDataBlock().doDismount(%passenger, true);
      }
   }
}

// ----------------------------------------------------------------------------
// Vehicle player mounting and dismounting
// ----------------------------------------------------------------------------

function VehicleData::isMountable(%this, %obj, %val)
{
   %obj.mountable = %val;
}

function VehicleData::mountPlayer(%this, %vehicle, %player)
{
   //echo("\c4VehicleData::mountPlayer("@ %this.getName() @", "@ %vehicle @", "@ %player.client.nameBase @")");

   if (isObject(%vehicle) && %vehicle.getDamageState() !$= "Destroyed")
   {
      %player.startFade(1000, 0, true);
      %this.schedule(1000, "setMountVehicle", %vehicle, %player);
      %player.schedule(1500, "startFade", 1000, 0, false);
   }
}

function VehicleData::setMountVehicle(%this, %vehicle, %player)
{
   //echo("\c4VehicleData::setMountVehicle("@ %this.getName() @", "@ %vehicle @", "@ %player.client.nameBase @")");

   if (isObject(%vehicle) && %vehicle.getDamageState() !$= "Destroyed")
   {
      %node = %this.findEmptySeat(%vehicle, %player);
      if (%node >= 0)
      {
         //echo("\c4Mount Node: "@ %node);
         %vehicle.mountObject(%player, %node);
         //%player.playAudio(0, MountVehicleSound);
         %player.mVehicle = %vehicle;
      }
   }
}

function VehicleData::findEmptySeat(%this, %vehicle, %player)
{
   //echo("\c4This vehicle has "@ %this.numMountPoints @" mount points.");

   for (%i = 0; %i < %this.numMountPoints; %i++)
   {
      %node = %vehicle.getMountNodeObject(%i);
      if (%node == 0)
         return %i;
   }
   return -1;
}

function VehicleData::switchSeats(%this, %vehicle, %player)
{
   for (%i = 0; %i < %this.numMountPoints; %i++)
   {
      %node = %vehicle.getMountNodeObject(%i);
      if (%node == %player || %node > 0)
         continue;

      if (%node == 0)
         return %i;
   }
   return -1;
}
