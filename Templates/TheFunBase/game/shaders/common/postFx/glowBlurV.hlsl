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

#include "./postFx.hlsl"
#include "./../torque.hlsl"


uniform float2 texSize0;

struct VertToPix
{
   float4 hpos       : POSITION;

   float2 uv0        : TEXCOORD0;
   float2 uv1        : TEXCOORD1;
   float2 uv2        : TEXCOORD2;
   float2 uv3        : TEXCOORD3;

   float2 uv4        : TEXCOORD4;
   float2 uv5        : TEXCOORD5;
   float2 uv6        : TEXCOORD6;
   float2 uv7        : TEXCOORD7;
};

VertToPix main( PFXVert IN )
{
   VertToPix OUT;
   
   OUT.hpos = IN.pos;
   
   float2 uv = IN.uv + (0.5f / texSize0);

   OUT.uv0 = uv + ( ( BLUR_DIR * 3.5f ) / texSize0 );
   OUT.uv1 = uv + ( ( BLUR_DIR * 2.5f ) / texSize0 );
   OUT.uv2 = uv + ( ( BLUR_DIR * 1.5f ) / texSize0 );
   OUT.uv3 = uv + ( ( BLUR_DIR * 0.5f ) / texSize0 );

   OUT.uv4 = uv - ( ( BLUR_DIR * 3.5f ) / texSize0 );
   OUT.uv5 = uv - ( ( BLUR_DIR * 2.5f ) / texSize0 );
   OUT.uv6 = uv - ( ( BLUR_DIR * 1.5f ) / texSize0 );
   OUT.uv7 = uv - ( ( BLUR_DIR * 0.5f ) / texSize0 );
   
   return OUT;
}
