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

// These are set by the game engine.  
// The render target size is one-quarter the scene rendering size.  
uniform sampler2D colorSampler;  
uniform sampler2D depthSampler;  
uniform vec2 dofEqWorld;  
uniform float depthOffset;
uniform vec2 targetSize;
uniform float maxWorldCoC;
//uniform vec2 dofEqWeapon;  
//uniform vec2 dofRowDelta;  // vec2( 0, 0.25 / renderTargetHeight )  

in vec2 tcColor0;
#define IN_tcColor0 tcColor0
in vec2 tcColor1;
#define IN_tcColor1 tcColor1
in vec2 tcDepth0;
#define IN_tcDepth0 tcDepth0
in vec2 tcDepth1;
#define IN_tcDepth1 tcDepth1
in vec2 tcDepth2;
#define IN_tcDepth2 tcDepth2
in vec2 tcDepth3;
#define IN_tcDepth3 tcDepth3

out vec4 OUT_col;

void main()
{  
   //return vec4( 1.0, 0.0, 1.0, 1.0 );
   
   vec2 dofRowDelta = vec2( 0, 0.25 / targetSize.y );
   
   //vec2 dofEqWorld = vec2( -60, 1.0 );
   
   half3 color;  
   half maxCoc;  
   vec4 depth;  
   half4 viewCoc;  
   half4 sceneCoc;  
   half4 curCoc;  
   half4 coc;  
   vec2 rowOfs[4];  
   
   // "rowOfs" reduces how many moves PS2.0 uses to emulate swizzling.  
   rowOfs[0] = vec2(0);  
   rowOfs[1] = dofRowDelta.xy;  
   rowOfs[2] = dofRowDelta.xy * 2;  
   rowOfs[3] = dofRowDelta.xy * 3;  
   
   // Use bilinear filtering to average 4 color samples for free.  
   color = half3(0);  
   color += texture( colorSampler, IN_tcColor0.xy + rowOfs[0] ).rgb;  
   color += texture( colorSampler, IN_tcColor1.xy + rowOfs[0] ).rgb;  
   color += texture( colorSampler, IN_tcColor0.xy + rowOfs[2] ).rgb;  
   color += texture( colorSampler, IN_tcColor1.xy + rowOfs[2] ).rgb;  
   color /= 4;  
   
   // Process 4 samples at a time to use vector hardware efficiently.  
   // The CoC will be 1 if the depth is negative, so use "min" to pick  
   // between "sceneCoc" and "viewCoc".  
         
   coc = half4(0);
   for ( int i = 0; i < 4; i++ )
   {
      depth[0] = prepassUncondition( depthSampler, ( IN_tcDepth0.xy + rowOfs[i] ) ).w;
      depth[1] = prepassUncondition( depthSampler, ( IN_tcDepth1.xy + rowOfs[i] ) ).w;
      depth[2] = prepassUncondition( depthSampler, ( IN_tcDepth2.xy + rowOfs[i] ) ).w;
      depth[3] = prepassUncondition( depthSampler, ( IN_tcDepth3.xy + rowOfs[i] ) ).w;
      
      // @todo OPENGL INTEL need review
      coc = max( coc, clamp( half4(dofEqWorld.x) * depth + half4(dofEqWorld.y), half4(0.0), half4(maxWorldCoC) ) );  
   }   
   
   /*
   depth[0] = texture( depthSampler, pixel.tcDepth0.xy + rowOfs[0] ).r;  
   depth[1] = texture( depthSampler, pixel.tcDepth1.xy + rowOfs[0] ).r;  
   depth[2] = texture( depthSampler, pixel.tcDepth2.xy + rowOfs[0] ).r;  
   depth[3] = texture( depthSampler, pixel.tcDepth3.xy + rowOfs[0] ).r;  
   viewCoc = saturate( dofEqWeapon.x * -depth + dofEqWeapon.y );  
   sceneCoc = saturate( dofEqWorld.x * depth + dofEqWorld.y ); 
   curCoc = min( viewCoc, sceneCoc );  
   coc = curCoc;  
   
   depth[0] = texture( depthSampler, pixel.tcDepth0.xy + rowOfs[1] ).r;  
   depth[1] = texture( depthSampler, pixel.tcDepth1.xy + rowOfs[1] ).r;  
   depth[2] = texture( depthSampler, pixel.tcDepth2.xy + rowOfs[1] ).r;  
   depth[3] = texture( depthSampler, pixel.tcDepth3.xy + rowOfs[1] ).r;  
   viewCoc = saturate( dofEqWeapon.x * -depth + dofEqWeapon.y );  
   sceneCoc = saturate( dofEqWorld.x * depth + dofEqWorld.y );  
   curCoc = min( viewCoc, sceneCoc );  
   coc = max( coc, curCoc );  
   
   depth[0] = texture( depthSampler, pixel.tcDepth0.xy + rowOfs[2] ).r;  
   depth[1] = texture( depthSampler, pixel.tcDepth1.xy + rowOfs[2] ).r;  
   depth[2] = texture( depthSampler, pixel.tcDepth2.xy + rowOfs[2] ).r;  
   depth[3] = texture( depthSampler, pixel.tcDepth3.xy + rowOfs[2] ).r;  
   viewCoc = saturate( dofEqWeapon.x * -depth + dofEqWeapon.y );  
   sceneCoc = saturate( dofEqWorld.x * depth + dofEqWorld.y );  
   curCoc = min( viewCoc, sceneCoc );  
   coc = max( coc, curCoc );  
   
   depth[0] = texture( depthSampler, pixel.tcDepth0.xy + rowOfs[3] ).r;  
   depth[1] = texture( depthSampler, pixel.tcDepth1.xy + rowOfs[3] ).r;  
   depth[2] = texture( depthSampler, pixel.tcDepth2.xy + rowOfs[3] ).r;  
   depth[3] = texture( depthSampler, pixel.tcDepth3.xy + rowOfs[3] ).r;  
   viewCoc = saturate( dofEqWeapon.x * -depth + dofEqWeapon.y );  
   sceneCoc = saturate( dofEqWorld.x * depth + dofEqWorld.y );  
   curCoc = min( viewCoc, sceneCoc );  
   coc = max( coc, curCoc );  
   */
   
   maxCoc = max( max( coc[0], coc[1] ), max( coc[2], coc[3] ) );  
   
   //OUT_col = half4( 1.0, 0.0, 1.0, 1.0 );
   OUT_col = half4( color, maxCoc );  
   //OUT_col = half4( color, 1.0f );
   //OUT_col = half4( maxCoc.rrr, 1.0 );
}  