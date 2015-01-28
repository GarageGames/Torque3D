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
#include "farFrustumQuad.glsl"
#include "../../../gl/torque.glsl"
#include "../../../gl/lighting.glsl"
#include "lightingUtils.glsl"
#include "../../shadowMap/shadowMapIO_GLSL.h"
#include "softShadow.glsl"

in vec4 hpos;
in vec2 uv0;
in vec3 wsEyeRay;
in vec3 vsEyeRay;

uniform sampler2D ShadowMap ;

#ifdef USE_SSAO_MASK
uniform sampler2D ssaoMask ;
uniform vec4 rtParams2;
#endif

uniform sampler2D prePassBuffer;             
uniform vec3 lightDirection;
uniform vec4 lightColor;
uniform float  lightBrightness;
uniform vec4 lightAmbient; 
uniform vec3 eyePosWorld; 
uniform mat4x4 worldToLightProj;
uniform vec4 scaleX;
uniform vec4 scaleY;
uniform vec4 offsetX;
uniform vec4 offsetY;
uniform vec4 atlasXOffset;
uniform vec4 atlasYOffset;
uniform vec2 atlasScale;
uniform vec4 zNearFarInvNearFar;
uniform vec4 lightMapParams;
uniform vec2 fadeStartLength;
uniform vec4 farPlaneScalePSSM;
uniform vec4 overDarkPSSM;
uniform float shadowSoftness;

out vec4 OUT_col;

