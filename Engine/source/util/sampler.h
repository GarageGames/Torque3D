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
#ifndef _SAMPLER_H_
#define _SAMPLER_H_

#include "platform/types.h"

/// The sampling framework.
///
/// Sampling allows per-frame snaphots of specific values to be logged.  For
/// each value that you want to have sampled, you define a sampling key and
/// then simply call the sample function at an appropriate place.  If you
/// want to sample the same value multiple times within a single frame, simply
/// register several keys for it.
///
/// The easiest way to use this facility is with the SAMPLE macro.
///
/// @code
/// SAMPLE( "my/sample/value", my.sample->val );
/// @endcode
///
/// @section SamplerUsage Using the Sampler
///
/// Before you use the sampler it is important that you let your game run for
/// some frames and make sure that all relevant code paths have been touched (i.e.
/// if you want to sample Atlas data, have an Atlas instance on screen).  This
/// will ensure that sampling keys are registered with the sampler.
///
/// Then use the console to first enable the keys you are interested in.  For
/// example, to enable sampling for all Atlas keys:
///
/// @code
/// enableSamples( "atlas/*" );
/// @endcode
///
/// Finally, you have to start the actual sampling.  This is achieved with the
/// beginSampling console function that takes a string informing the backend
/// where to store sample data and optionally a name of the specific logging backend
/// to use.  The default is the CSV backend.  In most cases, the logging store
/// will be a file name.
///
/// @code
/// beginSampling( "mysamples.csv" );
/// @endcode
///
/// To stop sampling, use:
///
/// @code
/// stopSampling();
/// @endcode
///
/// @section Sample Keys
///
/// Sample key name should generally follow the pattern "path/to/group/samplename".
/// This allows to very easily enable or disable specific sets of keys using
/// wildcards.
///
/// Note that sampling keys are case-insensitive.

namespace Sampler
{
   void init();
   void destroy();

   void beginFrame();
   void endFrame();

   void sample( U32 key, bool value );
   void sample( U32 key, S32 value );
   void sample( U32 key, F32 value );
   void sample( U32 key, const char* value );
   
   inline void sample( U32 key, U32 value )
   {
      sample( key, S32( value ) );
   }

   /// Register a new sample key.
   ///
   /// @note Note that all keys are disabled by default.
   U32 registerKey( const char* name );

   /// Enable sampling for all keys that match the given name
   /// pattern.  Slashes are treated as separators.
   void enableKeys( const char* pattern, bool state = true );
};

#ifdef TORQUE_ENABLE_SAMPLING
#  define SAMPLE( name, value )           \
{                                         \
   static U32 key;                        \
   if( !key )                             \
      key = Sampler::registerKey( name ); \
   Sampler::sample( key, value );         \
}
#else
#  define SAMPLE( name, value )
#endif

#define SAMPLE_VECTOR( name, value )      \
{                                         \
   SAMPLE( name "/x", value.x );          \
   SAMPLE( name "/y", value.y );          \
   SAMPLE( name "/z", value.z );          \
}

#define SAMPLE_MATRIX( name, value )                 \
{                                                    \
   SAMPLE( name "/a1", value[ value.idx( 0, 0 ) ] ); \
   SAMPLE( name "/a2", value[ value.idx( 1, 0 ) ] ); \
   SAMPLE( name "/a3", value[ value.idx( 2, 0 ) ] ); \
   SAMPLE( name "/a4", value[ value.idx( 3, 0 ) ] ); \
   SAMPLE( name "/b1", value[ value.idx( 0, 1 ) ] ); \
   SAMPLE( name "/b2", value[ value.idx( 1, 1 ) ] ); \
   SAMPLE( name "/b3", value[ value.idx( 2, 1 ) ] ); \
   SAMPLE( name "/b4", value[ value.idx( 3, 1 ) ] ); \
   SAMPLE( name "/c1", value[ value.idx( 0, 2 ) ] ); \
   SAMPLE( name "/c2", value[ value.idx( 1, 2 ) ] ); \
   SAMPLE( name "/c3", value[ value.idx( 2, 2 ) ] ); \
   SAMPLE( name "/c4", value[ value.idx( 3, 2 ) ] ); \
   SAMPLE( name "/d1", value[ value.idx( 0, 3 ) ] ); \
   SAMPLE( name "/d2", value[ value.idx( 1, 3 ) ] ); \
   SAMPLE( name "/d3", value[ value.idx( 2, 3 ) ] ); \
   SAMPLE( name "/d4", value[ value.idx( 3, 3 ) ] ); \
}

#endif // _SAMPLER_H_
