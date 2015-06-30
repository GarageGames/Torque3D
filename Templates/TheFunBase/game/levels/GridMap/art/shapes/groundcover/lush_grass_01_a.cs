
singleton TSShapeConstructor(Lush_grass_01_aDae)
{
   baseShape = "./lush_grass_01_a.dae";
};

function Lush_grass_01_aDae::onLoad(%this)
{
   %this.addImposter("0", "6", "0", "0", "128", "0", "0");
}
