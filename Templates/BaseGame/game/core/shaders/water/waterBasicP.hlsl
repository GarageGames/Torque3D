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

#include "../torque.hlsl"

//-----------------------------------------------------------------------------
// Defines                                                                  
//-----------------------------------------------------------------------------

// miscParams
#define FRESNEL_BIAS       miscParams[0]
#define FRESNEL_POWER      miscParams[1]
#define CLARITY            miscParams[2]
#define ISRIVER            miscParams[3]

// reflectParams
#define REFLECT_PLANE_Z    reflectParams[0]
#define REFLECT_MIN_DIST   reflectParams[1]
#define REFLECT_MAX_DIST   reflectParams[2]
#define NO_REFLECT         reflectParams[3]

// distortionParams
#define DISTORT_START_DIST distortionParams[0]
#define DISTORT_END_DIST   distortionParams[1]
#define DISTORT_FULL_DEPTH distortionParams[2]

// ConnectData.misc
#define LIGHT_VEC IN.misc.xyz
#define WORLD_Z   IN.objPos.w

// specularParams
#define SPEC_POWER         specularParams[3]
#define SPEC_COLOR         specularParams.xyz

//-----------------------------------------------------------------------------
// Structures                                                                  
//-----------------------------------------------------------------------------

struct ConnectData
{
   float4 hpos             : TORQUE_POSITION;   
   
   // TexCoord 0 and 1 (xy,zw) for ripple texture lookup
   float4 rippleTexCoord01 : TEXCOORD0;   
   
   // TexCoord 2 for ripple texture lookup
   float2 rippleTexCoord2  : TEXCOORD1;
   
   // Screenspace vert position BEFORE wave transformation
   float4 posPreWave       : TEXCOORD2;
   
   // Screenspace vert position AFTER wave transformation
   float4 posPostWave      : TEXCOORD3;   
   
   // Worldspace unit distance/depth of this vertex/pixel
   float  pixelDist        : TEXCOORD4;   
   
   // Objectspace vert position BEFORE wave transformation	
   // w coord is world space z position.
   float4 objPos           : TEXCOORD5; 
   
   float3 misc             : TEXCOORD6;   
};

//-----------------------------------------------------------------------------
// approximate Fresnel function
//-----------------------------------------------------------------------------
float fresnel(float NdotV, float bias, float power)
{
   return bias + (1.0-bias)*pow(abs(1.0 - max(NdotV, 0)), power);
}

//-----------------------------------------------------------------------------
// Uniforms                                                                  
//-----------------------------------------------------------------------------
TORQUE_UNIFORM_SAMPLER2D(bumpMap,0);
//uniform sampler2D    prepassTex  : register( S1 );
TORQUE_UNIFORM_SAMPLER2D(reflectMap,2);
TORQUE_UNIFORM_SAMPLER2D(refractBuff,3);
TORQUE_UNIFORM_SAMPLERCUBE(skyMap,4);
//uniform sampler      foamMap     : register( S5 );
uniform float4       baseColor;
uniform float4       miscParams;
uniform float4       reflectParams;
uniform float3       ambientColor;
uniform float3       eyePos;
uniform float3       distortionParams;
uniform float3       fogData;
uniform float4       fogColor;
uniform float4       rippleMagnitude;
uniform float4       specularParams;
uniform float4x4     modelMat;

