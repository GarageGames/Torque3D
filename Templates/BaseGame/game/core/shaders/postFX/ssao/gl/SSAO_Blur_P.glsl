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
#include "shadergen:/autogenConditioners.h"

in vec4 uv0;
#define IN_uv0 uv0
in vec2 uv1;
#define IN_uv1 uv1
in vec2 uv2;
#define IN_uv2 uv2
in vec2 uv3;
#define IN_uv3 uv3

in vec2 uv4;
#define IN_uv4 uv4
in vec2 uv5;
#define IN_uv5 uv5
in vec2 uv6;
#define IN_uv6 uv6
in vec2 uv7;
#define IN_uv7 uv7

uniform sampler2D occludeMap ;
uniform sampler2D prepassMap ;
uniform float blurDepthTol;
uniform float blurNormalTol;

out vec4 OUT_col;

void _sample( vec2 uv, float weight, vec4 centerTap, inout int usedCount, inout float occlusion, inout float total )
{
   //return;
   vec4 tap = prepassUncondition( prepassMap, uv );   
   
   if ( abs( tap.a - centerTap.a ) < blurDepthTol )
   {
      if ( dot( tap.xyz, centerTap.xyz ) > blurNormalTol )
      {
         usedCount++;
         total += weight;
         occlusion += texture( occludeMap, uv ).r * weight;
      }
   }   
}

void main()
{   
   //vec4 centerTap;
   vec4 centerTap = prepassUncondition( prepassMap, IN_uv0.zw );
   
   //return centerTap;
   
   //float centerOcclude = texture( occludeMap, IN_uv0.zw ).r;
   //return vec4( centerOcclude.rrr, 1 );

   vec4 kernel = vec4( 0.175, 0.275, 0.375, 0.475 ); //25f;

   float occlusion = 0;
   int usedCount = 0;
   float total = 0.0;
         
   _sample( IN_uv0.xy, kernel.x, centerTap, usedCount, occlusion, total );
   _sample( IN_uv1, kernel.y, centerTap, usedCount, occlusion, total );
   _sample( IN_uv2, kernel.z, centerTap, usedCount, occlusion, total );
   _sample( IN_uv3, kernel.w, centerTap, usedCount, occlusion, total );
   
   _sample( IN_uv4, kernel.x, centerTap, usedCount, occlusion, total );
   _sample( IN_uv5, kernel.y, centerTap, usedCount, occlusion, total );
   _sample( IN_uv6, kernel.z, centerTap, usedCount, occlusion, total );
   _sample( IN_uv7, kernel.w, centerTap, usedCount, occlusion, total );   
   
   occlusion += texture( occludeMap, IN_uv0.zw ).r * 0.5;
   total += 0.5;
   //occlusion /= 3.0;
   
   //occlusion /= (float)usedCount / 8.0;
   occlusion /= total;
   
   OUT_col = vec4( vec3(occlusion), 1 );   
   
   
   //return vec4( 0,0,0,occlusion );
   
   //vec3 color = texture( colorMap, IN_uv0.zw );
      
   //return vec4( color, occlusion );
}