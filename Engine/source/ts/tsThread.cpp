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
#include "ts/tsShapeInstance.h"


//-------------------------------------------------------------------------------------
// This file contains the shape instance thread class (defined in tsShapeInstance.h)
// and the tsShapeInstance functions to interface with the thread class.
//-------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------
// Thread class
//-------------------------------------------------------------------------------------

// given a position on the thread, choose correct keyframes
// slight difference between one-shot and cyclic sequences -- see comments below for details
void TSThread::selectKeyframes(F32 pos, const TSSequence * seq, S32 * k1, S32 * k2, F32 * kpos)
{
   S32 numKF = seq->numKeyframes;
   F32 kf;

   if (seq->isCyclic())
   {
      // cyclic sequence:
      // pos=0 and pos=1 are equivalent, so we don't have a keyframe at pos=1
      // last keyframe corresponds to pos=n/(n-1) up to (not including) pos=1
      // (where n == num keyframes)

      AssertFatal(pos>=0.0f && pos<1.0f,"TSThread::selectKeyframes");

      kf = pos * (F32) (numKF);

      // set keyPos
      if (kpos)
         *kpos = kf - (S32) kf;

      // make sure compiler doing what we want...
      AssertFatal(*kpos>=0.0f && *kpos<1.0f,"TSThread::selectKeyframes");

      S32 kfIdx1 = (S32) kf;

      // following assert could happen if pos1<1 && pos1==1...paradoxically...
      AssertFatal(kfIdx1<=seq->numKeyframes,"TSThread::selectKeyframes");

      S32 kfIdx2 = (kfIdx1==seq->numKeyframes-1) ? 0 : kfIdx1+1;

      if (k1)
         *k1 = kfIdx1;
      if (k2)
         *k2 = kfIdx2;
   }
   else
   {
      // one-shot sequence:
      // pos=0 and pos=1 are now different, so we have a keyframe at pos=1
      // last keyframe corresponds to pos=1
      // rest of the keyframes are equally spaced (so 1/(n-1) pos units long)
      // (where n == num keyframes)

      AssertFatal(pos>=0.0f && pos<=1.0f,"TSThread::selectKeyframes");

      if (pos==1.0f)
      {
         if (kpos)
            *kpos = 0.0f;
         if (k1)
            *k1 = seq->numKeyframes-1;
         if (k2)
            *k2 = seq->numKeyframes-1;
      }
      else
      {
         kf = pos * (F32) (numKF-1);

         // set keyPos
         if (kpos)
            *kpos = kf - (S32) kf;

         S32 kfIdx1 = (S32) kf;

         // following assert could happen if pos1<1 && pos1==1...paradoxically...
         AssertFatal(kfIdx1<seq->numKeyframes,"TSThread::selectKeyFrames: invalid keyframe!");

         S32 kfIdx2 = kfIdx1+1;

         if (k1)
            *k1 = kfIdx1;
         if (k2)
            *k2 = kfIdx2;
      }
   }
}

