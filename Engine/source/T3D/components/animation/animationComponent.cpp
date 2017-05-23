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

#include "T3D/components/animation/animationComponent.h"
#include "T3D/components/animation/animationComponent_ScriptBinding.h"
#include "T3D/components/render/meshComponent.h"

#include "platform/platform.h"
#include "console/consoleTypes.h"
#include "core/util/safeDelete.h"
#include "core/resourceManager.h"
#include "core/stream/fileStream.h"
#include "console/consoleTypes.h"
#include "console/consoleObject.h"
#include "ts/tsShapeInstance.h"
#include "core/stream/bitStream.h"
#include "sim/netConnection.h"
#include "gfx/gfxTransformSaver.h"
#include "console/engineAPI.h"
#include "lighting/lightQuery.h"
#include "gfx/sim/debugDraw.h" 

extern bool gEditingMission;

//////////////////////////////////////////////////////////////////////////
// Callbacks
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_CALLBACK( AnimationComponent, onAnimationStart, void, ( Component* obj, const String& animName ), ( obj, animName ),
                   "@brief Called when we collide with another object.\n\n"
                   "@param obj The ShapeBase object\n"
                   "@param collObj The object we collided with\n"
                   "@param vec Collision impact vector\n"
                   "@param len Length of the impact vector\n" );

IMPLEMENT_CALLBACK(AnimationComponent, onAnimationEnd, void, (Component* obj, const char* animName), (obj, animName),
                   "@brief Called when we collide with another object.\n\n"
                   "@param obj The ShapeBase object\n"
                   "@param collObj The object we collided with\n"
                   "@param vec Collision impact vector\n"
                   "@param len Length of the impact vector\n" );

IMPLEMENT_CALLBACK(AnimationComponent, onAnimationTrigger, void, (Component* obj, const String& animName, S32 triggerID), (obj, animName, triggerID),
                   "@brief Called when we collide with another object.\n\n"
                   "@param obj The ShapeBase object\n"
                   "@param collObj The object we collided with\n"
                   "@param vec Collision impact vector\n"
                   "@param len Length of the impact vector\n" );


//////////////////////////////////////////////////////////////////////////
// Constructor/Destructor
//////////////////////////////////////////////////////////////////////////
AnimationComponent::AnimationComponent() : Component()
{
   mNetworked = true;
   mNetFlags.set(Ghostable | ScopeAlways);

   mFriendlyName = "Animation(Component)";
   mComponentType = "Render";

   mDescription = getDescriptionText("Allows a rendered mesh to be animated");

   mOwnerRenderInst = NULL;

   mOwnerShapeInstance = NULL;

   for (U32 i = 0; i < MaxScriptThreads; i++)
   {
      mAnimationThreads[i].sequence = -1;
      mAnimationThreads[i].thread = 0;
      mAnimationThreads[i].sound = 0;
      mAnimationThreads[i].state = Thread::Stop;
      mAnimationThreads[i].atEnd = false;
      mAnimationThreads[i].timescale = 1.f;
      mAnimationThreads[i].position = -1.f;
      mAnimationThreads[i].transition = true;
   }
}

AnimationComponent::~AnimationComponent()
{
   for(S32 i = 0;i < mFields.size();++i)
   {
      ComponentField &field = mFields[i];
      SAFE_DELETE_ARRAY(field.mFieldDescription);
   }

   SAFE_DELETE_ARRAY(mDescription);
}

IMPLEMENT_CO_NETOBJECT_V1(AnimationComponent);

bool AnimationComponent::onAdd()
{
   if (!Parent::onAdd())
      return false;

   //we need at least one layer
   for (U32 i = 0; i < MaxScriptThreads; i++) 
   {
      Thread& st = mAnimationThreads[i];

      if (st.sequence != -1) 
      {
         // TG: Need to see about suppressing non-cyclic sounds
         // if the sequences were activated before the object was
         // ghosted.
         // TG: Cyclic animations need to have a random pos if
         // they were started before the object was ghosted.

         // If there was something running on the old shape, the thread
         // needs to be reset. Otherwise we assume that it's been
         // initialized either by the constructor or from the server.
         bool reset = st.thread != 0;
         st.thread = 0;

         if (st.sequence != -1)
         {
            setThreadSequence(i, st.sequence, reset);
         }
      }

      if (st.thread)
         updateThread(st);
   }

   return true;
}

void AnimationComponent::onRemove()
{
   Parent::onRemove();

   mOwnerRenderInst = NULL;
}

void AnimationComponent::onComponentAdd()
{
   //test if this is a shape component!
   RenderComponentInterface *shapeInstanceInterface = mOwner->getComponent<RenderComponentInterface>();
   if (shapeInstanceInterface)
   {
      shapeInstanceInterface->onShapeInstanceChanged.notify(this, &AnimationComponent::targetShapeChanged);
      targetShapeChanged(shapeInstanceInterface);
   }
}

