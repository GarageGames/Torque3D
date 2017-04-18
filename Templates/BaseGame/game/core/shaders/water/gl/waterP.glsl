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

#include "../../gl/hlslCompat.glsl"  
#include "shadergen:/autogenConditioners.h"
#include "../../gl/torque.glsl"

//-----------------------------------------------------------------------------
// Defines                                                                  
//-----------------------------------------------------------------------------

#define PIXEL_DIST			IN_rippleTexCoord2.z
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
#define FOAM_OPACITY       		foamParams[0]
#define FOAM_MAX_DEPTH     foamParams[1]
#define FOAM_AMBIENT_LERP  		foamParams[2]
#define FOAM_RIPPLE_INFLUENCE 	foamParams[3]

// specularParams
#define SPEC_POWER         specularParams[3]
#define SPEC_COLOR         specularParams.xyz

//-----------------------------------------------------------------------------
// Structures                                                                  
//-----------------------------------------------------------------------------

//ConnectData IN

in vec4 hpos;   

// TexCoord 0 and 1 (xy,zw) for ripple texture lookup
in vec4 rippleTexCoord01;

// xy is TexCoord 2 for ripple texture lookup 
// z is the Worldspace unit distance/depth of this vertex/pixel
// w is amount of the crestFoam ( more at crest of waves ).
in vec4 rippleTexCoord2;

// Screenspace vert position BEFORE wave transformation
in vec4 posPreWave;

// Screenspace vert position AFTER wave transformation
in vec4 posPostWave;

// Objectspace vert position BEFORE wave transformation	
// w coord is world space z position.
in vec4 objPos;   

in vec4 foamTexCoords;

in mat3 tangentMat;


#define IN_hpos hpos
#define IN_rippleTexCoord01 rippleTexCoord01
#define IN_rippleTexCoord2 rippleTexCoord2
#define IN_posPreWave posPreWave
#define IN_posPostWave posPostWave
#define IN_objPos objPos
#define IN_foamTexCoords foamTexCoords
#define IN_tangentMat tangentMat

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
uniform samplerCube  skyMap;
uniform sampler2D      foamMap;
uniform sampler1D    depthGradMap;
uniform vec4         specularParams;
uniform vec4       baseColor;
uniform vec4       miscParams;
uniform vec2       fogParams;
uniform vec4       reflectParams;
uniform vec3       reflectNormal;
uniform vec2       wetnessParams;
uniform float        farPlaneDist;
uniform vec3       distortionParams;
uniform vec4         foamParams;
uniform vec3       ambientColor;
uniform vec3         eyePos; // This is in object space!
uniform vec3       fogData;
uniform vec4       fogColor;
uniform vec4         rippleMagnitude;
uniform vec4         rtParams1;
uniform float        depthGradMax;
uniform vec3         inLightVec;
uniform mat4         modelMat;
uniform vec4	      sunColor;
uniform float        sunBrightness;
uniform float        reflectivity;

out vec4 OUT_col;

