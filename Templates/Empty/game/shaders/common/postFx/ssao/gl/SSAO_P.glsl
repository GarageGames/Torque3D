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
#include "../../gl/postFX.glsl"

#define DOSMALL
#define DOLARGE

uniform sampler2D prepassMap ;
uniform sampler2D randNormalTex ;
uniform sampler1D powTable ;

uniform vec2 nearFar;
uniform vec2 worldToScreenScale;
uniform vec2 texSize0;
uniform vec2 texSize1;
uniform vec2 targetSize;

// Script-set constants.

uniform float overallStrength;

uniform float sRadius;
uniform float sStrength;
uniform float sDepthMin;
uniform float sDepthMax;
uniform float sDepthPow;
uniform float sNormalTol;
uniform float sNormalPow;

uniform float lRadius;
uniform float lStrength;
uniform float lDepthMin;
uniform float lDepthMax;
uniform float lDepthPow;
uniform float lNormalTol;
uniform float lNormalPow;

out vec4 OUT_col;


#ifndef QUALITY
   #define QUALITY 2
#endif


#if QUALITY == 0
   #define sSampleCount 4
   #define totalSampleCount 12
#elif QUALITY == 1
   #define sSampleCount 6
   #define totalSampleCount 24
#elif QUALITY == 2
   #define sSampleCount 8
   #define totalSampleCount 32
#endif


float getOcclusion( float depthDiff, float depthMin, float depthMax, float depthPow, 
                    float normalDiff, float dt, float normalTol, float normalPow )
{
   if ( depthDiff < 0.0 )
      return 0.0;
      
   float delta = abs( depthDiff );
   
   if ( delta < depthMin || delta > depthMax )
      return 0.0;   
      
   delta = saturate( delta / depthMax );
   
   if ( dt > 0.0 )
      normalDiff *= dt;
   else
      normalDiff = 1.0;
      
      
   normalDiff *= 1.0 - ( dt * 0.5 + 0.5 );
   
   return ( 1.0 - texture( powTable, delta ).r ) * normalDiff;   
}


