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

// This is the default save location for any TSForestItemData(s) created in the
// Forest Editor Editor (this script is executed from onServerCreated())

datablock TSForestItemData(ExampleForestMesh)
{
   shapeFile = "levels/GridMap/art/shapes/trees/defaulttree/defaulttree.dae";
   internalName = "ExampleForestMesh";
   windScale = "1";
   trunkBendScale = "0.02";
   branchAmp = "0.05";
   detailAmp = "0.1";
   detailFreq = "0.2";
   mass = "5";
   rigidity = "20";
   dampingCoefficient = "0.2";
   tightnessCoefficient = "4";
};


datablock TSForestItemData(tree_oak_bush_a)
{
   internalName = "tree_oak_bush_a";
   shapeFile = "levels/GridMap/art/shapes/trees/trees_oak/tree_oak_bush_a.dae";
   tightnessCoefficient = "4";
   windScale = "0.3";
   trunkBendScale = "0.05";
   branchAmp = "0";
   detailAmp = "1";
   detailFreq = "0.3";
};

datablock TSForestItemData(tree_oak_bush_c)
{
   internalName = "tree_oak_bush_c";
   shapeFile = "levels/GridMap/art/shapes/trees/trees_oak/tree_oak_bush_c.dae";
   tightnessCoefficient = "4";
   windScale = "0.4";
   trunkBendScale = "0";
   branchAmp = "0";
   detailAmp = "1";
   detailFreq = "0.3";
};


datablock TSForestItemData(tree_oak_large_a)
{
   internalName = "tree_oak_large_a";
   shapeFile = "levels/GridMap/art/shapes/trees/trees_oak/tree_oak_large_a.dae";
   tightnessCoefficient = "4";
   windScale = "0.4";
   trunkBendScale = "0.05";
   branchAmp = "0.1";
   detailAmp = "1";
   detailFreq = "0.6";
   rigidity = "10";
};



datablock TSForestItemData(tree_oak_large_c)
{
   internalName = "tree_oak_large_c";
   shapeFile = "levels/GridMap/art/shapes/trees/trees_oak/tree_oak_large_c.dae";
   mass = "7";
   tightnessCoefficient = "4";
   windScale = "0.4";
   trunkBendScale = "0.05";
   branchAmp = "0.1";
   detailAmp = "0.5";
   detailFreq = "0.6";
};

datablock TSForestItemData(tree_oak_bush_b)
{
   internalName = "tree_oak_bush_b";
   shapeFile = "levels/GridMap/art/shapes/trees/trees_oak/tree_oak_bush_b.dae";
   tightnessCoefficient = "4";
   windScale = "0.3";
   trunkBendScale = "0.05";
   branchAmp = "0";
   detailAmp = "1";
   detailFreq = "0.3";
};


datablock TSForestItemData(tree_oak_sml_a)
{
   internalName = "tree_oak_sml_a";
   shapeFile = "levels/GridMap/art/shapes/trees/trees_oak/tree_oak_sml_a.dae";
   tightnessCoefficient = "4";
   windScale = "0.4";
   trunkBendScale = "0.05";
   branchAmp = "0.1";
   detailAmp = "0.5";
   detailFreq = "0.6";
};


datablock TSForestItemData(tree_oak_forest_a)
{
   internalName = "tree_oak_forest_a";
   shapeFile = "levels/GridMap/art/shapes/trees/trees_oak/tree_oak_forest_a.dae";
   tightnessCoefficient = "4";
   windScale = "0.4";
   trunkBendScale = "0.05";
   branchAmp = "0.1";
   detailAmp = "0.5";
   detailFreq = "0.6";
};

datablock TSForestItemData(tree_oak_large_b)
{
   internalName = "tree_oak_large_b";
   shapeFile = "levels/GridMap/art/shapes/trees/trees_oak/tree_oak_large_b.dae";
   tightnessCoefficient = "4";
   windScale = "0.4";
   trunkBendScale = "0.056";
   branchAmp = "0.1";
   detailAmp = "0.5";
   detailFreq = "0.6";
};


datablock TSForestItemData(Test)
{
   internalName = "Test";
   shapeFile = "levels/GridMap/art/shapes/trees/trees_oak/untitled.dae";
   tightnessCoefficient = "0.1";
   windScale = "0.4";
   trunkBendScale = "0";
   branchAmp = "0";
   detailAmp = "0.5";
   detailFreq = "0.5";
   radius = "1";
};

datablock TSForestItemData(test2)
{
   internalName = "test2";
   shapeFile = "levels/GridMap/art/shapes/trees/trees_oak/test2.dae";
};

datablock TSForestItemData(grass_field_large)
{
   internalName = "grass_field_large";
   shapeFile = "levels/GridMap/art/shapes/groundcover/grass_field_large.dae";
   radius = "0.4";
   rigidity = "5";
   tightnessCoefficient = "4";
   windScale = "0.6";
   trunkBendScale = "0.1";
   detailAmp = "0.4";
   detailFreq = "0.4";
   collidable = "0";
};

datablock TSForestItemData(grass_field_sml)
{
   internalName = "grass_field_sml";
   shapeFile = "levels/GridMap/art/shapes/groundcover/grass_field_sml.dae";
   radius = "0.2";
   tightnessCoefficient = "4";
   windScale = "0.5";
   trunkBendScale = "0.1";
   detailAmp = "0.5";
   detailFreq = "0.5";
   collidable = "0";
};

datablock TSForestItemData(grass_field_long)
{
   internalName = "grass_field_long";
   shapeFile = "levels/GridMap/art/shapes/groundcover/grass_field_long.dae";
   tightnessCoefficient = "4";
   windScale = "0.6";
   trunkBendScale = "0.1";
   detailAmp = "0.5";
   detailFreq = "0.5";
};

datablock TSForestItemData(grass_field_filler)
{
   internalName = "grass_field_filler";
   shapeFile = "levels/GridMap/art/shapes/groundcover/grass_field_filler.dae";
   tightnessCoefficient = "4";
   windScale = "0.6";
   trunkBendScale = "0.1";
   detailAmp = "0.5";
   detailFreq = "0.5";
};

