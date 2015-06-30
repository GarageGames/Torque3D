
singleton TSShapeConstructor(Lush_ground_02_aDae)
{
   baseShape = "./lush_ground_02_a.dae";
};

function Lush_ground_02_aDae::onLoad(%this)
{
   %this.addImposter("0", "6", "0", "0", "128", "0", "0");
}
