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

// ----------------------------------------------------------------------------
// Sample grass
// ----------------------------------------------------------------------------

singleton Material(TerrainFX_grass1)
{
   mapTo = "grass1";
   footstepSoundId = 0;
   terrainMaterials = "1";
   ShowDust = "1";
   showFootprints = "1";
   materialTag0 = "Terrain";
   effectColor[0] = "0.42 0.42 0 1";
   effectColor[1] = "0.42 0.42 0 1";
   impactSoundId = "0";
};

new TerrainMaterial()
{
   internalName = "grass1";
   diffuseMap = "art/terrains/Example/grass1";
   detailMap = "art/terrains/Example/grass1_d";
   detailSize = "10";
   isManaged = "1";
   detailBrightness = "1";
   Enabled = "1";
   diffuseSize = "200";
};

singleton Material(TerrainFX_grass2)
{
   mapTo = "grass2";
   footstepSoundId = 0;
   terrainMaterials = "1";
   ShowDust = "1";
   showFootprints = "1";
   materialTag0 = "Terrain";
   effectColor[0] = "0.42 0.42 0 1";
   effectColor[1] = "0.42 0.42 0 1";
   impactSoundId = "0";
};

new TerrainMaterial()
{
   internalName = "grass2";
   diffuseMap = "art/terrains/Example/grass2";
   detailMap = "art/terrains/Example/grass2_d";
   detailSize = "10";
   isManaged = "1";
   detailBrightness = "1";
   Enabled = "1";
   diffuseSize = "200";
};

singleton Material(TerrainFX_grass1dry)
{
   mapTo = "grass1_dry";
   footstepSoundId = 0;
   terrainMaterials = "1";
   ShowDust = "1";
   showFootprints = "1";
   materialTag0 = "Terrain";
   effectColor[0] = "0.63 0.55 0 1";
};

new TerrainMaterial()
{
   internalName = "grass1_dry";
   diffuseMap = "art/terrains/Example/grass1_dry";
   detailMap = "art/terrains/Example/grass1_dry_d";
   detailSize = "10";
   detailDistance = "100";
   isManaged = "1";
   detailBrightness = "1";
   Enabled = "1";
   diffuseSize = "250";
   detailStrength = "2";
};

singleton Material(TerrainFX_dirt_grass)
{
   mapTo = "dirt_grass";
   footstepSoundId = 0;
   terrainMaterials = "1";
   ShowDust = "1";
   showFootprints = "1";
   materialTag0 = "Terrain";
   effectColor[0] = "0.63 0.55 0 1";
};

new TerrainMaterial()
{
   internalName = "dirt_grass";
   diffuseMap = "art/terrains/Example/dirt_grass";
   detailMap = "art/terrains/Example/dirt_grass_d";
   detailSize = "5";
   detailDistance = "100";
   isManaged = "1";
   detailBrightness = "1";
   Enabled = "1";
   diffuseSize = "200";
};

// ----------------------------------------------------------------------------
// Sample rock
// ----------------------------------------------------------------------------

singleton Material(TerrainFX_rocktest)
{
   mapTo = "rocktest";
   footstepSoundId = "1";
   terrainMaterials = "1";
   ShowDust = "1";
   showFootprints = "1";
   materialTag0 = "Terrain";
   impactSoundId = "1";
   effectColor[0] = "0.25 0.25 0.25 1";
   effectColor[1] = "0.25 0.25 0.25 0";
};

new TerrainMaterial()
{
   internalName = "rocktest";
   diffuseMap = "art/terrains/Example/rocktest";
   detailMap = "art/terrains/Example/rocktest_d";
   detailSize = "10";
   detailDistance = "100";
   isManaged = "1";
   detailBrightness = "1";
   Enabled = "1";
   diffuseSize = "400";
};

// ----------------------------------------------------------------------------
// Sample rock
// ----------------------------------------------------------------------------

singleton Material(TerrainFX_stone)
{
   mapTo = "stone";
   footstepSoundId = "1";
   terrainMaterials = "1";
   ShowDust = "1";
   showFootprints = "1";
   materialTag0 = "Terrain";
   impactSoundId = "1";
   effectColor[0] = "0.25 0.25 0.25 1";
   effectColor[1] = "0.25 0.25 0.25 0";
};

new TerrainMaterial()
{
   internalName = "stone";
   diffuseMap = "art/terrains/Example/stone";
   detailMap = "art/terrains/Example/stone_d";
   detailSize = "10";
   detailDistance = "100";
   isManaged = "1";
   detailBrightness = "1";
   Enabled = "1";
   diffuseSize = "400";
   useSideProjection = "0";
};
// ----------------------------------------------------------------------------
// Sample sand
// ----------------------------------------------------------------------------

singleton Material(TerrainFX_sand)
{
   mapTo = "sand";
   footstepSoundId = "3";
   terrainMaterials = "1";
   ShowDust = "1";
   showFootprints = "1";
   materialTag0 = "Terrain";
   specularPower[0] = "1";
   effectColor[0] = "0.84 0.71 0.5 1";
   effectColor[1] = "0.84 0.71 0.5 0.349";
};

new TerrainMaterial()
{
   internalName = "sand";
   diffuseMap = "art/terrains/Example/sand";
   detailMap = "art/terrains/Example/sand_d";
   detailSize = "10";
   detailDistance = "100";
   isManaged = "1";
   detailBrightness = "1";
   Enabled = "1";
   diffuseSize = "200";
};
