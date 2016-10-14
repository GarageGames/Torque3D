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
#include "T3D/sfx/sfxEmitter.h"
#include "sfx/sfxSystem.h"
#include "sfx/sfxTrack.h"
#include "sfx/sfxSource.h"
#include "sfx/sfxTypes.h"
#include "scene/sceneRenderState.h"
#include "core/stream/bitStream.h"
#include "sim/netConnection.h"
#include "math/mathIO.h"
#include "math/mQuat.h"
#include "renderInstance/renderPassManager.h"
#include "gfx/gfxTransformSaver.h"
#include "gfx/gfxDrawUtil.h"
#include "gfx/primBuilder.h"
#include "console/engineAPI.h"


IMPLEMENT_CO_NETOBJECT_V1( SFXEmitter );

ConsoleDocClass( SFXEmitter,
   "@brief An invisible 3D object that emits sound.\n\n"
   
   "Sound emitters are used to place sounds in the level.  They are full 3D objects with their own position and orientation and "
   "when assigned 3D sounds, the transform and velocity of the sound emitter object will be applied to the 3D sound.\n\n"
   
   "Sound emitters can be set up of in either of two ways:\n"
   
   "<ul>\n"
   "<li><p>By assigning an existing SFXTrack to the emitter's #track property.</p>\n"
   "<p>In this case the general sound setup (3D, streaming, looping, etc.) will be taken from #track.  However, the emitter's "
      "own properties will still override their corresponding properties in the #track's SFXDescription.</p></li>\n"
   "<li><p>By directly assigning a sound file to the emitter's #fileName property.</p>\n"
   "<p>In this case, the sound file will be set up for playback according to the properties defined on the emitter.</p>\n"
   "</ul>\n\n"
   
   "Using #playOnAdd emitters can be configured to start playing immediately when they are added to the system (e.g. when the level "
   "objects are loaded from the mission file).\n\n"
      
   "@note A sound emitter need not necessarily emit a 3D sound.  Instead, sound emitters may also be used to play "
      "non-positional sounds.  For placing background audio to a level, however, it is usually easier to use LevelInfo::soundAmbience.\n\n"
      
   "@section SFXEmitter_networking Sound Emitters and Networking\n\n"
   
   "It is important to be aware of the fact that sounds will only play client-side whereas SFXEmitter objects are server-side "
   "entities.  This means that a server-side object has no connection to the actual sound playing on the client.  It is thus "
   "not possible for the server-object to perform queries about playback status and other source-related properties as these "
   "may in fact differ from client to client.\n\n"
   
   "@ingroup SFX\n"
);


extern bool gEditingMission;
bool SFXEmitter::smRenderEmitters;
F32 SFXEmitter::smRenderPointSize = 0.8f;
F32 SFXEmitter::smRenderRadialIncrements = 5.f;
F32 SFXEmitter::smRenderSweepIncrements = 5.f;
F32 SFXEmitter::smRenderPointDistance = 5.f;
ColorI SFXEmitter::smRenderColorPlayingInRange( 50, 255, 50, 255 );
ColorI SFXEmitter::smRenderColorPlayingOutOfRange( 50, 128, 50, 255 );
ColorI SFXEmitter::smRenderColorStoppedInRange( 0, 0, 0, 255 );
ColorI SFXEmitter::smRenderColorStoppedOutOfRange( 128, 128, 128, 255 );
ColorI SFXEmitter::smRenderColorInnerCone( 0, 0, 255, 255 );
ColorI SFXEmitter::smRenderColorOuterCone( 255, 0, 255, 255 );
ColorI SFXEmitter::smRenderColorOutsideVolume( 255, 0, 0, 255 );
ColorI SFXEmitter::smRenderColorRangeSphere( 200, 0, 0, 90 );


//-----------------------------------------------------------------------------

SFXEmitter::SFXEmitter()
   :  SceneObject(),
      mSource( NULL ),
      mTrack( NULL ),
      mUseTrackDescriptionOnly( false ),
      mLocalProfile( &mDescription ),
      mPlayOnAdd( true )
{
   mTypeMask |= MarkerObjectType;
   mNetFlags.set( Ghostable | ScopeAlways );

   mDescription.mIs3D = true;
   mDescription.mIsLooping = true;
   mDescription.mIsStreaming = false;
   mDescription.mFadeInTime = -1.f;
   mDescription.mFadeOutTime = -1.f;
   
   mLocalProfile._registerSignals();

   mObjBox.minExtents.set( -1.f, -1.f, -1.f );
   mObjBox.maxExtents.set( 1.f, 1.f, 1.f );
}

//-----------------------------------------------------------------------------

SFXEmitter::~SFXEmitter()
{
   mLocalProfile._unregisterSignals();
   
   if( mSource )
      SFX_DELETE( mSource );
}

//-----------------------------------------------------------------------------

