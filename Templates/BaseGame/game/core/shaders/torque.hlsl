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

#ifndef _TORQUE_HLSL_
#define _TORQUE_HLSL_

#include "./shaderModel.hlsl"

static float M_HALFPI_F   = 1.57079632679489661923f;
static float M_PI_F       = 3.14159265358979323846f;
static float M_2PI_F      = 6.28318530717958647692f;


/// Calculate fog based on a start and end positions in worldSpace.
float computeSceneFog(  float3 startPos,
                        float3 endPos,
                        float fogDensity,
                        float fogDensityOffset,
                        float fogHeightFalloff )
{      
   float f = length( startPos - endPos ) - fogDensityOffset;
   float h = 1.0 - ( endPos.z * fogHeightFalloff );  
   return exp( -fogDensity * f * h );  
}


/// Calculate fog based on a start and end position and a height.
/// Positions do not need to be in worldSpace but height does.
float computeSceneFog( float3 startPos,
                       float3 endPos,
                       float height,
                       float fogDensity,
                       float fogDensityOffset,
                       float fogHeightFalloff )
{
   float f = length( startPos - endPos ) - fogDensityOffset;
   float h = 1.0 - ( height * fogHeightFalloff );
   return exp( -fogDensity * f * h );
}


/// Calculate fog based on a distance, height is not used.
float computeSceneFog( float dist, float fogDensity, float fogDensityOffset )
{
   float f = dist - fogDensityOffset;
   return exp( -fogDensity * f );
}


/// Convert a float4 uv in viewport space to render target space.
float2 viewportCoordToRenderTarget( float4 inCoord, float4 rtParams )
{   
   float2 outCoord = inCoord.xy / inCoord.w;
   outCoord = ( outCoord * rtParams.zw ) + rtParams.xy;  
   return outCoord;
}


/// Convert a float2 uv in viewport space to render target space.
float2 viewportCoordToRenderTarget( float2 inCoord, float4 rtParams )
{   
   float2 outCoord = ( inCoord * rtParams.zw ) + rtParams.xy;
   return outCoord;
}


/// Convert a float4 quaternion into a 3x3 matrix.
float3x3 quatToMat( float4 quat )
{
   float xs = quat.x * 2.0f;
   float ys = quat.y * 2.0f;
   float zs = quat.z * 2.0f;

   float wx = quat.w * xs;
   float wy = quat.w * ys;
   float wz = quat.w * zs;
   
   float xx = quat.x * xs;
   float xy = quat.x * ys;
   float xz = quat.x * zs;
   
   float yy = quat.y * ys;
   float yz = quat.y * zs;
   float zz = quat.z * zs;
   
   float3x3 mat;
   
   mat[0][0] = 1.0f - (yy + zz);
   mat[0][1] = xy - wz;
   mat[0][2] = xz + wy;

   mat[1][0] = xy + wz;
   mat[1][1] = 1.0f - (xx + zz);
   mat[1][2] = yz - wx;

   mat[2][0] = xz - wy;
   mat[2][1] = yz + wx;
   mat[2][2] = 1.0f - (xx + yy);   

   return mat;
}


/// The number of additional substeps we take when refining
/// the results of the offset parallax mapping function below.
///
/// You should turn down the number of steps if your needing
/// more performance out of your parallax surfaces.  Increasing
/// the number doesn't yeild much better results and is rarely
/// worth the additional cost.
///
#define PARALLAX_REFINE_STEPS 3

/// Performs fast parallax offset mapping using 
/// multiple refinement steps.
///
/// @param texMap The texture map whos alpha channel we sample the parallax depth.
/// @param texCoord The incoming texture coordinate for sampling the parallax depth.
/// @param negViewTS The negative view vector in tangent space.
/// @param depthScale The parallax factor used to scale the depth result.
///
float2 parallaxOffset(TORQUE_SAMPLER2D(texMap), float2 texCoord, float3 negViewTS, float depthScale)
{
   float depth = TORQUE_TEX2D(texMap, texCoord).a/(PARALLAX_REFINE_STEPS*2);
   float2 offset = negViewTS.xy * (depth * depthScale)/(PARALLAX_REFINE_STEPS);

   for (int i = 0; i < PARALLAX_REFINE_STEPS; i++)
   {
      depth = (depth + TORQUE_TEX2D(texMap, texCoord + offset).a)/(PARALLAX_REFINE_STEPS*2);
      offset = negViewTS.xy * (depth * depthScale)/(PARALLAX_REFINE_STEPS);
   }

   return offset;
}

/// Same as parallaxOffset but for dxtnm where depth is stored in the red channel instead of the alpha
float2 parallaxOffsetDxtnm(TORQUE_SAMPLER2D(texMap), float2 texCoord, float3 negViewTS, float depthScale)
{
   float depth = TORQUE_TEX2D(texMap, texCoord).r/(PARALLAX_REFINE_STEPS*2);
   float2 offset = negViewTS.xy * (depth * depthScale)/(PARALLAX_REFINE_STEPS*2);

   for (int i = 0; i < PARALLAX_REFINE_STEPS; i++)
   {
      depth = (depth + TORQUE_TEX2D(texMap, texCoord + offset).r)/(PARALLAX_REFINE_STEPS*2);
      offset = negViewTS.xy * (depth * depthScale)/(PARALLAX_REFINE_STEPS*2);
   }

   return offset;
}


