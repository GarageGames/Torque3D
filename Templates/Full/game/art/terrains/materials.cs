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
new TerrainMaterial()
{
   diffuseMap = "art/terrains/Desert01";
   diffuseSize = "200";
   detailMap = "art/terrains/sand_blown_micro";
   detailSize = "10";
   detailStrength = "0.2";
   internalName = "sand_micro";
   normalMap = "art/terrains/sand_blown_micro_normal";
   parallaxScale = "0.05";
   detailDistance = "200";
};

new TerrainMaterial()
{
   diffuseMap = "art/terrains/cliff_wall";
   diffuseSize = "200";
   useSideProjection = "1";
   internalName = "cliff";
   detailMap = "art/terrains/cliff_wall";
   detailSize = "75";
   detailStrength = "0.5";
   detailDistance = "2000";
   normalMap = "art/terrains/cliff_wall_normal";
};

new Material( SandTerrain )
{
   customFootstepSound = playerFootstepSand;
   ImpactSoundId = 0;         // Soft impact sound.
   ShowDust = true;           // Show dust particles.
   ShowFootprints = true;     // Show footprints.
};

new Material( RockTerrain )
{
   customFootstepSound = playerFootstepStone;
   ImpactSoundId = 1;         // Hard impact sound.
   ShowDust = false;           // Show dust particles.
   ShowFootprints = false;     // Show footprints.
};

new Material( TerrainDesert01 : SandTerrain )
{
   MapTo = "Desert01";
};

new Material( Terraincliff_wall01 : RockTerrain )
{
   MapTo = "cliff_wall";
};
