
singleton TSShapeConstructor(Tree_aspen_small_bDae)
{
   baseShape = "./tree_aspen_small_b.dae";
};

function Tree_aspen_small_bDae::onLoad(%this)
{
   %this.addImposter("25", "4", "0", "0", "128", "1", "0");
}
