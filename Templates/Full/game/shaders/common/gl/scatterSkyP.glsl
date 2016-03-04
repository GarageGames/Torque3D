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

#include "torque.glsl"
#include "hlslCompat.glsl"


// Conn
in vec4  rayleighColor;
#define IN_rayleighColor rayleighColor
in vec4  mieColor;
#define IN_mieColor mieColor
in vec3  v3Direction;
#define IN_v3Direction v3Direction
in float zPosition;
#define IN_zPosition zPosition
in vec3  pos;
#define IN_pos pos

uniform samplerCube nightSky ;
uniform vec4 nightColor;
uniform vec2 nightInterpAndExposure;
uniform float useCubemap;
uniform vec3 lightDir;
uniform vec3 sunDir;

out vec4 OUT_col;

void main()
{ 

   float fCos = dot( lightDir, IN_v3Direction ) / length(IN_v3Direction);
   float fCos2 = fCos*fCos;
    
   float g = -0.991;
   float g2 = -0.991 * -0.991;

   float fMiePhase = 1.5 * ((1.0 - g2) / (2.0 + g2)) * (1.0 + fCos2) / pow(abs(1.0 + g2 - 2.0*g*fCos), 1.5);
   
   vec4 color = IN_rayleighColor + fMiePhase * IN_mieColor;
   color.a = color.b;
   
   vec4 nightSkyColor = texture(nightSky, -v3Direction);
   nightSkyColor = mix(nightColor, nightSkyColor, useCubemap);

   float fac = dot( normalize( pos ), sunDir );
   fac = max( nightInterpAndExposure.y, pow( clamp( fac, 0.0, 1.0 ), 2 ) );
   OUT_col = mix( color, nightSkyColor, nightInterpAndExposure.y );
   
   // Clip based on the camera-relative
   // z position of the vertex, passed through
   // from the vertex position.
   if(zPosition < 0.0)
      discard;

   OUT_col.a = 1;
   
   OUT_col = clamp(OUT_col, 0.0, 1.0);
   
   OUT_col = hdrEncode( OUT_col );
}
