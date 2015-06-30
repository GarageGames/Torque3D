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

singleton Material(plants_grasses)
{
   mapTo = "unmapped_mat";
   diffuseMap[0] = "plants_grasses.dds";
   doubleSided = "1";
   alphaTest = "1";
   alphaRef = "140";
   materialTag1 = "beamng";
   materialTag0 = "beamng";
   materialTag2 = "vegetation";
   materialTag3 = "Natural";
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

singleton Material(lush_vegetation_bb)
{
   mapTo = "unmapped_mat";
   diffuseMap[0] = "lush_vegetation_bb_d.dds";
   materialTag0 = "beamng";
   materialTag1 = "vegetation";
   alphaTest = "1";
   alphaRef = "107";
   doubleSided = "1";
   specularPower[0] = "1";
   materialTag2 = "vegetation";
   useAnisotropic[0] = "0";
   materialTag3 = "Natural";
   subSurface[0] = "0";
   subSurfaceColor[0] = "0.858824 1 0 1";
   subSurfaceRolloff[0] = "0.5";
   castShadows = "0";
};

singleton Material(lush_ferns_02_a_lush_vegetation_001)
{
   mapTo = "lush_vegetation.001";
   diffuseMap[0] = "lush_vegetation_d";
   doubleSided = "1";
   translucent = "1";
};

singleton Material(utah_plants_bb)
{
   mapTo = "utah_plants_bb";
   diffuseMap[0] = "Utah_grassesBB_d.dds";
   doubleSided = "1";
   alphaTest = "1";
   alphaRef = "80";
   materialTag1 = "beamng";
   materialTag0 = "beamng";
   materialTag2 = "vegetation";
   materialTag3 = "Natural";
   useAnisotropic[0] = "1";
   specularPower[0] = "21";
   normalMap[0] = "Utah_grassesBB_n.dds";
};
