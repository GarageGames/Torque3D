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

#include "../postFx.hlsl"
#include "shadergen:/autogenConditioners.h"

// GPU Gems 3, pg 443-444
float GetEdgeWeight(float2 uv0, in sampler2D prepassBuffer, in float2 targetSize)
{
   float2 offsets[9] = {
      float2( 0.0,  0.0),
      float2(-1.0, -1.0),
      float2( 0.0, -1.0),
      float2( 1.0, -1.0),
      float2( 1.0,  0.0),
      float2( 1.0,  1.0),
      float2( 0.0,  1.0),
      float2(-1.0,  1.0),
      float2(-1.0,  0.0),
   };
   
   
   float2 PixelSize = 1.0 / targetSize;
   
   float Depth[9];
   float3 Normal[9];
   
   for(int i = 0; i < 9; i++)
   {
      float2 uv = uv0 + offsets[i] * PixelSize;
      float4 gbSample = prepassUncondition( prepassBuffer, uv );
      Depth[i] = gbSample.a;
      Normal[i] = gbSample.rgb;
   }
   
   float4 Deltas1 = float4(Depth[1], Depth[2], Depth[3], Depth[4]);
   float4 Deltas2 = float4(Depth[5], Depth[6], Depth[7], Depth[8]);
   
   Deltas1 = abs(Deltas1 - Depth[0]);
   Deltas2 = abs(Depth[0] - Deltas2);
   
   float4 maxDeltas = max(Deltas1, Deltas2);
   float4 minDeltas = max(min(Deltas1, Deltas2), 0.00001);
   
   float4 depthResults = step(minDeltas * 25.0, maxDeltas);
   
   Deltas1.x = dot(Normal[1], Normal[0]);
   Deltas1.y = dot(Normal[2], Normal[0]);
   Deltas1.z = dot(Normal[3], Normal[0]);
   Deltas1.w = dot(Normal[4], Normal[0]);
   
   Deltas2.x = dot(Normal[5], Normal[0]);
   Deltas2.y = dot(Normal[6], Normal[0]);
   Deltas2.z = dot(Normal[7], Normal[0]);
   Deltas2.w = dot(Normal[8], Normal[0]);
   
   Deltas1 = abs(Deltas1 - Deltas2);
   
   float4 normalResults = step(0.4, Deltas1);
   
   normalResults = max(normalResults, depthResults);
   
   return dot(normalResults, float4(1.0, 1.0, 1.0, 1.0)) * 0.25;
}

float4 main( PFXVertToPix IN, 
             uniform sampler2D prepassBuffer :register(S0),
             uniform float2 targetSize : register(C0) ) : COLOR0
{
   return GetEdgeWeight(IN.uv0, prepassBuffer, targetSize );//rtWidthHeightInvWidthNegHeight.zw);
}
