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

in vec4 vPosition;

#define IN_pos vPosition

out vec4 wsEyeDir;
out vec4 ssPos;
out vec4 vsEyeDir;

#define OUT_hpos gl_Position
#define OUT_wsEyeDir wsEyeDir
#define OUT_ssPos ssPos
#define OUT_vsEyeDir vsEyeDir

uniform mat4 modelview;
uniform mat4 objTrans;
uniform mat4 worldViewOnly;
uniform vec3 eyePosWorld;

void main()
{
   OUT_hpos = tMul( modelview, IN_pos );
   OUT_wsEyeDir = tMul( objTrans, IN_pos ) - vec4( eyePosWorld, 0.0 );
   OUT_vsEyeDir = tMul( worldViewOnly, IN_pos );
   OUT_ssPos = OUT_hpos;

   correctSSP(gl_Position);
}

