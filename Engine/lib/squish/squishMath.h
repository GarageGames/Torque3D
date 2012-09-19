#ifndef _SQUISH_MATH_H_
#define _SQUISH_MATH_H_

#define FLT_MAX         3.402823466e+38F
#define FLT_EPSILON     1.192092896e-07F
#define INT_MAX       2147483647    /* maximum (signed) int value */

// Abstract the math in squish so it doesn't use std:: directly
namespace SquishMath
{
   float fabs( const float f );
   float pow( const float x, const float y );
   float cos( const float theta );
   float sin( const float theta );
   float sqrt( const float a );
   float atan2( const float a, const float b );

   float min( const float a, const float b );
   float max( const float a, const float b );

   float floor( const float a );
   float ceil( const float a );

   int min( const int a, const int b );
   int max( const int a, const int b );
};

#endif