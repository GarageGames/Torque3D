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

#include "shadergen:/autogenConditioners.h"

#include "farFrustumQuad.hlsl"
#include "lightingUtils.hlsl"
#include "../../lighting.hlsl"


struct ConvexConnectP
{
   float4 ssPos : TEXCOORD0;
   float3 vsEyeDir : TEXCOORD1;
};

float4 main(   ConvexConnectP IN,
               uniform sampler2D prePassBuffer : register(S0),
               
               uniform float4 lightPosition,
               uniform float4 lightColor,
               uniform float  lightRange,
               
               uniform float4 vsFarPlane,
               uniform float4 rtParams0 ) : COLOR0
{
   // Compute scene UV
   float3 ssPos = IN.ssPos.xyz / IN.ssPos.w;
   float2 uvScene = getUVFromSSPos(ssPos, rtParams0);
   
   // Sample/unpack the normal/z data
   float4 prepassSample = prepassUncondition(prePassBuffer, uvScene);
   float3 normal = prepassSample.rgb;
   float depth = prepassSample.a;
   
   // Eye ray - Eye -> Pixel
   float3 eyeRay = getDistanceVectorToPlane(-vsFarPlane.w, IN.vsEyeDir, vsFarPlane);
   float3 viewSpacePos = eyeRay * depth;
      
   // Build light vec, get length, clip pixel if needed
   float3 lightVec = lightPosition.xyz - viewSpacePos;
   float lenLightV = length(lightVec);
   clip(lightRange - lenLightV);

   // Do a very simple falloff instead of real attenuation
   float atten = 1.0 - saturate(lenLightV / lightRange);

   // Normalize lightVec
   lightVec /= lenLightV;

   // N.L * Attenuation
   float Sat_NL_Att = saturate(dot(lightVec, normal)) * atten;     
   
   // Output, no specular
   return lightinfoCondition(lightColor.rgb, Sat_NL_Att, 0.0, 0.0);
}
