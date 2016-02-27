//-----------------------------------------------------------------------------
// Copyright (c) 2012 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

singleton Material(Turret_Base)
{
   mapTo = "Turret_Base";
   diffuseMap[0] = "art/shapes/weapons/Turret/Turret_D.dds";
   specular[0] = "0.9 0.9 0.9 1";
   specularPower[0] = "10";
   translucentBlendOp = "None";
   normalMap[0] = "art/shapes/weapons/Turret/Turret_N.dds";
   pixelSpecular[0] = "1";
   specularMap[0] = "art/shapes/weapons/Turret/Turret_D.dds";
   useAnisotropic[0] = "1";
   castDynamicShadows = true;    
   materialTag0 = "Weapon";
};

singleton Material(Turret_Lazer_Base)
{
   mapTo = "Turret_Lazer_Base";
   diffuseMap[0] = "art/shapes/weapons/Turret/Turret_Lazer_D.dds";
   diffuseColor[0] = "1 1 1 1";
   specular[0] = "0.9 0.9 0.9 1";
   specularPower[0] = "10";
   translucent = "1";
   glow[0] = "1";
   emissive[0] = "1";
   doubleSided = "0";
   translucentBlendOp = "Add";
   animFlags[0] = "0x00000000";
   scrollDir[0] = "0 0";
   rotSpeed[0] = "0";
   rotPivotOffset[0] = "0 0";
   waveFreq[0] = "0";
   waveAmp[0] = "0";
   castShadows = "1";
   castDynamicShadows = true;    
   translucentZWrite = "0";
   materialTag0 = "Weapon";
   materialTag1 = "FX";
};

singleton Material(Turret_Lazer_Base)
{
   mapTo = "Turret_Lazer_Base";
   diffuseMap[0] = "./Turret_Lazer_D.dds";
   specular[0] = "0.9 0.9 0.9 1";
   specularPower[0] = "10";
   translucent = "1";
   glow[0] = "1";
   emissive[0] = "1";
   translucentBlendOp = "Add";
   castDynamicShadows = true;    
};

singleton Material(CollisionMat)
{
   mapTo = "CollisionMat";
   diffuseColor[0] = "1 0 0 0.75";
   specular[0] = "0.9 0.9 0.9 1";
   specularPower[0] = "10";
   translucent = "1";
};


singleton Material(Turret_MuzzleFlash_Base)
{
   mapTo = "Turret_MuzzleFlash_Base";
   diffuseColor[0] = "1 1 1 1";
   diffuseMap[0] = "art/shapes/weapons/Turret/Turret_MuzzleFlash.dds";
   specular[0] = "0 0 0 1";
   specularPower[0] = "10";
   glow[0] = "1";
   emissive[0] = "1";
   doubleSided = "1";
   animFlags[0] = "0x00000005";
   scrollDir[0] = "-0.15 -0.15";
   rotSpeed[0] = "0.25";
   rotPivotOffset[0] = "-0.5 -0.5";
   waveFreq[0] = "5.313";
   waveAmp[0] = "0.016";
   castShadows = "0";
   translucent = "1";
   translucentBlendOp = "Add";
   translucentZWrite = "0";
   materialTag1 = "FX";
   materialTag0 = "Weapon";
};