datablock TSForestItemData(tree_aspen_overhang_a)
{
   internalName = "tree_aspen_overhang_a";
   shapeFile = "levels/GridMap/art/shapes/trees/trees_aspen/tree_aspen_overhang_a.dae";
   tightnessCoefficient = "4";
   windScale = "0.6";
   trunkBendScale = "0";
   branchAmp = "0.1";
   detailAmp = "0.1";
   detailFreq = "0.6";
};

datablock TSForestItemData(tree_aspen_large_b)
{
   internalName = "tree_aspen_large_b";
   shapeFile = "levels/GridMap/art/shapes/trees/trees_aspen/tree_aspen_large_b.dae";
   branchAmp = "0.1";
   detailAmp = "0.1";
   detailFreq = "0.6";
   tightnessCoefficient = "4";
   windScale = "0.6";
};

datablock TSForestItemData(tree_aspen_forest_b)
{
   internalName = "tree_aspen_forest_b";
   shapeFile = "levels/GridMap/art/shapes/trees/trees_aspen/tree_aspen_forest_b.dae";
   tightnessCoefficient = "4";
   windScale = "0.4";
   branchAmp = "0.1";
   detailAmp = "0.1";
   detailFreq = "0.6";
   trunkBendScale = "0.03";
};

datablock TSForestItemData(tree_aspen_forest_a)
{
   internalName = "tree_aspen_forest_a";
   shapeFile = "levels/GridMap/art/shapes/trees/trees_aspen/tree_aspen_forest_a.dae";
   tightnessCoefficient = "4";
   windScale = "0.4";
   branchAmp = "0.1";
   detailAmp = "0.1";
   detailFreq = "0.6";
   trunkBendScale = "0.03";
};

datablock TSForestItemData(tree_aspen_small_a)
{
   internalName = "tree_aspen_small_a";
   shapeFile = "levels/GridMap/art/shapes/trees/trees_aspen/tree_aspen_small_a.dae";
   trunkBendScale = "0.03";
   branchAmp = "0.1";
   detailAmp = "0.1";
   detailFreq = "0.6";
   tightnessCoefficient = "4";
   windScale = "0.4";
   radius = "0.2";
};

datablock TSForestItemData(tree_aspen_small_b)
{
   internalName = "tree_aspen_small_b";
   shapeFile = "levels/GridMap/art/shapes/trees/trees_aspen/tree_aspen_small_b.dae";
   tightnessCoefficient = "4";
   windScale = "0.4";
   trunkBendScale = "0.03";
   branchAmp = "0.1";
   detailAmp = "0.1";
   detailFreq = "0.6";
};

datablock TSForestItemData(tree_aspen_small_c)
{
   internalName = "tree_aspen_small_c";
   shapeFile = "levels/GridMap/art/shapes/trees/trees_aspen/tree_aspen_small_c.dae";
   trunkBendScale = "0.03";
   branchAmp = "0.1";
   detailAmp = "0.1";
   detailFreq = "0.6";
   tightnessCoefficient = "4";
   windScale = "0.4";
};

datablock TSForestItemData(tree_aspen_small_d)
{
   internalName = "tree_aspen_small_d";
   shapeFile = "levels/GridMap/art/shapes/trees/trees_aspen/tree_aspen_small_d.dae";
   windScale = "0.4";
   trunkBendScale = "0.03";
   branchAmp = "0.1";
   detailAmp = "0.1";
   detailFreq = "0.4";
};

datablock TSForestItemData(tree_aspen_bush_c)
{
   internalName = "tree_aspen_bush_c";
   shapeFile = "levels/GridMap/art/shapes/trees/trees_aspen/tree_aspen_bush_c.dae";
   tightnessCoefficient = "4";
   windScale = "0.4";
   trunkBendScale = "0.03";
   branchAmp = "0.1";
   detailAmp = "0.1";
   detailFreq = "0.3";
};

datablock TSForestItemData(tree_aspen_bush_a)
{
   internalName = "tree_aspen_bush_a";
   shapeFile = "levels/GridMap/art/shapes/trees/trees_aspen/tree_aspen_bush_a.dae";
   tightnessCoefficient = "4";
   windScale = "0.4";
   trunkBendScale = "0.03";
   branchAmp = "0.1";
   detailAmp = "0.1";
   detailFreq = "0.3";
   radius = "0.5";
};

datablock TSForestItemData(tree_aspen_bush_b)
{
   internalName = "tree_aspen_bush_b";
   shapeFile = "levels/GridMap/art/shapes/trees/trees_aspen/tree_aspen_bush_b.dae";
   tightnessCoefficient = "4";
   windScale = "0.4";
   trunkBendScale = "0.03";
   branchAmp = "0.1";
   detailAmp = "0.1";
   detailFreq = "0.3";
   radius = "0.5";
};

datablock TSForestItemData(tree_aspen_large_a)
{
   internalName = "tree_aspen_large_a";
   shapeFile = "levels/GridMap/art/shapes/trees/trees_aspen/tree_aspen_large_a.dae";
   tightnessCoefficient = "4";
   windScale = "0.4";
   trunkBendScale = "0.02";
   branchAmp = "0.1";
   detailAmp = "0.1";
   detailFreq = "0.6";
   mass = "50";
   radius = "0.5";
};



datablock TSForestItemData(vine_ground_medium)
{
   internalName = "vine_ground_medium";
   shapeFile = "levels/GridMap/art/shapes/trees/vines/vine_ground_medium.dae";
   tightnessCoefficient = "4";
   windScale = "0.4";
   detailAmp = "0.1";
   detailFreq = "0.5";
   radius = "0.5";
};

datablock TSForestItemData(vine_bush_wide)
{
   internalName = "vine_bush_wide";
   shapeFile = "levels/GridMap/art/shapes/trees/vines/vine_bush_wide.dae";
   tightnessCoefficient = "4";
   windScale = "0.4";
   trunkBendScale = "0.01";
   branchAmp = "0.1";
   detailAmp = "0.1";
   detailFreq = "0.6";
   radius = "0.8";
};

