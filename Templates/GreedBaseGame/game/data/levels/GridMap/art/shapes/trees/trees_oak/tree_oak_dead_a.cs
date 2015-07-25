
singleton TSShapeConstructor(Tree_oak_dead_aDae)
{
   baseShape = "./tree_oak_dead_a.dae";
};

function Tree_oak_dead_aDae::onLoad(%this)
{
   %this.addImposter("25", "6", "0", "0", "128", "1", "0");
}
