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

#include "../../shaderModel.hlsl"
#include "../../shaderModelAutoGen.hlsl"

#include "farFrustumQuad.hlsl"
#include "../../torque.hlsl"
#include "../../lighting.hlsl"
#include "lightingUtils.hlsl"
#include "../shadowMap/shadowMapIO_HLSL.h"
#include "softShadow.hlsl"

TORQUE_UNIFORM_SAMPLER2D(prePassBuffer, 0);
TORQUE_UNIFORM_SAMPLER2D(shadowMap, 1);
TORQUE_UNIFORM_SAMPLER2D(dynamicShadowMap, 2);

#ifdef USE_SSAO_MASK
TORQUE_UNIFORM_SAMPLER2D(ssaoMask, 3);
uniform float4 rtParams3;
#endif
//register 4?
TORQUE_UNIFORM_SAMPLER2D(lightBuffer, 5);
TORQUE_UNIFORM_SAMPLER2D(colorBuffer, 6);
TORQUE_UNIFORM_SAMPLER2D(matInfoBuffer, 7);

uniform float  lightBrightness;
uniform float3 lightDirection;

uniform float4 lightColor;
uniform float4 lightAmbient;

uniform float shadowSoftness;
uniform float3 eyePosWorld;

uniform float4 atlasXOffset;
uniform float4 atlasYOffset;
uniform float4 zNearFarInvNearFar;
uniform float4 lightMapParams;
uniform float4 farPlaneScalePSSM;
uniform float4 overDarkPSSM;

uniform float2 fadeStartLength;
uniform float2 atlasScale;

uniform float4x4 eyeMat;

// Static Shadows
uniform float4x4 worldToLightProj;
uniform float4 scaleX;
uniform float4 scaleY;
uniform float4 offsetX;
uniform float4 offsetY;
// Dynamic Shadows
uniform float4x4 dynamicWorldToLightProj;
uniform float4 dynamicScaleX;
uniform float4 dynamicScaleY;
uniform float4 dynamicOffsetX;
uniform float4 dynamicOffsetY;
uniform float4 dynamicFarPlaneScalePSSM;

float4 AL_VectorLightShadowCast( TORQUE_SAMPLER2D(sourceShadowMap),
                                float2 texCoord,
                                float4x4 worldToLightProj,
                                float4 worldPos,
                                float4 scaleX,
                                float4 scaleY,
                                float4 offsetX,
                                float4 offsetY,
                                float4 farPlaneScalePSSM,
                                float4 atlasXOffset,
                                float4 atlasYOffset,
                                float2 atlasScale,
                                float shadowSoftness, 
                                float dotNL ,
                                float4 overDarkPSSM)
{
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
         
      float3 debugColor = float3(0,0,0);
   
      #ifdef NO_SHADOW
         debugColor = float3(1.0,1.0,1.0);
      #endif

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

      return float4(debugColor,
                    softShadow_filter(  TORQUE_SAMPLER2D_MAKEARG(sourceShadowMap),
                                 texCoord,
                                 shadowCoord,
                                 farPlaneScale * shadowSoftness,
                                 distToLight,
                                 dotNL,
                                 dot( finalMask, overDarkPSSM ) ) );
};

