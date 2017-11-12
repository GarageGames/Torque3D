//-----------------------------------------------------------------------------
// Copyright (c) 2014 Guy Allard
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

#ifndef _BB_SCRIPTFUNC_H_
#define _BB_SCRIPTFUNC_H_

#ifndef _BB_CORE_H_
#include "BadBehavior/core/Core.h"
#endif

namespace BadBehavior
{
   static const U8 MAX_COMMAND_ARGS = 4;

   //---------------------------------------------------------------------------
   // ScriptFunc - executes a function on the owner object
   //---------------------------------------------------------------------------
   class ScriptFunc : public LeafNode
   {
      typedef LeafNode Parent;

   protected:
      // the function to call
      StringTableEntry mScriptFunction;
      
      // the arguments for the function
      StringTableEntry mScriptArgs[MAX_COMMAND_ARGS];

      // status to return if the command does not return a value
      Status mDefaultReturnStatus;

   public:
      ScriptFunc();

      virtual Task *createTask(SimObject &owner, BehaviorTreeRunner &runner);
      
      static void initPersistFields();

      // execute the command
      Status evaluate(SimObject *owner);

      DECLARE_CONOBJECT(ScriptFunc);
   };

   //---------------------------------------------------------------------------
   // ScriptFunc task
   //---------------------------------------------------------------------------
   class ScriptFuncTask : public Task
   {
      typedef Task Parent;
   
   protected:
      virtual Task* update();
        
   public:
      ScriptFuncTask(Node &node, SimObject &owner, BehaviorTreeRunner &runner);
   };

} // namespace BadBehavior
#endif