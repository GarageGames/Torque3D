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
#include "../../gl/torque.glsl"

//-----------------------------------------------------------------------------
// Defines                                                                  
//-----------------------------------------------------------------------------

#ifdef TORQUE_BASIC_LIGHTING
   #define BASIC
#endif

// miscParams
#define FRESNEL_BIAS       miscParams[0]
#define FRESNEL_POWER      miscParams[1]
// miscParams[2] is unused
#define ISRIVER            miscParams[3]

// reflectParams
#define REFLECT_PLANE_Z    reflectParams[0]
#define REFLECT_MIN_DIST   reflectParams[1]
#define REFLECT_MAX_DIST   reflectParams[2]
#define NO_REFLECT         reflectParams[3]

// fogParams
#define FOG_DENSITY        fogParams[0]
#define FOG_DENSITY_OFFSET fogParams[1]

// wetnessParams
#define WET_DEPTH          wetnessParams[0]
#define WET_COLOR_FACTOR   wetnessParams[1]

// distortionParams
#define DISTORT_START_DIST distortionParams[0]
#define DISTORT_END_DIST   distortionParams[1]
#define DISTORT_FULL_DEPTH distortionParams[2]

// foamParams
#define FOAM_SCALE         foamParams[0]
#define FOAM_MAX_DEPTH     foamParams[1]

// Incoming data
// Worldspace position of this pixel
varying vec3 worldPos;

// TexCoord 0 and 1 (xy,zw) for ripple texture lookup
varying vec4 rippleTexCoord01;

// TexCoord 2 for ripple texture lookup
varying vec2 rippleTexCoord2;

// Screenspace vert position BEFORE wave transformation
varying vec4 posPreWave;

// Screenspace vert position AFTER wave transformation
varying vec4 posPostWave;

// Worldspace unit distance/depth of this vertex/pixel
varying float pixelDist;

varying vec3 fogPos;

varying float worldSpaceZ;

varying vec4 foamTexCoords;

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
uniform sampler2D    prepassTex;
uniform sampler2D    reflectMap;
uniform sampler2D      refractBuff;
uniform samplerCUBE  skyMap;
uniform sampler2D      foamMap;
uniform vec4       specularColor;
uniform float        specularPower;
uniform vec4       baseColor;
uniform vec4       miscParams;
uniform vec2       fogParams;
uniform vec4       reflectParams;
uniform vec3       reflectNormal;
uniform vec2       wetnessParams;
uniform float        farPlaneDist;
uniform vec3       distortionParams;
//uniform vec4       renderTargetParams;
uniform vec2       foamParams;
uniform vec3       foamColorMod;
uniform vec3       ambientColor;
uniform vec3       eyePos;
uniform vec3       inLightVec;
uniform vec3       fogData;
uniform vec4       fogColor;
//uniform vec4       rtParams;
uniform vec2       rtScale;
uniform vec2       rtHalfPixel;
uniform vec4       rtOffset;
uniform vec3       rippleMagnitude;

