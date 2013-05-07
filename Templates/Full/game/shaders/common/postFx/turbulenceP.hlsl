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

#include "./postFx.hlsl"

uniform float  accumTime;

float4 main( PFXVertToPix IN, uniform sampler2D inputTex : register(S0) ) : COLOR
{
	float reduction = 128;	
	float power = 1.0;
	float speed = 3.0;
	float frequency=8;
	
	float backbuffer_edge_coef=0.98;
	float2 screen_center = float2(0.5, 0.5);	
	float2 cPos = (IN.uv0 - screen_center);
	
	float len = 1.0 - length(cPos);		
	float2 uv = clamp((cPos / len * cos(len * frequency - (accumTime * speed)) * (power / reduction)), 0, 1);
	return tex2D(inputTex, IN.uv0 * backbuffer_edge_coef + uv);

//    float4 color = tex2D(inputTex, IN.uv0 * backbuffer_edge_coef+(sin*right));           
//	return color;

}