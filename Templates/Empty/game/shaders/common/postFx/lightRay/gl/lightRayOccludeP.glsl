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
#include "shadergen:/autogenConditioners.h"
#include "../../gl/postFX.glsl"

uniform sampler2D backBuffer;   // The original backbuffer.
uniform sampler2D prepassTex;   // The pre-pass depth and normals.

uniform float brightScalar;

const vec3 LUMINANCE_VECTOR = vec3(0.3125f, 0.6154f, 0.0721f);

out vec4 OUT_col;

void main()
{
    vec4 col = vec4( 0, 0, 0, 1 );
    
    // Get the depth at this pixel.
    float depth = prepassUncondition( prepassTex, IN_uv0 ).w;
    
    // If the depth is equal to 1.0, read from the backbuffer
    // and perform the exposure calculation on the result.
    if ( depth >= 0.999 )
    {
        col = texture( backBuffer, IN_uv0 );

        //col = 1 - exp(-120000 * col);
        col += dot( vec3(col), LUMINANCE_VECTOR ) + 0.0001f;
        col *= brightScalar;
    }
    
    OUT_col = col;
}
