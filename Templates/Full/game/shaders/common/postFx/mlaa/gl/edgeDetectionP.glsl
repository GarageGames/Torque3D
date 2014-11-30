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

// An implementation of "Practical Morphological Anti-Aliasing" from 
// GPU Pro 2 by Jorge Jimenez, Belen Masia, Jose I. Echevarria, 
// Fernando Navarro, and Diego Gutierrez.
//
// http://www.iryoku.com/mlaa/

#include "../../../gl/hlslCompat.glsl"
#include "shadergen:/autogenConditioners.h"

uniform sampler2D colorMapG;
uniform sampler2D prepassMap;

uniform vec3 lumaCoefficients;
uniform float threshold;
uniform float depthThreshold;

in vec2 texcoord;
in vec4 offset[2];

out vec4 OUT_col;

void main()
{
   // Luma calculation requires gamma-corrected colors (texture 'colorMapG').
   //
   // Note that there is a lot of overlapped luma calculations; performance
   // can be improved if this luma calculation is performed in the main pass,
   // which may give you an edge if used in conjunction with a z prepass.

   float L = dot(texture(colorMapG, texcoord).rgb, lumaCoefficients);

   float Lleft = dot(texture(colorMapG, offset[0].xy).rgb, lumaCoefficients);
   float Ltop = dot(texture(colorMapG, offset[0].zw).rgb, lumaCoefficients);  
   float Lright = dot(texture(colorMapG, offset[1].xy).rgb, lumaCoefficients);
   float Lbottom = dot(texture(colorMapG, offset[1].zw).rgb, lumaCoefficients);

   vec4 delta = abs(vec4(L) - vec4(Lleft, Ltop, Lright, Lbottom));
   vec4 edges = step(threshold, delta);

   // Add depth edges to color edges
   float D = prepassUncondition(prepassMap, texcoord).w;
   float Dleft = prepassUncondition(prepassMap, offset[0].xy).w;
   float Dtop  = prepassUncondition(prepassMap, offset[0].zw).w;
   float Dright = prepassUncondition(prepassMap, offset[1].xy).w;
   float Dbottom = prepassUncondition(prepassMap, offset[1].zw).w;

   delta = abs(vec4(D) - vec4(Dleft, Dtop, Dright, Dbottom));
   edges += step(depthThreshold, delta);

   if (dot(edges, vec4(1.0)) == 0.0)
      discard;

   OUT_col = edges;
}