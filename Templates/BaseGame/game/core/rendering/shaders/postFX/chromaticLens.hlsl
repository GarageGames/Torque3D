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

// Based on 'Cubic Lens Distortion HLSL Shader' by François Tarlier
// www.francois-tarlier.com/blog/index.php/2009/11/cubic-lens-distortion-shader

#include "./postFx.hlsl"
#include "./../torque.hlsl"

TORQUE_UNIFORM_SAMPLER2D(backBuffer, 0);
uniform float distCoeff;
uniform float cubeDistort;
uniform float3 colorDistort;


float4 main( PFXVertToPix IN ) : TORQUE_TARGET0
{
    float2 tex = IN.uv0;

    float f = 0;
    float r2 = (tex.x - 0.5) * (tex.x - 0.5) + (tex.y - 0.5) * (tex.y - 0.5);       

    // Only compute the cubic distortion if necessary.
    if ( cubeDistort == 0.0 )
        f = 1 + r2 * distCoeff;
    else
        f = 1 + r2 * (distCoeff + cubeDistort * sqrt(r2));

    // Distort each color channel seperately to get a chromatic distortion effect.
    float3 outColor;
    float3 distort = f.xxx + colorDistort;

    for ( int i=0; i < 3; i++ )
    {
        float x = distort[i] * ( tex.x - 0.5 ) + 0.5;
        float y = distort[i] * ( tex.y - 0.5 ) + 0.5;
        outColor[i] = TORQUE_TEX2DLOD( backBuffer, float4(x,y,0,0) )[i];
    }

    return float4( outColor.rgb, 1 );
}