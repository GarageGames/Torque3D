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

#ifndef _SFXEMITTER_H_
#define _SFXEMITTER_H_

#ifndef _SCENEOBJECT_H_
   #include "scene/sceneObject.h"
#endif
#ifndef _SFXPROFILE_H_
   #include "sfx/sfxProfile.h"
#endif
#ifndef _SFXDESCRIPTION_H_
   #include "sfx/sfxDescription.h"
#endif
#ifndef _GFXSTATEBLOCK_H_
   #include "gfx/gfxStateBlock.h"
#endif


class SFXSource;
class SFXTrack;

//RDTODO: make 3D sound emitters yield their source when being culled

/// The SFXEmitter is used to place 2D or 3D sounds into a 
/// mission.
///
/// If the profile is set then the emitter plays that.  If the
/// profile is null and the filename is set then the local emitter
/// options are used.
///
/// Note that you can call SFXEmitter.play() and SFXEmitter.stop()
/// to control playback from script.
///
class SFXEmitter : public SceneObject
{
   public:
   
      typedef SceneObject Parent;

   protected:

      /// Network update masks.
      enum UpdateMasks 
      {
         InitialUpdateMask    = BIT(0),
         TransformUpdateMask  = BIT(1),
         DirtyUpdateMask      = BIT(2),

         SourcePlayMask       = BIT(3),
         SourceStopMask       = BIT(4),

         AllSourceMasks = SourcePlayMask | SourceStopMask,
      };

      /// Dirty flags used to handle sound property
      /// updates locally and across the network.
      enum Dirty
      {
         Track                      = BIT(  0 ),
         Filename                   = BIT(  2 ),
         Volume                     = BIT(  4 ),
         IsLooping                  = BIT(  5 ),
         Is3D                       = BIT(  6 ),
         MinDistance                = BIT(  7 ),
         MaxDistance                = BIT(  8 ),
         ConeInsideAngle            = BIT(  9 ),
         ConeOutsideAngle           = BIT( 10 ),
         ConeOutsideVolume          = BIT( 11 ),
         Transform                  = BIT( 12 ),
         SourceGroup                = BIT( 13 ),
         OutsideAmbient             = BIT( 14 ),
         IsStreaming                = BIT( 15 ),
         FadeInTime                 = BIT( 16 ),
         FadeOutTime                = BIT( 17 ),
         Pitch                      = BIT( 18 ),
         ScatterDistance            = BIT( 19 ),
         TrackOnly                  = BIT( 20 ),

         AllDirtyMask               = 0xFFFFFFFF,
      };

      /// The current dirty flags.
      BitSet32 mDirty;

      /// The sound source for the emitter.
      SFXSource *mSource;

      /// The selected track or null if the local
      /// profile should be used.
      SFXTrack *mTrack;
      
      /// Whether to leave sound setup exclusively to the assigned mTrack and not
      /// override part of the track's description with emitter properties.
      bool mUseTrackDescriptionOnly;

      /// A local profile object used to coax the
      /// sound system to play a custom sound.
      SFXProfile mLocalProfile;

      /// The description used by the local profile.
      SFXDescription mDescription;

      /// If true playback starts when the emitter
      /// is added to the scene.
      bool mPlayOnAdd;
      
      /// State block for cone rendering in editor.
      GFXStateBlockRef mRenderSB;
      
      /// If true, render all emitters when in editor (not only selected one).
      static bool smRenderEmitters;
      
      /// Point size for rendering point clouds of emitter cones in editor.
      /// @todo Currently not implemented.
      static F32 smRenderPointSize;
      
      ///
      static F32 smRenderRadialIncrements;
      
      ///
      static F32 smRenderSweepIncrements;
      
      ///
      static F32 smRenderPointDistance;
      
      /// Point color when emitter is playing and in range of listener.
      static ColorI smRenderColorPlayingInRange;
      
      /// Point color when emitter is playing but out of range of listern.
      static ColorI smRenderColorPlayingOutOfRange;
      
      /// Point color when emitter is not playing but in range of listener.
      static ColorI smRenderColorStoppedInRange;
      
      /// Point color when emitter is not playing and not in range of listener.
      static ColorI smRenderColorStoppedOutOfRange;
      
      ///
      static ColorI smRenderColorInnerCone;
      
      ///
      static ColorI smRenderColorOuterCone;
      
      ///
      static ColorI smRenderColorOutsideVolume;
      
      ///
      static ColorI smRenderColorRangeSphere;

      /// Helper which reads a flag from the stream and 
      /// updates the mDirty bits.
      bool _readDirtyFlag( BitStream *stream, U32 flag );

      /// Called when the emitter state has been marked
      /// dirty and the source needs to be updated.
      void _update();
      
      /// Render emitter object in editor.
      void _renderObject( ObjectRenderInst* ri, SceneRenderState* state, BaseMatInstance* overrideMat );
      
      /// Render visual feedback for 3D sounds in editor.
      void _render3DVisualFeedback();
      
      ///
      void _renderCone( F32 radialIncrements,
                        F32 sweepIncrements,
                        F32 pointDistance,
                        F32 startAngle,
                        F32 stopAngle,
                        F32 startVolume,
                        F32 stopVolume,
                        const ColorI& color );

      /// Return the playback status of the emitter's associated sound.
      /// This should only be called on either the ghost or the server object if the server is running
      /// in-process.  Otherwise, the method will not return a meaningful value.
      SFXStatus _getPlaybackStatus() const;

   public:

      SFXEmitter();
      virtual ~SFXEmitter();
      
      /// Return the sound source object associated with the emitter.
      /// @note This will only return a meaningful result when called on ghost objects.
      SFXSource* getSource() const { return mSource; }
            
      /// Return true if this object emits a 3D sound.
      bool is3D() const;
            
      /// Return true if the SFX system's listener is in range of this emitter.
      bool isInRange() const;
      
      /// Sends network event to start playback if 
      /// the emitter source is not already playing.
      void play();

      /// Sends network event to stop emitter 
      /// playback on all ghosted clients.
      void stop();

      // SimObject
      bool onAdd();
      void onRemove();
      void onStaticModified( const char *slotName, const char *newValue = NULL );
      U32 packUpdate( NetConnection *conn, U32 mask, BitStream *stream );
      void unpackUpdate( NetConnection *conn, BitStream *stream );
      void setTransform( const MatrixF &mat );
      void setScale( const VectorF &scale );
      bool containsPoint( const Point3F& point ) { return false; }
      void prepRenderImage( SceneRenderState* state );
      void inspectPostApply();

      static void initPersistFields();
      static void consoleInit();

      DECLARE_CONOBJECT( SFXEmitter );
      DECLARE_DESCRIPTION( "A 3D object emitting sound." );
      DECLARE_CATEGORY( "3D Sound" );
};

#endif // _SFXEMITTER_H_
