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

#include "../../../gl/hlslCompat.glsl"
#include "../../../gl/torque.glsl"

in vec4 vPosition;
in vec2 vTexCoord0;
in vec3 vTexCoord1;

#define IN_pos  vPosition
#define _IN_uv vTexCoord0
#define IN_wsEyeRay vTexCoord1

#define OUT_hpos gl_Position
out vec3 wsEyeRay;
#define OUT_wsEyeRay wsEyeRay
out vec2 uv0;
#define OUT_uv0 uv0
out vec2 uv1;
#define OUT_uv1 uv1
out vec2 uv2;
#define OUT_uv2 uv2
out vec2 uv3;
#define OUT_uv3 uv3
out vec2 uv4;
#define OUT_uv4 uv4
out vec2 uv5;
#define OUT_uv5 uv5
out vec2 uv6;
#define OUT_uv6 uv6
out vec2 uv7;
#define OUT_uv7 uv7

uniform vec2 texSize0;
uniform vec4 rtParams0;
uniform vec2 oneOverTargetSize; 


void main()
{
   OUT_hpos = IN_pos;
   
   vec2 IN_uv = viewportCoordToRenderTarget( _IN_uv, rtParams0 );
   
   // I don't know why this offset is necessary, but it is.
   //IN_uv = IN_uv * oneOverTargetSize;

   OUT_uv0 = IN_uv + ( ( BLUR_DIR * 3.5f ) / texSize0 );
   OUT_uv1 = IN_uv + ( ( BLUR_DIR * 2.5f ) / texSize0 );
   OUT_uv2 = IN_uv + ( ( BLUR_DIR * 1.5f ) / texSize0 );
   OUT_uv3 = IN_uv + ( ( BLUR_DIR * 0.5f ) / texSize0 );

   OUT_uv4 = IN_uv - ( ( BLUR_DIR * 3.5f ) / texSize0 );
   OUT_uv5 = IN_uv - ( ( BLUR_DIR * 2.5f ) / texSize0 );
   OUT_uv6 = IN_uv - ( ( BLUR_DIR * 1.5f ) / texSize0 );
   OUT_uv7 = IN_uv - ( ( BLUR_DIR * 0.5f ) / texSize0 );   
   
   /*
   OUT_uv0 = viewportCoordToRenderTarget( OUT_uv0, rtParams0 );
   OUT_uv1 = viewportCoordToRenderTarget( OUT_uv1, rtParams0 );
   OUT_uv2 = viewportCoordToRenderTarget( OUT_uv2, rtParams0 );
   OUT_uv3 = viewportCoordToRenderTarget( OUT_uv3, rtParams0 );
   
   OUT_uv4 = viewportCoordToRenderTarget( OUT_uv4, rtParams0 );
   OUT_uv5 = viewportCoordToRenderTarget( OUT_uv5, rtParams0 );
   OUT_uv6 = viewportCoordToRenderTarget( OUT_uv6, rtParams0 );
   OUT_uv7 = viewportCoordToRenderTarget( OUT_uv7, rtParams0 );
   */
   
   correctSSP(gl_Position);
}
