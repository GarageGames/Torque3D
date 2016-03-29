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

#include "../shaderModelAutoGen.hlsl"
#include "../torque.hlsl"

//-----------------------------------------------------------------------------
// Defines                                                                  
//-----------------------------------------------------------------------------

#define PIXEL_DIST			IN.rippleTexCoord2.z
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
#define FOAM_MAX_DEPTH     		foamParams[1]
#define FOAM_AMBIENT_LERP  		foamParams[2]
#define FOAM_RIPPLE_INFLUENCE 	foamParams[3]

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
   
   // xy is TexCoord 2 for ripple texture lookup 
   // z is the Worldspace unit distance/depth of this vertex/pixel
   // w is amount of the crestFoam ( more at crest of waves ).
   float4 rippleTexCoord2  : TEXCOORD1;
   
   // Screenspace vert position BEFORE wave transformation
   float4 posPreWave       : TEXCOORD2;
   
   // Screenspace vert position AFTER wave transformation
   float4 posPostWave      : TEXCOORD3;   
    
   // Objectspace vert position BEFORE wave transformation	
   // w coord is world space z position.
   float4 objPos           : TEXCOORD4;   
   
   float4 foamTexCoords    : TEXCOORD5;
   
   float3x3 tangentMat     : TEXCOORD6;
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
TORQUE_UNIFORM_SAMPLER2D(prepassTex, 1);
TORQUE_UNIFORM_SAMPLER2D(reflectMap, 2);
TORQUE_UNIFORM_SAMPLER2D(refractBuff, 3);
TORQUE_UNIFORM_SAMPLERCUBE(skyMap, 4);
TORQUE_UNIFORM_SAMPLER2D(foamMap, 5);
TORQUE_UNIFORM_SAMPLER1D(depthGradMap, 6);
uniform float4       specularParams;
uniform float4       baseColor;
uniform float4       miscParams;
uniform float2       fogParams;
uniform float4       reflectParams;
uniform float3       reflectNormal;
uniform float2       wetnessParams;
uniform float        farPlaneDist;
uniform float3       distortionParams;
uniform float4       foamParams;
uniform float3       ambientColor;
uniform float3       eyePos; // This is in object space!
uniform float3       fogData;
uniform float4       fogColor;
uniform float4       rippleMagnitude;
uniform float4       rtParams1;
uniform float        depthGradMax;
uniform float3       inLightVec;
uniform float4x4     modelMat;
uniform float4	     sunColor;
uniform float        sunBrightness;
uniform float        reflectivity;

