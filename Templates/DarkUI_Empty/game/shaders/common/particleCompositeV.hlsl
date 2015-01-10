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

#include "hlslStructs.h"

struct VertOut
{
   float4 hpos : POSITION;
   float4 offscreenPos : TEXCOORD0;
   float4 backbufferPos : TEXCOORD1;
};

uniform float4 screenRect; // point, extent

VertOut main( float4 uvCoord : COLOR )
{
   VertOut OUT;
   
   OUT.hpos = float4(uvCoord.xy, 1.0, 1.0);
   OUT.hpos.xy *= screenRect.zw;
   OUT.hpos.xy += screenRect.xy;
   
   OUT.backbufferPos = OUT.hpos;
   OUT.offscreenPos = OUT.hpos;
   
   return OUT;
}

