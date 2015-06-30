
singleton TSShapeConstructor(Tree_oak_bush_cDae)
{
   baseShape = "./tree_oak_bush_c.dae";
   loadLights = "0";
};

function Tree_oak_bush_cDae::onLoad(%this)
{
   %this.addImposter("10", "0", "0", "1", "64", "0", "0");
   %this.setDetailLevelSize("100", "200");
}
