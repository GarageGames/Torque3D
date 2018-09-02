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

#include "hlslCompat.glsl"

in vec4 vPosition;
in vec4 vColor;
in vec2 vTexCoord0;

#define In_pos    vPosition
#define In_color  vColor
#define In_uv0    vTexCoord0

out vec4 color;
out vec2 uv0;
out vec4 pos;

#define OUT_hpos gl_Position
#define OUT_color color
#define OUT_uv0 uv0
#define OUT_pos pos

uniform mat4 modelViewProj;
uniform mat4 fsModelViewProj;

void main()
{
   OUT_hpos = tMul( modelViewProj, In_pos );
	OUT_pos = tMul( fsModelViewProj, In_pos );
	OUT_color = In_color;
	OUT_uv0 = In_uv0;
	
   correctSSP(gl_Position);
}

