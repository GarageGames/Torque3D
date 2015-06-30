
singleton TSShapeConstructor(Grass_field_longDae)
{
   baseShape = "./grass_field_long.dae";
};

function Grass_field_longDae::onLoad(%this)
{
   %this.setDetailLevelSize("150", "100");
   %this.removeImposter();
}
