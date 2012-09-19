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

#include "shaders/common/postFx/postFx.hlsl"

uniform sampler2D diffuseMap : register(S0);

struct VertToPix
{
	float4 hpos       : POSITION;
	float2 uv        : TEXCOORD0;
};

static float offset[3] = { 0.0, 1.3846153846, 3.2307692308 };
static float weight[3] = { 0.2270270270, 0.3162162162, 0.0702702703 };  

uniform float2 oneOverTargetSize;

float4 main( VertToPix IN ) : COLOR
{	
	float4 OUT = tex2D( diffuseMap, IN.uv ) * weight[0];
			        
	for ( int i=1; i < 3; i++ )
	{
		float2 sample = (BLUR_DIR * offset[i]) * oneOverTargetSize;
		OUT += tex2D( diffuseMap, IN.uv + sample ) * weight[i];  
		OUT += tex2D( diffuseMap, IN.uv - sample ) * weight[i];  
	}
					   
	return OUT;
}
