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

#include "../../gl/hlslCompat.glsl"
#include "../../gl/torque.glsl"

in vec4 vPosition;
in vec2 vTexCoord0;

uniform vec2 texSize0;

out vec4 hpos; //POSITION;
out vec2 uv0; //TEXCOORD0;
out vec2 uv1; //TEXCOORD1;
out vec2 uv2; //TEXCOORD2;
out vec2 uv3; //TEXCOORD3;
out vec2 uv4; //TEXCOORD4;
out vec2 uv5; //TEXCOORD5;
out vec2 uv6; //TEXCOORD6;
out vec2 uv7; //TEXCOORD7;

void main()
{  
   gl_Position = vPosition;
   hpos = gl_Position;
   
   vec2 uv = vTexCoord0 + (0.5f / texSize0);

   uv0 = uv + ( ( BLUR_DIR * 3.5f ) / texSize0 );
   uv1 = uv + ( ( BLUR_DIR * 2.5f ) / texSize0 );
   uv2 = uv + ( ( BLUR_DIR * 1.5f ) / texSize0 );
   uv3 = uv + ( ( BLUR_DIR * 0.5f ) / texSize0 );

   uv4 = uv - ( ( BLUR_DIR * 3.5f ) / texSize0 );
   uv5 = uv - ( ( BLUR_DIR * 2.5f ) / texSize0 );
   uv6 = uv - ( ( BLUR_DIR * 1.5f ) / texSize0 );
   uv7 = uv - ( ( BLUR_DIR * 0.5f ) / texSize0 );
   
   correctSSP(gl_Position);
}
