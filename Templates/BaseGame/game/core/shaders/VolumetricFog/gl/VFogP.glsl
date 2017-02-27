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

uniform sampler2D prepassTex;
uniform sampler2D depthBuffer;
uniform sampler2D frontBuffer;
uniform sampler2D density;
  
uniform float accumTime;
uniform vec4 fogColor;
uniform float fogDensity;
uniform float preBias;
uniform float textured;
uniform float modstrength;
uniform vec4 modspeed;//xy speed layer 1, zw speed layer 2
uniform vec2 viewpoint;
uniform vec2 texscale;
uniform vec3 ambientColor;
uniform float numtiles;
uniform float fadesize;
uniform vec2 PixelSize;

in vec4 _hpos;
#define IN_hpos _hpos
out vec4 OUT_col;

void main()
{
	vec2 uvscreen=((IN_hpos.xy/IN_hpos.w) + 1.0 ) / 2.0;
	uvscreen.y = 1.0 - uvscreen.y;
	
	float obj_test = prepassUncondition( prepassTex, uvscreen).w * preBias;
	float depth = tex2D(depthBuffer,uvscreen).r;
	float front = tex2D(frontBuffer,uvscreen).r;

	if (depth <= front)
	{
		OUT_col = vec4(0,0,0,0);
		return;
	}
	
	else if ( obj_test < depth )
		depth = obj_test;
	if ( front >= 0.0)
		depth -= front;

	float diff = 1.0;
	vec3 col = fogColor.rgb;
	if (textured != 0.0)
	{
		vec2 offset = viewpoint + ((-0.5 + (texscale * uvscreen)) * numtiles);

		vec2 mod1 = tex2D(density,(offset + (modspeed.xy*accumTime))).rg;
		vec2 mod2= tex2D(density,(offset + (modspeed.zw*accumTime))).rg;
		diff = (mod2.r + mod1.r) * modstrength;
		col *= (2.0 - ((mod1.g + mod2.g) * fadesize))/2.0;
	}

   col *= ambientColor;

   vec4 returnColor = vec4(col, 1.0 - saturate(exp(-fogDensity  * depth * diff * fadesize)));

	OUT_col = hdrEncode(returnColor);
}
