
singleton TSShapeConstructor(Tree_beech_large_cDae)
{
   baseShape = "./tree_beech_large_c.dae";
};

function Tree_beech_large_cDae::onLoad(%this)
{
   %this.addImposter("25", "4", "0", "0", "256", "1", "0");
}
