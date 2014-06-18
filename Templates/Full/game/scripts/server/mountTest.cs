datablock StaticShapeData( StaticShapeBoulder )
{	
   shapeFile = "art/shapes/rocks/boulder.dts";
};

datablock ItemData( ItemBoulder )
{	
   shapeFile = "art/shapes/rocks/boulder.dts";
};

datablock WheeledVehicleData(CustomCheetah : CheetahCar)
{
   nameTag = 'Custom Cheetah';
};

function CustomCheetah::onAdd(%this, %obj)
{
   CheetahCar::onAdd(%this, %obj);
   %obj.unmountImage(%this.turretSlot);
   
   // StaticShape
   %staticRock = new StaticShape() {
      datablock = StaticShapeBoulder;
      scale = "0.2 0.2 0.2";
   };
   %staticRock.setShapeName("StaticShape");
   %obj.staticRock = %staticRock;
   %staticRock.car = %obj;
   %obj.mountObject(%staticRock, %this.turretSlot, "1.4 0 .5 0 0 1 0");
   
   // Item
   %itemRock = new Item() {
      datablock = ItemBoulder;
      scale = "0.2 0.2 0.2";
   };
   %itemRock.setShapeName("Item");
   %obj.itemRock = %itemRock;
   %itemRock.car = %obj;
   %obj.mountObject(%itemRock, %this.turretSlot, "-1.4 0 .5 0 0 1 0");

   // RigidShape
   %rigidRock = new RigidShape() {
      datablock = BouncingBoulder;
      scale = "0.2 0.2 0.2";
   };
   %rigidRock.setShapeName("RigidShape");
   %obj.rigidRock = %itemRock;
   %rigidRock.car = %obj;
   %obj.mountObject(%rigidRock, %this.turretSlot, "0 1.4 .6 0 0 1 0");

   // Vehicle
   %vehicleMount = new WheeledVehicle() {
      datablock = CheetahCar;
      scale = "0.1 0.1 0.1";
   };
   %vehicleMount.setShapeName("MountedCheetah");
   %obj.vehicleMount = %vehicleMount;
   %vehicleMount.car = %obj;
   %obj.mountObject(%vehicleMount, %this.turretSlot, "0 0 -.17 0 0 1 1.57");

   // TSStatic
   %tsStaticMount = new TSStatic() {
      shapeName = "art/shapes/rocks/boulder.dts";
      scale = "0.1 0.1 0.1";
   };
   %obj.tsStaticMount = %tsStaticMount;
   %tsStaticMount.car = %obj;
   %obj.mountObject(%tsStaticMount, %this.turretSlot, "0 1.4 0 0 0 1 0");
}

function CustomCheetah::onRemove(%this, %obj)
{
   if( isObject(%obj.tsStaticMount) )
   {
      %obj.unmountObject(%obj.tsStaticMount);
      %obj.tsStaticMount.delete();
   }

   if( isObject(%obj.vehicleMount) )
   {
      %obj.unmountObject(%obj.vehicleMount);
      %obj.vehicleMount.delete();
   }

   if( isObject(%obj.rigidRock) )
   {
      %obj.unmountObject(%obj.rigidRock);
      %obj.rigidRock.delete();
   }

   if( isObject(%obj.itemRock) )
   {
      %obj.unmountObject(%obj.itemRock);
      %obj.itemRock.delete();
   }

   if( isObject(%obj.staticRock) )
   {
      %obj.unmountObject(%obj.staticRock);
      %obj.staticRock.delete();
   }

   CheetahCar::onRemove(%this, %obj);
}