void main()
{          
   const vec3 ptSphere[32] = vec3[]
   (
   	vec3( 0.295184, 0.077723, 0.068429 ),
   	vec3( -0.271976, -0.365221, -0.838363 ),
   	vec3( 0.547713, 0.467576, 0.488515 ),
   	vec3( 0.662808, -0.031733, -0.584758 ),
   	vec3( -0.025717, 0.218955, -0.657094 ),
   	vec3( -0.310153, -0.365223, -0.370701 ),
   	vec3( -0.101407, -0.006313, -0.747665 ),
   	vec3( -0.769138, 0.360399, -0.086847 ),
   	vec3( -0.271988, -0.275140, -0.905353 ),
   	vec3( 0.096740, -0.566901, 0.700151 ),
   	vec3( 0.562872, -0.735136, -0.094647 ),
   	vec3( 0.379877, 0.359278, 0.190061 ),
   	vec3( 0.519064, -0.023055, 0.405068 ),
   	vec3( -0.301036, 0.114696, -0.088885 ),
   	vec3( -0.282922, 0.598305, 0.487214 ),
   	vec3( -0.181859, 0.251670, -0.679702 ),
   	vec3( -0.191463, -0.635818, -0.512919 ),
   	vec3( -0.293655, 0.427423, 0.078921 ),
   	vec3( -0.267983, 0.680534, -0.132880 ),
   	vec3( 0.139611, 0.319637, 0.477439 ),
   	vec3( -0.352086, 0.311040, 0.653913 ),
   	vec3( 0.321032, 0.805279, 0.487345 ),
   	vec3( 0.073516, 0.820734, -0.414183 ),
   	vec3( -0.155324, 0.589983, -0.411460 ),
   	vec3( 0.335976, 0.170782, -0.527627 ),
   	vec3( 0.463460, -0.355658, -0.167689 ),
   	vec3( 0.222654, 0.596550, -0.769406 ),
   	vec3( 0.922138, -0.042070, 0.147555 ),
   	vec3( -0.727050, -0.329192, 0.369826 ),
   	vec3( -0.090731, 0.533820, 0.463767 ),
   	vec3( -0.323457, -0.876559, -0.238524 ),
   	vec3( -0.663277, -0.372384, -0.342856 )
   );
   
   // Sample a random normal for reflecting the 
   // sphere vector later in our loop.   
   vec4 noiseMapUV = vec4( ( IN_uv1 * ( targetSize / texSize1 ) ).xy, 0, 0 );
   vec3 reflectNormal = normalize( tex2Dlod( randNormalTex, noiseMapUV ).xyz * 2.0 - 1.0 );   
   //return vec4( reflectNormal, 1 );
   
   vec4 prepass = prepassUncondition( prepassMap, IN_uv0 );
   vec3 normal = prepass.xyz;
   float depth = prepass.a;
   //return vec4( ( depth ).xxx, 1 );
      
   // Early out if too far away.
   if ( depth > 0.99999999 )
   {
      OUT_col = vec4( 0,0,0,0 );
      return;
   }

   // current fragment coords in screen space
   vec3 ep = vec3( IN_uv0, depth );        
   
   float bl;
   vec3 baseRay, ray, se, occNorm, projRadius;
   float normalDiff = 0;
   float depthMin, depthMax, dt, depthDiff;    
   vec4 occluderFragment;
   int i;
   float sOcclusion = 0.0;
   float lOcclusion = 0.0;
   
   //------------------------------------------------------------
   // Small radius
   //------------------------------------------------------------   

#ifdef DOSMALL

   bl = 0.0;
   
   projRadius.xy =  ( vec2( sRadius ) / ( depth * nearFar.y ) ) * ( worldToScreenScale / texSize0 );
   projRadius.z = sRadius / nearFar.y;
   
   depthMin = projRadius.z * sDepthMin;
   depthMax = projRadius.z * sDepthMax;
   
   //float maxr = 1;
   //radiusDepth = clamp( radiusDepth, 0.0001, maxr.rrr );   
   //if ( radiusDepth.x < 1.0 / targetSize.x )
   //   return color;      
   //radiusDepth.xyz = 0.0009;
   
   for ( i = 0; i < sSampleCount; i++ )
   {
      baseRay = reflect( ptSphere[i], reflectNormal );
      
      dt = dot( baseRay.xyz, normal );
      
      baseRay *= sign( dt );
         
      ray = ( projRadius * baseRay.xzy );
      ray.y = -ray.y;      
       
      se = ep + ray;
            
      occluderFragment = prepassUncondition( prepassMap, se.xy );                  
      
      depthDiff = se.z - occluderFragment.a; 
      
      dt = dot( occluderFragment.xyz, baseRay.xyz );
      normalDiff = dot( occluderFragment.xyz, normal );        
      
      bl += getOcclusion( depthDiff, depthMin, depthMax, sDepthPow, normalDiff, dt, sNormalTol, sNormalPow );         
   }
   
   sOcclusion = sStrength * ( bl / float(sSampleCount) );

#endif // DOSMALL
   
   
   //------------------------------------------------------------
   // Large radius
   //------------------------------------------------------------
   
#ifdef DOLARGE
      
   bl = 0.0;

   projRadius.xy =  ( vec2( lRadius ) / ( depth * nearFar.y ) ) * ( worldToScreenScale / texSize0 );
   projRadius.z = lRadius / nearFar.y;
   
   depthMin = projRadius.z * lDepthMin;
   depthMax = projRadius.z * lDepthMax;
   
   //projRadius.xy = clamp( projRadius.xy, 0.0, 0.01 );
   //float maxr = 1;
   //radiusDepth = clamp( radiusDepth, 0.0001, maxr.rrr );   
   //if ( radiusDepth.x < 1.0 / targetSize.x )
   //   return color;      
   //radiusDepth.xyz = 0.0009;   
   
   for ( i = sSampleCount; i < totalSampleCount; i++ )
   {
      baseRay = reflect( ptSphere[i], reflectNormal );
      
      dt = dot( baseRay.xyz, normal );
      
      baseRay *= sign( dt );
         
      ray = ( projRadius * baseRay.xzy );
      ray.y = -ray.y;      
       
      se = ep + ray;
            
      occluderFragment = prepassUncondition( prepassMap, se.xy );                  
      
      depthDiff = se.z - occluderFragment.a;       
      
      normalDiff = dot( occluderFragment.xyz, normal );        
      dt = dot( occluderFragment.xyz, baseRay.xyz );         
               
      bl += getOcclusion( depthDiff, depthMin, depthMax, lDepthPow, normalDiff, dt, lNormalTol, lNormalPow );        
   }
      
   lOcclusion = lStrength * ( bl / float( totalSampleCount - sSampleCount ) );

#endif // DOLARGE
   
   float occlusion = saturate( max( sOcclusion, lOcclusion ) * overallStrength );   
   
   // Note black is unoccluded and white is fully occluded.  This
   // seems backwards, but it makes it simple to deal with the SSAO
   // being disabled in the lighting shaders.   
   
   OUT_col = vec4(occlusion, vec3(0.0));
}


