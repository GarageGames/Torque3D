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

struct Vertex
{
   float3 pos : POSITION;
   float4 color : COLOR0;
   float2 uv0 : TEXCOORD0;
};

struct Conn
{
   float4 hpos : TORQUE_POSITION;
   float4 color : TEXCOORD0;
   float2 uv0 : TEXCOORD1;
	float4 pos : TEXCOORD2;
};


uniform float4x4 modelViewProj;
uniform float4x4 fsModelViewProj;

Conn main( Vertex In )
{
   Conn Out;

   Out.hpos = mul( modelViewProj, float4(In.pos,1.0) );
   Out.pos = mul(fsModelViewProj, float4(In.pos, 1.0) );
	Out.color = In.color;
	Out.uv0 = In.uv0;
	
   return Out;
}

