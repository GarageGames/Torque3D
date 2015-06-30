
singleton TSShapeConstructor(Tree_aspen_forest_aDae)
{
   baseShape = "./tree_aspen_forest_a.dae";
   loadLights = "0";
};

function Tree_aspen_forest_aDae::onLoad(%this)
{
   %this.addImposter("25", "4", "0", "0", "256", "1", "0");
}
