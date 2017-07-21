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

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// Arcane-FX for MIT Licensed Open Source version of Torque 3D from GarageGames
// Copyright (C) 2015 Faust Logic, Inc.
//
//    Changes:
//        datablock-temp-clone -- Implements creation of temporary datablock clones to
//            allow late substitution of datablock fields.
//        substitutions -- Types equating SFXDescription with legacy AudioDescription
//            and SFXProfile with legacy AudioProfile. (deprecated)
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#include "platform/platform.h"

#include "sfx/sfxProfile.h"
#include "sfx/sfxDescription.h"
#include "sfx/sfxSystem.h"
#include "sfx/sfxStream.h"
#include "sim/netConnection.h"
#include "core/stream/bitStream.h"
#include "core/resourceManager.h"
#include "console/engineAPI.h"

using namespace Torque;

IMPLEMENT_CO_DATABLOCK_V1( SFXProfile );


ConsoleDocClass( SFXProfile,
   "@brief Encapsulates a single sound file for playback by the sound system.\n\n"
   
   "SFXProfile combines a sound description (SFXDescription) with a sound file such that it can be played "
   "by the sound system.  To be able to play a sound file, the sound system will always require a profile "
   "for it to be created.  However, several of the SFX functions (sfxPlayOnce(), sfxCreateSource()) perform "
   "this creation internally for convenience using temporary profile objects.\n\n"
   
   "Sound files can be in either OGG or WAV format.  However, extended format support is available when using FMOD. "
   "See @ref SFX_formats.\n\n"

   "@section SFXProfile_loading Profile Loading\n\n"
   
   "By default, the sound data referenced by a profile will be loaded when the profile is first played and the "
   "data then kept until either the profile is deleted or until the sound device on which the sound data is held "
   "is deleted.\n\n"
   
   "This initial loading my incur a small delay when the sound is first played.  To avoid this, a profile may be "
   "expicitly set to load its sound data immediately when the profile is added to the system.  This is done by "
   "setting the #preload property to true.\n\n"
   
   "@note Sounds using streamed playback (SFXDescription::isStreaming) cannot be preloaded and will thus "
      "ignore the #preload flag.\n\n"
      
   "@tsexample\n"
      "datablock SFXProfile( Shore01Snd )\n"
      "{\n"
      "   fileName     = \"art/sound/Lakeshore_mono_01\";\n"
      "   description  = Shore01Looping3d;\n"
      "   preload      = true;\n"
      "};\n"
   "@endtsexample\n\n"
      
   "@ingroup SFX\n"
   "@ingroup Datablocks\n"
);


//-----------------------------------------------------------------------------

SFXProfile::SFXProfile()
   : mPreload( false )
{
}

//-----------------------------------------------------------------------------

SFXProfile::SFXProfile( SFXDescription* desc, const String& filename, bool preload )
   : Parent( desc ),
     mFilename( filename ),
     mPreload( preload )
{
}

//-----------------------------------------------------------------------------

// AFX CODE BLOCK (datablock-temp-clone) <<
/* ORIGINAL CODE
SFXProfile::~SFXProfile()
{
}
*/
// AFX CODE BLOCK (datablock-temp-clone) >>

//-----------------------------------------------------------------------------

void SFXProfile::initPersistFields()
{
   addGroup( "Sound" );
   
      addField( "filename",    TypeStringFilename,        Offset( mFilename, SFXProfile ),
         "%Path to the sound file.\n"
         "If the extension is left out, it will be inferred by the sound system.  This allows to "
         "easily switch the sound format without having to go through the profiles and change the "
         "filenames there, too.\n" );
      addField( "preload",     TypeBool,                  Offset( mPreload, SFXProfile ),
         "Whether to preload sound data when the profile is added to system.\n"
         "@note This flag is ignored by streamed sounds.\n\n"
         "@ref SFXProfile_loading" );
      
   endGroup( "Sound" );

   // AFX CODE BLOCK (substitutions) <<
   // disallow some field substitutions
   disableFieldSubstitutions("description");
   // AFX CODE BLOCK (substitutions) >>
   
   Parent::initPersistFields();
}

//-----------------------------------------------------------------------------

bool SFXProfile::onAdd()
{
   if( !Parent::onAdd() )
      return false;
         
   // If we're a streaming profile we don't preload
   // or need device events.
   if( SFX && !mDescription->mIsStreaming )
   {
      // If preload is enabled we load the resource
      // and device buffer now to avoid a delay on
      // first playback.
      if( mPreload && !_preloadBuffer() )
         Con::errorf( "SFXProfile(%s)::onAdd: The preload failed!", getName() );
   }

   _registerSignals();

   return true;
}

//-----------------------------------------------------------------------------

void SFXProfile::onRemove()
{
   _unregisterSignals();

   Parent::onRemove();
}

//-----------------------------------------------------------------------------

