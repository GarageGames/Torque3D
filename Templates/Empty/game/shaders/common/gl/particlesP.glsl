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

#include "hlslCompat.glsl"
#include "torque.glsl"

// With advanced lighting we get soft particles.
#ifdef TORQUE_LINEAR_DEPTH
   #define SOFTPARTICLES  
#endif

#define CLIP_Z // TODO: Make this a proper macro

uniform sampler2D diffuseMap;

#ifdef SOFTPARTICLES
   
   #include "shadergen:/autogenConditioners.h"
   
   uniform float oneOverSoftness;
   uniform float oneOverFar;
   uniform sampler2D prepassTex;
   //uniform vec3 vEye;
   uniform vec4 prePassTargetParams;
#endif

uniform float alphaFactor;
uniform float alphaScale;

varying vec4 color;
varying vec2 uv0;
varying vec4 pos;


void main()
{
   float softBlend = 1.0;
   
   #ifdef SOFTPARTICLES
      float2 tc = pos.xy * vec2(1.0, -1.0 ) / pos.w;
      tc = viewportCoordToRenderTarget(saturate( ( tc + 1.0 ) * 0.5 ), prePassTargetParams); 
   
   	float sceneDepth = prepassUncondition( prepassTex, tc ).w;   	   	   			
   	float depth = pos.w * oneOverFar;   	
	   float diff = sceneDepth - depth;
	#ifdef CLIP_Z
	   // If drawing offscreen, this acts as the depth test, since we don't line up with the z-buffer
	   // When drawing high-res, though, we want to be able to take advantage of hi-z
	   // so this is #ifdef'd out
      if (diff < 0.0)
         discard;
	#endif
      softBlend = saturate( diff * oneOverSoftness );
   #endif
		   
   vec4 diffuse = texture2D( diffuseMap, uv0 );
   
   // Scale output color by the alpha factor (turn LerpAlpha into pre-multiplied alpha)
   vec3 colorScale = ( alphaFactor < 0.0 ? color.rgb * diffuse.rgb : ( alphaFactor > 0.0 ? vec3(color.a * alphaFactor * diffuse.a * softBlend) : vec3(softBlend) ) );
   
   gl_FragColor = hdrEncode( vec4(color.rgb * diffuse.rgb * colorScale, softBlend * color.a * diffuse.a * alphaScale) );
}

