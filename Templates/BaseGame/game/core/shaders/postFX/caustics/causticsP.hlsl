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
#include "../../shaderModelAutoGen.hlsl"

uniform float     accumTime;
uniform float3    eyePosWorld;
uniform float4    rtParams0;
uniform float4    waterFogPlane;

TORQUE_UNIFORM_SAMPLER2D(prepassTex, 0);
TORQUE_UNIFORM_SAMPLER2D(causticsTex0, 1);
TORQUE_UNIFORM_SAMPLER2D(causticsTex1, 2);

float distanceToPlane(float4 plane, float3 pos)
{
   return (plane.x * pos.x + plane.y * pos.y + plane.z * pos.z) + plane.w;
}

float4 main( PFXVertToPix IN ) : TORQUE_TARGET0
{   
   //Sample the pre-pass
   float4 prePass = TORQUE_PREPASS_UNCONDITION( prepassTex, IN.uv0 );
   
   //Get depth
   float depth = prePass.w;   
   if(depth > 0.9999)
      return float4(0,0,0,0);
   
   //Get world position
   float3 pos = eyePosWorld + IN.wsEyeRay * depth;
   
   // Check the water depth
   float waterDepth = -distanceToPlane(waterFogPlane, pos);
   if(waterDepth < 0)
      return float4(0,0,0,0);
   waterDepth = saturate(waterDepth);
   
   //Use world position X and Y to calculate caustics UV 
   float2 causticsUV0 = (abs(pos.xy * 0.25) % float2(1, 1));
   float2 causticsUV1 = (abs(pos.xy * 0.2) % float2(1, 1));
   
   //Animate uvs
   float timeSin = sin(accumTime);
   causticsUV0.xy += float2(accumTime*0.1, timeSin*0.2);
   causticsUV1.xy -= float2(accumTime*0.15, timeSin*0.15);   
   
   //Sample caustics texture   
   float4 caustics = TORQUE_TEX2D(causticsTex0, causticsUV0);   
   caustics *= TORQUE_TEX2D(causticsTex1, causticsUV1);
   
   //Use normal Z to modulate caustics  
   //float waterDepth = 1 - saturate(pos.z + waterFogPlane.w + 1);
   caustics *= saturate(prePass.z) * pow(abs(1-depth), 64) * waterDepth; 
      
   return caustics;   
}
