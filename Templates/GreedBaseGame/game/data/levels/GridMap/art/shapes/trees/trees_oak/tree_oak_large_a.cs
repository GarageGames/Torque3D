
singleton TSShapeConstructor(Tree_oak_large_aDae)
{
   baseShape = "./tree_oak_large_a.dae";
   loadLights = "0";
};

function Tree_oak_large_aDae::onLoad(%this)
{
   %this.addImposter("25", "4", "0", "0", "256", "0", "0");
}
