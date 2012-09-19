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

#include "../postFx.hlsl"
#include "shadergen:/autogenConditioners.h"

uniform sampler2D inputTex : register(S0);
uniform float2 oneOverTargetSize;
uniform float gaussMultiplier;
uniform float gaussMean;
uniform float gaussStdDev;

#define PI 3.141592654

float computeGaussianValue( float x, float mean, float std_deviation )
{
    // The gaussian equation is defined as such:
    /*    
      -(x - mean)^2
      -------------
      1.0               2*std_dev^2
      f(x,mean,std_dev) = -------------------- * e^
      sqrt(2*pi*std_dev^2)
      
     */

    float tmp = ( 1.0f / sqrt( 2.0f * PI * std_deviation * std_deviation ) );
    float tmp2 = exp( ( -( ( x - mean ) * ( x - mean ) ) ) / ( 2.0f * std_deviation * std_deviation ) );
    return tmp * tmp2;
}

float4 main( PFXVertToPix IN ) : COLOR
{
   float4 color = { 0.0f, 0.0f, 0.0f, 0.0f };
   float offset = 0;
   float weight = 0;
   float x = 0;
   float fI = 0;

   for( int i = 0; i < 9; i++ )
   {
      fI = (float)i;
      offset = (i - 4.0) * oneOverTargetSize.x;
      x = (i - 4.0) / 4.0;
      weight = gaussMultiplier * computeGaussianValue( x, gaussMean, gaussStdDev );
      color += (tex2D( inputTex, IN.uv0 + float2( offset, 0.0f ) ) * weight );
   }
   
   return float4( color.rgb, 1.0f );
}