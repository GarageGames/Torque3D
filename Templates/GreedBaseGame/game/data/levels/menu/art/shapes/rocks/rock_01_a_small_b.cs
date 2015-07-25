
singleton TSShapeConstructor(Rock_01_a_small_bDae)
{
   baseShape = "./rock_01_a_small_b.dae";
};

function Rock_01_a_small_bDae::onLoad(%this)
{
   %this.addImposter("25", "4", "0", "0", "32", "1", "0");
}
