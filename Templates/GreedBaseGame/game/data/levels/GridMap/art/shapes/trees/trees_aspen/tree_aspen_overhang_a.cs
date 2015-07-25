
singleton TSShapeConstructor(Tree_aspen_overhang_aDae)
{
   baseShape = "./tree_aspen_overhang_a.dae";
   loadLights = "0";
};

function Tree_aspen_overhang_aDae::onLoad(%this)
{
   %this.addImposter("25", "4", "0", "0", "128", "1", "0");
}
