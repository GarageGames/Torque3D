
singleton TSShapeConstructor(Test2Dae)
{
   baseShape = "./test2.dae";
};

function Test2Dae::onLoad(%this)
{
   %this.addImposter("0", "16", "0", "0", "256", "0", "0");
   %this.setDetailLevelSize("2", "600");
}
