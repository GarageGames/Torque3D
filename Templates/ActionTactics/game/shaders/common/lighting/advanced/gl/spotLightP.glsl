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
#include "farFrustumQuad.glsl"
#include "lightingUtils.glsl"
#include "../../shadowMap/shadowMapIO_GLSL.h"
#include "shadergen:/autogenConditioners.h"


#if TORQUE_SM >= 30

   // Enables high quality soft shadow 
   // filtering for SM3.0 and above.
   #define SOFTSHADOW_SM3

   #include "softShadow.glsl"

#else


#endif


varying vec4 ssPos;
varying vec4 wsEyeDir;


uniform sampler2D prePassBuffer;
uniform sampler2D shadowMap;
#ifdef ACCUMULATE_LUV
   uniform sampler2D scratchTarget;
#endif

uniform vec4 renderTargetParams;

uniform vec3 lightPosition;
uniform vec4 lightColor;
uniform float lightBrightness;
uniform float lightRange;
uniform vec2 lightAttenuation;
uniform vec3 lightDirection;
uniform vec4 lightSpotParams;
uniform vec4 lightMapParams;

uniform vec3 eyePosWorld;
uniform vec4 farPlane;
uniform float negFarPlaneDotEye;
uniform mat4x4 worldToLightProj;

uniform vec4 lightParams;
uniform float shadowSoftness;
uniform float constantSpecularPower;


void main()
{
   // Compute scene UV
   vec3 ssPosP = ssPos.xyz / ssPos.w;
   vec2 uvScene = getUVFromSSPos( ssPosP, renderTargetParams );
   
   // Sample/unpack the normal/z data
   vec4 prepassSample = prepassUncondition( prePassBuffer, uvScene );
   vec3 normal = prepassSample.rgb;
   float depth = prepassSample.a;
   
   // Eye ray - Eye -> Pixel
   vec3 eyeRay = getDistanceVectorToPlane( negFarPlaneDotEye, wsEyeDir.xyz / wsEyeDir.w , farPlane );
      
   // Get world space pixel position
   vec3 worldPos = eyePosWorld + eyeRay * depth;
      
   // Build light vec, get length, clip pixel if needed
   vec3 lightToPxlVec = worldPos - lightPosition;
   float lenLightV = length( lightToPxlVec );
   lightToPxlVec /= lenLightV;

   //lightDirection = float3( -lightDirection.xy, lightDirection.z ); //float3( 0, 0, -1 );
   float cosAlpha = dot( lightDirection, lightToPxlVec );   
   if ( cosAlpha - lightSpotParams.x < 0.0 ) discard;
   if ( lightRange - lenLightV < 0.0 ) discard;

   float atten = attenuate( lightColor, lightAttenuation, lenLightV );
   atten *= ( cosAlpha - lightSpotParams.x ) / lightSpotParams.y;
   if ( atten - 1e-6 < 0.0 ) discard;
   
   float nDotL = dot( normal, -lightToPxlVec );

   #ifdef NO_SHADOW
   
      float shadowed = 1.0;
      	
   #else

      // Find Shadow coordinate
      vec4 pxlPosLightProj = vec4( worldToLightProj * vec4( worldPos, 1.0 ) );
      vec2 shadowCoord = ( ( pxlPosLightProj.xy / pxlPosLightProj.w ) * 0.5 ) + vec2( 0.5, 0.5 );

      // Get a linear depth from the light source.
      float distToLight = pxlPosLightProj.z / lightRange;

      #ifdef SOFTSHADOW_SM3

         float shadowed = softShadow_filter( shadowMap,
                                             gTapRotationTex,
                                             ssPosP.xy,
                                             shadowCoord,
                                             shadowSoftness,
                                             distToLight,
                                             nDotL,
                                             lightParams.y );
                                             
      #else // !SOFTSHADOW_SM3

         // Simple exponential shadow map.
         float occluder = decodeShadowMap( texture2DLod( shadowMap, shadowCoord, 0.0 ) );
         float esmFactor = lightParams.y;
         float shadowed = clamp( exp( esmFactor * ( occluder - distToLight ) ), 0.0, 1.0 );

      #endif

   #endif // !NO_SHADOW
      
   // NOTE: Do not clip on fully shadowed pixels as it would
   // cause the hardware occlusion query to disable the shadow.

   // Specular term
   float specular = calcSpecular(   -lightToPxlVec, 
                                    normal, 
                                    normalize( -eyeRay ), 
                                    constantSpecularPower, 
                                    lightColor.a * lightBrightness );
    
   // N.L * Attenuation
   float Sat_NL_Att = clamp( nDotL * atten * shadowed, 0.0, 1.0 );
   
   // In LUV color mode we need to blend in the 
   // output from the previous target.
   vec4 previousPix = vec4(0.0);
	#ifdef ACCUMULATE_LUV
      previousPix = texture2DLod( scratchTarget, uvScene, 0.0 );
   #endif

   // Output
   gl_FragColor = lightinfoCondition(  lightColor.rgb * lightBrightness, 
                                       Sat_NL_Att, 
                                       specular, 
                                       previousPix ) * lightMapParams;
}
