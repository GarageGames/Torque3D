singleton Material(ind_planks_01)
{
   mapTo = "Ind_planks";
   diffuseMap[0] = "ind_planks_01_d.dds";
   normalMap[0] = "ind_planks_01_n.dds";
   specularPower[0] = "1";
   pixelSpecular[0] = "0";
   useAnisotropic[0] = "1";
   alphaTest = "1";
   alphaRef = "121";
   materialTag1 = "beamng";
   materialTag2 = "Industrial";
   specularMap[0] = "ind_planks_01_s.dds";
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

singleton Material(misc_billboard_title)
{
   mapTo = "misc_billboard_images";
   materialTag0 = "beamng";
   specularPower[0] = "1";
    materialTag1 = "beamng";
   useAnisotropic[0] = "1";
   detailScale[0] = "10 10";
   materialTag2 = "Miscellaneous";
   materialTag3 = "Natural";
   diffuseMap[0] = "billboards_01.dds";
   pixelSpecular[0] = "1";
};


singleton Material(billboards_03_a_ColorEffectR27G177B88_material)
{
   mapTo = "ColorEffectR27G177B88-material";
   diffuseColor[0] = "0.0856185 0.555098 0.277363 1";
   specular[0] = "0.5 0.5 0.5 1";
   specularPower[0] = "50";
   doubleSided = "1";
   translucentBlendOp = "None";
};

singleton Material(billboards_03_a_misc_billboard_title)
{
   mapTo = "misc_billboard_title";
   diffuseMap[0] = "billboards_01b";
   specular[0] = "0.5 0.5 0.5 1";
   specularPower[0] = "50";
   doubleSided = "1";
   translucentBlendOp = "None";
};
