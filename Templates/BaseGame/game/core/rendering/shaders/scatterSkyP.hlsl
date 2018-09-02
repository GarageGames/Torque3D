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

#include "shaderModel.hlsl"
#include "torque.hlsl"

struct Conn
{
   float4 hpos : TORQUE_POSITION;
   float4 rayleighColor : TEXCOORD0;
   float4 mieColor : TEXCOORD1;
   float3 v3Direction : TEXCOORD2;
   float3 pos : TEXCOORD3;
};

TORQUE_UNIFORM_SAMPLERCUBE(nightSky, 0);
uniform float4 nightColor;
uniform float2 nightInterpAndExposure;
uniform float useCubemap;
uniform float3 lightDir;
uniform float3 sunDir;

float4 main( Conn In ) : TORQUE_TARGET0
{ 

   float fCos = dot( lightDir, In.v3Direction ) / length(In.v3Direction);
   float fCos2 = fCos*fCos;
    
   float g = -0.991;
   float g2 = -0.991 * -0.991;

   float fMiePhase = 1.5 * ((1.0 - g2) / (2.0 + g2)) * (1.0 + fCos2) / pow(abs(1.0 + g2 - 2.0*g*fCos), 1.5);
   
   float4 color = In.rayleighColor + fMiePhase * In.mieColor;
   color.a = color.b;
  
   float4 Out; 
   
   float4 nightSkyColor = TORQUE_TEXCUBE(nightSky, -In.v3Direction);
   nightSkyColor = lerp( nightColor, nightSkyColor, useCubemap );
   
   float fac = dot( normalize( In.pos ), sunDir );
   fac = max( nightInterpAndExposure.y, pow( saturate( fac ), 2 ) );
   Out = lerp( color, nightSkyColor, nightInterpAndExposure.y );
   
   Out.a = 1;
   Out = saturate(Out);

   return hdrEncode( Out );
}
