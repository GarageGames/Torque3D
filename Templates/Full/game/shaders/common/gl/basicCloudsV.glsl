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

//CloudVert
in vec4 vPosition;
in vec2 vTexCoord0;

#define IN_pos       vPosition
#define IN_uv0       vTexCoord0

uniform mat4  modelview;
uniform float     accumTime;
uniform float     texScale;
uniform vec2    texDirection;
uniform vec2    texOffset;

out vec2 texCoord;
#define OUT_texCoord texCoord

void main()
{  
   gl_Position = tMul(modelview, IN_pos);
   
   vec2 uv = IN_uv0;
   uv += texOffset;
   uv *= texScale;
   uv += accumTime * texDirection;

   OUT_texCoord = uv;   
   
   correctSSP(gl_Position);
}