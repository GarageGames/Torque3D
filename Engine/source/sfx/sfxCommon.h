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

#ifndef _SFXCOMMON_H_
#define _SFXCOMMON_H_

#ifndef _PLATFORM_H_
   #include "platform/platform.h"
#endif
#ifndef _MMATHFN_H_
   #include "math/mMathFn.h"
#endif
#ifndef _MRANDOM_H_
   #include "math/mRandom.h"
#endif
#ifndef _MMATRIX_H_
   #include "math/mMatrix.h"
#endif
#ifndef _MPOINT3_H_
   #include "math/mPoint3.h"
#endif
#ifndef _TYPETRAITS_H_
   #include "platform/typetraits.h"
#endif
#ifndef _DYNAMIC_CONSOLETYPES_H_
   #include "console/dynamicTypes.h"
#endif



class SFXEnvironment;
class SFXPlayList;


//-----------------------------------------------------------------------------
//    SFXStatus.
//-----------------------------------------------------------------------------


/// The sound playback state.
enum SFXStatus 
{
   /// Initial state; no operation yet performed on sound.
   SFXStatusNull,

   /// Sound is playing.
   SFXStatusPlaying,

   /// Sound has been stopped.
   SFXStatusStopped,

   /// Sound is paused.
   SFXStatusPaused,

   /// Sound stream is starved and playback blocked.
   SFXStatusBlocked,

   /// Temporary state while transitioning to another state.  This is used when multiple
   /// threads concurrently maintain a status and need to perform a sequence of actions before
   /// being able to fully go from a previous to a new current state.  In this case, the
   /// transition state marks the status as being under update on another thread.
   ///
   /// @note Not all places that use SFXStatus actually use this state.
   SFXStatusTransition,
};

DefineEnumType( SFXStatus );


inline const char* SFXStatusToString( SFXStatus status )
{
   switch ( status )
   {
      case SFXStatusPlaying:     return "playing";
      case SFXStatusStopped:     return "stopped";
      case SFXStatusPaused:      return "paused";
      case SFXStatusBlocked:     return "blocked";
      case SFXStatusTransition:  return "transition";
      
      case SFXStatusNull:
      default: ;
   }
   
   return "null";
}


//-----------------------------------------------------------------------------
//    SFXChannel.
//-----------------------------------------------------------------------------


/// Animatable channels in the SFX system.
enum SFXChannel
{
   SFXChannelVolume,
   SFXChannelPitch,
   SFXChannelPriority,
   SFXChannelPositionX,
   SFXChannelPositionY,
   SFXChannelPositionZ,
   SFXChannelRotationX,
   SFXChannelRotationY,
   SFXChannelRotationZ,
   SFXChannelVelocityX,
   SFXChannelVelocityY,
   SFXChannelVelocityZ,
   SFXChannelMinDistance,
   SFXChannelMaxDistance,
   SFXChannelConeInsideAngle,
   SFXChannelConeOutsideAngle,
   SFXChannelConeOutsideVolume,
   SFXChannelCursor,
   SFXChannelStatus,
   SFXChannelUser0,
   SFXChannelUser1,
   SFXChannelUser2,
   SFXChannelUser3,
   
   /// Total number of animatable channels.
   SFX_NUM_CHANNELS
};

DefineEnumType( SFXChannel );


//-----------------------------------------------------------------------------
//    SFXDistanceModel.
//-----------------------------------------------------------------------------


/// Rolloff curve used for distance volume attenuation of 3D sounds.
enum SFXDistanceModel
{
   SFXDistanceModelLinear,             ///< Volume decreases linearly from min to max where it reaches zero.
   SFXDistanceModelLogarithmic,        ///< Volume halves every min distance steps starting from min distance; attenuation stops at max distance.
};

DefineEnumType( SFXDistanceModel );

/// Compute the distance attenuation based on the given distance model.
///
/// @param minDistance Reference distance; attenuation starts here.
/// @param maxDistance
/// @param distance Actual distance of sound from listener.
/// @param volume Unattenuated volume.
/// @param rolloffFactor Rolloff curve scale factor.
///
/// @return The attenuated volume.
inline F32 SFXDistanceAttenuation( SFXDistanceModel model, F32 minDistance, F32 maxDistance, F32 distance, F32 volume, F32 rolloffFactor )
{
   F32 gain = 1.0f;
      
   switch( model )
   {
      case SFXDistanceModelLinear:
      
         distance = getMax( distance, minDistance );
         distance = getMin( distance, maxDistance );
         
         gain = ( 1 - ( distance - minDistance ) / ( maxDistance - minDistance ) );
         break;
                  
      case SFXDistanceModelLogarithmic:
      
         distance = getMax( distance, minDistance );
         distance = getMin( distance, maxDistance );
         
         gain = minDistance / ( minDistance + rolloffFactor * ( distance - minDistance ) );
         break;
         
   }
   
   return ( volume * gain );
}


