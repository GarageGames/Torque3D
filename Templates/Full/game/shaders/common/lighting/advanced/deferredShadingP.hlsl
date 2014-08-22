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
#include "../../postfx/postFx.hlsl"
#include "shaders/common/torque.hlsl"


float4 main( PFXVertToPix IN, 
             uniform sampler2D colorBufferTex : register(S0),
             uniform sampler2D lightPrePassTex : register(S1),
             uniform sampler2D matInfoTex : register(S2)) : COLOR0
{        
   float4 lightBuffer = tex2D( lightPrePassTex, IN.uv0 );
   float4 colorBuffer = tex2D( colorBufferTex, IN.uv0 );
   float4 matInfo = tex2D( matInfoTex, IN.uv0 );
   float specular = lightBuffer.a;

   // Diffuse Color Altered by Metalness
   bool metalness = getFlag(matInfo.r, 3);
   if ( metalness )
   {
	colorBuffer *= (1.0 - colorBuffer.a);
   }

   colorBuffer *= float4(lightBuffer.rgb, 1.0);
   colorBuffer += float4(specular, specular, specular, 1.0);

   return hdrEncode( colorBuffer );   
}