void SFXEmitter::consoleInit()
{
   Con::addVariable( "$SFXEmitter::renderEmitters", TypeBool, &smRenderEmitters,
      "Whether to render enhanced range feedback in the editor on all emitters regardless of selection state.\n"
	  "@ingroup SFX\n");
      
   //TODO: not implemented ATM
   //Con::addVariable( "$SFXEmitter::renderPointSize", TypeF32, &smRenderPointSize );
   
   Con::addVariable( "$SFXEmitter::renderPointDistance", TypeF32, &smRenderPointDistance,
      "The distance between individual points in the sound emitter rendering in the editor as the points move from the emitter's center away to maxDistance.\n"
	  "@ingroup SFX\n");
   Con::addVariable( "$SFXEmitter::renderRadialIncrements", TypeF32, &smRenderRadialIncrements,
      "The stepping (in degrees) for the radial sweep along the axis of the XY plane sweep for sound emitter rendering in the editor.\n"
	  "@ingroup SFX\n");
   Con::addVariable( "$SFXEmitter::renderSweepIncrements", TypeF32, &smRenderSweepIncrements,
      "The stepping (in degrees) for the radial sweep on the XY plane for sound emitter rendering in the editor.\n"
	  "@ingroup SFX\n");
   Con::addVariable( "$SFXEmitter::renderColorPlayingInRange", TypeColorI, &smRenderColorPlayingInRange,
      "The color with which to render a sound emitter's marker cube in the editor when the emitter's sound is playing and in range of the listener.\n"
	  "@ingroup SFX\n" );
   Con::addVariable( "$SFXEmitter::renderColorPlayingOutOfRange", TypeColorI, &smRenderColorPlayingOutOfRange,
      "The color with which to render a sound emitter's marker cube in the editor when the emitter's sound is playing but out of the range of the listener.\n"
	  "@ingroup SFX\n" );
   Con::addVariable( "$SFXEmitter::renderColorStoppedInRange", TypeColorI, &smRenderColorStoppedInRange,
      "The color with which to render a sound emitter's marker cube in the editor when the emitter's sound is not playing but the emitter is in range of the listener.\n"
	  "@ingroup SFX\n" );
   Con::addVariable( "$SFXEmitter::renderColorStoppedOutOfRange", TypeColorI, &smRenderColorStoppedOutOfRange,
      "The color with which to render a sound emitter's marker cube in the editor when the emitter's sound is not playing and the emitter is out of range of the listener.\n"
	  "@ingroup SFX\n" );
   Con::addVariable( "$SFXEmitter::renderColorInnerCone", TypeColorI, &smRenderColorInnerCone,
      "The color with which to render dots in the inner sound cone (Editor only).\n"
	  "@ingroup SFX\n");
   Con::addVariable( "$SFXEmitter::renderColorOuterCone", TypeColorI, &smRenderColorOuterCone,
      "The color with which to render dots in the outer sound cone (Editor only).\n"
	  "@ingroup SFX\n" );
   Con::addVariable( "$SFXEmitter::renderColorOutsideVolume", TypeColorI, &smRenderColorOutsideVolume,
      "The color with which to render dots outside of the outer sound cone (Editor only).\n"
	  "@ingroup SFX\n" );
   Con::addVariable( "$SFXEmitter::renderColorRangeSphere", TypeColorI, &smRenderColorRangeSphere,
      "The color of the range sphere with which to render sound emitters in the editor.\n"
	  "@ingroup SFX\n" );
}

//-----------------------------------------------------------------------------

