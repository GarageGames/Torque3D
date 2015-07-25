
singleton TSShapeConstructor(Tree_aspen_small_lowDae)
{
   baseShape = "./tree_aspen_small_low.dae";
};

function Tree_aspen_small_lowDae::onLoad(%this)
{
   %this.addImposter("25", "4", "0", "0", "128", "1", "0");
}
