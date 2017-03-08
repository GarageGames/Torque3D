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

singleton Material(FP_Ryder_Base)
{
   mapTo = "FP_Ryder_Base";
   diffuseMap[0] = "./FP_Ryder_D.dds";
   normalMap[0] = "./FP_Ryder_N.dds";
   specularMap[0] = "./FP_Ryder_S.dds";
   specular[0] = "0.9 0.9 0.9 1";
   specularPower[0] = "10";
   translucentBlendOp = "None";
   pixelSpecular[0] = "1";
   useAnisotropic[0] = "1";
   castDynamicShadows = true;    
};

singleton Material(TP_Ryder_Base)
{
   mapTo = "TP_Ryder_Base";
   diffuseMap[0] = "./TP_Ryder_D.dds";
   normalMap[0] = "./TP_Ryder_N.dds";
   specularMap[0] = "./TP_Ryder_D.dds";
   specular[0] = "1.0 1.0 1.0 1";
   specularPower[0] = "10";
   translucentBlendOp = "None";
   pixelSpecular[0] = "1";
   castDynamicShadows = true;    
};

singleton Material(Ryder_MuzzleFlash_Base)
{
   mapTo = "Ryder_MuzzleFlash_Base";
   diffuseMap[0] = "./Ryder_MuzzleFlash.dds";
   diffuseColor[0] = "0.05 0.05 0.05 1";
   specular[0] = "0 0 0 1";
   specularPower[0] = "10";
   translucent = "1";
   glow[0] = "1";
   emissive[0] = "1";
   doubleSided = "1";
   translucentBlendOp = "Add";
   animFlags[0] = "0x00000005";
   scrollDir[0] = "-0.15 -0.15";
   rotSpeed[0] = "0.25";
   rotPivotOffset[0] = "-0.5 -0.5";
   waveFreq[0] = "5.313";
   waveAmp[0] = "0.016";
   castShadows = "0";
};

