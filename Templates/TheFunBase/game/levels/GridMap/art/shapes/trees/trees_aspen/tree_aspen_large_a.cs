
singleton TSShapeConstructor(Tree_aspen_large_aDae)
{
   baseShape = "./tree_aspen_large_a.dae";
};

function Tree_aspen_large_aDae::onLoad(%this)
{
   %this.addImposter("25", "4", "0", "0", "256", "1", "0");
}