void TSThread::getGround(F32 t, MatrixF * pMat)
{
   static const QuatF unitRotation(0,0,0,1);
   static const Point3F unitTranslation = Point3F::Zero;

   const QuatF * q1, * q2;
   QuatF rot1,rot2;
   const Point3F * p1, * p2;

   // if N = sequence->numGroundFrames, then there are N+1 positions we
   // interpolate betweeen:  0/N, 1/N ... N/N
   // we need to convert the p passed to us into 2 ground keyframes:

   // the 0.99999f is in case 'p' is exactly 1.0f, which is legal but 'kf'
   // needs to be strictly less than 'sequence->numGroundFrames'
   F32 kf = 0.999999f * t * (F32) getSequence()->numGroundFrames;

   // get frame number and interp param (kpos)
   S32 frame = (S32)kf;
   F32 kpos = kf - (F32)frame;

   // now point pT1 and pT2 at transforms for keyframes 'frame' and 'frame+1'

   // following a little strange:  first ground keyframe (0/N in comment above) is
   // assumed to be ident. and not found in the list.
   if (frame)
   {
      p1 = &mShapeInstance->mShape->mGroundTranslations[getSequence()->firstGroundFrame + frame - 1];
      q1 = &mShapeInstance->mShape->mGroundRotations[getSequence()->firstGroundFrame + frame - 1].getQuatF(&rot1);
   }
   else
   {
      p1 = &unitTranslation;
      q1 = &unitRotation;
   }

   // similar to above, ground keyframe number 'frame+1' is actually offset by 'frame'
   p2 = &mShapeInstance->mShape->mGroundTranslations[getSequence()->firstGroundFrame + frame];
   q2 = &mShapeInstance->mShape->mGroundRotations[getSequence()->firstGroundFrame + frame].getQuatF(&rot2);

   QuatF q;
   Point3F p;
   TSTransform::interpolate(*q1,*q2,kpos,&q);
   TSTransform::interpolate(*p1,*p2,kpos,&p);
   TSTransform::setMatrix(q,p,pMat);
}

void TSThread::setSequence(S32 seq, F32 toPos)
{
   const TSShape * shape = mShapeInstance->mShape;

   AssertFatal(shape && shape->mSequences.size()>seq && toPos>=0.0f && toPos<=1.0f,
      "TSThread::setSequence: invalid shape handle, sequence number, or position.");

   mShapeInstance->clearTransition(this);

   mSequence = seq;
   mPriority = getSequence()->priority;
   mPos = toPos;
   mMakePath = getSequence()->makePath();
   mPath.start = mPath.end = 0;
   mPath.loop = 0;

   // 1.0f doesn't exist on cyclic sequences
   if (mPos>0.9999f && getSequence()->isCyclic())
      mPos = 0.9999f;

   // select keyframes
   selectKeyframes(mPos,getSequence(),&mKeyNum1,&mKeyNum2,&mKeyPos);
}

void TSThread::transitionToSequence(S32 seq, F32 toPos, F32 duration, bool continuePlay)
{
   AssertFatal(duration>=0.0f,"TSThread::transitionToSequence: negative duration not allowed");

   // make sure these nodes are smoothly interpolated to new positions...
   // basically, any node we controlled just prior to transition, or at any stage
   // of the transition is interpolated.  If we start to transtion from A to B,
   // but before reaching B we transtion to C, we interpolate all nodes controlled
   // by A, B, or C to their new position.
   if (mTransitionData.inTransition)
   {
      mTransitionData.oldRotationNodes.overlap(getSequence()->rotationMatters);
      mTransitionData.oldTranslationNodes.overlap(getSequence()->translationMatters);
      mTransitionData.oldScaleNodes.overlap(getSequence()->scaleMatters);
   }
   else
   {
      mTransitionData.oldRotationNodes = getSequence()->rotationMatters;
      mTransitionData.oldTranslationNodes = getSequence()->translationMatters;
      mTransitionData.oldScaleNodes = getSequence()->scaleMatters;
   }

   // set time characteristics of transition
   mTransitionData.oldSequence = mSequence;
   mTransitionData.oldPos = mPos;
   mTransitionData.duration = duration;
   mTransitionData.pos = 0.0f;
   mTransitionData.direction = mTimeScale>0.0f ? 1.0f : -1.0f;
   mTransitionData.targetScale = continuePlay ? 1.0f : 0.0f;

   // in transition...
   mTransitionData.inTransition = true;

   // set target sequence data
   mSequence = seq;
   mPriority = getSequence()->priority;
   mPos = toPos;
   mMakePath = getSequence()->makePath();
   mPath.start = mPath.end = 0;
   mPath.loop = 0;

   // 1.0f doesn't exist on cyclic sequences
   if (mPos>0.9999f && getSequence()->isCyclic())
      mPos = 0.9999f;

   // select keyframes
   selectKeyframes(mPos,getSequence(),&mKeyNum1,&mKeyNum2,&mKeyPos);
}

