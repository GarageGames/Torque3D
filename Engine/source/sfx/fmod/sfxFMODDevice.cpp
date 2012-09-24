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
#include "platform/threads/mutex.h"
#include "sfx/fmod/sfxFMODDevice.h"
#include "sfx/fmod/sfxFMODBuffer.h"
#include "sfx/sfxSystem.h"
#include "platform/async/asyncUpdate.h"
#include "console/consoleTypes.h"
#include "core/volume.h"


bool                 SFXFMODDevice::smPrefDisableSoftware = false;
bool                 SFXFMODDevice::smPrefUseSoftwareOcclusion = true;
bool                 SFXFMODDevice::smPrefUseSoftwareHRTF = true;
bool                 SFXFMODDevice::smPrefUseSoftwareReverbLowmem = false;
bool                 SFXFMODDevice::smPrefEnableProfile = false;
bool                 SFXFMODDevice::smPrefGeometryUseClosest = false;
const char*          SFXFMODDevice::smPrefDSoundHRTF = "full";
const char*          SFXFMODDevice::smPrefPluginPath = "";
U32                  SFXFMODDevice::smStatMemUsageCore;
U32                  SFXFMODDevice::smStatMemUsageEvents;
U32                  SFXFMODDevice::smStatNumEventSources;
SFXFMODDevice*       SFXFMODDevice::smInstance;
FMOD_SYSTEM*         SFXFMODDevice::smSystem;
FMOD_EVENTSYSTEM*    SFXFMODDevice::smEventSystem;
FModFNTable*         SFXFMODDevice::smFunc;
Mutex*               FModFNTable::mutex;


//-----------------------------------------------------------------------------

String FMODResultToString( FMOD_RESULT result )
{
   switch( result )
   {
      #define FMOD_ERROR( n ) case n: return #n;
      #include "fmodErrors.h"
      #undef FMOD_ERROR
      
      default:
         break;
   }
   
   return String();
}

//------------------------------------------------------------------------------
// FMOD filesystem wrappers.
//FIXME: these are not thread-safe and cannot be used as such

FMOD_RESULT F_CALLBACK fmodFileOpenCallback( const char* name, int unicode, unsigned int* filesize, void** handle, void** userdata )
{
   String fileName;
   if( unicode )
      fileName = String( ( UTF16* ) name );
   else
      fileName = String( name );
      
   Torque::FS::FileRef file = Torque::FS::OpenFile( fileName, Torque::FS::File::Read );
   if( !file )
      return FMOD_ERR_FILE_NOTFOUND;
   else if( file->getStatus() != Torque::FS::File::Open )
      return FMOD_ERR_FILE_BAD;
      
   // Add a reference so we can pass it into FMOD.
   file->incRefCount();
      
   *filesize = U32( file->getSize() );
   *handle = file.getPointer();

   return FMOD_OK;
}

FMOD_RESULT F_CALLBACK fmodFileCloseCallback( void* handle, void* userdata )
{
   Torque::FS::File* file = reinterpret_cast< Torque::FS::File* >( handle );
   file->decRefCount();
   return FMOD_OK;
}

FMOD_RESULT F_CALLBACK fmodFileReadCallback( void* handle, void* buffer, unsigned int sizebytes, unsigned int* bytesread, void* userdata )
{
   Torque::FS::File* file = reinterpret_cast< Torque::FS::File* >( handle );

   U32 numRead = file->read( buffer, sizebytes );
   *bytesread = numRead;
   
   if( file->getStatus() == Torque::FS::File::EndOfFile )
      return FMOD_ERR_FILE_EOF;
   else if( file->getStatus() != Torque::FS::File::Open )
      return FMOD_ERR_FILE_BAD;
      
   return FMOD_OK;
}

FMOD_RESULT F_CALLBACK fmodFileSeekCallback( void* handle, unsigned int pos, void* userdata )
{
   Torque::FS::File* file = reinterpret_cast< Torque::FS::File* >( handle );
   
   if( file->setPosition( pos, Torque::FS::File::Begin ) != pos )
      return FMOD_ERR_FILE_COULDNOTSEEK;
      
   return FMOD_OK;
}

//-----------------------------------------------------------------------------