datablock TSForestItemData(vine_bush_large)
{
   internalName = "vine_bush_large";
   shapeFile = "levels/GridMap/art/shapes/trees/vines/vine_bush_large.dae";
   tightnessCoefficient = "4";
   windScale = "0.4";
   trunkBendScale = "0.1";
   branchAmp = "0.1";
   detailAmp = "0.1";
   detailFreq = "0.6";
};

datablock TSForestItemData(vine_ground_large)
{
   internalName = "vine_ground_large";
   shapeFile = "levels/GridMap/art/shapes/trees/vine/vine_ground_large.dae";
   tightnessCoefficient = "4";
   windScale = "0.4";
   trunkBendScale = "0.1";
   branchAmp = "0.1";
   detailAmp = "0.1";
   detailFreq = "0.6";
};

datablock TSForestItemData(vine_ground_large)
{
   internalName = "vine_ground_large";
   shapeFile = "levels/GridMap/art/shapes/trees/vines/vine_ground_large.dae";
   tightnessCoefficient = "4";
   windScale = "0.4";
   branchAmp = "0.1";
   detailAmp = "0.1";
   detailFreq = "0.6";
   radius = "0.5";
   trunkBendScale = "0.1";
};

datablock TSForestItemData(vine_wall_wide_b)
{
   internalName = "vine_wall_wide_b";
   shapeFile = "levels/GridMap/art/shapes/trees/vines/vine_wall_wide_b.dae";
   tightnessCoefficient = "4";
   windScale = "0.3";
   branchAmp = "0.1";
   detailAmp = "0.1";
   detailFreq = "0.6";
};

datablock TSForestItemData(vine_wall_small)
{
   internalName = "vine_wall_small";
   shapeFile = "levels/GridMap/art/shapes/trees/vines/vine_wall_small.dae";
   tightnessCoefficient = "4";
   windScale = "0.3";
   branchAmp = "0.1";
   detailAmp = "0.1";
   detailFreq = "0.6";
   radius = "0.4";
};

datablock TSForestItemData(vine_wall_medium)
{
   internalName = "vine_wall_medium";
   shapeFile = "levels/GridMap/art/shapes/trees/vines/vine_wall_medium.dae";
   tightnessCoefficient = "4";
   windScale = "0.3";
   branchAmp = "0.1";
   detailAmp = "0.1";
   detailFreq = "0.6";
};

datablock TSForestItemData(vine_wall_large)
{
   internalName = "vine_wall_large";
   shapeFile = "levels/GridMap/art/shapes/trees/vines/vine_wall_large.dae";
   tightnessCoefficient = "4";
   windScale = "0.3";
   branchAmp = "0.1";
   detailAmp = "0.1";
   detailFreq = "0.6";
};

datablock TSForestItemData(vine_ground_small)
{
   internalName = "vine_ground_small";
   shapeFile = "levels/GridMap/art/shapes/trees/vines/vine_ground_small.dae";
   tightnessCoefficient = "4";
   windScale = "0.3";
   branchAmp = "0.1";
   detailAmp = "0.1";
   detailFreq = "0.6";
   radius = "0.5";
};

datablock TSForestItemData(vine_bush_small)
{
   internalName = "vine_bush_small";
   shapeFile = "levels/GridMap/art/shapes/trees/vines/vine_bush_small.dae";
   radius = "0.5";
   windScale = "0.4";
   branchAmp = "0.1";
   detailAmp = "0.1";
   detailFreq = "0.6";
};

datablock TSForestItemData(vine_bush_wide_b)
{
   internalName = "vine_bush_wide_b";
   shapeFile = "levels/GridMap/art/shapes/trees/vines/vine_bush_wide_b.dae";
   tightnessCoefficient = "4";
   windScale = "0.4";
   trunkBendScale = "0.04";
   branchAmp = "0.1";
   detailAmp = "0.1";
   detailFreq = "0.6";
};

datablock TSForestItemData(vine_roof_wide)
{
   internalName = "vine_roof_wide";
   shapeFile = "levels/GridMap/art/shapes/trees/vines/vine_roof_wide.dae";
   tightnessCoefficient = "4";
   windScale = "0.3";
   trunkBendScale = "0";
   branchAmp = "0.1";
   detailAmp = "0.1";
   detailFreq = "0.6";
};

datablock TSForestItemData(vine_roof_small)
{
   internalName = "vine_roof_small";
   shapeFile = "levels/GridMap/art/shapes/trees/vines/vine_roof_small.dae";
   windScale = "0";
   trunkBendScale = "0";
   branchAmp = "0";
   detailAmp = "0";
   detailFreq = "0.6";
   tightnessCoefficient = "4";
};

datablock TSForestItemData(vine_roof_end)
{
   internalName = "vine_roof_end";
   shapeFile = "levels/GridMap/art/shapes/trees/vines/vine_roof_end.dae";
   windScale = "0.4";
   trunkBendScale = "0.02";
   branchAmp = "0.1";
   detailAmp = "0.1";
   detailFreq = "0.6";
};

datablock TSForestItemData(vine_side_2m)
{
   internalName = "vine_side_2m";
   shapeFile = "levels/GridMap/art/shapes/trees/vines/vine_side_2m.dae";
   tightnessCoefficient = "4";
   windScale = "0.4";
   branchAmp = "0.1";
   detailAmp = "0.1";
   detailFreq = "0.6";
};

datablock TSForestItemData(vine_side_4m)
{
   internalName = "vine_side_4m";
   shapeFile = "levels/GridMap/art/shapes/trees/vines/vine_side_4m.dae";
   tightnessCoefficient = "4";
   windScale = "0.4";
   branchAmp = "0.1";
   detailAmp = "0.1";
   detailFreq = "0.6";
};

