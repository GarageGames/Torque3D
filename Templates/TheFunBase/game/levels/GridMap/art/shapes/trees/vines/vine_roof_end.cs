
singleton TSShapeConstructor(Vine_roof_endDae)
{
   baseShape = "./vine_roof_end.dae";
};

function Vine_roof_endDae::onLoad(%this)
{
   %this.setDetailLevelSize("450", "400");
}
