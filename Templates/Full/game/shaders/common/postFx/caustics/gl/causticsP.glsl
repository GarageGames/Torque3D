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

#include "../../../gl/hlslCompat.glsl"
#include "../../gl/postFX.glsl"
#include "shadergen:/autogenConditioners.h"

uniform vec3    eyePosWorld;
uniform vec4    rtParams0;
uniform vec4    waterFogPlane;
uniform float     accumTime;

uniform sampler2D prepassTex;
uniform sampler2D causticsTex0;
uniform sampler2D causticsTex1;
uniform vec2 targetSize;

out vec4 OUT_col;

float distanceToPlane(vec4 plane, vec3 pos)
{
   return (plane.x * pos.x + plane.y * pos.y + plane.z * pos.z) + plane.w;
}

void main()             
{   
   //Sample the pre-pass
   vec4 prePass = prepassUncondition( prepassTex, IN_uv0 );
   
   //Get depth
   float depth = prePass.w;   
   if(depth > 0.9999)
   {
      OUT_col = vec4(0,0,0,0);
      return;
   }
   
   //Get world position
   vec3 pos = eyePosWorld + IN_wsEyeRay * depth;
   
   // Check the water depth
   float waterDepth = -distanceToPlane(waterFogPlane, pos);
   if(waterDepth < 0)
   {
      OUT_col = vec4(0,0,0,0);
      return;
   }
   waterDepth = saturate(waterDepth);
   
   //Use world position X and Y to calculate caustics UV 
   vec2 causticsUV0 = mod(abs(pos.xy * 0.25), vec2(1, 1));
   vec2 causticsUV1 = mod(abs(pos.xy * 0.2), vec2(1, 1));
   
   //Animate uvs
   float timeSin = sin(accumTime);
   causticsUV0.xy += vec2(accumTime*0.1, timeSin*0.2);
   causticsUV1.xy -= vec2(accumTime*0.15, timeSin*0.15);   
   
   //Sample caustics texture   
   vec4 caustics = texture(causticsTex0, causticsUV0);   
   caustics *= texture(causticsTex1, causticsUV1);
   
   //Use normal Z to modulate caustics  
   //float waterDepth = 1 - saturate(pos.z + waterFogPlane.w + 1);
   caustics *= saturate(prePass.z) * pow(1-depth, 64) * waterDepth; 
      
   OUT_col = caustics;   
}