bool SFXProfile::preload( bool server, String &errorStr )
{
   if ( !Parent::preload( server, errorStr ) )
      return false;

   // TODO: Investigate how NetConnection::filesWereDownloaded()
   // effects the system.

   // Validate the datablock... has nothing to do with mPreload.
   if(  !server &&
        NetConnection::filesWereDownloaded() &&
        ( mFilename.isEmpty() || !SFXResource::exists( mFilename ) ) )
      return false;

   return true;
}

//-----------------------------------------------------------------------------

void SFXProfile::packData(BitStream* stream)
{
   Parent::packData( stream );

   char buffer[256];
   if ( mFilename.isEmpty() )
      buffer[0] = 0;
   else
      dStrncpy( buffer, mFilename.c_str(), 256 );
   stream->writeString( buffer );

   stream->writeFlag( mPreload );
}

//-----------------------------------------------------------------------------

void SFXProfile::unpackData(BitStream* stream)
{
   Parent::unpackData( stream );

   char buffer[256];
   stream->readString( buffer );
   mFilename = buffer;

   mPreload = stream->readFlag();
}

//-----------------------------------------------------------------------------

bool SFXProfile::isLooping() const
{
   return getDescription()->mIsLooping;
}

//-----------------------------------------------------------------------------

void SFXProfile::_registerSignals()
{
   SFX->getEventSignal().notify( this, &SFXProfile::_onDeviceEvent );
   ResourceManager::get().getChangedSignal().notify( this, &SFXProfile::_onResourceChanged );
}

//-----------------------------------------------------------------------------

void SFXProfile::_unregisterSignals()
{
   ResourceManager::get().getChangedSignal().remove( this, &SFXProfile::_onResourceChanged );
   if( SFX )
      SFX->getEventSignal().remove( this, &SFXProfile::_onDeviceEvent );
}

//-----------------------------------------------------------------------------

void SFXProfile::_onDeviceEvent( SFXSystemEventType evt )
{
   switch( evt )
   {
      case SFXSystemEvent_CreateDevice:
      {
         if( mPreload && !mDescription->mIsStreaming && !_preloadBuffer() )
            Con::errorf( "SFXProfile::_onDeviceEvent: The preload failed! %s", getName() );
         break;
      }
      
      default:
         break;
   }
}

//-----------------------------------------------------------------------------

void SFXProfile::_onResourceChanged( const Torque::Path& path )
{
   if( path != Path( mFilename ) )
      return;
   
   // Let go of the old resource and buffer.
            
   mResource = NULL;
   mBuffer = NULL;
      
   // Load the new resource.
      
   getResource();
      
   if( mPreload && !mDescription->mIsStreaming )
   {
      if( !_preloadBuffer() )
         Con::errorf( "SFXProfile::_onResourceChanged() - failed to preload '%s'", mFilename.c_str() );
   }
      
   mChangedSignal.trigger( this );
}

//-----------------------------------------------------------------------------

bool SFXProfile::_preloadBuffer()
{
   AssertFatal( !mDescription->mIsStreaming, "SFXProfile::_preloadBuffer() - must not be called for streaming profiles" );

   mBuffer = _createBuffer();
   return ( !mBuffer.isNull() );
}

//-----------------------------------------------------------------------------

Resource<SFXResource>& SFXProfile::getResource()
{
   if( !mResource && !mFilename.isEmpty() )
      mResource = SFXResource::load( mFilename );

   return mResource;
}

//-----------------------------------------------------------------------------

SFXBuffer* SFXProfile::getBuffer()
{
   if ( mDescription->mIsStreaming )
   {
      // Streaming requires unique buffers per 
      // source, so this creates a new buffer.
      if ( SFX )
         return _createBuffer();

      return NULL;
   }

   if ( mBuffer.isNull() )
      _preloadBuffer();

   return mBuffer;
}

//-----------------------------------------------------------------------------

SFXBuffer* SFXProfile::_createBuffer()
{
   SFXBuffer* buffer = 0;
   
   // Try to create through SFXDevie.
   
   if( !mFilename.isEmpty() && SFX )
   {
      buffer = SFX->_createBuffer( mFilename, mDescription );
      if( buffer )
      {
         #ifdef TORQUE_DEBUG
         const SFXFormat& format = buffer->getFormat();
         Con::printf( "%s SFX: %s (%i channels, %i kHz, %.02f sec, %i kb)",
            mDescription->mIsStreaming ? "Streaming" : "Loaded", mFilename.c_str(),
            format.getChannels(),
            format.getSamplesPerSecond() / 1000,
            F32( buffer->getDuration() ) / 1000.0f,
            format.getDataLength( buffer->getDuration() ) / 1024 );
         #endif
      }
   }
   
   // If that failed, load through SFXResource.
   
   if( !buffer )
   {
      Resource< SFXResource >& resource = getResource();
      if( resource != NULL && SFX )
      {
         #ifdef TORQUE_DEBUG
         const SFXFormat& format = resource->getFormat();
         Con::printf( "%s SFX: %s (%i channels, %i kHz, %.02f sec, %i kb)",
            mDescription->mIsStreaming ? "Streaming" : "Loading", resource->getFileName().c_str(),
            format.getChannels(),
            format.getSamplesPerSecond() / 1000,
            F32( resource->getDuration() ) / 1000.0f,
            format.getDataLength( resource->getDuration() ) / 1024 );
         #endif

         ThreadSafeRef< SFXStream > sfxStream = resource->openStream();
         buffer = SFX->_createBuffer( sfxStream, mDescription );
      }
   }

   return buffer;
}

