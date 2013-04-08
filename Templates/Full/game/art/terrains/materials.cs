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
   ShowDust = "false";  
   showFootprints = "false"; 
   materialTag0 = "Terrain"; 
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

singleton Material(TerrainFX_grass1dry)  
{  
   mapTo = "grass1-dry";  
   footstepSoundId = 0;  
   terrainMaterials = "1";  
   ShowDust = "false";  
   showFootprints = "false";
   materialTag0 = "Terrain";  
};  

new TerrainMaterial()
{
   internalName = "grass1-dry";
   diffuseMap = "art/terrains/Example/grass1-dry";
   detailMap = "art/terrains/Example/grass1-dry_d";
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
   ShowDust = "false";  
   showFootprints = "false";
   materialTag0 = "Terrain";  
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
   footstepSoundId = 0;  
   terrainMaterials = "1";  
   ShowDust = "false";  
   showFootprints = "false"; 
   materialTag0 = "Terrain"; 
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
// Sample sand
// ----------------------------------------------------------------------------

singleton Material(TerrainFX_sand)  
{  
   mapTo = "sand";  
   footstepSoundId = 0;  
   terrainMaterials = "1";  
   ShowDust = "false";  
   showFootprints = "false"; 
   materialTag0 = "Terrain";  
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
