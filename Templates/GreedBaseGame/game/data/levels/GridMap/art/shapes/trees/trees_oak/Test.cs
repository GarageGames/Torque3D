
singleton TSShapeConstructor(TestDae)
{
   baseShape = "./Test.dae";
};

function TestDae::onLoad(%this)
{
   %this.setDetailLevelSize("2", "0");
   %this.addImposter("20", "0", "0", "0", "64", "0", "0");
   %this.setDetailLevelSize("50", "200");
}