bool TSThread::isInTransition()
{
   return mTransitionData.inTransition;
}

void TSThread::animateTriggers()
{
   if (!getSequence()->numTriggers)
      return;

   switch (mPath.loop)
   {
      case -1 :
         activateTriggers(mPath.start,0);
         activateTriggers(1,mPath.end);
         break;
      case  0 :
         activateTriggers(mPath.start,mPath.end);
         break;
      case  1 :
         activateTriggers(mPath.start,1);
         activateTriggers(0,mPath.end);
         break;
      default:
      {
         if (mPath.loop>0)
         {
            activateTriggers(mPath.end,1);
            activateTriggers(0,mPath.end);
         }
         else
         {
            activateTriggers(mPath.end,0);
            activateTriggers(1,mPath.end);
         }
      }
   }
}

void TSThread::activateTriggers(F32 a, F32 b)
{
   S32 i;
   const TSShape * shape = mShapeInstance->mShape;
   S32 firstTrigger = getSequence()->firstTrigger;
   S32 numTriggers = getSequence()->numTriggers;

   // first find triggers at position a and b
   // we assume there aren't many triggers, so
   // search is linear
   F32 lastPos = -1.0f;
   S32 aIndex  = numTriggers+firstTrigger; // initialized to handle case where pos past all triggers
   S32 bIndex  = numTriggers+firstTrigger; // initialized to handle case where pos past all triggers
   for (i=firstTrigger; i<numTriggers+firstTrigger; i++)
   {
      // is a between this trigger and previous one...
      if (a>lastPos && a<=shape->mTriggers[i].pos)
         aIndex = i;
      // is b between this trigger and previous one...
      if (b>lastPos && b<=shape->mTriggers[i].pos)
         bIndex = i;
      lastPos = shape->mTriggers[i].pos;
   }

   // activate triggers between aIndex and bIndex (depends on direction)
   if (aIndex<=bIndex)
   {
      for (i=aIndex; i<bIndex; i++)
      {
         U32 state = shape->mTriggers[i].state;
         bool on = (state & TSShape::Trigger::StateOn)!=0;
         mShapeInstance->setTriggerStateBit(state & TSShape::Trigger::StateMask, on);
      }
   }
   else
   {
      for (i=aIndex-1; i>=bIndex; i--)
      {
         U32 state = shape->mTriggers[i].state;
         bool on = (state & TSShape::Trigger::StateOn)!=0;
         if (state & TSShape::Trigger::InvertOnReverse)
            on = !on;
         mShapeInstance->setTriggerStateBit(state & TSShape::Trigger::StateMask, on);
      }
   }
}

F32 TSThread::getPos()
{
   return mTransitionData.inTransition ? mTransitionData.pos : mPos;
}

F32 TSThread::getTime()
{
   return mTransitionData.inTransition ? mTransitionData.pos * mTransitionData.duration : mPos * getSequence()->duration;
}

F32 TSThread::getDuration()
{
   return mTransitionData.inTransition ? mTransitionData.duration : getSequence()->duration;
}

F32 TSThread::getScaledDuration()
{
   return getDuration() / mFabs(mTimeScale);
}

F32 TSThread::getTimeScale()
{
   return mTimeScale;
}

void TSThread::setTimeScale(F32 ts)
{
   mTimeScale = ts;
}

