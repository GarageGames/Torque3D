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

#include "shadergen:/autogenConditioners.h"

#include "farFrustumQuad.hlsl"
#include "../../torque.hlsl"
#include "../../lighting.hlsl"
#include "lightingUtils.hlsl"
#include "../shadowMap/shadowMapIO_HLSL.h"
#include "softShadow.hlsl"


uniform sampler2D ShadowMap : register(S1);

#ifdef USE_SSAO_MASK
uniform sampler2D ssaoMask : register(S2);
uniform float4 rtParams2;
#endif


float4 main( FarFrustumQuadConnectP IN,

             uniform sampler2D prePassBuffer : register(S0),
             
             uniform sampler2D lightBuffer : register(S5),
             uniform sampler2D colorBuffer : register(S6),
             uniform sampler2D matInfoBuffer : register(S7),
             
             uniform float3 lightDirection,
             uniform float4 lightColor,
             uniform float  lightBrightness,
             uniform float4 lightAmbient,
             
             uniform float3 eyePosWorld,
             uniform float4x4 eyeMat, 
             
             uniform float4x4 worldToLightProj,

             uniform float4 scaleX,
             uniform float4 scaleY,
             uniform float4 offsetX,
             uniform float4 offsetY,
             uniform float4 atlasXOffset,
             uniform float4 atlasYOffset,
             uniform float2 atlasScale,
             uniform float4 zNearFarInvNearFar,
             uniform float4 lightMapParams,

             uniform float2 fadeStartLength,
             uniform float4 farPlaneScalePSSM,
             uniform float4 overDarkPSSM,
             uniform float shadowSoftness ) : COLOR0
{
   // Emissive.
   float4 matInfo = tex2D( matInfoBuffer, IN.uv0 );   
   bool emissive = getFlag( matInfo.r, 0 );
   if ( emissive )
   {
       return float4(1.0, 1.0, 1.0, 0.0);
   }

   // Sample/unpack the normal/z data
   float4 prepassSample = prepassUncondition( prePassBuffer, IN.uv0 );
   float3 normal = prepassSample.rgb;
   float depth = prepassSample.a;

   // Use eye ray to get ws pos
   float4 worldPos = float4(eyePosWorld + IN.wsEyeRay * depth, 1.0f);
   
   // Get the light attenuation.
   float dotNL = dot(-lightDirection, normal);

   #ifdef PSSM_DEBUG_RENDER
      float3 debugColor = 0;
   #endif
   
   #ifdef NO_SHADOW

      // Fully unshadowed.
      float shadowed = 1.0;

      #ifdef PSSM_DEBUG_RENDER
         debugColor = 1.0;
      #endif

   #else

      // Compute shadow map coordinate
      float4 pxlPosLightProj = mul(worldToLightProj, worldPos);
      float2 baseShadowCoord = pxlPosLightProj.xy / pxlPosLightProj.w;   

      // Distance to light, in shadowmap space
      float distToLight = pxlPosLightProj.z / pxlPosLightProj.w;
         
      // Figure out which split to sample from.  Basically, we compute the shadowmap sample coord
      // for all of the splits and then check if its valid.  
      float4 shadowCoordX = baseShadowCoord.xxxx;
      float4 shadowCoordY = baseShadowCoord.yyyy;
      float4 farPlaneDists = distToLight.xxxx;      
      shadowCoordX *= scaleX;
      shadowCoordY *= scaleY;
      shadowCoordX += offsetX;
      shadowCoordY += offsetY;
      farPlaneDists *= farPlaneScalePSSM;
      
      // If the shadow sample is within -1..1 and the distance 
      // to the light for this pixel is less than the far plane 
      // of the split, use it.
      float4 finalMask;
      if (  shadowCoordX.x > -0.99 && shadowCoordX.x < 0.99 && 
            shadowCoordY.x > -0.99 && shadowCoordY.x < 0.99 &&
            farPlaneDists.x < 1.0 )
         finalMask = float4(1, 0, 0, 0);

      else if (   shadowCoordX.y > -0.99 && shadowCoordX.y < 0.99 &&
                  shadowCoordY.y > -0.99 && shadowCoordY.y < 0.99 && 
                  farPlaneDists.y < 1.0 )
         finalMask = float4(0, 1, 0, 0);

      else if (   shadowCoordX.z > -0.99 && shadowCoordX.z < 0.99 && 
                  shadowCoordY.z > -0.99 && shadowCoordY.z < 0.99 && 
                  farPlaneDists.z < 1.0 )
         finalMask = float4(0, 0, 1, 0);
         
      else
         finalMask = float4(0, 0, 0, 1);
         

      #ifdef PSSM_DEBUG_RENDER
         if ( finalMask.x > 0 )
            debugColor += float3( 1, 0, 0 );
         else if ( finalMask.y > 0 )
            debugColor += float3( 0, 1, 0 );
         else if ( finalMask.z > 0 )
            debugColor += float3( 0, 0, 1 );
         else if ( finalMask.w > 0 )
            debugColor += float3( 1, 1, 0 );
      #endif

      // Here we know what split we're sampling from, so recompute the texcoord location
      // Yes, we could just use the result from above, but doing it this way actually saves
      // shader instructions.
      float2 finalScale;
      finalScale.x = dot(finalMask, scaleX);
      finalScale.y = dot(finalMask, scaleY);

      float2 finalOffset;
      finalOffset.x = dot(finalMask, offsetX);
      finalOffset.y = dot(finalMask, offsetY);

      float2 shadowCoord;                  
      shadowCoord = baseShadowCoord * finalScale;      
      shadowCoord += finalOffset;

      // Convert to texcoord space
      shadowCoord = 0.5 * shadowCoord + float2(0.5, 0.5);
      shadowCoord.y = 1.0f - shadowCoord.y;

      // Move around inside of atlas 
      float2 aOffset;
      aOffset.x = dot(finalMask, atlasXOffset);
      aOffset.y = dot(finalMask, atlasYOffset);

      shadowCoord *= atlasScale;
      shadowCoord += aOffset;
              
      // Each split has a different far plane, take this into account.
      float farPlaneScale = dot( farPlaneScalePSSM, finalMask );
      distToLight *= farPlaneScale;
      
      float shadowed = softShadow_filter(   ShadowMap,
                                             IN.uv0.xy,
                                             shadowCoord,
                                             farPlaneScale * shadowSoftness,
                                             distToLight,
                                             dotNL,
                                             dot( finalMask, overDarkPSSM ) );
  
      // Fade out the shadow at the end of the range.
      float4 zDist = (zNearFarInvNearFar.x + zNearFarInvNearFar.y * depth);
      float fadeOutAmt = ( zDist.x - fadeStartLength.x ) * fadeStartLength.y;
      shadowed = lerp( shadowed, 1.0, saturate( fadeOutAmt ) );

      #ifdef PSSM_DEBUG_RENDER
         if ( fadeOutAmt > 1.0 )
            debugColor = 1.0;
      #endif

   #endif // !NO_SHADOW

   // Specular term
   float specular = AL_CalcSpecular(   -lightDirection, 
                                       normal, 
                                       normalize(-IN.vsEyeRay) ) * lightBrightness * shadowed;
                                    
   float Sat_NL_Att = saturate( dotNL * shadowed ) * lightBrightness;
   float3 lightColorOut = lightMapParams.rgb * lightColor.rgb;

   // Felix' Normal Mapped Ambient.
   float ambientBrightness = (float)lightAmbient;
   float3 worldNormal = normalize(mul(eyeMat, float4(normal,1.0))).xyz;
   float ambientContrast = 0.5;  
   float4 upAmbient = lerp( 1 - lightAmbient * 0.65, lightAmbient, 1-ambientBrightness*ambientContrast );
   float4 lightAmbientTwoTone = lerp( lightAmbient * 0.8 , upAmbient , worldNormal.b ); 
   float4 addToResult = lightAmbientTwoTone + dotNL * lightColor * ambientBrightness * 0.25; 

   //float4 addToResult = lightAmbient;

   // TODO: This needs to be removed when lightmapping is disabled
   // as its extra work per-pixel on dynamic lit scenes.
   //
   // Special lightmapping pass.
   if ( lightMapParams.a < 0.0 )
   {
      // This disables shadows on the backsides of objects.
      shadowed = dotNL < 0.0f ? 1.0f : shadowed;

      Sat_NL_Att = 1.0f;
      lightColorOut = shadowed;
      specular *= lightBrightness;
      addToResult = ( 1.0 - shadowed ) * abs(lightMapParams);
   }

   // Sample the AO texture.      
   #ifdef USE_SSAO_MASK
      float ao = 1.0 - tex2D( ssaoMask, viewportCoordToRenderTarget( IN.uv0.xy, rtParams2 ) ).r;
      addToResult *= ao;
   #endif

   #ifdef PSSM_DEBUG_RENDER
      lightColorOut = debugColor;
   #endif

   bool translucent = getFlag( matInfo.r, 2 );
   if ( translucent && matInfo.g > 0.1 )
   {
      float fLTDistortion = 0.1;
      int iLTPower = 10;
      float fLTAmbient = 0.0;
      float fLTThickness = matInfo.g;
      float fLTScale = 0.5;

      float3 vLTLight = lightDirection + normal * fLTDistortion;
      float fLTDot = pow(saturate(dot(normalize(IN.vsEyeRay), -vLTLight)), iLTPower) * fLTScale;
      float3 fLT = 1.0 * (fLTDot + fLTAmbient) * fLTThickness;

      addToResult = lightColor * float4( fLT, 0.0);
   }

   float4 colorSample = tex2D( colorBuffer, IN.uv0 );
   return AL_DeferredOutput(lightColorOut, colorSample.rgb, matInfo, addToResult, specular, colorSample.a, Sat_NL_Att);
}
