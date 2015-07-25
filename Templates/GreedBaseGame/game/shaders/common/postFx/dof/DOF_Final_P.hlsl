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
#include "./../postFx.hlsl"

uniform sampler2D colorSampler      : register(S0); // Original source image  
uniform sampler2D smallBlurSampler  : register(S1); // Output of SmallBlurPS()  
uniform sampler2D largeBlurSampler  : register(S2); // Blurred output of DofDownsample()  
uniform sampler2D depthSampler      : register(S3); // 
uniform float2 oneOverTargetSize;  
uniform float4 dofLerpScale;  
uniform float4 dofLerpBias;  
uniform float3 dofEqFar;  
uniform float maxFarCoC;

//static float d0 = 0.1;
//static float d1 = 0.1;
//static float d2 = 0.8;
//static float4 dofLerpScale = float4( -1.0 / d0, -1.0 / d1, -1.0 / d2, 1.0 / d2 );
//static float4 dofLerpBias = float4( 1.0, (1.0 - d2) / d1, 1.0 / d2, (d2 - 1.0) / d2 );
//static float3 dofEqFar = float3( 2.0, 0.0, 1.0 ); 

float4 tex2Doffset( sampler2D s, float2 tc, float2 offset )  
{  
   return tex2D( s, tc + offset * oneOverTargetSize );  
}  

half3 GetSmallBlurSample( float2 tc )  
{  
   half3 sum;  
   const half weight = 4.0 / 17;  
   sum = 0;  // Unblurred sample done by alpha blending  
   //sum += weight * tex2Doffset( colorSampler, tc, float2( 0, 0 ) ).rgb;
   sum += weight * tex2Doffset( colorSampler, tc, float2( +0.5, -1.5 ) ).rgb;  
   sum += weight * tex2Doffset( colorSampler, tc, float2( -1.5, -0.5 ) ).rgb;  
   sum += weight * tex2Doffset( colorSampler, tc, float2( -0.5, +1.5 ) ).rgb;  
   sum += weight * tex2Doffset( colorSampler, tc, float2( +1.5, +0.5 ) ).rgb;  
   return sum;  
}  

half4 InterpolateDof( half3 small, half3 med, half3 large, half t )  
{  
   //t = 2;
   half4 weights;
   half3 color;  
   half  alpha;  
   
   // Efficiently calculate the cross-blend weights for each sample.  
   // Let the unblurred sample to small blur fade happen over distance  
   // d0, the small to medium blur over distance d1, and the medium to  
   // large blur over distance d2, where d0 + d1 + d2 = 1.  
   //float4 dofLerpScale = float4( -1 / d0, -1 / d1, -1 / d2, 1 / d2 );  
   //float4 dofLerpBias = float4( 1, (1 – d2) / d1, 1 / d2, (d2 – 1) / d2 );  
   
   weights = saturate( t * dofLerpScale + dofLerpBias );  
   weights.yz = min( weights.yz, 1 - weights.xy );  
   
   // Unblurred sample with weight "weights.x" done by alpha blending  
   color = weights.y * small + weights.z * med + weights.w * large;  
   //color = med;
   alpha = dot( weights.yzw, half3( 16.0 / 17, 1.0, 1.0 ) ); 
   //alpha = 0.0;
   
   return half4( color, alpha );  
}  

half4 main( PFXVertToPix IN ) : COLOR
{  
   //return half4( 1,0,1,1 );
   //return half4( tex2D( colorSampler, IN.uv0 ).rgb, 1.0 );
   //return half4( tex2D( colorSampler, texCoords ).rgb, 0 );
   half3 small;  
   half4 med;  
   half3 large;  
   half depth;  
   half nearCoc;  
   half farCoc;  
   half coc;  
   
   small = GetSmallBlurSample( IN.uv0 );  
   //small = half3( 1,0,0 );
   //return half4( small, 1.0 );
   med = tex2D( smallBlurSampler, IN.uv1 );  
   //med.rgb = half3( 0,1,0 );
   //return half4(med.rgb, 0.0);
   large = tex2D( largeBlurSampler, IN.uv2 ).rgb;  
   //large = half3( 0,0,1 );
   //return large;
   //return half4(large.rgb,1.0);
   nearCoc = med.a;  
   
   // Since the med blur texture is screwed up currently
   // replace it with the large, but this needs to be fixed.
   //med.rgb = large;
   
   //nearCoc = 0;
   depth = prepassUncondition( depthSampler, float4( IN.uv3, 0, 0 ) ).w;  
   //return half4(depth.rrr,1);
   //return half4(nearCoc.rrr,1.0);
   
   if (depth > 0.999 )  
   {  
      coc = nearCoc; // We don't want to blur the sky.  
      //coc = 0;
   }  
   else  
   {  
      // dofEqFar.x and dofEqFar.y specify the linear ramp to convert  
      // to depth for the distant out-of-focus region.  
      // dofEqFar.z is the ratio of the far to the near blur radius.  
      farCoc = clamp( dofEqFar.x * depth + dofEqFar.y, 0.0, maxFarCoC );  
      coc = max( nearCoc, farCoc * dofEqFar.z );  
      //coc = nearCoc;
   } 

   //coc = nearCoc;
   //coc = farCoc;
   //return half4(coc.rrr,0.5);
   //return half4(farCoc.rrr,1);
   //return half4(nearCoc.rrr,1);
   
   //return half4( 1,0,1,0 );
   return InterpolateDof( small, med.rgb, large, coc );  
}  