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

//--- RifleShell.DAE MATERIALS BEGIN ---
singleton Material(RifleShell_RifleShell)
{
	mapTo = "RifleShell";

	diffuseMap[0] = "art/shapes/weapons/RifleShell/RifleShell_diff";
	normalMap[0] = "";
	specularMap[0] = "";

	diffuseColor[0] = "1 1 1 1";
	specular[0] = "0.996078 0.988235 0.921569 0.954";
	specularPower[0] = "19";

	doubleSided = false;
	translucent = false;
	translucentBlendOp = "None";
   pixelSpecular[0] = "1";
   materialTag0 = "Weapon";
};

singleton Material(RifleShell_ColorEffectR88G88B225_material)
{
	mapTo = "ColorEffectR88G88B225-material";

	diffuseMap[0] = "";
	normalMap[0] = "";
	specularMap[0] = "";

	diffuseColor[0] = "0.345098 0.345098 0.882353 1";
	specular[0] = "1 1 1 1";
	specularPower[0] = 10;

	doubleSided = false;
	translucent = false;
	translucentBlendOp = "None";
	materialTag0 = "Weapon";
};

//--- RifleShell.DAE MATERIALS END ---