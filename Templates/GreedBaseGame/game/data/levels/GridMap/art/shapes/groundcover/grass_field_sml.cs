
singleton TSShapeConstructor(Grass_field_smlDae)
{
   baseShape = "./grass_field_sml.dae";
   loadLights = "0";
};

function Grass_field_smlDae::onLoad(%this)
{
   %this.setDetailLevelSize("2", "150");
   %this.removeImposter();
}