//-----------------------------------------------------------------------------
//    SFXFormat.
//-----------------------------------------------------------------------------


/// This class defines the various types of sound data that may be
/// used in the sound system.
///
/// Unlike with most sound APIs, we consider each sample point to comprise
/// all channels in a sound stream rather than only one value for a single
/// channel.
class SFXFormat
{
   protected:

      /// The number of sound channels in the data.
      U8 mChannels;

      /// The number of bits per sound sample.
      U8 mBitsPerSample;

      /// The frequency in samples per second.
      U32 mSamplesPerSecond;

   public:

      SFXFormat(  U8 channels = 0,                  
                  U8 bitsPerSample = 0,
                  U32 samplesPerSecond = 0 )
         :  mChannels( channels ),
            mSamplesPerSecond( samplesPerSecond ), 
            mBitsPerSample( bitsPerSample )
      {}

      /// Copy constructor.
      SFXFormat( const SFXFormat &format )
         :  mChannels( format.mChannels ),
            mBitsPerSample( format.mBitsPerSample ),
            mSamplesPerSecond( format.mSamplesPerSecond )
      {}

   public:

      /// Sets the format.
      void set(   U8 channels,                  
                  U8 bitsPerSample,
                  U32 samplesPerSecond )
      {
         mChannels = channels;
         mBitsPerSample = bitsPerSample;
         mSamplesPerSecond = samplesPerSecond;
      }

      /// Comparision between formats.
      bool operator == ( const SFXFormat& format ) const 
      { 
         return   mChannels == format.mChannels && 
                  mBitsPerSample == format.mBitsPerSample &&
                  mSamplesPerSecond == format.mSamplesPerSecond;
      }

      /// Returns the number of sound channels.
      U8 getChannels() const { return mChannels; }

      /// Returns true if there is a single sound channel.
      bool isMono() const { return mChannels == 1; }

      /// Is true if there are two sound channels.
      bool isStereo() const { return mChannels == 2; }

      /// Is true if there are more than two sound channels.
      bool isMultiChannel() const { return mChannels > 2; }

      /// 
      U32 getSamplesPerSecond() const { return mSamplesPerSecond; }

      /// The bits of data per channel.
      U8 getBitsPerChannel() const { return mBitsPerSample / mChannels; }

      /// The number of bytes of data per channel.
      U8 getBytesPerChannel() const { return getBitsPerChannel() / 8; }

      /// The number of bits per sound sample.
      U8 getBitsPerSample() const { return mBitsPerSample; }

      /// The number of bytes of data per sample.
      /// @note Be aware that this comprises all channels.
      U8 getBytesPerSample() const { return mBitsPerSample / 8; }

      /// Returns the duration from the sample count.
      U32 getDuration( U32 samples ) const
      {
         // Use 64bit types to avoid overflow during division. 
         return ( (U64)samples * (U64)1000 ) / (U64)mSamplesPerSecond;
      }

      ///
      U32 getSampleCount( U32 ms ) const
      {
         return U64( mSamplesPerSecond ) * U64( ms ) / U64( 1000 );
      }

      /// Returns the data length in bytes.
      U32 getDataLength( U32 ms ) const
      {
         U32 bytes = ( ( (U64)ms * (U64)mSamplesPerSecond ) * (U64)getBytesPerSample() ) / (U64)1000;
         return bytes;
      }
};


//-----------------------------------------------------------------------------
//    SFXReverb.
//-----------------------------------------------------------------------------


/// Reverb environment properties.
///
/// @note A given device may not implement all properties.
class SFXReverbProperties
{
   public:
   
      typedef void Parent;
         
      F32   mEnvSize;
      F32   mEnvDiffusion;
      S32   mRoom;
      S32   mRoomHF;
      S32   mRoomLF;
      F32   mDecayTime;
      F32   mDecayHFRatio;
      F32   mDecayLFRatio;
      S32   mReflections;
      F32   mReflectionsDelay;
      F32   mReflectionsPan[ 3 ];
      S32   mReverb;
      F32   mReverbDelay;
      F32   mReverbPan[ 3 ];
      F32   mEchoTime;
      F32   mEchoDepth;
      F32   mModulationTime;
      F32   mModulationDepth;
      F32   mAirAbsorptionHF;
      F32   mHFReference;
      F32   mLFReference;
      F32   mRoomRolloffFactor;
      F32   mDiffusion;
      F32   mDensity;
      S32   mFlags;
      