void TSThread::advancePos(F32 delta)
{
   if (mFabs(delta)>0.00001f)
   {
      // make dirty what this thread changes
      U32 dirtyFlags = getSequence()->dirtyFlags | (mTransitionData.inTransition ? TSShapeInstance::TransformDirty : 0);
      for (S32 i=0; i<mShapeInstance->getShape()->mSubShapeFirstNode.size(); i++)
         mShapeInstance->mDirtyFlags[i] |= dirtyFlags;
   }

   if (mTransitionData.inTransition)
   {
      mTransitionData.pos += mTransitionData.direction * delta;
      if (mTransitionData.pos<0 || mTransitionData.pos>=1.0f)
      {
         mShapeInstance->clearTransition(this);
         if (mTransitionData.pos<0.0f)
            // return to old sequence
            mShapeInstance->setSequence(this,mTransitionData.oldSequence,mTransitionData.oldPos);
      }
      // re-adjust delta to be correct time-wise
      delta *= mTransitionData.targetScale * mTransitionData.duration / getSequence()->duration;
   }

   // even if we are in a transition, keep playing the sequence

   if (mMakePath)
   {
      mPath.start = mPos;
      mPos += delta;
      if (!getSequence()->isCyclic())
      {
         mPos = mClampF(mPos , 0.0f, 1.0f);
         mPath.loop = 0;
      }
      else
      {
         mPath.loop = (S32)mPos;
         if (mPos < 0.0f)
            mPath.loop--;
         mPos -= mPath.loop;
         // following necessary because of floating point roundoff errors
         if (mPos < 0.0f) mPos += 1.0f;
         if (mPos >= 1.0f) mPos -= 1.0f;
      }
      mPath.end = mPos;

      animateTriggers(); // do this automatically...no need for user to call it

      AssertFatal(mPos>=0.0f && mPos<=1.0f,"TSThread::advancePos (1)");
      AssertFatal(!getSequence()->isCyclic() || mPos<1.0f,"TSThread::advancePos (2)");
   }
   else
   {
      mPos += delta;
      if (!getSequence()->isCyclic())
         mPos = mClampF(mPos, 0.0f, 1.0f);
      else
      {
         mPos -= S32(mPos);
         // following necessary because of floating point roundoff errors
         if (mPos < 0.0f) mPos += 1.0f;
         if (mPos >= 1.0f) mPos -= 1.0f;
      }
      AssertFatal(mPos>=0.0f && mPos<=1.0f,"TSThread::advancePos (3)");
      AssertFatal(!getSequence()->isCyclic() || mPos<1.0f,"TSThread::advancePos (4)");
   }

   // select keyframes
   selectKeyframes(mPos,getSequence(),&mKeyNum1,&mKeyNum2,&mKeyPos);
}

void TSThread::advanceTime(F32 delta)
{
   advancePos(mTimeScale * delta / getDuration());
}

void TSThread::setPos(F32 pos)
{
   advancePos(pos-getPos());
}

void TSThread::setTime(F32 time)
{
   setPos(mTimeScale * time/getDuration());
}

S32 TSThread::getKeyframeCount()
{
   AssertFatal(!mTransitionData.inTransition,"TSThread::getKeyframeCount: not while in transition");

   return getSequence()->numKeyframes + 1;
}

S32 TSThread::getKeyframeNumber()
{
   AssertFatal(!mTransitionData.inTransition,"TSThread::getKeyframeNumber: not while in transition");

   return mKeyNum1;
}

void TSThread::setKeyframeNumber(S32 kf)
{
   AssertFatal(kf>=0 && kf<= getSequence()->numKeyframes,
      "TSThread::setKeyframeNumber: invalid frame specified.");
   AssertFatal(!mTransitionData.inTransition,"TSThread::setKeyframeNumber: not while in transition");

   mKeyNum1 = mKeyNum2 = kf;
   mKeyPos = 0;
   mPos = 0;
}

TSThread::TSThread(TSShapeInstance * _shapeInst)
{
   mTimeScale = 1.0f;
   mShapeInstance = _shapeInst;
   mTransitionData.inTransition = false;
   mBlendDisabled = false;
   setSequence(0,0.0f);
}

