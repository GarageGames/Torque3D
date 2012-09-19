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
#include "T3D/gameBase/processList.h"

#include "T3D/gameBase/gameBase.h"
#include "platform/profiler.h"
#include "console/consoleTypes.h"

//----------------------------------------------------------------------------

ProcessObject::ProcessObject()
 : mProcessTag( 0 ),   
   mOrderGUID( 0 ),
   mProcessTick( false ),
   mIsGameBase( false )
{ 
   mProcessLink.next = mProcessLink.prev = this;
}

void ProcessObject::plUnlink()
{
   mProcessLink.next->mProcessLink.prev = mProcessLink.prev;
   mProcessLink.prev->mProcessLink.next = mProcessLink.next;
   mProcessLink.next = mProcessLink.prev = this;
}

void ProcessObject::plLinkAfter(ProcessObject * obj)
{
   AssertFatal(mProcessLink.next == this && mProcessLink.prev == this,"ProcessObject::plLinkAfter: must be unlinked before calling this");
#ifdef TORQUE_DEBUG
   ProcessObject * test1 = obj;
   ProcessObject * test2 = obj->mProcessLink.next;
   ProcessObject * test3 = obj->mProcessLink.prev;
   ProcessObject * test4 = this;
#endif

   // Link this after obj
   mProcessLink.next = obj->mProcessLink.next;
   mProcessLink.prev = obj;
   obj->mProcessLink.next = this;
   mProcessLink.next->mProcessLink.prev = this;

#ifdef TORQUE_DEBUG
   AssertFatal(test1->mProcessLink.next->mProcessLink.prev==test1 && test1->mProcessLink.prev->mProcessLink.next==test1,"Doh!");
   AssertFatal(test2->mProcessLink.next->mProcessLink.prev==test2 && test2->mProcessLink.prev->mProcessLink.next==test2,"Doh!");
   AssertFatal(test3->mProcessLink.next->mProcessLink.prev==test3 && test3->mProcessLink.prev->mProcessLink.next==test3,"Doh!");
   AssertFatal(test4->mProcessLink.next->mProcessLink.prev==test4 && test4->mProcessLink.prev->mProcessLink.next==test4,"Doh!");
#endif
}

void ProcessObject::plLinkBefore(ProcessObject * obj)
{
   AssertFatal(mProcessLink.next == this && mProcessLink.prev == this,"ProcessObject::plLinkBefore: must be unlinked before calling this");
#ifdef TORQUE_DEBUG
   ProcessObject * test1 = obj;
   ProcessObject * test2 = obj->mProcessLink.next;
   ProcessObject * test3 = obj->mProcessLink.prev;
   ProcessObject * test4 = this;
#endif

   // Link this before obj
   mProcessLink.next = obj;
   mProcessLink.prev = obj->mProcessLink.prev;
   obj->mProcessLink.prev = this;
   mProcessLink.prev->mProcessLink.next = this;

#ifdef TORQUE_DEBUG
   AssertFatal(test1->mProcessLink.next->mProcessLink.prev==test1 && test1->mProcessLink.prev->mProcessLink.next==test1,"Doh!");
   AssertFatal(test2->mProcessLink.next->mProcessLink.prev==test2 && test2->mProcessLink.prev->mProcessLink.next==test2,"Doh!");
   AssertFatal(test3->mProcessLink.next->mProcessLink.prev==test3 && test3->mProcessLink.prev->mProcessLink.next==test3,"Doh!");
   AssertFatal(test4->mProcessLink.next->mProcessLink.prev==test4 && test4->mProcessLink.prev->mProcessLink.next==test4,"Doh!");
#endif
}

void ProcessObject::plJoin(ProcessObject * head)
{
   ProcessObject * tail1 = head->mProcessLink.prev;
   ProcessObject * tail2 = mProcessLink.prev;
   tail1->mProcessLink.next = this;
   mProcessLink.prev = tail1;
   tail2->mProcessLink.next = head;
   head->mProcessLink.prev = tail2;
}

//--------------------------------------------------------------------------

ProcessList::ProcessList()
{
   mCurrentTag = 0;
   mDirty = false;

   mTotalTicks = 0;
   mLastTick = 0;
   mLastTime = 0;
   mLastDelta = 0.0f;
}

void ProcessList::addObject( ProcessObject *obj )
{
   obj->plLinkAfter(&mHead);
}

//----------------------------------------------------------------------------

