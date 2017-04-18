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

#ifdef TORQUE_VERTEX_SHADER

in vec4 vPosition;
in vec2 vTexCoord0;
in vec3 vTexCoord1;

#define IN_pos       vPosition
#define IN_uv        vTexCoord0.st
#define IN_wsEyeRay  vTexCoord1.xyz

out vec2 uv0;
out vec2 uv1;
out vec2 uv2;
out vec2 uv3;
out vec3 wsEyeRay;

#define OUT_uv0 uv0
#define OUT_uv1 uv1
#define OUT_uv2 uv2
#define OUT_uv3 uv3
#define OUT_wsEyeRay wsEyeRay
#define OUT_hpos gl_Position

#endif //TORQUE_VERTEX_SHADER


#ifdef TORQUE_PIXEL_SHADER

in vec2 uv0;
in vec2 uv1;
in vec2 uv2;
in vec2 uv3;
in vec3 wsEyeRay;

#define IN_uv0 uv0
#define IN_uv1 uv1
#define IN_uv2 uv2
#define IN_uv3 uv3
#define IN_wsEyeRay wsEyeRay

#endif//TORQUE_PIXEL_SHADER