      SFXReverbProperties()
         : mEnvSize( 7.5f ),
           mEnvDiffusion( 1.0f ),
           mRoom( -1000 ),
           mRoomHF( -100 ),
           mRoomLF( 0 ),
           mDecayTime( 1.49f ),
           mDecayHFRatio( 0.83f ),
           mDecayLFRatio( 1.0f ),
           mReflections( -2602 ),
           mReflectionsDelay( 0.007f ),
           mReverb( 200 ),
           mReverbDelay( 0.011f ),
           mEchoTime( 0.25f ),
           mEchoDepth( 0.0f ),
           mModulationTime( 0.25f ),
           mModulationDepth( 0.0f ),
           mAirAbsorptionHF( -5.0f ),
           mHFReference( 5000.0f ),
           mLFReference( 250.0f ),
           mRoomRolloffFactor( 0.0f ),
           mDiffusion( 100.0f ),
           mDensity( 100.0f ),
           mFlags( 0 )
      {
         mReflectionsPan[ 0 ] = 0.0f;
         mReflectionsPan[ 1 ] = 0.0f;
         mReflectionsPan[ 2 ] = 0.0f;
         
         mReverbPan[ 0 ] = 0.0f;
         mReverbPan[ 1 ] = 0.0f;
         mReverbPan[ 2 ] = 0.0f;
      }
      
      void validate()
      {
         mEnvSize                = mClampF( mEnvSize,                1.0f,     100.0f );
         mEnvDiffusion           = mClampF( mEnvDiffusion,           0.0f,     1.0f );
         mRoom                   = mClamp( mRoom,                    -10000,  0 );
         mRoomHF                 = mClamp( mRoomHF,                  -10000,  0 );
         mRoomLF                 = mClamp( mRoomLF,                  -10000,  0 );
         mDecayTime              = mClampF( mDecayTime,              0.1f,     20.0f );
         mDecayHFRatio           = mClampF( mDecayHFRatio,           0.1f,     2.0f );
         mDecayLFRatio           = mClampF( mDecayLFRatio,           0.1f,     2.0f );
         mReflections            = mClamp( mReflections,             -10000,  1000 );
         mReflectionsDelay       = mClampF( mReflectionsDelay,       0.0f,     0.3f );
         mReverb                 = mClamp( mReverb,                  -10000,  2000 );
         mReverbDelay            = mClampF( mReverbDelay,            0.0f,     0.1f );
         mEchoTime               = mClampF( mEchoTime,               0.075f,   0.25f );
         mEchoDepth              = mClampF( mEchoDepth,              0.0f,     1.0f );
         mModulationTime         = mClampF( mModulationTime,         0.04f,    4.0f );
         mModulationDepth        = mClampF( mModulationDepth,        0.0f,     1.0f );
         mAirAbsorptionHF        = mClampF( mAirAbsorptionHF,        -100.0f,    0.0f );
         mHFReference            = mClampF( mHFReference,            1000.0f,  20000.0f );
         mLFReference            = mClampF( mLFReference,            20.0f,    1000.0f );
         mRoomRolloffFactor      = mClampF( mRoomRolloffFactor,      0.0f,     10.0f );
         mDiffusion              = mClampF( mDiffusion,              0.0f,     100.0f );
         mDensity                = mClampF( mDensity,                0.0f,     100.0f );
      }
};


//-----------------------------------------------------------------------------
//    SFXSoundReverbProperties.
//-----------------------------------------------------------------------------


/// Sound reverb properties.
///
/// @note A given SFX device may not implement all properties.
class SFXSoundReverbProperties
{
   public:
   
      typedef void Parent;
   
      S32   mDirect;
      S32   mDirectHF;
      S32   mRoom;
      S32   mRoomHF;
      S32   mObstruction;
      F32   mObstructionLFRatio;
      S32   mOcclusion;
      F32   mOcclusionLFRatio;
      F32   mOcclusionRoomRatio;
      F32   mOcclusionDirectRatio;
      S32   mExclusion;
      F32   mExclusionLFRatio;
      S32   mOutsideVolumeHF;
      F32   mDopplerFactor;
      F32   mRolloffFactor;
      F32   mRoomRolloffFactor;
      F32   mAirAbsorptionFactor;
      S32   mFlags;
      
      SFXSoundReverbProperties()
         : mDirect( 0 ),
           mDirectHF( 0 ),
           mRoom( 0 ),
           mRoomHF( 0 ),
           mObstruction( 0 ),
           mObstructionLFRatio( 0.0f ),
           mOcclusion( 0 ),
           mOcclusionLFRatio( 0.25f ),
           mOcclusionRoomRatio( 1.5f ),
           mOcclusionDirectRatio( 1.0f ),
           mExclusion( 0 ),
           mExclusionLFRatio( 1.0f ),
           mOutsideVolumeHF( 0 ),
           mDopplerFactor( 0.0f ),
           mRolloffFactor( 0.0f ),
           mRoomRolloffFactor( 0.0f ),
           mAirAbsorptionFactor( 1.0f ),
           mFlags( 0 )
      {
      }
      