void SFXEmitter::initPersistFields()
{
   addGroup( "Media" );
   
      addField( "track",               TypeSFXTrackName,          Offset( mTrack, SFXEmitter),
         "The track which the emitter should play.\n"
         "@note If assigned, this field will take precedence over a #fileName that may also be assigned to the "
            "emitter." );
      addField( "fileName",            TypeStringFilename,        Offset( mLocalProfile.mFilename, SFXEmitter),
         "The sound file to play.\n"
         "Use @b either this property @b or #track.  If both are assigned, #track takes precendence.  The primary purpose of this "
         "field is to avoid the need for the user to define SFXTrack datablocks for all sounds used in a level." );
   
   endGroup( "Media");

   addGroup( "Sound" );

      addField( "playOnAdd",           TypeBool,      Offset( mPlayOnAdd, SFXEmitter ),
         "Whether playback of the emitter's sound should start as soon as the emitter object is added to the level.\n"
         "If this is true, the emitter will immediately start to play when the level is loaded." );
      addField( "useTrackDescriptionOnly", TypeBool,  Offset( mUseTrackDescriptionOnly, SFXEmitter ),
         "If this is true, all fields except for #playOnAdd and #track are ignored on the emitter object.\n"
         "This is useful to prevent fields in the #track's description from being overridden by emitter fields." );
      addField( "isLooping",           TypeBool,      Offset( mDescription.mIsLooping, SFXEmitter ),
         "Whether to play #fileName in an infinite loop.\n"
         "If a #track is assigned, the value of this field is ignored.\n"
         "@see SFXDescription::isLooping" );
      addField( "isStreaming",         TypeBool,      Offset( mDescription.mIsStreaming, SFXEmitter ),
         "Whether to use streamed playback for #fileName.\n"
         "If a #track is assigned, the value of this field is ignored.\n"
         "@see SFXDescription::isStreaming\n\n"
         "@ref SFX_streaming" );
      addField( "sourceGroup",         TypeSFXSourceName, Offset( mDescription.mSourceGroup, SFXEmitter ),
         "The SFXSource to which to assign the sound of this emitter as a child.\n"
         "@note This field is ignored if #useTrackDescriptionOnly is true.\n\n"
         "@see SFXDescription::sourceGroup" );
      addField( "volume",              TypeF32,       Offset( mDescription.mVolume, SFXEmitter ),
         "Volume level to apply to the sound.\n"
         "@note This field is ignored if #useTrackDescriptionOnly is true.\n\n"
         "@see SFXDescription::volume" );
      addField( "pitch",               TypeF32,       Offset( mDescription.mPitch, SFXEmitter ),
         "Pitch shift to apply to the sound.  Default is 1 = play at normal speed.\n"
         "@note This field is ignored if #useTrackDescriptionOnly is true.\n\n"
         "@see SFXDescription::pitch" );
      addField( "fadeInTime",          TypeF32,       Offset( mDescription.mFadeInTime, SFXEmitter ),
         "Number of seconds to gradually fade in volume from zero when playback starts.\n"
         "@note This field is ignored if #useTrackDescriptionOnly is true.\n\n"
         "@see SFXDescription::fadeInTime" );
      addField( "fadeOutTime",         TypeF32,       Offset( mDescription.mFadeOutTime, SFXEmitter ),
         "Number of seconds to gradually fade out volume down to zero when playback is stopped or paused.\n"
         "@note This field is ignored if #useTrackDescriptionOnly is true.\n\n"
         "@see SFXDescription::fadeOutTime" );

   endGroup( "Sound");

   addGroup( "3D Sound" );
   
      addField( "is3D",                TypeBool,      Offset( mDescription.mIs3D, SFXEmitter ),
         "Whether to play #fileName as a positional (3D) sound or not.\n"
         "If a #track is assigned, the value of this field is ignored.\n\n"
         "@see SFXDescription::is3D" );
      addField( "referenceDistance",   TypeF32,       Offset( mDescription.mMinDistance, SFXEmitter ),
         "Distance at which to start volume attenuation of the 3D sound.\n"
         "@note This field is ignored if #useTrackDescriptionOnly is true.\n\n"
         "@see SFXDescription::referenceDistance" );
      addField( "maxDistance",         TypeF32,       Offset( mDescription.mMaxDistance, SFXEmitter ),
         "Distance at which to stop volume attenuation of the 3D sound.\n"
         "@note This field is ignored if #useTrackDescriptionOnly is true.\n\n"
         "@see SFXDescription::maxDistance" );
      addField( "scatterDistance",     TypePoint3F,   Offset( mDescription.mScatterDistance, SFXEmitter ),
         "Bounds on random offset to apply to initial 3D sound position.\n"
         "@note This field is ignored if #useTrackDescriptionOnly is true.\n\n"
         "@see SFXDescription::scatterDistance" );
      addField( "coneInsideAngle",     TypeS32,       Offset( mDescription.mConeInsideAngle, SFXEmitter ),
         "Angle of inner volume cone of 3D sound in degrees.\n"
         "@note This field is ignored if #useTrackDescriptionOnly is true.\n\n"
         "@see SFXDescription::coneInsideAngle" );
      addField( "coneOutsideAngle",    TypeS32,       Offset( mDescription.mConeOutsideAngle, SFXEmitter ),
         "Angle of outer volume cone of 3D sound in degrees\n"
         "@note This field is ignored if #useTrackDescriptionOnly is true.\n\n"
         "@see SFXDescription::coneOutsideAngle" );
      addField( "coneOutsideVolume",   TypeF32,       Offset( mDescription.mConeOutsideVolume, SFXEmitter ),
         "Volume scale factor of outside of outer volume 3D sound cone.\n"
         "@note This field is ignored if #useTrackDescriptionOnly is true.\n\n"
         "@see SFXDescription::coneOutsideVolume" );
   
   endGroup( "3D Sound" );

   Parent::initPersistFields();
}

//-----------------------------------------------------------------------------

U32 SFXEmitter::packUpdate( NetConnection *con, U32 mask, BitStream *stream )
{
   U32 retMask = Parent::packUpdate( con, mask, stream );

   if( stream->writeFlag( mask & InitialUpdateMask ) )
   {
      // If this is the initial update then all the source
      // values are dirty and must be transmitted.
      mask |= TransformUpdateMask;
      mDirty = AllDirtyMask;

      // Clear the source masks... they are not
      // used during an initial update!
      mask &= ~AllSourceMasks;
   }

   stream->writeFlag( mPlayOnAdd );

   // transform
   if( stream->writeFlag( mask & TransformUpdateMask ) )
      stream->writeAffineTransform( mObjToWorld );

   // track
   if( stream->writeFlag( mDirty.test( Track ) ) )
      sfxWrite( stream, mTrack );

   // filename
   if( stream->writeFlag( mDirty.test( Filename ) ) )
      stream->writeString( mLocalProfile.mFilename );

   // volume
   if( stream->writeFlag( mDirty.test( Volume ) ) )
      stream->write( mDescription.mVolume );
      
   // pitch
   if( stream->writeFlag( mDirty.test( Pitch ) ) )
      stream->write( mDescription.mPitch );

   // islooping
   if( stream->writeFlag( mDirty.test( IsLooping ) ) )
      stream->writeFlag( mDescription.mIsLooping );
      
   // isStreaming
   if( stream->writeFlag( mDirty.test( IsStreaming ) ) )
      stream->writeFlag( mDescription.mIsStreaming );

   // is3d
   if( stream->writeFlag( mDirty.test( Is3D ) ) )
      stream->writeFlag( mDescription.mIs3D );

   // minDistance
   if( stream->writeFlag( mDirty.test( MinDistance ) ) )
      stream->write( mDescription.mMinDistance );

   // maxdistance
   if( stream->writeFlag( mDirty.test( MaxDistance) ) )
      stream->write( mDescription.mMaxDistance );

   // coneinsideangle
   if( stream->writeFlag( mDirty.test( ConeInsideAngle ) ) )
      stream->write( mDescription.mConeInsideAngle );

   // coneoutsideangle
   if( stream->writeFlag( mDirty.test( ConeOutsideAngle ) ) )
      stream->write( mDescription.mConeOutsideAngle );

   // coneoutsidevolume
   if( stream->writeFlag( mDirty.test( ConeOutsideVolume ) ) )
      stream->write( mDescription.mConeOutsideVolume );

   // sourcegroup
   if( stream->writeFlag( mDirty.test( SourceGroup ) ) )
      sfxWrite( stream, mDescription.mSourceGroup );
      
   // fadein
   if( stream->writeFlag( mDirty.test( FadeInTime ) ) )
      stream->write( mDescription.mFadeInTime );
      
   // fadeout
   if( stream->writeFlag( mDirty.test( FadeOutTime ) ) )
      stream->write( mDescription.mFadeOutTime );
      
   // scatterdistance
   if( stream->writeFlag( mDirty.test( ScatterDistance ) ) )
      mathWrite( *stream, mDescription.mScatterDistance );

   mDirty.clear();
   
   stream->writeFlag( mUseTrackDescriptionOnly );

   // We should never have both source masks 
   // enabled at the same time!
   AssertFatal( ( mask & AllSourceMasks ) != AllSourceMasks, 
      "SFXEmitter::packUpdate() - Bad source mask!" );

   // Write the source playback state.
   stream->writeFlag( mask & SourcePlayMask );
   stream->writeFlag( mask & SourceStopMask );

   return retMask;
}