void main()             
{
   // Sample/unpack the normal/z data
   vec4 prepassSample = prepassUncondition( prePassBuffer, uv0 );
   vec3 normal = prepassSample.rgb;
   float depth = prepassSample.a;

   // Use eye ray to get ws pos
   vec4 worldPos = vec4(eyePosWorld + wsEyeRay * depth, 1.0f);
   
   // Get the light attenuation.
   float dotNL = dot(-lightDirection, normal);

   #ifdef PSSM_DEBUG_RENDER
      vec3 debugColor = vec3(0);
   #endif
   
   #ifdef NO_SHADOW

      // Fully unshadowed.
      float shadowed = 1.0;

      #ifdef PSSM_DEBUG_RENDER
         debugColor = vec3(1.0);
      #endif

   #else

      // Compute shadow map coordinate
      vec4 pxlPosLightProj = tMul(worldToLightProj, worldPos);
      vec2 baseShadowCoord = pxlPosLightProj.xy / pxlPosLightProj.w;   

      // Distance to light, in shadowmap space
      float distToLight = pxlPosLightProj.z / pxlPosLightProj.w;
         
      // Figure out which split to sample from.  Basically, we compute the shadowmap sample coord
      // for all of the splits and then check if its valid.  
      vec4 shadowCoordX = vec4( baseShadowCoord.x );
      vec4 shadowCoordY = vec4( baseShadowCoord.y );
      vec4 farPlaneDists = vec4( distToLight );      
      shadowCoordX *= scaleX;
      shadowCoordY *= scaleY;
      shadowCoordX += offsetX;
      shadowCoordY += offsetY;
      farPlaneDists *= farPlaneScalePSSM;
      
      // If the shadow sample is within -1..1 and the distance 
      // to the light for this pixel is less than the far plane 
      // of the split, use it.
      vec4 finalMask;
      if (  shadowCoordX.x > -0.99 && shadowCoordX.x < 0.99 && 
            shadowCoordY.x > -0.99 && shadowCoordY.x < 0.99 &&
            farPlaneDists.x < 1.0 )
         finalMask = vec4(1, 0, 0, 0);

      else if (   shadowCoordX.y > -0.99 && shadowCoordX.y < 0.99 &&
                  shadowCoordY.y > -0.99 && shadowCoordY.y < 0.99 && 
                  farPlaneDists.y < 1.0 )
         finalMask = vec4(0, 1, 0, 0);

      else if (   shadowCoordX.z > -0.99 && shadowCoordX.z < 0.99 && 
                  shadowCoordY.z > -0.99 && shadowCoordY.z < 0.99 && 
                  farPlaneDists.z < 1.0 )
         finalMask = vec4(0, 0, 1, 0);
         
      else
         finalMask = vec4(0, 0, 0, 1);
         

      #ifdef PSSM_DEBUG_RENDER
         if ( finalMask.x > 0 )
            debugColor += vec3( 1, 0, 0 );
         else if ( finalMask.y > 0 )
            debugColor += vec3( 0, 1, 0 );
         else if ( finalMask.z > 0 )
            debugColor += vec3( 0, 0, 1 );
         else if ( finalMask.w > 0 )
            debugColor += vec3( 1, 1, 0 );
      #endif

      // Here we know what split we're sampling from, so recompute the texcoord location
      // Yes, we could just use the result from above, but doing it this way actually saves
      // shader instructions.
      vec2 finalScale;
      finalScale.x = dot(finalMask, scaleX);
      finalScale.y = dot(finalMask, scaleY);

      vec2 finalOffset;
      finalOffset.x = dot(finalMask, offsetX);
      finalOffset.y = dot(finalMask, offsetY);

      vec2 shadowCoord;                  
      shadowCoord = baseShadowCoord * finalScale;      
      shadowCoord += finalOffset;

      // Convert to texcoord space
      shadowCoord = 0.5 * shadowCoord + vec2(0.5, 0.5);
      shadowCoord.y = 1.0f - shadowCoord.y;

      // Move around inside of atlas 
      vec2 aOffset;
      aOffset.x = dot(finalMask, atlasXOffset);
      aOffset.y = dot(finalMask, atlasYOffset);

      shadowCoord *= atlasScale;
      shadowCoord += aOffset;
              
      // Each split has a different far plane, take this into account.
      float farPlaneScale = dot( farPlaneScalePSSM, finalMask );
      distToLight *= farPlaneScale;
      
      float shadowed = softShadow_filter(   ShadowMap,
                                             uv0.xy,
                                             shadowCoord,
                                             farPlaneScale * shadowSoftness,
                                             distToLight,
                                             dotNL,
                                             dot( finalMask, overDarkPSSM ) );
  
      // Fade out the shadow at the end of the range.
      vec4 zDist = vec4(zNearFarInvNearFar.x + zNearFarInvNearFar.y * depth);
      float fadeOutAmt = ( zDist.x - fadeStartLength.x ) * fadeStartLength.y;
      shadowed = mix( shadowed, 1.0, saturate( fadeOutAmt ) );

      #ifdef PSSM_DEBUG_RENDER
         if ( fadeOutAmt > 1.0 )
            debugColor = vec3(1.0);
      #endif

   #endif // !NO_SHADOW

   // Specular term
   float specular = AL_CalcSpecular(   -lightDirection, 
                                       normal, 
                                       normalize(-vsEyeRay) ) * lightBrightness * shadowed;
   
   float Sat_NL_Att = saturate( dotNL * shadowed ) * lightBrightness;
   vec3 lightColorOut = lightMapParams.rgb * lightColor.rgb;
   vec4 addToResult = (lightAmbient * (1 - ambientCameraFactor)) + ( lightAmbient * ambientCameraFactor * saturate(dot(normalize(-vsEyeRay), normal)) );

   // TODO: This needs to be removed when lightmapping is disabled
   // as its extra work per-pixel on dynamic lit scenes.
   //
   // Special lightmapping pass.
   if ( lightMapParams.a < 0.0 )
   {
      // This disables shadows on the backsides of objects.
      shadowed = dotNL < 0.0f ? 1.0f : shadowed;

      Sat_NL_Att = 1.0f;
      lightColorOut = vec3(shadowed);
      specular *= lightBrightness;
      addToResult = ( 1.0 - shadowed ) * abs(lightMapParams);
   }

   // Sample the AO texture.      
   #ifdef USE_SSAO_MASK
      float ao = 1.0 - texture( ssaoMask, viewportCoordToRenderTarget( uv0.xy, rtParams2 ) ).r;
      addToResult *= ao;
   #endif

   #ifdef PSSM_DEBUG_RENDER
      lightColorOut = debugColor;
   #endif
   
   OUT_col = lightinfoCondition( lightColorOut, Sat_NL_Att, specular, addToResult );  
   
}