datablock TSForestItemData(vine_side_8m)
{
   internalName = "vine_side_8m";
   shapeFile = "levels/GridMap/art/shapes/trees/vines/vine_side_8m.dae";
   tightnessCoefficient = "4";
   windScale = "0.4";
   branchAmp = "0.1";
   detailAmp = "0.1";
   detailFreq = "0.6";
};

datablock TSForestItemData(vine_side_1m)
{
   internalName = "vine_side_1m";
   shapeFile = "levels/GridMap/art/shapes/trees/vines/vine_side_1m.dae";
   tightnessCoefficient = "4";
   windScale = "0.4";
   branchAmp = "0.1";
   detailAmp = "0.1";
   detailFreq = "0.6";
};

datablock TSForestItemData(tree_oak_sml_b)
{
   internalName = "tree_oak_sml_b";
   shapeFile = "levels/GridMap/art/shapes/trees/trees_oak/tree_oak_sml_b.dae";
   tightnessCoefficient = "4";
   windScale = "0.4";
   trunkBendScale = "0.1";
   branchAmp = "0.1";
   detailAmp = "0.1";
   detailFreq = "0.6";
};

datablock TSForestItemData(vine_wall_wide_a)
{
   internalName = "vine_wall_wide_a";
   shapeFile = "levels/GridMap/art/shapes/trees/vines/vine_wall_wide_a.dae";
   radius = "0.5";
   tightnessCoefficient = "4";
   windScale = "0.4";
   branchAmp = "0.1";
   detailAmp = "0.1";
   detailFreq = "0.6";
};

datablock TSForestItemData(lush_ferns_01)
{
   internalName = "lush_ferns_01";
   shapeFile = "levels/GridMap/art/shapes/groundcover/lush_ferns_01_a.dae";
   tightnessCoefficient = "4";
   windScale = "0.4";
   trunkBendScale = "0.1";
   branchAmp = "0.1";
   detailAmp = "0.04";
   detailFreq = "1";
   radius = "0.3";
};

datablock TSForestItemData(lush_ferns_02_a)
{
   internalName = "lush_ferns_02_a";
   shapeFile = "levels/GridMap/art/shapes/groundcover/lush_ferns_02_b.dae";
   radius = "0.2";
   tightnessCoefficient = "4";
   windScale = "0.4";
   trunkBendScale = "0.1";
   branchAmp = "0.1";
   detailAmp = "0.4";
   detailFreq = "0.5";
};

datablock TSForestItemData(lush_big_a)
{
   internalName = "lush_big_a";
   shapeFile = "levels/GridMap/art/shapes/groundcover/lush_big_a.dae";
   tightnessCoefficient = "4";
   windScale = "0.4";
   trunkBendScale = "0.1";
   branchAmp = "0.1";
   detailAmp = "0.1";
   detailFreq = "0.5";
   radius = "0.2";
};

datablock TSForestItemData(lush_big_b)
{
   internalName = "lush_big_b";
   shapeFile = "levels/GridMap/art/shapes/groundcover/lush_big_b.dae";
   tightnessCoefficient = "4";
   windScale = "0.4";
   trunkBendScale = "0.1";
   branchAmp = "0.1";
   detailFreq = "0.5";
   radius = "0.2";
};

datablock TSForestItemData(lush_general_01_b)
{
   internalName = "lush_general_01_b";
   shapeFile = "levels/GridMap/art/shapes/groundcover/lush_general_01_b.dae";
   radius = "0.2";
   tightnessCoefficient = "4";
   windScale = "0.4";
   branchAmp = "0.05";
   detailAmp = "0.1";
   detailFreq = "0.2";
};

datablock TSForestItemData(lush_general_01_c)
{
   internalName = "lush_general_01_c";
   shapeFile = "levels/GridMap/art/shapes/groundcover/lush_general_01_c.dae";
   tightnessCoefficient = "4";
   windScale = "0.4";
   branchAmp = "0.05";
   detailAmp = "0.2";
   detailFreq = "0.2";
   radius = "0.2";
};

datablock TSForestItemData(lush_general_01_d)
{
   internalName = "lush_general_01_d";
   shapeFile = "levels/GridMap/art/shapes/groundcover/lush_general_01_d.dae";
   radius = "0.2";
   tightnessCoefficient = "4";
   windScale = "0.4";
   branchAmp = "0.05";
   detailAmp = "0.2";
   detailFreq = "0.2";
};

datablock TSForestItemData(lush_general_01_e)
{
   internalName = "lush_general_01_e";
   shapeFile = "levels/GridMap/art/shapes/groundcover/lush_general_01_e.dae";
   tightnessCoefficient = "4";
   windScale = "0.4";
   detailAmp = "0.5";
   detailFreq = "0.5";
   radius = "0.2";
};

datablock TSForestItemData(lush_general_01_a)
{
   internalName = "lush_general_01_a";
   shapeFile = "levels/GridMap/art/shapes/groundcover/lush_general_01_a.dae";
   windScale = "0.4";
   branchAmp = "0.02";
   detailAmp = "0.1";
   detailFreq = "0.2";
   radius = "0.2";
   tightnessCoefficient = "4";
};

datablock TSForestItemData(lush_ferns_01_a)
{
   internalName = "lush_ferns_01_a";
   shapeFile = "levels/GridMap/art/shapes/groundcover/lush_ferns_01_a.dae";
   radius = "0.4";
   tightnessCoefficient = "4";
   windScale = "0.4";
   trunkBendScale = "0.1";
   branchAmp = "0.05";
   detailAmp = "0.3";
   detailFreq = "0.3";
};

datablock TSForestItemData(lush_ferns_01_b)
{
   internalName = "lush_ferns_01_b";
   shapeFile = "levels/GridMap/art/shapes/groundcover/lush_ferns_01_b.dae";
   radius = "0.2";
   tightnessCoefficient = "4";
   windScale = "0.4";
   trunkBendScale = "0.1";
   branchAmp = "0.05";
   detailAmp = "0.3";
   detailFreq = "0.3";
};

