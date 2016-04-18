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
#include "hlslStructs.hlsl"

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

struct Conn
{
   float4 HPOS             : POSITION;
	float2 TEX0             : TEXCOORD0;
	float4 tangentToCube0   : TEXCOORD1;
	float4 tangentToCube1   : TEXCOORD2;
	float4 tangentToCube2   : TEXCOORD3;
   float4 outLightVec      : TEXCOORD4;
   float3 pos              : TEXCOORD5;
   float3 outEyePos        : TEXCOORD6;
   
};


uniform float4x4 modelview : register(VC_WORLD_PROJ);
uniform float3x3 cubeTrans : register(VC_CUBE_TRANS);
uniform float3   cubeEyePos : register(VC_CUBE_EYE_POS);
uniform float3   inLightVec : register(VC_LIGHT_DIR1);
uniform float3   eyePos : register(VC_EYE_POS);

//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------
Conn main( VertexIn_PNTTTB In)
{
   Conn Out;

   Out.HPOS = mul(modelview, float4(In.pos,1.0));
   Out.TEX0 = In.uv0;

   
	float3x3 objToTangentSpace;
	objToTangentSpace[0] = In.T;
	objToTangentSpace[1] = In.B;
	objToTangentSpace[2] = In.normal;
   
   
   Out.tangentToCube0.xyz = mul( objToTangentSpace, cubeTrans[0].xyz );
   Out.tangentToCube1.xyz = mul( objToTangentSpace, cubeTrans[1].xyz );
   Out.tangentToCube2.xyz = mul( objToTangentSpace, cubeTrans[2].xyz );
   
   float3 pos = mul( cubeTrans, In.pos ).xyz;
   float3 eye = cubeEyePos - pos;
   normalize( eye );

   Out.tangentToCube0.w = eye.x;
   Out.tangentToCube1.w = eye.y;
   Out.tangentToCube2.w = eye.z;

   Out.outLightVec.xyz = -inLightVec;
   Out.outLightVec.xyz = mul(objToTangentSpace, Out.outLightVec);
   Out.pos = mul(objToTangentSpace, In.pos.xyz / 100.0);
   Out.outEyePos.xyz = mul(objToTangentSpace, eyePos.xyz / 100.0);
   Out.outLightVec.w = step( 0.0, dot( -inLightVec, In.normal ) );
   
   
   return Out;
}


