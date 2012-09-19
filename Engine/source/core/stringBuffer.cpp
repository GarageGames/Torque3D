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

#include "core/stringBuffer.h"
#include "core/frameAllocator.h"
#include "core/strings/unicode.h"
#include "core/strings/stringFunctions.h"


#if defined(TORQUE_DEBUG)
   class StringBufferManager
   {
      public:
         static StringBufferManager& getManager();
         Vector<StringBuffer*> strings;
         U64 request8;
         U64 request16;

         StringBufferManager()
         {
            VECTOR_SET_ASSOCIATION( strings );
         }
         
         void add(StringBuffer* s);
         void remove(StringBuffer* s);
         void updateStats();
         void dumpStats();
         void dumpAllStrings();
   };

   ConsoleFunction(sbmDumpStats, void, 1, 1, "")
   {
      StringBufferManager::getManager().dumpStats();
   }

   ConsoleFunction(sbmDumpStrings, void, 1, 1, "")
   {
      StringBufferManager::getManager().dumpAllStrings();
   }
#endif // TORQUE_DEBUG


#if defined(TORQUE_DEBUG)
#define SBMAddThisStringBuffer() \
   StringBufferManager::getManager().add(this); \
   rc = new RequestCounts; \
   clearRequestCounts()
#define incRequestCount8() rc->requestCount8++
#define incRequestCount16() rc->requestCount16++
#define decRequestCount16() rc->requestCount16--
#else
#define SBMAddThisStringBuffer()
#define incRequestCount8()
#define incRequestCount16()
#define decRequestCount16()
#endif

StringBuffer::StringBuffer() : mBuffer(), mBuffer8(), mDirty8(true)
{
   VECTOR_SET_ASSOCIATION( mBuffer );
   SBMAddThisStringBuffer();
   mBuffer.push_back(0);
};

/// Copy constructor. Very important.
StringBuffer::StringBuffer(const StringBuffer &copy) : mBuffer(), mBuffer8()
{
   VECTOR_SET_ASSOCIATION( mBuffer );
   SBMAddThisStringBuffer();
   set(&copy);
};

StringBuffer::StringBuffer(const StringBuffer *in)
: mBuffer(), mBuffer8()
{
   VECTOR_SET_ASSOCIATION( mBuffer );
   SBMAddThisStringBuffer();
   set(in);
}

StringBuffer::StringBuffer(const UTF8 *in)
: mBuffer(), mBuffer8()
{
   VECTOR_SET_ASSOCIATION( mBuffer );
   SBMAddThisStringBuffer();
   set(in);
}

StringBuffer::StringBuffer(const UTF16 *in)
: mBuffer(), mBuffer8()
{
   VECTOR_SET_ASSOCIATION( mBuffer );
   SBMAddThisStringBuffer();
   set(in);
}

//-------------------------------------------------------------------------

StringBuffer::~StringBuffer()
{
   // Everything will get cleared up nicely cuz it's a vector. Sweet.
   #if defined(TORQUE_DEBUG)
   StringBufferManager::getManager().remove(this);
   delete rc;
   #endif

}

//-------------------------------------------------------------------------

void StringBuffer::set(const StringBuffer *in)
{
   // Copy the vector.
   mBuffer.setSize(in->mBuffer.size());
   dMemcpy(mBuffer.address(), in->mBuffer.address(), sizeof(UTF16) * mBuffer.size());
   mDirty8 = true;
}

void StringBuffer::set(const UTF8 *in)
{
   incRequestCount8();
   // Convert and store. Note that a UTF16 version of the string cannot be longer.
   FrameTemp<UTF16> tmpBuff(dStrlen(in)+1);
   if(!in || in[0] == 0 || !convertUTF8toUTF16(in, tmpBuff, dStrlen(in)+1))
   {
      // Easy out, it's a blank string, or a bad string.
      mBuffer.clear();
      mBuffer.push_back(0);
      AssertFatal(mBuffer.last() == 0, "StringBuffer::set UTF8 - not a null terminated string!");
      mDirty8 = true;
      return;
   }

   // Otherwise, we've a copy to do. (This might not be strictly necessary.)
   mBuffer.setSize(dStrlen(tmpBuff)+1);
   dMemcpy(mBuffer.address(), tmpBuff, sizeof(UTF16) * mBuffer.size());
   mBuffer.compact();
   AssertFatal(mBuffer.last() == 0, "StringBuffer::set UTF8 - not a null terminated string!");
   mDirty8 = true;
}

void StringBuffer::set(const UTF16 *in)
{
   incRequestCount16();
   // Just copy, it's already UTF16.
   U32 newsize = dStrlen(in);
   mBuffer.setSize(newsize + 1);
   dMemcpy(mBuffer.address(), in, sizeof(UTF16) * newsize);
   mBuffer[newsize] = '\0';
   mBuffer.compact();
   AssertFatal(mBuffer.last() == 0, "StringBuffer::set UTF16 - not a null terminated string!");
   mDirty8 = true;
}

