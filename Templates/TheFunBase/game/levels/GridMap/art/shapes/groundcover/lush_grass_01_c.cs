
singleton TSShapeConstructor(Lush_grass_01_cDae)
{
   baseShape = "./lush_grass_01_c.dae";
};

function Lush_grass_01_cDae::onLoad(%this)
{
   %this.removeImposter();
}
