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
#include "lightingUtils.glsl"
#include "../../shadowMap/shadowMapIO_GLSL.h"

varying vec2 uv0;
varying vec3 wsEyeRay;

uniform sampler2D prePassBuffer;
uniform sampler2D ShadowMap;

#if TORQUE_SM >= 30

   // Enables high quality soft shadow 
   // filtering for SM3.0 and above.
   #define SOFTSHADOW_SM3

   #include "softShadow.glsl"

#else

   
   
#endif
             
uniform vec3 lightDirection;
uniform vec4 lightColor;
uniform float lightBrightness;
uniform vec4 lightAmbient;
uniform vec4 lightTrilight;
            
uniform vec3 eyePosWorld;

uniform mat4 worldToLightProj;
uniform vec4 splitDistStart;
uniform vec4 splitDistEnd;
uniform vec4 scaleX;
uniform vec4 scaleY;
uniform vec4 offsetX;
uniform vec4 offsetY;
uniform vec4 atlasXOffset;
uniform vec4 atlasYOffset;
uniform vec2 atlasScale;
uniform vec4 zNearFarInvNearFar;
uniform vec4 lightMapParams;

uniform float constantSpecularPower;
uniform vec2 fadeStartLength;
uniform vec4 farPlaneScalePSSM;
uniform vec4 splitFade;
uniform vec4 overDarkPSSM;
uniform float shadowSoftness;


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
   
   #ifdef NO_SHADOW

      // Fully unshadowed.
      float shadowed = 1.0;

   #else
   
      // Compute shadow map coordinate
      vec4 pxlPosLightProj = worldToLightProj * worldPos;
      vec2 baseShadowCoord = pxlPosLightProj.xy / pxlPosLightProj.w;   
   
      float distOffset = 0.0;
      float shadowed = 0.0;
      float fadeAmt = 0.0;
      vec4 zDist = vec4(zNearFarInvNearFar.x + zNearFarInvNearFar.y * depth);
                    
      // Calculate things dependant on the shadowmap split
      for ( int i = 0; i < 2; i++ )
      {
         float zDistSplit = zDist.x + distOffset;
         vec4 mask0;
         mask0.x = float(zDistSplit >= splitDistStart.x);
         mask0.y = float(zDistSplit >= splitDistStart.y);
         mask0.z = float(zDistSplit >= splitDistStart.z);
         mask0.w = float(zDistSplit >= splitDistStart.w);
         
         vec4 mask1;
         mask1.x = float(zDistSplit < splitDistEnd.x);
         mask1.y = float(zDistSplit < splitDistEnd.y);
         mask1.z = float(zDistSplit < splitDistEnd.z);
         mask1.w = float(zDistSplit < splitDistEnd.w);
         
         vec4 finalMask = mask0 * mask1;
         
         float splitFadeDist = dot( finalMask, splitFade );

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
         //shadowCoord.y = 1.0f - shadowCoord.y;

         // Move around inside of atlas 
         vec2 aOffset;
         aOffset.x = dot(finalMask, atlasXOffset);
         aOffset.y = dot(finalMask, atlasYOffset);

         shadowCoord *= atlasScale;
         shadowCoord += aOffset;
                    
         // Distance to light, in shadowmap space
         float distToLight = pxlPosLightProj.z / pxlPosLightProj.w;
         
         // Each split has a different far plane, take this into account.
         float farPlaneScale = dot( farPlaneScalePSSM, finalMask );
         distToLight *= farPlaneScale;
         
         #ifdef SOFTSHADOW_SM3

            float esmShadow = softShadow_filter(   ShadowMap,
                                                   gTapRotationTex,
                                                   uv0.xy,
                                                   shadowCoord,
                                                   farPlaneScale * shadowSoftness,
                                                   distToLight,
                                                   dotNL,
                                                   dot( finalMask, overDarkPSSM ) );
                                                   
         #else // !SOFTSHADOW_SM3

            float occluder = decodeShadowMap( texture2DLod( ShadowMap, shadowCoord, 0.0 ) );
            float overDark = dot( finalMask, overDarkPSSM );                      
            float esmShadow = saturate( exp( esmFactor * ( occluder - distToLight ) ) );

         #endif

         if ( i == 0 )
         {
            float endDist = dot(splitDistEnd, finalMask);
            fadeAmt = smoothstep(endDist - splitFadeDist, endDist, zDist).x;
            shadowed = esmShadow * ( 1.0 - fadeAmt );
         }
         else
            shadowed += esmShadow * fadeAmt;

         distOffset += splitFadeDist;
      }
  
      // Fade out the shadow at the end of the range.
      float fadeOutAmt = ( zDist.x - fadeStartLength.x ) * fadeStartLength.y;
      shadowed = mix( shadowed, 1.0, clamp( fadeOutAmt, 0.0, 1.0 ) );

   #endif // !NO_SHADOW
      
   // Calc lighting coefficents
   float specular = calcSpecular(   -lightDirection, 
                                    normal, 
                                    normalize(-wsEyeRay), 
                                    constantSpecularPower, 
                                    lightColor.a * lightBrightness );
   
   float Sat_NL_Att = clamp(dotNL, 0.0, 1.0) * shadowed;
   
   // Trilight, described by Tom Forsyth
   // http://home.comcast.net/~tom_forsyth/papers/trilight/trilight.html
#ifdef ACCUMULATE_LUV

   // In LUV multiply in the brightness of the light color (normaly done in the attenuate function)
   Sat_NL_Att *= lightColor.a;  
   
   vec4 ambientBlend = lightAmbient;
   ambientBlend.b *= clamp(-dotNL, 0.0, 1.0);
   
   vec3 trilight = lightTrilight.rgb;
   trilight.b *= clamp(1.0 - abs(dotNL), 0.0, 1.0);
   
   ambientBlend.rg = mix(ambientBlend.rg, trilight.rg, clamp(0.5 * trilight.b / lightAmbient.b, 0.0, 1.0));
   ambientBlend.b += trilight.b;

#else

   // RGB
   // TODO: Trilight seems broken... it does lighting in shadows!
   //vec4 ambientBlend = vec4(lightTrilight.rgb * clamp(1.0 - abs(dotNL), 0.0, 1.0) + lightAmbient.rgb * clamp(-dotNL, 0.0, 1.0), 0.0);
   vec4 ambientBlend = vec4(lightAmbient.rgb, 0.0);

#endif
   
   // Output
   gl_FragColor = lightinfoCondition( lightColor.rgb * lightBrightness, Sat_NL_Att, specular, ambientBlend) * lightMapParams;
}
