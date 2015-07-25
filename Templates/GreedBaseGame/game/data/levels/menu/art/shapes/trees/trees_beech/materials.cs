singleton Material(beech_trunk)
{
   mapTo = "beech_bark";
   diffuseColor[0] = "0.94902 0.87451 0.788235 1";
   diffuseMap[0] = "vegetation_beech_bark_d.dds";
   specularPower[0] = "29";
   pixelSpecular[0] = "0";
   useAnisotropic[0] = "0";
   doubleSided = "0";
   alphaTest = "0";
   alphaRef = "1";
   materialTag1 = "vegetation";
   materialTag0 = "beamng";
   specular[0] = "1 1 1 1";
   subSurface[0] = "0";
   subSurfaceColor[0] = "1 0.2 0.2 1";
   subSurfaceRolloff[0] = "0.2";
   normalMap[0] = "vegetation_beech_bark_n.dds";
   specularMap[0] = "vegetation_beech_bark_s.dds";
   materialTag2 = "Natural";
};

singleton Material(beech_leaves)
{
   mapTo = "beech_leaves";
   diffuseMap[0] = "vegetation_beech_d.dds";
   alphaRef = "70";
   materialTag1 = "vegetation";
   materialTag0 = "beamng";
   normalMap[0] = "vegetation_beech_n.dds";
   specularMap[0] = "vegetation_beech_s.dds";
   useAnisotropic[0] = "1";
   alphaTest = "1";
   doubleSided = "1";
   specularPower[0] = "1";
   materialTag2 = "Natural";
};

singleton Material(tree_beech_bush_e_beech_leaves_002)
{
   mapTo = "beech_leaves.002";
   diffuseMap[0] = "vegetation_beech_d";
   normalMap[0] = "vegetation_beech_n";
   doubleSided = "1";
   translucent = "1";
};
