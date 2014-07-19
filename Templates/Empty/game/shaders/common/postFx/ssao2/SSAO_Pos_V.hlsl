// SSAO: depth-to-viewspace-position vertex shader

#include "./../postFx.hlsl"
#include "./../../torque.hlsl"

uniform float4 rtParams0;
uniform float4 rtParams1;
uniform float4 rtParams2;
uniform float4 rtParams3;

uniform float2 nearFar;
uniform float2 targetSize;

// script constants
uniform float cameraFOV;

                    
PFXVertToPix main( PFXVert IN )
{
   PFXVertToPix OUT;
   
   OUT.hpos = IN.pos;
   OUT.uv0 = viewportCoordToRenderTarget( IN.uv, rtParams0 ); 
   OUT.uv1 = viewportCoordToRenderTarget( IN.uv, rtParams1 ); 
   OUT.uv2 = viewportCoordToRenderTarget( IN.uv, rtParams2 ); 
   OUT.uv3 = viewportCoordToRenderTarget( IN.uv, rtParams3 );
   
   float2 cornerVec = normalize(targetSize);
   float len = nearFar.y * tan(radians(cameraFOV/2.0));
   
   // Vector pointing toward view space frustum corner cooresponding to this vertex
   OUT.wsEyeRay = float3(len * (IN.pos.xy * cornerVec), nearFar.y);
   
   return OUT;
}
