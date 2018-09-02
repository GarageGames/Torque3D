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
#include "../../torque.hlsl"

TORQUE_UNIFORM_SAMPLER2D(backBuffer, 0);

uniform float3 LensCenter;    // x=Left X, y=Right X, z=Y
uniform float2 ScreenCenter;
uniform float2 Scale;
uniform float2 ScaleIn;
uniform float4 HmdWarpParam;

// Scales input texture coordinates for distortion.
// ScaleIn maps texture coordinates to Scales to ([-1, 1]), although top/bottom will be
// larger due to aspect ratio.
float2 HmdWarp(float2 in01, float2 lensCenter)
{
   float2 theta = (in01 - lensCenter) * ScaleIn; // Scales to [-1, 1]
   float rSq = theta.x * theta.x + theta.y * theta.y;
   float2 theta1 = theta * (HmdWarpParam.x + HmdWarpParam.y * rSq + HmdWarpParam.z * rSq * rSq + HmdWarpParam.w * rSq * rSq * rSq);
   return lensCenter + Scale * theta1;
}

float4 main( PFXVertToPix IN ) : TORQUE_TARGET0  
{
   float2 texCoord;
   float xOffset;
   float2 lensCenter;
   lensCenter.y = LensCenter.z;
   if(IN.uv0.x < 0.5)
   {
      texCoord.x = IN.uv0.x;
      texCoord.y = IN.uv0.y;
      xOffset = 0.0;
      lensCenter.x = LensCenter.x;
   }
   else
   {
      texCoord.x = IN.uv0.x - 0.5;
      texCoord.y = IN.uv0.y;
      xOffset = 0.5;
      lensCenter.x = LensCenter.y;
   }
   
   float2 tc = HmdWarp(texCoord, lensCenter);
   
   float4 color;
   if (any(clamp(tc, ScreenCenter-float2(0.25,0.5), ScreenCenter+float2(0.25, 0.5)) - tc))
   {
      color = float4(0,0,0,0);
   }
   else
   {
      tc.x += xOffset;
      color = TORQUE_TEX2D(backBuffer, tc);
   }

   return color;    
}
