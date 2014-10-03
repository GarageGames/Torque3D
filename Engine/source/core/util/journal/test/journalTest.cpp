//-----------------------------------------------------------------------------
// Copyright (c) 2014 GarageGames, LLC
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

#ifdef TORQUE_TESTS_ENABLED
#include "testing/unitTesting.h"
#include "core/util/journal/journaledSignal.h"
#include "core/util/safeDelete.h"

FIXTURE(Journal)
{
public:
   // Used for basic API test.
   struct receiver
   {
      U16 lastTriggerValue;
      void trigger(U16 msg)
      {
         lastTriggerValue = msg;
      }
   };

   // Used for non-basic test.
   typedef JournaledSignal<void(U32, U16)> EventA;
   typedef JournaledSignal<void(U8, S8)> EventB;
   typedef JournaledSignal<void(U32, S32)> EventC;

   // Root, non-dynamic signal receiver.
   struct multiReceiver {
      U32 recvA, recvB, recvC;

      EventA *dynamicA;
      EventB *dynamicB;
      EventC *dynamicC;

      void receiverRoot(U8 msg)
      {
         if(msg==1)
         {
            dynamicA = new EventA();
            dynamicA->notify(this, &multiReceiver::receiverA);
         }

         if(msg==2)
         {
            dynamicB = new EventB();
            dynamicB->notify(this, &multiReceiver::receiverB);
         }

         if(msg==3)
         {
            dynamicC = new EventC();
            dynamicC->notify(this, &multiReceiver::receiverC);
         }
      }

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
   };
};

TEST_FIX(Journal, BasicAPI)
{
   receiver rec;
   rec.lastTriggerValue = 0;

   // Set up a journaled signal to test with.
   JournaledSignal<void(U16)> testEvent;
   testEvent.notify(&rec, &receiver::trigger);

   // Initialize journal recording and fire off some events...
   Journal::Record("test.jrn");
   ASSERT_TRUE(Journal::IsRecording());

   testEvent.trigger(16);
   testEvent.trigger(17);
   testEvent.trigger(18);

   EXPECT_EQ(rec.lastTriggerValue, 18)
      << "Should encounter last triggered value (18).";

   Journal::Stop();
   ASSERT_FALSE(Journal::IsRecording());

   // Clear it...
   rec.lastTriggerValue = 0;

   // and play back - should get same thing.
   Journal::Play("test.jrn");

   // Since we fired 3 events, it should take three loops.
   EXPECT_TRUE(Journal::PlayNext()) << "Should be two more events.";
   EXPECT_TRUE(Journal::PlayNext()) << "Should be one more event.";
   EXPECT_FALSE(Journal::PlayNext()) << "Should be no more events.";

   EXPECT_EQ(rec.lastTriggerValue, 18)
      << "Should encounter last journaled value (18).";
}

TEST_FIX(Journal, DynamicSignals)
{
   multiReceiver rec;

   // Reset our state values.
   rec.recvA = rec.recvB = rec.recvC = 0;

   // Set up a signal to start with.
   JournaledSignal<void(U8)> testEvent;
   testEvent.notify(&rec, &multiReceiver::receiverRoot);

   // Initialize journal recording and fire off some events...
   Journal::Record("test.jrn");
   ASSERT_TRUE(Journal::IsRecording());

   testEvent.trigger(1);
   rec.dynamicA->trigger(8, 100);
   testEvent.trigger(2);
   rec.dynamicA->trigger(8, 8);
   rec.dynamicB->trigger(9, 'a');
   testEvent.trigger(3);
   SAFE_DELETE(rec.dynamicB); // Test a deletion.
   rec.dynamicC->trigger(8, 1);
   rec.dynamicC->trigger(8, 1);

   // Did we end up with expected values? Check before clearing.
   EXPECT_EQ(rec.recvA, 108) << "recvA wasn't 108 - something broken in signals?";
   EXPECT_EQ(rec.recvB, 'a') << "recvB wasn't 'a' - something broken in signals?";
   EXPECT_EQ(rec.recvC, 2) << "recvC wasn't 2 - something broken in signals?";

   // Reset our state values.
   rec.recvA = rec.recvB = rec.recvC = 0;

   // And kill the journal...
   Journal::Stop();

   // Also kill our remaining dynamic signals.
   SAFE_DELETE(rec.dynamicA);
   SAFE_DELETE(rec.dynamicB);
   SAFE_DELETE(rec.dynamicC);

   // Play back - should get same thing.
   Journal::Play("test.jrn");

   // Since we fired 8 events, it should take 7+1=8 loops.
   for(S32 i = 0; i < 7; i++)
   {
      EXPECT_TRUE(Journal::PlayNext())
         << "Should be more events.";
   }

   EXPECT_FALSE(Journal::PlayNext())
      << "Should be no more events.";

   EXPECT_EQ(rec.recvA, 108) << "recvA wasn't 108 - something broken in journal?";
   EXPECT_EQ(rec.recvB, 'a') << "recvB wasn't 'a' - something broken in journal?";
   EXPECT_EQ(rec.recvC, 2) << "recvC wasn't 2 - something broken in journal?";
}

#endif
