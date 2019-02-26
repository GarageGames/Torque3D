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

#include <stdio.h>
#include "console/consoleInternal.h"
#include "console/stringStack.h"

StringStack::StringStack()
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

StringStack::~StringStack()
{
   if( mBuffer )
      dFree( mBuffer );
   if( mArgBuffer )
      dFree( mArgBuffer );
}

void StringStack::validateBufferSize(U32 size)
{
   if(size > mBufferSize)
   {
      mBufferSize = size + 2048;
      mBuffer = (char *) dRealloc(mBuffer, mBufferSize);
   }
}

void StringStack::validateArgBufferSize(U32 size)
{
   if(size > mArgBufferSize)
   {
      mArgBufferSize = size + 2048;
      mArgBuffer = (char *) dRealloc(mArgBuffer, mArgBufferSize);
   }
}

void StringStack::setIntValue(U32 i)
{
   validateBufferSize(mStart + 32);
   dSprintf(mBuffer + mStart, 32, "%d", i);
   mLen = dStrlen(mBuffer + mStart);
}

void StringStack::setFloatValue(F64 v)
{
   validateBufferSize(mStart + 32);
   dSprintf(mBuffer + mStart, 32, "%g", v);
   mLen = dStrlen(mBuffer + mStart);
}

char *StringStack::getReturnBuffer(U32 size)
{
   if(size > ReturnBufferSpace)
   {
      AssertFatal(Con::isMainThread(), "Manipulating return buffer from a secondary thread!");
      validateArgBufferSize(size);
      return mArgBuffer;
   }
   else
   {
      validateBufferSize(mStart + size);
      return mBuffer + mStart;
   }
}

char *StringStack::getArgBuffer(U32 size)
{
   AssertFatal(Con::isMainThread(), "Manipulating console arg buffer from a secondary thread!");
   validateBufferSize(mStart + mFunctionOffset + size);
   char *ret = mBuffer + mStart + mFunctionOffset;
   mFunctionOffset += size;
   return ret;
}

void StringStack::clearFunctionOffset()
{
   //Con::printf("StringStack mFunctionOffset = 0 (from %i)", mFunctionOffset);
   mFunctionOffset = 0;
}

void StringStack::setStringValue(const char *s)
{
   if(!s)
   {
      mLen = 0;
      mBuffer[mStart] = 0;
      return;
   }
   mLen = dStrlen(s);

   validateBufferSize(mStart + mLen + 2);
   dStrcpy(mBuffer + mStart, s, mBufferSize - mStart);
}

void StringStack::advance()
{
   mStartOffsets[mStartStackSize++] = mStart;
   mStart += mLen;
   mLen = 0;
}

void StringStack::advanceChar(char c)
{
   mStartOffsets[mStartStackSize++] = mStart;
   mStart += mLen;
   mBuffer[mStart] = c;
   mBuffer[mStart+1] = 0;
   mStart += 1;
   mLen = 0;
}

void StringStack::push()
{
   advanceChar(0);
}

void StringStack::rewind()
{
   mStart = mStartOffsets[--mStartStackSize];
   mLen = dStrlen(mBuffer + mStart);
}

void StringStack::rewindTerminate()
{
   mBuffer[mStart] = 0;
   mStart = mStartOffsets[--mStartStackSize];
   mLen   = dStrlen(mBuffer + mStart);
}

U32 StringStack::compare()
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

void StringStack::pushFrame()
{
   //Con::printf("StringStack pushFrame [frame=%i, start=%i]", mNumFrames, mStartStackSize);
   mFrameOffsets[mNumFrames++] = mStartStackSize;
   mStartOffsets[mStartStackSize++] = mStart;
   mStart += ReturnBufferSpace;
   validateBufferSize(0);
}

void StringStack::popFrame()
{
   //Con::printf("StringStack popFrame [frame=%i, start=%i]", mNumFrames, mStartStackSize);
   mStartStackSize = mFrameOffsets[--mNumFrames];
   mStart = mStartOffsets[mStartStackSize];
   mLen = 0;
}

void StringStack::clearFrames()
{
   //Con::printf("StringStack clearFrames");
   mNumFrames = 0;
   mStart = 0;
   mLen = 0;
   mStartStackSize = 0;
   mFunctionOffset = 0;
}


void ConsoleValueStack::getArgcArgv(StringTableEntry name, U32 *argc, ConsoleValueRef **in_argv, bool popStackFrame /* = false */)
{
   U32 startStack = mStackFrames[mFrame-1];
   U32 argCount   = getMin(mStackPos - startStack, (U32)MaxArgs - 1);

   *in_argv = mArgv;
   mArgv[0].value = CSTK.pushStackString(name);
   
   for(U32 i = 0; i < argCount; i++) {
      ConsoleValueRef *ref = &mArgv[i+1];
      ref->value = &mStack[startStack + i];
   }
   argCount++;
   
   *argc = argCount;

   if(popStackFrame)
      popFrame();
}

ConsoleValueStack::ConsoleValueStack() : 
mFrame(0),
mStackPos(0)
{
   for (int i=0; i<ConsoleValueStack::MaxStackDepth; i++) {
      mStack[i].init();
      mStack[i].type = ConsoleValue::TypeInternalString;
   }
}

ConsoleValueStack::~ConsoleValueStack()
{
}

