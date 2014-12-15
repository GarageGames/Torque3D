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

#ifndef _TORQUE_GLSL_
#define _TORQUE_GLSL_


float M_HALFPI_F   = 1.57079632679489661923;
float M_PI_F       = 3.14159265358979323846;
float M_2PI_F      = 6.28318530717958647692;

/// Calculate fog based on a start and end positions in worldSpace.
float computeSceneFog(  vec3 startPos,
                        vec3 endPos,
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
float computeSceneFog( vec3 startPos,
                       vec3 endPos,
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


/// Convert a vec4 uv in viewport space to render target space.
vec2 viewportCoordToRenderTarget( vec4 inCoord, vec4 rtParams )
{   
   vec2 outCoord = inCoord.xy / inCoord.w;
   outCoord = ( outCoord * rtParams.zw ) + rtParams.xy;  
   return outCoord;
}


/// Convert a vec2 uv in viewport space to render target space.
vec2 viewportCoordToRenderTarget( vec2 inCoord, vec4 rtParams )
{   
   vec2 outCoord = ( inCoord * rtParams.zw ) + rtParams.xy;  
   return outCoord;
}


/// Convert a vec4 quaternion into a 3x3 matrix.
mat3x3 quatToMat( vec4 quat )
{
   float xs = quat.x * 2.0;
   float ys = quat.y * 2.0;
   float zs = quat.z * 2.0;

   float wx = quat.w * xs;
   float wy = quat.w * ys;
   float wz = quat.w * zs;
   
   float xx = quat.x * xs;
   float xy = quat.x * ys;
   float xz = quat.x * zs;
   
   float yy = quat.y * ys;
   float yz = quat.y * zs;
   float zz = quat.z * zs;
   
   mat3x3 mat;
   
   mat[0][0] = 1.0 - (yy + zz);
   mat[1][0] = xy - wz;
   mat[2][0] = xz + wy;

   mat[0][1] = xy + wz;
   mat[1][1] = 1.0 - (xx + zz);
   mat[2][1] = yz - wx;

   mat[0][2] = xz - wy;
   mat[1][2] = yz + wx;
   mat[2][2] = 1.0 - (xx + yy);   

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
vec2 parallaxOffset( sampler2D texMap, vec2 texCoord, vec3 negViewTS, float depthScale )
{
   float depth = texture( texMap, texCoord ).a;
   vec2 offset = negViewTS.xy * ( depth * depthScale );

   for ( int i=0; i < PARALLAX_REFINE_STEPS; i++ )
   {
      depth = ( depth + texture( texMap, texCoord + offset ).a ) * 0.5;
      offset = negViewTS.xy * ( depth * depthScale );
   }

   return offset;
}


/// The maximum value for 16bit per component integer HDR encoding.
const float HDR_RGB16_MAX = 100.0;
/// The maximum value for 10bit per component integer HDR encoding.
const float HDR_RGB10_MAX = 4.0;

/// Encodes an HDR color for storage into a target.
vec3 hdrEncode( vec3 _sample )
{
   #if defined( TORQUE_HDR_RGB16 )

      return _sample / HDR_RGB16_MAX;

   #elif defined( TORQUE_HDR_RGB10 ) 

      return _sample / HDR_RGB10_MAX;

   #else

      // No encoding.
      return _sample;

   #endif
}

/// Encodes an HDR color for storage into a target.
vec4 hdrEncode( vec4 _sample )
{
   return vec4( hdrEncode( _sample.rgb ), _sample.a );
}

/// Decodes an HDR color from a target.
vec3 hdrDecode( vec3 _sample )
{
   #if defined( TORQUE_HDR_RGB16 )

      return _sample * HDR_RGB16_MAX;

   #elif defined( TORQUE_HDR_RGB10 )

      return _sample * HDR_RGB10_MAX;

   #else

      // No encoding.
      return _sample;

   #endif
}

/// Decodes an HDR color from a target.
vec4 hdrDecode( vec4 _sample )
{
   return vec4( hdrDecode( _sample.rgb ), _sample.a );
}

/// Returns the luminance for an HDR pixel.
float hdrLuminance( vec3 _sample )
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
   //float lum = max( _sample.r, max( _sample.g, _sample.b ) );

   ////////////////////////////////////////////////////////////////////////////
   // The perceptual relative luminance.
   //
   // See http://en.wikipedia.org/wiki/Luminance_(relative)
   //
   const vec3 RELATIVE_LUMINANCE = vec3( 0.2126, 0.7152, 0.0722 );
   float lum = dot( _sample, RELATIVE_LUMINANCE );
  
   ////////////////////////////////////////////////////////////////////////////
   //
   // The average component luminance.
   //
   //const vec3 AVERAGE_LUMINANCE = vec3( 0.3333, 0.3333, 0.3333 );
   //float lum = dot( _sample, AVERAGE_LUMINANCE );

   return lum;
}

#ifdef TORQUE_PIXEL_SHADER
/// Called from the visibility feature to do screen
/// door transparency for fading of objects.
void fizzle(vec2 vpos, float visibility)
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
   
   mat2x2 m = mat2x2( vpos.x, vpos.y, 0.916, 0.350 );
   if( (visibility - fract( determinant( m ) )) < 0 ) //if(a < 0) discard;
      discard;
}
#endif //TORQUE_PIXEL_SHADER

/// Basic assert macro.  If the condition fails, then the shader will output color.
/// @param condition This should be a bvec[2-4].  If any items is false, condition is considered to fail.
/// @param color The color that should be outputted if the condition fails.
/// @note This macro will only work in the void main() method of a pixel shader.
#define assert(condition, color) { if(!any(condition)) { OUT_col = color; return; } }

#endif // _TORQUE_GLSL_
