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

//-----------------------------------------------------------------------------
// Structures                                                                  
//-----------------------------------------------------------------------------
#include "shaderModel.hlsl"

struct CloudVert
{
   float3 pos        : POSITION;
   float3 normal     : NORMAL;
   float3 binormal   : BINORMAL;
   float3 tangent    : TANGENT;
   float2 uv0        : TEXCOORD0;
};

struct ConnectData
{
   float4 hpos                : TORQUE_POSITION;   
   float4 texCoord12          : TEXCOORD0;
   float4 texCoord34          : TEXCOORD1;   
   float3 vLightTS            : TEXCOORD2;   // light vector in tangent space, denormalized
   float3 vViewTS             : TEXCOORD3;   // view vector in tangent space, denormalized
   float  worldDist           : TEXCOORD4;
};

//-----------------------------------------------------------------------------
// Uniforms                                                                        
//-----------------------------------------------------------------------------
uniform float4x4  modelview;
uniform float3    eyePosWorld;
uniform float3    sunVec;
uniform float2    texOffset0;
uniform float2    texOffset1;
uniform float2    texOffset2;
uniform float3    texScale;

//-----------------------------------------------------------------------------
// Main                                                                        
//-----------------------------------------------------------------------------
ConnectData main( CloudVert IN )
{   
   ConnectData OUT;

   OUT.hpos = mul(modelview, float4(IN.pos,1.0));
   OUT.hpos.w = OUT.hpos.z;
   // Offset the uv so we don't have a seam directly over our head.
   float2 uv = IN.uv0 + float2( 0.5, 0.5 );
   
   OUT.texCoord12.xy = uv * texScale.x;
   OUT.texCoord12.xy += texOffset0;
   
   OUT.texCoord12.zw = uv * texScale.y;
   OUT.texCoord12.zw += texOffset1;
   
   OUT.texCoord34.xy = uv * texScale.z;
   OUT.texCoord34.xy += texOffset2;
   
   OUT.texCoord34.z = IN.pos.z;
   OUT.texCoord34.w = 0.0;

   // Transform the normal, tangent and binormal vectors from object space to 
   // homogeneous projection space:
   float3 vNormalWS   = -IN.normal; 
   float3 vTangentWS  = -IN.tangent;
   float3 vBinormalWS = -IN.binormal;

   // Compute position in world space:
   float4 vPositionWS = float4(IN.pos, 1.0) + float4(eyePosWorld, 1); //mul( IN.pos, objTrans );

   // Compute and output the world view vector (unnormalized):
   float3 vViewWS = eyePosWorld - vPositionWS.xyz;

   // Compute denormalized light vector in world space:
   float3 vLightWS = -sunVec;

   // Normalize the light and view vectors and transform it to the tangent space:
   float3x3 mWorldToTangent = float3x3( vTangentWS, vBinormalWS, vNormalWS );

   // Propagate the view and the light vectors (in tangent space):
   OUT.vLightTS = mul( vLightWS, mWorldToTangent );
   OUT.vViewTS  = mul( mWorldToTangent, vViewWS  );
   
   OUT.worldDist = saturate( pow( max( IN.pos.z, 0 ), 2 ) );

   return OUT;
}
