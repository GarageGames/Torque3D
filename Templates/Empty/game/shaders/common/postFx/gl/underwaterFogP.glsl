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
#include "../../gl/torque.glsl"
#include "postFX.glsl"
#include "shadergen:/autogenConditioners.h"

//-----------------------------------------------------------------------------
// Defines                                                                  
//-----------------------------------------------------------------------------

// oceanFogData
#define FOG_DENSITY        waterFogData[0]
#define FOG_DENSITY_OFFSET waterFogData[1]
#define WET_DEPTH          waterFogData[2]
#define WET_DARKENING      waterFogData[3]

//-----------------------------------------------------------------------------
// Uniforms                                                                  
//-----------------------------------------------------------------------------

uniform sampler2D prepassTex ; 
uniform sampler2D backbuffer ;
uniform sampler1D waterDepthGradMap;
uniform vec3    eyePosWorld;
uniform vec3    ambientColor;     
uniform vec4    waterColor;       
uniform vec4    waterFogData;    
uniform vec4    waterFogPlane;    
uniform vec2    nearFar;      
uniform vec4    rtParams0;
uniform float     waterDepthGradMax;

out vec4 OUT_col;

void main() 
{    
   //vec2 prepassCoord = IN_uv0;
   //IN_uv0 = ( IN_uv0.xy * rtParams0.zw ) + rtParams0.xy;
   float depth = prepassUncondition( prepassTex, IN_uv0 ).w;
   //return vec4( depth.rrr, 1 );
   
   // Skip fogging the extreme far plane so that 
   // the canvas clear color always appears.
   //clip( 0.9 - depth );
   
   // We assume that the eye position is below water because
   // otherwise this shader/posteffect should not be active.
   
   depth *= nearFar.y;
   
   vec3 eyeRay = normalize( IN_wsEyeRay );
   
   vec3 rayStart = eyePosWorld;
   vec3 rayEnd = eyePosWorld + ( eyeRay * depth );
   //return vec4( rayEnd, 1 );
   
   vec4 plane = waterFogPlane; //vec4( 0, 0, 1, -waterHeight );
   //plane.w -= 0.15;
   
   float startSide = dot( plane.xyz, rayStart ) + plane.w;
   if ( startSide > 0 )
   {
      rayStart.z -= ( startSide );
      //return vec4( 1, 0, 0, 1 );
   }

   vec3 hitPos;
   vec3 ray = rayEnd - rayStart;
   float rayLen = length( ray );
   vec3 rayDir = normalize( ray );
   
   float endSide = dot( plane.xyz, rayEnd ) + plane.w;     
   float planeDist;
   
   if ( endSide < -0.005 )    
   {  
      //return vec4( 0, 0, 1, 1 );   
      hitPos = rayEnd;
      planeDist = endSide;
   }
   else
   {   
      //return vec4( 0, 0, 0, 0 );
      float den = dot( ray, plane.xyz );
      
      // Parallal to the plane, return the endPnt.
      //if ( den == 0.0f )
      //   return endPnt;          
      
      float dist = -( dot( plane.xyz, rayStart ) + plane.w ) / den;  
      if ( dist < 0.0 )         
         dist = 0.0;
         //return vec4( 1, 0, 0, 1 );
      //return vec4( ( dist ).rrr, 1 );
              
         
      hitPos = mix( rayStart, rayEnd, dist );
      
      planeDist = dist;
   }
      
   float delta = length( hitPos - rayStart );  
      
   float fogAmt = 1.0 - saturate( exp( -FOG_DENSITY * ( delta - FOG_DENSITY_OFFSET ) ) );   
   //return vec4( fogAmt.rrr, 1 );

   // Calculate the water "base" color based on depth.
   vec4 fogColor = waterColor * texture( waterDepthGradMap, saturate( delta / waterDepthGradMax ) );      
   // Modulate baseColor by the ambientColor.
   fogColor *= vec4( ambientColor.rgb, 1 );
   
   vec3 inColor = hdrDecode( texture( backbuffer, IN_uv0 ).rgb );
   inColor.rgb *= 1.0 - saturate( abs( planeDist ) / WET_DEPTH ) * WET_DARKENING;
   //return vec4( inColor, 1 );
   
   vec3 outColor = mix( inColor, fogColor.rgb, fogAmt );
   
   OUT_col = vec4( hdrEncode( outColor ), 1 );        
}