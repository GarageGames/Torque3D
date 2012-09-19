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
#ifndef _SINGLELIGHTSHADOWMAP_H_
#define _SINGLELIGHTSHADOWMAP_H_

#ifndef _LIGHTSHADOWMAP_H_
#include "lighting/shadowMap/lightShadowMap.h"
#endif

//
// SingleLightShadowMap, holds the shadow map and various other things for a light.
//
// This represents everything we need to render the shadowmap for one light.
class SingleLightShadowMap : public LightShadowMap
{
public:
   SingleLightShadowMap( LightInfo *light );
   ~SingleLightShadowMap();

   // LightShadowMap
   virtual ShadowType getShadowType() const { return ShadowType_Spot; }
   virtual void _render( RenderPassManager* renderPass, const SceneRenderState *diffuseState );
   virtual void setShaderParameters(GFXShaderConstBuffer* params, LightingShaderConstants* lsc);
};


#endif // _SINGLELIGHTSHADOWMAP_H_