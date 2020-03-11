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

#include "shadergen:/autogenConditioners.h"

uniform sampler2D colorMapG  : register(S0);
uniform sampler2D deferredMap : register(S1);

uniform float3 lumaCoefficients;
uniform float threshold;
uniform float depthThreshold;


float4 main(   float2 texcoord : TEXCOORD0,
               float4 offset[2]: TEXCOORD1) : COLOR0 
{
   // Luma calculation requires gamma-corrected colors (texture 'colorMapG').
   //
   // Note that there is a lot of overlapped luma calculations; performance
   // can be improved if this luma calculation is performed in the main pass,
   // which may give you an edge if used in conjunction with a z deferred.

   float L = dot(tex2D(colorMapG, texcoord).rgb, lumaCoefficients);

   float Lleft = dot(tex2D(colorMapG, offset[0].xy).rgb, lumaCoefficients);
   float Ltop = dot(tex2D(colorMapG, offset[0].zw).rgb, lumaCoefficients);  
   float Lright = dot(tex2D(colorMapG, offset[1].xy).rgb, lumaCoefficients);
   float Lbottom = dot(tex2D(colorMapG, offset[1].zw).rgb, lumaCoefficients);

   float4 delta = abs(L.xxxx - float4(Lleft, Ltop, Lright, Lbottom));
   float4 edges = step(threshold, delta);

   // Add depth edges to color edges
   float D = deferredUncondition(deferredMap, texcoord).w;
   float Dleft = deferredUncondition(deferredMap, offset[0].xy).w;
   float Dtop  = deferredUncondition(deferredMap, offset[0].zw).w;
   float Dright = deferredUncondition(deferredMap, offset[1].xy).w;
   float Dbottom = deferredUncondition(deferredMap, offset[1].zw).w;

   delta = abs(D.xxxx - float4(Dleft, Dtop, Dright, Dbottom));
   edges += step(depthThreshold, delta);

   if (dot(edges, 1.0) == 0.0)
      discard;

   return edges;
}