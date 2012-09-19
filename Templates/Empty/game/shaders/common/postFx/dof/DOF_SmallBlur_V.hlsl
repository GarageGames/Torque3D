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

// This vertex and pixel shader applies a 3 x 3 blur to the image in  
// colorMapSampler, which is the same size as the render target.  
// The sample weights are 1/16 in the corners, 2/16 on the edges,  
// and 4/16 in the center.  

#include "./../postFx.hlsl"
#include "./../../torque.hlsl"

struct Vert
{
   float4 position : POSITION;
   float2 texCoords : TEXCOORD0;
};

struct Pixel
{  
   float4 position : POSITION;  
   float4 texCoords : TEXCOORD0;  
};  

uniform float2 oneOverTargetSize;  
uniform float4 rtParams0;

Pixel main( Vert IN )  
{  
   Pixel OUT;  
   const float4 halfPixel = { -0.5, 0.5, -0.5, 0.5 };     
   OUT.position = IN.position; //Transform_ObjectToClip( IN.position );  
   
   //float2 uv = IN.texCoords + rtParams0.xy;
   float2 uv = viewportCoordToRenderTarget( IN.texCoords, rtParams0 );
   OUT.texCoords = uv.xxyy + halfPixel * oneOverTargetSize.xxyy;  
   return OUT;  
}  