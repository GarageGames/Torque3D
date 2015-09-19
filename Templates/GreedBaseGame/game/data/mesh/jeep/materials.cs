//-----------------------------------------------------------------------------
// Bullet Material Identifiers
//-----------------------------------------------------------------------------
//0 = Metal
//1 = Glass
//2 = Wood
//3 = Concrete/Stone/Rock/Asphault
//4 = Dirt Terrain
//5 = Generic
//6 = None


///-------------------------
/// Willys Body Start
///-------------------------
singleton Material(Willy_access)
{
   diffuseMap[0] = "art/shapes/vehicles/jeep/Willy_access";
   normalMap[0] = "art/shapes/vehicles/jeep/Willy_access_bump";
   pixelSpecular[0] = 1;
   specular[0] = "0.05 0.05 0.05 0.05";
   specularPower[0] = 255;
   mapTo = "Willy_access";
   emissive[0] = "0";
   bulletDecal = 0;
   friction = 1;
};
singleton Material(Willy_body)
{
   diffuseMap[0] = "art/shapes/vehicles/jeep/Willy_body";
   normalMap[0] = "art/shapes/vehicles/jeep/Willy_body_bump";
   pixelSpecular[0] = 0;
   specular[0] = "0.05 0.05 0.05 0.05";
   specularPower[0] = 1;
   mapTo = "Willy_body";
   bulletDecal = 0;
   friction = 1;
};
singleton Material(Willy_grill_glass)
{
   diffuseMap[0] = "art/shapes/vehicles/jeep/Willy_grill_glass";
   normalMap[0] = "art/shapes/vehicles/jeep/Willy_grill_glass_bump";
   pixelSpecular[0] = 1;
   specular[0] = "0.05 0.05 0.05 0.05";
   specularPower[0] = 255;
   mapTo = "Willy_grill_glass";
   emissive[0] = "0";
   bulletDecal = 0;
   friction = 1;
};
singleton Material(Willy_inner)
{
   diffuseMap[0] = "art/shapes/vehicles/jeep/Willy_inner";
   normalMap[0] = "art/shapes/vehicles/jeep/Willy_inner_bump";
   pixelSpecular[0] = 0;
   specular[0] = "0.05 0.05 0.05 0.05";
   specularPower[0] = 1;
   mapTo = "Willy_inner";
   emissive[0] = "0";
   bulletDecal = 0;
   friction = 1;
};
singleton Material(willy_wheels)
{
   diffuseMap[0] = "art/shapes/vehicles/jeep/willy_wheels";
   normalMap[0] = "art/shapes/vehicles/jeep/willy_wheels_bump";
   pixelSpecular[0] = 0;
   specular[0] = "0.05 0.05 0.05 0.05";
   specularPower[0] = 1;
   mapTo = "willy_wheels";
   emissive[0] = "0";
   bulletDecal = 0;
   friction = 1;
};

singleton Material(willy_lights)
{
   mapTo = "jeep_lights";
   diffuseMap[0] = "art/shapes/vehicles/jeep/jeep_lights.png";
   specular[0] = "0.992157 0.992157 0.992157 1";
   specularPower[0] = "1";
   pixelSpecular[0] = "1";
   parallaxScale[0] = "0";
   glow[0] = "0";
   emissive[0] = "0";
   castShadows = "1";
   friction = "1";
   bulletDecal = "0";
   doubleSided = "1";
   showDust = "1";
};

singleton Material(willybrakes1)
{
   mapTo = "willybrakes1";
   diffuseMap[0] = "art/shapes/vehicles/jeep/willybrakes1.png";
   specular[0] = "0.05 0.05 0.05 0.05";
   specularPower[0] = "246";
   pixelSpecular[0] = "1";
   parallaxScale[0] = "0.972222";
   glow[0] = "1";
   emissive[0] = "1";
   castShadows = "0";
};

singleton Material(jeepHands)
{
   mapTo = "Hand_cm";
   diffuseMap[0] = "Hand_cm";
   normalMap[0] = "Hand_nm";
};
/*
singleton Material(loadbox)
{
   diffuseMap[0] = "art/shapes/vehicles/jeep/box.png";
   //normalMap[0] = "art/shapes/vehicles/jeep/Willy_access_bump";
   pixelSpecular[0] = 1;
   specular[0] = "0.05 0.05 0.05 0.05";
   specularPower[0] = 255;
   mapTo = "loadbox";
   emissive[0] = "0";
   bulletDecal = 0;
   friction = 1;
};

*/

singleton Material(jeep_glass)
{
   mapTo = "jeep_glass";
   diffuseMap[0] = "art/shapes/vehicles/jeep/jeep_glass";
   specularPower[0] = "1";
   pixelSpecular[0] = "1";
   doubleSided = "1";
   translucent = "1";
   translucentZWrite = "1";
   alphaRef = "20";
   cubemap = "DesertSkyCubemap";
   showDust = "1";
   bulletDecal = "0";
   friction = "1";
};
