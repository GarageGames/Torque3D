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

#include "platform/platform.h"
#include "util/sampler.h"

#include "core/util/safeDelete.h"
#include "core/util/tVector.h"
#include "core/stream/fileStream.h"
#include "console/console.h"
#include "console/consoleTypes.h"

/// Bookkeeping structure for registered sampling keys.

struct SampleKey
{
   bool		mEnabled;
   const char*	mName;

   bool matchesPattern( const char* pattern )
   {
      U32 indexInName = 0;
      U32 indexInPattern = 0;

      while( mName[ indexInName ] != '\0' )
      {
         if( pattern[ indexInPattern ] == '\0' )
            break;
         else if( dToupper( mName[ indexInName ] ) == dToupper( pattern[ indexInPattern ] ) )
         {
            indexInName ++;
            indexInPattern ++;
         }
         else if( pattern[ indexInPattern ] == '*' )
         {
            // Handle senseless concatenation of wildcards.
            while( pattern[ indexInPattern ] == '*' )
               indexInPattern ++;

            // Skip to next slash in name.
            while( mName[ indexInName ] && mName[ indexInName ] != '/' )
               indexInName ++;
         }
         else
            return false;
      }

      return ( pattern[ indexInPattern ] == '\0'
         || ( indexInPattern > 0 && pattern[ indexInPattern ] == '*' ) );
   }
};

/// A sampler backend is responsible for storing the actual sampling data.

struct ISamplerBackend
{
   virtual ~ISamplerBackend() {}

   virtual bool init( const char* location ) = 0;
   virtual void beginFrame() = 0;
   virtual void endFrame() = 0;

   virtual void sample( U32 key, bool value ) = 0;
   virtual void sample( U32 key, S32 value ) = 0;
   virtual void sample( U32 key, F32 value ) = 0;
   virtual void sample( U32 key, const char* value ) = 0;
};

static bool					gSamplerRunning;
static S32					gSamplingFrequency = 1;	///< Frequency = samples taken every nth frame.
static U32					gCurrentFrameDelta;
static Vector< SampleKey >	gSampleKeys( __FILE__, __LINE__ );
static ISamplerBackend*		gSamplerBackend;

//--------------------------------------------------------------------------------
//	CSV Backend.
//--------------------------------------------------------------------------------

/// A sampler backend that outputs samples to a CSV file.

class CSVSamplerBackend : public ISamplerBackend
{
   /// Value holder for an individual sample.  Unfortunately, since the
   /// order in which samples arrive at the sampler may vary from frame to
   /// frame, we cannot emit data immediately but rather have to buffer
   /// it in these sample records and then flush them to disk once we receive
   /// the endFrame call.
   struct SampleRecord
   {
      U32			mKey;
      U32			mType;		//< Console type code.
      bool		mSet;
      union
      {
         bool		mBool;
         S32			mS32;
         F32			mF32;
         const char*	mString;
      } mValue;

      SampleRecord() {}
      SampleRecord( U32 key )
         : mKey( key ), mSet( false ) {}

      void set( bool value )
      {
         mType = TypeBool;
         mValue.mBool = value;
         mSet = true;
      }
      void set( S32 value )
      {
         mType = TypeS32;
         mValue.mS32 = value;
         mSet = true;
      }
      void set( F32 value )
      {
         mType = TypeF32;
         mValue.mF32 = value;
         mSet = true;
      }
      void set( const char* str )
      {
         mType = TypeString;
         mValue.mString = dStrdup( str );
         mSet = true;
      }

      void clean()
      {
         if( mType == TypeString )
            dFree( ( void* ) mValue.mString );
         mSet = false;
      }
   };

   FileStream				mStream;
   Vector< SampleRecord >	mRecords;

   ~CSVSamplerBackend()
   {
      mStream.close();
   }

   /// Open the file and emit a row with the names of all enabled keys.
   virtual bool init( const char* fileName )
   {
      if( !mStream.open( fileName, Torque::FS::File::Write ) )
      {
         Con::errorf( "CSVSamplerBackend::init -- could not open '%s' for writing", fileName );
         return false;
      }

      Con::printf( "CSVSamplerBackend::init -- writing samples to '%s'", fileName );

      bool first = true;
      for( U32 i = 0; i < gSampleKeys.size(); ++ i )
      {
         SampleKey& key = gSampleKeys[ i ];
         if( key.mEnabled )
         {
            if( !first )
               mStream.write( 1, "," );

            mRecords.push_back( SampleRecord( i + 1 ) );
            mStream.write( dStrlen( key.mName ), key.mName );
            first = false;
         }
      }

      newline();
      return true;
   }

   virtual void beginFrame()
   {
   }

   virtual void endFrame()
   {
      char buffer[ 256 ];

      for( U32 i = 0; i < mRecords.size(); ++ i )
      {
         if( i != 0 )
            mStream.write( 1, "," );

         SampleRecord& record = mRecords[ i ];
         if( record.mSet )
         {
            if( record.mType == TypeBool )
            {
               if( record.mValue.mBool )
                  mStream.write( 4, "true" );
               else
                  mStream.write( 5, "false" );
            }
            else if( record.mType == TypeS32 )
            {
               dSprintf( buffer, sizeof( buffer ), "%d", record.mValue.mS32 );
               mStream.write( dStrlen( buffer ), buffer );
            }
            else if( record.mType == TypeF32 )
            {
               dSprintf( buffer, sizeof( buffer ), "%f", record.mValue.mF32 );
               mStream.write( dStrlen( buffer ), buffer );
            }
            else if( record.mType == TypeString )
            {
               //FIXME: does not do doubling of double quotes in the string at the moment
               mStream.write( 1, "\"" );
               mStream.write( dStrlen( record.mValue.mString ), record.mValue.mString );
               mStream.write( 1, "\"" );
            }
            else
               AssertWarn( false, "CSVSamplerBackend::endFrame - bug: invalid sample type" );
         }

         record.clean();
      }

      newline();
   }