float4 main( FarFrustumQuadConnectP IN ) : TORQUE_TARGET0
{
   // Emissive.
   float4 matInfo = TORQUE_TEX2D( matInfoBuffer, IN.uv0 );   
   bool emissive = getFlag( matInfo.r, 0 );
   if ( emissive )
   {
       return float4(1.0, 1.0, 1.0, 0.0);
   }
   
   float4 colorSample = TORQUE_TEX2D( colorBuffer, IN.uv0 );
   float3 subsurface = float3(0.0,0.0,0.0); 
   if (getFlag( matInfo.r, 1 ))
   {
      subsurface = colorSample.rgb;
      if (colorSample.r>colorSample.g)
         subsurface = float3(0.772549, 0.337255, 0.262745);
	  else
         subsurface = float3(0.337255, 0.772549, 0.262745);
	}
   // Sample/unpack the normal/z data
   float4 prepassSample = TORQUE_PREPASS_UNCONDITION( prePassBuffer, IN.uv0 );
   float3 normal = prepassSample.rgb;
   float depth = prepassSample.a;

   // Use eye ray to get ws pos
   float4 worldPos = float4(eyePosWorld + IN.wsEyeRay * depth, 1.0f);
   
   // Get the light attenuation.
   float dotNL = dot(-lightDirection, normal);

   #ifdef PSSM_DEBUG_RENDER
      float3 debugColor = float3(0,0,0);
   #endif
   
   #ifdef NO_SHADOW

      // Fully unshadowed.
      float shadowed = 1.0;

      #ifdef PSSM_DEBUG_RENDER
         debugColor = float3(1.0,1.0,1.0);
      #endif

   #else
      
      float4 static_shadowed_colors = AL_VectorLightShadowCast( TORQUE_SAMPLER2D_MAKEARG(shadowMap),
                                                        IN.uv0.xy,
                                                        worldToLightProj,
                                                        worldPos,
                                                        scaleX, scaleY,
                                                        offsetX, offsetY,
                                                        farPlaneScalePSSM,
                                                        atlasXOffset, atlasYOffset,
                                                        atlasScale,
                                                        shadowSoftness, 
                                                        dotNL,
                                                        overDarkPSSM);
      float4 dynamic_shadowed_colors = AL_VectorLightShadowCast( TORQUE_SAMPLER2D_MAKEARG(dynamicShadowMap),
                                                        IN.uv0.xy,
                                                        dynamicWorldToLightProj,
                                                        worldPos,
                                                        dynamicScaleX, dynamicScaleY,
                                                        dynamicOffsetX, dynamicOffsetY,
                                                        dynamicFarPlaneScalePSSM,
                                                        atlasXOffset, atlasYOffset,
                                                        atlasScale,
                                                        shadowSoftness, 
                                                        dotNL,
                                                        overDarkPSSM);
      
      float static_shadowed = static_shadowed_colors.a;
      float dynamic_shadowed = dynamic_shadowed_colors.a;
	  
      #ifdef PSSM_DEBUG_RENDER
	     debugColor = static_shadowed_colors.rgb*0.5+dynamic_shadowed_colors.rgb*0.5;
      #endif
  
      // Fade out the shadow at the end of the range.
      float4 zDist = (zNearFarInvNearFar.x + zNearFarInvNearFar.y * depth);
      float fadeOutAmt = ( zDist.x - fadeStartLength.x ) * fadeStartLength.y;

      static_shadowed = lerp( static_shadowed, 1.0, saturate( fadeOutAmt ) );
      dynamic_shadowed = lerp( dynamic_shadowed, 1.0, saturate( fadeOutAmt ) );

      // temp for debugging. uncomment one or the other.
      //float shadowed = static_shadowed;
      //float shadowed = dynamic_shadowed;
      float shadowed = min(static_shadowed, dynamic_shadowed);

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
   
   float4 addToResult = (lightAmbient * (1 - ambientCameraFactor)) + ( lightAmbient * ambientCameraFactor * saturate(dot(normalize(-IN.vsEyeRay), normal)) );

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
      float ao = 1.0 - TORQUE_TEX2D( ssaoMask, viewportCoordToRenderTarget( IN.uv0.xy, rtParams3 ) ).r;
      addToResult *= ao;
   #endif

   #ifdef PSSM_DEBUG_RENDER
      lightColorOut = debugColor;
   #endif

   return AL_DeferredOutput(lightColorOut+subsurface*(1.0-Sat_NL_Att), colorSample.rgb, matInfo, addToResult, specular, Sat_NL_Att);
}
