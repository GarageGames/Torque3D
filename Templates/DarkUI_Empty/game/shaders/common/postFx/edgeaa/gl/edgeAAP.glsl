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
#include "shadergen:/autogenConditioners.h"
#include "../../gl/postFX.glsl"

uniform sampler2D edgeBuffer;
uniform sampler2D backBuffer;
uniform vec2 targetSize;

out vec4 OUT_col;

void main()
{
   vec2 pixelSize = 1.0 / targetSize;

   // Sample edge buffer, bail if not on an edge
   float edgeSample = texture(edgeBuffer, IN_uv0).r;
   clip(edgeSample - 1e-6);
   
   // Ok we're on an edge, so multi-tap sample, average, and return
   vec2 offsets[9] = vec2[](
      vec2( 0.0,  0.0),
      vec2(-1.0, -1.0),
      vec2( 0.0, -1.0),
      vec2( 1.0, -1.0),
      vec2( 1.0,  0.0),
      vec2( 1.0,  1.0),
      vec2( 0.0,  1.0),
      vec2(-1.0,  1.0),
      vec2(-1.0,  0.0)
   );
      
   vec4 accumColor = vec4(0.0);
   for(int i = 0; i < 9; i++)
   {
      // Multiply the intensity of the edge, by the UV, so that things which maybe
      // aren't quite full edges get sub-pixel sampling to reduce artifacts
      
      // Scaling offsets by 0.5 to reduce the range bluriness from extending to
      // far outward from the edge.
      
      vec2 offsetUV = IN_uv1 + edgeSample * ( offsets[i] * 0.5 ) * pixelSize;//rtWidthHeightInvWidthNegHeight.zw;
      //offsetUV *= 0.999;
      accumColor+= texture(backBuffer, offsetUV);
   }
   accumColor /= 9.0;
   
   OUT_col = accumColor;
}
