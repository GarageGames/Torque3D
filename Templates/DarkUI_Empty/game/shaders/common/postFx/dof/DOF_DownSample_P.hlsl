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

// These are set by the game engine.  
// The render target size is one-quarter the scene rendering size.  
uniform sampler2D colorSampler : register(S0);  
uniform sampler2D depthSampler : register(S1);  
uniform float2 dofEqWorld;  
uniform float depthOffset;
uniform float2 targetSize;
uniform float maxWorldCoC;
//uniform float2 dofEqWeapon;  
//uniform float2 dofRowDelta;  // float2( 0, 0.25 / renderTargetHeight )  

struct Pixel
{  
   float4 position : POSITION;  
   float2 tcColor0 : TEXCOORD0;  
   float2 tcColor1 : TEXCOORD1;  
   float2 tcDepth0 : TEXCOORD2;  
   float2 tcDepth1 : TEXCOORD3;  
   float2 tcDepth2 : TEXCOORD4;  
   float2 tcDepth3 : TEXCOORD5;  
};  

half4 main( Pixel IN ) : COLOR  
{  
   //return float4( 1.0, 0.0, 1.0, 1.0 );
   
   float2 dofRowDelta = float2( 0, 0.25 / targetSize.y );
   
   //float2 dofEqWorld = float2( -60, 1.0 );
   
   half3 color;  
   half maxCoc;  
   float4 depth;  
   half4 viewCoc;  
   half4 sceneCoc;  
   half4 curCoc;  
   half4 coc;  
   float2 rowOfs[4];  
   
   // "rowOfs" reduces how many moves PS2.0 uses to emulate swizzling.  
   rowOfs[0] = 0;  
   rowOfs[1] = dofRowDelta.xy;  
   rowOfs[2] = dofRowDelta.xy * 2;  
   rowOfs[3] = dofRowDelta.xy * 3;  
   
   // Use bilinear filtering to average 4 color samples for free.  
   color = 0;  
   color += tex2D( colorSampler, IN.tcColor0.xy + rowOfs[0] ).rgb;  
   color += tex2D( colorSampler, IN.tcColor1.xy + rowOfs[0] ).rgb;  
   color += tex2D( colorSampler, IN.tcColor0.xy + rowOfs[2] ).rgb;  
   color += tex2D( colorSampler, IN.tcColor1.xy + rowOfs[2] ).rgb;  
   color /= 4;  
   
   // Process 4 samples at a time to use vector hardware efficiently.  
   // The CoC will be 1 if the depth is negative, so use "min" to pick  
   // between "sceneCoc" and "viewCoc".  
         
   for ( int i = 0; i < 4; i++ )
   {
      depth[0] = prepassUncondition( depthSampler, float4( IN.tcDepth0.xy + rowOfs[i], 0, 0 ) ).w;
      depth[1] = prepassUncondition( depthSampler, float4( IN.tcDepth1.xy + rowOfs[i], 0, 0 ) ).w;
      depth[2] = prepassUncondition( depthSampler, float4( IN.tcDepth2.xy + rowOfs[i], 0, 0 ) ).w;
      depth[3] = prepassUncondition( depthSampler, float4( IN.tcDepth3.xy + rowOfs[i], 0, 0 ) ).w;
      coc[i] = clamp( dofEqWorld.x * depth + dofEqWorld.y, 0.0, maxWorldCoC );  
   }   
   
   /*
   depth[0] = tex2D( depthSampler, pixel.tcDepth0.xy + rowOfs[0] ).r;  
   depth[1] = tex2D( depthSampler, pixel.tcDepth1.xy + rowOfs[0] ).r;  
   depth[2] = tex2D( depthSampler, pixel.tcDepth2.xy + rowOfs[0] ).r;  
   depth[3] = tex2D( depthSampler, pixel.tcDepth3.xy + rowOfs[0] ).r;  
   viewCoc = saturate( dofEqWeapon.x * -depth + dofEqWeapon.y );  
   sceneCoc = saturate( dofEqWorld.x * depth + dofEqWorld.y ); 
   curCoc = min( viewCoc, sceneCoc );  
   coc = curCoc;  
   
   depth[0] = tex2D( depthSampler, pixel.tcDepth0.xy + rowOfs[1] ).r;  
   depth[1] = tex2D( depthSampler, pixel.tcDepth1.xy + rowOfs[1] ).r;  
   depth[2] = tex2D( depthSampler, pixel.tcDepth2.xy + rowOfs[1] ).r;  
   depth[3] = tex2D( depthSampler, pixel.tcDepth3.xy + rowOfs[1] ).r;  
   viewCoc = saturate( dofEqWeapon.x * -depth + dofEqWeapon.y );  
   sceneCoc = saturate( dofEqWorld.x * depth + dofEqWorld.y );  
   curCoc = min( viewCoc, sceneCoc );  
   coc = max( coc, curCoc );  
   
   depth[0] = tex2D( depthSampler, pixel.tcDepth0.xy + rowOfs[2] ).r;  
   depth[1] = tex2D( depthSampler, pixel.tcDepth1.xy + rowOfs[2] ).r;  
   depth[2] = tex2D( depthSampler, pixel.tcDepth2.xy + rowOfs[2] ).r;  
   depth[3] = tex2D( depthSampler, pixel.tcDepth3.xy + rowOfs[2] ).r;  
   viewCoc = saturate( dofEqWeapon.x * -depth + dofEqWeapon.y );  
   sceneCoc = saturate( dofEqWorld.x * depth + dofEqWorld.y );  
   curCoc = min( viewCoc, sceneCoc );  
   coc = max( coc, curCoc );  
   
   depth[0] = tex2D( depthSampler, pixel.tcDepth0.xy + rowOfs[3] ).r;  
   depth[1] = tex2D( depthSampler, pixel.tcDepth1.xy + rowOfs[3] ).r;  
   depth[2] = tex2D( depthSampler, pixel.tcDepth2.xy + rowOfs[3] ).r;  
   depth[3] = tex2D( depthSampler, pixel.tcDepth3.xy + rowOfs[3] ).r;  
   viewCoc = saturate( dofEqWeapon.x * -depth + dofEqWeapon.y );  
   sceneCoc = saturate( dofEqWorld.x * depth + dofEqWorld.y );  
   curCoc = min( viewCoc, sceneCoc );  
   coc = max( coc, curCoc );  
   */
   
   maxCoc = max( max( coc[0], coc[1] ), max( coc[2], coc[3] ) );  
   
   //return half4( 1.0, 0.0, 1.0, 1.0 );
   return half4( color, maxCoc );  
   //return half4( color, 1.0f );
   //return half4( maxCoc.rrr, 1.0 );
}  