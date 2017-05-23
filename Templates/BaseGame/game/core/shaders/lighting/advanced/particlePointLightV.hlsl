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

#include "../../hlslStructs.hlsl"
#include "../../shaderModel.hlsl"

struct ConvexConnectV
{
   float4 hpos : TORQUE_POSITION;
   float4 ssPos : TEXCOORD0;
   float3 vsEyeDir : TEXCOORD1;
};

uniform float4x4 viewProj;
uniform float4x4 view;
uniform float3 particlePosWorld;
uniform float  lightRange;

ConvexConnectV main( VertexIn_P IN  )
{
   ConvexConnectV OUT;
   float4 pos = float4(IN.pos, 0.0);
   float4 vPosWorld = pos + float4(particlePosWorld, 0.0) + pos * lightRange;
   OUT.hpos = mul(viewProj, vPosWorld);
   OUT.vsEyeDir = mul(view, vPosWorld);
   OUT.ssPos = OUT.hpos;

   return OUT;
}
