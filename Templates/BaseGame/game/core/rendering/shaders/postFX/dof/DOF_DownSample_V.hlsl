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

#include "./../postFx.hlsl"
#include "./../../torque.hlsl"

struct Vert
{
   float3 pos        : POSITION;
   float2 tc         : TEXCOORD0;
   float3 wsEyeRay   : TEXCOORD1;
};

struct Pixel
{  
   float4 position : TORQUE_POSITION;  
   float2 tcColor0 : TEXCOORD0;  
   float2 tcColor1 : TEXCOORD1;  
   float2 tcDepth0 : TEXCOORD2;  
   float2 tcDepth1 : TEXCOORD3;  
   float2 tcDepth2 : TEXCOORD4;  
   float2 tcDepth3 : TEXCOORD5;  
};  

uniform float4    rtParams0;
uniform float2    oneOverTargetSize;  

Pixel main( Vert IN )  
{  
   Pixel OUT; 
   OUT.position = float4(IN.pos,1.0);
   
   float2 uv = viewportCoordToRenderTarget( IN.tc, rtParams0 ); 
   //OUT.position = mul( IN.pos, modelView );  
   OUT.tcColor1 = uv + float2( +1.0, -0.0 ) * oneOverTargetSize;  
   OUT.tcColor0 = uv + float2( -1.0, -0.0 ) * oneOverTargetSize;  
   OUT.tcDepth0 = uv + float2( -0.5, -0.0 ) * oneOverTargetSize;    
   OUT.tcDepth1 = uv + float2( -1.5, -0.0 ) * oneOverTargetSize;    
   OUT.tcDepth2 = uv + float2( +1.5, -0.0 ) * oneOverTargetSize;    
   OUT.tcDepth3 = uv + float2( +2.5, -0.0 ) * oneOverTargetSize;    
   return OUT;
}  