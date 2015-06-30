
singleton TSShapeConstructor(Lush_ground_01_aDae)
{
   baseShape = "./lush_ground_01_a.dae";
};

function Lush_ground_01_aDae::onLoad(%this)
{
   %this.setDetailLevelSize("350", "400");
   %this.addImposter("0", "6", "0", "0", "128", "0", "0");
}
