

singleton Material(GridMat)
{
   mapTo = "GridMat";
   diffuseMap[0] = "levels/GridMap/art/shapes/buildings/concrete-01_GM_d.dds";
   materialTag0 = "beamng";
   specularPower[0] = "1";
   materialTag1 = "beamng";
   materialTag2 = "Industrial";
   useAnisotropic[0] = "1";
   diffuseColor[1] = "1 1 1 1";
   normalMap[0] = "levels/GridMap/art/shapes/buildings/concrete-01_GM_n2.dds";
   specularStrength[0] = "0.0980392";
   specularMap[0] = "levels/GridMap/art/shapes/buildings/Concrete-01_s.dds";
   detailMap[0] = "levels/GridMap/art/shapes/buildings/concrete-01_Macro_d.dds";
   detailScale[0] = "0.1 0.1";
   diffuseColor[0] = "0.772549 0.772549 0.772549 1";
};

singleton Material(GridMat_alpha)
{
   mapTo = "GridMat_alpha";
   diffuseMap[0] = "levels/GridMap/art/shapes/misc/grid_10_b_d.dds";
   materialTag0 = "beamng";
   specularPower[0] = "1";
   materialTag1 = "beamng";
   materialTag2 = "Industrial";
   useAnisotropic[0] = "1";
   castShadows = "1";
   alphaTest = "1";
   alphaRef = "20";
   doubleSided = "0";
};

singleton Material(glass_01)
{
   mapTo = "glass_01";
   doubleSided = "0";
   translucentBlendOp = "LerpAlpha";
   specularPower[0] = "1";
   useAnisotropic[0] = "1";
    materialTag0 = "beamng";
    materialTag1 = "building";
    materialTag2 = "east_coast_usa";
   translucentZWrite = "0";
   diffuseMap[0] = "levels/GridMap/art/shapes/misc/glass_01_d.dds";
   translucent = "1";
   pixelSpecular[0] = "1";
   cubemap = "BNG_Sky_02_cubemap";
   diffuseColor[0] = "0.992157 0.992157 0.992157 1";
};

singleton Material(metal_orange_plate_a)
{
   mapTo = "metal_orange_plate_a";
   diffuseMap[0] = "metal_orange_plate_a_d.dds";
   materialTag0 = "beamng";
   specularPower[0] = "1";
   materialTag1 = "beamng";
   materialTag2 = "Industrial";
   useAnisotropic[0] = "1";
   normalMap[0] = "metal_orange_plate_a_n.dds";
   specularMap[0] = "metal_orange_plate_a_s.dds";
};

singleton Material(concrete_bridge_01a)
{
   mapTo = "concrete_bridge_01a";
   diffuseMap[0] = "concrete_bridge_01a_d.dds";
   normalMap[0] = "concrete_bridge_01a_n.dds";
   specularMap[0] = "concrete_bridge_01a_s.dds";
   specularPower[0] = "4";
   pixelSpecular[0] = "1";
   useAnisotropic[0] = "1";
   materialTag0 = "beamng";
   materialTag1 = "Industrial";
   specular[0] = "0.4 0.4 0.4 1";
   diffuseColor[0] = "0.8 0.8 0.8 1";
};

singleton Material(speedbump_01a)
{
   mapTo = "speedbump_01a";
   diffuseMap[0] = "speedbump_01a_d.dds";
   normalMap[0] = "speedbump_01a_n.dds";
   specularMap[0] = "speedbump_01a_s.dds";
   specularPower[0] = "3";
   pixelSpecular[0] = "1";
   useAnisotropic[0] = "1";
   materialTag0 = "beamng";
   materialTag1 = "Industrial";
};

singleton Material(misc_billboard_images)
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

singleton Material(misc_billboard_news)
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
   emissive[0] = "1";
};

singleton Material(billboards_01_a_misc_billboard_news)
{
   mapTo = "misc_billboard_news";
   diffuseMap[0] = "billboards_01b";
   specular[0] = "0.5 0.5 0.5 1";
   specularPower[0] = "1";
   doubleSided = "1";
   translucentBlendOp = "None";
   pixelSpecular[0] = "1";
   animFlags[0] = "0x00000001";
   scrollDir[0] = "0.207 0";
   scrollSpeed[0] = "0.588";
   materialTag1 = "beamng";
   materialTag3 = "Natural";
   materialTag2 = "Miscellaneous";
   materialTag0 = "Industrial";
};

singleton Material(billboards_01_a_misc_billboard_title)
{
   mapTo = "misc_billboard_title";
   diffuseMap[0] = "levels/GridMap/art/shapes/misc/billboards_01b";
   specular[0] = "0.5 0.5 0.5 1";
   specularPower[0] = "1";
   doubleSided = "1";
   translucentBlendOp = "None";
   pixelSpecular[0] = "1";
   materialTag0 = "Industrial";
   materialTag1 = "beamng";
   useAnisotropic[0] = "1";
   emissive[0] = "1";
   materialTag2 = "Industrial";
};

