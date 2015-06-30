
singleton TSShapeConstructor(Tree_oak_large_bDae)
{
   baseShape = "./tree_oak_large_b.dae";
   loadLights = "0";
};

function Tree_oak_large_bDae::onLoad(%this)
{
   %this.addImposter("25", "8", "0", "1", "128", "1", "0");
}
