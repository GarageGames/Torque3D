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

#ifndef _STRINGSTACK_H_
#define _STRINGSTACK_H_

#ifndef _STRINGFUNCTIONS_H_
#include "core/strings/stringFunctions.h"
#endif

#ifndef _STRINGTABLE_H_
#include "core/stringTable.h"
#endif

#ifndef _CONSOLE_H_
#include "console/console.h"
#endif

typedef U32 StringStackPtr;
struct StringStack;

/// Helper class which stores a relative pointer in the StringStack buffer
class StringStackPtrRef
{
public:
   StringStackPtrRef() : mOffset(0) {;}
   StringStackPtrRef(StringStackPtr offset) : mOffset(offset) {;}

   StringStackPtr mOffset;

   /// Get pointer to string in stack stk
   inline char *getPtr(StringStack *stk);
};

/// Core stack for interpreter operations.
///
/// This class provides some powerful semantics for working with strings, and is
/// used heavily by the console interpreter.
struct StringStack
{
   enum {
      MaxStackDepth = 1024,
      MaxArgs = 20,
      ReturnBufferSpace = 512
   };
   char *mBuffer;
   U32   mBufferSize;
   const char *mArgV[MaxArgs];
   U32 mFrameOffsets[MaxStackDepth];
   U32 mStartOffsets[MaxStackDepth];

   U32 mNumFrames;
   U32 mArgc;

   U32 mStart;
   U32 mLen;
   U32 mStartStackSize;
   U32 mFunctionOffset;

   U32 mArgBufferSize;
   char *mArgBuffer;

   void validateBufferSize(U32 size)
   {
      if(size > mBufferSize)
      {
         mBufferSize = size + 2048;
         mBuffer = (char *) dRealloc(mBuffer, mBufferSize);
      }
   }

   void validateArgBufferSize(U32 size)
   {
      if(size > mArgBufferSize)
      {
         mArgBufferSize = size + 2048;
         mArgBuffer = (char *) dRealloc(mArgBuffer, mArgBufferSize);
      }
   }

   StringStack()
   {
      mBufferSize = 0;
      mBuffer = NULL;
      mArgBufferSize = 0;
      mArgBuffer = NULL;
      mNumFrames = 0;
      mStart = 0;
      mLen = 0;
      mStartStackSize = 0;
      mFunctionOffset = 0;
      validateBufferSize(8192);
      validateArgBufferSize(2048);
      dMemset(mBuffer, '\0', mBufferSize);
      dMemset(mArgBuffer, '\0', mArgBufferSize);
   }
   ~StringStack()
   {
      if( mBuffer )
         dFree( mBuffer );
      if( mArgBuffer )
         dFree( mArgBuffer );
   }

   /// Set the top of the stack to be an integer value.
   void setIntValue(U32 i)
   {
      validateBufferSize(mStart + 32);
      dSprintf(mBuffer + mStart, 32, "%d", i);
      mLen = dStrlen(mBuffer + mStart);
   }

   /// Set the top of the stack to be a float value.
   void setFloatValue(F64 v)
   {
      validateBufferSize(mStart + 32);
      dSprintf(mBuffer + mStart, 32, "%g", v);
      mLen = dStrlen(mBuffer + mStart);
   }

   /// Return a temporary buffer we can use to return data.
   char* getReturnBuffer(U32 size)
   {
      AssertFatal(Con::isMainThread(), "Manipulating return buffer from a secondary thread!");
      validateArgBufferSize(size);
      return mArgBuffer;
   }

   /// Return a buffer we can use for arguments.
   ///
   /// This updates the function offset.
   char *getArgBuffer(U32 size)
   {
      AssertFatal(Con::isMainThread(), "Manipulating console arg buffer from a secondary thread!");
      validateBufferSize(mStart + mFunctionOffset + size);
      char *ret = mBuffer + mStart + mFunctionOffset;
      mFunctionOffset += size;
      return ret;
   }

   /// Clear the function offset.
   void clearFunctionOffset()
   {
      //Con::printf("StringStack mFunctionOffset = 0 (from %i)", mFunctionOffset);
      mFunctionOffset = 0;
   }

   /// Set a string value on the top of the stack.
   void setStringValue(const char *s)
   {
      if(!s)
      {
         mLen = 0;
         mBuffer[mStart] = 0;
         return;
      }
      mLen = dStrlen(s);

      validateBufferSize(mStart + mLen + 2);
      dStrcpy(mBuffer + mStart, s);
   }

   /// Get the top of the stack, as a StringTableEntry.
   ///
   /// @note Don't free this memory!
   inline StringTableEntry getSTValue()
   {
      return StringTable->insert(mBuffer + mStart);
   }

   /// Get an integer representation of the top of the stack.
   inline U32 getIntValue()
   {
      return dAtoi(mBuffer + mStart);
   }

   /// Get a float representation of the top of the stack.
   inline F64 getFloatValue()
   {
      return dAtof(mBuffer + mStart);
   }

