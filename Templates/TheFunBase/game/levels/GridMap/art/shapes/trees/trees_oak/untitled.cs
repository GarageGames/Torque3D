
singleton TSShapeConstructor(UntitledDae)
{
   baseShape = "./untitled.dae";
   loadLights = "0";
};

function UntitledDae::onLoad(%this)
{
   %this.setDetailLevelSize("2", "180");
   %this.addImposter("0", "0", "0", "0", "64", "0", "0");
}
