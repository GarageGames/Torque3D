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


uniform vec2 texSize0;

#if !defined(PIXEL_SIZE)
#define PIXEL_SIZE (1.0 / texSize0)
#define MAX_SEARCH_STEPS 8    
#define MAX_DISTANCE 33
#endif

// Typical Multiply-Add operation to ease translation to assembly code.

vec4 mad(vec4 m, vec4 a, vec4 b) 
{
   #if defined(XBOX)
      vec4 result;
      asm {
         mad result, m, a, b
      };
      return result;
   #else
      return m * a + b;
   #endif
}


// This one just returns the first level of a mip map chain, which allow us to
// avoid the nasty ddx/ddy warnings, even improving the performance a little 
// bit.
vec4 tex2Dlevel0(sampler2D map, vec2 texcoord) 
{
   return tex2Dlod(map, vec4(texcoord, 0.0, 0.0));
}


// Same as above, this eases translation to assembly code;
vec4 tex2Doffset(sampler2D map, vec2 texcoord, vec2 offset) 
{
   #if defined(XBOX) && MAX_SEARCH_STEPS < 6
      vec4 result;
      float x = offset.x;
      float y = offset.y;
      asm {
         tfetch2D result, texcoord, map, OffsetX = x, OffsetY = y
      };
      return result;
   #else
      return tex2Dlevel0(map, texcoord + PIXEL_SIZE * offset);
   #endif
}


// Ok, we have the distance and both crossing edges, can you please return 
// the vec2 blending weights?
vec2 Area(vec2 distance, float e1, float e2) 
{
   // * By dividing by areaSize - 1.0 below we are implicitely offsetting to
   //   always fall inside of a pixel
   // * Rounding prevents bilinear access precision problems
   float areaSize = MAX_DISTANCE * 5.0;
   vec2 pixcoord = MAX_DISTANCE * round(4.0 * vec2(e1, e2)) + distance;
   vec2 texcoord = pixcoord / (areaSize - 1.0);
   return tex2Dlevel0(areaMap, texcoord).rg;
}


// Search functions for the 2nd pass.
float SearchXLeft(vec2 texcoord) 
{
   // We compare with 0.9 to prevent bilinear access precision problems.
   float i;
   float e = 0.0;
   for (i = -1.5; i > -2.0 * MAX_SEARCH_STEPS; i -= 2.0) 
   {
      e = tex2Doffset(edgesMapL, texcoord, vec2(i, 0.0)).g;
      /*[flatten]*/ if (e < 0.9) break;
   }
   return max(i + 1.5 - 2.0 * e, -2.0 * MAX_SEARCH_STEPS);
}

// Search functions for the 2nd pass.
float SearchXRight(vec2 texcoord) 
{
   float i;
   float e = 0.0;
   for (i = 1.5; i < 2.0 * MAX_SEARCH_STEPS; i += 2.0) 
   {
      e = tex2Doffset(edgesMapL, texcoord, vec2(i, 0.0)).g;
      /*[flatten]*/ if (e < 0.9) break;
   }
   return min(i - 1.5 + 2.0 * e, 2.0 * MAX_SEARCH_STEPS);
}

// Search functions for the 2nd pass.
float SearchYUp(vec2 texcoord) 
{
   float i;
   float e = 0.0;
   for (i = -1.5; i > -2.0 * MAX_SEARCH_STEPS; i -= 2.0) 
   {
      e = tex2Doffset(edgesMapL, texcoord, vec2(i, 0.0).yx).r;
      /*[flatten]*/ if (e < 0.9) break;
   }
   return max(i + 1.5 - 2.0 * e, -2.0 * MAX_SEARCH_STEPS);
}

// Search functions for the 2nd pass.
float SearchYDown(vec2 texcoord)
{
   float i;
   float e = 0.0;
   for (i = 1.5; i < 2.0 * MAX_SEARCH_STEPS; i += 2.0) 
   {
      e = tex2Doffset(edgesMapL, texcoord, vec2(i, 0.0).yx).r;
      /*[flatten]*/ if (e < 0.9) break;
   }
   return min(i - 1.5 + 2.0 * e, 2.0 * MAX_SEARCH_STEPS);
}
