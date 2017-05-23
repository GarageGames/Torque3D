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

#include "./../postFx.hlsl"
#include "./../../torque.hlsl"


uniform float4 rtParams0;
uniform float2 texSize0;
uniform float2 oneOverTargetSize; 

struct VertToPix
{
   float4 hpos       : TORQUE_POSITION;

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
   
   OUT.hpos = float4(IN.pos,1.0);
   
   IN.uv = viewportCoordToRenderTarget( IN.uv, rtParams0 );
   
   // I don't know why this offset is necessary, but it is.
   //IN.uv = IN.uv * oneOverTargetSize;

   OUT.uv0 = IN.uv + ( ( BLUR_DIR * 3.5f ) / texSize0 );
   OUT.uv1 = IN.uv + ( ( BLUR_DIR * 2.5f ) / texSize0 );
   OUT.uv2 = IN.uv + ( ( BLUR_DIR * 1.5f ) / texSize0 );
   OUT.uv3 = IN.uv + ( ( BLUR_DIR * 0.5f ) / texSize0 );

   OUT.uv4 = IN.uv - ( ( BLUR_DIR * 3.5f ) / texSize0 );
   OUT.uv5 = IN.uv - ( ( BLUR_DIR * 2.5f ) / texSize0 );
   OUT.uv6 = IN.uv - ( ( BLUR_DIR * 1.5f ) / texSize0 );
   OUT.uv7 = IN.uv - ( ( BLUR_DIR * 0.5f ) / texSize0 );   
   
   /*
   OUT.uv0 = viewportCoordToRenderTarget( OUT.uv0, rtParams0 );
   OUT.uv1 = viewportCoordToRenderTarget( OUT.uv1, rtParams0 );
   OUT.uv2 = viewportCoordToRenderTarget( OUT.uv2, rtParams0 );
   OUT.uv3 = viewportCoordToRenderTarget( OUT.uv3, rtParams0 );
   
   OUT.uv4 = viewportCoordToRenderTarget( OUT.uv4, rtParams0 );
   OUT.uv5 = viewportCoordToRenderTarget( OUT.uv5, rtParams0 );
   OUT.uv6 = viewportCoordToRenderTarget( OUT.uv6, rtParams0 );
   OUT.uv7 = viewportCoordToRenderTarget( OUT.uv7, rtParams0 );
   */
   
   return OUT;
}
