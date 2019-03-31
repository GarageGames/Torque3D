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

#include "torque.glsl"
#include "hlslCompat.glsl"

in vec4 offscreenPos;
in vec4 backbufferPos;

#define IN_offscreenPos offscreenPos
#define IN_backbufferPos backbufferPos

uniform sampler2D colorSource;
uniform vec4 offscreenTargetParams;

#ifdef TORQUE_LINEAR_DEPTH
#define REJECT_EDGES
uniform sampler2D edgeSource;
uniform vec4 edgeTargetParams;
#endif

out vec4 OUT_col;

void main()
{  
   // Off-screen particle source screenspace position in XY
   // Back-buffer screenspace position in ZW
   vec4 ssPos = vec4(offscreenPos.xy / offscreenPos.w, backbufferPos.xy / backbufferPos.w);
   
	vec4 uvScene = ( ssPos + 1.0 ) / 2.0;
	uvScene.yw = 1.0 - uvScene.yw;
	uvScene.xy = viewportCoordToRenderTarget(uvScene.xy, offscreenTargetParams);
	
#ifdef REJECT_EDGES
   // Cut out particles along the edges, this will create the stencil mask
	uvScene.zw = viewportCoordToRenderTarget(uvScene.zw, edgeTargetParams);
	float edge = texture( edgeSource, uvScene.zw ).r;
	clip( -edge );
#endif
	
	// Sample offscreen target and return
   OUT_col = texture( colorSource, uvScene.xy );
}