//-----------------------------------------------------------------------------

bool SFXEmitter::_readDirtyFlag( BitStream* stream, U32 mask )
{
   bool flag = stream->readFlag();
   if ( flag )
      mDirty.set( mask );

   return flag;
}

//-----------------------------------------------------------------------------

void SFXEmitter::unpackUpdate( NetConnection *conn, BitStream *stream )
{
   Parent::unpackUpdate( conn, stream );

   // initial update?
   bool initialUpdate = stream->readFlag();

   mPlayOnAdd = stream->readFlag();

   // transform
   if ( _readDirtyFlag( stream, Transform ) )
   {
      MatrixF mat;
      stream->readAffineTransform(&mat);
      Parent::setTransform(mat);
   }

   // track
   if ( _readDirtyFlag( stream, Track ) )
   {
      String errorStr;
      if( !sfxReadAndResolve( stream, &mTrack, errorStr ) )
         Con::errorf( "%s", errorStr.c_str() );
   }

   // filename
   if ( _readDirtyFlag( stream, Filename ) )
      mLocalProfile.mFilename = stream->readSTString();

   // volume
   if ( _readDirtyFlag( stream, Volume ) )
      stream->read( &mDescription.mVolume );
      
   // pitch
   if( _readDirtyFlag( stream, Pitch ) )
      stream->read( &mDescription.mPitch );

   // islooping
   if ( _readDirtyFlag( stream, IsLooping ) )
      mDescription.mIsLooping = stream->readFlag();
      
   if( _readDirtyFlag( stream, IsStreaming ) )
      mDescription.mIsStreaming = stream->readFlag();

   // is3d
   if ( _readDirtyFlag( stream, Is3D ) )
      mDescription.mIs3D = stream->readFlag();

   // mindistance
   if ( _readDirtyFlag( stream, MinDistance ) )
      stream->read( &mDescription.mMinDistance );

   // maxdistance
   if ( _readDirtyFlag( stream, MaxDistance ) )
   {
      stream->read( &mDescription.mMaxDistance );
      mObjScale.set( mDescription.mMaxDistance, mDescription.mMaxDistance, mDescription.mMaxDistance );
   }

   // coneinsideangle
   if ( _readDirtyFlag( stream, ConeInsideAngle ) )
      stream->read( &mDescription.mConeInsideAngle );

   // coneoutsideangle
   if ( _readDirtyFlag( stream, ConeOutsideAngle ) )
      stream->read( &mDescription.mConeOutsideAngle );

   // coneoutsidevolume
   if ( _readDirtyFlag( stream, ConeOutsideVolume ) )
      stream->read( &mDescription.mConeOutsideVolume );

   // sourcegroup
   if ( _readDirtyFlag( stream, SourceGroup ) )
   {
      String errorStr;
      if( !sfxReadAndResolve( stream, &mDescription.mSourceGroup, errorStr ) )
         Con::errorf( "%s", errorStr.c_str() );
   }
      
   // fadein
   if ( _readDirtyFlag( stream, FadeInTime ) )
      stream->read( &mDescription.mFadeInTime );
      
   // fadeout
   if( _readDirtyFlag( stream, FadeOutTime ) )
      stream->read( &mDescription.mFadeOutTime );
      
   // scatterdistance
   if( _readDirtyFlag( stream, ScatterDistance ) )
      mathRead( *stream, &mDescription.mScatterDistance );
      
   mUseTrackDescriptionOnly = stream->readFlag();

   // update the emitter now?
   if ( !initialUpdate )
      _update();

   // Check the source playback masks.
   if ( stream->readFlag() ) // SourcePlayMask
      play();
   if ( stream->readFlag() ) // SourceStopMask
      stop();
}

//-----------------------------------------------------------------------------