S32 TSThread::operator<(const TSThread & th2) const
{
   if (getSequence()->isBlend() == th2.getSequence()->isBlend())
   {
      // both blend or neither blend, sort based on priority only -- higher priority first
      S32 ret = 0; // do it this way to (hopefully) take advantage of 'conditional move' assembly instruction
      if (mPriority > th2.mPriority)
         ret = -1;
      if (th2.mPriority > mPriority)
         ret = 1;
      return ret;
   }
   else
   {
      // one is blend, the other is not...sort based on blend -- non-blended first
      AssertFatal(!getSequence()->isBlend() || !th2.getSequence()->isBlend(),"compareThreads: unequal 'trues'");

      S32 ret = -1; // do it this way to (hopefully) take advantage of 'conditional move' assembly instruction
      if (getSequence()->isBlend())
         ret = 1;
      return ret;
   }
}


//-------------------------------------------------------------------------------------
// TSShapeInstance Thread Interface -- more implemented in header file
//-------------------------------------------------------------------------------------

TSThread * TSShapeInstance::addThread()
{
   if (mShape->mSequences.empty())
      return NULL;

   mThreadList.increment();
   mThreadList.last() = new TSThread(this);
   setDirty(AllDirtyMask);
   return mThreadList.last();
}

TSThread * TSShapeInstance::getThread(S32 threadNumber)
{
   AssertFatal(threadNumber < mThreadList.size() && threadNumber>=0,"TSShapeInstance::getThread: threadNumber out of bounds.");
   return mThreadList[threadNumber];
}

void TSShapeInstance::destroyThread(TSThread * thread)
{
   if (!thread)
      return;

   clearTransition(thread);

   S32 i;
   for (i=0; i<mThreadList.size(); i++)
      if (thread==mThreadList[i])
         break;

   AssertFatal(i<mThreadList.size(),"TSShapeInstance::destroyThread was requested to destroy a thread that this instance doesn't own!");

   delete mThreadList[i];
   mThreadList.erase(i);
   setDirty(AllDirtyMask);
   checkScaleCurrentlyAnimated();
}

U32 TSShapeInstance::threadCount()
{
   return mThreadList.size();
}

void TSShapeInstance::setSequence(TSThread * thread, S32 seq, F32 pos)
{
   if ( (thread->mTransitionData.inTransition && mTransitionThreads.size()>1) || mTransitionThreads.size()>0)
   {
      // if we have transitions, make sure transforms are up to date...
      sortThreads();
      animateNodeSubtrees();
   }

   thread->setSequence(seq,pos);
   setDirty(AllDirtyMask);
   mGroundThread = NULL;

   if (mScaleCurrentlyAnimated && !thread->getSequence()->animatesScale())
      checkScaleCurrentlyAnimated();
   else if (!mScaleCurrentlyAnimated && thread->getSequence()->animatesScale())
      mScaleCurrentlyAnimated=true;

   updateTransitions();
}

U32 TSShapeInstance::getSequence(TSThread * thread)
{
   //AssertFatal( thread->sequence >= 0, "TSShapeInstance::getSequence: range error A");
   //AssertFatal( thread->sequence < mShape->sequences.size(), "TSShapeInstance::getSequence: range error B");
   return (U32)thread->mSequence;
}

void TSShapeInstance::transitionToSequence(TSThread * thread, S32 seq, F32 pos, F32 duration, bool continuePlay)
{
   // make sure all transforms on all detail levels are accurate
   sortThreads();
   animateNodeSubtrees();

   thread->transitionToSequence(seq,pos,duration,continuePlay);
   setDirty(AllDirtyMask);
   mGroundThread = NULL;

   if (mScaleCurrentlyAnimated && !thread->getSequence()->animatesScale())
      checkScaleCurrentlyAnimated();
   else if (!mScaleCurrentlyAnimated && thread->getSequence()->animatesScale())
      mScaleCurrentlyAnimated=true;

   mTransitionRotationNodes.overlap(thread->mTransitionData.oldRotationNodes);
   mTransitionRotationNodes.overlap(thread->getSequence()->rotationMatters);

   mTransitionTranslationNodes.overlap(thread->mTransitionData.oldTranslationNodes);
   mTransitionTranslationNodes.overlap(thread->getSequence()->translationMatters);

   mTransitionScaleNodes.overlap(thread->mTransitionData.oldScaleNodes);
   mTransitionScaleNodes.overlap(thread->getSequence()->scaleMatters);

   // if we aren't already in the list of transition threads, add us now
   S32 i;
   for (i=0; i<mTransitionThreads.size(); i++)
      if (mTransitionThreads[i]==thread)
         break;
   if (i==mTransitionThreads.size())
      mTransitionThreads.push_back(thread);

   updateTransitions();
}

