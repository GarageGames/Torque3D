
singleton TSShapeConstructor(Tree_oak_sml_bDae)
{
   baseShape = "./tree_oak_sml_b.dae";
   loadLights = "0";
};

function Tree_oak_sml_bDae::onLoad(%this)
{
   %this.addImposter("25", "4", "0", "0", "128", "1", "0");
   %this.setDetailLevelSize("600", "700");
   %this.setDetailLevelSize("200", "250");
}
