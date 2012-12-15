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

#include "sfx/sfxProvider.h"
#include "sfx/fmod/sfxFMODDevice.h"
#include "core/util/safeRelease.h"
#include "console/console.h"
#include "core/util/safeDelete.h"
#include "core/module.h"
#include "console/consoleTypes.h"


class SFXFMODProvider : public SFXProvider
{
public:

   SFXFMODProvider()
      : SFXProvider( "FMOD" )
   {
      Con::addVariable( "$SFX::Device::fmodNumEventSources", TypeS32, &SFXFMODDevice::smStatNumEventSources,
         "The current number of SFXFMODEventSource instances in the system.\n"
         "This tells the number of sounds in the system that are currently playing FMOD Designer events.\n\n"
         "@note Only relevant if an %FMOD sound device is used.\n\n"
         "@ingroup SFXFMOD" );
      Con::addVariable( "$SFX::Device::fmodCoreMem", TypeS32, &SFXFMODDevice::smStatMemUsageCore,
         "Current number of bytes allocated by the core %FMOD sound system.\n\n"
         "@note Only relevant if an %FMOD sound device is used.\n\n"
         "@ingroup SFXFMOD" );
      Con::addVariable( "$SFX::Device::fmodEventMem", TypeS32, &SFXFMODDevice::smStatMemUsageEvents,
         "Current number of bytes allocated by the %FMOD Designer event system.\n\n"
         "@note Only relevant if an %FMOD sound device is used and the FMOD event DLL is loaded.\n\n"
         "@ingroup SFXFMOD" );
         
      Con::addVariable( "$pref::SFX::FMOD::disableSoftware", TypeBool, &SFXFMODDevice::smPrefDisableSoftware,
         "Whether to disable the %FMOD software mixer to conserve memory.\n"
         "All sounds not created with SFXDescription::useHardware or using DSP effects will fail to load.\n\n"
         "@note Only applies when using an %FMOD sound device.\n\n"
         "@ingroup SFXFMOD" );
      Con::addVariable( "$pref::SFX::FMOD::useSoftwareHRTF", TypeBool, &SFXFMODDevice::smPrefUseSoftwareHRTF,
         "Whether to enable HRTF in %FMOD's software mixer.\n"
         "This will add a lowpass filter effect to the DSP effect chain of all sounds mixed in software.\n\n"
         "@note Only applies when using an %FMOD sound device.\n\n"
         "@ingroup SFXFMOD" );
      Con::addVariable( "$pref::SFX::FMOD::useSoftwareReverbLowmem", TypeBool, &SFXFMODDevice::smPrefUseSoftwareReverbLowmem,
         "If true, %FMOD's SFX reverb is run using 22/24kHz delay buffers, halving the memory required.\n\n"
         "@note Only applies when using an %FMOD sound device.\n\n"
         "@ingroup SFXFMOD" );
      Con::addVariable( "$pref::SFX::FMOD::enableProfile", TypeBool, &SFXFMODDevice::smPrefEnableProfile,
         "Whether to enable support for %FMOD's profiler.\n\n"
         "@note Only applies when using an %FMOD sound device.\n\n"
         "@ref FMOD_profiler\n\n"
         "@ingroup SFXFMOD" );
      Con::addVariable( "$pref::SFX::FMOD::DSoundHRTF", TypeString, &SFXFMODDevice::smPrefDSoundHRTF,
         "The type of HRTF to use for hardware-mixed 3D sounds when %FMOD is using DirectSound for sound output "
         "and hardware-acceleration is not available.\n\n"
         "Options are\n"
         "- \"none\": simple stereo panning/doppler/attenuation\n"
         "- \"light\": slightly higher quality than \"none\"\n"
         "- \"full\": full quality 3D playback\n\n"
         "@note Only applies when using an %FMOD sound device.\n\n"
         "@ingroup SFXFMOD" );
      Con::addVariable( "$pref::SFX::FMOD::pluginPath", TypeString, &SFXFMODDevice::smPrefPluginPath,
         "%Path to additional %FMOD plugins.\n\n"
         "@note Only applies when using an %FMOD sound device.\n\n"
         "@ingroup SFXFMOD" );
   }
   virtual ~SFXFMODProvider();

protected:
   FModFNTable mFMod;

   struct FModDeviceInfo : SFXDeviceInfo
   {
      FMOD_CAPS mCaps;
      FMOD_SPEAKERMODE mSpeakerMode;
   };

   void init();
   
   bool _createSystem();

public:

   SFXDevice* createDevice( const String& deviceName, bool useHardware, S32 maxBuffers );

};

MODULE_BEGIN( FMOD )

   MODULE_INIT_BEFORE( SFX )
   MODULE_SHUTDOWN_AFTER( SFX )
   
   SFXFMODProvider* mProvider;
      
   MODULE_INIT
   {
      mProvider = new SFXFMODProvider;
   }
   
   MODULE_SHUTDOWN
   {
      delete mProvider;
   }
   
MODULE_END;


//------------------------------------------------------------------------------
// Helper