void AnimationComponent::componentAddedToOwner(Component *comp)
{
   if (comp->getId() == getId())
      return;

   //test if this is a shape component!
   RenderComponentInterface *shapeInstanceInterface = dynamic_cast<RenderComponentInterface*>(comp);
   if (shapeInstanceInterface)
   {
      shapeInstanceInterface->onShapeInstanceChanged.notify(this, &AnimationComponent::targetShapeChanged);
      targetShapeChanged(shapeInstanceInterface);
   }
}

void AnimationComponent::componentRemovedFromOwner(Component *comp)
{
   if (comp->getId() == getId()) //?????????
      return;

   //test if this is a shape component!
   RenderComponentInterface *shapeInstanceInterface = dynamic_cast<RenderComponentInterface*>(comp);
   if (shapeInstanceInterface)
   {
      shapeInstanceInterface->onShapeInstanceChanged.remove(this, &AnimationComponent::targetShapeChanged);
      mOwnerRenderInst = NULL;
   }
}

void AnimationComponent::targetShapeChanged(RenderComponentInterface* instanceInterface)
{
   mOwnerRenderInst = instanceInterface;

   if (!mOwnerRenderInst || !getShape())
      return;

   MeshComponent* meshComp = dynamic_cast<MeshComponent*>(mOwnerRenderInst);

   mOwnerShapeInstance = meshComp->getShapeInstance();

   if (!mOwnerShapeInstance)
      return;

   for (U32 i = 0; i < MaxScriptThreads; i++)
   {
      Thread& st = mAnimationThreads[i];

      st.thread = mOwnerShapeInstance->addThread();
   }
}

void AnimationComponent::initPersistFields()
{
   Parent::initPersistFields();
}

U32 AnimationComponent::packUpdate(NetConnection *con, U32 mask, BitStream *stream)
{
   U32 retMask = Parent::packUpdate(con, mask, stream);

   //early test if we lack an owner, ghost-wise
   //no point in trying, just re-queue the mask and go
   if (!mOwner || con->getGhostIndex(mOwner) == -1)
   {
      stream->writeFlag(false);
      return retMask |= ThreadMask;
   }
   else
   {
      stream->writeFlag(true);

      for (int i = 0; i < MaxScriptThreads; i++) 
      {
         Thread& st = mAnimationThreads[i];
         if (stream->writeFlag( (st.sequence != -1 || st.state == Thread::Destroy) && (mask & (ThreadMaskN << i)) ) ) 
         {
            stream->writeInt(st.sequence,ThreadSequenceBits);
            stream->writeInt(st.state,2);
            stream->write(st.timescale);
            stream->write(st.position);
            stream->writeFlag(st.atEnd);
            stream->writeFlag(st.transition);
         }
      }
   }

   return retMask;
}

void AnimationComponent::unpackUpdate(NetConnection *con, BitStream *stream)
{
   Parent::unpackUpdate(con, stream);

   if (stream->readFlag()) 
   {
      for (S32 i = 0; i < MaxScriptThreads; i++) 
      {
         if (stream->readFlag()) 
         {
            Thread& st = mAnimationThreads[i];
            U32 seq = stream->readInt(ThreadSequenceBits);
            st.state = stream->readInt(2);
            stream->read( &st.timescale );
            stream->read( &st.position );
            st.atEnd = stream->readFlag();
            bool transition = stream->readFlag();

            if (!st.thread || st.sequence != seq && st.state != Thread::Destroy)
               setThreadSequence(i, seq, false, transition);
            else
               updateThread(st);

         }
      }
   }
}
void AnimationComponent::processTick()
{
   Parent::processTick();

   if (!isActive())
      return;

   if (isServerObject()) 
   {
      // Server only...
      advanceThreads(TickSec);
   }
}

void AnimationComponent::advanceTime(F32 dt)
{
   Parent::advanceTime(dt);

   // On the client, the shape threads and images are
   // advanced at framerate.
   advanceThreads(dt);
}
//
const char *AnimationComponent::getThreadSequenceName(U32 slot)
{
   Thread& st = mAnimationThreads[slot];
   if (st.sequence == -1)
   {
      // Invalid Animation.
      return "";
   }

   // Name Index
   TSShape* shape = getShape();

   if (shape)
   {
      const U32 nameIndex = shape->sequences[st.sequence].nameIndex;

      // Return Name.
      return shape->getName(nameIndex);
   }

   return "";
}

