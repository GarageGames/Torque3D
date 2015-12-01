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

// Volumetric Fog final pixel shader V2.00

#include "shadergen:/autogenConditioners.h"
#include "../torque.hlsl"

uniform sampler2D prepassTex : register(S0);
uniform sampler2D depthBuffer : register(S1);
uniform sampler2D frontBuffer : register(S2);
uniform sampler2D density : register(S3);
  
uniform float accumTime;
uniform float4 fogColor;
uniform float fogDensity;
uniform float preBias;
uniform float textured;
uniform float modstrength;
uniform float4 modspeed;//xy speed layer 1, zw speed layer 2
uniform float2 viewpoint;
uniform float2 texscale;
uniform float3 ambientColor;
uniform float numtiles;
uniform float fadesize;
uniform float2 PixelSize;

struct ConnectData
{
   float4 hpos : POSITION;
   float4 htpos : TEXCOORD0;
   float2 uv0 : TEXCOORD1;
};

float4 main( ConnectData IN ) : COLOR0
{
	float2 uvscreen=((IN.htpos.xy/IN.htpos.w) + 1.0 ) / 2.0;
	uvscreen.y = 1.0 - uvscreen.y;
	
	float obj_test = prepassUncondition( prepassTex, uvscreen).w * preBias;
	float depth = tex2D(depthBuffer,uvscreen).r;
	float front = tex2D(frontBuffer,uvscreen).r;

	if (depth <= front)
		return float4(0,0,0,0);
	else if ( obj_test < depth )
		depth = obj_test;
	if ( front >= 0.0)
		depth -= front;

	float diff = 1.0;
	float3 col = fogColor.rgb;
	if (textured != 0.0)
	{
		float2 offset = viewpoint + ((-0.5 + (texscale * uvscreen)) * numtiles);

		float2 mod1 = tex2D(density,(offset + (modspeed.xy*accumTime))).rg;
		float2 mod2= tex2D(density,(offset + (modspeed.zw*accumTime))).rg;
		diff = (mod2.r + mod1.r) * modstrength;
		col *= (2.0 - ((mod1.g + mod2.g) * fadesize))/2.0;
	}

	col *= ambientColor;

	float4 resultColor = float4(col, 1.0 - saturate(exp(-fogDensity  * depth * diff * fadesize)));

	return hdrEncode(resultColor);
}
