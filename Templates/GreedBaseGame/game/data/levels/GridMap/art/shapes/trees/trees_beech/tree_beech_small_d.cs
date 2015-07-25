
singleton TSShapeConstructor(Tree_beech_small_dDae)
{
   baseShape = "./tree_beech_small_d.dae";
};

function Tree_beech_small_dDae::onLoad(%this)
{
   %this.addImposter("25", "4", "0", "0", "128", "1", "0");
}
