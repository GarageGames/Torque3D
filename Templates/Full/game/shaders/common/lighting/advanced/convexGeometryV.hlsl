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

struct VertData
{
   float3 pos : POSITION;
   float4 color : COLOR;
};

struct ConvexConnectV
{
   float4 hpos : TORQUE_POSITION;
   float4 wsEyeDir : TEXCOORD0;
   float4 ssPos : TEXCOORD1;
   float4 vsEyeDir : TEXCOORD2;
};

ConvexConnectV main( VertData IN,
                     uniform float4x4 modelview,
                     uniform float4x4 objTrans,
                     uniform float4x4 worldViewOnly,
                     uniform float3 eyePosWorld )
{
   ConvexConnectV OUT;

   OUT.hpos = mul( modelview, float4(IN.pos,1.0) );
   OUT.wsEyeDir = mul(objTrans, float4(IN.pos, 1.0)) - float4(eyePosWorld, 0.0);
   OUT.vsEyeDir = mul(worldViewOnly, float4(IN.pos, 1.0));
   OUT.ssPos = OUT.hpos;

   return OUT;
}
