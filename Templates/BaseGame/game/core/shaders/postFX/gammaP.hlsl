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
#include "./postFx.hlsl"  
#include "../torque.hlsl"

TORQUE_UNIFORM_SAMPLER2D(backBuffer, 0);
TORQUE_UNIFORM_SAMPLER1D(colorCorrectionTex, 1);

uniform float OneOverGamma;
uniform float Brightness;
uniform float Contrast;

float4 main( PFXVertToPix IN ) : TORQUE_TARGET0  
{
    float4 color = TORQUE_TEX2D(backBuffer, IN.uv0.xy);

   // Apply the color correction.
   color.r = TORQUE_TEX1D( colorCorrectionTex, color.r ).r;
   color.g = TORQUE_TEX1D( colorCorrectionTex, color.g ).g;
   color.b = TORQUE_TEX1D( colorCorrectionTex, color.b ).b;

   // Apply gamma correction
    color.rgb = pow( saturate(color.rgb), OneOverGamma );

   // Apply contrast
   color.rgb = ((color.rgb - 0.5f) * Contrast) + 0.5f;
 
   // Apply brightness
   color.rgb += Brightness;

    return color;    
}