datablock TSForestItemData(lush_ferns_02_b)
{
   internalName = "lush_ferns_02_b";
   shapeFile = "levels/GridMap/art/shapes/groundcover/lush_ferns_02_b.dae";
   radius = "0.4";
   windScale = "0.4";
   trunkBendScale = "0.1";
   branchAmp = "0.05";
   detailAmp = "0.3";
   detailFreq = "0.3";
};

datablock TSForestItemData(lush_grass_01_a)
{
   internalName = "lush_grass_01_a";
   shapeFile = "levels/GridMap/art/shapes/groundcover/lush_grass_01_a.dae";
   radius = "0.2";
   tightnessCoefficient = "4";
   windScale = "0.4";
   trunkBendScale = "0.1";
   branchAmp = "0.05";
   detailAmp = "0.1";
   detailFreq = "0.6";
};

datablock TSForestItemData(lush_grass_01_c)
{
   internalName = "lush_grass_01_c";
   shapeFile = "levels/GridMap/art/shapes/groundcover/lush_grass_01_c.dae";
   radius = "0.2";
   tightnessCoefficient = "4";
   windScale = "0.4";
   trunkBendScale = "0.1";
   branchAmp = "0.05";
   detailAmp = "0.1";
   detailFreq = "0.6";
};

datablock TSForestItemData(lush_grass_01_b)
{
   internalName = "lush_grass_01_b";
   shapeFile = "levels/GridMap/art/shapes/groundcover/lush_grass_01_b.dae";
   radius = "0.15";
   tightnessCoefficient = "4";
   windScale = "0.4";
   trunkBendScale = "0.1";
   branchAmp = "0.05";
   detailAmp = "0.1";
   detailFreq = "0.6";
};

datablock TSForestItemData(LJFTest)
{
   internalName = "LJFTest";
   shapeFile = "levels/GridMap/art/shapes/rocks/Limestone_rock_test.cached.dts";
};

datablock TSForestItemData(tree_beech_large_c)
{
   internalName = "tree_beech_large_c";
   shapeFile = "levels/GridMap/art/shapes/trees/trees_beech/tree_beech_large_c.dae";
   radius = "0.5";
   dampingCoefficient = "4";
   windScale = "0.4";
   trunkBendScale = "0.05";
   branchAmp = "0.5";
   detailAmp = "0.4";
   detailFreq = "0.5";
};

datablock TSForestItemData(tree_beech_overhang_a)
{
   internalName = "tree_beech_overhang_a";
   shapeFile = "levels/GridMap/art/shapes/trees/trees_beech/tree_beech_overhang_a.dae";
   radius = "0.5";
   dampingCoefficient = "4";
   windScale = "0.4";
   trunkBendScale = "0.05";
   branchAmp = "0.5";
   detailAmp = "0.4";
   detailFreq = "0.5";
};

datablock TSForestItemData(tree_beech_small_d)
{
   internalName = "tree_beech_small_d";
   shapeFile = "levels/GridMap/art/shapes/trees/trees_beech/tree_beech_small_d.dae";
   dampingCoefficient = "4";
   windScale = "0.4";
   trunkBendScale = "0.05";
   branchAmp = "0.5";
   detailAmp = "0.4";
   detailFreq = "0.5";
};

datablock TSForestItemData(tree_beech_small_c)
{
   internalName = "tree_beech_small_c";
   shapeFile = "levels/GridMap/art/shapes/trees/trees_beech/tree_beech_small_c.dae";
   dampingCoefficient = "4";
   windScale = "0.4";
   trunkBendScale = "0.05";
   branchAmp = "0.5";
   detailAmp = "0.4";
   detailFreq = "0.5";
};

datablock TSForestItemData(tree_beech_small_b)
{
   internalName = "tree_beech_small_b";
   shapeFile = "levels/GridMap/art/shapes/trees/trees_beech/tree_beech_small_b.dae";
   dampingCoefficient = "4";
   windScale = "0.4";
   trunkBendScale = "0.05";
   branchAmp = "0.5";
   detailAmp = "0.4";
   detailFreq = "0.5";
};

datablock TSForestItemData(tree_beech_forest_a)
{
   internalName = "tree_beech_forest_a";
   shapeFile = "levels/GridMap/art/shapes/trees/trees_beech/tree_beech_forest_a.dae";
   dampingCoefficient = "4";
   windScale = "0.4";
   trunkBendScale = "0.05";
   branchAmp = "0.5";
   detailAmp = "0.4";
   detailFreq = "0.5";
};

datablock TSForestItemData(tree_beech_forest_b)
{
   internalName = "tree_beech_forest_b";
   shapeFile = "levels/GridMap/art/shapes/trees/trees_beech/tree_beech_forest_b.dae";
   dampingCoefficient = "4";
   windScale = "0.4";
   trunkBendScale = "0.05";
   branchAmp = "0.5";
   detailAmp = "0.4";
   detailFreq = "0.5";
};

datablock TSForestItemData(tree_beech_large_b)
{
   internalName = "tree_beech_large_b";
   shapeFile = "levels/GridMap/art/shapes/trees/trees_beech/tree_beech_large_b.dae";
   dampingCoefficient = "4";
   windScale = "0.4";
   trunkBendScale = "0.05";
   branchAmp = "0.5";
   detailAmp = "0.4";
   detailFreq = "0.5";
};

datablock TSForestItemData(tree_beech_bush_c)
{
   internalName = "tree_beech_bush_c";
   shapeFile = "levels/GridMap/art/shapes/trees/trees_beech/tree_beech_bush_c.dae";
   dampingCoefficient = "4";
   windScale = "0.4";
   trunkBendScale = "0.05";
   branchAmp = "1.5";
   detailAmp = "0.3";
   detailFreq = "0.4";
   radius = "0.5";
};