void SFXEmitter::onStaticModified( const char* slotName, const char* newValue )
{
   // NOTE: The signature for this function is very 
   // misleading... slotName is a StringTableEntry.

   // We don't check for changes on the client side.
   if ( isClientObject() )
      return;

   // Lookup and store the property names once here
   // and we can then just do pointer compares. 
   static StringTableEntry slotPosition   = StringTable->lookup( "position" );
   static StringTableEntry slotRotation   = StringTable->lookup( "rotation" );
   static StringTableEntry slotScale      = StringTable->lookup( "scale" );
   static StringTableEntry slotTrack      = StringTable->lookup( "track" );
   static StringTableEntry slotFilename   = StringTable->lookup( "fileName" );
   static StringTableEntry slotVolume     = StringTable->lookup( "volume" );
   static StringTableEntry slotPitch      = StringTable->lookup( "pitch" );
   static StringTableEntry slotIsLooping  = StringTable->lookup( "isLooping" );
   static StringTableEntry slotIsStreaming= StringTable->lookup( "isStreaming" );
   static StringTableEntry slotIs3D       = StringTable->lookup( "is3D" );
   static StringTableEntry slotRefDist    = StringTable->lookup( "referenceDistance" );
   static StringTableEntry slotMaxDist    = StringTable->lookup( "maxDistance" );
   static StringTableEntry slotConeInAng  = StringTable->lookup( "coneInsideAngle" );
   static StringTableEntry slotConeOutAng = StringTable->lookup( "coneOutsideAngle" );
   static StringTableEntry slotConeOutVol = StringTable->lookup( "coneOutsideVolume" );
   static StringTableEntry slotFadeInTime = StringTable->lookup( "fadeInTime" );
   static StringTableEntry slotFadeOutTime= StringTable->lookup( "fadeOutTime" );
   static StringTableEntry slotScatterDistance = StringTable->lookup( "scatterDistance" );
   static StringTableEntry slotSourceGroup= StringTable->lookup( "sourceGroup" );
   static StringTableEntry slotUseTrackDescriptionOnly = StringTable->lookup( "useTrackDescriptionOnly" );

   // Set the dirty flags.
   mDirty.clear();
   if(  slotName == slotPosition ||
         slotName == slotRotation ||
         slotName == slotScale )
      mDirty.set( Transform );

   else if( slotName == slotTrack )
      mDirty.set( Track );

   else if( slotName == slotFilename )
      mDirty.set( Filename );

   else if( slotName == slotVolume )
      mDirty.set( Volume );
      
   else if( slotName == slotPitch )
      mDirty.set( Pitch );

   else if( slotName == slotIsLooping )
      mDirty.set( IsLooping );
      
   else if( slotName == slotIsStreaming )
      mDirty.set( IsStreaming );

   else if( slotName == slotIs3D )
      mDirty.set( Is3D );

   else if( slotName == slotRefDist )
      mDirty.set( MinDistance );

   else if( slotName == slotMaxDist )
      mDirty.set( MaxDistance );

   else if( slotName == slotConeInAng )
      mDirty.set( ConeInsideAngle );

   else if( slotName == slotConeOutAng )
      mDirty.set( ConeOutsideAngle );

   else if( slotName == slotConeOutVol )
      mDirty.set( ConeOutsideVolume );
      
   else if( slotName == slotFadeInTime )
      mDirty.set( FadeInTime );
      
   else if( slotName == slotFadeOutTime )
      mDirty.set( FadeOutTime );
      
   else if( slotName == slotScatterDistance )
      mDirty.set( ScatterDistance );
      
   else if( slotName == slotSourceGroup )
      mDirty.set( SourceGroup );
      
   else if( slotName == slotUseTrackDescriptionOnly )
      mDirty.set( TrackOnly );

   if( mDirty )
      setMaskBits( DirtyUpdateMask );
}

//-----------------------------------------------------------------------------

void SFXEmitter::inspectPostApply()
{
   // Parent will call setScale so sync up scale with distance.
   
   F32 maxDistance = mDescription.mMaxDistance;
   if( mUseTrackDescriptionOnly && mTrack )
      maxDistance = mTrack->getDescription()->mMaxDistance;
      
   mObjScale.set( maxDistance, maxDistance, maxDistance );
   
   Parent::inspectPostApply();
}

//-----------------------------------------------------------------------------

bool SFXEmitter::onAdd()
{
   if( !Parent::onAdd() )
      return false;

   if( isServerObject() )
   {
      // Validate the data we'll be passing across
      // the network to the client.
      mDescription.validate();
      
      // Read an old 'profile' field for backwards-compatibility.
      
      if( !mTrack )
      {
         static const char* sProfile = StringTable->insert( "profile" );
         const char* profileName = getDataField( sProfile, NULL );
         if( profileName &&  profileName[ 0 ] )
         {
            if( !Sim::findObject( profileName, mTrack ) )
               Con::errorf( "SFXEmitter::onAdd - No SFXTrack '%s' in SFXEmitter '%i' (%s)", profileName, getId(), getName() );
            else
            {
               // Remove the old 'profile' field.
               setDataField( sProfile, NULL, "" );
            }
         }
      }

      // Convert a legacy 'channel' field, if we have one.
      
      static const char* sChannel = StringTable->insert( "channel" );
      const char* channelValue = getDataField( sChannel, NULL );
      if( channelValue && channelValue[ 0 ] )
      {
         const char* group = Con::evaluatef( "return sfxOldChannelToGroup( %s );", channelValue );
         SFXSource* sourceGroup;
         if( !Sim::findObject( group, sourceGroup ) )
            Con::errorf( "SFXEmitter::onAdd - could not resolve channel '%s' to SFXSource", channelValue );
         else
         {
            static const char* sSourceGroup = StringTable->insert( "sourceGroup" );
            setDataField( sSourceGroup, NULL, sourceGroup->getIdString() );
            
            // Remove the old 'channel' field.
            setDataField( sChannel, NULL, "" );
         }
      }
   }
   else
   {
      _update();

      // Do we need to start playback?
      if( mPlayOnAdd && mSource )
         mSource->play();
   }
   
   // Setup the bounds.

   mObjScale.set( mDescription.mMaxDistance, mDescription.mMaxDistance, mDescription.mMaxDistance );
   resetWorldBox();

   addToScene();
   return true;
}

