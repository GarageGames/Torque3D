
singleton TSShapeConstructor(Tree_oak_forest_aDae)
{
   baseShape = "./tree_oak_forest_a.dae";
   loadLights = "0";
};

function Tree_oak_forest_aDae::onLoad(%this)
{
   %this.addImposter("25", "4", "0", "0", "128", "1", "0");
   %this.setDetailLevelSize("600", "400");
}
