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
