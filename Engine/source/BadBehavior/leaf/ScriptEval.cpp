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

#include "ScriptEval.h"

using namespace BadBehavior;

//------------------------------------------------------------------------------
// ScriptEval node - warning, slow!
//------------------------------------------------------------------------------
IMPLEMENT_CONOBJECT(ScriptEval);

ScriptEval::ScriptEval()
   : mDefaultReturnStatus(SUCCESS) 
{
}

void ScriptEval::initPersistFields()
{
   addGroup( "Behavior" );

   addField( "behaviorScript", TypeCommand, Offset(mBehaviorScript, ScriptEval),
      "@brief The command to execute when the leaf is ticked.  Max 255 characters." );

   addField( "defaultReturnStatus", TYPEID< BadBehavior::Status >(), Offset(mDefaultReturnStatus, ScriptEval),
      "@brief The default value for this node to return.");

   endGroup( "Behavior" );

   Parent::initPersistFields();
}

Task *ScriptEval::createTask(SimObject &owner, BehaviorTreeRunner &runner)
{
   return new ScriptEvalTask(*this, owner, runner);
}

Status ScriptEval::evaluateScript( SimObject *owner )
{
   PROFILE_SCOPE(ScriptEval_evaluateScript);

   if(mBehaviorScript.isEmpty())
      return mDefaultReturnStatus;

   // get the result
   const char *result = Con::evaluatef("%%obj=%s; %s return %s;", 
                                       owner->getIdString(),
                                       mBehaviorScript.c_str(),
                                       EngineMarshallData< BehaviorReturnType >(mDefaultReturnStatus));
   
   if(result[0] == '1' || result[0] == '0')
      // map true or false to SUCCEED or FAILURE
      return static_cast<Status>(dAtoi(result));

   // convert the returned value to our internal enum type
   return EngineUnmarshallData< BehaviorReturnType >()( result );
}

//------------------------------------------------------------------------------
// RunScript task
//------------------------------------------------------------------------------
ScriptEvalTask::ScriptEvalTask(Node &node, SimObject &owner, BehaviorTreeRunner &runner)
   : Parent(node, owner, runner)
{
}

Task* ScriptEvalTask::update()
{
   mStatus = static_cast<ScriptEval*>(mNodeRep)->evaluateScript( mOwner );

   return NULL; // leaves don't have children
}