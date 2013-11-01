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
uniform float2 projectionOffset;
uniform float4 targetViewport;

float4 main( PFXVertToPix IN, uniform sampler2D inputTex : register(S0) ) : COLOR
{
	float speed = 2.0;
	float distortion = 6.0;
	
	float y = IN.uv0.y + (cos((IN.uv0.y+projectionOffset.y) * distortion + accumTime * speed) * 0.01);
   float x = IN.uv0.x + (sin((IN.uv0.x+projectionOffset.x) * distortion + accumTime * speed) * 0.01);

   // Clamp the calculated uv values to be within the target's viewport
	y = clamp(y, targetViewport.y, targetViewport.w);
	x = clamp(x, targetViewport.x, targetViewport.z);
	
    return tex2D (inputTex, float2(x, y));
}
