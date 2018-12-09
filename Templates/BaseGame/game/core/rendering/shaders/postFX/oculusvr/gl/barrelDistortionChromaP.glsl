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
#include "../postFx.hlsl"  
#include "../../torque.hlsl"

uniform sampler2D backBuffer : register(S0);

uniform vec3 LensCenter;    // x=Left X, y=Right X, z=Y
uniform vec2 ScreenCenter;
uniform vec2 Scale;
uniform vec2 ScaleIn;
uniform vec4 HmdWarpParam;
uniform vec4 HmdChromaAbParam; // Chromatic aberration correction

vec4 main( PFXVertToPix IN ) : COLOR0  
{
   vec2 texCoord;
   float xOffset;
   vec2 lensCenter;
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
   
   // Scales input texture coordinates for distortion.
   // ScaleIn maps texture coordinates to Scales to ([-1, 1]), although top/bottom will be
   // larger due to aspect ratio.
   vec2 theta = (texCoord - lensCenter) * ScaleIn; // Scales to [-1, 1]
   float rSq = theta.x * theta.x + theta.y * theta.y;
   vec2 theta1 = theta * (HmdWarpParam.x + HmdWarpParam.y * rSq + HmdWarpParam.z * rSq * rSq + HmdWarpParam.w * rSq * rSq * rSq);

   // Detect whether blue texture coordinates are out of range
   // since these will scaled out the furthest.
   vec2 thetaBlue = theta1 * (HmdChromaAbParam.z + HmdChromaAbParam.w * rSq);
   vec2 tcBlue = lensCenter + Scale * thetaBlue;
   
   vec4 color;
   if (any(clamp(tcBlue, ScreenCenter-vec2(0.25,0.5), ScreenCenter+vec2(0.25, 0.5)) - tcBlue))
   {
      color = vec4(0,0,0,0);
   }
   else
   {
      // Now do blue texture lookup.
      tcBlue.x += xOffset;
      float blue = texture(backBuffer, tcBlue).b;

      // Do green lookup (no scaling).
      vec2 tcGreen = lensCenter + Scale * theta1;
      tcGreen.x += xOffset;
      float green = texture(backBuffer, tcGreen).g;

      // Do red scale and lookup.
      vec2 thetaRed = theta1 * (HmdChromaAbParam.x + HmdChromaAbParam.y * rSq);
      vec2 tcRed = lensCenter + Scale * thetaRed;
      tcRed.x += xOffset;
      float red = texture(backBuffer, tcRed).r;

      color = vec4(red, green, blue, 1);
   }

   return color;    
}