//-----------------------------------------------------------------------------

void SFXEmitter::onRemove()
{
   SFX_DELETE( mSource );

   removeFromScene();
   Parent::onRemove();
}

//-----------------------------------------------------------------------------

void SFXEmitter::_update()
{
   AssertFatal( isClientObject(), "SFXEmitter::_update() - This shouldn't happen on the server!" );

   // Store the playback status so we
   // we can restore it.
   SFXStatus prevState = mSource ? mSource->getStatus() : SFXStatusNull;

   // Make sure all the settings are valid.
   mDescription.validate();

   const MatrixF& transform = getTransform();
   const VectorF& velocity = getVelocity();
   
   // Did we change the source?
   if( mDirty.test( Track | Filename | Is3D | IsLooping | IsStreaming | TrackOnly ) )
   {
      SFX_DELETE( mSource );

      // Do we have a track?
      if( mTrack )
      {
         mSource = SFX->createSource( mTrack, &transform, &velocity );
         if( !mSource )
            Con::errorf( "SFXEmitter::_update() - failed to create sound for track %i (%s)",
               mTrack->getId(), mTrack->getName() );

         // If we're supposed to play when the emitter is 
         // added to the scene then also restart playback 
         // when the profile changes.
         prevState = mPlayOnAdd ? SFXStatusPlaying : prevState;
         
         // Force an update of properties set on the local description.
         
         mDirty.set( AllDirtyMask );
      }
      
      // Else take the local profile
      else
      {
         // Clear the resource and buffer on profile
         // to force reload.

         mLocalProfile.mResource = NULL;
         mLocalProfile.mBuffer = NULL;

         if( !mLocalProfile.mFilename.isEmpty() )
         {
            mSource = SFX->createSource( &mLocalProfile, &transform, &velocity );
            if( !mSource )
               Con::errorf( "SFXEmitter::_update() - failed to create sound for: %s",
                  mLocalProfile.mFilename.c_str() );
            
            prevState = mPlayOnAdd ? SFXStatusPlaying : prevState;
         }
      }
      
      mDirty.clear( Track | Filename | Is3D | IsLooping | IsStreaming | TrackOnly );
   }

   // Cheat if the editor is open and the looping state
   // is toggled on a local profile sound.  It makes the
   // editor feel responsive and that things are working.
   if(  gEditingMission &&
        !mTrack && 
        mPlayOnAdd && 
        mDirty.test( IsLooping ) )
      prevState = SFXStatusPlaying;
      
   bool useTrackDescriptionOnly = ( mUseTrackDescriptionOnly && mTrack );

   // The rest only applies if we have a source.
   if( mSource )
   {
      // Set the volume irrespective of the profile.
      if( mDirty.test( Volume ) && !useTrackDescriptionOnly )
         mSource->setVolume( mDescription.mVolume );
         
      if( mDirty.test( Pitch ) && !useTrackDescriptionOnly )
         mSource->setPitch( mDescription.mPitch );
         
      if( mDirty.test( FadeInTime | FadeOutTime ) && !useTrackDescriptionOnly )
         mSource->setFadeTimes( mDescription.mFadeInTime, mDescription.mFadeOutTime );
         
      if( mDirty.test( SourceGroup ) && mDescription.mSourceGroup && !useTrackDescriptionOnly )
         mDescription.mSourceGroup->addObject( mSource );

      // Skip these 3d only settings.
      if( mDescription.mIs3D )
      {
         if( mDirty.test( Transform ) )
         {
            mSource->setTransform( transform );
            mSource->setVelocity( velocity );
         }
         
         if( mDirty.test( MinDistance | MaxDistance ) && !useTrackDescriptionOnly )
         {
            mSource->setMinMaxDistance(   mDescription.mMinDistance,
                                          mDescription.mMaxDistance );
         }

         if( mDirty.test( ConeInsideAngle | ConeOutsideAngle | ConeOutsideVolume ) && !useTrackDescriptionOnly )
         {
            mSource->setCone( F32( mDescription.mConeInsideAngle ),
                              F32( mDescription.mConeOutsideAngle ),
                              mDescription.mConeOutsideVolume );
         }
         
         mDirty.clear( Transform | MinDistance | MaxDistance | ConeInsideAngle | ConeOutsideAngle | ConeOutsideVolume );
      }     

      // Restore the pre-update playback state.
      if( prevState == SFXStatusPlaying )
         mSource->play();
         
      mDirty.clear( Volume | Pitch | Transform | FadeInTime | FadeOutTime | SourceGroup );
   }
}

//-----------------------------------------------------------------------------

void SFXEmitter::prepRenderImage( SceneRenderState* state )
{
   // Only render in editor.
   if( !gEditingMission )
      return;

   ObjectRenderInst* ri = state->getRenderPass()->allocInst< ObjectRenderInst >();

   ri->renderDelegate.bind( this, &SFXEmitter::_renderObject );
   ri->type = RenderPassManager::RIT_Editor;
   ri->defaultKey = 0;
   ri->defaultKey2 = 0;

   state->getRenderPass()->addInst( ri );
}

