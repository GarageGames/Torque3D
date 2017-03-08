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

#define IN_HLSL
#include "shdrConsts.h"
#include "shaderModel.hlsl"

//-----------------------------------------------------------------------------
// Data
//-----------------------------------------------------------------------------
struct v2f
{
   float4 HPOS             : TORQUE_POSITION;
	float2 TEX0             : TEXCOORD0;
	float4 tangentToCube0   : TEXCOORD1;
	float4 tangentToCube1   : TEXCOORD2;
	float4 tangentToCube2   : TEXCOORD3;
   float4 lightVec         : TEXCOORD4;
   float3 pixPos           : TEXCOORD5;
   float3 eyePos           : TEXCOORD6;
};



struct Fragout
{
   float4 col : TORQUE_TARGET0;
};

// Uniforms
TORQUE_UNIFORM_SAMPLER2D(diffMap,0);
//TORQUE_UNIFORM_SAMPLERCUBE(cubeMap, 1); not used?
TORQUE_UNIFORM_SAMPLER2D(bumpMap,2);

uniform float4    specularColor   : register(PC_MAT_SPECCOLOR);
uniform float4    ambient : register(PC_AMBIENT_COLOR);
uniform float     specularPower : register(PC_MAT_SPECPOWER);
uniform float accumTime : register(PC_ACCUM_TIME);

//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------
Fragout main(v2f IN)
{
	Fragout OUT;

   float2 texOffset;
   float sinOffset1 = sin( accumTime * 1.5 + IN.TEX0.y * 6.28319 * 3.0 ) * 0.03;
   float sinOffset2 = sin( accumTime * 3.0 + IN.TEX0.y * 6.28319 ) * 0.04;
   
   texOffset.x = IN.TEX0.x + sinOffset1 + sinOffset2;
   texOffset.y = IN.TEX0.y + cos( accumTime * 3.0 + IN.TEX0.x * 6.28319 * 2.0 ) * 0.05;
   
   
   float4 bumpNorm = TORQUE_TEX2D( bumpMap, texOffset ) * 2.0 - 1.0;
   float4 diffuse = TORQUE_TEX2D( diffMap, texOffset );

   OUT.col = diffuse * (saturate( dot( IN.lightVec, bumpNorm.xyz ) ) + ambient);
   
   float3 eyeVec = normalize(IN.eyePos - IN.pixPos);
   float3 halfAng = normalize(eyeVec + IN.lightVec.xyz);
   float specular = saturate( dot(bumpNorm, halfAng) ) * IN.lightVec.w;
   specular = pow(abs(specular), specularPower);
   OUT.col += specularColor * specular;
   
	
   
   return OUT;
}