datablock TSForestItemData(tree_beech_bush_a)
{
   internalName = "tree_beech_bush_a";
   shapeFile = "levels/GridMap/art/shapes/trees/trees_beech/tree_beech_bush_a.dae";
   radius = "0.5";
   dampingCoefficient = "4";
   windScale = "0.4";
   trunkBendScale = "0.05";
   branchAmp = "1.5";
   detailAmp = "0.3";
   detailFreq = "0.3";
};

datablock TSForestItemData(tree_beech_bush_b)
{
   internalName = "tree_beech_bush_b";
   shapeFile = "levels/GridMap/art/shapes/trees/trees_beech/tree_beech_bush_b.dae";
   radius = "0.5";
   dampingCoefficient = "4";
   windScale = "0.4";
   trunkBendScale = "0.05";
   branchAmp = "0.5";
   detailAmp = "0.3";
   detailFreq = "0.3";
};

datablock TSForestItemData(tree_oak_forest_b)
{
   internalName = "tree_oak_forest_b";
   shapeFile = "levels/GridMap/art/shapes/trees/trees_oak/tree_oak_forest_b.dae";
   windScale = "0.4";
   trunkBendScale = "0.01";
   branchAmp = "0.5";
   detailAmp = "0.5";
   detailFreq = "0.5";
};

datablock TSForestItemData(tree_oak_dead_a)
{
   internalName = "tree_oak_dead_a";
   shapeFile = "levels/GridMap/art/shapes/trees/trees_oak/tree_oak_dead_a.dae";
   radius = "2";
   windScale = "0.4";
};

datablock TSForestItemData(tree_oak_dead_b)
{
   internalName = "tree_oak_dead_b";
   shapeFile = "levels/GridMap/art/shapes/trees/trees_oak/tree_oak_dead_b.dae";
   windScale = "0.4";
};

datablock TSForestItemData(tree_oak_blocker_a)
{
   internalName = "tree_oak_blocker_a";
   shapeFile = "levels/GridMap/art/shapes/trees/trees_oak/tree_oak_blocker_a.dae";
   radius = "0.6";
   dampingCoefficient = "4";
   windScale = "0.4";
   trunkBendScale = "0.01";
   branchAmp = "0.08";
   detailAmp = "0.5";
   detailFreq = "0.5";
};

datablock TSForestItemData(tree_aspen_blocker_a)
{
   internalName = "tree_aspen_blocker_a";
   shapeFile = "levels/GridMap/art/shapes/trees/trees_aspen/tree_aspen_blocker_a.dae";
   windScale = "0.4";
   trunkBendScale = "0.03";
   branchAmp = "0.1";
   detailAmp = "0.1";
   detailFreq = "0.4";
};

datablock TSForestItemData(tree_beech_blocker_a)
{
   internalName = "tree_beech_blocker_a";
   shapeFile = "levels/GridMap/art/shapes/trees/trees_beech/tree_beech_blocker_a.dae";
   radius = "0.8";
   dampingCoefficient = "4";
   windScale = "0.4";
   trunkBendScale = "0.05";
   branchAmp = "0.5";
   detailAmp = "0.4";
   detailFreq = "0.5";
};

datablock TSForestItemData(tree_beech_dead_a)
{
   internalName = "tree_beech_dead_a";
   shapeFile = "levels/GridMap/art/shapes/trees/trees_beech/tree_beech_dead_a.dae";
   radius = "0.5";
};

datablock TSForestItemData(tree_beech_dead_b)
{
   internalName = "tree_beech_dead_b";
   shapeFile = "levels/GridMap/art/shapes/trees/trees_beech/tree_beech_dead_b.dae";
   radius = "0.5";
};

datablock TSForestItemData(tree_oak_forest_canopy_a)
{
   internalName = "tree_oak_forest_canopy_a";
   shapeFile = "levels/GridMap/art/shapes/trees/trees_oak/tree_oak_forest_canopy_a.dae";
   windScale = "0.4";
   trunkBendScale = "0.01";
   branchAmp = "0.5";
   detailAmp = "0.5";
   detailFreq = "0.5";
};

datablock TSForestItemData(tree_aspen_small_low)
{
   internalName = "tree_aspen_small_low";
   shapeFile = "levels/GridMap/art/shapes/trees/trees_aspen/tree_aspen_small_low.dae";
   windScale = "0.4";
   trunkBendScale = "0.03";
   branchAmp = "0.1";
   detailAmp = "0.1";
   detailFreq = "0.6";
   radius = "0.3";
};

datablock TSForestItemData(tree_aspen_small_low_group)
{
   internalName = "tree_aspen_small_low_group";
   shapeFile = "levels/GridMap/art/shapes/trees/trees_aspen/tree_aspen_small_low_group.dae";
   radius = "0.5";
   windScale = "0.4";
   trunkBendScale = "0.03";
   branchAmp = "0.1";
   detailAmp = "0.1";
   detailFreq = "0.6";
};

datablock TSForestItemData(tree_beech_bush_d)
{
   internalName = "tree_beech_bush_d";
   shapeFile = "levels/GridMap/art/shapes/trees/trees_beech/tree_beech_bush_d.dae";
   radius = "0.7";
   windScale = "0.4";
   trunkBendScale = "0.03";
   branchAmp = "0.1";
   detailAmp = "0.4";
   detailFreq = "0.4";
};

datablock TSForestItemData(tree_beech_bush_e)
{
   internalName = "tree_beech_bush_e";
   shapeFile = "levels/GridMap/art/shapes/trees/trees_beech/tree_beech_bush_e.dae";
   windScale = "0.4";
   trunkBendScale = "0.03";
   branchAmp = "0.2";
   detailAmp = "0.4";
   detailFreq = "0.4";
};

datablock TSForestItemData(rock_01_a_large_b)
{
   internalName = "rock_01_a_large_b";
   shapeFile = "levels/GridMap/art/shapes/rocks/rock_01_a_large_b.dae";
   radius = "0.3";
};

datablock TSForestItemData(rock_01_a_large_c)
{
   internalName = "rock_01_a_large_c";
   shapeFile = "levels/GridMap/art/shapes/rocks/rock_01_a_large_c.dae";
   radius = "0.3";
};

