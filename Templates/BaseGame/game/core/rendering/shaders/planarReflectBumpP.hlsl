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

//-----------------------------------------------------------------------------
// Structures                                                                  
//-----------------------------------------------------------------------------

#include "shaderModel.hlsl"

struct ConnectData
{
   float4 hpos            : TORQUE_POSITION;
   float2 texCoord        : TEXCOORD0;
   float4 tex2            : TEXCOORD1;
};


struct Fragout
{
   float4 col : TORQUE_TARGET0;
};

TORQUE_UNIFORM_SAMPLER2D(texMap, 0);
TORQUE_UNIFORM_SAMPLER2D(refractMap, 1);
TORQUE_UNIFORM_SAMPLER2D(bumpMap, 2);


//-----------------------------------------------------------------------------
// Fade edges of axis for texcoord passed in
//-----------------------------------------------------------------------------
float fadeAxis( float val )
{
   // Fades from 1.0 to 0.0 when less than 0.1
   float fadeLow = saturate( val * 10.0 );
   
   // Fades from 1.0 to 0.0 when greater than 0.9
   float fadeHigh = 1.0 - saturate( (val - 0.9) * 10.0 );

   return fadeLow * fadeHigh;
}


//-----------------------------------------------------------------------------
// Main                                                                        
//-----------------------------------------------------------------------------
Fragout main( ConnectData IN )
{
   Fragout OUT;

   float3 bumpNorm = TORQUE_TEX2D( bumpMap, IN.tex2 ) * 2.0 - 1.0;
   float2 offset = float2( bumpNorm.x, bumpNorm.y );
   float4 texIndex = IN.texCoord;

   // The fadeVal is used to "fade" the distortion at the edges of the screen.
   // This is done so it won't sample the reflection texture out-of-bounds and create artifacts
   // Note - this can be done more efficiently with a texture lookup
   float fadeVal = fadeAxis( texIndex.x / texIndex.w ) * fadeAxis( texIndex.y / texIndex.w );

   const float distortion = 0.2;
   texIndex.xy += offset * distortion * fadeVal;

   float4 reflectColor = TORQUE_TEX2DPROJ( refractMap, texIndex );
   float4 diffuseColor = TORQUE_TEX2D( texMap, IN.tex2 );

   OUT.col = diffuseColor + reflectColor * diffuseColor.a;

   return OUT;
}