void TSShapeInstance::clearTransition(TSThread * thread)
{
   if (!thread->mTransitionData.inTransition)
      return;

   // if other transitions are still playing,
   // make sure transforms are up to date
   if (mTransitionThreads.size()>1)
      animateNodeSubtrees();

   // turn off transition...
   thread->mTransitionData.inTransition = false;

   // remove us from transition list
   S32 i;
   if (mTransitionThreads.size() != 0) {
      for (i=0; i<mTransitionThreads.size(); i++)
	 if (mTransitionThreads[i]==thread)
	    break;
      AssertFatal(i!=mTransitionThreads.size(),"TSShapeInstance::clearTransition");
      mTransitionThreads.erase(i);
   }

   // recompute transitionNodes
   mTransitionRotationNodes.clearAll();
   mTransitionTranslationNodes.clearAll();
   mTransitionScaleNodes.clearAll();
   for (i=0; i<mTransitionThreads.size(); i++)
   {
      mTransitionRotationNodes.overlap(mTransitionThreads[i]->mTransitionData.oldRotationNodes);
      mTransitionRotationNodes.overlap(mTransitionThreads[i]->getSequence()->rotationMatters);

      mTransitionTranslationNodes.overlap(mTransitionThreads[i]->mTransitionData.oldTranslationNodes);
      mTransitionTranslationNodes.overlap(mTransitionThreads[i]->getSequence()->translationMatters);

      mTransitionScaleNodes.overlap(mTransitionThreads[i]->mTransitionData.oldScaleNodes);
      mTransitionScaleNodes.overlap(mTransitionThreads[i]->getSequence()->scaleMatters);
   }

   setDirty(ThreadDirty);

   updateTransitions();
}

void TSShapeInstance::updateTransitions()
{
   if (mTransitionThreads.empty())
      return;

   TSIntegerSet transitionNodes;
   updateTransitionNodeTransforms(transitionNodes);

   S32 i;
   mNodeReferenceRotations.setSize(mShape->mNodes.size());
   mNodeReferenceTranslations.setSize(mShape->mNodes.size());
   for (i=0; i<mShape->mNodes.size(); i++)
   {
      if (mTransitionRotationNodes.test(i))
         mNodeReferenceRotations[i].set(smNodeCurrentRotations[i]);
      if (mTransitionTranslationNodes.test(i))
         mNodeReferenceTranslations[i] = smNodeCurrentTranslations[i];
   }

   if (animatesScale())
   {
      // Make sure smNodeXXXScale arrays have been resized
      TSIntegerSet dummySet;
      handleDefaultScale(0, 0, dummySet);

      if (animatesUniformScale())
      {
         mNodeReferenceUniformScales.setSize(mShape->mNodes.size());
         for (i=0; i<mShape->mNodes.size(); i++)
         {
            if (mTransitionScaleNodes.test(i))
               mNodeReferenceUniformScales[i] = smNodeCurrentUniformScales[i];
         }
      }
      else if (animatesAlignedScale())
      {
         mNodeReferenceScaleFactors.setSize(mShape->mNodes.size());
         for (i=0; i<mShape->mNodes.size(); i++)
         {
            if (mTransitionScaleNodes.test(i))
               mNodeReferenceScaleFactors[i] = smNodeCurrentAlignedScales[i];
         }
      }
      else
      {
         mNodeReferenceScaleFactors.setSize(mShape->mNodes.size());
         mNodeReferenceArbitraryScaleRots.setSize(mShape->mNodes.size());
         for (i=0; i<mShape->mNodes.size(); i++)
         {
            if (mTransitionScaleNodes.test(i))
            {
               mNodeReferenceScaleFactors[i] = smNodeCurrentArbitraryScales[i].mScale;
               mNodeReferenceArbitraryScaleRots[i].set(smNodeCurrentArbitraryScales[i].mRotate);
            }
         }
      }
   }

   // reset transition durations to account for new reference transforms
   for (i=0; i<mTransitionThreads.size(); i++)
   {
      TSThread * th = mTransitionThreads[i];
      if (th->mTransitionData.inTransition)
      {
         th->mTransitionData.duration *= 1.0f - th->mTransitionData.pos;
         th->mTransitionData.pos = 0.0f;
      }
   }
}

