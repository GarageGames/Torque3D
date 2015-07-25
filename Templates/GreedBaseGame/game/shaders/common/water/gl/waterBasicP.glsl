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

#include "../../gl/torque.glsl"
#include "../../gl/hlslCompat.glsl"

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
#define LIGHT_VEC IN_misc.xyz
#define WORLD_Z   IN_objPos.w

// specularParams
#define SPEC_POWER         specularParams[3]
#define SPEC_COLOR         specularParams.xyz

//-----------------------------------------------------------------------------
// Defines                                                                  
//-----------------------------------------------------------------------------

// TexCoord 0 and 1 (xy,zw) for ripple texture lookup
in vec4 rippleTexCoord01;
#define IN_rippleTexCoord01 rippleTexCoord01

// TexCoord 2 for ripple texture lookup
in vec2 rippleTexCoord2;
#define IN_rippleTexCoord2 rippleTexCoord2

// Screenspace vert position BEFORE wave transformation
in vec4 posPreWave;
#define IN_posPreWave posPreWave

// Screenspace vert position AFTER wave transformation
in vec4 posPostWave;
#define IN_posPostWave posPostWave 

// Worldspace unit distance/depth of this vertex/pixel
in float  pixelDist;
#define IN_pixelDist pixelDist

in vec4 objPos;
#define IN_objPos objPos

in vec3 misc;
#define IN_misc misc

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
uniform sampler2D      bumpMap;
//uniform sampler2D    prepassTex;
uniform sampler2D    reflectMap;
uniform sampler2D      refractBuff;
uniform samplerCube  skyMap;
//uniform sampler2D      foamMap;
uniform vec4       baseColor;
uniform vec4       miscParams;
uniform vec4       reflectParams;
uniform vec3       ambientColor;
uniform vec3       eyePos;
uniform vec3       distortionParams;
uniform vec3       fogData;
uniform vec4       fogColor;
uniform vec4       rippleMagnitude;
uniform vec4       specularParams;
uniform mat4     modelMat;

out vec4 OUT_col;

//-----------------------------------------------------------------------------
// Main                                                                        
//-----------------------------------------------------------------------------
void main()
{ 
   // Modulate baseColor by the ambientColor.
   vec4 waterBaseColor = baseColor * vec4( ambientColor.rgb, 1 );
   
   // Get the bumpNorm...
   vec3 bumpNorm = ( texture( bumpMap, IN_rippleTexCoord01.xy ).rgb * 2.0 - 1.0 ) * rippleMagnitude.x;
   bumpNorm       += ( texture( bumpMap, IN_rippleTexCoord01.zw ).rgb * 2.0 - 1.0 ) * rippleMagnitude.y;      
   bumpNorm       += ( texture( bumpMap, IN_rippleTexCoord2 ).rgb * 2.0 - 1.0 ) * rippleMagnitude.z;  
   
   bumpNorm = normalize( bumpNorm );
   bumpNorm = mix( bumpNorm, vec3(0,0,1), 1.0 - rippleMagnitude.w );
   
   // We subtract a little from it so that we don't 
   // distort where the water surface intersects the
   // camera near plane.
   float distortAmt = saturate( IN_pixelDist / 1.0 ) * 0.8;
      
   vec4 distortPos = IN_posPostWave;
   distortPos.xy += bumpNorm.xy * distortAmt;   
 
 #ifdef UNDERWATER
   OUT_col = hdrEncode( textureProj( refractBuff, distortPos ) );   
 #else

   vec3 eyeVec = IN_objPos.xyz - eyePos;
   eyeVec = tMul( mat3(modelMat), eyeVec );
   vec3 reflectionVec = reflect( eyeVec, bumpNorm ); 

   // Color that replaces the reflection color when we do not
   // have one that is appropriate.
   vec4 fakeColor = vec4(ambientColor,1);
   
   // Use fakeColor for ripple-normals that are angled towards the camera  
   eyeVec = -eyeVec;
   eyeVec = normalize( eyeVec );
   float ang = saturate( dot( eyeVec, bumpNorm ) );   
   float fakeColorAmt = ang;   
      
    // Get reflection map color
   vec4 refMapColor = hdrDecode( textureProj( reflectMap, distortPos ) ); 
   // If we do not have a reflection texture then we use the cubemap.
   refMapColor = mix( refMapColor, texture( skyMap, reflectionVec ), NO_REFLECT );      
   
   // Combine reflection color and fakeColor.
   vec4 reflectColor = mix( refMapColor, fakeColor, fakeColorAmt );
   //return refMapColor;
   
   // Get refract color
   vec4 refractColor = hdrDecode( textureProj( refractBuff, distortPos ) );   
   
   // calc "diffuse" color by lerping from the water color
   // to refraction image based on the water clarity.
   vec4 diffuseColor = mix( refractColor, waterBaseColor, 1.0f - CLARITY );   
   
   // fresnel calculation 
   float fresnelTerm = fresnel( ang, FRESNEL_BIAS, FRESNEL_POWER );	
   //return vec4( fresnelTerm.rrr, 1 );
   
   // Also scale the frensel by our distance to the
   // water surface.  This removes the hard reflection
   // when really close to the water surface.
   fresnelTerm *= saturate( IN_pixelDist - 0.1 );
   
   // Combine the diffuse color and reflection image via the
   // fresnel term and set out output color.
   vec4 OUT = mix( diffuseColor, reflectColor, fresnelTerm );  
   
   #ifdef WATER_SPEC

      // Get some specular reflection.
      vec3 newbump = bumpNorm;
      newbump.xy *= 3.5;
      newbump = normalize( bumpNorm );
      half3 halfAng = normalize( eyeVec + -LIGHT_VEC );
      float specular = saturate( dot( newbump, halfAng ) );
      specular = pow( specular, SPEC_POWER );   
      
      OUT.rgb = OUT.rgb + ( SPEC_COLOR * specular.xxx );  

   #else // Disable fogging if spec is on because otherwise we run out of instructions.
   
      // Fog it.   
      float factor = computeSceneFog( eyePos, 
                                      IN_objPos.xyz, 
                                      WORLD_Z,
                                      fogData.x,
                                      fogData.y,
                                      fogData.z );

      //OUT.rgb = mix( OUT.rgb, fogColor.rgb, 1.0 - saturate( factor ) );   

   #endif
   
   OUT_col = hdrEncode( OUT );
   
#endif   
}
