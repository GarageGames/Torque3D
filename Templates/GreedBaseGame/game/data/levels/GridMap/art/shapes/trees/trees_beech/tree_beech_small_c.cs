
singleton TSShapeConstructor(Tree_beech_small_cDae)
{
   baseShape = "./tree_beech_small_c.dae";
};

function Tree_beech_small_cDae::onLoad(%this)
{
   %this.addImposter("25", "4", "0", "0", "128", "1", "0");
}
