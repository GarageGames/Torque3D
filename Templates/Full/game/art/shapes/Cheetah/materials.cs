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

singleton Material(Cheetah_Main)
{
   mapTo = "Cheetah_Main";
   diffuseMap[0] = "art/shapes/Cheetah/Cheetah_D";
   specular[0] = "0.9 0.9 0.9 1";
   specularPower[0] = "10";
   translucentBlendOp = "None";
   normalMap[0] = "art/shapes/Cheetah/Cheetah_N";
   specularMap[0] = "art/shapes/Cheetah/Cheetah_S";
   castDynamicShadows = true;
};

singleton Material(Cheetah_TailLights)
{
   mapTo = "Cheetah_TailLights";
   diffuseColor[0] = "0.588235 0 0 1";
   specular[0] = "0.9 0.9 0.9 1";
   specularPower[0] = "10";
   translucentBlendOp = "None";
   glow[0] = "1";
   emissive[0] = "1";
};

singleton Material(Cheetah_MuzzleFlash)
{
   mapTo = "Cheetah_MuzzleFlash";
   diffuseMap[0] = "art/shapes/Cheetah/Cheetah_MuzzleFlash_D";
   specular[0] = "0.9 0.9 0.9 1";
   specularPower[0] = "10";
   translucentBlendOp = "AddAlpha";
   glow[0] = "1";
   emissive[0] = "1";
   castShadows = "0";
   translucent = "1";
};

singleton Material(Cheetah_Collider)
{
   mapTo = "Collider";
   diffuseColor[0] = "1 0 0 0.75";
   specular[0] = "0.9 0.9 0.9 1";
   specularPower[0] = "10";
   translucent = "1";
};