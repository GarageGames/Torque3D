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
#include "./../shaderModelAutoGen.hlsl"

TORQUE_UNIFORM_SAMPLER2D(prepassTex, 0);
uniform float3    eyePosWorld;
uniform float4    fogColor;
uniform float3    fogData;
uniform float4    rtParams0;

float4 main( PFXVertToPix IN ) : TORQUE_TARGET0
{   
   //float2 prepassCoord = ( IN.uv0.xy * rtParams0.zw ) + rtParams0.xy;   
   float depth = TORQUE_PREPASS_UNCONDITION( prepassTex, IN.uv0 ).w;
   //return float4( depth, 0, 0, 0.7 );
   
   float factor = computeSceneFog( eyePosWorld,
                                   eyePosWorld + ( IN.wsEyeRay * depth ),
                                   fogData.x, 
                                   fogData.y, 
                                   fogData.z );

   return hdrEncode( float4( toLinear(fogColor.rgb), 1.0 - saturate( factor ) ) );     
}