      void validate()
      {
         mDirect              = mClamp( mDirect,                -10000,  1000 );
         mDirectHF            = mClamp( mDirectHF,              -10000,  0 );
         mRoom                = mClamp( mRoom,                  -10000,  1000 );
         mRoomHF              = mClamp( mRoomHF,                -10000,  0 );
         mObstruction         = mClamp( mObstruction,           -10000,  0 );
         mObstructionLFRatio  = mClampF( mObstructionLFRatio,   0.0f,     1.0f );
         mOcclusion           = mClamp( mOcclusion,             -10000,  0 );
         mOcclusionLFRatio    = mClampF( mOcclusionLFRatio,     0.0f,     1.0f );
         mOcclusionRoomRatio  = mClampF( mOcclusionRoomRatio,   0.0f,     10.0f );
         mOcclusionDirectRatio= mClampF( mOcclusionDirectRatio, 0.0f,     10.0f );
         mExclusion           = mClamp( mExclusion,             -10000,  0 );
         mExclusionLFRatio    = mClampF( mExclusionLFRatio,     0.0f,     1.0f );
         mOutsideVolumeHF     = mClamp( mOutsideVolumeHF,       -10000,  0 );
         mDopplerFactor       = mClampF( mDopplerFactor,        0.0f,     10.0f );
         mRolloffFactor       = mClampF( mRolloffFactor,        0.0f,     10.0f );
         mRoomRolloffFactor   = mClampF( mRoomRolloffFactor,    0.0f,     10.0f );
         mAirAbsorptionFactor = mClampF( mAirAbsorptionFactor,  0.0f,     10.0f );
      }
};


//-----------------------------------------------------------------------------
//    SFXListenerProperties.
//-----------------------------------------------------------------------------


///
class SFXListenerProperties
{
   public:
   
      typedef void Parent;
      
      /// Position and orientation of the listener.
      MatrixF mTransform;
      
      ///
      Point3F mVelocity;

      SFXListenerProperties()
         : mTransform( true ),
           mVelocity( 0.0f, 0.0f, 0.0f ) {}
           
      SFXListenerProperties( const MatrixF& transform, const Point3F& velocity )
         : mTransform( transform ),
           mVelocity( velocity ) {}
           
      ///
      const MatrixF& getTransform() const { return mTransform; }
      MatrixF& getTransform() { return mTransform; }
      
      ///
      const Point3F& getVelocity() const { return mVelocity; }
      Point3F& getVelocity() { return mVelocity; }
};


//-----------------------------------------------------------------------------
//    SFXMaterialProperties.
//-----------------------------------------------------------------------------


///
class SFXMaterialProperties
{
   public:
   
      typedef void Parent;
   
      ///
      bool mDoubleSided;
   
      ///
      F32 mDirectOcclusion;
      
      ///
      F32 mReverbOcclusion;
      
      SFXMaterialProperties()
         : mDoubleSided( false ),
           mDirectOcclusion( 0.5f ),
           mReverbOcclusion( 0.5f ) {}
      
      void validate()
      {
         mDirectOcclusion = mClampF( mDirectOcclusion, 0.0f, 1.0f );
         mReverbOcclusion = mClampF( mReverbOcclusion, 0.0f, 1.0f );
      }
};


//-----------------------------------------------------------------------------
//    SFXVariantFloat.
//-----------------------------------------------------------------------------


/// An array of float values with optional random variances.
template< int NUM_VALUES >
struct SFXVariantFloat
{
   /// Base value.
   F32 mValue[ NUM_VALUES ];
      
   /// Variance of value.  Final value will be
   ///
   ///   mClampF( randF( mValue + mVariance[ 0 ], mValue + mVariance[ 1 ] ), min, max )
   ///
   /// with min and max being dependent on the context of the value.
   F32 mVariance[ NUM_VALUES ][ 2 ];
            
   F32 getValue( U32 index = 0, F32 min = TypeTraits< F32 >::MIN, F32 max = TypeTraits< F32 >::MAX ) const
   {
      AssertFatal( index < NUM_VALUES, "SFXVariantFloat::getValue() - index out of range!" );
      
      return mClampF( gRandGen.randF( mValue[ index ] + mVariance[ index ][ 0 ],
                                      mValue[ index ] + mVariance[ index ][ 1 ] ),
                      min, max );
   }
   
   void validate()
   {
      for( U32 i = 0; i < NUM_VALUES; ++ i )
         mVariance[ i ][ 0 ] = getMin( mVariance[ i ][ 0 ], mVariance[ i ][ 1 ] );
   }
};


#endif // _SFXCOMMON_H_
