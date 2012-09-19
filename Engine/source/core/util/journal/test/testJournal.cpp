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

#include "unit/test.h"
#include "core/util/journal/journaledSignal.h"
#include "core/util/safeDelete.h"

using namespace UnitTesting;

CreateUnitTest(TestsJournalRecordAndPlayback, "Journal/Basic")
{
   U32 _lastTriggerValue;

   void triggerReceiver(U16 msg)
   {
      _lastTriggerValue = msg;
   }

   void run()
   {
      // Reset the last trigger value just in case...
      _lastTriggerValue = 0;

      // Set up a journaled signal to test with.
      JournaledSignal<void(U16)> testEvent;

      testEvent.notify(this, &TestsJournalRecordAndPlayback::triggerReceiver);

      // Initialize journal recording and fire off some events...
      Journal::Record("test.jrn");

      testEvent.trigger(16);
      testEvent.trigger(17);
      testEvent.trigger(18);

      test(_lastTriggerValue == 18, "Should encounter last triggered value (18).");

      Journal::Stop();

      // Clear it...
      _lastTriggerValue = 0;

      // and play back - should get same thing.
      Journal::Play("test.jrn");

      // Since we fired 3 events, it should take three loops.
      test(Journal::PlayNext(), "Should be two more events.");
      test(Journal::PlayNext(), "Should be one more event.");
      test(!Journal::PlayNext(), "Should be no more events.");

      test(_lastTriggerValue == 18, "Should encounter last journaled value (18).");
   }
};

CreateUnitTest(TestsJournalDynamicSignals, "Journal/DynamicSignals")
{
   typedef JournaledSignal<void(U32, U16)> EventA;
   typedef JournaledSignal<void(U8, S8)> EventB;
   typedef JournaledSignal<void(U32, S32)> EventC;

   EventA *dynamicA;
   EventB *dynamicB;
   EventC *dynamicC;

   // Root, non-dynamic signal receiver.
   void receiverRoot(U8 msg)
   {
      if(msg==1)
      {
         dynamicA = new EventA();
         dynamicA->notify(this, &TestsJournalDynamicSignals::receiverA);
      }

      if(msg==2)
      {
         dynamicB = new EventB();
         dynamicB->notify(this, &TestsJournalDynamicSignals::receiverB);
      }

      if(msg==3)
      {
         dynamicC = new EventC();
         dynamicC->notify(this, &TestsJournalDynamicSignals::receiverC);
      }
   }

   U32 recvA, recvB, recvC;

   void receiverA(U32, U16 d)
   {
      recvA += d;
   }

   void receiverB(U8, S8 d)
   {
      recvB += d;
   }

   void receiverC(U32, S32 d)
   {
      recvC += d;
   }

   void run()
   {
      // Reset our state values.
      recvA = recvB = recvC = 0;

      // Set up a signal to start with.
      JournaledSignal<void(U8)> testEvent;
      testEvent.notify(this, &TestsJournalDynamicSignals::receiverRoot);

      // Initialize journal recording and fire off some events...
      Journal::Record("test.jrn");

      testEvent.trigger(1);
      dynamicA->trigger(8, 100);
      testEvent.trigger(2);
      dynamicA->trigger(8, 8);
      dynamicB->trigger(9, 'a');
      testEvent.trigger(3);
      SAFE_DELETE(dynamicB); // Test a deletion.
      dynamicC->trigger(8, 1);
      dynamicC->trigger(8, 1);

      // Did we end up with expected values? Check before clearing.
      test(recvA == 108, "recvA wasn't 108 - something broken in signals?");
      test(recvB == 'a', "recvB wasn't 'a' - something broken in signals?");
      test(recvC == 2, "recvC wasn't 2 - something broken in signals?");

      // Reset our state values.
      recvA = recvB = recvC = 0;

      // And kill the journal...
      Journal::Stop();

      // Also kill our remaining dynamic signals.
      SAFE_DELETE(dynamicA);
      SAFE_DELETE(dynamicB);
      SAFE_DELETE(dynamicC);

      // Play back - should get same thing.
      Journal::Play("test.jrn");

      // Since we fired 8 events, it should take 7+1=8 loops.
      for(S32 i=0; i<7; i++)
         test(Journal::PlayNext(), "Should be more events.");
      test(!Journal::PlayNext(), "Should be no more events.");

      test(recvA == 108, "recvA wasn't 108 - something broken in journal?");
      test(recvB == 'a', "recvB wasn't 'a' - something broken in journal?");
      test(recvC == 2, "recvC wasn't 2 - something broken in journal?");
   }
};
