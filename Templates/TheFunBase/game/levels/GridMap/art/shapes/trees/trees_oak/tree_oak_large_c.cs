
singleton TSShapeConstructor(Tree_oak_large_cDae)
{
   baseShape = "./tree_oak_large_c.dae";
};

function Tree_oak_large_cDae::onLoad(%this)
{
   %this.addImposter("25", "4", "0", "0", "128", "1", "0");
}