   /// Get a string representation of the top of the stack.
   ///
   /// @note This returns a pointer to the actual top of the stack, be careful!
   inline const char *getStringValue()
   {
      return mBuffer + mStart;
   }

   inline const char *getPreviousStringValue()
   {
      return mBuffer + mStartOffsets[mStartStackSize-1];
   }

   inline StringStackPtr getStringValuePtr()
   {
      return (getStringValue() - mBuffer);
   }

   inline StringStackPtr getPreviousStringValuePtr()
   {
      return (getPreviousStringValue() - mBuffer);
   }

   /// Advance the start stack, placing a zero length string on the top.
   ///
   /// @note You should use StringStack::push, not this, if you want to
   ///       properly push the stack.
   void advance()
   {
      mStartOffsets[mStartStackSize++] = mStart;
      mStart += mLen;
      mLen = 0;
   }

   /// Advance the start stack, placing a single character, null-terminated strong
   /// on the top.
   ///
   /// @note You should use StringStack::push, not this, if you want to
   ///       properly push the stack.
   void advanceChar(char c)
   {
      mStartOffsets[mStartStackSize++] = mStart;
      mStart += mLen;
      mBuffer[mStart] = c;
      mBuffer[mStart+1] = 0;
      mStart += 1;
      mLen = 0;
   }

   /// Push the stack, placing a zero-length string on the top.
   void push()
   {
      advanceChar(0);
   }

   inline void setLen(U32 newlen)
   {
      mLen = newlen;
   }

   /// Pop the start stack.
   void rewind()
   {
      mStart = mStartOffsets[--mStartStackSize];
      mLen = dStrlen(mBuffer + mStart);
   }

   // Terminate the current string, and pop the start stack.
   void rewindTerminate()
   {
      mBuffer[mStart] = 0;
      mStart = mStartOffsets[--mStartStackSize];
      mLen   = dStrlen(mBuffer + mStart);
   }

   /// Compare 1st and 2nd items on stack, consuming them in the process,
   /// and returning true if they matched, false if they didn't.
   U32 compare()
   {
      // Figure out the 1st and 2nd item offsets.
      U32 oldStart = mStart;
      mStart = mStartOffsets[--mStartStackSize];

      // Compare current and previous strings.
      U32 ret = !dStricmp(mBuffer + mStart, mBuffer + oldStart);

      // Put an empty string on the top of the stack.
      mLen = 0;
      mBuffer[mStart] = 0;

      return ret;
   }

   void pushFrame()
   {
      //Con::printf("StringStack pushFrame [frame=%i, start=%i]", mNumFrames, mStartStackSize);
      mFrameOffsets[mNumFrames++] = mStartStackSize;
      mStartOffsets[mStartStackSize++] = mStart;
      mStart += ReturnBufferSpace;
      validateBufferSize(0);
   }

   void popFrame()
   {
      //Con::printf("StringStack popFrame [frame=%i, start=%i]", mNumFrames, mStartStackSize);
      mStartStackSize = mFrameOffsets[--mNumFrames];
      mStart = mStartOffsets[mStartStackSize];
      mLen = 0;
   }

   void clearFrames()
   {
      //Con::printf("StringStack clearFrames");
      mNumFrames = 0;
      mStart = 0;
      mLen = 0;
      mStartStackSize = 0;
      mFunctionOffset = 0;
   }

   /// Get the arguments for a function call from the stack.
   void getArgcArgv(StringTableEntry name, U32 *argc, const char ***in_argv, bool popStackFrame = false);
};


// New console value stack
class ConsoleValueStack
{
   enum {
      MaxStackDepth = 1024,
      MaxArgs = 20,
      ReturnBufferSpace = 512
   };

public:
   ConsoleValueStack();
   ~ConsoleValueStack();

   void pushVar(ConsoleValue *variable);
   void pushValue(ConsoleValue &value);
   ConsoleValue* reserveValues(U32 numValues);
   bool reserveValues(U32 numValues, ConsoleValueRef *values);
   ConsoleValue* pop();

   ConsoleValue *pushString(const char *value);
   ConsoleValue *pushStackString(const char *value);
   ConsoleValue *pushStringStackPtr(StringStackPtr ptr);
   ConsoleValue *pushUINT(U32 value);
   ConsoleValue *pushFLT(float value);

   void pushFrame();
   void popFrame();

   void resetFrame();
   void clearFrames();

   void getArgcArgv(StringTableEntry name, U32 *argc, ConsoleValueRef **in_argv, bool popStackFrame = false);

   ConsoleValue mStack[MaxStackDepth];
   U32 mStackFrames[MaxStackDepth];

   U32 mFrame;
   U32 mStackPos;

   ConsoleValueRef mArgv[MaxArgs];
};

extern StringStack STR;
extern ConsoleValueStack CSTK;

inline char* StringStackPtrRef::getPtr(StringStack *stk) { return stk->mBuffer + mOffset; }

#endif
