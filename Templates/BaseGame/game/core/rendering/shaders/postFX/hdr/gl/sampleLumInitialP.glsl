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
#include "../../gl/postFX.glsl"

uniform sampler2D inputTex;
uniform vec2 texSize0;

uniform float g_fMinLuminace;

out vec4 OUT_col;

const vec2 gTapOffsets[9] = vec2[]
(
   vec2( -1.0, -1.0 ), vec2( 0.0, -1.0 ), vec2( 1.0, -1.0 ),
   vec2( -1.0, 0.0 ),  vec2( 0.0, 0.0 ),  vec2( 1.0, 0.0 ),
   vec2( -1.0, 1.0 ),  vec2( 0.0, 1.0 ),  vec2( 1.0, 1.0 )
);


void main()
{
   vec2 tsize = 1.0 / texSize0;

   vec3 _sample;
   float average = 0.0;
     
   for ( int i = 0; i < 9; i++ )
   {
      // Decode the hdr value.
      _sample = hdrDecode( texture( inputTex, IN_uv0 + ( gTapOffsets[i] * tsize ) ).rgb );

      // Get the luminance and add it to the average.
      float lum = max( hdrLuminance( _sample ), g_fMinLuminace );
      average += log( lum );
   }

   average = exp( average / 9.0 );

   OUT_col = vec4( average, 0.0, 0.0, 1.0 );
}
