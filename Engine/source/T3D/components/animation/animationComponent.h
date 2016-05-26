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

#ifndef ANIMATION_COMPONENT_H
#define ANIMATION_COMPONENT_H

#ifndef COMPONENT_H
#include "T3D/components/component.h"
#endif
#ifndef _TSSHAPE_H_
#include "ts/tsShapeInstance.h"
#endif
#ifndef ENTITY_H
#include "T3D/entity.h"
#endif
#ifndef RENDER_COMPONENT_INTERFACE_H
#include "T3D/components/render/renderComponentInterface.h"
#endif

class SceneRenderState;

class AnimationComponent : public Component
{
   typedef Component Parent;
public:
   enum PublicConstants {
      ThreadSequenceBits = 6,
      MaxSequenceIndex = (1 << ThreadSequenceBits) - 1,
      MaxScriptThreads = 16,            ///< Should be a power of 2
   };

   enum MaskBits {
      ThreadMaskN = Parent::NextFreeMask << 0,
      ThreadMask = (ThreadMaskN << MaxScriptThreads) - ThreadMaskN,
      NextFreeMask = ThreadMaskN << MaxScriptThreads
   };

protected:

   struct Thread 
   {
      /// State of the animation thread.
      enum State 
      {
         Play, Stop, Pause, Destroy
      };
      TSThread* thread; ///< Pointer to 3space data.
      U32 state;        ///< State of the thread
      ///
      ///  @see Thread::State
      S32 sequence;     ///< The animation sequence which is running in this thread.
      F32 timescale;    ///< Timescale
      U32 sound;        ///< Handle to sound.
      bool atEnd;       ///< Are we at the end of this thread?
      F32 position;
      bool transition;
   };

   Thread mAnimationThreads[MaxScriptThreads];

protected:
   RenderComponentInterface * mOwnerRenderInst;

   TSShapeInstance *mOwnerShapeInstance;

public:
   AnimationComponent();
   virtual ~AnimationComponent();
   DECLARE_CONOBJECT(AnimationComponent);

   virtual bool onAdd();
   virtual void onRemove();
   static void initPersistFields();

   virtual void onComponentAdd();

   virtual void componentAddedToOwner(Component *comp);
   virtual void componentRemovedFromOwner(Component *comp);

   virtual U32 packUpdate(NetConnection *con, U32 mask, BitStream *stream);
   virtual void unpackUpdate(NetConnection *con, BitStream *stream);

   TSShape* getShape();

   void targetShapeChanged(RenderComponentInterface* instanceInterface);

   virtual void processTick();
   virtual void advanceTime(F32 dt);

   const char *getThreadSequenceName(U32 slot);
   bool setThreadSequence(U32 slot, S32 seq, bool reset = true, bool transition = true, F32 transitionTime = 0.5);
   void updateThread(Thread& st);
   bool stopThread(U32 slot);
   bool destroyThread(U32 slot);
   bool pauseThread(U32 slot);
   bool playThread(U32 slot);
   bool playThread(U32 slot, const char* name, bool transition, F32 transitionTime);
   bool setThreadAnimation(U32 slot, const char* name);
   bool setThreadPosition(U32 slot, F32 pos);
   bool setThreadDir(U32 slot, bool forward);
   bool setThreadTimeScale(U32 slot, F32 timeScale);
   void stopThreadSound(Thread& thread);
   void startSequenceSound(Thread& thread);
   void advanceThreads(F32 dt);

   S32 getThreadSequenceID(S32 slot);

   //other helper functions
   S32 getAnimationCount();
   S32 getAnimationIndex(const char* name);
   const char* getAnimationName(S32 index);

   //callbacks
   DECLARE_CALLBACK(void, onAnimationStart, (Component* obj, const String& animName));
   DECLARE_CALLBACK(void, onAnimationEnd, (Component* obj, const char* animName));
   DECLARE_CALLBACK(void, onAnimationTrigger, (Component* obj, const String& animName, S32 triggerID));
};

#endif //_ANIMATION_COMPONENT_H