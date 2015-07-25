
singleton TSShapeConstructor(Grass_field_fillerDae)
{
   baseShape = "./grass_field_filler.dae";
};

function Grass_field_fillerDae::onLoad(%this)
{
   %this.setDetailLevelSize("150", "100");
   %this.removeImposter();
}