//-------------------------------------------------------------------------

void StringBuffer::append(const StringBuffer &in)
{
   append(in.mBuffer.address(), in.length());
}

void StringBuffer::append(const UTF8* in)
{
   incRequestCount8();
   decRequestCount16(); // because we're about to inc it when we go through append(utf16)
   
   // convert to UTF16, because that's our internal format.
   // if the conversion fails, exit.
   UTF16* tmp = convertUTF8toUTF16(in);
   AssertFatal(tmp, "StringBuffer::append(UTF8) - could not convert UTF8 string!");
   if(!tmp)
      return;
      
   append(tmp);
   delete[] tmp;
}

void StringBuffer::append(const UTF16* in)
{
   AssertFatal(in, "StringBuffer::append(UTF16) - null UTF16 string!");
   append(in, dStrlen(in));
}

void StringBuffer::append(const UTF16* in, const U32 len)
{
   incRequestCount16();
   
   // Stick in onto the end of us - first make space.
   U32 oldSize = length();
   mBuffer.increment(len);
 
   // Copy it in, ignoring both our terminator and theirs.
   dMemcpy(&mBuffer[oldSize], in, sizeof(UTF16) * len);

   // Terminate the string.
   mBuffer.last() = 0;
   
   // mark utf8 buffer dirty
   mDirty8 = true;
}

void StringBuffer::insert(const U32 charOffset, const StringBuffer &in)
{
   insert(charOffset, in.mBuffer.address(), in.length());
}

void StringBuffer::insert(const U32 charOffset, const UTF8* in)
{
   incRequestCount8();
   decRequestCount16();
   
   // convert to UTF16, because that's our internal format.
   // if the conversion fails, exit.
   UTF16* tmp = convertUTF8toUTF16(in);
   AssertFatal(tmp, "StringBuffer::insert(UTF8) - could not convert UTF8 string!");
   if(!tmp)
      return;
      
   insert(charOffset, tmp);
   delete[] tmp;
}

void StringBuffer::insert(const U32 charOffset, const UTF16* in)
{
   AssertFatal(in, "StringBuffer::insert(UTF16) - null UTF16 string!");
   insert(charOffset, in, dStrlen(in));
}

void StringBuffer::insert(const U32 charOffset, const UTF16* in, const U32 len)
{
   incRequestCount16();
   
   // Deal with append case.
   if(charOffset >= length())
   {
      append(in, len);
      return;
   }

   // Append was easy, now we have to do some work.
   
   // Copy everything we have that comes after charOffset past where the new
   // string data will be.

   // Figure the number of UTF16's to copy, taking into account the possibility
   // that we may be inserting a long string into a short string.

   // How many chars come after the insert point? Add 1 to deal with terminator.
   const U32 copyCharLength = (S32)(length() - charOffset) + 1;

   // Make some space in our buffer. We only need in.length() more as we
   // will be dropping one of the two terminators present in this operation.
   mBuffer.increment(len);

   for(S32 i=copyCharLength-1; i>=0; i--)
      mBuffer[charOffset+i+len] = mBuffer[charOffset+i];

   //  Can't copy directly: memcpy behavior is undefined if src and dst overlap.
   //dMemcpy(&mBuffer[charOffset+len], &mBuffer[charOffset], sizeof(UTF16) * copyCharLength);

   // And finally copy the new data in, not including its terminator.
   dMemcpy(&mBuffer[charOffset], in, sizeof(UTF16) * len);

   // All done!
   AssertFatal(mBuffer.last() == 0, "StringBuffer::insert - not a null terminated string!");
   
   // mark utf8 buffer dirty
   mDirty8 = true;
}

StringBuffer StringBuffer::substring(const U32 start, const U32 len) const
{
   // Deal with bonehead user input.
   if(start >= length() || len == 0)
   {
      // Either they asked beyond the end of the string or we're null. Either
      // way, let's give them a null string.
      return StringBuffer("");
   }

   AssertFatal(start < length(), "StringBuffer::substring - invalid start!");
   AssertFatal(start+len <= length(), "StringBuffer::substring - invalid len!");
   AssertFatal(len > 0, "StringBuffer::substring - len must be >= 1.");

   StringBuffer tmp;
   tmp.mBuffer.clear();
   for(S32 i=0; i<len; i++)
      tmp.mBuffer.push_back(mBuffer[start+i]);
   if(tmp.mBuffer.last() != 0) tmp.mBuffer.push_back(0);
   
   // Make sure this shit is terminated; we might get a totally empty string.
   if(!tmp.mBuffer.size()) 
      tmp.mBuffer.push_back(0);

   return tmp;
}

UTF8* StringBuffer::createSubstring8(const U32 start, const U32 len) const
{
   incRequestCount8();
   
   StringBuffer sub = this->substring(start, len);
   return sub.createCopy8();
}

