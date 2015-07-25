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

singleton Material(Mat_FP_Soldier_Arms_Main)
{
   mapTo = "base_FP_Soldier_Arms_Main";
   diffuseMap[0] = "art/shapes/actors/Soldier/FP/FP_SoldierArms_D";
   specular[0] = "0.9 0.9 0.9 1";
   specularPower[0] = "10";
   translucentBlendOp = "None";
   normalMap[0] = "art/shapes/actors/Soldier/FP/FP_SoldierArms_N.dds";
   specularMap[0] = "art/shapes/actors/Soldier/FP/FP_SoldierArms_S.dds";
};

//-----------------------------------------------------------------------------
// Soldier Skins
// Add names to PlayerData.availableSkins list in art/datablock/player.cs

singleton Material(Mat_DarkBlue_FP_Soldier_Arms_Main : Mat_FP_Soldier_Arms_Main)
{
   mapTo = "DarkBlue_FP_Soldier_Arms_Main";
   diffuseMap[0] = "Soldier_FPSarms_DarkBlue_Dif.dds";
};

singleton Material(Mat_DarkGreen_FP_Soldier_Arms_Main : Mat_FP_Soldier_Arms_Main)
{
   mapTo = "DarkGreen_FP_Soldier_Arms_Main";
   diffuseMap[0] = "Soldier_FPSarms_DarkGreen_Dif.dds";
};

singleton Material(Mat_LightGreen_FP_Soldier_Arms_Main : Mat_FP_Soldier_Arms_Main)
{
   mapTo = "LightGreen_FP_Soldier_Arms_Main";
   diffuseMap[0] = "Soldier_FPSarms_LightGreen_Dif.dds";
};

singleton Material(Mat_Orange_FP_Soldier_Arms_Main : Mat_FP_Soldier_Arms_Main)
{
   mapTo = "Orange_FP_Soldier_Arms_Main";
   diffuseMap[0] = "Soldier_FPSarms_Orange_Dif.dds";
};

singleton Material(Mat_Red_FP_Soldier_Arms_Main : Mat_FP_Soldier_Arms_Main)
{
   mapTo = "Red_FP_Soldier_Arms_Main";
   diffuseMap[0] = "Soldier_FPSarms_Red_Dif.dds";
};

singleton Material(Mat_Teal_FP_Soldier_Arms_Main : Mat_FP_Soldier_Arms_Main)
{
   mapTo = "Teal_FP_Soldier_Arms_Main";
   diffuseMap[0] = "Soldier_FPSarms_Teal_Dif.dds";
};

singleton Material(Mat_Violet_FP_Soldier_Arms_Main : Mat_FP_Soldier_Arms_Main)
{
   mapTo = "Violet_FP_Soldier_Arms_Main";
   diffuseMap[0] = "Soldier_FPSarms_Violet_Dif.dds";
};

singleton Material(Mat_Yellow_FP_Soldier_Arms_Main : Mat_FP_Soldier_Arms_Main)
{
   mapTo = "Yellow_FP_Soldier_Arms_Main";
   diffuseMap[0] = "Soldier_FPSarms_Yellow_Dif.dds";
};
