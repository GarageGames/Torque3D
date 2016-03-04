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
#include "softShadow.glsl"
#include "../../../gl/lighting.glsl"
#include "../../../gl/torque.glsl"

in vec4 wsEyeDir;
in vec4 ssPos;
in vec4 vsEyeDir;
in vec4 color;

#define IN_wsEyeDir wsEyeDir
#define IN_ssPos ssPos
#define IN_vsEyeDir vsEyeDir
#define IN_color color

#ifdef USE_COOKIE_TEX

/// The texture for cookie rendering.
uniform sampler2D cookieMap;

#endif

uniform sampler2D prePassBuffer;
uniform sampler2D shadowMap;
uniform sampler2D dynamicShadowMap;

uniform sampler2D lightBuffer;
uniform sampler2D colorBuffer;
uniform sampler2D matInfoBuffer;

uniform vec4 rtParams0;

uniform vec3 lightPosition;
uniform vec4 lightColor;
uniform float  lightBrightness;
uniform float  lightRange;
uniform vec2 lightAttenuation;
uniform vec3 lightDirection;
uniform vec4 lightSpotParams;
uniform vec4 lightMapParams;

uniform vec4 vsFarPlane;
uniform mat4 viewToLightProj;
uniform mat4 dynamicViewToLightProj;

uniform vec4 lightParams;
uniform float shadowSoftness;

out vec4 OUT_col;

void main()
{   
   // Compute scene UV
   vec3 ssPos = IN_ssPos.xyz / IN_ssPos.w;
   vec2 uvScene = getUVFromSSPos( ssPos, rtParams0 );

   // Emissive.
   vec4 matInfo = texture( matInfoBuffer, uvScene );   
   bool emissive = getFlag( matInfo.r, 0 );
   if ( emissive )
   {
       OUT_col = vec4(0.0, 0.0, 0.0, 0.0);
	   return;
   }
   
   // Sample/unpack the normal/z data
   vec4 prepassSample = prepassUncondition( prePassBuffer, uvScene );
   vec3 normal = prepassSample.rgb;
   float depth = prepassSample.a;
   
   // Eye ray - Eye -> Pixel
   vec3 eyeRay = getDistanceVectorToPlane( -vsFarPlane.w, IN_vsEyeDir.xyz, vsFarPlane );
   vec3 viewSpacePos = eyeRay * depth;
      
   // Build light vec, get length, clip pixel if needed
   vec3 lightToPxlVec = viewSpacePos - lightPosition;
   float lenLightV = length( lightToPxlVec );
   lightToPxlVec /= lenLightV;

   //lightDirection = vec3( -lightDirection.xy, lightDirection.z ); //vec3( 0, 0, -1 );
   float cosAlpha = dot( lightDirection, lightToPxlVec );   
   clip( cosAlpha - lightSpotParams.x );
   clip( lightRange - lenLightV );

   float atten = attenuate( lightColor, lightAttenuation, lenLightV );
   atten *= ( cosAlpha - lightSpotParams.x ) / lightSpotParams.y;
   clip( atten - 1e-6 );
   atten = saturate( atten );
   
   float nDotL = dot( normal, -lightToPxlVec );

   // Get the shadow texture coordinate
   vec4 pxlPosLightProj = tMul( viewToLightProj, vec4( viewSpacePos, 1 ) );
   vec2 shadowCoord = ( ( pxlPosLightProj.xy / pxlPosLightProj.w ) * 0.5 ) + vec2( 0.5, 0.5 );
   shadowCoord.y = 1.0f - shadowCoord.y;

   // Get the dynamic shadow texture coordinate
   vec4 dynpxlPosLightProj = tMul( dynamicViewToLightProj, vec4( viewSpacePos, 1 ) );
   vec2 dynshadowCoord = ( ( dynpxlPosLightProj.xy / dynpxlPosLightProj.w ) * 0.5 ) + vec2( 0.5, 0.5 );
   dynshadowCoord.y = 1.0f - dynshadowCoord.y;
   #ifdef NO_SHADOW
   
      float shadowed = 1.0;
      	
   #else

      // Get a linear depth from the light source.
      float distToLight = pxlPosLightProj.z / lightRange;

      float static_shadowed = softShadow_filter( shadowMap,
                                          ssPos.xy,
                                          shadowCoord,
                                          shadowSoftness,
                                          distToLight,
                                          nDotL,
                                          lightParams.y );

      float dynamic_shadowed = softShadow_filter( dynamicShadowMap,
                                          ssPos.xy,
                                          dynshadowCoord,
                                          shadowSoftness,
                                          distToLight,
                                          nDotL,
                                          lightParams.y );
      float shadowed = min(static_shadowed, dynamic_shadowed);
   #endif // !NO_SHADOW
   
   vec3 lightcol = lightColor.rgb;
   #ifdef USE_COOKIE_TEX

      // Lookup the cookie sample.
      vec4 cookie = texture( cookieMap, shadowCoord );

      // Multiply the light with the cookie tex.
      lightcol *= cookie.rgb;

      // Use a maximum channel luminance to attenuate 
      // the lighting else we get specular in the dark
      // regions of the cookie texture.
      atten *= max( cookie.r, max( cookie.g, cookie.b ) );

   #endif

   // NOTE: Do not clip on fully shadowed pixels as it would
   // cause the hardware occlusion query to disable the shadow.

   // Specular term
   float specular = AL_CalcSpecular(   -lightToPxlVec, 
                                       normal, 
                                       normalize( -eyeRay ) ) * lightBrightness * atten * shadowed;

   float Sat_NL_Att = saturate( nDotL * atten * shadowed ) * lightBrightness;
   vec3 lightColorOut = lightMapParams.rgb * lightcol;
   vec4 addToResult = vec4(0.0);

   // TODO: This needs to be removed when lightmapping is disabled
   // as its extra work per-pixel on dynamic lit scenes.
   //
   // Special lightmapping pass.
   if ( lightMapParams.a < 0.0 )
   {
      // This disables shadows on the backsides of objects.
      shadowed = nDotL < 0.0f ? 1.0f : shadowed;

      Sat_NL_Att = 1.0f;
      shadowed = mix( 1.0f, shadowed, atten );
      lightColorOut = vec3(shadowed);
      specular *= lightBrightness;
      addToResult = ( 1.0 - shadowed ) * abs(lightMapParams);
   }

   vec4 colorSample = texture( colorBuffer, uvScene );
   OUT_col = AL_DeferredOutput(lightColorOut, colorSample.rgb, matInfo, addToResult, specular, Sat_NL_Att);
}
