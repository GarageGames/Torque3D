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

#include "ts/tsShapeAlloc.h"

#define readOnly()  AssertFatal(mMode==TSShapeAlloc::ReadMode, "TSShapeAlloc: write-only function called when reading")
#define writeOnly() AssertFatal(mMode==TSShapeAlloc::WriteMode,"TSShapeAlloc: read-only function called when writing")

void TSShapeAlloc::setRead(S32 * memBuffer32, S16 * memBuffer16, S8 * memBuffer8, bool clear)
{
   mMemBuffer32 = memBuffer32;
   mMemBuffer16 = memBuffer16;
   mMemBuffer8  = memBuffer8 ;

   mMemGuard32  = 0;
   mMemGuard16  = 0;
   mMemGuard8   = 0;

   mSaveGuard32  = 0;
   mSaveGuard16  = 0;
   mSaveGuard8   = 0;

   if (clear)
   {
      mDest = NULL;
      mSize = 0;
   }

   setSkipMode(false);
   mMode = TSShapeAlloc::ReadMode;
}

void TSShapeAlloc::setWrite()
{
   mMemBuffer32 = 0;
   mMemBuffer16 = 0;
   mMemBuffer8  = 0;

   mSize32 = mFullSize32 = 0;
   mSize16 = mFullSize16 = 0;
   mSize8  = mFullSize8  = 0;

   mMemGuard32  = 0;
   mMemGuard16  = 0;
   mMemGuard8   = 0;

   setSkipMode(false); // doesn't really do anything here...
   mMode = TSShapeAlloc::WriteMode;
}

void TSShapeAlloc::doAlloc()
{
   readOnly();

   mDest = new S8[mSize];
   mSize = 0;
}

void TSShapeAlloc::align32()
{
   readOnly();

   S32 aligned = mSize+3 & (~0x3);
   allocShape8(aligned-mSize);
}

#define IMPLEMENT_ALLOC(suffix,type)                          \
                                                              \
type TSShapeAlloc::get##suffix()                              \
{                                                             \
   readOnly();                                                \
   return *(mMemBuffer##suffix++);                            \
}                                                             \
                                                              \
void TSShapeAlloc::get##suffix(type * dest, S32 num)          \
{                                                             \
   readOnly();                                                \
   dMemcpy(dest,mMemBuffer##suffix,sizeof(type)*num);         \
   mMemBuffer##suffix += num;                                 \
}                                                             \
                                                              \
type * TSShapeAlloc::allocShape##suffix(S32 num)              \
{                                                             \
   readOnly();                                                \
   type * ret = (type*) mDest;                                \
   if (mDest)                                                 \
      mDest += mMult*num*sizeof(type);                        \
   mSize += sizeof(type)*mMult*num;                           \
   return ret;                                                \
}                                                             \
                                                              \
type * TSShapeAlloc::getPointer##suffix(S32 num)              \
{                                                             \
   readOnly();                                                \
   type * ret = (type*)mMemBuffer##suffix;                    \
   mMemBuffer##suffix += num;                                 \
   return ret;                                                \
}                                                             \
                                                              \
type * TSShapeAlloc::copyToShape##suffix(S32 num, bool returnSomething) \
{                                                             \
   readOnly();                                                \
   type * ret = (!returnSomething || mDest) ? (type*)mDest : mMemBuffer##suffix; \
   if (mDest)                                                 \
   {                                                          \
      dMemcpy((S8*)mDest,(S8*)mMemBuffer##suffix,             \
              mMult*num*sizeof(type));                        \
      mDest += mMult*num*sizeof(type);                        \
   }                                                          \
   mMemBuffer##suffix += num;                                 \
   mSize += sizeof(type)*mMult*num;                           \
   return ret;                                                \
}                                                             \
                                                              \
bool TSShapeAlloc::checkGuard##suffix()                       \
{                                                             \
   readOnly();                                                \
   mSaveGuard##suffix=get##suffix();                          \
   bool ret = (mSaveGuard##suffix==mMemGuard##suffix);        \
   mMemGuard##suffix += 1;                                    \
   return ret;                                                \
}                                                             \
                                                              \
type TSShapeAlloc::getPrevGuard##suffix()                     \
{                                                             \
   readOnly();                                                \
   return mMemGuard##suffix - 1;                              \
}                                                             \
                                                              \
type TSShapeAlloc::getSaveGuard##suffix()                     \
{                                                             \
   readOnly();                                                \
   return mSaveGuard##suffix;                                 \
}                                                             \
                                                              \
type * TSShapeAlloc::getBuffer##suffix()                      \
{                                                             \
   writeOnly();                                               \
   return mMemBuffer##suffix;                                 \
}                                                             \
                                                              \
S32 TSShapeAlloc::getBufferSize##suffix()                     \
{                                                             \
   writeOnly();                                               \
   return mSize##suffix;                                      \
}                                                             \
                                                              \
type * TSShapeAlloc::extend##suffix(S32 add)                  \
{                                                             \
   writeOnly();                                               \
   if (mSize##suffix+add>mFullSize##suffix)                   \
   {                                                          \
      S32 numPages = 1+(mFullSize##suffix+add)/TSShapeAlloc::PageSize; \
      mFullSize##suffix = numPages*TSShapeAlloc::PageSize;    \
      type * temp = new type[mFullSize##suffix];              \
      dMemcpy(temp,mMemBuffer##suffix, mSize##suffix * sizeof(type)); \
      delete [] mMemBuffer##suffix;                           \
      mMemBuffer##suffix = temp;                              \
   }                                                          \
   type * ret = mMemBuffer##suffix + mSize##suffix;            \
   mSize##suffix += add;                                      \
   return ret;                                                \
}                                                             \
                                                              \
type TSShapeAlloc::set##suffix(type entry)                    \
{                                                             \
   writeOnly();                                               \
   *extend##suffix(1) = entry;                                \
   return entry;                                              \
}                                                             \
                                                              \
void TSShapeAlloc::copyToBuffer##suffix(type * entries, S32 count) \
{                                                             \
   writeOnly();                                               \
   if (entries)                                               \
      dMemcpy((U8*)extend##suffix(count),(U8*)entries,count*sizeof(type)); \
   else                                                       \
      dMemset((U8*)extend##suffix(count),0,count*sizeof(type)); \
}                                                             \
                                                              \
void TSShapeAlloc::setGuard##suffix()                         \
{                                                             \
   writeOnly();                                               \
   set##suffix(mMemGuard##suffix);                            \
   mMemGuard##suffix += 1;                                    \
}

IMPLEMENT_ALLOC(32,S32)
IMPLEMENT_ALLOC(16,S16)
IMPLEMENT_ALLOC(8,S8)

void TSShapeAlloc::checkGuard()
{
   bool check32 = checkGuard32();
   bool check16 = checkGuard16();
   bool check8  = checkGuard8();
   AssertFatal(check32,avar("TSShapeAlloc::checkGuard32: found %i, wanted %i",getSaveGuard32(),getPrevGuard32()));
   AssertFatal(check16,avar("TSShapeAlloc::checkGuard16: found %i, wanted %i",getSaveGuard16(),getPrevGuard16()));
   AssertFatal(check8 ,avar("TSShapeAlloc::checkGuard8:  found %i, wanted %i",getSaveGuard8() ,getPrevGuard8()));
}

void TSShapeAlloc::setGuard()
{
   setGuard32();
   setGuard16();
   setGuard8();
}