//-----------------------------------------------------------------------------
// Main                                                                        
//-----------------------------------------------------------------------------
float4 main( ConnectData IN ) : TORQUE_TARGET0
{ 
   // Modulate baseColor by the ambientColor.
   float4 waterBaseColor = baseColor * float4( ambientColor.rgb, 1 );
   waterBaseColor = toLinear(waterBaseColor);
   
   // Get the bumpNorm...
   float3 bumpNorm = ( TORQUE_TEX2D( bumpMap, IN.rippleTexCoord01.xy ).rgb * 2.0 - 1.0 ) * rippleMagnitude.x;
   bumpNorm       += ( TORQUE_TEX2D( bumpMap, IN.rippleTexCoord01.zw ).rgb * 2.0 - 1.0 ) * rippleMagnitude.y;      
   bumpNorm       += ( TORQUE_TEX2D( bumpMap, IN.rippleTexCoord2 ).rgb * 2.0 - 1.0 ) * rippleMagnitude.z;  
   
   bumpNorm = normalize( bumpNorm );
   bumpNorm = lerp( bumpNorm, float3(0,0,1), 1.0 - rippleMagnitude.w );
   
   // We subtract a little from it so that we don't 
   // distort where the water surface intersects the
   // camera near plane.
   float distortAmt = saturate( IN.pixelDist / 1.0 ) * 0.8;
      
   float4 distortPos = IN.posPostWave;
   distortPos.xy += bumpNorm.xy * distortAmt;   
 
 #ifdef UNDERWATER
   return hdrEncode( TORQUE_TEX2DPROJ( refractBuff, distortPos ) );   
 #else

   float3 eyeVec = IN.objPos.xyz - eyePos;
   eyeVec = mul( (float3x3)modelMat, eyeVec );
   float3 reflectionVec = reflect( eyeVec, bumpNorm ); 

   // Color that replaces the reflection color when we do not
   // have one that is appropriate.
   float4 fakeColor = float4(ambientColor,1);
   
   // Use fakeColor for ripple-normals that are angled towards the camera  
   eyeVec = -eyeVec;
   eyeVec = normalize( eyeVec );
   float ang = saturate( dot( eyeVec, bumpNorm ) );   
   float fakeColorAmt = ang;   
      
    // Get reflection map color
   float4 refMapColor = hdrDecode( TORQUE_TEX2DPROJ( reflectMap, distortPos ) ); 
   // If we do not have a reflection texture then we use the cubemap.
   refMapColor = lerp( refMapColor, TORQUE_TEXCUBE( skyMap, reflectionVec ), NO_REFLECT );      
   
   // Combine reflection color and fakeColor.
   float4 reflectColor = lerp( refMapColor, fakeColor, fakeColorAmt );
   //return refMapColor;
   
   // Get refract color
   float4 refractColor = hdrDecode( TORQUE_TEX2DPROJ( refractBuff, distortPos ) );   
   
   // calc "diffuse" color by lerping from the water color
   // to refraction image based on the water clarity.
   float4 diffuseColor = lerp( refractColor, waterBaseColor, 1.0f - CLARITY );   
   
   // fresnel calculation 
   float fresnelTerm = fresnel( ang, FRESNEL_BIAS, FRESNEL_POWER );	
   //return float4( fresnelTerm.rrr, 1 );
   
   // Also scale the frensel by our distance to the
   // water surface.  This removes the hard reflection
   // when really close to the water surface.
   fresnelTerm *= saturate( IN.pixelDist - 0.1 );
   
   // Combine the diffuse color and reflection image via the
   // fresnel term and set out output color.
   float4 OUT = lerp( diffuseColor, reflectColor, fresnelTerm );  
   
   #ifdef WATER_SPEC

      // Get some specular reflection.
      float3 newbump = bumpNorm;
      newbump.xy *= 3.5;
      newbump = normalize( bumpNorm );
      half3 halfAng = normalize( eyeVec + -LIGHT_VEC );
      float specular = saturate( dot( newbump, halfAng ) );
      specular = pow( specular, SPEC_POWER );   
      
      OUT.rgb = OUT.rgb + ( SPEC_COLOR * specular.xxx );  

   #else // Disable fogging if spec is on because otherwise we run out of instructions.
   
      // Fog it.   
      float factor = computeSceneFog( eyePos, 
                                      IN.objPos.xyz, 
                                      WORLD_Z,
                                      fogData.x,
                                      fogData.y,
                                      fogData.z );

      //OUT.rgb = lerp( OUT.rgb, fogColor.rgb, 1.0 - saturate( factor ) );   

   #endif
   
   return hdrEncode( OUT );
   
#endif   
}
