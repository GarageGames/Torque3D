singleton Material(grass_field)
{
   mapTo = "grass_field";
   diffuseMap[0] = "grass_field.dds";
   normalMap[0] = "normalsfix.dds";
   doubleSided = "1";
   alphaTest = "1";
   alphaRef = "120";
   materialTag1 = "beamng";
   materialTag0 = "beamng";
   materialTag2 = "vegetation";
   materialTag3 = "Natural";
};

singleton Material(lush_vegetation)
{
   mapTo = "lush_vegetation";
   diffuseMap[0] = "lush_vegetation_d.dds";
   materialTag0 = "beamng";
   materialTag1 = "vegetation";
   alphaTest = "1";
   alphaRef = "107";
   doubleSided = "1";
   specularPower[0] = "1";
   materialTag2 = "vegetation";
   useAnisotropic[0] = "0";
   specularMap[0] = "lush_vegetation_s.dds";
   materialTag3 = "Natural";
   subSurface[0] = "0";
   subSurfaceColor[0] = "0.858824 1 0 1";
   subSurfaceRolloff[0] = "0.5";
   normalMap[0] = "lush_vegetation_n.dds";
};

singleton Material(BNGGrass)
{
   mapTo = "unmapped_mat";
   diffuseColor[0] = "0.996078 0.996078 0.996078 1";
   diffuseMap[0] = "grass_field_sml.dds";
   useAnisotropic[0] = "1";
   doubleSided = "1";
   alphaTest = "1";
   alphaRef = "127";
   materialTag0 = "beamng";
   materialTag2 = "vegetation";
   materialTag1 = "vegetation";
};

singleton Material(BNG_Grass_Tall)
{
   mapTo = "unmapped_mat";
   diffuseMap[0] = "grass_field_long.dds";
   materialTag0 = "beamng";
   materialTag1 = "vegetation";
   alphaTest = "1";
   alphaRef = "107";
   doubleSided = "1";
   specularPower[0] = "1";
   materialTag2 = "vegetation";
   useAnisotropic[0] = "0";
};