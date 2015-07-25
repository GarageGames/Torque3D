
singleton TSShapeConstructor(Tree_oak_forest_bDae)
{
   baseShape = "./tree_oak_forest_b.dae";
};

function Tree_oak_forest_bDae::onLoad(%this)
{
   %this.addImposter("25", "4", "0", "1", "128", "1", "0");
   %this.setDetailLevelSize("600", "400");
}
