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

uniform sampler2D diffMap;
uniform sampler2D bumpMap;
uniform samplerCube cubeMap;
uniform vec4 specularColor;
uniform float specularPower;
uniform vec4 ambient;
uniform float accumTime;

in vec2 TEX0;
in vec4 outLightVec;
in vec3 outPos;
in vec3 outEyePos;

out vec4 OUT_col;

void main()
{
   vec2 texOffset;
   float sinOffset1 = sin( accumTime * 1.5 + TEX0.y * 6.28319 * 3.0 ) * 0.03;
   float sinOffset2 = sin( accumTime * 3.0 + TEX0.y * 6.28319 ) * 0.04;
   
   texOffset.x = TEX0.x + sinOffset1 + sinOffset2;
   texOffset.y = TEX0.y + cos( accumTime * 3.0 + TEX0.x * 6.28319 * 2.0 ) * 0.05;
   
   vec4 bumpNorm = texture(bumpMap, texOffset) * 2.0 - 1.0;
   vec4 diffuse = texture(diffMap, texOffset);
   
   OUT_col = diffuse * (clamp(dot(outLightVec.xyz, bumpNorm.xyz), 0.0, 1.0) + ambient);
   
   vec3 eyeVec = normalize(outEyePos - outPos);
   vec3 halfAng = normalize(eyeVec + outLightVec.xyz);
   float specular = clamp(dot(bumpNorm.xyz, halfAng), 0.0, 1.0) * outLightVec.w;
   specular = pow(specular, specularPower);
   OUT_col += specularColor * specular;
}
