// SSAO: depth-to-viewspace-position pixel shader

#include "shadergen:/autogenConditioners.h"
#include "./../postFx.hlsl"

uniform sampler2D prepassMap : register(S0);

uniform float2 nearFar;


float4 main(PFXVertToPix IN) : COLOR
{
   float4 prepass = prepassUncondition(prepassMap, IN.uv0);
   float depth = prepass.a;
   
   // Interpolated ray pointing to far plane in view space
   float3 frustumRayVS = normalize(IN.wsEyeRay);
   
   return float4(nearFar.y * depth * frustumRayVS, 1.0);
}