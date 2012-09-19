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

// Borrowed from: http://burtleburtle.net/bob/hash/doobs.html
//
// Original code by:
//
// By Bob Jenkins, 1996.  bob_jenkins@burtleburtle.net.  You may use this
// code any way you wish, private, educational, or commercial.  It's free.

#include "platform/platform.h"

#include "core/util/hashFunction.h"

namespace Torque
{

#define hashsize(n) ((U32)1<<(n))
#define hashmask(n) (hashsize(n)-1)

/*
--------------------------------------------------------------------
mix -- mix 3 32-bit values reversibly.
For every delta with one or two bits set, and the deltas of all three
high bits or all three low bits, whether the original value of a,b,c
is almost all zero or is uniformly distributed,
* If mix() is run forward or backward, at least 32 bits in a,b,c
have at least 1/4 probability of changing.
* If mix() is run forward, every bit of c will change between 1/3 and
2/3 of the time.  (Well, 22/100 and 78/100 for some 2-bit deltas.)
mix() was built out of 36 single-cycle latency instructions in a
structure that could supported 2x parallelism, like so:
a -= b;
a -= c; x = (c>>13);
b -= c; a ^= x;
b -= a; x = (a<<8);
c -= a; b ^= x;
c -= b; x = (b>>13);
...
Unfortunately, superscalar Pentiums and Sparcs can't take advantage
of that parallelism.  They've also turned some of those single-cycle
latency instructions into multi-cycle latency instructions.  Still,
this is the fastest good hash I could find.  There were about 2^^68
to choose from.  I only looked at a billion or so.
--------------------------------------------------------------------
*/
#define mix(a,b,c) \
{ \
   a -= b; a -= c; a ^= (c>>13); \
   b -= c; b -= a; b ^= (a<<8); \
   c -= a; c -= b; c ^= (b>>13); \
   a -= b; a -= c; a ^= (c>>12);  \
   b -= c; b -= a; b ^= (a<<16); \
   c -= a; c -= b; c ^= (b>>5); \
   a -= b; a -= c; a ^= (c>>3);  \
   b -= c; b -= a; b ^= (a<<10); \
   c -= a; c -= b; c ^= (b>>15); \
}

/*
--------------------------------------------------------------------
hash() -- hash a variable-length key into a 32-bit value
k       : the key (the unaligned variable-length array of bytes)
len     : the length of the key, counting by bytes
initval : can be any 4-byte value
Returns a 32-bit value.  Every bit of the key affects every bit of
the return value.  Every 1-bit and 2-bit delta achieves avalanche.
About 6*len+35 instructions.

The best hash table sizes are powers of 2.  There is no need to do
mod a prime (mod is sooo slow!).  If you need less than 32 bits,
use a bitmask.  For example, if you need only 10 bits, do
h = (h & hashmask(10));
In which case, the hash table should have hashsize(10) elements.

If you are hashing n strings (U8 **)k, do it like this:
for (i=0, h=0; i<n; ++i) h = hash( k[i], len[i], h);

By Bob Jenkins, 1996.  bob_jenkins@burtleburtle.net.  You may use this
code any way you wish, private, educational, or commercial.  It's free.

See http://burtleburtle.net/bob/hash/evahash.html
Use for hash table lookup, or anything where one collision in 2^^32 is
acceptable.  Do NOT use for cryptographic purposes.
--------------------------------------------------------------------
*/

U32 hash(register const U8 *k, register U32 length, register U32 initval)
{
   register U32 a,b,c,len;

   /* Set up the internal state */
   len = length;
   a = b = 0x9e3779b9;  /* the golden ratio; an arbitrary value */
   c = initval;         /* the previous hash value */

   /*---------------------------------------- handle most of the key */
   while (len >= 12)
   {
      a += (k[0] +((U32)k[1]<<8) +((U32)k[2]<<16) +((U32)k[3]<<24));
      b += (k[4] +((U32)k[5]<<8) +((U32)k[6]<<16) +((U32)k[7]<<24));
      c += (k[8] +((U32)k[9]<<8) +((U32)k[10]<<16)+((U32)k[11]<<24));
      mix(a,b,c);
      k += 12; len -= 12;
   }

   /*------------------------------------- handle the last 11 bytes */
   c += length;
   switch(len)              /* all the case statements fall through */
   {
   case 11: c+=((U32)k[10]<<24);
   case 10: c+=((U32)k[9]<<16);
   case 9 : c+=((U32)k[8]<<8);
      /* the first byte of c is reserved for the length */
   case 8 : b+=((U32)k[7]<<24);
   case 7 : b+=((U32)k[6]<<16);
   case 6 : b+=((U32)k[5]<<8);
   case 5 : b+=k[4];
   case 4 : a+=((U32)k[3]<<24);
   case 3 : a+=((U32)k[2]<<16);
   case 2 : a+=((U32)k[1]<<8);
   case 1 : a+=k[0];
      /* case 0: nothing left to add */
   }
   mix(a,b,c);
   /*-------------------------------------------- report the result */
   return c;
}


/*
--------------------------------------------------------------------
mix -- mix 3 64-bit values reversibly.
mix() takes 48 machine instructions, but only 24 cycles on a superscalar
  machine (like Intel's new MMX architecture).  It requires 4 64-bit
  registers for 4::2 parallelism.
All 1-bit deltas, all 2-bit deltas, all deltas composed of top bits of
  (a,b,c), and all deltas of bottom bits were tested.  All deltas were
  tested both on random keys and on keys that were nearly all zero.
  These deltas all cause every bit of c to change between 1/3 and 2/3
  of the time (well, only 113/400 to 287/400 of the time for some
  2-bit delta).  These deltas all cause at least 80 bits to change
  among (a,b,c) when the mix is run either forward or backward (yes it
  is reversible).
This implies that a hash using mix64 has no funnels.  There may be
  characteristics with 3-bit deltas or bigger, I didn't test for
  those.
--------------------------------------------------------------------
*/
#define mix64(a,b,c) \
{ \
  a -= b; a -= c; a ^= (c>>43); \
  b -= c; b -= a; b ^= (a<<9); \
  c -= a; c -= b; c ^= (b>>8); \
  a -= b; a -= c; a ^= (c>>38); \
  b -= c; b -= a; b ^= (a<<23); \
  c -= a; c -= b; c ^= (b>>5); \
  a -= b; a -= c; a ^= (c>>35); \
  b -= c; b -= a; b ^= (a<<49); \
  c -= a; c -= b; c ^= (b>>11); \
  a -= b; a -= c; a ^= (c>>12); \
  b -= c; b -= a; b ^= (a<<18); \
  c -= a; c -= b; c ^= (b>>22); \
}

/*
--------------------------------------------------------------------
hash64() -- hash a variable-length key into a 64-bit value
  k     : the key (the unaligned variable-length array of bytes)
  len   : the length of the key, counting by bytes
  level : can be any 8-byte value
Returns a 64-bit value.  Every bit of the key affects every bit of
the return value.  No funnels.  Every 1-bit and 2-bit delta achieves
avalanche.  About 41+5len instructions.

The best hash table sizes are powers of 2.  There is no need to do
mod a prime (mod is sooo slow!).  If you need less than 64 bits,
use a bitmask.  For example, if you need only 10 bits, do
  h = (h & hashmask(10));
In which case, the hash table should have hashsize(10) elements.

If you are hashing n strings (ub1 **)k, do it like this:
  for (i=0, h=0; i<n; ++i) h = hash( k[i], len[i], h);

By Bob Jenkins, Jan 4 1997.  bob_jenkins@burtleburtle.net.  You may
use this code any way you wish, private, educational, or commercial,
but I would appreciate if you give me credit.

See http://burtleburtle.net/bob/hash/evahash.html
Use for hash table lookup, or anything where one collision in 2^^64
is acceptable.  Do NOT use for cryptographic purposes.
--------------------------------------------------------------------
*/

U64 hash64( register const U8 *k, register U32 length, register U64 initval )
{
  register U64 a,b,c,len;

  /* Set up the internal state */
  len = length;
  a = b = initval;                         /* the previous hash value */
  c = 0x9e3779b97f4a7c13LL; /* the golden ratio; an arbitrary value */

  /*---------------------------------------- handle most of the key */
  while (len >= 24)
  {
    a += (k[0]        +((U64)k[ 1]<< 8)+((U64)k[ 2]<<16)+((U64)k[ 3]<<24)
     +((U64)k[4 ]<<32)+((U64)k[ 5]<<40)+((U64)k[ 6]<<48)+((U64)k[ 7]<<56));
    b += (k[8]        +((U64)k[ 9]<< 8)+((U64)k[10]<<16)+((U64)k[11]<<24)
     +((U64)k[12]<<32)+((U64)k[13]<<40)+((U64)k[14]<<48)+((U64)k[15]<<56));
    c += (k[16]       +((U64)k[17]<< 8)+((U64)k[18]<<16)+((U64)k[19]<<24)
     +((U64)k[20]<<32)+((U64)k[21]<<40)+((U64)k[22]<<48)+((U64)k[23]<<56));
    mix64(a,b,c);
    k += 24; len -= 24;
  }

  /*------------------------------------- handle the last 23 bytes */
  c += length;
  switch(len)              /* all the case statements fall through */
  {
  case 23: c+=((U64)k[22]<<56);
  case 22: c+=((U64)k[21]<<48);
  case 21: c+=((U64)k[20]<<40);
  case 20: c+=((U64)k[19]<<32);
  case 19: c+=((U64)k[18]<<24);
  case 18: c+=((U64)k[17]<<16);
  case 17: c+=((U64)k[16]<<8);
    /* the first byte of c is reserved for the length */
  case 16: b+=((U64)k[15]<<56);
  case 15: b+=((U64)k[14]<<48);
  case 14: b+=((U64)k[13]<<40);
  case 13: b+=((U64)k[12]<<32);
  case 12: b+=((U64)k[11]<<24);
  case 11: b+=((U64)k[10]<<16);
  case 10: b+=((U64)k[ 9]<<8);
  case  9: b+=((U64)k[ 8]);
  case  8: a+=((U64)k[ 7]<<56);
  case  7: a+=((U64)k[ 6]<<48);
  case  6: a+=((U64)k[ 5]<<40);
  case  5: a+=((U64)k[ 4]<<32);
  case  4: a+=((U64)k[ 3]<<24);
  case  3: a+=((U64)k[ 2]<<16);
  case  2: a+=((U64)k[ 1]<<8);
  case  1: a+=((U64)k[ 0]);
    /* case 0: nothing left to add */
  }
  mix64(a,b,c);
  /*-------------------------------------------- report the result */
  return c;
}

} // namespace
