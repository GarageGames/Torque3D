
singleton TSShapeConstructor(Tree_oak_sml_aDae)
{
   baseShape = "./tree_oak_sml_a.dae";
   loadLights = "0";
};

function Tree_oak_sml_aDae::onLoad(%this)
{
   %this.addImposter("1", "4", "0", "0", "128", "1", "0");
}
