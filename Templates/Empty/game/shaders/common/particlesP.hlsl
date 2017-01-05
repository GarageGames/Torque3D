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
// With advanced lighting we get soft particles.
#ifdef TORQUE_LINEAR_DEPTH
   #define SOFTPARTICLES  
#endif

#ifdef SOFTPARTICLES
   
   #include "shaderModelAutoGen.hlsl"
   
   uniform float oneOverSoftness;
   uniform float oneOverFar;
   TORQUE_UNIFORM_SAMPLER2D(prepassTex, 1);
   //uniform float3 vEye;
   uniform float4 prePassTargetParams;
#endif

#define CLIP_Z // TODO: Make this a proper macro

struct Conn
{
   float4 hpos : TORQUE_POSITION;
   float4 color : TEXCOORD0;
   float2 uv0 : TEXCOORD1;
   float4 pos : TEXCOORD2;
};

TORQUE_UNIFORM_SAMPLER2D(diffuseMap, 0);
TORQUE_UNIFORM_SAMPLER2D(paraboloidLightMap, 2);

float4 lmSample( float3 nrm )
{
   bool calcBack = (nrm.z < 0.0);
   if ( calcBack )
      nrm.z = nrm.z * -1.0;

   float2 lmCoord;
   lmCoord.x = (nrm.x / (2*(1 + nrm.z))) + 0.5;
   lmCoord.y = 1-((nrm.y / (2*(1 + nrm.z))) + 0.5);


   // If this is the back, offset in the atlas
   if ( calcBack )
      lmCoord.x += 1.0;
      
   // Atlasing front and back maps, so scale
   lmCoord.x *= 0.5;

   return TORQUE_TEX2D(paraboloidLightMap, lmCoord);
}


uniform float alphaFactor;
uniform float alphaScale;

float4 main( Conn IN ) : TORQUE_TARGET0
{
   float softBlend = 1;
   
   #ifdef SOFTPARTICLES
      float2 tc = IN.pos.xy * float2(1.0, -1.0) / IN.pos.w;
      tc = viewportCoordToRenderTarget(saturate( ( tc + 1.0 ) * 0.5 ), prePassTargetParams); 
   
      float sceneDepth = TORQUE_PREPASS_UNCONDITION(prepassTex, tc).w;
   	float depth = IN.pos.w * oneOverFar;   	
	float diff = sceneDepth - depth;
	#ifdef CLIP_Z
	   // If drawing offscreen, this acts as the depth test, since we don't line up with the z-buffer
	   // When drawing high-res, though, we want to be able to take advantage of hi-z
	   // so this is #ifdef'd out
	   //clip(diff);
	#endif
      softBlend = saturate( diff * oneOverSoftness );
   #endif
	   
   float4 diffuse = TORQUE_TEX2D( diffuseMap, IN.uv0 );
   
   //return float4( lmSample(float3(0, 0, -1)).rgb, IN.color.a * diffuse.a * softBlend * alphaScale);
   
   // Scale output color by the alpha factor (turn LerpAlpha into pre-multiplied alpha)
   float3 colorScale = ( alphaFactor < 0.0 ? IN.color.rgb * diffuse.rgb : ( alphaFactor > 0.0 ? IN.color.a * diffuse.a * alphaFactor * softBlend : softBlend ) );
   
   return hdrEncode( float4( IN.color.rgb * diffuse.rgb * colorScale,
                  IN.color.a * diffuse.a * softBlend * alphaScale ) );
}

