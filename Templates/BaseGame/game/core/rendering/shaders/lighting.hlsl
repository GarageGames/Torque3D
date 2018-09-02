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

#include "./torque.hlsl"

#ifndef TORQUE_SHADERGEN

// These are the uniforms used by most lighting shaders.

uniform float4 inLightPos[3];
uniform float4 inLightInvRadiusSq;
uniform float4 inLightColor[4];

#ifndef TORQUE_BL_NOSPOTLIGHT
   uniform float4 inLightSpotDir[3];
   uniform float4 inLightSpotAngle;
   uniform float4 inLightSpotFalloff;
#endif

uniform float4 ambient;
#define ambientCameraFactor 0.3
uniform float specularPower;
uniform float4 specularColor;

#endif // !TORQUE_SHADERGEN


void compute4Lights( float3 wsView, 
                     float3 wsPosition, 
                     float3 wsNormal,
                     float4 shadowMask,

                     #ifdef TORQUE_SHADERGEN
                     
                        float4 inLightPos[3],
                        float4 inLightInvRadiusSq,
                        float4 inLightColor[4],
                        float4 inLightSpotDir[3],
                        float4 inLightSpotAngle,
                        float4 inLightSpotFalloff,
                        float specularPower,
                        float4 specularColor,

                     #endif // TORQUE_SHADERGEN
                     
                     out float4 outDiffuse,
                     out float4 outSpecular )
{
   // NOTE: The light positions and spotlight directions
   // are stored in SoA order, so inLightPos[0] is the
   // x coord for all 4 lights... inLightPos[1] is y... etc.
   //
   // This is the key to fully utilizing the vector units and
   // saving a huge amount of instructions.
   //
   // For example this change saved more than 10 instructions 
   // over a simple for loop for each light.
   
   int i;

   float4 lightVectors[3];
   for ( i = 0; i < 3; i++ )
      lightVectors[i] = wsPosition[i] - inLightPos[i];

   float4 squareDists = 0;
   for ( i = 0; i < 3; i++ )
      squareDists += lightVectors[i] * lightVectors[i];

   // Accumulate the dot product between the light 
   // vector and the normal.
   //
   // The normal is negated because it faces away from
   // the surface and the light faces towards the
   // surface... this keeps us from needing to flip
   // the light vector direction which complicates
   // the spot light calculations.
   //
   // We normalize the result a little later.
   //
   float4 nDotL = 0;
   for ( i = 0; i < 3; i++ )
      nDotL += lightVectors[i] * -wsNormal[i];

   float4 rDotL = 0;
   #ifndef TORQUE_BL_NOSPECULAR

      // We're using the Phong specular reflection model
      // here where traditionally Torque has used Blinn-Phong
      // which has proven to be more accurate to real materials.
      //
      // We do so because its cheaper as do not need to 
      // calculate the half angle for all 4 lights.
      //   
      // Advanced Lighting still uses Blinn-Phong, but the
      // specular reconstruction it does looks fairly similar
      // to this.
      //
      float3 R = reflect( wsView, -wsNormal );

      for ( i = 0; i < 3; i++ )
         rDotL += lightVectors[i] * R[i];

   #endif
 
   // Normalize the dots.
   //
   // Notice we're using the half type here to get a
   // much faster sqrt via the rsq_pp instruction at 
   // the loss of some precision.
   //
   // Unless we have some extremely large point lights
   // i don't believe the precision loss will matter.
   //
   half4 correction = (half4)rsqrt( squareDists );
   nDotL = saturate( nDotL * correction );
   rDotL = clamp( rDotL * correction, 0.00001, 1.0 );

   // First calculate a simple point light linear 
   // attenuation factor.
   //
   // If this is a directional light the inverse
   // radius should be greater than the distance
   // causing the attenuation to have no affect.
   //
   float4 atten = saturate( 1.0 - ( squareDists * inLightInvRadiusSq ) );

   #ifndef TORQUE_BL_NOSPOTLIGHT

      // The spotlight attenuation factor.  This is really
      // fast for what it does... 6 instructions for 4 spots.

      float4 spotAtten = 0;
      for ( i = 0; i < 3; i++ )
         spotAtten += lightVectors[i] * inLightSpotDir[i];

      float4 cosAngle = ( spotAtten * correction ) - inLightSpotAngle;
      atten *= saturate( cosAngle * inLightSpotFalloff );

   #endif

   // Finally apply the shadow masking on the attenuation.
   atten *= shadowMask;

   // Get the final light intensity.
   float4 intensity = nDotL * atten;

   // Combine the light colors for output.
   outDiffuse = 0;
   for ( i = 0; i < 4; i++ )
      outDiffuse += intensity[i] * inLightColor[i];

   // Output the specular power.
   float4 specularIntensity = pow( rDotL, specularPower.xxxx ) * atten;
   
   // Apply the per-light specular attenuation.
   float4 specular = float4(0,0,0,1);
   for ( i = 0; i < 4; i++ )
      specular += float4( inLightColor[i].rgb * inLightColor[i].a * specularIntensity[i], 1 );

   // Add the final specular intensity values together
   // using a single dot product operation then get the
   // final specular lighting color.
   outSpecular = specularColor * specular;
}


// This value is used in AL as a constant power to raise specular values
// to, before storing them into the light info buffer. The per-material 
// specular value is then computer by using the integer identity of 
// exponentiation: 
//
//    (a^m)^n = a^(m*n)
//
//       or
//
//    (specular^constSpecular)^(matSpecular/constSpecular) = specular^(matSpecular*constSpecular)   
//
#define AL_ConstantSpecularPower 12.0f

/// The specular calculation used in Advanced Lighting.
///
///   @param toLight    Normalized vector representing direction from the pixel 
///                     being lit, to the light source, in world space.
///
///   @param normal  Normalized surface normal.
///   
///   @param toEye   The normalized vector representing direction from the pixel 
///                  being lit to the camera.
///
float AL_CalcSpecular( float3 toLight, float3 normal, float3 toEye )
{
   // (R.V)^c
   float specVal = dot( normalize( -reflect( toLight, normal ) ), toEye );

   // Return the specular factor.
   return pow( max( specVal, 0.00001f ), AL_ConstantSpecularPower );
}

/// The output for Deferred Lighting
///
///   @param toLight    Normalized vector representing direction from the pixel 
///                     being lit, to the light source, in world space.
///
///   @param normal  Normalized surface normal.
///   
///   @param toEye   The normalized vector representing direction from the pixel 
///                  being lit to the camera.
///
float4 AL_DeferredOutput(
      float3   lightColor,
      float3   diffuseColor,
      float4   matInfo,
      float4   ambient,
      float    specular,
      float    shadowAttenuation)
{
   float3 specularColor = float3(specular, specular, specular);
   bool metalness = getFlag(matInfo.r, 3);
   if ( metalness )
   {
       specularColor = 0.04 * (1 - specular) + diffuseColor * specular;
   }
   
   //specular = color * map * spec^gloss
   float specularOut = (specularColor * matInfo.b * min(pow(abs(specular), max(( matInfo.a/ AL_ConstantSpecularPower),1.0f)),matInfo.a)).r;
   
   lightColor *= shadowAttenuation;
   lightColor += ambient.rgb;
   return float4(lightColor.rgb, specularOut); 
}
