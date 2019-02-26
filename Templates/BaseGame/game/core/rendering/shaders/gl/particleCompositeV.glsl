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

in vec2  vTexCoord0;
#define uvCoord vTexCoord0

out vec4 offscreenPos;
out vec4 backbufferPos;

#define OUT_hpos gl_Position
#define OUT_offscreenPos offscreenPos
#define OUT_backbufferPos backbufferPos

uniform vec4 screenRect; // point, extent

void main()
{
   OUT_hpos = vec4(uvCoord.xy, 1.0, 1.0);
   OUT_hpos.xy *= screenRect.zw;
   OUT_hpos.xy += screenRect.xy;
   
   OUT_backbufferPos = OUT_hpos;
   OUT_offscreenPos = OUT_hpos;
   
   correctSSP(gl_Position);
}

