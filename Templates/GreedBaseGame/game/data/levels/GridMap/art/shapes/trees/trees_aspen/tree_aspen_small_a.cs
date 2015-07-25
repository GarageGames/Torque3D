
singleton TSShapeConstructor(Tree_aspen_small_aDae)
{
   baseShape = "./tree_aspen_small_a.dae";
};

function Tree_aspen_small_aDae::onLoad(%this)
{
   %this.addImposter("25", "4", "0", "0", "128", "1", "0");
}