//-----------------------------------------------------------------------------

U32 SFXProfile::getSoundDuration()
{
   Resource< SFXResource  >& resource = getResource();
   if( resource != NULL )
      return mResource->getDuration();
   else
      return 0;
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SFXProfile, getSoundDuration, F32, (),,
   "Return the length of the sound data in seconds.\n\n"
   "@return The length of the sound data in seconds or 0 if the sound referenced by the profile could not be found." )
{
   return ( F32 ) object->getSoundDuration() * 0.001f;
}

// AFX CODE BLOCK (datablock-temp-clone) <<

// enable this to help verify that temp-clones of AudioProfile are being deleted
//#define TRACK_AUDIO_PROFILE_CLONES

#ifdef TRACK_AUDIO_PROFILE_CLONES
static int audio_prof_clones = 0;
#endif

SFXProfile::SFXProfile(const SFXProfile& other, bool temp_clone) : SFXTrack(other, temp_clone)
{
#ifdef TRACK_AUDIO_PROFILE_CLONES
   audio_prof_clones++;
   if (audio_prof_clones == 1)
     Con::errorf("SFXProfile -- Clones are on the loose!");
#endif
   mResource = other.mResource;
   mFilename = other.mFilename;
   mPreload = other.mPreload;
   mBuffer = other.mBuffer; // -- AudioBuffer loaded using mFilename
   mChangedSignal = other.mChangedSignal;
}

SFXProfile::~SFXProfile()
{
  if (!isTempClone())
    return;

  // cleanup after a temp-clone

  if (mDescription && mDescription->isTempClone())
  {
    delete mDescription;
    mDescription = 0;
  }

#ifdef TRACK_AUDIO_PROFILE_CLONES
  if (audio_prof_clones > 0)
  {
    audio_prof_clones--;
    if (audio_prof_clones == 0)
      Con::errorf("SFXProfile -- Clones eliminated!");
  }
  else
    Con::errorf("SFXProfile -- Too many clones deleted!");
#endif
}

//
// Clone and perform substitutions on the SFXProfile and on any SFXDescription
// it references.
//
SFXProfile* SFXProfile::cloneAndPerformSubstitutions(const SimObject* owner, S32 index)
{
  if (!owner)
    return this;

  SFXProfile* sub_profile_db = this;

  // look for mDescriptionObject subs
  SFXDescription* desc_db;
  if (mDescription && mDescription->getSubstitutionCount() > 0)
  {
    SFXDescription* orig_db = mDescription;
    desc_db = new SFXDescription(*orig_db, true);
    orig_db->performSubstitutions(desc_db, owner, index);
  }
  else
    desc_db = 0;

  if (this->getSubstitutionCount() > 0 || desc_db)
  {
    sub_profile_db = new SFXProfile(*this, true);
    performSubstitutions(sub_profile_db, owner, index);
    if (desc_db)
      sub_profile_db->mDescription = desc_db;
  }

  return sub_profile_db;
}

void SFXProfile::onPerformSubstitutions() 
{ 
   if ( SFX )
   {
      // If preload is enabled we load the resource
      // and device buffer now to avoid a delay on
      // first playback.
      if ( mPreload && !_preloadBuffer() )
         Con::errorf( "SFXProfile(%s)::onPerformSubstitutions: The preload failed!", getName() );

      // We need to get device change notifications.
      SFX->getEventSignal().notify( this, &SFXProfile::_onDeviceEvent );
   }
}
// AFX CODE BLOCK (datablock-temp-clone) >>

// AFX CODE BLOCK (sfx-legacy) <<
// This allows legacy AudioProfile datablocks to be recognized as an alias
// for SFXProfile. It is intended to ease the transition from older scripts
// especially those that still need to support pre-1.7 applications.
// (This maybe removed in future releases so treat as deprecated.)
class AudioProfile : public SFXProfile
{
	typedef SFXProfile Parent;
public:
	DECLARE_CONOBJECT(AudioProfile);
};

IMPLEMENT_CO_DATABLOCK_V1(AudioProfile);

ConsoleDocClass( AudioProfile,
   "@brief Allows legacy AudioProfile datablocks to be treated as SFXProfile datablocks.\n\n"

   "@ingroup afxMisc\n"
   "@ingroup AFX\n"
   "@ingroup Datablocks\n"
);

// AFX CODE BLOCK (sfx-legacy) >>

