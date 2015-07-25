
singleton TSShapeConstructor(Tree_aspen_large_bDae)
{
   baseShape = "./tree_aspen_large_b.dae";
   loadLights = "0";
};

function Tree_aspen_large_bDae::onLoad(%this)
{
   %this.addImposter("25", "4", "0", "0", "256", "1", "0");
}
