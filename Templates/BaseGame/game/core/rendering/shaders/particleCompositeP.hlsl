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
#include "torque.hlsl"
#include "shaderModel.hlsl"

TORQUE_UNIFORM_SAMPLER2D(colorSource, 0);
uniform float4 offscreenTargetParams;

#ifdef TORQUE_LINEAR_DEPTH
#define REJECT_EDGES
TORQUE_UNIFORM_SAMPLER2D(edgeSource, 1);
uniform float4 edgeTargetParams;
#endif

struct Conn
{
   float4 hpos : TORQUE_POSITION;
   float4 offscreenPos : TEXCOORD0;
   float4 backbufferPos : TEXCOORD1;
};


float4 main(Conn IN) : TORQUE_TARGET0
{  
   // Off-screen particle source screenspace position in XY
   // Back-buffer screenspace position in ZW
   float4 ssPos = float4(IN.offscreenPos.xy / IN.offscreenPos.w, IN.backbufferPos.xy / IN.backbufferPos.w);
   
	float4 uvScene = ( ssPos + 1.0 ) / 2.0;
	uvScene.yw = 1.0 - uvScene.yw;
	uvScene.xy = viewportCoordToRenderTarget(uvScene.xy, offscreenTargetParams);
	
#ifdef REJECT_EDGES
   // Cut out particles along the edges, this will create the stencil mask
	uvScene.zw = viewportCoordToRenderTarget(uvScene.zw, edgeTargetParams);
	float edge = TORQUE_TEX2D( edgeSource, uvScene.zw ).r;
	clip( -edge );
#endif
	
	// Sample offscreen target and return
   return TORQUE_TEX2D( colorSource, uvScene.xy );
}