/// The maximum value for 16bit per component integer HDR encoding.
static const float HDR_RGB16_MAX = 100.0;

/// The maximum value for 10bit per component integer HDR encoding.
static const float HDR_RGB10_MAX = 4.0;

/// Encodes an HDR color for storage into a target.
float3 hdrEncode( float3 sample )
{
   #if defined( TORQUE_HDR_RGB16 )

      return sample / HDR_RGB16_MAX;

   #elif defined( TORQUE_HDR_RGB10 ) 

      return sample / HDR_RGB10_MAX;

   #else

      // No encoding.
      return sample;

   #endif
}

/// Encodes an HDR color for storage into a target.
float4 hdrEncode( float4 sample )
{
   return float4( hdrEncode( sample.rgb ), sample.a );
}

/// Decodes an HDR color from a target.
float3 hdrDecode( float3 sample )
{
   #if defined( TORQUE_HDR_RGB16 )

      return sample * HDR_RGB16_MAX;

   #elif defined( TORQUE_HDR_RGB10 )

      return sample * HDR_RGB10_MAX;

   #else

      // No encoding.
      return sample;

   #endif
}

/// Decodes an HDR color from a target.
float4 hdrDecode( float4 sample )
{
   return float4( hdrDecode( sample.rgb ), sample.a );
}

/// Returns the luminance for an HDR pixel.
float hdrLuminance( float3 sample )
{
   // There are quite a few different ways to
   // calculate luminance from an rgb value.
   //
   // If you want to use a different technique
   // then plug it in here.
   //

   ////////////////////////////////////////////////////////////////////////////
   //
   // Max component luminance.
   //
   //float lum = max( sample.r, max( sample.g, sample.b ) );

   ////////////////////////////////////////////////////////////////////////////
   // The perceptual relative luminance.
   //
   // See http://en.wikipedia.org/wiki/Luminance_(relative)
   //
   const float3 RELATIVE_LUMINANCE = float3( 0.2126, 0.7152, 0.0722 );
   float lum = dot( sample, RELATIVE_LUMINANCE );
  
   ////////////////////////////////////////////////////////////////////////////
   //
   // The average component luminance.
   //
   //const float3 AVERAGE_LUMINANCE = float3( 0.3333, 0.3333, 0.3333 );
   //float lum = dot( sample, AVERAGE_LUMINANCE );

   return lum;
}

/// Called from the visibility feature to do screen
/// door transparency for fading of objects.
void fizzle(float2 vpos, float visibility)
{
   // NOTE: The magic values below are what give us 
   // the nice even pattern during the fizzle.
   //
   // These values can be changed to get different 
   // patterns... some better than others.
   //
   // Horizontal Blinds - { vpos.x, 0.916, vpos.y, 0 }
   // Vertical Lines - { vpos.x, 12.9898, vpos.y, 78.233 }
   //
   // I'm sure there are many more patterns here to 
   // discover for different effects.
   
   float2x2 m = { vpos.x, 0.916, vpos.y, 0.350 };
   clip( visibility - frac( determinant( m ) ) );
}

// Deferred Shading: Material Info Flag Check
bool getFlag(float flags, int num)
{
   int process = round(flags * 255);
   int squareNum = pow(2, num);
   return (fmod(process, pow(2, squareNum)) >= squareNum); 
}

// #define TORQUE_STOCK_GAMMA
#ifdef TORQUE_STOCK_GAMMA
// Sample in linear space. Decodes gamma.
float4 toLinear(float4 tex)
{
   return tex;
}
// Encodes gamma.
float4 toGamma(float4 tex)
{
   return tex;
}
float3 toLinear(float3 tex)
{
   return tex;
}
// Encodes gamma.
float3 toGamma(float3 tex)
{
   return tex;
}
float3 toLinear(float3 tex)
{
   return tex;
}
// Encodes gamma.
float3 toLinear(float3 tex)
{
   return tex;
}
#else
// Sample in linear space. Decodes gamma.
float4 toLinear(float4 tex)
{
   return float4(pow(abs(tex.rgb), 2.2), tex.a);
}
// Encodes gamma.
float4 toGamma(float4 tex)
{
   return float4(pow(abs(tex.rgb), 1.0/2.2), tex.a);
}
// Sample in linear space. Decodes gamma.
float3 toLinear(float3 tex)
{
   return pow(abs(tex.rgb), 2.2);
}
// Encodes gamma.
float3 toGamma(float3 tex)
{
   return pow(abs(tex.rgb), 1.0/2.2);
}
#endif //

#endif // _TORQUE_HLSL_
