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

#include "./postFx.hlsl"
#include "../torque.hlsl"
#include "../shaderModelAutoGen.hlsl"

uniform float4x4 matPrevScreenToWorld;
uniform float4x4 matWorldToScreen;

// Passed in from setShaderConsts()
uniform float velocityMultiplier;
TORQUE_UNIFORM_SAMPLER2D(backBuffer, 0);
TORQUE_UNIFORM_SAMPLER2D(prepassTex, 1);

float4 main(PFXVertToPix IN) : TORQUE_TARGET0
{
   float samples = 5;
   
   // First get the prepass texture for uv channel 0
   float4 prepass = TORQUE_PREPASS_UNCONDITION( prepassTex, IN.uv0 );
   
   // Next extract the depth
   float depth = prepass.a;
   
   // Create the screen position
   float4 screenPos = float4(IN.uv0.x*2-1, IN.uv0.y*2-1, depth*2-1, 1);

   // Calculate the world position
   float4 D = mul(screenPos, matWorldToScreen);
   float4 worldPos = D / D.w;
   
   // Now calculate the previous screen position
   float4 previousPos = mul( worldPos, matPrevScreenToWorld );
   previousPos /= previousPos.w;
	
   // Calculate the XY velocity
   float2 velocity = ((screenPos - previousPos) / velocityMultiplier).xy;
   
   // Generate the motion blur
   float4 color = TORQUE_TEX2D(backBuffer, IN.uv0);
	IN.uv0 += velocity;
	
   for(int i = 1; i<samples; ++i, IN.uv0 += velocity)
   {
      float4 currentColor = TORQUE_TEX2D(backBuffer, IN.uv0);
      color += currentColor;
   }
   
   return color / samples;
}