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

singleton Material(OctahedronMat)
{
   mapTo = "green";

   diffuseMap[0] = "camera";

   translucent = "1";
   translucentBlendOp = "LerpAlpha";
   emissive = "0";
   castShadows = "0";

   colorMultiply[0] = "0 1 0 1";
};

singleton Material(SimpleConeMat)
{
   mapTo = "blue";

   diffuseMap[0] = "blue";
   translucent = "0";
   emissive = "1";
   castShadows = "0";
};

//--- camera.dts MATERIALS BEGIN ---
singleton Material(CameraMat)
{
   mapTo = "pasted__phongE1";

	diffuseMap[0] = "camera";

	diffuseColor[0] = "0 0.627451 1 1";
	specular[0] = "1 1 1 1";
	specularPower[0] = 211;
	pixelSpecular[0] = 1;
	emissive[0] = 1;

	doubleSided = 1;
	translucent = true;
	translucentBlendOp = "LerpAlpha";
   castShadows = false;
   materialTag0 = "Miscellaneous";
};

//--- camera.dts MATERIALS END ---
//--- noshape.dts MATERIALS BEGIN ---
singleton Material(noshape_NoShape)
{
   mapTo = "NoShape";

	diffuseMap[0] = "";

	diffuseColor[0] = "0.8 0.003067 0 .8";
	emissive[0] = 0;

	doubleSided = false;
	translucent = 1;
	translucentBlendOp = "LerpAlpha";
	castShadows = false;
};

//--- noshape.dts MATERIALS END ---

//--- noshapetext.dae MATERIALS BEGIN ---
singleton Material(noshapetext_lambert1)
{
   mapTo = "lambert1";

	diffuseMap[0] = "";

	diffuseColor[0] = "0.4 0.4 0.4 1";
	specular[0] = "1 1 1 1";
	specularPower[0] = 8;
	pixelSpecular[0] = false;
	emissive[0] = true;

	doubleSided = false;
	translucent = false;
	translucentBlendOp = "None";
};

singleton Material(noshapetext_noshape_mat)
{
   mapTo = "noshape_mat";

	diffuseMap[0] = "";

	diffuseColor[0] = "0.4 0.3504 0.363784 0.33058";
	specular[0] = "1 1 1 1";
	specularPower[0] = 8;
	pixelSpecular[0] = false;
	emissive[0] = true;

	doubleSided = false;
	translucent = true;
	translucentBlendOp = "None";
};

//--- noshapetext.dae MATERIALS END ---

//--- portal MATERIALS BEGIN ---

singleton Material(portal5_portal_top)
{
   mapTo = "portal_top";

	diffuseMap[0] = "top";
   normalMap[0]  = "top-normal";

   diffuseColor[0] = "0.4 0.4 0.4 1";
	specular[0] = "0.5 0.5 0.5 1";
	specularPower[0] = 2;
	pixelSpecular[0] = false;
	emissive[0] = true;

	doubleSided = false;
	translucent = false;
	translucentBlendOp = "None";
};

singleton Material(portal5_portal_lightray)
{
   mapTo = "portal_lightray";

	diffuseMap[0] = "lightray";

	diffuseColor[0] = "0.4 0.4 0.4 0.64462";
	specular[0] = "0.5 0.5 0.5 1";
	specularPower[0] = 2;
	pixelSpecular[0] = false;
	emissive[0] = true;

	doubleSided = 1;
	translucent = true;
	translucentBlendOp = "AddAlpha";
   castShadows = "0";
};
//--- portal MATERIALS END ---