datablock TSForestItemData(rock_01_a_small_a)
{
   internalName = "rock_01_a_small_a";
   shapeFile = "levels/GridMap/art/shapes/rocks/rock_01_a_small_a.dae";
   radius = "0.15";
};

datablock TSForestItemData(rock_01_a_small_b)
{
   internalName = "rock_01_a_small_b";
   shapeFile = "levels/GridMap/art/shapes/rocks/rock_01_a_small_b.dae";
   radius = "0.15";
};

datablock TSForestItemData(rock_01_a_small_c)
{
   internalName = "rock_01_a_small_c";
   shapeFile = "levels/GridMap/art/shapes/rocks/rock_01_a_small_c.dae";
   radius = "0.15";
};

datablock TSForestItemData(rock_01_a_large_a)
{
   internalName = "rock_01_a_large_a";
   shapeFile = "levels/GridMap/art/shapes/rocks/rock_01_a_large_a.dae";
   radius = "0.3";
};

datablock TSForestItemData(tree_palm_a)
{
   internalName = "tree_palm_a";
   shapeFile = "levels/GridMap/art/shapes/trees/trees_palm/tree_palm_a.dae";
   radius = "0.5";
   windScale = "0.5";
   branchAmp = "0.1";
   detailAmp = "1.5";
   detailFreq = "0.2";
   trunkBendScale = "0.05";
   rigidity = "10";
};

datablock TSForestItemData(tree_palm_b)
{
   internalName = "tree_palm_b";
   shapeFile = "levels/GridMap/art/shapes/trees/trees_palm/tree_palm_b.dae";
   radius = "0.5";
   windScale = "0.5";
   branchAmp = "0.1";
   detailAmp = "1.5";
   detailFreq = "0.2";
   trunkBendScale = "0.05";
};

datablock TSForestItemData(tree_palm_c)
{
   internalName = "tree_palm_c";
   shapeFile = "levels/GridMap/art/shapes/trees/trees_palm/tree_palm_c.dae";
   radius = "0.5";
   windScale = "0.5";
   trunkBendScale = "0.05";
   branchAmp = "0.1";
   detailAmp = "1.5";
   detailFreq = "0.2";
};

datablock TSForestItemData(tree_palm_overhang_b)
{
   internalName = "tree_palm_overhang_b";
   shapeFile = "levels/GridMap/art/shapes/trees/trees_palm/tree_palm_overhang_b.dae";
   windScale = "0.5";
   trunkBendScale = "0.05";
   branchAmp = "0.1";
   detailAmp = "1.5";
   detailFreq = "0.2";
};

datablock TSForestItemData(tree_palm_overhang_a)
{
   internalName = "tree_palm_overhang_a";
   shapeFile = "levels/GridMap/art/shapes/trees/trees_palm/tree_palm_overhang_a.dae";
   windScale = "0.5";
   trunkBendScale = "0.05";
   branchAmp = "0.1";
   detailAmp = "1.5";
   detailFreq = "0.2";
};

datablock TSForestItemData(tree_palm_e)
{
   internalName = "tree_palm_e";
   shapeFile = "levels/GridMap/art/shapes/trees/trees_palm/tree_palm_e.dae";
   radius = "0.5";
   windScale = "0.5";
   trunkBendScale = "0.05";
   branchAmp = "0.1";
   detailAmp = "1.5";
   detailFreq = "0.2";
};

datablock TSForestItemData(tree_palm_f)
{
   internalName = "tree_palm_f";
   shapeFile = "levels/GridMap/art/shapes/trees/trees_palm/tree_palm_f.dae";
   windScale = "0.5";
   trunkBendScale = "0.05";
   branchAmp = "0.1";
   detailAmp = "1.5";
   detailFreq = "0.2";
   radius = "0.3";
};

datablock TSForestItemData(tree_palm_d)
{
   internalName = "tree_palm_d";
   shapeFile = "levels/GridMap/art/shapes/trees/trees_palm/tree_palm_d.dae";
   radius = "0.5";
   windScale = "0.5";
   trunkBendScale = "0.05";
   branchAmp = "0.1";
   detailAmp = "1.5";
   detailFreq = "0.2";
};

datablock TSForestItemData(utah_greybush_01)
{
   internalName = "utah_greybush_01";
   shapeFile = "levels/GridMap/art/shapes/groundcover/utah_greybush_01.dae";
   radius = "0.5";
   windScale = "0.4";
   trunkBendScale = "0.1";
   branchAmp = "0.1";
   detailAmp = "0.3";
   detailFreq = "0.3";
};

datablock TSForestItemData(utah_rock_big_a)
{
   internalName = "utah_rock_big_a";
   shapeFile = "levels/GridMap/art/shapes/rocks/utah_rock_big_a.dae";
   radius = "0.8";
};

datablock TSForestItemData(utah_rock_big_b)
{
   internalName = "utah_rock_big_b";
   shapeFile = "levels/GridMap/art/shapes/rocks/utah_rock_big_b.dae";
   radius = "0.8";
};

datablock TSForestItemData(utah_rock_big_d)
{
   internalName = "utah_rock_big_d";
   shapeFile = "levels/GridMap/art/shapes/rocks/utah_rock_big_d.dae";
   radius = "0.8";
};

datablock TSForestItemData(utah_rock_big_c)
{
   internalName = "utah_rock_big_c";
   shapeFile = "levels/GridMap/art/shapes/rocks/utah_rock_big_c.dae";
   radius = "0.8";
};

datablock TSForestItemData(utah_rock_small_a)
{
   internalName = "utah_rock_small_a";
   shapeFile = "levels/GridMap/art/shapes/rocks/utah_rock_small_a.dae";
   radius = "0.5";
};

datablock TSForestItemData(utah_rock_small_b)
{
   internalName = "utah_rock_small_b";
   shapeFile = "levels/GridMap/art/shapes/rocks/utah_rock_small_b.dae";
   radius = "0.5";
};

