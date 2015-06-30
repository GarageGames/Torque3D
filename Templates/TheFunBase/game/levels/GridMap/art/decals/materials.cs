singleton Material(BNG_TESTARROWS)
{
   mapTo = "unmapped_mat";
   diffuseMap[0] = "levels/GridMap/art/decals/gm_crashtestmarkings.dds";
   materialTag0 = "RoadAndPath";
   materialTag1 = "beamng";
   specularPower[0] = "24";
   pixelSpecular[0] = "1";
   useAnisotropic[0] = "1";
   translucent = "1";
   translucentZWrite = "1";
   specular[0] = "0.00392157 0.00392157 0.00392157 1";
   scrollSpeed[0] = "4.118";
   animFlags[0] = "0x00000009";
   scrollDir[0] = "0 -0.1";
   waveFreq[0] = "5.313";
   waveAmp[0] = "0.844";
   sequenceFramePerSec[0] = "30";
   sequenceSegmentSize[0] = "0.32";
};

singleton Material(BNG_Road_Dirt)
{
   mapTo = "unmapped_mat";
   diffuseMap[0] = "road-01_d.dds";
   materialTag0 = "RoadAndPath";
   materialTag1 = "beamng";
   normalMap[0] = "road-01_n.dds";
   specularPower[0] = "24";
   pixelSpecular[0] = "1";
   specularMap[0] = "road-01_s.dds";
   useAnisotropic[0] = "1";
   translucent = "1";
   translucentZWrite = "1";
   specular[0] = "0.00392157 0.00392157 0.00392157 1";
   scrollSpeed[0] = "4.118";
};

singleton Material(Road_01_tracks_large_decal_01)
{
   mapTo = "unmapped_mat";
   diffuseMap[0] = "levels/GridMap/art/decals/road-01_tracks_large_decal_01.dds";
   useAnisotropic[0] = "1";
   castShadows = "0";
   translucent = "1";
   translucentZWrite = "1";
   materialTag0 = "beamng";
   materialTag1 = "decal";
   materialTag2 = "RoadAndPath";
   specularPower[0] = "1";
   specularStrength[0] = "0.196078";
   pixelSpecular[0] = "0";
};

singleton Material(Road_01_decal)
{
   mapTo = "unmapped_mat";
   diffuseMap[0] = "levels/GridMap/art/decals/road-01_decals.dds";
   materialTag0 = "RoadAndPath";
   translucent = "1";
   materialTag1 = "beamng";
   specular[0] = "0.992157 0.992157 0.992157 1";
   specularPower[0] = "1";
   specularMap[0] = "levels/GridMap/art/decals/Road-01_s.dds";
   translucentZWrite = "1";
   castShadows = "0";
   normalMap[0] = "levels/GridMap/art/decals/road-01_n.dds";
};

singleton Material(ind_decal_stuff_01)
{
   mapTo = "unmapped_mat";
   diffuseMap[0] = "ind_decals_01.dds";
   materialTag0 = "decal";
   materialTag1 = "Industrial";
   materialTag2 = "beamng";
   translucent = "1";
   translucentZWrite = "1";
   specularPower[0] = "1";
   useAnisotropic[0] = "1";
};

datablock DecalData(Road_01_d)
{
   Material = "Road_01_decal";
   randomize = "1";
   texCols = "1";
   textureCoords[0] = "0 0 1 1";
   textureCoords[1] = "0 0.5 1 0.5";
   textureCoords[2] = "0 0.5 0.5 0.5";
   textureCoords[3] = "0.5 0.5 0.5 0.5";
   textureCoordCount = "0";
};

datablock DecalData(road_01_tracks_large_decal_01d)
{
   Material = "Road_01_tracks_large_decal_01";
   textureCoordCount = "3";
   randomize = "0";
   texRows = "2";
   texCols = "2";
   textureCoords[0] = "0 0 0.5 0.5";
   textureCoords[1] = "0.5 0 0.5 0.5";
   textureCoords[2] = "0 0.5 0.5 0.5";
   textureCoords[3] = "0.5 0.5 0.5 0.5";
   size = "15";
   frame = "3";
};

datablock DecalData(ind_stuff_01)
{
   Material = "ind_decal_stuff_01";
   textureCoordCount = "3";
   size = "1";
   randomize = "1";
   texRows = "2";
   texCols = "2";
   textureCoords[0] = "0 0 0.5 0.5";
   textureCoords[1] = "0.5 0 0.5 0.5";
   textureCoords[2] = "0 0.5 0.5 0.5";
   textureCoords[3] = "0.5 0.5 0.5 0.5";
   textureCoords[5] = "0 0 1 15";
   fadeStartPixelSize = "51";
   fadeEndPixelSize = "60";
   frame = "1";
};

singleton Material(gm_crashtestmarkings)
{
   mapTo = "unmapped_mat";
   diffuseMap[0] = "gm_crashtestmarkings.dds";
   materialTag0 = "RoadAndPath";
   materialTag1 = "beamng";
   normalMap[0] = "road-01_n.dds";
   specularPower[0] = "24";
   pixelSpecular[0] = "1";
   specularMap[0] = "road-01_s.dds";
   useAnisotropic[0] = "1";
   translucent = "1";
   translucentZWrite = "1";
   specular[0] = "0.00392157 0.00392157 0.00392157 1";
   scrollSpeed[0] = "4.118";
};
