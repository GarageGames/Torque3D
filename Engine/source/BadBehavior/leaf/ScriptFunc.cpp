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

#include "console/engineAPI.h"
#include "platform/profiler.h"

#include "ScriptFunc.h"

using namespace BadBehavior;

//------------------------------------------------------------------------------
// ScriptFunc node
//------------------------------------------------------------------------------
IMPLEMENT_CONOBJECT(ScriptFunc);

ScriptFunc::ScriptFunc()
   : mDefaultReturnStatus(SUCCESS),
     mScriptFunction(StringTable->insert(""))
{
   for(U8 i = 0; i < MAX_COMMAND_ARGS; ++i)
      mScriptArgs[i] = StringTable->insert("");
}


void ScriptFunc::initPersistFields()
{
   addGroup( "Behavior" );

   addField( "func", TypeCaseString, Offset(mScriptFunction, ScriptFunc),
      "@brief The function to execute when the leaf is ticked." );

   addField( "args", TypeCaseString, Offset(mScriptArgs, ScriptFunc), MAX_COMMAND_ARGS,
      "@brief The arguments to be passed to the function to be executed." );

   addField( "defaultReturnStatus", TYPEID< BadBehavior::Status >(), Offset(mDefaultReturnStatus, ScriptFunc),
      "@brief The default value for this node to return.");

   endGroup( "Behavior" );

   Parent::initPersistFields();
}

Task *ScriptFunc::createTask(SimObject &owner, BehaviorTreeRunner &runner)
{
   return new ScriptFuncTask(*this, owner, runner);
}

Status ScriptFunc::evaluate( SimObject *owner )
{
   PROFILE_SCOPE(ScriptFunc_evaluate);

   if(!owner)
      return INVALID;

   if(!mScriptFunction || !mScriptFunction[0])
      return mDefaultReturnStatus;

   S32 argc = 0;

   const char *args[MAX_COMMAND_ARGS + 2];
   args[0] = mScriptFunction;

   while(argc < MAX_COMMAND_ARGS && (mScriptArgs[argc] && mScriptArgs[argc][0]))
   {
      args[argc + 2] = mScriptArgs[argc];
      ++argc;
   }

   argc += 2;

   // get the result
   const char *result = Con::execute(owner, argc, args);

   // if function didn't return a result, return our default status
   if(!result || !result[0])
      return mDefaultReturnStatus;
   
   // map true or false to SUCCEED or FAILURE
   if(result[0] == '1' || result[0] == '0') 
      return static_cast<Status>(dAtoi(result));
   
   // convert the returned value to our internal enum type
   return EngineUnmarshallData< BehaviorReturnType >()( result );
}

//------------------------------------------------------------------------------
// ScriptFunc task
//------------------------------------------------------------------------------
ScriptFuncTask::ScriptFuncTask(Node &node, SimObject &owner, BehaviorTreeRunner &runner)
   : Parent(node, owner, runner)
{
}

Task* ScriptFuncTask::update()
{
   mStatus = static_cast<ScriptFunc*>(mNodeRep)->evaluate( mOwner );

   if(mStatus != RUNNING && mStatus != SUSPENDED)
      mIsComplete = true;

   return NULL; // leaves don't have children
}