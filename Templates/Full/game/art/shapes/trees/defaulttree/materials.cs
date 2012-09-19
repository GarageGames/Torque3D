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


singleton Material(defaultTree_bark_material)
{
	mapTo = "defaulttree_bark-material";

	diffuseMap[0] = "art/shapes/trees/defaulttree/defaulttree_bark_diffuse.dds";
	normalMap[0] = "art/shapes/trees/defaulttree/defaulttree_bark_normal_specular.dds";
	specularMap[0] = "";

	diffuseColor[0] = "1 1 1 1";
	specular[0] = "0.9 0.9 0.9 1";
	specularPower[0] = 10;

	doubleSided = false;
	translucent = false;
	translucentBlendOp = "None";
   pixelSpecular[0] = "1";
};

singleton Material(defaulttree_material)
{
	mapTo = "defaulttree-material";

	diffuseMap[0] = "art/shapes/trees/defaulttree/defaulttree_diffuse_transparency.dds";
	normalMap[0] = "art/shapes/trees/defaulttree/defaulttree_normal_specular.dds";
	specularMap[0] = "";

	diffuseColor[0] = "1 1 1 1";
	specular[0] = "0.9 0.9 0.9 1";
	specularPower[0] = 10;

	doubleSided = false;
	translucent = false;
	translucentBlendOp = "None";
   pixelSpecular[0] = "1";
   alphaTest = "1";
   alphaRef = "127";
};

singleton Material(defaultTree_fronds_material)
{
   mapTo = "defaulttree_fronds-material";
   diffuseMap[0] = "art/shapes/trees/defaulttree/defaulttree_frond_diffuse_transparency.dds";
   normalMap[0] = "art/shapes/trees/defaulttree/defaulttree_frond_normal_specular.dds";
   specular[0] = "0.9 0.9 0.9 1";
   specularPower[0] = "10";
   pixelSpecular[0] = "1";
   translucentBlendOp = "None";
   alphaTest = "1";
   alphaRef = "114";
   translucent = "1";
};
