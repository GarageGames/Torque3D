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

#ifndef _TERRRENDER_H_
#define _TERRRENDER_H_

#ifndef _TERRDATA_H_
#include "terrain/terrData.h"
#endif

enum TerrConstants 
{
   MaxClipPlanes       = 8, ///< left, right, top, bottom - don't need far tho...
   //MaxTerrainMaterials = 256,

   MaxTerrainLights = 64,
   MaxVisibleLights = 31,
   ClipPlaneMask    = (1 << MaxClipPlanes) - 1,
   FarSphereMask    = 0x80000000,
   FogPlaneBoxMask  = 0x40000000,
};

class SceneRenderState;


// Allows a lighting system to plug into terrain rendering
class TerrainLightingPlugin
{
public:
   virtual ~TerrainLightingPlugin() {}
   
   virtual void setupLightStage(LightManager * lm, LightInfo* light, SceneData& sgData, BaseMatInstance* basemat, BaseMatInstance** dmat) = 0;
   virtual void cleanupLights(LightManager * lm) {}
};


/// A special texture profile used for the terrain layer id map.
GFX_DeclareTextureProfile( TerrainLayerTexProfile );

#endif // _TERRRENDER_H_