   void newline()
   {
      mStream.write( 1, "\n" );
   }

   SampleRecord* lookup( U32 key )
   {
      //TODO: do this properly with a binary search (the mRecords array is already sorted by key)

      for( U32 i = 0; i < mRecords.size(); ++ i )
         if( mRecords[ i ].mKey == key )
            return &mRecords[ i ];

      AssertFatal( false, "CSVSamplerBackend::lookup - internal error: sample key not found" );
      return NULL; // silence compiler
   }

   virtual void sample( U32 key, bool value )
   {
      lookup( key )->set( value );
   }
   virtual void sample( U32 key, S32 value )
   {
      lookup( key )->set( value );
   }
   virtual void sample( U32 key, F32 value )
   {
      lookup( key )->set( value );
   }
   virtual void sample( U32 key, const char* value )
   {
      lookup( key )->set( value );
   }
};

//--------------------------------------------------------------------------------
//	Internal Functions.
//--------------------------------------------------------------------------------

static void stopSampling()
{
   if( gSamplerRunning )
   {
      SAFE_DELETE( gSamplerBackend );
      gSamplerRunning = false;
   }
}

static void beginSampling( const char* location, const char* backend )
{
   if( gSamplerRunning )
      stopSampling();

   if( dStricmp( backend, "CSV" ) == 0 )
      gSamplerBackend = new CSVSamplerBackend;
   else
   {
      Con::errorf( "beginSampling -- No backend called '%s'", backend );
      return;
   }

   if( !gSamplerBackend->init( location ) )
   {
      SAFE_DELETE( gSamplerBackend );
   }
   else
   {
      gSamplerRunning = true;
      gCurrentFrameDelta = 0;
   }
}

//--------------------------------------------------------------------------------
//	Sampler Functions.
//--------------------------------------------------------------------------------

void Sampler::init()
{
   Con::addVariable( "Sampler::frequency", TypeS32, &gSamplingFrequency, "Samples taken every nth frame.\n"
	   "@ingroup Rendering");
}

void Sampler::beginFrame()
{
   gCurrentFrameDelta ++;
   if( gSamplerBackend && gCurrentFrameDelta == gSamplingFrequency )
      gSamplerBackend->beginFrame();
}

void Sampler::endFrame()
{
   if( gSamplerBackend && gCurrentFrameDelta == gSamplingFrequency )
   {
      gSamplerBackend->endFrame();
      gCurrentFrameDelta = 0;
   }
}

void Sampler::destroy()
{
   if( gSamplerBackend )
      SAFE_DELETE( gSamplerBackend );
}

U32 Sampler::registerKey( const char* name )
{
   gSampleKeys.push_back( SampleKey() );
   U32 index = gSampleKeys.size();
   SampleKey& key = gSampleKeys.last();

   key.mName = name;
   key.mEnabled = false;

   return index;
}

void Sampler::enableKeys( const char* pattern, bool state )
{
   if( gSamplerRunning )
   {
      Con::errorf( "Sampler::enableKeys -- cannot change key states while sampling" );
      return;
   }

   for( U32 i = 0; i < gSampleKeys.size(); ++ i )
      if( gSampleKeys[ i ].matchesPattern( pattern ) )
      {
         gSampleKeys[ i ].mEnabled = state;
         Con::printf( "Sampler::enableKeys -- %s %s", state ? "enabling" : "disabling",
            gSampleKeys[ i ].mName );
      }
}

#define SAMPLE_FUNC( type )                           \
   void Sampler::sample( U32 key, type value )        \
{                                                     \
   if( gSamplerRunning                                \
   && gCurrentFrameDelta == gSamplingFrequency        \
   && gSampleKeys[ key - 1 ].mEnabled )               \
   gSamplerBackend->sample( key, value );             \
}

SAMPLE_FUNC( bool );
SAMPLE_FUNC( S32 );
SAMPLE_FUNC( F32 );
SAMPLE_FUNC( const char* );

//--------------------------------------------------------------------------------
//	Console Functions.
//--------------------------------------------------------------------------------

ConsoleFunction( beginSampling, void, 2, 3, "(location, [backend]) -"
				"@brief Takes a string informing the backend where to store "
				"sample data and optionally a name of the specific logging "
				"backend to use.  The default is the CSV backend. In most "
				"cases, the logging store will be a file name."
				"@tsexample\n"
				"beginSampling( \"mysamples.csv\" );\n"
				"@endtsexample\n\n"
				"@ingroup Rendering")
{
   const char* location = argv[ 1 ];
   const char* backend = "CSV";
   if( argc > 2 )
      backend = argv[ 2 ];

   beginSampling( location, backend );
}

ConsoleFunction( stopSampling, void, 1, 1, "()"
				"@brief Stops the rendering sampler\n\n"
				"@ingroup Rendering\n")
{
   stopSampling();
}

ConsoleFunction( enableSamples, void, 2, 3, "(pattern, [state]) -"
				"@brief Enable sampling for all keys that match the given name "
				"pattern. Slashes are treated as separators.\n\n"
				"@ingroup Rendering")
{
   const char* pattern = argv[ 1 ];
   bool state = true;
   if( argc > 2 )
      state = dAtob( argv[ 2 ] );

   Sampler::enableKeys( pattern, state );
}
