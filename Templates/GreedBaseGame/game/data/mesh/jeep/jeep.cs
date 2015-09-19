
singleton TSShapeConstructor(rigid_JeepDts)
{
   baseShape = "./jeep.dts";
};

function rigid_JeepDts::onLoad(%this)
{
   /*%this.addNode("mount5", "jeep70", "-0.0100881 -3.75938 3.98839 1 0 0 0", "1");*/
   %this.renameNode("mount10", "mount27");
   %this.renameNode("mount11", "mount28");
   %this.renameNode("brakelight1", "mount29");
   %this.renameNode("brakelight2", "mount30");
}
