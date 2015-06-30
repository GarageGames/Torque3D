
singleton TSShapeConstructor(Tree_aspen_blocker_aDae)
{
   baseShape = "./tree_aspen_blocker_a.dae";
};

function Tree_aspen_blocker_aDae::onLoad(%this)
{
   %this.addImposter("25", "4", "0", "0", "128", "1", "0");
}
