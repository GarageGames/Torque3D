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

singleton Material(ProxMine_Base)
{
   mapTo = "ProxMine_Base";
   diffuseMap[0] = "ProxMine_D.dds";
   specular[0] = "0.9 0.9 0.9 1";
   specularPower[0] = "10";
   translucentBlendOp = "None";
   normalMap[0] = "ProxMine_N.dds";
   pixelSpecular[0] = "1";
   specularMap[0] = "ProxMine_S.dds";
   useAnisotropic[0] = "1";
};

singleton Material(ProxMine_Glow_Base)
{
   mapTo = "ProxMine_Glow_Base";
   diffuseMap[0] = "ProxMine_Glow.dds";
   specular[0] = "0.9 0.9 0.9 1";
   specularPower[0] = "10";
   translucent = "1";
   glow[0] = "1";
   emissive[0] = "1";
   translucentBlendOp = "Add";
};
