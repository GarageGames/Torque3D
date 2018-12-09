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
#include "console/console.h"

#include "console/ast.h"
#include "core/tAlgorithm.h"

#include "core/strings/findMatch.h"
#include "core/strings/stringUnit.h"
#include "console/consoleInternal.h"
#include "core/stream/fileStream.h"
#include "console/compiler.h"

#include "console/simBase.h"
#include "console/telnetDebugger.h"
#include "sim/netStringTable.h"
#include "console/ICallMethod.h"
#include "console/stringStack.h"
#include "util/messaging/message.h"
#include "core/frameAllocator.h"

#include "console/codeInterpreter.h"
#include "console/returnBuffer.h"

#ifndef TORQUE_TGB_ONLY
#include "materials/materialDefinition.h"
#include "materials/materialManager.h"
#endif

using namespace Compiler;

namespace Con
{
   // Current script file name and root, these are registered as
   // console variables.
   extern StringTableEntry gCurrentFile;
   extern StringTableEntry gCurrentRoot;
   extern S32 gObjectCopyFailures;
}

namespace Con
{
   const char *getNamespaceList(Namespace *ns)
   {
      U32 size = 1;
      Namespace * walk;
      for (walk = ns; walk; walk = walk->mParent)
         size += dStrlen(walk->mName) + 4;
      char *ret = Con::getReturnBuffer(size);
      ret[0] = 0;
      for (walk = ns; walk; walk = walk->mParent)
      {
         dStrcat(ret, walk->mName, size);
         if (walk->mParent)
            dStrcat(ret, " -> ", size);
      }
      return ret;
   }
}

//------------------------------------------------------------

F64 consoleStringToNumber(const char *str, StringTableEntry file, U32 line)
{
   F64 val = dAtof(str);
   if (val != 0)
      return val;
   else if (!dStricmp(str, "true"))
      return 1;
   else if (!dStricmp(str, "false"))
      return 0;
   else if (file)
   {
      Con::warnf(ConsoleLogEntry::General, "%s (%d): string always evaluates to 0.", file, line);
      return 0;
   }
   return 0;
}

//------------------------------------------------------------

namespace Con
{
   ReturnBuffer retBuffer;

   char *getReturnBuffer(U32 bufferSize)
   {
      return retBuffer.getBuffer(bufferSize);
   }

   char *getReturnBuffer(const char *stringToCopy)
   {
      U32 len = dStrlen(stringToCopy) + 1;
      char *ret = retBuffer.getBuffer(len);
      dMemcpy(ret, stringToCopy, len);
      return ret;
   }

   char* getReturnBuffer(const String& str)
   {
      const U32 size = str.size();
      char* ret = retBuffer.getBuffer(size);
      dMemcpy(ret, str.c_str(), size);
      return ret;
   }

   char* getReturnBuffer(const StringBuilder& str)
   {
      char* buffer = Con::getReturnBuffer(str.length() + 1);
      str.copy(buffer);
      buffer[str.length()] = '\0';

      return buffer;
   }

   char *getArgBuffer(U32 bufferSize)
   {
      return STR.getArgBuffer(bufferSize);
   }

   char *getFloatArg(F64 arg)
   {
      char *ret = STR.getArgBuffer(32);
      dSprintf(ret, 32, "%g", arg);
      return ret;
   }

   char *getIntArg(S32 arg)
   {
      char *ret = STR.getArgBuffer(32);
      dSprintf(ret, 32, "%d", arg);
      return ret;
   }

   char* getBoolArg(bool arg)
   {
      char *ret = STR.getArgBuffer(32);
      dSprintf(ret, 32, "%d", arg);
      return ret;
   }

   char *getStringArg(const char *arg)
   {
      U32 len = dStrlen(arg) + 1;
      char *ret = STR.getArgBuffer(len);
      dMemcpy(ret, arg, len);
      return ret;
   }

   char* getStringArg(const String& arg)
   {
      const U32 size = arg.size();
      char* ret = STR.getArgBuffer(size);
      dMemcpy(ret, arg.c_str(), size);
      return ret;
   }
}

//------------------------------------------------------------

void ExprEvalState::setCurVarName(StringTableEntry name)
{
   if (name[0] == '$')
      currentVariable = globalVars.lookup(name);
   else if (getStackDepth() > 0)
      currentVariable = getCurrentFrame().lookup(name);
   if (!currentVariable && gWarnUndefinedScriptVariables)
      Con::warnf(ConsoleLogEntry::Script, "Variable referenced before assignment: %s", name);
}

void ExprEvalState::setCurVarNameCreate(StringTableEntry name)
{
   if (name[0] == '$')
      currentVariable = globalVars.add(name);
   else if (getStackDepth() > 0)
      currentVariable = getCurrentFrame().add(name);
   else
   {
      currentVariable = NULL;
      Con::warnf(ConsoleLogEntry::Script, "Accessing local variable in global scope... failed: %s", name);
   }
}

//------------------------------------------------------------

S32 ExprEvalState::getIntVariable()
{
   return currentVariable ? currentVariable->getIntValue() : 0;
}

F64 ExprEvalState::getFloatVariable()
{
   return currentVariable ? currentVariable->getFloatValue() : 0;
}

const char *ExprEvalState::getStringVariable()
{
   return currentVariable ? currentVariable->getStringValue() : "";
}

//------------------------------------------------------------

void ExprEvalState::setIntVariable(S32 val)
{
   AssertFatal(currentVariable != NULL, "Invalid evaluator state - trying to set null variable!");
   currentVariable->setIntValue(val);
}

void ExprEvalState::setFloatVariable(F64 val)
{
   AssertFatal(currentVariable != NULL, "Invalid evaluator state - trying to set null variable!");
   currentVariable->setFloatValue(val);
}

void ExprEvalState::setStringVariable(const char *val)
{
   AssertFatal(currentVariable != NULL, "Invalid evaluator state - trying to set null variable!");
   currentVariable->setStringValue(val);
}

void ExprEvalState::setStringStackPtrVariable(StringStackPtr str)
{
   AssertFatal(currentVariable != NULL, "Invalid evaluator state - trying to set null variable!");
   currentVariable->setStringStackPtrValue(str);
}

void ExprEvalState::setCopyVariable()
{
   if (copyVariable)
   {
      switch (copyVariable->value.type)
      {
         case ConsoleValue::TypeInternalInt:
            currentVariable->setIntValue(copyVariable->getIntValue());
            break;
         case ConsoleValue::TypeInternalFloat:
            currentVariable->setFloatValue(copyVariable->getFloatValue());
            break;
         default:
            currentVariable->setStringValue(copyVariable->getStringValue());
            break;
      }
   }
}

//------------------------------------------------------------


ConsoleValueRef CodeBlock::exec(U32 ip, const char *functionName, Namespace *thisNamespace, U32 argc, ConsoleValueRef *argv, bool noCalls, StringTableEntry packageName, S32 setFrame)
{
   CodeInterpreter interpreter(this);
   return interpreter.exec(ip, functionName, thisNamespace, argc, argv, noCalls, packageName, setFrame);
}

//------------------------------------------------------------