//-----------------------------------------------------------------------------
// Main                                                                        
//-----------------------------------------------------------------------------
void main()
{ 
   vec4 rtParams = vec4( rtOffset.x / rtOffset.z + rtHalfPixel.x, 
                             rtOffset.y / rtOffset.w + rtHalfPixel.x,
                             rtScale );
                             
   // Modulate baseColor by the ambientColor.
   vec4 waterBaseColor = baseColor * vec4( ambientColor.rgb, 1 );
   
   // Get the bumpNorm...
   vec3 bumpNorm = ( tex2D( bumpMap, IN.rippleTexCoord01.xy ) * 2.0 - 1.0 ) * rippleMagnitude.x;
   bumpNorm       += ( tex2D( bumpMap, IN.rippleTexCoord01.zw ) * 2.0 - 1.0 ) * rippleMagnitude.y;      
   bumpNorm       += ( tex2D( bumpMap, IN.rippleTexCoord2 ) * 2.0 - 1.0 ) * rippleMagnitude.z;   
      
   // JCF: this was here, but seems to make the dot product against the bump
   // normal we use below for cubeMap fade-in to be less reliable.
   //bumpNorm.xy *= 0.75;
   //bumpNorm = normalize( bumpNorm );
   //return vec4( bumpNorm, 1 );
   
   // Get depth of the water surface (this pixel).
   // Convert from WorldSpace to EyeSpace.
   float pixelDepth = IN.pixelDist / farPlaneDist; 
   
   // Get prepass depth at the undistorted pixel.
   //vec4 prepassCoord = IN.posPostWave;
   //prepassCoord.xy += renderTargetParams.xy;
   vec2 prepassCoord = viewportCoordToRenderTarget( IN.posPostWave, rtParams );
   //vec2 prepassCoord = IN.posPostWave.xy;

   float startDepth = prepassUncondition( tex2D( prepassTex, prepassCoord ) ).w;  
   //return vec4( startDepth.rrr, 1 );
   
   // The water depth in world units of the undistorted pixel.
   float startDelta = ( startDepth - pixelDepth );
   if ( startDelta <= 0.0 )
   {
      //return vec4( 1, 0, 0, 1 );
      startDelta = 0;
   }

   startDelta *= farPlaneDist;
            
   // Calculate the distortion amount for the water surface.
   // 
   // We subtract a little from it so that we don't 
   // distort where the water surface intersects the
   // camera near plane.
   float distortAmt = saturate( ( IN.pixelDist - DISTORT_START_DIST ) / DISTORT_END_DIST );
   
   // Scale down distortion in shallow water.
   distortAmt *= saturate( startDelta / DISTORT_FULL_DEPTH );   
   //distortAmt = 0;

   // Do the intial distortion... we might remove it below.
   vec2 distortDelta = bumpNorm.xy * distortAmt;
   vec4 distortPos = IN.posPostWave;
   distortPos.xy += distortDelta;      
      
   prepassCoord = viewportCoordToRenderTarget( distortPos, rtParams );   
   //prepassCoord = distortPos;   
   //prepassCoord.xy += renderTargetParams.xy;

   // Get prepass depth at the position of this distorted pixel.
   float prepassDepth = prepassUncondition( tex2D( prepassTex, prepassCoord ) ).w;      
    
   float delta = ( prepassDepth - pixelDepth ) * farPlaneDist;
      
   if ( delta < 0.0 )
   {
      // If we got a negative delta then the distorted
      // sample is above the water surface.  Mask it out
      // by removing the distortion.
      distortPos = IN.posPostWave;
      delta = startDelta;
      distortAmt = 0;
   } 
   else
   {
      float diff = ( prepassDepth - startDepth ) * farPlaneDist;
   
      if ( diff < 0 )
      {
         distortAmt = saturate( ( IN.pixelDist - DISTORT_START_DIST ) / DISTORT_END_DIST );
         distortAmt *= saturate( delta / DISTORT_FULL_DEPTH );

         distortDelta = bumpNorm.xy * distortAmt;
         
         distortPos = IN.posPostWave;         
         distortPos.xy += distortDelta;    
        
         prepassCoord = viewportCoordToRenderTarget( distortPos, rtParams );
         //prepassCoord = distortPos;
         //prepassCoord.xy += renderTargetParams.xy;

         // Get prepass depth at the position of this distorted pixel.
         prepassDepth = prepassUncondition( tex2D( prepassTex, prepassCoord ) ).w;
         delta = ( prepassDepth - pixelDepth ) * farPlaneDist;
      }
       
      if ( delta < 0.1 )
      {
         // If we got a negative delta then the distorted
         // sample is above the water surface.  Mask it out
         // by removing the distortion.
         distortPos = IN.posPostWave;
         delta = startDelta;
         distortAmt = 0;
      } 
   }
   
   //return vec4( prepassDepth.rrr, 1 );
     
   vec4 temp = IN.posPreWave;
   temp.xy += bumpNorm.xy * distortAmt;   
   vec2 reflectCoord = viewportCoordToRenderTarget( temp, rtParams );     
   
   vec2 refractCoord = viewportCoordToRenderTarget( distortPos, rtParams );
   
   // Use cubemap colors instead of reflection colors in several cases...
        
   // First lookup the CubeMap color
   // JCF: which do we want to use here, the reflectNormal or the bumpNormal
   // neithor of them is exactly right and how can we combine the two together?
   //bumpNorm = reflectNormal;
   vec3 eyeVec = IN.worldPos - eyePos;
   vec3 reflectionVec = reflect( eyeVec, bumpNorm );   
   //vec4 cubeColor = texCUBE( skyMap, reflectionVec );
   //return cubeColor;
   // JCF: using ambient color instead of cubeColor for waterPlane, how do we still use the cubemap for rivers?   
   vec4 cubeColor = vec4(ambientColor,1);
   //cubeColor.rgb = vec3( 0, 0, 1 );
   
   // Use cubeColor for waves that are angled towards camera   
   eyeVec = -eyeVec;
   eyeVec = normalize( eyeVec );
   float ang = saturate( dot( eyeVec, bumpNorm ) );   
   float cubeAmt = ang;
   
   //float rplaneDist = (reflectPlane.x * IN.pos.x + reflectPlane.y * IN.pos.y + reflectPlane.z * IN.pos.z) + reflectPlane.w;
   //rplaneDist = saturate( abs( rplaneDist ) / 0.5 );
   
   
//#ifdef RIVER
   // for verts far from the reflect plane z position
   float rplaneDist = abs( REFLECT_PLANE_Z - IN.worldPos.z );
   rplaneDist = saturate( ( rplaneDist - 1.0 ) / 2.0 );  
   //rplaneDist = REFLECT_PLANE_Z / eyePos.z;
   rplaneDist *= ISRIVER;
   cubeAmt = max( cubeAmt, rplaneDist );
//#endif
   
   //rplaneDist = IN.worldPos.z / eyePos.z;
   
   //return vec4( rplaneDist.rrr, 1 );   
   //return vec4( (reflectParams[REFLECT_PLANE_Z] / 86.0 ).rrr, 1 );
   
   // and for verts farther from the camera
   //float cubeAmt = ( eyeDist - reflectParams[REFLECT_MIN_DIST] ) / ( reflectParams[REFLECT_MAX_DIST] - reflectParams[REFLECT_MIN_DIST] );
   //cubeAmt = saturate ( cubeAmt );      
   
   //float temp = ( eyeDist - reflectParams[REFLECT_MIN_DIST] ) / ( reflectParams[REFLECT_MAX_DIST] - reflectParams[REFLECT_MIN_DIST] );
   //temp = saturate ( temp );      
   
   // If the camera is very very close to the reflect plane.
   //float eyeToPlaneDist = eyePos.z - REFLECT_PLANE_Z; // dot( reflectNormal, eyePos ) + REFLECT_PLANE_Z;
   //eyeToPlaneDist = abs( eyeToPlaneDist );
   //eyeToPlaneDist = 1.0 - saturate( abs( eyeToPlaneDist ) / 1 );
   
   //return vec4( eyeToPlaneDist.rrr, 1 );
   
   //cubeAmt = max( cubeAmt, eyeToPlaneDist );
   //cubeAmt = max( cubeAmt, rplaneDist );
   //cubeAmt = max( cubeAmt, ang );
   //cubeAmt = max( cubeAmt, rplaneDist );
   //cubeAmt = max( cubeAmt, IN.depth.w );
   
   // All cubemap if fullReflect is specifically user disabled
   cubeAmt = max( cubeAmt, NO_REFLECT );      
 
#ifndef UNDERWATER

   
   // Get foam color and amount
   IN.foamTexCoords.xy += distortDelta * 0.5; 
   IN.foamTexCoords.zw += distortDelta * 0.5;
   
   vec4 foamColor = tex2D( foamMap, IN.foamTexCoords.xy );   
   foamColor += tex2D( foamMap, IN.foamTexCoords.zw );
   //foamColor += tex2D( foamMap, IN.rippleTexCoord2 ) * 0.3;     
   foamColor = saturate( foamColor );
   // Modulate foam color by ambient color so we don't have glowing white
   // foam at night.
   foamColor.rgb = lerp( foamColor.rgb, ambientColor.rgb, foamColorMod.rgb );
   
   float foamDelta = saturate( delta / FOAM_MAX_DEPTH );      
   float foamAmt = 1.0 - foamDelta;
   
   // Fade out the foam in very very low depth,
   // this improves the shoreline a lot.
   float diff = 0.8 - foamAmt;
   if ( diff < 0.0 )
   {
      //return vec4( 1,0,0,1 );
      foamAmt -= foamAmt * abs( diff ) / 0.2;
   }
   //return vec4( foamAmt.rrr, 1 );
   
   foamAmt *= FOAM_SCALE * foamColor.a;
   //return vec4( foamAmt.rrr, 1 );

   // Get reflection map color
   vec4 refMapColor = tex2D( reflectMap, reflectCoord );   
   
   //cubeAmt = 0;
   
   // Combine cube and foam colors into reflect color
   vec4 reflectColor = lerp( refMapColor, cubeColor, cubeAmt );
   //return refMapColor;
   
   // This doesn't work because REFLECT_PLANE_Z is in worldSpace
   // while eyePos is actually in objectSpace!
   
   //float eyeToPlaneDist = eyePos.z - REFLECT_PLANE_Z; // dot( reflectNormal, eyePos ) + REFLECT_PLANE_Z;   
   //float transitionFactor = 1.0 - saturate( ( abs( eyeToPlaneDist ) - 0.5 ) / 5 );         
   //reflectColor = lerp( reflectColor, waterBaseColor, transitionFactor );   
   
   //return reflectColor;
   
   // Get refract color
   vec4 refractColor = tex2D( refractBuff, refractCoord );   
   //return refractColor;
   
   // We darken the refraction color a bit to make underwater 
   // elements look wet.  We fade out this darkening near the
   // surface in order to not have hard water edges.
   // @param WET_DEPTH The depth in world units at which full darkening will be recieved.
   // @param WET_COLOR_FACTOR The refract color is scaled down by this amount when at WET_DEPTH
   refractColor.rgb *= 1.0f - ( saturate( delta / WET_DEPTH ) * WET_COLOR_FACTOR );
   
   // Add Water fog/haze.
   float fogDelta = delta - FOG_DENSITY_OFFSET;
   //return vec4( fogDelta.rrr, 1 );
   if ( fogDelta < 0.0 )
      fogDelta = 0.0;     
   float fogAmt = 1.0 - saturate( exp( -FOG_DENSITY * fogDelta )  );
   //return vec4( fogAmt.rrr, 1 );
   
   // calc "diffuse" color by lerping from the water color
   // to refraction image based on the water clarity.
   vec4 diffuseColor = lerp( refractColor, waterBaseColor, fogAmt );
   
   // fresnel calculation   
   float fresnelTerm = fresnel( ang, FRESNEL_BIAS, FRESNEL_POWER );	
   //return vec4( fresnelTerm.rrr, 1 );
   
   // Scale the frensel strength by fog amount
   // so that parts that are very clear get very little reflection.
   fresnelTerm *= fogAmt;   
   //return vec4( fresnelTerm.rrr, 1 );   
   
   // Also scale the frensel by our distance to the
   // water surface.  This removes the hard reflection
   // when really close to the water surface.
   fresnelTerm *= saturate( IN.pixelDist - 0.1 );
   
   // Combine the diffuse color and reflection image via the
   // fresnel term and set out output color.
   vec4 gl_FragColor = lerp( diffuseColor, reflectColor, fresnelTerm );
   
   //float brightness = saturate( 1.0 - ( waterHeight - eyePosWorld.z - 5.0 ) / 50.0 );
   //gl_FragColor.rgb *= brightness;
   
#else
   vec4 refractColor = tex2D( refractBuff, refractCoord );   
   vec4 gl_FragColor = refractColor;   
#endif

#ifndef UNDERWATER
   gl_FragColor.rgb = lerp( gl_FragColor.rgb, foamColor.rgb, foamAmt );
#endif
   
   gl_FragColor.a = 1.0;
   
   // specular experiments

// 1:
/*
   float fDot = dot( bumpNorm, inLightVec );
   vec3 reflect = normalize( 2.0 * bumpNorm * fDot - eyeVec );
   // float specular = saturate(dot( reflect, inLightVec ) );
   float specular = pow( reflect, specularPower );
   gl_FragColor += specularColor * specular;
*/


// 2:  This almost looks good 
/*
   bumpNorm.xy *= 2.0;
   bumpNorm = normalize( bumpNorm );

   vec3 halfAng = normalize( eyeVec + inLightVec );
   float specular = saturate( dot( bumpNorm, halfAng) );
   specular = pow(specular, specularPower);
   gl_FragColor += specularColor * specular;
*/

#ifndef UNDERWATER      

   float factor = computeSceneFog( eyePos, 
                                   IN.fogPos, 
                                   IN.worldSpaceZ,
                                   fogData.x,
                                   fogData.y,
                                   fogData.z );

  gl_FragColor.rgb = lerp( gl_FragColor.rgb, fogColor.rgb, 1.0 - saturate( factor ) );   
   
#endif

   //return vec4( refMapColor.rgb, 1 );
   gl_FragColor.a = 1.0;
   
   return gl_FragColor;
}
