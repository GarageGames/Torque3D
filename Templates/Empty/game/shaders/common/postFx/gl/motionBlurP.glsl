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

#include "../../gl/hlslCompat.glsl"
#include "../../gl/torque.glsl"
#include "shadergen:/autogenConditioners.h"
#include "postFX.glsl"

#undef IN_uv0
#define _IN_uv0 uv0

uniform mat4 matPrevScreenToWorld;
uniform mat4 matWorldToScreen;

// Passed in from setShaderConsts()
uniform float velocityMultiplier;

uniform sampler2D backBuffer;
uniform sampler2D prepassTex;

out vec4 OUT_col;

void main()
{
   vec2 IN_uv0 = _IN_uv0;
   float samples = 5;
   
   // First get the prepass texture for uv channel 0
   vec4 prepass = prepassUncondition( prepassTex, IN_uv0 );
   
   // Next extract the depth
   float depth = prepass.a;
   
   // Create the screen position
   vec4 screenPos = vec4(IN_uv0.x*2-1, IN_uv0.y*2-1, depth*2-1, 1);

   // Calculate the world position
   vec4 D = tMul(screenPos, matWorldToScreen);
   vec4 worldPos = D / D.w;
   
   // Now calculate the previous screen position
   vec4 previousPos = tMul( worldPos, matPrevScreenToWorld );
   previousPos /= previousPos.w;
	
   // Calculate the XY velocity
   vec2 velocity = ((screenPos - previousPos) / velocityMultiplier).xy;
   
   // Generate the motion blur
   vec4 color = texture(backBuffer, IN_uv0);
	IN_uv0 += velocity;
	
   for(int i = 1; i<samples; ++i, IN_uv0 += velocity)
   {
      vec4 currentColor = texture(backBuffer, IN_uv0);
      color += currentColor;
   }
   
   OUT_col = color / samples;
}