bool fmodBindFunction( DLibrary *dll, void *&fnAddress, const char* name )
{
   if( !dll )
      return false;
      
   fnAddress = dll->bind( name );

   if (!fnAddress)
      Con::warnf( "FMOD Loader: DLL bind failed for %s", name );

   return fnAddress != 0;
}

//------------------------------------------------------------------------------

void SFXFMODProvider::init()
{
#ifdef TORQUE_FMOD_STATIC

   // FMOD statically linked.
   
   mFMod.isLoaded = true;
   #define FMOD_FUNCTION(fn_name, fn_args) \
      (*(void**)&mFMod.fn_name.fn) = &fn_name;
   
   #ifndef TORQUE_FMOD_NO_EVENTS
   mFMod.eventIsLoaded = true;
      #define FMOD_EVENT_FUNCTION(fn_name, fn_args) \
         (*(void**)&mFMod.fn_name.fn) = &fn_name;
   #else
      #define FMOD_EVENT_FUNCTION( fn_name, fn_args )
   #endif

   #include FMOD_FN_FILE
   
   #undef FMOD_FUNCTION
   #undef FMOD_EVENT_FUNCTION
   
#else

   // FMOD dynamically linked.
   
   const char* dllName;
   const char* pDllName; // plugin-based DLL
   const char* eventDllName;
   
#ifdef TORQUE_OS_WIN32
   dllName = "fmodex.dll";
   pDllName = "fmodexp.dll";
   eventDllName = "fmod_event.dll";
#elif defined( TORQUE_OS_MAC )
   dllName = "libfmodex.dylib";
   pDllName = "libfmodexp.dylib";
   eventDllName = "libfmodevent.dylib";
#else
#  warning Need to set FMOD DLL filename for platform.
   return;
#endif

   // Grab the functions we'll want from the fmod DLL.
   mFMod.dllRef = OsLoadLibrary( dllName );

   // Try the plugin-based version.
   if( !mFMod.dllRef )
      mFMod.dllRef = OsLoadLibrary( pDllName );

   if(!mFMod.dllRef)
   {
      Con::warnf( "SFXFMODProvider - Could not locate '%s' or '%s' - FMOD  not available.", dllName, pDllName );
      return;
   }
   
   mFMod.eventDllRef = OsLoadLibrary( eventDllName );
   if(!mFMod.eventDllRef)
      Con::warnf( "SFXFMODProvider - Could not locate %s - FMOD Designer integration not available.", eventDllName );

   mFMod.isLoaded = true;
   mFMod.eventIsLoaded = true;

   #define FMOD_FUNCTION(fn_name, fn_args) \
      mFMod.isLoaded &= fmodBindFunction(mFMod.dllRef, *(void**)&mFMod.fn_name.fn, #fn_name);
   #define FMOD_EVENT_FUNCTION(fn_name, fn_args) \
      mFMod.eventIsLoaded &= fmodBindFunction(mFMod.eventDllRef, *(void**)&mFMod.fn_name.fn, #fn_name);
            
   #include FMOD_FN_FILE
   
   #undef FMOD_FUNCTION
   #undef FMOD_EVENT_FUNCTION

   if(mFMod.isLoaded == false)
   {
      Con::warnf("SFXFMODProvider - Could not load %s - FMOD not available.", dllName);
      return;
   }
   if( !mFMod.eventIsLoaded && mFMod.eventDllRef )
      Con::warnf("SFXFMODProvider - Could not load %s - FMOD Designer integration not available.", eventDllName);

#endif

   FMOD_RESULT res;

   // Create the FMOD system object.
      
   if( !_createSystem() )
      return;
      
   // Check that the Ex API version is OK.
   
   unsigned int version;
   res = mFMod.FMOD_System_GetVersion(SFXFMODDevice::smSystem, &version);
   FModAssert(res, "SFXFMODProvider - Failed to get FMOD version!");

   Con::printf( "SFXFMODProvider - FMOD Ex API version: %x.%x.%x",
      ( version & 0xffff0000 ) >> 16,
      ( version & 0x0000ff00 ) >> 8,
      ( version & 0x000000ff )
   );

   if(version < FMOD_VERSION)
   {
      Con::warnf("SFXFMODProvider - FMOD Ex API version in DLL is too old - FMOD  not available.");
      return;
   }
   
   // Check that the Designer API version is ok.
   
   if( mFMod.eventIsLoaded )
   {
      res = mFMod.FMOD_EventSystem_GetVersion( SFXFMODDevice::smEventSystem, &version );
      FModAssert(res, "SFXFMODProvider - Failed to get FMOD version!");
      
      Con::printf( "SFXFMODProvider - FMOD Designer API version: %x.%x.%x",
         ( version & 0xffff0000 ) >> 16,
         ( version & 0x0000ff00 ) >> 8,
         ( version & 0x000000ff )
      );

      if( version < FMOD_EVENT_VERSION )
      {
         Con::errorf( "SFXFMODProvider - FMOD Designer API version in DLL is too old!" );
         return;
      }
   }

   // Now, enumerate our devices.
   int numDrivers;
   res = mFMod.FMOD_System_GetNumDrivers(SFXFMODDevice::smSystem, &numDrivers);
   FModAssert(res, "SFXFMODProvider - Failed to get driver count - FMOD  not available.");

   char nameBuff[256];

   for(S32 i=0; i<numDrivers; i++)
   {
      res = mFMod.FMOD_System_GetDriverInfo(SFXFMODDevice::smSystem, i, nameBuff, 256, ( FMOD_GUID* ) NULL);
      if( res != FMOD_OK )
      {
         Con::errorf( "SFXFMODProvider - Failed to get driver name (%s)", FMODResultToString( res ).c_str() );
         continue;
      }
      nameBuff[ 255 ] = '\0';
      
      FMOD_CAPS caps;
      FMOD_SPEAKERMODE speakerMode;
      res = mFMod.FMOD_System_GetDriverCaps( SFXFMODDevice::smSystem, i, &caps, ( int* ) 0, &speakerMode );
      if( res != FMOD_OK )
      {
         Con::errorf( "SFXFMODProvider - Failed to get driver caps (%s)", FMODResultToString( res ).c_str() );
         continue;
      }

      // Great - got something - so add it to the list of options.
      FModDeviceInfo *fmodInfo = new FModDeviceInfo();
      fmodInfo->name = String( nameBuff );
      fmodInfo->hasHardware = caps & FMOD_CAPS_HARDWARE;
      fmodInfo->maxBuffers = 32;
      fmodInfo->driver = String();
      fmodInfo->mCaps = caps;
      fmodInfo->mSpeakerMode = speakerMode;

      mDeviceInfo.push_back(fmodInfo);
   }

   // Did we get any devices?
   if ( mDeviceInfo.empty() )
   {
      Con::warnf( "SFXFMODProvider - No valid devices found - FMOD  not available." );
      return;
   }

   // TODO: FMOD_Memory_Initialize
#ifdef TORQUE_OS_XENON
   const dsize_t memSz = 5 * 1024 * 1024;
   void *memBuffer = XPhysicalAlloc( memSz, MAXULONG_PTR, 0, PAGE_READWRITE );
   mFMod.FMOD_Memory_Initialize( memBuffer, memSz, FMOD_MEMORY_ALLOCCALLBACK(NULL), FMOD_MEMORY_REALLOCCALLBACK(NULL), FMOD_MEMORY_FREECALLBACK(NULL) );
#endif

   // Wow, we made it - register the provider.
   regProvider( this );
}

SFXFMODProvider::~SFXFMODProvider()
{
   if( SFXFMODDevice::smEventSystem )
   {
      mFMod.FMOD_EventSystem_Release( SFXFMODDevice::smEventSystem );
      SFXFMODDevice::smEventSystem = NULL;
      SFXFMODDevice::smSystem = NULL;
   }
   else if( SFXFMODDevice::smSystem )
   {
      mFMod.FMOD_System_Release( SFXFMODDevice::smSystem );
      SFXFMODDevice::smSystem = NULL;
   }   
}

SFXDevice* SFXFMODProvider::createDevice( const String& deviceName, bool useHardware, S32 maxBuffers )
{
   FModDeviceInfo* info = dynamic_cast< FModDeviceInfo* >
      ( _findDeviceInfo( deviceName ) );

   if( !info )
      return NULL;
      
   if( !SFXFMODDevice::smSystem && !_createSystem() )
      return false;

   SFXFMODDevice* device = new SFXFMODDevice(this, &mFMod, 0, info->name );
   if( !device->_init() )
      SAFE_DELETE( device );

   return device;
}

bool SFXFMODProvider::_createSystem()
{
   AssertFatal( !SFXFMODDevice::smEventSystem, "SFXFMODProvider::_createSystem() - event system already created!" );
   AssertFatal( !SFXFMODDevice::smSystem, "SFXFMODProvider::_createSystem() - system already created!" );
   
   if( mFMod.eventIsLoaded )
   {
      FMOD_RESULT res = mFMod.FMOD_EventSystem_Create( &SFXFMODDevice::smEventSystem );
      if( res != FMOD_OK )
      {
         Con::errorf( "SFXFMODProvider - could not create the FMOD event system." );
         return false;
      }
      
      res = mFMod.FMOD_EventSystem_GetSystemObject( SFXFMODDevice::smEventSystem, &SFXFMODDevice::smSystem );
      if( res != FMOD_OK )
      {
         Con::errorf( "SFXFMODProvider - could not retrieve the FMOD system object." );
         return false;
      }
   }
   else
   {
      // Allocate the FMod system.
      
      FMOD_RESULT res = mFMod.FMOD_System_Create( &SFXFMODDevice::smSystem );
      if( res != FMOD_OK )
      {
         // Failed - deal with it!
         Con::errorf("SFXFMODProvider - could not create the FMOD system.");
         return false;
      }
   }
      
   return true;
}