SFXFMODDevice::SFXFMODDevice( SFXProvider* provider, 
                              FModFNTable *fmodFnTbl, 
                              int deviceIdx, 
                              String name )
   :  SFXDevice( name, provider, false, 32 ),
      m3drolloffmode( FMOD_3D_INVERSEROLLOFF ),
      mDeviceIndex( deviceIdx )
{
	// Store off the function pointers for later use.
	smFunc = fmodFnTbl;

   smStatMemUsageCore = 0;
   smStatMemUsageEvents = 0;
   smStatNumEventSources = 0;
      
   // Register our SFXSystem plugin.
   
   SFX->addPlugin( &mPlugin );
   
   smInstance = this;
}

//-----------------------------------------------------------------------------

SFXFMODDevice::~SFXFMODDevice()
{
   _releaseAllResources();
   
   SFX->removePlugin( &mPlugin );
   
   if( smEventSystem )
   {
      smFunc->FMOD_EventSystem_Release( smEventSystem );
      smEventSystem = NULL;
      smSystem = NULL;
   }
   else
      smFunc->FMOD_System_Close( smSystem );
      
   smInstance = NULL;
}

//-----------------------------------------------------------------------------

bool SFXFMODDevice::_init()
{
   #define FMOD_CHECK( message )                               \
      if( result != FMOD_OK )                                  \
      {                                                        \
         Con::errorf( "SFXFMODDevice::_init() - %s (%s)",      \
            message,                                           \
            FMOD_ErrorString( result ) );                      \
         return false;                                         \
      }

	AssertISV(smSystem, 
      "SFXFMODDevice::_init() - can't init w/o an existing FMOD system handle!");

   FMOD_RESULT result;
   
   // Get some prefs.
   
   if( smPrefPluginPath && smPrefPluginPath[ 0 ] )
   {
      char fullPath[ 4096 ];
      Platform::makeFullPathName( smPrefPluginPath, fullPath, sizeof( fullPath ) );
      
      smFunc->FMOD_System_SetPluginPath( smSystem, fullPath );
   }
   else
   {
      smFunc->FMOD_System_SetPluginPath( smSystem, Platform::getExecutablePath() );
   }
   
	// Initialize everything from fmod.
	FMOD_SPEAKERMODE speakermode;
	FMOD_CAPS        caps;
	result = smFunc->FMOD_System_GetDriverCaps(smSystem, 0, &caps, ( int* ) 0, &speakermode);
   FMOD_CHECK( "SFXFMODDevice::init - Failed to get driver caps" );

	result = smFunc->FMOD_System_SetDriver(smSystem, mDeviceIndex);
   FMOD_CHECK( "SFXFMODDevice::init - Failed to set driver" );

	result = smFunc->FMOD_System_SetSpeakerMode(smSystem, speakermode);
   FMOD_CHECK( "SFXFMODDevice::init - Failed to set the user selected speaker mode" );

	if (caps & FMOD_CAPS_HARDWARE_EMULATED)             /* The user has the 'Acceleration' slider set to off!  This is really bad for latency!. */
	{                                                   /* You might want to warn the user about this. */
		result = smFunc->FMOD_System_SetDSPBufferSize(smSystem, 1024, 10);
      FMOD_CHECK( "SFXFMODDevice::init - Failed to set DSP buffer size" );
	}
   
   Con::printf( "\nFMOD Device caps:" );
   #define PRINT_CAP( name )              \
      if( caps & FMOD_CAPS_ ## name )     \
         Con::printf( #name );
         
   PRINT_CAP( HARDWARE );
   PRINT_CAP( HARDWARE_EMULATED );
   PRINT_CAP( OUTPUT_MULTICHANNEL );
   PRINT_CAP( OUTPUT_FORMAT_PCM8 );
   PRINT_CAP( OUTPUT_FORMAT_PCM16 );
   PRINT_CAP( OUTPUT_FORMAT_PCM24 );
   PRINT_CAP( OUTPUT_FORMAT_PCM32 );
   PRINT_CAP( OUTPUT_FORMAT_PCMFLOAT );
   PRINT_CAP( REVERB_LIMITED );
   
   Con::printf( "" );
   
   bool tryAgain;
   do
   {
      tryAgain = false;
      
      FMOD_INITFLAGS flags = FMOD_INIT_NORMAL | FMOD_INIT_VOL0_BECOMES_VIRTUAL;
      
      if( smPrefDisableSoftware )
         flags |= FMOD_INIT_SOFTWARE_DISABLE;
      if( smPrefUseSoftwareOcclusion )
         flags |= FMOD_INIT_OCCLUSION_LOWPASS;
      if( smPrefUseSoftwareHRTF )
         flags |= FMOD_INIT_HRTF_LOWPASS;
      if( smPrefUseSoftwareReverbLowmem )
         flags |= FMOD_INIT_SOFTWARE_REVERB_LOWMEM;
      if( smPrefEnableProfile )
         flags |= FMOD_INIT_ENABLE_PROFILE;
      if( smPrefGeometryUseClosest )
         flags |= FMOD_INIT_GEOMETRY_USECLOSEST;
      
      if( smEventSystem )
         result = smFunc->FMOD_EventSystem_Init( smEventSystem, 100, flags, ( void* ) 0, FMOD_EVENT_INIT_NORMAL );
      else
         result = smFunc->FMOD_System_Init( smSystem, 100, flags, ( void* ) 0 );
         
      if( result == FMOD_ERR_OUTPUT_CREATEBUFFER )         /* Ok, the speaker mode selected isn't supported by this soundcard.  Switch it back to stereo... */
      {
         result = smFunc->FMOD_System_SetSpeakerMode( smSystem, FMOD_SPEAKERMODE_STEREO );
         FMOD_CHECK( "SFXFMODDevice::init - failed on fallback speaker mode setup" );
         tryAgain = true;
      }
   } while( tryAgain );
   FMOD_CHECK( "SFXFMODDevice::init - failed to init system" );
   
   // Print hardware channel info.

   if( caps & FMOD_CAPS_HARDWARE )
   {
      int num3D, num2D, numTotal;
      
      if( smFunc->FMOD_System_GetHardwareChannels( smSystem, &num2D, &num3D, &numTotal ) == FMOD_OK )
         Con::printf( "FMOD Hardware channels: 2d=%i, 3d=%i, total=%i", num2D, num3D, numTotal );
   }

   // Set up filesystem.
   
   //FIXME: Don't do this for now.  Crashes on Windows.
   #if 0
   smFunc->FMOD_System_SetFileSystem( smSystem, fmodFileOpenCallback, fmodFileCloseCallback, fmodFileReadCallback, fmodFileSeekCallback, -1 );
   #endif
      
   // Set capabilities.
   
   mCaps = CAPS_Reverb | CAPS_VoiceManagement;
   if( smEventSystem )
      mCaps |= CAPS_FMODDesigner;

   // Start the update thread.
   
   #ifndef TORQUE_DEDICATED // Avoid dependency on platform/async for Linx dedicated.
   
   if( !Con::getBoolVariable( "$_forceAllMainThread" ) )
   {
      SFXInternal::gUpdateThread = new AsyncPeriodicUpdateThread
         ( "FMOD Update Thread", SFXInternal::gBufferUpdateList,
           Con::getIntVariable( "$pref::SFX::updateInterval", SFXInternal::DEFAULT_UPDATE_INTERVAL ) );
      SFXInternal::gUpdateThread->start();
   }
   
   #endif
   
   return true;
}

//-----------------------------------------------------------------------------

SFXBuffer* SFXFMODDevice::createBuffer( const ThreadSafeRef< SFXStream >& stream, SFXDescription* description )
{
   AssertFatal( stream, "SFXFMODDevice::createBuffer() - Got a null stream!" );
   AssertFatal( description, "SFXFMODDevice::createBuffer() - Got null description!" );

   SFXFMODBuffer *buffer = SFXFMODBuffer::create( stream, description );
   if ( buffer )
      _addBuffer( buffer );

   return buffer;
}

//-----------------------------------------------------------------------------

SFXBuffer* SFXFMODDevice::createBuffer( const String& filename, SFXDescription* description )
{
   AssertFatal( filename.isNotEmpty(), "SFXFMODDevice::createBuffer() - Got an empty filename!" );
   AssertFatal( description, "SFXFMODDevice::createBuffer() - Got null description!" );

   SFXFMODBuffer* buffer = SFXFMODBuffer::create( filename, description );
   if( buffer )
      _addBuffer( buffer );
      
   return buffer;
}

//-----------------------------------------------------------------------------

SFXVoice* SFXFMODDevice::createVoice( bool is3D, SFXBuffer* buffer )
{
   AssertFatal( buffer, "SFXFMODDevice::createVoice() - Got null buffer!" );

   SFXFMODBuffer* fmodBuffer = dynamic_cast<SFXFMODBuffer*>( buffer );
   AssertFatal( fmodBuffer, "SFXFMODDevice::createVoice() - Got bad buffer!" );

   SFXFMODVoice* voice = SFXFMODVoice::create( this, fmodBuffer );
   if ( !voice )
      return NULL;

   _addVoice( voice );
	return voice;
}

//-----------------------------------------------------------------------------

void SFXFMODDevice::update()
{
   Parent::update();

   if( smEventSystem )
   {
      FModAssert( smFunc->FMOD_EventSystem_Update( smEventSystem ), "Failed to update event system!" );
   }
   else
   {
      FModAssert(smFunc->FMOD_System_Update(smSystem), "Failed to update system!");
   }
}

//-----------------------------------------------------------------------------

void SFXFMODDevice::setNumListeners( U32 num )
{
   smFunc->FMOD_System_Set3DNumListeners( smSystem, num );
}

//-----------------------------------------------------------------------------

void SFXFMODDevice::setListener( U32 index, const SFXListenerProperties& listener )
{
   FMOD_VECTOR position, forward, up, velocity;
   
   TorqueTransformToFMODVectors( listener.getTransform(), position, forward, up );
   TorqueVectorToFMODVector( listener.getVelocity(), velocity );

	// Do the listener state update, then update!
	smFunc->FMOD_System_Set3DListenerAttributes( smSystem, index, &position, &velocity, &forward, &up );
}

//-----------------------------------------------------------------------------

void SFXFMODDevice::setDistanceModel( SFXDistanceModel model )
{
   switch( model )
   {
      case SFXDistanceModelLinear:
         m3drolloffmode = FMOD_3D_LINEARROLLOFF;
         break;
         
      case SFXDistanceModelLogarithmic:
         m3drolloffmode = FMOD_3D_INVERSEROLLOFF;
         break;
         
      default:
         AssertWarn( false, "SFXFMODDevice::setDistanceModel - model not implemented" );
   }
}

//-----------------------------------------------------------------------------

void SFXFMODDevice::setDopplerFactor( F32 factor )
{
   F32 dopplerFactor;
   F32 distanceFactor;
   F32 rolloffFactor;
   
   smFunc->FMOD_System_Get3DSettings( smSystem, &dopplerFactor, &distanceFactor, &rolloffFactor );
   dopplerFactor = factor;
   smFunc->FMOD_System_Set3DSettings( smSystem, dopplerFactor, distanceFactor, rolloffFactor );
}

//-----------------------------------------------------------------------------

void SFXFMODDevice::setRolloffFactor( F32 factor )
{
   F32 dopplerFactor;
   F32 distanceFactor;
   F32 rolloffFactor;
   
   smFunc->FMOD_System_Get3DSettings( smSystem, &dopplerFactor, &distanceFactor, &rolloffFactor );
   rolloffFactor = factor;
   smFunc->FMOD_System_Set3DSettings( smSystem, dopplerFactor, distanceFactor, rolloffFactor );
}

//-----------------------------------------------------------------------------

void SFXFMODDevice::setReverb( const SFXReverbProperties& reverb )
{
   FMOD_REVERB_PROPERTIES prop = FMOD_PRESET_GENERIC;

   prop.Environment           = 0;
   prop.EnvDiffusion          = reverb.mEnvDiffusion;
   prop.Room                  = reverb.mRoom;
   prop.RoomHF                = reverb.mRoomHF;
   prop.RoomLF                = reverb.mRoomLF;
   prop.DecayTime             = reverb.mDecayTime;
   prop.DecayLFRatio          = reverb.mDecayLFRatio;
   prop.DecayHFRatio          = reverb.mDecayHFRatio;
   prop.Reflections           = reverb.mReflections;
   prop.ReflectionsDelay      = reverb.mReflectionsDelay;
   prop.Reverb                = reverb.mReverb;
   prop.ReverbDelay           = reverb.mReverbDelay;
   prop.ModulationTime        = reverb.mModulationTime;
   prop.ModulationDepth       = reverb.mModulationDepth;
   prop.HFReference           = reverb.mHFReference;
   prop.LFReference           = reverb.mLFReference;
   prop.Diffusion             = reverb.mDiffusion;
   prop.Density               = reverb.mDensity;
   prop.Flags                 = reverb.mFlags;
   
   // Here we only want to affect 3D sounds.  While not quite obvious from the docs,
   // SetReverbProperties sets the global reverb environment for 2D sounds whereas
   // SetAmbientReverbProperties sets the global reverb environment for 3D sounds.
   
   FMOD_RESULT result = smFunc->FMOD_System_SetReverbAmbientProperties( smSystem, &prop );
   if( result != FMOD_OK )
      Con::errorf( "SFXFMODDevice::setReverb - Failed to set reverb (%s)", FMODResultToString( result ).c_str() );
}

//-----------------------------------------------------------------------------

void SFXFMODDevice::resetReverb()
{
   FMOD_REVERB_PROPERTIES prop = FMOD_PRESET_OFF;
   smFunc->FMOD_System_SetReverbProperties( smSystem, &prop );
}

//-----------------------------------------------------------------------------

void SFXFMODDevice::updateMemUsageStats()
{
   smFunc->FMOD_System_GetMemoryInfo( smSystem, ( unsigned int ) FMOD_MEMBITS_ALL,
      ( unsigned int ) 0, ( unsigned int* ) &smStatMemUsageCore, ( unsigned int* ) 0 );
      
   if( smEventSystem )
      smFunc->FMOD_EventSystem_GetMemoryInfo( smEventSystem, ( unsigned int ) 0,
         ( unsigned int ) FMOD_EVENT_MEMBITS_ALL, ( unsigned int* ) &smStatMemUsageEvents, ( unsigned int* ) 0 );
}

//=============================================================================
//    Console Functions.
//=============================================================================
// MARK: ---- Console Functions ----

//------------------------------------------------------------------------------

ConsoleFunction( fmodDumpDSPInfo, void, 1, 1, "()"
				"@brief Dump information about the standard DSP effects.\n\n"
				"@ingroup SFXFMOD")
{
   if( !SFXFMODDevice::smFunc )
      return;
      
   const U32 firstDSPType = FMOD_DSP_TYPE_MIXER;
   const U32 lastDSPType = FMOD_DSP_TYPE_TREMOLO;
   
   for( U32 i = firstDSPType; i <= lastDSPType; ++ i )
   {
      FMOD_DSP* dsp;
      if( SFXFMODDevice::smFunc->FMOD_System_CreateDSPByType( SFXFMODDevice::smSystem, ( FMOD_DSP_TYPE ) i, &dsp ) == FMOD_OK )
      {
         // Print general info.
         
         char name[ 33 ];
         unsigned int version;
         int channels;
         int numParameters;
         
         dMemset( name, 0, sizeof( name ) );
         SFXFMODDevice::smFunc->FMOD_DSP_GetInfo( dsp, name, &version, &channels, ( int* ) NULL, ( int* ) NULL );
         SFXFMODDevice::smFunc->FMOD_DSP_GetNumParameters( dsp, &numParameters );
         
         Con::printf( "----------------------------------------------------------------" );
         Con::printf( "DSP: %s", name );
         Con::printf( "Version: %i.%i", ( version & 0xffff0000 ) >> 16, version & 0xffff );
         Con::printf( "Channels: %i", channels );
         Con::printf( "Parameters: %i", numParameters );
         Con::printf( "" );
         
         // Print parameter info.
         
         for( U32 n = 0; n < numParameters; ++ n )
         {
            char name[ 17 ];
            char label[ 17 ];
            char description[ 1024 ];
            float minValue, maxValue;
            float value;
            char valueString[ 256 ];
            
            dMemset( name, 0, sizeof( name ) );
            dMemset( label, 0, sizeof( label ) );
            dMemset( description, 0, sizeof( description ) );
            dMemset( valueString, 0, sizeof( valueString ) );
            
            SFXFMODDevice::smFunc->FMOD_DSP_GetParameterInfo( dsp, n, name, label, description, sizeof( description ) - 1, &minValue, &maxValue );
            SFXFMODDevice::smFunc->FMOD_DSP_GetParameter( dsp, n, &value, valueString, sizeof( valueString ) - 1 );
            
            Con::printf( "* Parameter %i", n );
            Con::printf( "Name: %s", name );
            Con::printf( "Label: %s", label );
            Con::printf( "Description: %s", description );
            Con::printf( "Min: %f", minValue );
            Con::printf( "Max: %f", maxValue );
            Con::printf( "Value: %f (%s)", value, valueString );
            Con::printf( "" );
         }
         
         // Release the DSP.
         
         SFXFMODDevice::smFunc->FMOD_DSP_Release( dsp );
      }
   }
}

//-----------------------------------------------------------------------------

ConsoleFunction( fmodDumpMemoryStats, void, 1, 1, "()"
				"@return Prints the current memory consumption of the FMOD module\n\n"
				"@ingroup SFXFMOD")
{
   int current = 0;
   int max = 0;

   if (SFXFMODDevice::smFunc && SFXFMODDevice::smFunc->FMOD_Memory_GetStats.fn)
         SFXFMODDevice::smFunc->FMOD_Memory_GetStats(&current, &max);
   Con::printf("Fmod current: %d, max: %d", current, max);
}
