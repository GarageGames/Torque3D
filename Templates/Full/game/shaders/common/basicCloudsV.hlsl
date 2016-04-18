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

#include "shaderModel.hlsl"

struct CloudVert
{
   float3 pos        : POSITION;
   float2 uv0        : TEXCOORD0;
};

struct ConnectData
{
   float4 hpos : TORQUE_POSITION;
   float2 texCoord : TEXCOORD0;
};

uniform float4x4  modelview;
uniform float2    texDirection;
uniform float2    texOffset;
uniform float     accumTime;
uniform float     texScale;


ConnectData main( CloudVert IN )
{
   ConnectData OUT;

   OUT.hpos = mul(modelview, float4(IN.pos,1.0));
   OUT.hpos.w = OUT.hpos.z;

   float2 uv = IN.uv0;
   uv += texOffset;
   uv *= texScale;
   uv += accumTime * texDirection;

   OUT.texCoord = uv;

   return OUT;
}