void ConsoleValueStack::pushVar(ConsoleValue *variable)
{
   if (mStackPos == ConsoleValueStack::MaxStackDepth) {
      AssertFatal(false, "Console Value Stack is empty");
      return;
   }

   switch (variable->type)
   {
   case ConsoleValue::TypeInternalInt:
      mStack[mStackPos++].setIntValue((S32)variable->getIntValue());
   case ConsoleValue::TypeInternalFloat:
      mStack[mStackPos++].setFloatValue((F32)variable->getFloatValue());
   default:
      mStack[mStackPos++].setStackStringValue(variable->getStringValue());
   }
}

void ConsoleValueStack::pushValue(ConsoleValue &variable)
{
   if (mStackPos == ConsoleValueStack::MaxStackDepth) {
      AssertFatal(false, "Console Value Stack is empty");
      return;
   }

   switch (variable.type)
   {
   case ConsoleValue::TypeInternalInt:
      mStack[mStackPos++].setIntValue((S32)variable.getIntValue());
   case ConsoleValue::TypeInternalFloat:
      mStack[mStackPos++].setFloatValue((F32)variable.getFloatValue());
   case ConsoleValue::TypeInternalStringStackPtr:
      mStack[mStackPos++].setStringStackPtrValue(variable.getStringStackPtr());
   default:
      mStack[mStackPos++].setStringValue(variable.getStringValue());
   }
}

ConsoleValue* ConsoleValueStack::reserveValues(U32 count)
{
   U32 startPos = mStackPos;
   if (startPos+count >= ConsoleValueStack::MaxStackDepth) {
      AssertFatal(false, "Console Value Stack is empty");
      return NULL;
   }

   //Con::printf("[%i]CSTK reserveValues %i", mStackPos, count);
   mStackPos += count;
   return &mStack[startPos];
}

bool ConsoleValueStack::reserveValues(U32 count, ConsoleValueRef *outValues)
{
   U32 startPos = mStackPos;
   if (startPos+count >= ConsoleValueStack::MaxStackDepth) {
      AssertFatal(false, "Console Value Stack is empty");
      return false;
   }

   //Con::printf("[%i]CSTK reserveValues %i", mStackPos, count);
   for (U32 i=0; i<count; i++)
   {
      outValues[i].value = &mStack[mStackPos+i];
   }
   mStackPos += count;
   return true;
}

ConsoleValue *ConsoleValueStack::pushString(const char *value)
{
   if (mStackPos == ConsoleValueStack::MaxStackDepth) {
      AssertFatal(false, "Console Value Stack is empty");
      return NULL;
   }

   //Con::printf("[%i]CSTK pushString %s", mStackPos, value);

   mStack[mStackPos++].setStringValue(value);
   return &mStack[mStackPos-1];
}

ConsoleValue *ConsoleValueStack::pushStackString(const char *value)
{
   if (mStackPos == ConsoleValueStack::MaxStackDepth) {
      AssertFatal(false, "Console Value Stack is empty");
      return NULL;
   }

   //Con::printf("[%i]CSTK pushString %s", mStackPos, value);

   mStack[mStackPos++].setStackStringValue(value);
   return &mStack[mStackPos-1];
}

ConsoleValue *ConsoleValueStack::pushStringStackPtr(StringStackPtr value)
{
   if (mStackPos == ConsoleValueStack::MaxStackDepth) {
      AssertFatal(false, "Console Value Stack is empty");
      return NULL;
   }

   //Con::printf("[%i]CSTK pushStringStackPtr %s", mStackPos, StringStackPtrRef(value).getPtr(&STR));

   mStack[mStackPos++].setStringStackPtrValue(value);
   return &mStack[mStackPos-1];
}

ConsoleValue *ConsoleValueStack::pushUINT(U32 value)
{
   if (mStackPos == ConsoleValueStack::MaxStackDepth) {
      AssertFatal(false, "Console Value Stack is empty");
      return NULL;
   }

   //Con::printf("[%i]CSTK pushUINT %i", mStackPos, value);

   mStack[mStackPos++].setIntValue(value);
   return &mStack[mStackPos-1];
}

ConsoleValue *ConsoleValueStack::pushFLT(float value)
{
   if (mStackPos == ConsoleValueStack::MaxStackDepth) {
      AssertFatal(false, "Console Value Stack is empty");
      return NULL;
   }

   //Con::printf("[%i]CSTK pushFLT %f", mStackPos, value);

   mStack[mStackPos++].setFloatValue(value);
   return &mStack[mStackPos-1];
}

static ConsoleValue gNothing;

ConsoleValue* ConsoleValueStack::pop()
{
   if (mStackPos == 0) {
      AssertFatal(false, "Console Value Stack is empty");
      return &gNothing;
   }

   return &mStack[--mStackPos];
}

void ConsoleValueStack::pushFrame()
{
   //Con::printf("CSTK pushFrame[%i] (%i)", mFrame, mStackPos);
   mStackFrames[mFrame++] = mStackPos;
}

void ConsoleValueStack::resetFrame()
{
   if (mFrame == 0) {
      mStackPos = 0;
      return;
   }

   U32 start = mStackFrames[mFrame-1];
   //for (U32 i=start; i<mStackPos; i++) {
      //mStack[i].clear();
   //}
   mStackPos = start;
   //Con::printf("CSTK resetFrame to %i", mStackPos);
}

void ConsoleValueStack::clearFrames()
{
   mStackPos = 0;
   mFrame = 0;
}

void ConsoleValueStack::popFrame()
{
   //Con::printf("CSTK popFrame");
   if (mFrame == 0) {
      // Go back to start
      mStackPos = 0;
      return;
   }

   U32 start = mStackFrames[mFrame-1];
   //for (U32 i=start; i<mStackPos; i++) {
      //mStack[i].clear();
   //}
   mStackPos = start;
   mFrame--;
}