datablock TSForestItemData(utah_rock_small_c)
{
   internalName = "utah_rock_small_c";
   shapeFile = "levels/GridMap/art/shapes/rocks/utah_rock_small_c.dae";
   radius = "5";
};

datablock TSForestItemData(utah_rock_small_d)
{
   internalName = "utah_rock_small_d";
   shapeFile = "levels/GridMap/art/shapes/rocks/utah_rock_small_d.dae";
   radius = "0.5";
};

datablock TSForestItemData(utah_rock_small_e)
{
   internalName = "utah_rock_small_e";
   shapeFile = "levels/GridMap/art/shapes/rocks/utah_rock_small_e.dae";
   radius = "0.5";
};

datablock TSForestItemData(Utah_cliff_s_slope_23a)
{
   internalName = "Utah_cliff_s_slope_23a";
   shapeFile = "levels/GridMap/art/shapes/cliffs/Utah_cliff_s_slope_23a.dae";
};

datablock TSForestItemData(Utah_cliff_s_slope_23b)
{
   internalName = "Utah_cliff_s_slope_23b";
   shapeFile = "levels/GridMap/art/shapes/cliffs/Utah_cliff_s_slope_23b.dae";
};

datablock TSForestItemData(Utah_cliff_slope_50a)
{
   internalName = "Utah_cliff_slope_50a";
   shapeFile = "levels/GridMap/art/shapes/cliffs/Utah_cliff_slope_50a.dae";
};

datablock TSForestItemData(Utah_cliff_slope_50b)
{
   internalName = "Utah_cliff_slope_50b";
   shapeFile = "levels/GridMap/art/shapes/cliffs/Utah_cliff_slope_50b.dae";
};

datablock TSForestItemData(Utah_cliff_slope_50c)
{
   internalName = "Utah_cliff_slope_50c";
   shapeFile = "levels/GridMap/art/shapes/cliffs/Utah_cliff_slope_50c.dae";
};

datablock TSForestItemData(Utah_cliff_slope_50d)
{
   internalName = "Utah_cliff_slope_50d";
   shapeFile = "levels/GridMap/art/shapes/cliffs/Utah_cliff_slope_50d.dae";
};

datablock TSForestItemData(Utah_cliff_s_90_concave)
{
   internalName = "Utah_cliff_s_90_concave";
   shapeFile = "levels/GridMap/art/shapes/cliffs/Utah_cliff_s_90_concave.dae";
};

datablock TSForestItemData(Utah_cliff_s_90_convex)
{
   internalName = "Utah_cliff_s_90_convex";
   shapeFile = "levels/GridMap/art/shapes/cliffs/Utah_cliff_s_90_convex.dae";
};

datablock TSForestItemData(Utah_cliff_s_single_01)
{
   internalName = "Utah_cliff_s_single_01";
   shapeFile = "levels/GridMap/art/shapes/cliffs/Utah_cliff_s_single_01.dae";
};

datablock TSForestItemData(Utah_cliff_s_single_02)
{
   internalName = "Utah_cliff_s_single_02";
   shapeFile = "levels/GridMap/art/shapes/cliffs/Utah_cliff_s_single_02.dae";
};

datablock TSForestItemData(Utah_cliff_s_single_03)
{
   internalName = "Utah_cliff_s_single_03";
   shapeFile = "levels/GridMap/art/shapes/cliffs/Utah_cliff_s_single_03.dae";
};

datablock TSForestItemData(Utah_cliff_s_single_04)
{
   internalName = "Utah_cliff_s_single_04";
   shapeFile = "levels/GridMap/art/shapes/cliffs/Utah_cliff_s_single_04.dae";
};

datablock TSForestItemData(Utah_cliff_s_straight_10)
{
   internalName = "Utah_cliff_s_straight_10";
   shapeFile = "levels/GridMap/art/shapes/cliffs/Utah_cliff_s_straight_10.dae";
};

datablock TSForestItemData(Utah_cliff_s_straight_20)
{
   internalName = "Utah_cliff_s_straight_20";
   shapeFile = "levels/GridMap/art/shapes/cliffs/Utah_cliff_s_straight_20.dae";
};

datablock TSForestItemData(Utah_cliff_s_straight_50)
{
   internalName = "Utah_cliff_s_straight_50";
   shapeFile = "levels/GridMap/art/shapes/cliffs/Utah_cliff_s_straight_50.dae";
};

datablock TSForestItemData(Utah_cliff_stand_01)
{
   internalName = "Utah_cliff_stand_01";
   shapeFile = "levels/GridMap/art/shapes/cliffs/Utah_cliff_stand_01.dae";
};

datablock TSForestItemData(Utah_cliff_straight_50)
{
   internalName = "Utah_cliff_straight_50";
   shapeFile = "levels/GridMap/art/shapes/cliffs/Utah_cliff_straight_50.dae";
};

datablock TSForestItemData(Utah_cliff_straight_25)
{
   internalName = "Utah_cliff_straight_25";
   shapeFile = "levels/GridMap/art/shapes/cliffs/Utah_cliff_straight_25.dae";
};

datablock TSForestItemData(Utah_cliff_straight_50_concave)
{
   internalName = "Utah_cliff_straight_50_concave";
   shapeFile = "levels/GridMap/art/shapes/cliffs/Utah_cliff_straight_50_concave.dae";
};

datablock TSForestItemData(Utah_cliff_straight_50_convex)
{
   internalName = "Utah_cliff_straight_50_convex";
   shapeFile = "levels/GridMap/art/shapes/cliffs/Utah_cliff_straight_50_convex.dae";
};

datablock TSForestItemData(Utah_cliff_straight_150)
{
   internalName = "Utah_cliff_straight_150";
   shapeFile = "levels/GridMap/art/shapes/cliffs/Utah_cliff_straight_150.dae";
};

datablock TSForestItemData(Utah_cliff_straight_150b)
{
   internalName = "Utah_cliff_straight_150b";
   shapeFile = "levels/GridMap/art/shapes/cliffs/Utah_cliff_straight_150b.dae";
};
