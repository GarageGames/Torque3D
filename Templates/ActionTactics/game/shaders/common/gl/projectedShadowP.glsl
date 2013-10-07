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

varying vec2 texCoord;
varying vec4 color;
varying float fade;

uniform sampler2D inputTex;
uniform vec4 ambient;
            
            
void main()
{   
   vec3 LUMINANCE_VECTOR  = vec3(0.2125f, 0.4154f, 0.1721f);
   float esmFactor = 200.0;
   
   float lum = dot( ambient.rgb, LUMINANCE_VECTOR );   

   gl_FragColor.rgb = ambient.rgb * lum; 
   gl_FragColor.a = 0.0;
   float depth = texture2D(inputTex, texCoord).a;
   
   depth = depth * exp(depth - 10.0);
   depth = exp(esmFactor * depth) - 1.0;
   
   gl_FragColor.a = clamp(depth * 300.0, 0.0, 1.0) * (1.0 - lum) * fade * color.a;
}
