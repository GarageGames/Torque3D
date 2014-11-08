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
#include "../../../gl/torque.glsl"
#include "../../gl/postFX.glsl"

uniform vec4 rtParams0;
uniform vec4 rtParams1;
uniform vec4 rtParams2;
uniform vec4 rtParams3;
uniform vec2 oneOverTargetSize;  
                     
void main()
{        
   /*
   OUT.hpos = IN_pos;
   OUT_uv0 = IN_uv;
   OUT_uv1 = IN_uv;
   OUT_uv2 = IN_uv;
   OUT_uv3 = IN_uv;   
   */
      
   /*
   OUT_hpos = IN_pos;
   OUT_uv0 = IN_uv + rtParams0.xy;
   OUT_uv1 = IN_uv + rtParams1.xy;
   OUT_uv2 = IN_uv + rtParams2.xy;
   OUT_uv3 = IN_uv + rtParams3.xy;  
   */
   
   
   /*
   OUT_hpos = IN_pos;
   OUT_uv0 = IN_uv * rtParams0.zw;
   OUT_uv1 = IN_uv * rtParams1.zw;
   OUT_uv2 = IN_uv * rtParams2.zw;
   OUT_uv3 = IN_uv * rtParams3.zw;  
   */
   
   
   OUT_hpos = IN_pos;
   OUT_uv0 = viewportCoordToRenderTarget( IN_uv, rtParams0 ); 
   OUT_uv1 = viewportCoordToRenderTarget( IN_uv, rtParams1 ); // + vec2( -5, 1 ) * oneOverTargetSize; 
   OUT_uv2 = viewportCoordToRenderTarget( IN_uv, rtParams2 ); 
   OUT_uv3 = viewportCoordToRenderTarget( IN_uv, rtParams3 ); 
   

   OUT_wsEyeRay = IN_wsEyeRay;
   
   correctSSP(gl_Position);
}
