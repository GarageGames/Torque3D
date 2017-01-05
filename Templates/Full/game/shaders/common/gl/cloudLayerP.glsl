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

#include "hlslCompat.glsl"
#include "torque.glsl"
//-----------------------------------------------------------------------------
// Structures                                                                  
//-----------------------------------------------------------------------------
//ConnectData
in vec4 texCoord12;
#define IN_texCoord12 texCoord12
in vec4 texCoord34;
#define IN_texCoord34 texCoord34
in vec3 vLightTS; // light vector in tangent space, denormalized
#define IN_vLightTS vLightTS
in vec3 vViewTS;  // view vector in tangent space, denormalized
#define IN_vViewTS vViewTS
in float worldDist;
#define IN_worldDist worldDist

//-----------------------------------------------------------------------------
// Uniforms                                                                        
//-----------------------------------------------------------------------------
uniform sampler2D normalHeightMap;
uniform vec3      ambientColor;
uniform vec3      sunColor;
uniform float     cloudCoverage;
uniform vec3      cloudBaseColor;
uniform float	  cloudExposure;

out vec4 OUT_col;

//-----------------------------------------------------------------------------
// Globals                                                                        
//-----------------------------------------------------------------------------
// The per-color weighting to be used for luminance calculations in RGB order.
const vec3 LUMINANCE_VECTOR  = vec3(0.2125f, 0.7154f, 0.0721f);


//-----------------------------------------------------------------------------
// Functions                                                                        
//-----------------------------------------------------------------------------

// Calculates the Rayleigh phase function
float getRayleighPhase( float angle )
{
   return 0.75 * ( 1.0 + pow( angle, 2.0 ) );	
}

// Returns the output rgb color given a texCoord and parameters it uses
// for lighting calculation.
vec3 ComputeIllumination( vec2 texCoord, 
                            vec3 vLightTS, 
                            vec3 vViewTS, 
                            vec3 vNormalTS )
{   
   //return noiseNormal;
   //return vNormalTS;
   
   vec3 vLightTSAdj = vec3( -vLightTS.x, -vLightTS.y, vLightTS.z );
   
   float dp = dot( vNormalTS, vLightTSAdj );
   
   // Calculate the amount of illumination (lightTerm)...      
   
   // We do both a rim lighting effect and a halfLambertian lighting effect
   // and combine the result.
   float halfLambertTerm = clamp( pow( dp * 0.5 + 0.5, 1.0 ), 0.0, 1.0 );
   float rimLightTerm = pow( ( 1.0 - dp ), 1.0 );   
   float lightTerm = clamp( halfLambertTerm * 1.0 + rimLightTerm * dp, 0.0, 1.0 );
   lightTerm *= 0.5;
   
   // Use a simple RayleighPhase function to simulate single scattering towards
   // the camera.
   float angle = dot( vLightTS, vViewTS );
   lightTerm *= getRayleighPhase( angle );
   
   // Combine terms and colors into the output color.   
   //vec3 lightColor = ( lightTerm * sunColor * fOcclusionShadow ) + ambientColor;   
   vec3 lightColor = mix( ambientColor, sunColor, lightTerm );
   //lightColor = mix( lightColor, ambientColor, cloudCoverage );
   vec3 finalColor = cloudBaseColor * lightColor;
   
   return finalColor;
}   

void main()
{ 
   //  Normalize the interpolated vectors:
   vec3 vViewTS   = normalize( vViewTS  );
   vec3 vLightTS  = normalize( vLightTS );
     
   vec4 cResultColor = vec4( 0, 0, 0, 1 );
    
   vec2 texSample = IN_texCoord12.xy;
   
   vec4 noise1 = texture( normalHeightMap, IN_texCoord12.zw );
   noise1 = normalize( ( noise1 - 0.5 ) * 2.0 );   
   //return noise1;
   
   vec4 noise2 = texture( normalHeightMap, IN_texCoord34.xy );
   noise2 = normalize( ( noise2 - 0.5 ) * 2.0 );
   //return noise2;
      
   vec3 noiseNormal = normalize( noise1 + noise2 ).xyz;
   //return vec4( noiseNormal, 1.0 );
   
   float noiseHeight = noise1.a * noise2.a * ( cloudCoverage / 2.0 + 0.5 );         

   vec3 vNormalTS = normalize( texture( normalHeightMap, texSample ).xyz * 2.0 - 1.0 );   
   vNormalTS += noiseNormal;
   vNormalTS = normalize( vNormalTS );   
   
   // Compute resulting color for the pixel:
   cResultColor.rgb = ComputeIllumination( texSample, vLightTS, vViewTS, vNormalTS );
      
   float coverage = ( cloudCoverage - 0.5 ) * 2.0;
   cResultColor.a = texture( normalHeightMap, texSample ).a + coverage + noiseHeight;     
   
   if ( cloudCoverage > -1.0 )
      cResultColor.a /= 1.0 + coverage;
        
   cResultColor.a = clamp( cResultColor.a * pow( saturate(cloudCoverage), 0.25 ), 0.0, 1.0 );

   cResultColor.a = mix( cResultColor.a, 0.0, 1.0 - pow(IN_worldDist,2.0) );

   OUT_col = hdrEncode(cResultColor);
}   
