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

#ifndef _TSINTEGERSET_H_
#define _TSINTEGERSET_H_

#ifndef _PLATFORM_H_
#include "platform/platform.h"
#endif
#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif


#if defined(TORQUE_MAX_LIB)
#define MAX_TS_SET_DWORDS 32
#else
#define MAX_TS_SET_DWORDS 64
#endif

#define MAX_TS_SET_SIZE   (32*MAX_TS_SET_DWORDS)

class Stream;

/// The standard mathmatical set, where there are no duplicates.  However,
/// this set uses bits instead of numbers.
class TSIntegerSet
{
   /// The bits!
   U32 bits[MAX_TS_SET_DWORDS];

public:

   /// Sets this bit to false
   void clear(S32 index);
   /// Set this bit to true
   void set(S32 index);
   /// Is this bit true?
   bool test(S32 index) const;

   /// Sets all bits to false
   void clearAll(S32 upto = MAX_TS_SET_SIZE);
   /// Sets all bits to true
   void setAll(S32 upto = MAX_TS_SET_SIZE);
   /// Tests all bits for true
   bool testAll(S32 upto = MAX_TS_SET_SIZE) const;

   /// Counts set bits
   S32 count(S32 upto = MAX_TS_SET_SIZE) const;

   /// intersection (a & b)
   void intersect(const TSIntegerSet&);
   /// union (a | b)
   void overlap(const TSIntegerSet&);
   /// xor (a | b) & ( !(a & b) )
   void difference(const TSIntegerSet&);
   /// subtraction (a - b)
   void takeAway(const TSIntegerSet&);

   /// copy one integer set into another
   void copy(const TSIntegerSet&);

   void insert(S32 index, bool value);
   void erase(S32 index);

   void operator=(const TSIntegerSet& otherSet) { copy(otherSet); }

   S32 start() const;
   S32 end() const;
   void next(S32 & i) const;

   void read(Stream *);
   void write(Stream *) const;

   TSIntegerSet();
   TSIntegerSet(const TSIntegerSet&);
};

inline void TSIntegerSet::clear(S32 index)
{
   AssertFatal(index>=0 && index<MAX_TS_SET_SIZE,"TS::IntegerSet::clear");

   bits[index>>5] &= ~(1 << (index & 31));
}

inline void TSIntegerSet::set(S32 index)
{
   AssertFatal(index>=0 && index<MAX_TS_SET_SIZE,"TS::IntegerSet::set");

   bits[index>>5] |= 1 << (index & 31);
}

inline bool TSIntegerSet::test(S32 index) const
{
   AssertFatal(index>=0 && index<MAX_TS_SET_SIZE,"TS::IntegerSet::test");

   return ((bits[index>>5] & (1 << (index & 31)))!=0);
}

#endif