void TSShapeInstance::checkScaleCurrentlyAnimated()
{
   mScaleCurrentlyAnimated=true;
   for (S32 i=0; i<mThreadList.size(); i++)
      if (mThreadList[i]->getSequence()->animatesScale())
         return;
   mScaleCurrentlyAnimated=false;
}

void TSShapeInstance::setBlendEnabled(TSThread * thread, bool blendOn)
{
   thread->mBlendDisabled = !blendOn;
}

bool TSShapeInstance::getBlendEnabled(TSThread * thread)
{
   return !thread->mBlendDisabled;
}

void TSShapeInstance::setPriority(TSThread * thread, F32 priority)
{
   thread->mPriority = priority;
}

F32 TSShapeInstance::getPriority(TSThread * thread)
{
   return thread->mPriority;
}

F32 TSShapeInstance::getTime(TSThread * thread)
{
   return thread->getTime();
}

F32 TSShapeInstance::getPos(TSThread * thread)
{
   return thread->getPos();
}

void TSShapeInstance::setTime(TSThread * thread, F32 time)
{
   thread->setTime(time);
}

void TSShapeInstance::setPos(TSThread * thread, F32 pos)
{
   thread->setPos(pos);
}

bool TSShapeInstance::isInTransition(TSThread * thread)
{
   return thread->isInTransition();
}

F32 TSShapeInstance::getTimeScale(TSThread * thread)
{
   return thread->getTimeScale();
}

void TSShapeInstance::setTimeScale(TSThread * thread, F32 timeScale)
{
   thread->setTimeScale(timeScale);
}

F32 TSShapeInstance::getDuration(TSThread * thread)
{
   return thread->getDuration();
}

F32 TSShapeInstance::getScaledDuration(TSThread * thread)
{
   return thread->getScaledDuration();
}

S32 TSShapeInstance::getKeyframeCount(TSThread * thread)
{
   return thread->getKeyframeCount();
}

S32 TSShapeInstance::getKeyframeNumber(TSThread * thread)
{
   return thread->getKeyframeNumber();
}

void TSShapeInstance::setKeyframeNumber(TSThread * thread, S32 kf)
{
   thread->setKeyframeNumber(kf);
}


// advance time on a particular thread
void TSShapeInstance::advanceTime(F32 delta, TSThread * thread)
{
   thread->advanceTime(delta);
}

// advance time on all threads
void TSShapeInstance::advanceTime(F32 delta)
{
   for (S32 i=0; i<mThreadList.size(); i++)
      mThreadList[i]->advanceTime(delta);
}

// advance pos on a particular thread
void TSShapeInstance::advancePos(F32 delta, TSThread * thread)
{
   thread->advancePos(delta);
}

// advance pos on all threads
void TSShapeInstance::advancePos(F32 delta)
{
   for (S32 i=0; i<mThreadList.size(); i++)
      mThreadList[i]->advancePos(delta);
}

