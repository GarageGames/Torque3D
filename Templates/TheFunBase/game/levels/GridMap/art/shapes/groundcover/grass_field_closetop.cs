
singleton TSShapeConstructor(Grass_field_closetopDae)
{
   baseShape = "./grass_field_closetop.dae";
};

function Grass_field_closetopDae::onLoad(%this)
{
   %this.addImposter("25", "1", "0", "0", "4", "1", "0");
   %this.setDetailLevelSize("150", "80");
}
