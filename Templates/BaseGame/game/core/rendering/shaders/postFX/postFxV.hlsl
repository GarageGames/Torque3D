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

#include "./postFx.hlsl"
#include "./../torque.hlsl"


uniform float4 rtParams0;
uniform float4 rtParams1;
uniform float4 rtParams2;
uniform float4 rtParams3;
                    
PFXVertToPix main( PFXVert IN )
{
   PFXVertToPix OUT;
   
   OUT.hpos = float4(IN.pos,1.0);
   OUT.uv0 = viewportCoordToRenderTarget( IN.uv, rtParams0 ); 
   OUT.uv1 = viewportCoordToRenderTarget( IN.uv, rtParams1 ); 
   OUT.uv2 = viewportCoordToRenderTarget( IN.uv, rtParams2 ); 
   OUT.uv3 = viewportCoordToRenderTarget( IN.uv, rtParams3 ); 

   OUT.wsEyeRay = IN.wsEyeRay;
   
   return OUT;
}
