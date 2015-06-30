singleton Material(Limestone_rock_test_Limestone_rocks)
{
   mapTo = "unmapped_mat";
   diffuseColor[0] = "0.64 0.64 0.64 1";
   specular[0] = "0.5 0.5 0.5 1";
   specularPower[0] = "50";
   doubleSided = "1";
   translucentBlendOp = "None";
   materialTag1 = "Natural";
   materialTag0 = "beamng";
};

singleton Material(Limestone_rocks)
{
   mapTo = "Limestone_rocks";
   diffuseMap[0] = "rocktest.png";
   normalMap[0] = "rocktest_n.png";
   specularMap[0] = "rocktest_s.dds";
   materialTag1 = "Natural";
   materialTag0 = "beamng";
   detailMap[0] = "Rock-03_d.dds";
   specularPower[0] = "1";
   pixelSpecular[0] = "1";
};

singleton Material(rock_01_a)
{
   mapTo = "rocks_01_a";
   diffuseMap[0] = "rocks_01_a_d";
   normalMap[0] = "rocks_01_a_n";
   specularMap[0] = "rocks_01_a_s.dds";
   doubleSided = "1";
   translucentBlendOp = "None";
   materialTag0 = "beamng";
   materialTag1 = "Natural";
};

singleton Material(Cave_01a_terrain_rock)
{
   mapTo = "unmapped_mat";
   diffuseColor[0] = "0.8 0.8 0.8 1";
   doubleSided = "1";
   translucentBlendOp = "None";
   materialTag0 = "RoadAndPath";
};

singleton Material(terrain_rock)
{
   mapTo = "terrain_rock";
   diffuseColor[0] = "0.996078 0.996078 0.996078 1";
   diffuseMap[0] = "levels/GridMap/art/terrain/Rock-05-D.dds";
   normalMap[0] = "levels/GridMap/art/terrain/Rock-05-N.dds";
   doubleSided = "0";
   translucentBlendOp = "None";
   materialTag1 = "Natural";
   materialTag0 = "beamng";
   detailMap[0] = "levels/GridMap/art/shapes/rocks/overlay_rock_05.dds";
   detailScale[0] = "10 10";
   detailNormalMap[0] = "levels/GridMap/art/terrain/Rock-05-N.dds";
   detailNormalMapStrength[0] = "0.5";
   pixelSpecular[0] = "1";
   specularPower[0] = "1";
   specularStrength[0] = "0.0980392";
};

singleton Material(utah_rocks_a)
{
   mapTo = "utah_rocks_a";
   diffuseMap[0] = "utah_rocks_a_d.dds";
   normalMap[0] = "utah_rocks_a_n.dds";
   materialTag1 = "Natural";
   materialTag0 = "beamng";
   specularPower[0] = "1";
   pixelSpecular[0] = "0";
};
