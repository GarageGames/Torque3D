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


#include "../../../gl/torque.glsl"
#include "../../../gl/hlslCompat.glsl"

in vec4 vPosition;
in vec2 vTexCoord0;

#define IN_pos  vPosition
#define _IN_uv  vTexCoord0

uniform vec2 texSize0;
uniform vec4 rtParams0;
uniform vec2 oneOverTargetSize; 

#define OUT_hpos gl_Position

out vec4 uv0;
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


void main()
{
   OUT_hpos = IN_pos;
   
   vec2 IN_uv = viewportCoordToRenderTarget( _IN_uv, rtParams0 );
   
   //vec4 step = vec4( 3.5, 2.5, 1.5, 0.5 );
   //vec4 step = vec4( 4.0, 3.0, 2.0, 1.0 );
   vec4 step = vec4( 9.0, 5.0, 2.5, 0.5 );
   
   // I don't know why this offset is necessary, but it is.
   //IN_uv = IN_uv * oneOverTargetSize;

   OUT_uv0.xy = IN_uv + ( ( BLUR_DIR * step.x ) / texSize0 );
   OUT_uv1 = IN_uv + ( ( BLUR_DIR * step.y ) / texSize0 );
   OUT_uv2 = IN_uv + ( ( BLUR_DIR * step.z ) / texSize0 );
   OUT_uv3 = IN_uv + ( ( BLUR_DIR * step.w ) / texSize0 );

   OUT_uv4 = IN_uv - ( ( BLUR_DIR * step.x ) / texSize0 );
   OUT_uv5 = IN_uv - ( ( BLUR_DIR * step.y ) / texSize0 );
   OUT_uv6 = IN_uv - ( ( BLUR_DIR * step.z ) / texSize0 );
   OUT_uv7 = IN_uv - ( ( BLUR_DIR * step.w ) / texSize0 );   
   
   OUT_uv0.zw = IN_uv;
   
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
