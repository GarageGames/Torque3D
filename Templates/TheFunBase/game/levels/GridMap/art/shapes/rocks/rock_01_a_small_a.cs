
singleton TSShapeConstructor(Rock_01_a_small_aDae)
{
   baseShape = "./rock_01_a_small_a.dae";
};

function Rock_01_a_small_aDae::onLoad(%this)
{
   %this.addImposter("0", "4", "0", "0", "32", "0", "0");
}
