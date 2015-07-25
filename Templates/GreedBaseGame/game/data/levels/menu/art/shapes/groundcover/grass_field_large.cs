
singleton TSShapeConstructor(Grass_field_largeDae)
{
   baseShape = "./grass_field_large.dae";
};

function Grass_field_largeDae::onLoad(%this)
{
   %this.setDetailLevelSize("120", "150");
   %this.removeImposter();
   %this.setDetailLevelSize("200", "300");
}
