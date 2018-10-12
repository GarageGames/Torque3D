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

in vec2 texcoord;
in vec4 offset[2];

uniform sampler2D blendMap;
uniform sampler2D colorMapL;
uniform sampler2D colorMap;

// Dummy sampers to please include.
uniform sampler2D areaMap;
uniform sampler2D edgesMapL;
#include "./functions.glsl"

out vec4 OUT_col;

void main()
{
   // Fetch the blending weights for current pixel:
   vec4 topLeft = texture(blendMap, texcoord);
   float bottom = texture(blendMap, offset[1].zw).g;
   float right = texture(blendMap, offset[1].xy).a;
   vec4 a = vec4(topLeft.r, bottom, topLeft.b, right);

   // Up to 4 lines can be crossing a pixel (one in each edge). So, we perform
   // a weighted average, where the weight of each line is 'a' cubed, which
   // favors blending and works well in practice.
   vec4 w = a * a * a;

   // There is some blending weight with a value greater than 0.0?
   float sum = dot(w, vec4(1.0));
   if (sum < 1e-5)
      discard;

   vec4 color = vec4(0.0);

   // Add the contributions of the possible 4 lines that can cross this pixel:
   #ifdef BILINEAR_FILTER_TRICK
      vec4 coords = mad(vec4( 0.0, -a.r, 0.0,  a.g), PIXEL_SIZE.yyyy, texcoord.xyxy);
      color = mad(texture(colorMapL, coords.xy), vec4(w.r), color);
      color = mad(texture(colorMapL, coords.zw), vec4(w.g), color);

      coords = mad(vec4(-a.b,  0.0, a.a,  0.0), PIXEL_SIZE.xxxx, texcoord.xyxy);
      color = mad(texture(colorMapL, coords.xy), vec4(w.b), color);
      color = mad(texture(colorMapL, coords.zw), vec4(w.a), color);
   #else
      vec4 C = texture(colorMap, texcoord);
      vec4 Cleft = texture(colorMap, offset[0].xy);
      vec4 Ctop = texture(colorMap, offset[0].zw);
      vec4 Cright = texture(colorMap, offset[1].xy);
      vec4 Cbottom = texture(colorMap, offset[1].zw);
      color = mad(mix(C, Ctop, a.r), vec4(w.r), color);
      color = mad(mix(C, Cbottom, a.g), vec4(w.g), color);
      color = mad(mix(C, Cleft, a.b), vec4(w.b), color);
      color = mad(mix(C, Cright, a.a), vec4(w.a), color);
   #endif

   // Normalize the resulting color and we are finished!
   OUT_col = color / sum; 
}