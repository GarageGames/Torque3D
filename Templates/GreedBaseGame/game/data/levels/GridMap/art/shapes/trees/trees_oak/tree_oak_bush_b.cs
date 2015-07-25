
singleton TSShapeConstructor(Tree_oak_bush_bDae)
{
   baseShape = "./tree_oak_bush_b.dae";
   loadLights = "0";
};

function Tree_oak_bush_bDae::onLoad(%this)
{
   %this.addImposter("25", "0", "0", "0", "64", "1", "0");
}
