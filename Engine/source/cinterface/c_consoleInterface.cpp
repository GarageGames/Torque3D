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

#include "console/consoleInternal.h"
#include "console/simSet.h"
#include "console/engineAPI.h"

namespace Con
{
   DefineNewEngineFunction(AddConsumer, void, (ConsumerCallback cb), , "")
   {
      addConsumer(cb);
   }

   DefineNewEngineFunction(RemoveConsumer, void, (ConsumerCallback cb), , "")
   {
      removeConsumer(cb);
   }

   DefineNewEngineFunction(GetConsoleString, String, (String name),, "")
   {
      return getVariable(StringTable->insert(name));
   }

   DefineNewEngineFunction(SetConsoleString, void, (String name, String value),, "")
   {
      setVariable(StringTable->insert(name), StringTable->insert(value));
   }

   DefineNewEngineFunction(GetConsoleInt, S32, (String name),, "")
   {
      return getIntVariable(StringTable->insert(name));
   }

   DefineNewEngineFunction(SetConsoleInt, void, (String name, S32 value),, "")
   {
      setIntVariable(StringTable->insert(name), value);
   }

   DefineNewEngineFunction(GetConsoleFloat, F32, (String name),, "")
   {
      return getFloatVariable(StringTable->insert(name));
   }

   DefineNewEngineFunction(SetConsoleFloat, void, (String name, F32 value),, "")
   {
      setFloatVariable(StringTable->insert(name), value);
   }

   DefineNewEngineFunction(GetConsoleBool, bool, (String name),, "")
   {
      return getBoolVariable(StringTable->insert(name));
   }

   DefineNewEngineFunction(SetConsoleBool, void, (String name, bool value),, "")
   {
      setBoolVariable(StringTable->insert(name), value);
   }
}