void StringBuffer::cut(const U32 start, const U32 len)
{
   AssertFatal(start < length(), "StringBuffer::cut - invalid start!");
   AssertFatal(start+len <= length(), "StringBuffer::cut - invalid len!");
   AssertFatal(len > 0, "StringBuffer::cut - len >= 1.");

   AssertFatal(mBuffer.last() == 0, "StringBuffer::cut - not a null terminated string! (pre)");

   // Now snip things.
   for(S32 i=start; i<mBuffer.size()-len; i++)
      mBuffer[i] = mBuffer[i+len];
   mBuffer.decrement(len);
   mBuffer.compact();

   AssertFatal(mBuffer.last() == 0, "StringBuffer::cut - not a null terminated string! (post)");
   
   // mark utf8 buffer dirty
   mDirty8 = true;
}

const UTF16 StringBuffer::getChar(const U32 offset) const
{
   incRequestCount16();
   
   // Allow them to grab the null terminator if they want.
   AssertFatal(offset<mBuffer.size(), "StringBuffer::getChar - outside of range.");

   return mBuffer[offset];
}

//-------------------------------------------------------------------------

void StringBuffer::getCopy8(UTF8 *buff, const U32 buffSize) const
{
   incRequestCount8();
   AssertFatal(mBuffer.last() == 0, "StringBuffer::get UTF8 - not a null terminated string!");
   convertUTF16toUTF8(mBuffer.address(), buff, buffSize);
}

void StringBuffer::getCopy(UTF16 *buff, const U32 buffSize) const
{
   incRequestCount16();
   // Just copy it out.
   AssertFatal(mBuffer.last() == 0, "StringBuffer::get UTF8 - not a null terminated string!");
   dMemcpy(buff, mBuffer.address(), sizeof(UTF16) * getMin(buffSize, (U32)mBuffer.size()));
   // ensure null termination.
   buff[buffSize-1] = NULL;
}

UTF8* StringBuffer::createCopy8() const
{
   incRequestCount8();
   // convert will create a buffer of the appropriate size for a null terminated
   // input string.
   UTF8* out = convertUTF16toUTF8(mBuffer.address());
   return out;
}

const UTF16* StringBuffer::getPtr() const
{
   incRequestCount16();
   // get a pointer to the StringBuffer's data store.
   // this pointer is volatile, and not thread safe.
   // use this in situations where you can be sure that the StringBuffer will
   // not be modified out from under you.
   // the key here is, you avoid yet another data copy. data copy is slow on
   // most modern hardware.
   return mBuffer.address();
}

const UTF8* StringBuffer::getPtr8() const
{
   incRequestCount8();
   // get a pointer to the utf8 version of the StringBuffer's data store.
   // if the utf8 version is dirty, update it first.
   if(mDirty8)
      // force const cheating.
      const_cast<StringBuffer*>(this)->updateBuffer8();
   return mBuffer8.address();
}

void StringBuffer::updateBuffer8()
{
   U32 slackLen = getUTF8BufferSizeEstimate();
   mBuffer8.setSize(slackLen);
   U32 len = convertUTF16toUTF8(mBuffer.address(), mBuffer8.address(), slackLen);
   mBuffer8.setSize(len+1);
   mBuffer8.compact();
   mDirty8 = false;
}


#if defined(TORQUE_DEBUG)
StringBufferManager& StringBufferManager::getManager()
{ 
   static StringBufferManager _sbm; return _sbm;
}

void StringBufferManager::add(StringBuffer* s)
{
   strings.push_back(s); 
}

void StringBufferManager::remove(StringBuffer* s)
{
   U32 nstrings = strings.size() - 1;
   for(S32 i = nstrings; i >= 0; i--)
      if(strings[i] == s)
         strings.erase_fast(i);
}

void StringBufferManager::updateStats()
{
   request8 = 0;
   request16 = 0;
   U32 nstrings = strings.size();
   for(int i=0; i < nstrings; i++)
   {
      request8 += strings[i]->rc->requestCount8;
      request16 += strings[i]->rc->requestCount16;
   }
}

void StringBufferManager::dumpStats()
{
   updateStats();
   Con::printf("===== String Manager Stats =====");
   Con::printf(" strings:        %i", strings.size());
   Con::printf(" utf8 requests:  %Lu", request8);
   Con::printf(" utf16 requests: %Lu", request16);
}

void StringBufferManager::dumpAllStrings()
{
   U32 nstrings = strings.size();
   Con::printf("===== String Manager: All Strings =====");
   Con::printf(" utf8 | utf16 | string");
   for(int i=0; i < nstrings; i++)
   {
      UTF8* tmp = strings[i]->createCopy8();
      strings[i]->rc->requestCount8--;
      Con::printf("%5llu  %5llu    \"%s\"", strings[i]->rc->requestCount8, strings[i]->rc->requestCount16, tmp);
      delete[] tmp;
   }
}

#endif