//-----------------------------------------------------------------------------

void SFXEmitter::_renderObject( ObjectRenderInst* ri, SceneRenderState* state, BaseMatInstance* overrideMat )
{   
   // Check to see if the emitter is in range and playing
   // and assign a proper color depending on this.

   ColorI color;
   if( _getPlaybackStatus() == SFXStatusPlaying )
   {
      if( isInRange() )
         color = smRenderColorPlayingInRange;
      else
         color = smRenderColorPlayingOutOfRange;
   }
   else
   {
      if( isInRange() )
         color = smRenderColorStoppedInRange;
      else
         color = smRenderColorStoppedOutOfRange;
   }

   // Draw the cube.
   
   GFXStateBlockDesc desc;
   desc.setZReadWrite( true, false );
   desc.setBlend( true );
   desc.setCullMode( GFXCullNone );

   GFXDrawUtil *drawer = GFX->getDrawUtil();
   drawer->drawCube( desc,  Point3F( 0.5f, 0.5f, 0.5f ), getBoxCenter(), color );
   
   // Render visual feedback for 3D sounds.
   
   if( ( smRenderEmitters || isSelected() ) && is3D() )
      _render3DVisualFeedback();
}

//-----------------------------------------------------------------------------

void SFXEmitter::_render3DVisualFeedback()
{
   GFXTransformSaver saver;
   
   GFX->multWorld( getRenderTransform() );
   
   GFXStateBlockDesc desc;
   desc.setZReadWrite( true, false );
   desc.setBlend( true );
   desc.setCullMode( GFXCullNone );

   if( mRenderSB == NULL )
      mRenderSB = GFX->createStateBlock( desc );
   
   GFX->setStateBlock( mRenderSB );
   
   // Render the max range sphere.
   
   if( smRenderColorRangeSphere.alpha > 0 )
      GFX->getDrawUtil()->drawSphere( desc, mDescription.mMaxDistance, Point3F( 0.f, 0.f, 0.f ), smRenderColorRangeSphere );
   
   //TODO: some point size support in GFX would be nice

   // Prepare primitive list.  Make sure we stay within limits.
   
   F32 radialIncrements = smRenderRadialIncrements;
   F32 sweepIncrements = smRenderSweepIncrements;
   F32 pointDistance = smRenderPointDistance;
   
   F32 numPoints;
   while( 1 )
   {
      numPoints = mCeil( 360.f / radialIncrements ) *
                  mCeil( 360.f / sweepIncrements ) *
                  ( mDescription.mMaxDistance / pointDistance );
                  
      if( numPoints < 65536 )
         break;
         
      radialIncrements *= 1.1f;
      sweepIncrements *= 1.1f;
      pointDistance *= 1.5;
   }
                           
   PrimBuild::begin( GFXPointList, numPoints );

   // Render inner cone.
   
   _renderCone(
      radialIncrements,
      sweepIncrements,
      pointDistance,
      mDescription.mConeInsideAngle, 0.f,
      mDescription.mVolume, mDescription.mVolume,
      smRenderColorInnerCone );

   // Outer Cone and Outside volume only get rendered if mConeOutsideVolume > 0
   
   if( mDescription.mConeOutsideVolume > 0.f )
   {
      const F32 outsideVolume = mDescription.mVolume * mDescription.mConeOutsideVolume;
      
      // Render outer cone.
      
      _renderCone(
         radialIncrements,
         sweepIncrements,
         pointDistance,
         mDescription.mConeOutsideAngle, mDescription.mConeInsideAngle,
         outsideVolume, mDescription.mVolume,
         smRenderColorOuterCone );
      
      // Render outside volume.
      
      _renderCone(
         radialIncrements,
         sweepIncrements,
         pointDistance,
         360.f, mDescription.mConeOutsideAngle,
         outsideVolume, outsideVolume,
         smRenderColorOutsideVolume );
   }
   
   // Commit primitive list.
   
   PrimBuild::end();
}

//-----------------------------------------------------------------------------

void SFXEmitter::_renderCone( F32 radialIncrements, F32 sweepIncrements,
                              F32 pointDistance,
                              F32 startAngle, F32 stopAngle,
                              F32 startVolume, F32 stopVolume,
                              const ColorI& color )
{
   if( startAngle == stopAngle )
      return;
      
   const F32 startAngleRadians = mDegToRad( startAngle );
   const F32 stopAngleRadians = mDegToRad( stopAngle );
   const F32 radialIncrementsRadians = mDegToRad( radialIncrements );
      
   // Unit quaternions representing the start and end angle so we
   // can interpolate between the two without flipping.
   
   QuatF rotateZStart( EulerF( 0.f, 0.f, startAngleRadians / 2.f ) );
   QuatF rotateZEnd( EulerF( 0.f, 0.f, stopAngleRadians / 2.f ) );
   
   // Do an angular sweep on one side of our XY disc.  Since we do a full 360 radial sweep
   // around Y for each angle, we only need to sweep over one side.
   
   const F32 increment = 1.f / ( ( ( startAngle / 2.f ) - ( stopAngle / 2.f ) ) / sweepIncrements );
   for( F32 t = 0.f; t < 1.0f; t += increment )
   {
      // Quaternion to rotate point into place on XY disc.
      QuatF rotateZ;
      rotateZ.interpolate( rotateZStart, rotateZEnd, t );
      
      // Quaternion to rotate one position around Y axis.  Used for radial sweep.
      QuatF rotateYOne( EulerF( 0.f, radialIncrementsRadians, 0.f ) );

      // Do a radial sweep each step along the distance axis.  For each step, volume is
      // the same for any point on the sweep circle.

      for( F32 y = pointDistance; y <= mDescription.mMaxDistance; y += pointDistance )
      {
         ColorI c = color;
         
         // Compute volume at current point.  First off, find the interpolated volume
         // in the cone.  Only for the outer cone will this actually result in
         // interpolation.  For the remaining angles, the cone volume is constant.

         F32 volume = mLerp( startVolume, stopVolume, t );
         if( volume == 0.f )
            c.alpha = 0;
         else
         {         
            // Apply distance attenuation.
            
            F32 attenuatedVolume = SFXDistanceAttenuation(
               SFX->getDistanceModel(),
               mDescription.mMinDistance,
               mDescription.mMaxDistance,
               y,
               volume,
               SFX->getRolloffFactor() ); //RDTODO

            // Fade alpha according to how much volume we
            // have left at the current point.
            
            c.alpha = F32( c.alpha ) * ( attenuatedVolume / 1.f );
         }
         
         PrimBuild::color( c );
         
         // Create points by doing a full 360 degree radial sweep around Y.

         Point3F p( 0.f, y, 0.f );
         rotateZ.mulP( p, &p );
         
         for( F32 radialAngle = 0.f; radialAngle < 360.f; radialAngle += radialIncrements )
         {
            PrimBuild::vertex3f( p.x, p.y, p.z );
            rotateYOne.mulP( p, &p );
         }
      }
   }
}

