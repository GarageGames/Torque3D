
singleton TSShapeConstructor(Tree_aspen_forest_bDae)
{
   baseShape = "./tree_aspen_forest_b.dae";
   loadLights = "0";
};

function Tree_aspen_forest_bDae::onLoad(%this)
{
   %this.addImposter("25", "4", "0", "0", "128", "1", "0");
}
