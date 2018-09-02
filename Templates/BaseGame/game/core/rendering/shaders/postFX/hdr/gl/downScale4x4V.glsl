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

#define IN_GLSL
#include "../../../shdrConsts.h"
#include "../../../gl/hlslCompat.glsl"

in vec4 vPosition;
in vec2 vTexCoord0;

#define In_pos vPosition
#define In_uv vTexCoord0

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------
out vec4 texCoords[8];
#define Out_texCoords texCoords

#define Out_hpos gl_Position

uniform vec2 targetSize;

//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------
void main()
{
	Out_hpos = In_pos;
   
   // Sample from the 16 surrounding points. Since the center point will be in
   // the exact center of 16 texels, a 0.5f offset is needed to specify a texel
   // center.
   vec2 texSize = vec2( 1.0 / (targetSize.x - 1.0), 1.0 / (targetSize.y - 1.0) );
   
   vec4 uv;
	uv.xy = In_uv.xy;
	uv.zw = In_uv.xy;
   
   Out_texCoords[0] = uv;
   Out_texCoords[0].x += texSize.x;
   Out_texCoords[0].y += texSize.y;
   Out_texCoords[0].z += texSize.x;
   Out_texCoords[0].w += texSize.y;
   Out_texCoords[0].x += ( 0 - 1.5 ) * texSize.x;
   Out_texCoords[0].y += ( 0 - 1.5 ) * texSize.y;
   Out_texCoords[0].z += ( 1 - 1.5 ) * texSize.x;
   Out_texCoords[0].w += ( 0 - 1.5 ) * texSize.y;
   
   Out_texCoords[1] = uv;
   Out_texCoords[1].x += texSize.x;
   Out_texCoords[1].y += texSize.y;
   Out_texCoords[1].z += texSize.x;
   Out_texCoords[1].w += texSize.y;
   Out_texCoords[1].x += ( 2 - 1.5 ) * texSize.x;
   Out_texCoords[1].y += ( 0 - 1.5 ) * texSize.y;
   Out_texCoords[1].z += ( 3 - 1.5 ) * texSize.x;
   Out_texCoords[1].w += ( 0 - 1.5 ) * texSize.y;
   
   Out_texCoords[2] = uv;
   Out_texCoords[2].x += texSize.x;
   Out_texCoords[2].y += texSize.y;
   Out_texCoords[2].z += texSize.x;
   Out_texCoords[2].w += texSize.y;
   Out_texCoords[2].x += ( 0 - 1.5 ) * texSize.x;
   Out_texCoords[2].y += ( 1 - 1.5 ) * texSize.y;
   Out_texCoords[2].z += ( 1 - 1.5 ) * texSize.x;
   Out_texCoords[2].w += ( 1 - 1.5 ) * texSize.y;
   
   Out_texCoords[3] = uv;
   Out_texCoords[3].x += texSize.x;
   Out_texCoords[3].y += texSize.y;
   Out_texCoords[3].z += texSize.x;
   Out_texCoords[3].w += texSize.y;
   Out_texCoords[3].x += ( 2 - 1.5 ) * texSize.x;
   Out_texCoords[3].y += ( 1 - 1.5 ) * texSize.y;
   Out_texCoords[3].z += ( 3 - 1.5 ) * texSize.x;
   Out_texCoords[3].w += ( 1 - 1.5 ) * texSize.y;
   
   Out_texCoords[4] = uv;
   Out_texCoords[4].x += texSize.x;
   Out_texCoords[4].y += texSize.y;
   Out_texCoords[4].z += texSize.x;
   Out_texCoords[4].w += texSize.y;
   Out_texCoords[4].x += ( 0 - 1.5 ) * texSize.x;
   Out_texCoords[4].y += ( 2 - 1.5 ) * texSize.y;
   Out_texCoords[4].z += ( 1 - 1.5 ) * texSize.x;
   Out_texCoords[4].w += ( 2 - 1.5 ) * texSize.y;
   
   Out_texCoords[5] = uv;
   Out_texCoords[5].x += texSize.x;
   Out_texCoords[5].y += texSize.y;
   Out_texCoords[5].z += texSize.x;
   Out_texCoords[5].w += texSize.y;
   Out_texCoords[5].x += ( 2 - 1.5 ) * texSize.x;
   Out_texCoords[5].y += ( 2 - 1.5 ) * texSize.y;
   Out_texCoords[5].z += ( 3 - 1.5 ) * texSize.x;
   Out_texCoords[5].w += ( 2 - 1.5 ) * texSize.y;
   
   Out_texCoords[6] = uv;
   Out_texCoords[6].x += texSize.x;
   Out_texCoords[6].y += texSize.y;
   Out_texCoords[6].z += texSize.x;
   Out_texCoords[6].w += texSize.y;
   Out_texCoords[6].x += ( 0 - 1.5 ) * texSize.x;
   Out_texCoords[6].y += ( 3 - 1.5 ) * texSize.y;
   Out_texCoords[6].z += ( 1 - 1.5 ) * texSize.x;
   Out_texCoords[6].w += ( 3 - 1.5 ) * texSize.y;
   
   Out_texCoords[7] = uv;
   Out_texCoords[7].x += texSize.x;
   Out_texCoords[7].y += texSize.y;
   Out_texCoords[7].z += texSize.x;
   Out_texCoords[7].w += texSize.y;
   Out_texCoords[7].x += ( 2 - 1.5 ) * texSize.x;
   Out_texCoords[7].y += ( 3 - 1.5 ) * texSize.y;
   Out_texCoords[7].z += ( 3 - 1.5 ) * texSize.x;
   Out_texCoords[7].w += ( 3 - 1.5 ) * texSize.y;
   
	correctSSP(gl_Position);
}

