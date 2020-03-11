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

#ifndef EXAMPLE_COMPONENT_H
#define EXAMPLE_COMPONENT_H
#pragma once

#ifndef COMPONENT_H
#include "T3D/components/component.h"
#endif
#ifndef RENDER_COMPONENT_INTERFACE_H
#include "T3D/components/render/renderComponentInterface.h"
#endif

class SFXSource;

//SoundComponent
//A basic example of the various functions you can utilize to make your own component!
//This example doesn't really DO anything, persay, but you can readily copy it as a base
//and use it as a starting point for your own.
class SoundComponent : public Component, public RenderComponentInterface, public EditorInspectInterface
{
   typedef Component Parent;

public:
   enum PublicConstants
   {
      MaxSoundThreads = 4,            ///< Should be a power of 2
   };

   /// @name Network state masks
   /// @{

   ///
   enum SoundComponentMasks
   {
      SoundMaskN = Parent::NextFreeMask << 6,       ///< Extends + MaxSoundThreads bits
   };

   enum BaseMaskConstants
   {
      SoundMask = (SoundMaskN << MaxSoundThreads) - SoundMaskN,
   };
   /// @name Scripted Sound
   /// @{
   struct Sound {
      bool play;                    ///< Are we playing this sound?
      SimTime timeout;              ///< Time until we stop playing this sound.
      SFXTrack* profile;            ///< Profile on server
      SFXSource* sound;             ///< Sound on client
      Sound()
      {
         play = false;
         timeout = 0;
         profile = NULL;
         sound = NULL;
      }
   };
   Sound mSoundThread[MaxSoundThreads];
   SFXTrack* mSoundFile[MaxSoundThreads];
   bool mPreviewSound[MaxSoundThreads];
   bool mPlay[MaxSoundThreads];
   /// @}

   SoundComponent();
   virtual ~SoundComponent();
   DECLARE_CONOBJECT(SoundComponent);

   virtual bool onAdd();
   virtual void onRemove();
   static void initPersistFields();
   static bool _previewSound(void *object, const char *index, const char *data);
   static bool _autoplay(void *object, const char *index, const char *data);

   virtual U32 packUpdate(NetConnection *con, U32 mask, BitStream *stream);
   virtual void unpackUpdate(NetConnection *con, BitStream *stream);

   virtual void onComponentRemove();
   virtual void onComponentAdd();

   virtual void componentAddedToOwner(Component *comp);
   virtual void componentRemovedFromOwner(Component *comp);

   virtual void onInspect();
   virtual void onEndInspect();

   virtual void processTick();
   virtual void advanceTime(F32 dt);
   virtual void interpolateTick(F32 delta);

   void prepRenderImage(SceneRenderState* state);
   void _renderObject(ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance *overrideMat);

   virtual void playAudio(U32 slotNum, SFXTrack* profile = NULL);
   virtual void stopAudio(U32 slot);
   virtual void updateServerAudio();
   virtual void updateAudioState(Sound& st);
   virtual void updateAudioPos();

   //why god why
   virtual TSShape* getShape() { return NULL; };
   Signal< void(RenderComponentInterface*) > onShapeChanged;
   virtual TSShapeInstance* getShapeInstance() { return NULL; };
   Signal< void(RenderComponentInterface*) > onShapeInstanceChanged;
   virtual MatrixF getNodeTransform(S32 nodeIdx) { return MatrixF::Identity; };
   virtual Vector<MatrixF> getNodeTransforms() { return NULL; };
   virtual void setNodeTransforms(Vector<MatrixF> transforms) {};
};

#endif