//-----------------------------------------------------------------------------

void SFXEmitter::play()
{
   if( mSource )
      mSource->play();
   else
   {
      // By clearing the playback masks first we
      // ensure the last playback command called 
      // within a single tick is the one obeyed.
      clearMaskBits( AllSourceMasks );

      setMaskBits( SourcePlayMask );
   }
}

//-----------------------------------------------------------------------------

void SFXEmitter::stop()
{
   if ( mSource )
      mSource->stop();
   else
   {
      // By clearing the playback masks first we
      // ensure the last playback command called 
      // within a single tick is the one obeyed.
      clearMaskBits( AllSourceMasks );

      setMaskBits( SourceStopMask );
   }
}

//-----------------------------------------------------------------------------

SFXStatus SFXEmitter::_getPlaybackStatus() const
{
   const SFXEmitter* emitter = this;

   // We only have a source playing on client objects, so if this is a server
   // object, we want to know the playback status on the local client connection's
   // version of this emitter.
   
   if( isServerObject() )
   {
      S32 index = NetConnection::getLocalClientConnection()->getGhostIndex( ( NetObject* ) this );
      if( index != -1 )
         emitter = dynamic_cast< SFXEmitter* >( NetConnection::getConnectionToServer()->resolveGhost( index ) );
      else
         emitter = NULL;
   }
   
   if( emitter && emitter->mSource )
      return emitter->mSource->getStatus();
      
   return SFXStatusNull;
}

//-----------------------------------------------------------------------------

bool SFXEmitter::is3D() const
{
   if( mTrack != NULL )
      return mTrack->getDescription()->mIs3D;
   else
      return mDescription.mIs3D;
}

//-----------------------------------------------------------------------------

bool SFXEmitter::isInRange() const
{
   if( !mDescription.mIs3D )
      return false;
   
   const SFXListenerProperties& listener = SFX->getListener();
   const Point3F listenerPos = listener.getTransform().getPosition();
   const Point3F emitterPos = getPosition();
   const F32 dist = mDescription.mMaxDistance;
   
   return ( ( emitterPos - listenerPos ).len() <= dist );
}

//-----------------------------------------------------------------------------

void SFXEmitter::setTransform( const MatrixF &mat )
{
   // Set the transform directly from the 
   // matrix created by inspector.
   Parent::setTransform( mat );
   setMaskBits( TransformUpdateMask );
}

//-----------------------------------------------------------------------------

void SFXEmitter::setScale( const VectorF &scale )
{
   F32 maxDistance;
   
   if( mUseTrackDescriptionOnly && mTrack )
      maxDistance = mTrack->getDescription()->mMaxDistance;
   else
   {
      // Use the average of the three coords.
      maxDistance = ( scale.x + scale.y + scale.z ) / 3.0f;
      maxDistance = getMax( maxDistance, mDescription.mMinDistance );
      
      mDescription.mMaxDistance = maxDistance;
      
      mDirty.set( MaxDistance );
      setMaskBits( DirtyUpdateMask );
   }

   Parent::setScale( VectorF( maxDistance, maxDistance, maxDistance ) );
}

//=============================================================================
//    Console Methods.
//=============================================================================
// MARK: ---- Console Methods ----

//-----------------------------------------------------------------------------

DefineEngineMethod( SFXEmitter, play, void, (),,
   "Manually start playback of the emitter's sound.\n"
   "If this is called on the server-side object, the play command will be related to all client-side ghosts.\n" )
{
   object->play();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SFXEmitter, stop, void, (),,
   "Manually stop playback of the emitter's sound.\n"
   "If this is called on the server-side object, the stop command will be related to all client-side ghosts.\n" )
{
   object->stop();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SFXEmitter, getSource, SFXSource*, (),,
   "Get the sound source object from the emitter.\n\n"
   "@return The sound source used by the emitter or null."
   "@note This method will return null when called on the server-side SFXEmitter object.  Only client-side ghosts "
      "actually hold on to %SFXSources.\n\n" )
{
   return object->getSource();
}
