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

#include "../../torque.hlsl"
#include "../postFx.hlsl"
#include "../../shaderModelAutoGen.hlsl"

TORQUE_UNIFORM_SAMPLER2D(sceneTex, 0);
TORQUE_UNIFORM_SAMPLER2D(luminanceTex, 1);
TORQUE_UNIFORM_SAMPLER2D(bloomTex, 2);
TORQUE_UNIFORM_SAMPLER1D(colorCorrectionTex, 3);
TORQUE_UNIFORM_SAMPLER2D(prepassTex, 4);

uniform float2 texSize0;
uniform float2 texSize2;

uniform float g_fEnableToneMapping;
uniform float g_fMiddleGray;
uniform float g_fWhiteCutoff;
uniform float g_fEnableBlueShift;

uniform float3 g_fBlueShiftColor;
uniform float g_fBloomScale;

uniform float g_fOneOverGamma;
uniform float Brightness;
uniform float Contrast;

float4 main( PFXVertToPix IN ) : TORQUE_TARGET0
{
   float4 sample = hdrDecode( TORQUE_TEX2D( sceneTex, IN.uv0 ) );
   float adaptedLum = TORQUE_TEX2D( luminanceTex, float2( 0.5f, 0.5f ) ).r;
   float4 bloom = TORQUE_TEX2D( bloomTex, IN.uv0 );

   // For very low light conditions, the rods will dominate the perception
   // of light, and therefore color will be desaturated and shifted
   // towards blue.
   if ( g_fEnableBlueShift > 0.0f )
   {
      const float3 LUMINANCE_VECTOR = float3(0.2125f, 0.7154f, 0.0721f);

      // Define a linear blending from -1.5 to 2.6 (log scale) which
      // determines the lerp amount for blue shift
      float coef = 1.0f - ( adaptedLum + 1.5 ) / 4.1;
      coef = saturate( coef * g_fEnableBlueShift );

      // Lerp between current color and blue, desaturated copy
      float3 rodColor = dot( sample.rgb, LUMINANCE_VECTOR ) * g_fBlueShiftColor;
      sample.rgb = lerp( sample.rgb, rodColor, coef );
	  
      rodColor = dot( bloom.rgb, LUMINANCE_VECTOR ) * g_fBlueShiftColor;
      bloom.rgb = lerp( bloom.rgb, rodColor, coef );
   }

   // Map the high range of color values into a range appropriate for
   // display, taking into account the user's adaptation level, 
   // white point, and selected value for for middle gray.
   if ( g_fEnableToneMapping > 0.0f )
   {
      float Lp = (g_fMiddleGray / (adaptedLum + 0.0001)) * hdrLuminance( sample.rgb );
      //float toneScalar = ( Lp * ( 1.0 + ( Lp / ( g_fWhiteCutoff ) ) ) ) / ( 1.0 + Lp );
	  float toneScalar = Lp;
      sample.rgb = lerp( sample.rgb, sample.rgb * toneScalar, g_fEnableToneMapping );
   }

   // Add the bloom effect.
   float depth = TORQUE_PREPASS_UNCONDITION( prepassTex, IN.uv0 ).w;
   if (depth>0.9999)
      sample += g_fBloomScale * bloom;

   // Apply the color correction.
   sample.r = TORQUE_TEX1D( colorCorrectionTex, sample.r ).r;
   sample.g = TORQUE_TEX1D( colorCorrectionTex, sample.g ).g;
   sample.b = TORQUE_TEX1D( colorCorrectionTex, sample.b ).b;

	  
   // Apply gamma correction
   sample.rgb = pow( abs(sample.rgb), g_fOneOverGamma );
 
   // Apply contrast
   sample.rgb = ((sample.rgb - 0.5f) * Contrast) + 0.5f;
 
   // Apply brightness
   sample.rgb += Brightness;

   return sample;
}