bool AnimationComponent::setThreadSequence(U32 slot, S32 seq, bool reset, bool transition, F32 transTime)
{
   if (!mOwnerShapeInstance)
      return false;

   Thread& st = mAnimationThreads[slot];
   if (st.thread && st.sequence == seq && st.state == Thread::Play && !reset)
      return true;

   // Handle a -1 sequence, as this may be set when a thread has been destroyed.
   if (seq == -1)
      return true;

   if (seq < MaxSequenceIndex)
   {
      setMaskBits(-1);
      setMaskBits(ThreadMaskN << slot);
      st.sequence = seq;
      st.transition = transition;

      if (reset)
      {
         st.state = Thread::Play;
         st.atEnd = false;
         st.timescale = 1.f;
         st.position = 0.f;
      }

      if (mOwnerShapeInstance)
      {
         if (!st.thread)
            st.thread = mOwnerShapeInstance->addThread();

         if (transition)
         {
            mOwnerShapeInstance->transitionToSequence(st.thread, seq, st.position, transTime, true);
         }
         else
         {
            mOwnerShapeInstance->setSequence(st.thread, seq, 0);
            stopThreadSound(st);
         }

         updateThread(st);
      }
      return true;
   }
   return false;
}

S32 AnimationComponent::getThreadSequenceID(S32 slot)
{
   if (slot >= 0 && slot < AnimationComponent::MaxScriptThreads)
   {
      return mAnimationThreads[slot].sequence;
   }
   else
   {
      return -1;
   }
}

void AnimationComponent::updateThread(Thread& st)
{
   if (!mOwnerRenderInst)
      return;

   switch (st.state)
   {
      case Thread::Stop:
      {
         mOwnerShapeInstance->setTimeScale(st.thread, 1.f);
         mOwnerShapeInstance->setPos(st.thread, (st.timescale > 0.f) ? 0.0f : 1.0f);
      } // Drop through to pause state

      case Thread::Pause:
      {
         if (st.position != -1.f)
         {
            mOwnerShapeInstance->setTimeScale(st.thread, 1.f);
            mOwnerShapeInstance->setPos(st.thread, st.position);
         }

         mOwnerShapeInstance->setTimeScale(st.thread, 0.f);
         stopThreadSound(st);
      } break;

      case Thread::Play:
      {
         if (st.atEnd)
         {
            mOwnerShapeInstance->setTimeScale(st.thread, 1);
            mOwnerShapeInstance->setPos(st.thread, (st.timescale > 0.f) ? 1.0f : 0.0f);
            mOwnerShapeInstance->setTimeScale(st.thread, 0);
            stopThreadSound(st);
            st.state = Thread::Stop;
         }
         else
         {
            if (st.position != -1.f)
            {
               mOwnerShapeInstance->setTimeScale(st.thread, 1.f);
               mOwnerShapeInstance->setPos(st.thread, st.position);
            }

            mOwnerShapeInstance->setTimeScale(st.thread, st.timescale);
            if (!st.sound)
            {
               startSequenceSound(st);
            }
         }
      } break;

      case Thread::Destroy:
      {
         stopThreadSound(st);
         st.atEnd = true;
         st.sequence = -1;
         if (st.thread)
         {
            mOwnerShapeInstance->destroyThread(st.thread);
            st.thread = 0;
         }
      } break;
   }
}

bool AnimationComponent::stopThread(U32 slot)
{
   Thread& st = mAnimationThreads[slot];
   if (st.sequence != -1 && st.state != Thread::Stop) 
   {
      setMaskBits(ThreadMaskN << slot);
      st.state = Thread::Stop;
      updateThread(st);
      return true;
   }
   return false;
}

bool AnimationComponent::destroyThread(U32 slot)
{
   Thread& st = mAnimationThreads[slot];
   if (st.sequence != -1 && st.state != Thread::Destroy) 
   {
      setMaskBits(ThreadMaskN << slot);
      st.state = Thread::Destroy;
      updateThread(st);
      return true;
   }
   return false;
}

bool AnimationComponent::pauseThread(U32 slot)
{
   Thread& st = mAnimationThreads[slot];
   if (st.sequence != -1 && st.state != Thread::Pause) 
   {
      setMaskBits(ThreadMaskN << slot);
      st.state = Thread::Pause;
      updateThread(st);
      return true;
   }
   return false;
}

bool AnimationComponent::playThread(U32 slot)
{
   Thread& st = mAnimationThreads[slot];
   if (st.sequence != -1 && st.state != Thread::Play)
   {
      setMaskBits(ThreadMaskN << slot);
      st.state = Thread::Play;
      updateThread(st);
      return true;
   }
   return false;
}