void ProcessList::orderList()
{
   // ProcessObject tags are initialized to 0, so current tag should never be 0.
   if (++mCurrentTag == 0)
      mCurrentTag++;

   // Install a temporary head node
   ProcessObject list;
   list.plLinkBefore(mHead.mProcessLink.next);
   mHead.plUnlink();

   // start out by (bubble) sorting list by GUID
   for (ProcessObject * cur = list.mProcessLink.next; cur != &list; cur = cur->mProcessLink.next)
   {
      if (cur->mOrderGUID == 0)
         // special case -- can be no lower, so accept as lowest (this is also
         // a common value since it is what non ordered objects have)
         continue;

      for (ProcessObject * walk = cur->mProcessLink.next; walk != &list; walk = walk->mProcessLink.next)
      {
         if (walk->mOrderGUID < cur->mOrderGUID)
         {
            // swap walk and cur -- need to be careful because walk might be just after cur
            // so insert after item before cur and before item after walk
            ProcessObject * before = cur->mProcessLink.prev;
            ProcessObject * after = walk->mProcessLink.next;
            cur->plUnlink();
            walk->plUnlink();
            cur->plLinkBefore(after);
            walk->plLinkAfter(before);
            ProcessObject * swap = walk;
            walk = cur;
            cur = swap;
         }
      }
   }

   // Reverse topological sort into the original head node
   while (list.mProcessLink.next != &list) 
   {
      ProcessObject * ptr = list.mProcessLink.next;
      ProcessObject * afterObject = ptr->getAfterObject();
      ptr->mProcessTag = mCurrentTag;
      ptr->plUnlink();
      if (afterObject) 
      {
         // Build chain "stack" of dependent objects and patch
         // it to the end of the current list.
         while (afterObject && afterObject->mProcessTag != mCurrentTag)
         {
            afterObject->mProcessTag = mCurrentTag;
            afterObject->plUnlink();
            afterObject->plLinkBefore(ptr);
            ptr = afterObject;
            afterObject = ptr->getAfterObject();
         }
         ptr->plJoin(&mHead);
      }
      else
         ptr->plLinkBefore(&mHead);
   }
   mDirty = false;
}

GameBase* ProcessList::getGameBase( ProcessObject *obj )
{
   if ( !obj->mIsGameBase )
      return NULL;

   return static_cast< GameBase* >( obj );
}


void ProcessList::dumpToConsole()
{
   for (ProcessObject * pobj = mHead.mProcessLink.next; pobj != &mHead; pobj = pobj->mProcessLink.next)
   {
      SimObject * obj = dynamic_cast<SimObject*>(pobj);
      if (obj)
         Con::printf("id %i, order guid %i, type %s", obj->getId(), pobj->mOrderGUID, obj->getClassName());
      else
         Con::printf("---unknown object type, order guid %i", pobj->mOrderGUID);
   }
}

//----------------------------------------------------------------------------

bool ProcessList::advanceTime(SimTime timeDelta)
{
   PROFILE_START(ProcessList_AdvanceTime);

   // some drivers change the FPU control state, which will break our control object simulation
   // (leading to packet mismatch errors due to small FP differences).  So set it to the known 
   // state before advancing.
   U32 mathState = Platform::getMathControlState();
   Platform::setMathControlStateKnown();

   if (mDirty) 
      orderList();

   SimTime targetTime = mLastTime + timeDelta;
   SimTime targetTick = targetTime - (targetTime % TickMs);
   SimTime tickDelta = targetTick - mLastTick;
   bool tickPass = mLastTick != targetTick;

   if ( tickPass )
      mPreTick.trigger();

   // Advance all the objects.
   for (; mLastTick != targetTick; mLastTick += TickMs)
      onAdvanceObjects();

   mLastTime = targetTime;
   mLastDelta = ((TickMs - ((targetTime+1) % TickMs)) % TickMs) / F32(TickMs);

   if ( tickPass )
      mPostTick.trigger( tickDelta );

   // restore math control state in case others are relying on it being a certain value
   Platform::setMathControlState(mathState);

   PROFILE_END();
   return tickPass;
}

//----------------------------------------------------------------------------

void ProcessList::advanceObjects()
{
   PROFILE_START(ProcessList_AdvanceObjects);

   // A little link list shuffling is done here to avoid problems
   // with objects being deleted from within the process method.
   ProcessObject list;
   list.plLinkBefore(mHead.mProcessLink.next);
   mHead.plUnlink();
   for (ProcessObject * pobj = list.mProcessLink.next; pobj != &list; pobj = list.mProcessLink.next)
   {
      pobj->plUnlink();
      pobj->plLinkBefore(&mHead);
      
      onTickObject(pobj);
   }

   mTotalTicks++;

   PROFILE_END();
}



