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

#ifndef _SFXFMODDEVICE_H_
#define _SFXFMODDEVICE_H_

#ifndef _SFXDEVICE_H_
   #include "sfx/sfxDevice.h"
#endif
#ifndef _SFXFMODVOICE_H_
   #include "sfx/fmod/sfxFMODVoice.h"
#endif
#ifndef _SFXFMODBUFFER_H_
   #include "sfx/fmod/sfxFMODBuffer.h"
#endif
#ifndef _SFXFMODPLUGIN_H_
   #include "sfx/fmod/sfxFMODPlugin.h"
#endif

#include "core/util/tDictionary.h"


// Disable warning for unused static functions.
#ifdef TORQUE_COMPILER_VISUALC
   #pragma warning( disable : 4505 )
#endif

#if defined( TORQUE_OS_XENON ) || defined( TORQUE_OS_PS3 )
   #define TORQUE_FMOD_STATIC
   #define TORQUE_FMOD_NO_EVENTS //TEMP
#endif


#include "fmod.h"
#include "fmod_errors.h"
#include "fmod_event.h"

#include "platform/platformDlibrary.h"
#include "platform/threads/mutex.h"


// This doesn't appear to exist in some contexts, so let's just add it.
#if defined(TORQUE_OS_WIN) || defined(TORQUE_OS_XENON)
#ifndef WINAPI
#define WINAPI __stdcall
#endif
#else
#define WINAPI
#endif


#define FModAssert(x, msg) \
   { FMOD_RESULT result = ( x ); \
     AssertISV( result == FMOD_OK, String::ToString( "%s: %s", msg, FMOD_ErrorString( result ) ) ); }

#define FMOD_FN_FILE "sfx/fmod/fmodFunctions.h"


