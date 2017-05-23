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

singleton Material(Mat_Soldier_Main)
{
   mapTo = "base_Soldier_Main";

   diffuseMap[0] = "Soldier_Dif.dds";
   normalMap[0] = "Soldier_N.dds";
   specularMap[0] = "Soldier_Spec.dds";

   diffuseColor[0] = "1 1 1 1";
   specular[0] = "0.9 0.9 0.9 1";
   specularPower[0] = 10;

   doubleSided = false;
   translucent = false;
   showFootprints = "0";
   castDynamicShadows = true;
   materialTag0 = "Player";
};

singleton Material(Mat_Soldier_Dazzle)
{
   mapTo = "base_Soldier_Dazzle";

   diffuseMap[0] = "Soldier_Dazzle.dds";

   diffuseColor[0] = "1 1 1 1";
   specular[0] = "0.9 0.9 0.9 1";
   specularPower[0] = 10;

   doubleSided = false;
   translucent = "1";
   translucentBlendOp = "Add";
   glow[0] = "1";
   emissive[0] = "1";
   castShadows = "0";
   showFootprints = "0";
   castDynamicShadows = true;
   materialTag0 = "Player";
};

singleton Material(soldier_rigged_BoundsMaterial)
{
   mapTo = "BoundsMaterial";
   diffuseColor[0] = "0.0705882 1 0 0.27";
   specular[0] = "0.9 0.9 0.9 1";
   specularPower[0] = "10";
   translucent = "1";
};

singleton Material(soldier_rigged_ShapeBounds)
{
   mapTo = "ShapeBounds";
   diffuseColor[0] = "0.164706 1 0 1";
   specular[0] = "0.9 0.9 0.9 1";
   specularPower[0] = "10";
   translucentBlendOp = "None";
   materialTag0 = "Player";
};

//-----------------------------------------------------------------------------
// Soldier Skins
// Add names to PlayerData.availableSkins list in art/datablock/player.cs

singleton Material(Mat_DarkBlue_Soldier_Main : Mat_Soldier_Main)
{
   mapTo = "DarkBlue_Soldier_Main";
   diffuseMap[0] = "Soldier_DarkBlue_Dif.dds";
};

singleton Material(Mat_DarkGreen_Soldier_Main : Mat_Soldier_Main)
{
   mapTo = "DarkGreen_Soldier_Main";
   diffuseMap[0] = "Soldier_DarkGreen_Dif.dds";
};

singleton Material(Mat_LightGreen_Soldier_Main : Mat_Soldier_Main)
{
   mapTo = "LightGreen_Soldier_Main";
   diffuseMap[0] = "Soldier_LightGreen_Dif.dds";
};

singleton Material(Mat_Orange_Soldier_Main : Mat_Soldier_Main)
{
   mapTo = "Orange_Soldier_Main";
   diffuseMap[0] = "Soldier_Orange_Dif.dds";
};

singleton Material(Mat_Red_Soldier_Main : Mat_Soldier_Main)
{
   mapTo = "Red_Soldier_Main";
   diffuseMap[0] = "Soldier_Red_Dif.dds";
};

singleton Material(Mat_Teal_Soldier_Main : Mat_Soldier_Main)
{
   mapTo = "Teal_Soldier_Main";
   diffuseMap[0] = "Soldier_Teal_Dif.dds";
};

singleton Material(Mat_Violet_Soldier_Main : Mat_Soldier_Main)
{
   mapTo = "Violet_Soldier_Main";
   diffuseMap[0] = "Soldier_Violet_Dif.dds";
};

singleton Material(Mat_Yellow_Soldier_Main : Mat_Soldier_Main)
{
   mapTo = "Yellow_Soldier_Main";
   diffuseMap[0] = "Soldier_Yellow_Dif.dds";
};
