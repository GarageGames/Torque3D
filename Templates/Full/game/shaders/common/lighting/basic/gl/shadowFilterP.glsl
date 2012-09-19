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

uniform sampler2D diffuseMap;

varying vec2 uv;

uniform vec2 oneOverTargetSize;

void main()
{
   vec2 sNonUniformTaps[8];
      
   sNonUniformTaps[0] = vec2(0.992833, 0.979309);
   sNonUniformTaps[1] = vec2(-0.998585, 0.985853);
   sNonUniformTaps[2] = vec2(0.949299, -0.882562);
   sNonUniformTaps[3] = vec2(-0.941358, -0.893924);
   sNonUniformTaps[4] = vec2(0.545055, -0.589072);
   sNonUniformTaps[5] = vec2(0.346526, 0.385821);
   sNonUniformTaps[6] = vec2(-0.260183, 0.334412);
   sNonUniformTaps[7] = vec2(0.248676, -0.679605);
   
   gl_FragColor = vec4(0.0);
   
   vec2 texScale = vec2(1.0);
   
   for ( int i=0; i < 4; i++ )
   {
      vec2 offset = (oneOverTargetSize * texScale) * sNonUniformTaps[i];
      gl_FragColor += texture2D( diffuseMap, uv + offset );
   }
   
   gl_FragColor /= vec4(4.0);
   gl_FragColor.rgb = vec3(0.0);
}
