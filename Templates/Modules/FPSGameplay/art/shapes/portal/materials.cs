
singleton Material(portal_base_mat)
{
   mapTo = "portal_base";
   diffuseMap[0] = "data/FPSGameplay/art/shapes/portal/base.png";
   normalMap[0] = "data/FPSGameplay/art/shapes/portal/base-normal.png";
};

singleton Material(portal_top_mat)
{
   mapTo = "portal_top";
   diffuseMap[0] = "data/FPSGameplay/art/shapes/portal/top.png";
   normalMap[0] = "data/FPSGameplay/art/shapes/portal/top-normal.png";
   emissive[0] = "1";
};

singleton Material(portal_lightray_mat)
{
   mapTo = "portal_lightray";
   diffuseMap[0] = "data/FPSGameplay/art/shapes/portal/lightray.png";
   translucent = "1";
   translucentBlendOp = "AddAlpha";
   doubleSided = "1";
   emissive[0] = "1";
};
