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
#include "lightingUtils.hlsl"
#include "../../lighting.hlsl"
#include "../shadowMap/shadowMapIO_HLSL.h"
#include "softShadow.hlsl"


struct ConvexConnectP
{
   float4 wsEyeDir : TEXCOORD0;
   float4 ssPos : TEXCOORD1;
   float4 vsEyeDir : TEXCOORD2;
};


#ifdef USE_COOKIE_TEX

/// The texture for cookie rendering.
uniform samplerCUBE cookieMap : register(S2);

#endif


#ifdef SHADOW_CUBE

   float3 decodeShadowCoord( float3 shadowCoord )
   {
      return shadowCoord;
   }

   float4 shadowSample( samplerCUBE shadowMap, float3 shadowCoord )
   {
      return texCUBE( shadowMap, shadowCoord );
   }
  
#else

   float3 decodeShadowCoord( float3 paraVec )
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

      float3 shadowCoord;
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


float4 main(   ConvexConnectP IN,

               uniform sampler2D prePassBuffer : register(S0),

               #ifdef SHADOW_CUBE
                  uniform samplerCUBE shadowMap : register(S1),
               #else
                  uniform sampler2D shadowMap : register(S1),
               #endif

               uniform float4 rtParams0,

               uniform float3 lightPosition,
               uniform float4 lightColor,
               uniform float  lightBrightness,
               uniform float  lightRange,
               uniform float2 lightAttenuation,
               uniform float4 lightMapParams,

               uniform float4 vsFarPlane,
               uniform float3x3 viewToLightProj,

               uniform float4 lightParams,
               uniform float shadowSoftness ) : COLOR0
{   
   // Compute scene UV
   float3 ssPos = IN.ssPos.xyz / IN.ssPos.w;
   float2 uvScene = getUVFromSSPos( ssPos, rtParams0 );
   
   // Sample/unpack the normal/z data
   float4 prepassSample = prepassUncondition( prePassBuffer, uvScene );
   float3 normal = prepassSample.rgb;
   float depth = prepassSample.a;
   
   // Eye ray - Eye -> Pixel
   float3 eyeRay = getDistanceVectorToPlane( -vsFarPlane.w, IN.vsEyeDir.xyz, vsFarPlane );
   float3 viewSpacePos = eyeRay * depth;
      
   // Build light vec, get length, clip pixel if needed
   float3 lightVec = lightPosition - viewSpacePos;
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
         float occ = texCUBE( shadowMap, mul( viewToLightProj, -lightVec ) ).r;
         float shadowed = saturate( exp( lightParams.y * ( occ - distToLight ) ) );
         
      #else

         float2 shadowCoord = decodeShadowCoord( mul( viewToLightProj, -lightVec ) ).xy;
         
         float shadowed = softShadow_filter( shadowMap,
                                             ssPos.xy,
                                             shadowCoord,
                                             shadowSoftness,
                                             distToLight,
                                             nDotL,
                                             lightParams.y );

      #endif

   #endif // !NO_SHADOW
   
   #ifdef USE_COOKIE_TEX

      // Lookup the cookie sample.
      float4 cookie = texCUBE( cookieMap, mul( viewToLightProj, -lightVec ) );

      // Multiply the light with the cookie tex.
      lightColor.rgb *= cookie.rgb;

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
                                       normalize( -eyeRay ) ) * lightColor.a;

   float Sat_NL_Att = saturate( nDotL * atten * shadowed ) * lightBrightness;
   float3 lightColorOut = lightMapParams.rgb * lightColor.rgb;
   float4 addToResult = 0.0;
    
   // TODO: This needs to be removed when lightmapping is disabled
   // as its extra work per-pixel on dynamic lit scenes.
   //
   // Special lightmapping pass.
   if ( lightMapParams.a < 0.0 )
   {
      // This disables shadows on the backsides of objects.
      shadowed = nDotL < 0.0f ? 1.0f : shadowed;

      Sat_NL_Att = 1.0f;
      shadowed = lerp( 1.0f, shadowed, atten );
      lightColorOut = shadowed;
      specular *= lightBrightness;
      addToResult = ( 1.0 - shadowed ) * abs(lightMapParams);
   }

   return lightinfoCondition( lightColorOut, Sat_NL_Att, specular, addToResult );
}
