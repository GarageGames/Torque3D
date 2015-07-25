
singleton TSShapeConstructor(Tree_oak_blocker_aDae)
{
   baseShape = "./tree_oak_blocker_a.dae";
};

function Tree_oak_blocker_aDae::onLoad(%this)
{
   %this.addImposter("25", "4", "0", "0", "128", "1", "0");
}