//-----------------------------------------------------------------------------
// Main                                                                        
//-----------------------------------------------------------------------------
void main()
{ 
   // Get the bumpNorm...
   vec3 bumpNorm = ( texture( bumpMap, IN_rippleTexCoord01.xy ).rgb * 2.0 - 1.0 ) * rippleMagnitude.x;
   bumpNorm       += ( texture( bumpMap, IN_rippleTexCoord01.zw ).rgb * 2.0 - 1.0 ) * rippleMagnitude.y;      
   bumpNorm       += ( texture( bumpMap, IN_rippleTexCoord2.xy ).rgb * 2.0 - 1.0 ) * rippleMagnitude.z;         
                             
   bumpNorm = normalize( bumpNorm );
   bumpNorm = mix( bumpNorm, vec3(0,0,1), 1.0 - rippleMagnitude.w );
   bumpNorm = tMul( bumpNorm, IN_tangentMat ); 
   
   // Get depth of the water surface (this pixel).
   // Convert from WorldSpace to EyeSpace.
   float pixelDepth = PIXEL_DIST / farPlaneDist; 
   
   vec2 prepassCoord = viewportCoordToRenderTarget( IN_posPostWave, rtParams1 );

   float startDepth = prepassUncondition( prepassTex, prepassCoord ).w;  
   
   // The water depth in world units of the undistorted pixel.
   float startDelta = ( startDepth - pixelDepth );
   startDelta = max( startDelta, 0.0 );
   startDelta *= farPlaneDist;
            
   // Calculate the distortion amount for the water surface.
   // 
   // We subtract a little from it so that we don't 
   // distort where the water surface intersects the
   // camera near plane.
   float distortAmt = saturate( ( PIXEL_DIST - DISTORT_START_DIST ) / DISTORT_END_DIST );
   
   // Scale down distortion in shallow water.
   distortAmt *= saturate( startDelta / DISTORT_FULL_DEPTH );   

   // Do the intial distortion... we might remove it below.
   vec2 distortDelta = bumpNorm.xy * distortAmt;
   vec4 distortPos = IN_posPostWave;
   distortPos.xy += distortDelta;      
      
   prepassCoord = viewportCoordToRenderTarget( distortPos, rtParams1 );   

   // Get prepass depth at the position of this distorted pixel.
   float prepassDepth = prepassUncondition( prepassTex, prepassCoord ).w;      
   if ( prepassDepth > 0.99 )
     prepassDepth = 5.0;
    
   float delta = ( prepassDepth - pixelDepth ) * farPlaneDist;
      
   if ( delta < 0.0 )
   {
      // If we got a negative delta then the distorted
      // sample is above the water surface.  Mask it out
      // by removing the distortion.
      distortPos = IN_posPostWave;
      delta = startDelta;
      distortAmt = 0;
   } 
   else
   {
      float diff = ( prepassDepth - startDepth ) * farPlaneDist;
   
      if ( diff < 0 )
      {
         distortAmt = saturate( ( PIXEL_DIST - DISTORT_START_DIST ) / DISTORT_END_DIST );
         distortAmt *= saturate( delta / DISTORT_FULL_DEPTH );

         distortDelta = bumpNorm.xy * distortAmt;
         
         distortPos = IN_posPostWave;         
         distortPos.xy += distortDelta;    
        
         prepassCoord = viewportCoordToRenderTarget( distortPos, rtParams1 );

         // Get prepass depth at the position of this distorted pixel.
         prepassDepth = prepassUncondition( prepassTex, prepassCoord ).w;
	 if ( prepassDepth > 0.99 )
            prepassDepth = 5.0;
         delta = ( prepassDepth - pixelDepth ) * farPlaneDist;
      }
       
      if ( delta < 0.1 )
      {
         // If we got a negative delta then the distorted
         // sample is above the water surface.  Mask it out
         // by removing the distortion.
         distortPos = IN_posPostWave;
         delta = startDelta;
         distortAmt = 0;
      } 
   }
   
   vec4 temp = IN_posPreWave;
   temp.xy += bumpNorm.xy * distortAmt;   
   vec2 reflectCoord = viewportCoordToRenderTarget( temp, rtParams1 );     
   
   vec2 refractCoord = viewportCoordToRenderTarget( distortPos, rtParams1 );
   
   vec4 fakeColor = vec4(ambientColor,1);   
   vec3 eyeVec = IN_objPos.xyz - eyePos;
   eyeVec = tMul( mat3(modelMat), eyeVec );
   eyeVec = tMul( IN_tangentMat, eyeVec );
   vec3 reflectionVec = reflect( eyeVec, bumpNorm );   
   
   // Use fakeColor for ripple-normals that are angled towards the camera   
   eyeVec = -eyeVec;
   eyeVec = normalize( eyeVec );
   float ang = saturate( dot( eyeVec, bumpNorm ) );   
   float fakeColorAmt = ang; 
   
   // for verts far from the reflect plane z position
   float rplaneDist = abs( REFLECT_PLANE_Z - IN_objPos.w );
   rplaneDist = saturate( ( rplaneDist - 1.0 ) / 2.0 );  
   rplaneDist *= ISRIVER;
   fakeColorAmt = max( fakeColorAmt, rplaneDist );        
 
#ifndef UNDERWATER

   // Get foam color and amount
   vec2 foamRippleOffset = bumpNorm.xy * FOAM_RIPPLE_INFLUENCE;
   vec4 IN_foamTexCoords = IN_foamTexCoords;
   IN_foamTexCoords.xy += foamRippleOffset; 
   IN_foamTexCoords.zw += foamRippleOffset;
   
   vec4 foamColor = texture( foamMap, IN_foamTexCoords.xy );   
   foamColor += texture( foamMap, IN_foamTexCoords.zw ); 
   foamColor = saturate( foamColor );
   
   // Modulate foam color by ambient color
   // so we don't have glowing white foam at night.
   foamColor.rgb = mix( foamColor.rgb, ambientColor.rgb, FOAM_AMBIENT_LERP );
   
   float foamDelta = saturate( delta / FOAM_MAX_DEPTH );      
   float foamAmt = 1 - pow( foamDelta, 2 );
   
   // Fade out the foam in very very low depth,
   // this improves the shoreline a lot.
   float diff = 0.8 - foamAmt;
   if ( diff < 0.0 )
      foamAmt -= foamAmt * abs( diff ) / 0.2;

   foamAmt *= FOAM_OPACITY * foamColor.a;
   
   foamColor.rgb *= FOAM_OPACITY * foamAmt * foamColor.a;
   
   // Get reflection map color.
   vec4 refMapColor = texture( reflectMap, reflectCoord );  
   
   // If we do not have a reflection texture then we use the cubemap.
   refMapColor = mix( refMapColor, texture( skyMap, reflectionVec ), NO_REFLECT );
   
   fakeColor = ( texture( skyMap, reflectionVec ) );
   fakeColor.a = 1;
   // Combine reflection color and fakeColor.
   vec4 reflectColor = mix( refMapColor, fakeColor, fakeColorAmt );
   
   // Get refract color
   vec4 refractColor = hdrDecode( texture( refractBuff, refractCoord ) );    
   
   // We darken the refraction color a bit to make underwater 
   // elements look wet.  We fade out this darkening near the
   // surface in order to not have hard water edges.
   // @param WET_DEPTH The depth in world units at which full darkening will be recieved.
   // @param WET_COLOR_FACTOR The refract color is scaled down by this amount when at WET_DEPTH
   refractColor.rgb *= 1.0f - ( saturate( delta / WET_DEPTH ) * WET_COLOR_FACTOR );
   
   // Add Water fog/haze.
   float fogDelta = delta - FOG_DENSITY_OFFSET;

   if ( fogDelta < 0.0 )
      fogDelta = 0.0;     
   float fogAmt = 1.0 - saturate( exp( -FOG_DENSITY * fogDelta )  );
   
   // Calculate the water "base" color based on depth.
   vec4 waterBaseColor = baseColor * texture( depthGradMap, saturate( delta / depthGradMax ) );
   waterBaseColor = toLinear(waterBaseColor);
      
   // Modulate baseColor by the ambientColor.
   waterBaseColor *= vec4( ambientColor.rgb, 1 );     
   
   // calc "diffuse" color by lerping from the water color
   // to refraction image based on the water clarity.
   vec4 diffuseColor = mix( refractColor, waterBaseColor, fogAmt );
   
   // fresnel calculation   
   float fresnelTerm = fresnel( ang, FRESNEL_BIAS, FRESNEL_POWER );	
   
   // Scale the frensel strength by fog amount
   // so that parts that are very clear get very little reflection.
   fresnelTerm *= fogAmt;   
   
   // Also scale the frensel by our distance to the
   // water surface.  This removes the hard reflection
   // when really close to the water surface.
   fresnelTerm *= saturate( PIXEL_DIST - 0.1 );
   
   fresnelTerm *= reflectivity;
   
   // Combine the diffuse color and reflection image via the
   // fresnel term and set out output color.
   vec4 OUT = mix( diffuseColor, reflectColor, fresnelTerm );
   
   vec3 lightVec = inLightVec;
   
   // Get some specular reflection.
   vec3 newbump = bumpNorm;
   newbump.xy *= 3.5;
   newbump = normalize( newbump );
   vec3 halfAng = normalize( eyeVec + -lightVec );
   float specular = saturate( dot( newbump, halfAng ) );
   specular = pow( specular, SPEC_POWER );   
   
   // Scale down specularity in very shallow water to improve the transparency of the shoreline.
   specular *= saturate( delta / 2 );
   OUT.rgb = OUT.rgb + ( SPEC_COLOR * vec3(specular) );      
   
#else

   vec4 refractColor = hdrDecode( texture( refractBuff, refractCoord ) );   
   vec4 OUT = refractColor;  
   
#endif

#ifndef UNDERWATER
   
   OUT.rgb = OUT.rgb + foamColor.rgb;

   float factor = computeSceneFog( eyePos, 
                                   IN_objPos.xyz, 
                                   IN_objPos.w,
                                   fogData.x,
                                   fogData.y,
                                   fogData.z );

   OUT.rgb = mix( OUT.rgb, fogColor.rgb, 1.0 - saturate( factor ) );  
   
   //OUT.rgb = fogColor.rgb;
   
#endif

   OUT.a = 1.0;

   //return OUT;
   
   OUT_col = hdrEncode( OUT );
}
