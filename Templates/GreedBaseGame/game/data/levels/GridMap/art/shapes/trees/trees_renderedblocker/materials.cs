singleton Material(renderedblocker_alpha)
{
   mapTo = "renderedblocker_alpha";
   diffuseMap[0] = "RenderedBlocker_d.dds";
   alphaRef = "194";
   materialTag1 = "vegetation";
   materialTag0 = "beamng";
   useAnisotropic[0] = "1";
   alphaTest = "1";
   doubleSided = "1";
   specularPower[0] = "1";
   materialTag2 = "Natural";
   normalMap[0] = "RenderedBlocker_n.dds";
};

singleton Material(renderedblocker_solid)
{
   mapTo = "renderedblocker_solid";
   diffuseMap[0] = "RenderedBlocker_d.dds";
   alphaRef = "70";
   materialTag1 = "vegetation";
   materialTag0 = "beamng";
   useAnisotropic[0] = "0";
   alphaTest = "0";
   doubleSided = "1";
   specularPower[0] = "1";
   materialTag2 = "Natural";
   normalMap[0] = "RenderedBlocker_n.dds";
};

singleton Material(renderedblocker_long_a_renderedblocker_alpha_001)
{
   mapTo = "renderedblocker_alpha_001";
   diffuseColor[0] = "0.8 0.8 0.8 1";
   specular[0] = "0.5 0.5 0.5 1";
   specularPower[0] = "50";
   doubleSided = "1";
   translucent = "1";
};
