//-----------------------------------------------------------------------------
// Copyright (c) 2014 R.G.S. - Richards Game Studio, the Netherlands
//					  http://www.richardsgamestudio.com/
//
// If you find this code useful or you are feeling particularly generous I
// would ask that you please go to http://www.richardsgamestudio.com/ then
// choose Donations from the menu on the left side and make a donation to
// Richards Game Studio. It will be highly appreciated.
//
// The MIT License:
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

// Volumetric Fog Glow postFx pixel shader V1.00

uniform sampler2D diffuseMap;
uniform float strength;

out vec4 OUT_col;

in vec2 uv0;
in vec2 uv1;
in vec2 uv2;
in vec2 uv3;

in vec2 uv4;
in vec2 uv5;
in vec2 uv6;
in vec2 uv7;

void main()
{
   vec4 kernel = vec4( 0.175, 0.275, 0.375, 0.475 ) * strength;

   OUT_col = vec4(0);
   OUT_col += texture( diffuseMap, uv0 ) * kernel.x;
   OUT_col += texture( diffuseMap, uv1 ) * kernel.y;
   OUT_col += texture( diffuseMap, uv2 ) * kernel.z;
   OUT_col += texture( diffuseMap, uv3 ) * kernel.w;

   OUT_col += texture( diffuseMap, uv4 ) * kernel.x;
   OUT_col += texture( diffuseMap, uv5 ) * kernel.y;
   OUT_col += texture( diffuseMap, uv6 ) * kernel.z;
   OUT_col += texture( diffuseMap, uv7 ) * kernel.w;

   // Calculate a lumenance value in the alpha so we
   // can use alpha test to save fillrate.
   vec3 rgb2lum = vec3( 0.30, 0.59, 0.11 );
   OUT_col.a = dot( OUT_col.rgb, rgb2lum );
}
