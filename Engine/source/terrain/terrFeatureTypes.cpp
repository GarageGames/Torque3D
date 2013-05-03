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

#include "platform/platform.h"
#include "terrain/terrFeatureTypes.h"

#include "materials/materialFeatureTypes.h"


ImplementFeatureType( MFT_TerrainBaseMap, MFG_Texture, 100.0f, false );
ImplementFeatureType( MFT_TerrainParallaxMap, MFG_Texture, 101.0f, false );
ImplementFeatureType( MFT_TerrainDetailMap, MFG_Texture, 102.0f, false );
ImplementFeatureType( MFT_TerrainNormalMap, MFG_Texture, 103.0f, false );
ImplementFeatureType( MFT_TerrainMacroMap, MFG_Texture, 104.0f, false );
ImplementFeatureType( MFT_TerrainLightMap, MFG_Texture, 105.0f, false );
ImplementFeatureType( MFT_TerrainSideProject, MFG_Texture, 106.0f, false );
ImplementFeatureType( MFT_TerrainAdditive, MFG_PostProcess, 999.0f, false );