// Typedefs
#define FMOD_FUNCTION(fn_name, fn_args) \
   typedef FMOD_RESULT (WINAPI *FMODFNPTR##fn_name)fn_args;
#define FMOD_EVENT_FUNCTION(fn_name, fn_args) \
   typedef FMOD_RESULT (WINAPI *FMODFNPTR##fn_name)fn_args;
#include FMOD_FN_FILE
#undef FMOD_FUNCTION
#undef FMOD_EVENT_FUNCTION


/// FMOD API function table.
///
/// FMOD doesn't want to be called concurrently so in order to
/// not force everything to the main thread (where sound updates
/// would just stall during loading), we thunk all the API
/// calls and lock all API entry points to a single mutex.
struct FModFNTable
{
   FModFNTable()
      : isLoaded( false ),
        eventIsLoaded( false ),
        dllRef( NULL ),
        eventDllRef( NULL )
   {
      AssertFatal( mutex == NULL,
         "FModFNTable::FModFNTable() - this should be a singleton" );
      mutex = new Mutex;
   }
   ~FModFNTable()
   {
      dllRef = NULL;
      eventDllRef = NULL;      
      delete mutex;
   }

   bool isLoaded;
   bool eventIsLoaded;
   DLibraryRef dllRef;
   DLibraryRef eventDllRef;
   static Mutex* mutex;

   template< typename FN >
   struct Thunk
   {
      FN fn;

      template< typename A >
      FMOD_RESULT operator()( A a )
      {
         mutex->lock();
         FMOD_RESULT result = fn( a );
         mutex->unlock();
         return result;
      }
      template< typename A, typename B >
      FMOD_RESULT operator()( A a, B b )
      {
         mutex->lock();
         FMOD_RESULT result = fn( a, b );
         mutex->unlock();
         return result;
      }
      template< typename A, typename B, typename C >
      FMOD_RESULT operator()( A a, B b, C c )
      {
         mutex->lock();
         FMOD_RESULT result = fn( a, b, c );
         mutex->unlock();
         return result;
      }
      template< typename A, typename B, typename C, typename D >
      FMOD_RESULT operator()( A a, B b, C c, D d )
      {
         mutex->lock();
         FMOD_RESULT result = fn( a, b, c, d );
         mutex->unlock();
         return result;
      }
      template< typename A, typename B, typename C, typename D, typename E >
      FMOD_RESULT operator()( A a, B b, C c, D d, E e )
      {
         mutex->lock();
         FMOD_RESULT result = fn( a, b, c, d, e );
         mutex->unlock();
         return result;
      }
      template< typename A, typename B, typename C, typename D, typename E, typename F >
      FMOD_RESULT operator()( A a, B b, C c, D d, E e, F f )
      {
         mutex->lock();
         FMOD_RESULT result = fn( a, b, c, d, e, f );
         mutex->unlock();
         return result;
      }
      template< typename A, typename B, typename C, typename D, typename E, typename F, typename G >
      FMOD_RESULT operator()( A a, B b, C c, D d, E e, F f, G g )
      {
         mutex->lock();
         FMOD_RESULT result = fn( a, b, c, d, e, f, g );
         mutex->unlock();
         return result;
      }
      template< typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H >
      FMOD_RESULT operator()( A a, B b, C c, D d, E e, F f, G g, H h )
      {
         mutex->lock();
         FMOD_RESULT result = fn( a, b, c, d, e, f, g, h );
         mutex->unlock();
         return result;
      }
   };

#define FMOD_FUNCTION(fn_name, fn_args) \
   Thunk< FMODFNPTR##fn_name > fn_name;
#define FMOD_EVENT_FUNCTION(fn_name, fn_args) \
   Thunk< FMODFNPTR##fn_name > fn_name;
#include FMOD_FN_FILE
#undef FMOD_FUNCTION
#undef FMOD_EVENT_FUNCTION
};


inline void TorqueVectorToFMODVector( const Point3F& torque, FMOD_VECTOR& fmod )
{
   fmod.x = torque.x;
   fmod.y = torque.z;
   fmod.z = torque.y;
}

inline void TorqueTransformToFMODVectors( const MatrixF& transform, FMOD_VECTOR& position, FMOD_VECTOR& forward, FMOD_VECTOR& up )
{
   Point3F _pos, _fwd, _up;
   
   transform.getColumn( 3, &_pos );
	transform.getColumn( 1, &_fwd );
	transform.getColumn( 2, &_up );

   TorqueVectorToFMODVector( _pos, position );
   TorqueVectorToFMODVector( _fwd, forward );
   TorqueVectorToFMODVector( _up, up );
}

inline int TorquePriorityToFMODPriority( F32 priority )
{
   // Map [-2,2] to [256,0].
   
   F32 n = mClampF( priority, -2.0f, 2.0f ) + 2.0f;
   return ( n * 256.0f / 4.0f );
}

inline String FMODEventPathToTorqueName( const String& path )
{
   String p = path;
   p.replace( '/', '_' );
   p.replace( '-', '_' );
   p.replace( ' ', '_' );
   p.replace( '(', '_' );
   p.replace( ')', '_' );
   p.replace( '%', '_' );
   p.replace( '$', '_' );
   return p;
}

inline String FMODEventPathToTorqueName( const String& projectName, const String& path )
{
   return String::ToString( "%s_%s", projectName.c_str(), FMODEventPathToTorqueName( path ).c_str() );
}

extern String FMODResultToString( FMOD_RESULT result );


class SFXProvider;
class SFXFMODPlugin;



class SFXFMODDevice : public SFXDevice
{
   public:

      typedef SFXDevice Parent;
      friend class SFXFMODProvider; // _init
      friend class SFXFMODEventSource; // smStatNumEventSources

      explicit SFXFMODDevice();

      SFXFMODDevice( SFXProvider* provider, FModFNTable *fmodFnTbl, int deviceIdx, String name );

      virtual ~SFXFMODDevice();

   protected:

      FMOD_MODE m3drolloffmode;
      int mDeviceIndex;
      
      /// The FMOD SFXSystemPlugin instance.
      SFXFMODPlugin mPlugin;
      
      /// @name Console Variables
      /// @{
      
      /// Current core FMOD memory usage in bytes.
      static U32 smStatMemUsageCore;
      
      /// Current FMOD Event DLL memory usage in bytes.
      static U32 smStatMemUsageEvents;
      
      /// Current number of SFXFMODEventSource instances.
      static U32 smStatNumEventSources;
      
      ///
      static bool smPrefDisableSoftware;
      
      ///
      static bool smPrefUseSoftwareOcclusion;
      
      ///
      static bool smPrefUseSoftwareHRTF;
      
      ///
      static bool smPrefEnableProfile;
      
      ///
      static bool smPrefGeometryUseClosest;
      
      ///
      static const char* smPrefDSoundHRTF;
      
      ///
      static const char* smPrefPluginPath;
      
      /// @}
      
      bool _init();
      
      static SFXFMODDevice* smInstance;
      
   public:
   
      static SFXFMODDevice* instance() { return smInstance; }
   
      FMOD_MODE get3dRollOffMode() { return m3drolloffmode; }

      static FMOD_SYSTEM* smSystem;
      static FMOD_EVENTSYSTEM* smEventSystem;
      static FModFNTable* smFunc;
      
      // Update memory usage stats for metrics display.
      void updateMemUsageStats();

      // SFXDevice.
      virtual SFXBuffer* createBuffer( const ThreadSafeRef< SFXStream >& stream, SFXDescription* description );
      virtual SFXBuffer* createBuffer( const String& filename, SFXDescription* description );
      virtual SFXVoice* createVoice( bool is3D, SFXBuffer* buffer );
      virtual void update();
      virtual void setNumListeners( U32 num );
      virtual void setListener( U32 index, const SFXListenerProperties& listener );
      virtual void setDistanceModel( SFXDistanceModel model );
      virtual void setDopplerFactor( F32 factor );
      virtual void setRolloffFactor( F32 factor );
      virtual void setReverb( const SFXReverbProperties& reverb );
      virtual void resetReverb();
};

#endif // _SFXFMODDEVICE_H_
