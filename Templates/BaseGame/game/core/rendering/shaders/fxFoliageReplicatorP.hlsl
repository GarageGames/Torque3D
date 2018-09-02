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

#include "shdrConsts.h"
#include "shaderModel.hlsl"
//-----------------------------------------------------------------------------
// Structures                                                                  
//-----------------------------------------------------------------------------
struct ConnectData
{
   float4 hpos            : TORQUE_POSITION;
   float2 outTexCoord     : TEXCOORD0;
   float4 color			  : COLOR0;
   float4 groundAlphaCoeff : COLOR1;
   float2 alphaLookup	  : TEXCOORD1;
};

struct Fragout
{
   float4 col : TORQUE_TARGET0;
};

TORQUE_UNIFORM_SAMPLER2D(diffuseMap, 0);
TORQUE_UNIFORM_SAMPLER2D(alphaMap, 1);

uniform float4 groundAlpha;
uniform float4 ambient;

//-----------------------------------------------------------------------------
// Main                                                                        
//-----------------------------------------------------------------------------
Fragout main( ConnectData IN )
{
   Fragout OUT;

	float4 alpha = TORQUE_TEX2D(alphaMap, IN.alphaLookup);
   OUT.col = float4( ambient.rgb * IN.lum.rgb, 1.0 ) * TORQUE_TEX2D(diffuseMap, IN.texCoord);
   OUT.col.a = OUT.col.a * min(alpha, groundAlpha + IN.groundAlphaCoeff.x).x;
   
   return OUT;
}
