
singleton TSShapeConstructor(Tree_oak_bush_aDae)
{
   baseShape = "./tree_oak_bush_a.dae";
   loadLights = "0";
};

function Tree_oak_bush_aDae::onLoad(%this)
{
   %this.setDetailLevelSize("25", "10");
   %this.addImposter("10", "0", "0", "1", "64", "1", "0");
}
