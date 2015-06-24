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
#include "lightingUtils.glsl"
#include "../../../gl/lighting.glsl"
#include "../../shadowMap/shadowMapIO_GLSL.h"
#include "softShadow.glsl"

in vec4 wsEyeDir;
in vec4 ssPos;
in vec4 vsEyeDir;

#ifdef USE_COOKIE_TEX

/// The texture for cookie rendering.
uniform samplerCube cookieMap ;

#endif


#ifdef SHADOW_CUBE

   vec3 decodeShadowCoord( vec3 shadowCoord )
   {
      return shadowCoord;
   }

   vec4 shadowSample( samplerCube shadowMap, vec3 shadowCoord )
   {
      return texture( shadowMap, shadowCoord );
   }
  
#else

   vec3 decodeShadowCoord( vec3 paraVec )
   {
      // Flip y and z
      paraVec = paraVec.xzy;
      
      #ifndef SHADOW_PARABOLOID

         bool calcBack = (paraVec.z < 0.0);
         if ( calcBack )
         {
            paraVec.z = paraVec.z * -1.0;
            
            #ifdef SHADOW_DUALPARABOLOID
               paraVec.x = -paraVec.x;
            #endif
         }

      #endif

      vec3 shadowCoord;
      shadowCoord.x = (paraVec.x / (2*(1 + paraVec.z))) + 0.5;
      shadowCoord.y = 1-((paraVec.y / (2*(1 + paraVec.z))) + 0.5);
      shadowCoord.z = 0;
      
      // adjust the co-ordinate slightly if it is near the extent of the paraboloid
      // this value was found via experementation
      // NOTE: this is wrong, it only biases in one direction, not towards the uv 
      // center ( 0.5 0.5 ).
      //shadowCoord.xy *= 0.997;

      #ifndef SHADOW_PARABOLOID

         // If this is the back, offset in the atlas
         if ( calcBack )
            shadowCoord.x += 1.0;
         
         // Atlasing front and back maps, so scale
         shadowCoord.x *= 0.5;

      #endif

      return shadowCoord;
   }

#endif

uniform sampler2D prePassBuffer;

#ifdef SHADOW_CUBE
	uniform samplerCube shadowMap;
#else
	uniform sampler2D shadowMap;
#endif

uniform vec4 rtParams0;

uniform vec3 lightPosition;
uniform vec4 lightColor;
uniform float  lightBrightness;
uniform float  lightRange;
uniform vec2 lightAttenuation;
uniform vec4 lightMapParams;
uniform vec4 vsFarPlane;
uniform mat3 viewToLightProj;
uniform vec4 lightParams;
uniform float shadowSoftness;

out vec4 OUT_col;

void main()               
{   
   // Compute scene UV
   vec3 ssPos = ssPos.xyz / ssPos.w;
   vec2 uvScene = getUVFromSSPos( ssPos, rtParams0 );
   
   // Sample/unpack the normal/z data
   vec4 prepassSample = prepassUncondition( prePassBuffer, uvScene );
   vec3 normal = prepassSample.rgb;
   float depth = prepassSample.a;
   
   // Eye ray - Eye -> Pixel
   vec3 eyeRay = getDistanceVectorToPlane( -vsFarPlane.w, vsEyeDir.xyz, vsFarPlane );
   vec3 viewSpacePos = eyeRay * depth;
      
   // Build light vec, get length, clip pixel if needed
   vec3 lightVec = lightPosition - viewSpacePos;
   float lenLightV = length( lightVec );
   clip( lightRange - lenLightV );

   // Get the attenuated falloff.
   float atten = attenuate( lightColor, lightAttenuation, lenLightV );
   clip( atten - 1e-6 );

   // Normalize lightVec
   lightVec /= lenLightV;
   
   // If we can do dynamic branching then avoid wasting
   // fillrate on pixels that are backfacing to the light.
   float nDotL = dot( lightVec, normal );
   //DB_CLIP( nDotL < 0 );

   #ifdef NO_SHADOW
   
      float shadowed = 1.0;
      	
   #else

      // Get a linear depth from the light source.
      float distToLight = lenLightV / lightRange;      

      #ifdef SHADOW_CUBE
              
         // TODO: We need to fix shadow cube to handle soft shadows!
         float occ = texture( shadowMap, tMul( viewToLightProj, -lightVec ) ).r;
         float shadowed = saturate( exp( lightParams.y * ( occ - distToLight ) ) );
         
      #else

         vec2 shadowCoord = decodeShadowCoord( tMul( viewToLightProj, -lightVec ) ).xy;
         
         float shadowed = softShadow_filter( shadowMap,
                                             ssPos.xy,
                                             shadowCoord,
                                             shadowSoftness,
                                             distToLight,
                                             nDotL,
                                             lightParams.y );

      #endif

   #endif // !NO_SHADOW
   
   vec3 lightcol = lightColor.rgb;
   #ifdef USE_COOKIE_TEX

      // Lookup the cookie sample.
      vec4 cookie = texture( cookieMap, tMul( viewToLightProj, -lightVec ) );

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
   float specular = AL_CalcSpecular(   lightVec, 
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

   OUT_col = lightinfoCondition( lightColorOut, Sat_NL_Att, specular, addToResult );
}
