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

#include "../postFx.hlsl"
#include "shadergen:/autogenConditioners.h"

float4 main( PFXVertToPix IN, 
             uniform sampler2D edgeBuffer : register(S0),
             uniform sampler2D backBuffer : register(S1),
             uniform float2 targetSize : register(C0) ) : COLOR0
{
   float2 pixelSize = 1.0 / targetSize;

   // Sample edge buffer, bail if not on an edge
   float edgeSample = tex2D(edgeBuffer, IN.uv0).r;
   clip(edgeSample - 1e-6);
   
   // Ok we're on an edge, so multi-tap sample, average, and return
   float2 offsets[9] = {
      float2( 0.0,  0.0),
      float2(-1.0, -1.0),
      float2( 0.0, -1.0),
      float2( 1.0, -1.0),
      float2( 1.0,  0.0),
      float2( 1.0,  1.0),
      float2( 0.0,  1.0),
      float2(-1.0,  1.0),
      float2(-1.0,  0.0),
   };
      
   float4 accumColor = 0;
   for(int i = 0; i < 9; i++)
   {
      // Multiply the intensity of the edge, by the UV, so that things which maybe
      // aren't quite full edges get sub-pixel sampling to reduce artifacts
      
      // Scaling offsets by 0.5 to reduce the range bluriness from extending to
      // far outward from the edge.
      
      float2 offsetUV = IN.uv1 + edgeSample * ( offsets[i] * 0.5 ) * pixelSize;//rtWidthHeightInvWidthNegHeight.zw;
      //offsetUV *= 0.999;
      accumColor+= tex2D(backBuffer, offsetUV);
   }
   accumColor /= 9.0;
   
   return accumColor;
}
