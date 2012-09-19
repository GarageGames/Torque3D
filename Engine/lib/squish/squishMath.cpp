#include "squishMath.h"
#include <math.h>

float SquishMath::fabs( const float f )
{
   return ::fabs( f );
}

float SquishMath::pow( const float x, const float y )
{
   return ::pow( x, y );
}

float SquishMath::cos( const float theta )
{
   return ::cos( theta );
}

float SquishMath::sin( const float theta )
{
   return ::sin( theta );
}

float SquishMath::sqrt( const float a )
{
   return ::sqrtf( a );
}

float SquishMath::atan2( const float a, const float b )
{
   return ::atan2f( a, b );
}

float SquishMath::min( const float a, const float b )
{
   return a < b ? a : b;
}

float SquishMath::max( const float a, const float b )
{
   return a < b ? b : a;
}

float SquishMath::floor( const float a )
{
   return ::floorf( a );
}

float SquishMath::ceil( const float a )
{
   return ::ceilf( a );
}

int SquishMath::min( const int a, const int b )
{
   return a < b ? a : b;
}

int SquishMath::max( const int a, const int b )
{
   return a < b ? b : a;
}