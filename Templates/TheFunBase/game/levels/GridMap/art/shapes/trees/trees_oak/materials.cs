singleton Material(oak_trunk)
{
   mapTo = "oak_trunk";
   diffuseMap[0] = "vegetation_bark_d.dds";
   detailNormalMap[0] = "vegetation_bark_n.dds";
   normalMap[0] = "vegetation_bark_n.dds";
   detailMap[0] = "vegetation_bark_detail.dds";
   doubleSided = "0";
   translucentBlendOp = "LerpAlpha";
   materialTag1 = "beamng";
   materialTag0 = "beamng";
   materialTag2 = "Natural";
   materialTag3 = "vegetation";
   detailScale[0] = "0.1 0.1";
   detailNormalMapStrength[0] = "1.5";
   diffuseColor[0] = "0.862745 0.862745 0.862745 1";
   specularPower[0] = "1";
   specularStrength[0] = "0.392157";
};

singleton Material(oak_leaves)
{
   mapTo = "oak_leaves";
   diffuseMap[0] = "vegetation_oak_01_d.dds";
   normalMap[0] = "vegetation_oak_01_n.dds";
   specularMap[0] = "vegetation_oak_01_s.dds";
   specular[0] = "0.992157 0.992157 0.992157 1";
   specularPower[0] = "1";
   pixelSpecular[0] = "0";
   useAnisotropic[0] = "1";
   minnaertConstant[0] = "0";
   subSurfaceColor[0] = "0.466667 1 0 1";
   subSurfaceRolloff[0] = "1";
   doubleSided = "1";
   alphaTest = "1";
   alphaRef = "35";
   materialTag1 = "beamng";
   materialTag3 = "vegetation";
   materialTag2 = "Natural";
   materialTag0 = "vegetation";
};