//-----------------------------------------------------------------------------
// Main                                                                        
//-----------------------------------------------------------------------------
float4 main( ConnectData IN ) : TORQUE_TARGET0
{    
   // Get the bumpNorm...
   float3 bumpNorm = ( TORQUE_TEX2D( bumpMap, IN.rippleTexCoord01.xy ).rgb * 2.0 - 1.0 ) * rippleMagnitude.x;
   bumpNorm       += ( TORQUE_TEX2D( bumpMap, IN.rippleTexCoord01.zw ).rgb * 2.0 - 1.0 ) * rippleMagnitude.y;      
   bumpNorm       += ( TORQUE_TEX2D( bumpMap, IN.rippleTexCoord2.xy ).rgb * 2.0 - 1.0 ) * rippleMagnitude.z;         
  
   bumpNorm = normalize( bumpNorm );
   bumpNorm = lerp( bumpNorm, float3(0,0,1), 1.0 - rippleMagnitude.w );
   bumpNorm = mul( bumpNorm, IN.tangentMat ); 
   
   // Get depth of the water surface (this pixel).
   // Convert from WorldSpace to EyeSpace.
   float pixelDepth = PIXEL_DIST / farPlaneDist; 
   
   float2 prepassCoord = viewportCoordToRenderTarget( IN.posPostWave, rtParams1 );

   float startDepth = TORQUE_PREPASS_UNCONDITION( prepassTex, prepassCoord ).w;  
   
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
   float2 distortDelta = bumpNorm.xy * distortAmt;
   float4 distortPos = IN.posPostWave;
   distortPos.xy += distortDelta;      
      
   prepassCoord = viewportCoordToRenderTarget( distortPos, rtParams1 );   

   // Get prepass depth at the position of this distorted pixel.
   float prepassDepth = TORQUE_PREPASS_UNCONDITION( prepassTex, prepassCoord ).w;      
   if ( prepassDepth > 0.99 )
     prepassDepth = 5.0;
    
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
         distortAmt = saturate( ( PIXEL_DIST - DISTORT_START_DIST ) / DISTORT_END_DIST );
         distortAmt *= saturate( delta / DISTORT_FULL_DEPTH );

         distortDelta = bumpNorm.xy * distortAmt;
         
         distortPos = IN.posPostWave;         
         distortPos.xy += distortDelta;    
        
         prepassCoord = viewportCoordToRenderTarget( distortPos, rtParams1 );

         // Get prepass depth at the position of this distorted pixel.
         prepassDepth = TORQUE_PREPASS_UNCONDITION( prepassTex, prepassCoord ).w;
	 if ( prepassDepth > 0.99 )
            prepassDepth = 5.0;
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
     
   float4 temp = IN.posPreWave;
   temp.xy += bumpNorm.xy * distortAmt;   
   float2 reflectCoord = viewportCoordToRenderTarget( temp, rtParams1 );     
   
   float2 refractCoord = viewportCoordToRenderTarget( distortPos, rtParams1 );
   
   float4 fakeColor = float4(ambientColor,1);   
   float3 eyeVec = IN.objPos.xyz - eyePos;
   eyeVec = mul( (float3x3)modelMat, eyeVec );
   eyeVec = mul( IN.tangentMat, eyeVec );
   float3 reflectionVec = reflect( eyeVec, bumpNorm );
   
   // Use fakeColor for ripple-normals that are angled towards the camera   
   eyeVec = -eyeVec;
   eyeVec = normalize( eyeVec );
   float ang = saturate( dot( eyeVec, bumpNorm ) );   
   float fakeColorAmt = ang; 
   
   // for verts far from the reflect plane z position
   float rplaneDist = abs( REFLECT_PLANE_Z - IN.objPos.w );
   rplaneDist = saturate( ( rplaneDist - 1.0 ) / 2.0 );  
   rplaneDist *= ISRIVER;   
   fakeColorAmt = max( fakeColorAmt, rplaneDist );        
 
#ifndef UNDERWATER
   
   // Get foam color and amount
   float2 foamRippleOffset = bumpNorm.xy * FOAM_RIPPLE_INFLUENCE;
   IN.foamTexCoords.xy += foamRippleOffset; 
   IN.foamTexCoords.zw += foamRippleOffset;
   
   float4 foamColor = TORQUE_TEX2D( foamMap, IN.foamTexCoords.xy );   
   foamColor += TORQUE_TEX2D( foamMap, IN.foamTexCoords.zw ); 
   foamColor = saturate( foamColor );
   
   // Modulate foam color by ambient color
   // so we don't have glowing white foam at night.
   foamColor.rgb = lerp( foamColor.rgb, ambientColor.rgb, FOAM_AMBIENT_LERP );
   
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
   float4 refMapColor = TORQUE_TEX2D( reflectMap, reflectCoord );  
   
   // If we do not have a reflection texture then we use the cubemap.
   refMapColor = lerp( refMapColor, TORQUE_TEXCUBE( skyMap, reflectionVec ), NO_REFLECT );
   
   fakeColor = ( TORQUE_TEXCUBE( skyMap, reflectionVec ) );
   fakeColor.a = 1;
   // Combine reflection color and fakeColor.
   float4 reflectColor = lerp( refMapColor, fakeColor, fakeColorAmt );
   
   // Get refract color
   float4 refractColor = hdrDecode( TORQUE_TEX2D( refractBuff, refractCoord ) );    
   
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
   float4 waterBaseColor = baseColor * TORQUE_TEX1D( depthGradMap, saturate( delta / depthGradMax ) );
   waterBaseColor = toLinear(waterBaseColor);
      
   // Modulate baseColor by the ambientColor.
   waterBaseColor *= float4( ambientColor.rgb, 1 );     
   
   // calc "diffuse" color by lerping from the water color
   // to refraction image based on the water clarity.   
   float4 diffuseColor = lerp( refractColor, waterBaseColor, fogAmt );
   
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
   float4 OUT = lerp( diffuseColor, reflectColor, fresnelTerm );
   
   float3 lightVec = inLightVec;

   // Get some specular reflection.
   float3 newbump = bumpNorm;
   newbump.xy *= 3.5;
   newbump = normalize( bumpNorm );
   float3 halfAng = normalize( eyeVec + -lightVec );
   float specular = saturate( dot( newbump, halfAng ) );
   specular = pow( specular, SPEC_POWER );   
   
   // Scale down specularity in very shallow water to improve the transparency of the shoreline.
   specular *= saturate( delta / 2 );
   OUT.rgb = OUT.rgb + ( SPEC_COLOR * specular.xxx );      

#else

   float4 refractColor = hdrDecode( TORQUE_TEX2D( refractBuff, refractCoord ) );   
   float4 OUT = refractColor;  
   
#endif

#ifndef UNDERWATER

   OUT.rgb = OUT.rgb + foamColor.rgb;

   float factor = computeSceneFog( eyePos, 
                                   IN.objPos.xyz, 
                                   IN.objPos.w,
                                   fogData.x,
                                   fogData.y,
                                   fogData.z );

   OUT.rgb = lerp( OUT.rgb, fogColor.rgb, 1.0 - saturate( factor ) );  
   
   //OUT.rgb = fogColor.rgb;
   
#endif

   OUT.a = 1.0;

   //return OUT;

   return hdrEncode( OUT );
}
