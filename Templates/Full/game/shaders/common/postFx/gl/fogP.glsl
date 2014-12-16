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

#include "../../gl/hlslCompat.glsl"

#include "shadergen:/autogenConditioners.h"
#include "../../gl/torque.glsl"

uniform sampler2D prepassTex ;
uniform vec3    eyePosWorld;
uniform vec4    fogColor;
uniform vec3    fogData;
uniform vec4    rtParams0;

in vec2 uv0;
in vec3 wsEyeRay;

out vec4 OUT_col;

void main()
{   
   //vec2 prepassCoord = ( uv0.xy * rtParams0.zw ) + rtParams0.xy;   
   float depth = prepassUncondition( prepassTex, uv0 ).w;
   //return vec4( depth, 0, 0, 0.7 );
   
   float factor = computeSceneFog( eyePosWorld,
                                   eyePosWorld + ( wsEyeRay * depth ),
                                   fogData.x, 
                                   fogData.y, 
                                   fogData.z );

   OUT_col = hdrEncode( vec4( fogColor.rgb, 1.0 - saturate( factor ) ) );     
}