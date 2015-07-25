
singleton TSShapeConstructor(Tree_palm_bDae)
{
   baseShape = "./tree_palm_b.dae";
};

function Tree_palm_bDae::onLoad(%this)
{
   %this.addImposter("25", "4", "0", "0", "128", "1", "0");
}
