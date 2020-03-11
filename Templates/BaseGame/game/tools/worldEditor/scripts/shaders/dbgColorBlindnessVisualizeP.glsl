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

//Using calculations and values provided by Alan Zucconi
// www.alanzucconi.com

#include "../../../../core/rendering/shaders/gl/hlslCompat.glsl"
#include "shadergen:/autogenConditioners.h"

in vec2 uv0;
uniform sampler2D backBufferTex;

in float mode;

out vec4 OUT_col;

void main()
{   
    vec4 imageColor = texture( backBufferTex, uv0 );  

    if(mode == 0)
    {
        OUT_col = imageColor;
    }
    else
    {
        vec3 R = imageColor.r;
        vec3 G = imageColor.g;
        vec3 B = imageColor.b;

        if(mode == 1) // Protanopia
        {
            R = vec3(0.56667, 0.43333, 0);
            G = vec3(0.55833, 0.44167, 0);
            B = vec3(0, 0.24167, 0.75833);
        }
        else if(mode == 2) // Protanomaly
        {
            R = vec3(0.81667, 0.18333, 0);
            G = vec3(0.33333, 0.66667, 0);
            B = vec3(0, 0.125, 0.875);
        }
        else if(mode == 3) // Deuteranopia
        {
            R = vec3(0.625, 0.375, 0);
            G = vec3(0.70, 0.30, 0);
            B = vec3(0, 0.30, 0.70);
        }
        else if(mode == 4) // Deuteranomaly
        {
            R = vec3(0.80, 0.20, 0);
            G = vec3(0.25833, 0.74167, 0);
            B = vec3(0, 0.14167, 0.85833);
        }
        else if(mode == 5) // Tritanopia
        {
            R = vec3(0.95, 0.05, 0);
            G = vec3(0, 0.43333, 0.56667);
            B = vec3(0, 0.475, 0.525);
        }
        else if(mode == 6) // Tritanomaly
        {
            R = vec3(0.96667, 0.03333, 0);
            G = vec3(0, 0.73333, 0.26667);
            B = vec3(0, 0.18333, 0.81667);
        }
        else if(mode == 7) // Achromatopsia
        {
            R = vec3(0.299, 0.587, 0.114);
            G = vec3(0.299, 0.587, 0.114);
            B = vec3(0.299, 0.587, 0.114);
        }
        else if(mode == 8) // Achromatomaly
        {
            R = vec3(0.618, 0.32, 0.062);
            G = vec3(0.163, 0.775, 0.062);
            B = vec3(0.163, 0.320, 0.516);
        }

        //First set
        vec4 c = imageColor;

        c.r = c.r * R[0] + c.g * R[1] + c.b * R[2];
        c.g = c.r * G[0] + c.g * G[1] + c.b * G[2];
        c.b = c.r * B[0] + c.g * B[1] + c.b * B[2];

        vec3 cb = c.rgb;

        cb.r = c.r * R[0] + c.g * R[1] + c.b * R[2];
        cb.g = c.r * G[0] + c.g * G[1] + c.b * G[2];
        cb.b = c.r * B[0] + c.g * B[1] + c.b * B[2];

        // Difference
        vec3 diff = abs(c.rgb - cb);

        float lum = c.r*.3 + c.g*.59 + c.b*.11;
        vec3 bw = vec3(lum, lum, lum);

       // return vec4(lerp(bw, vec3(1, 0, 0), saturate((diff.r + diff.g + diff.b) / 3)), c.a); 
       OUT_col = vec4(c.rgb,1);
    }
}