singleton Material(container_01_a_containers_01_a)
{
   mapTo = "containers_01_a";
   diffuseColor[0] = "0.996078 0.992157 0.992157 1";
   doubleSided = "0";
   translucentBlendOp = "None";
   diffuseMap[0] = "containers_01_a_d.dds";
   normalMap[0] = "containers_01_a_n.dds";
   specularMap[0] = "containers_01_a_s.dds";
};

singleton Material(container_01_b_Material)
{
   mapTo = "Material";
   diffuseColor[0] = "0.64 0.64 0.64 1";
   doubleSided = "1";
   translucentBlendOp = "None";
};

singleton Material(parkinglot_01_b_asphalt_01_a)
{
   mapTo = "unmapped_mat";
   diffuseColor[0] = "0.64 0.64 0.64 1";
   doubleSided = "1";
   translucentBlendOp = "None";
   materialTag1 = "Industrial";
   materialTag0 = "beamng";
};

singleton Material(parkinglot_01_b_misc_markings_01)
{
   mapTo = "unmapped_mat";
   diffuseColor[0] = "0.64 0.64 0.64 1";
   specular[0] = "0.5 0.5 0.5 1";
   specularPower[0] = "50";
   doubleSided = "1";
   translucent = "1";
   materialTag1 = "RoadAndPath";
   materialTag0 = "beamng";
};

singleton Material(parkinglot_01_b_terrain_rockydirt)
{
   mapTo = "unmapped_mat";
   diffuseColor[0] = "0.64 0.64 0.64 1";
   doubleSided = "1";
   translucentBlendOp = "None";
   materialTag1 = "Industrial";
   materialTag0 = "beamng";
};

singleton Material(terrain_rockydirt)
{
   mapTo = "terrain_rockydirt";
   diffuseColor[0] = "0.835294 0.807843 0.8 1";
   diffuseMap[0] = "RockyDirt-01-D.dds";
   normalMap[0] = "RockyDirt-01-N.dds";
   specularMap[0] = "RockyDirt-01-S.dds";
   doubleSided = "1";
   translucentBlendOp = "None";
   materialTag1 = "Natural";
   materialTag0 = "beamng";
};

singleton Material(terrain_grass2)
{
   mapTo = "terrain_grass2";
   diffuseColor[0] = "0.87451 0.854902 0.639216 1";
   diffuseMap[0] = "Grass-02-D.dds";
   normalMap[0] = "Grass-02-N.dds";
   doubleSided = "1";
   translucentBlendOp = "None";
   materialTag1 = "Natural";
   materialTag0 = "beamng";
};

singleton Material(a_asphalt_01_a)
{
   mapTo = "asphalt_01_a";
   diffuseColor[0] = "0.803922 0.803922 0.803922 1";
   diffuseMap[0] = "asphalt_01_a_d.dds";
   specularPower[0] = "1";
   specularMap[0] = "asphalt_01_a_s.dds";
   doubleSided = "1";
   translucentBlendOp = "None";
   materialTag1 = "RoadAndPath";
   materialTag0 = "beamng";
};

singleton Material(misc_markings_01)
{
   mapTo = "misc_markings_01";
   diffuseColor[0] = "0.996078 0.996078 0.996078 1";
   diffuseMap[0] = "misc_markings.dds";
   specular[0] = "0.5 0.5 0.5 1";
   specularPower[0] = "1";
   pixelSpecular[0] = "1";
   doubleSided = "1";
   castShadows = "0";
   translucentZWrite = "1";
   alphaTest = "1";
   alphaRef = "40";
   materialTag1 = "Industrial";
   materialTag0 = "beamng";
};

singleton Material(gm_pipe_02_ColorEffectR27G177B88_material)
{
   mapTo = "ColorEffectR27G177B88-material";
   diffuseColor[0] = "0.0856185 0.555098 0.277363 1";
   specular[0] = "0.5 0.5 0.5 1";
   specularPower[0] = "50";
   doubleSided = "1";
   translucentBlendOp = "None";
};

singleton Material(gm_ramp_02_ColorEffectR27G177B88_material_001)
{
   mapTo = "ColorEffectR27G177B88-material_001";
   diffuseColor[0] = "0.0684948 0.444078 0.221891 1";
   specular[0] = "0.25 0.25 0.25 1";
   specularPower[0] = "50";
   translucentBlendOp = "None";
};

singleton Material(gm_ramp_02_GridMat_001)
{
   mapTo = "GridMat_001";
   diffuseColor[0] = "0.512 0.512 0.512 1";
   specular[0] = "0.5 0.5 0.5 1";
   specularPower[0] = "50";
   translucentBlendOp = "None";
};
