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

/// The vertex shader used in the generation and caching of the
/// base terrain texture.

struct VertData
{
   float3 position : POSITION;
   float2 texCoord : TEXCOORD0;
};

struct ConnectData
{
   float4 hpos : POSITION;
   float2 layerCoord : TEXCOORD0;
   float2 texCoord : TEXCOORD1;
};

ConnectData main( VertData IN,
                  uniform float2 texScale : register(C0) )
{
   ConnectData OUT;

   OUT.hpos = float4( IN.position.xyz, 1 );
   OUT.layerCoord = IN.texCoord;
   OUT.texCoord = IN.texCoord * texScale;

   return OUT;
}
