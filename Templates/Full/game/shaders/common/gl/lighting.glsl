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

// These are the uniforms used by most lighting shaders.

uniform vec3 inLightPos[4];
uniform vec4 inLightInvRadiusSq;
uniform vec4 inLightColor[4];
uniform vec4 ambient;
uniform float specularPower;
uniform vec4 specularColor;


// This is used to limit the maximum processed
// lights in the compute4Lights down for really
// low end GPUs.
//
// NOTE: If you want to support 10.5.x, this needs to be changed to 2.
#define C4L_MAX_LIGHTS 4


void compute4Lights( vec3 wsView, 
                     vec3 wsPosition, 
                     vec3 wsNormal, 
                     out vec4 outDiffuse,
                     out vec4 outSpecular )
{
   #ifdef PHONG_SPECULAR 
      // (R.V)^c
      float reflected = reflect( wsView, wsNormal );
   #endif

   vec4 nDotL = vec4( 0.0 );
   vec4 rDotL = vec4( 0.0 );
   vec4 sqDists = vec4( 0.0 );
   int i;

   for ( i = 0; i < C4L_MAX_LIGHTS; ++i )
   {
      vec3 lightVector = inLightPos[i] - wsPosition;
      vec3 lightDirection = normalize( lightVector );

      nDotL[i] = max( dot( lightDirection, wsNormal ), 0.0 );

      #ifdef PHONG_SPECULAR
         rDotL[i] = saturate( dot( lightDirection, reflected ) );
      #else
         // (N.H)^c [Blinn-Phong, TGEA style, default]
         rDotL[i] = dot( wsNormal, normalize( lightDirection + wsView ) );
      #endif

      sqDists[i] = dot( lightVector, lightVector );
   }

   // Attenuation
   vec4 atten = vec4( 1.0 ) - ( sqDists * inLightInvRadiusSq );

   // Combine the light colors for output.
   vec4 diffuse = clamp( nDotL * atten, vec4( 0.0 ), vec4( 1.0 ) );
   outDiffuse = vec4( 0.0 );
   for ( i = 0; i < C4L_MAX_LIGHTS; ++i )
      outDiffuse += vec4( diffuse[i] ) * inLightColor[i];

   // Output the specular power.
   rDotL = max( rDotL, vec4( 0.00001 ) );
   outSpecular = pow( rDotL, vec4( specularPower ) );
}


/// The standard specular calculation.
///
///   @param toLight    Normalized vector representing direction from the pixel 
///                     being lit, to the light source, in world space.
///
///   @param normal  Normalized surface normal.
///   
///   @param toEye   The normalized vector representing direction from the pixel 
///                  being lit to the camera.
///
///   @param specPwr    The specular exponent.
///
///   @param specScale  A scalar on the specular output used in RGB accumulation.
///
float calcSpecular( vec3 toLight, vec3 normal, vec3 toEye, float specPwr )
{
   #ifdef PHONG_SPECULAR 
      // (R.V)^c
      float specVal = dot( normalize( -reflect( toLight, normal ) ), toEye );
   #else
      // (N.H)^c [Blinn-Phong, TGEA style, default]
      float specVal = dot( normal, normalize( toLight + toEye ) );
   #endif

   // Return the specular factor.
   return pow( max( specVal, 0.00001f ), specPwr );
}