bool AnimationComponent::playThread(U32 slot, const char* name, bool transition, F32 transitionTime)
{
   if (slot < AnimationComponent::MaxScriptThreads)
   {
      if (!dStrEqual(name, ""))
      {
         if (TSShape* shape = getShape())
         {
            S32 seq = shape->findSequence(name);
            if (seq != -1 && setThreadSequence(slot, seq, true, transition, transitionTime))
            {
               return true;
            }
            else if (seq == -1)
            {
               //We tried to play a non-existaint sequence, so stop the thread just in case
               destroyThread(slot);
               return false;
            }
         }
      }
      else
      {
         if (playThread(slot))
            return true;
      }
   }

   return false;
}

bool AnimationComponent::setThreadAnimation(U32 slot, const char* name)
{
   if (slot < AnimationComponent::MaxScriptThreads)
   {
      if (!dStrEqual(name, ""))
      {
         if (TSShape* shape = getShape())
         {
            S32 seq = shape->findSequence(name);
            if (seq != -1 && setThreadSequence(slot, seq, false, false))
            {
               Thread& st = mAnimationThreads[slot];
               if (st.position == -1)
                  st.position = 0;
               //st.state = Thread::Pause;
               return true;
            }
            else if (seq == -1)
            {
               //We tried to play a non-existaint sequence, so stop the thread just in case
               destroyThread(slot);
               return false;
            }
         }
      }
      else
      {
         if (playThread(slot))
            return true;
      }
   }

   return false;
}

bool AnimationComponent::setThreadPosition(U32 slot, F32 pos)
{
   Thread& st = mAnimationThreads[slot];
   if (st.sequence != -1)
   {
      setMaskBits(ThreadMaskN << slot);
      st.position = pos;
      st.atEnd = false;
      updateThread(st);

      return true;
   }
   return false;
}

bool AnimationComponent::setThreadDir(U32 slot, bool forward)
{
   Thread& st = mAnimationThreads[slot];
   if (st.sequence != -1)
   {
      if ((st.timescale >= 0.f) != forward)
      {
         setMaskBits(ThreadMaskN << slot);
         st.timescale *= -1.f;
         st.atEnd = false;
         updateThread(st);
      }
      return true;
   }
   return false;
}

bool AnimationComponent::setThreadTimeScale(U32 slot, F32 timeScale)
{
   Thread& st = mAnimationThreads[slot];
   if (st.sequence != -1)
   {
      if (st.timescale != timeScale)
      {
         setMaskBits(ThreadMaskN << slot);
         st.timescale = timeScale;
         updateThread(st);
      }
      return true;
   }
   return false;
}

void AnimationComponent::stopThreadSound(Thread& thread)
{
   return;
}

void AnimationComponent::startSequenceSound(Thread& thread)
{
   return;
}

void AnimationComponent::advanceThreads(F32 dt)
{
   if (!mOwnerRenderInst)
      return;

   for (U32 i = 0; i < MaxScriptThreads; i++)
   {
      Thread& st = mAnimationThreads[i];
      if (st.thread && st.sequence != -1)
      {
         bool cyclic = getShape()->sequences[st.sequence].isCyclic();

         if (!getShape()->sequences[st.sequence].isCyclic() &&
            !st.atEnd &&
            ((st.timescale > 0.f) ? mOwnerShapeInstance->getPos(st.thread) >= 1.0 : mOwnerShapeInstance->getPos(st.thread) <= 0))
         {
            st.atEnd = true;
            updateThread(st);

            if (!isGhost())
            {
               Con::executef(this, "onAnimationEnd", st.thread->getSequenceName());
            }
         }

         // Make sure the thread is still valid after the call to onEndSequence_callback().
         // Someone could have called destroyThread() while in there.
         if (st.thread)
         {
            mOwnerShapeInstance->advanceTime(dt, st.thread);
         }

         if (mOwnerShapeInstance && !isGhost())
         {
            for (U32 i = 1; i < 32; i++)
            {
               if (mOwnerShapeInstance->getTriggerState(i))
               {
                  const char* animName = st.thread->getSequenceName().c_str();
                  onAnimationTrigger_callback(this, animName, i);
               }
            }
         }

         if (isGhost())
            mOwnerShapeInstance->animate();
      }
   }
}

TSShape* AnimationComponent::getShape()
{
   if (mOwner == NULL)
      return NULL;

   if (mOwnerRenderInst == NULL)
      return NULL;

   return mOwnerRenderInst->getShape();
}

S32 AnimationComponent::getAnimationCount()
{
   if (getShape())
      return getShape()->sequences.size();
   else
      return 0;
}

S32 AnimationComponent::getAnimationIndex(const char* name)
{
   if (getShape())
      return getShape()->findSequence(name);
   else
      return -1;
}

const char* AnimationComponent::getAnimationName(S32 index)
{
   if (getShape())
   {
      if (index >= 0 && index < getShape()->sequences.size())
         return getShape()->getName(getShape()->sequences[index].nameIndex);
   }

   return "";
}