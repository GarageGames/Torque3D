
singleton TSShapeConstructor(Rock_01_a_small_cDae)
{
   baseShape = "./rock_01_a_small_c.dae";
};

function Rock_01_a_small_cDae::onLoad(%this)
{
   %this.addImposter("25", "4", "0", "0", "32", "1", "0");
}
