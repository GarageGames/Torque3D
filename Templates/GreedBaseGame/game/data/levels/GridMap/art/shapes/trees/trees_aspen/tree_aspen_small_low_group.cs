
singleton TSShapeConstructor(Tree_aspen_small_low_groupDae)
{
   baseShape = "./tree_aspen_small_low_group.dae";
};

function Tree_aspen_small_low_groupDae::onLoad(%this)
{
   %this.addImposter("25", "4", "0", "0", "128", "1", "0");
}
