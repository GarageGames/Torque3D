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

#include "ScriptedBehavior.h"

using namespace BadBehavior;

//------------------------------------------------------------------------------
// ScriptedBehavior node
//------------------------------------------------------------------------------
IMPLEMENT_CONOBJECT(ScriptedBehavior);

IMPLEMENT_CALLBACK( ScriptedBehavior, onEnter, void, ( SimObject* owner ), ( owner ),
   "Called when the behavior is first run.\n"
   "@param owner The object that this behavior tree belongs to." );

IMPLEMENT_CALLBACK( ScriptedBehavior, onExit, void, ( SimObject* owner ), ( owner ),
   "Called when the behavior is complete.\n"
   "@param owner The object that this behavior tree belongs to." );

IMPLEMENT_CALLBACK( ScriptedBehavior, precondition, bool, ( SimObject* owner ), ( owner ),
   "Called prior to evaluating the behavior.\n"
   "@param owner The object that this behavior tree belongs to.\n"
   "A return value of false indicates that evaluation of the behavior should stop.\n"
   "A return value of true indicates that evaluation of the behavior should continue.");

IMPLEMENT_CALLBACK( ScriptedBehavior, behavior, Status, ( SimObject* owner ), ( owner ),
   "Evaluate the main behavior of this node.\n"
   "@param owner The object that this behavior tree belongs to.\n");

ScriptedBehavior::ScriptedBehavior()
{
}

bool ScriptedBehavior::precondition( SimObject *owner )
{
   PROFILE_SCOPE( ScriptedBehavior_precondition);

   if(isMethod("precondition"))
      return precondition_callback(owner);

   return true;
}

void ScriptedBehavior::onEnter( SimObject *owner )
{
   PROFILE_SCOPE( ScriptedBehavior_onEnter );
   onEnter_callback(owner);
}

void ScriptedBehavior::onExit( SimObject *owner )
{
   PROFILE_SCOPE( ScriptedBehavior_onExit );
   onExit_callback(owner);
}

Status ScriptedBehavior::behavior( SimObject *owner )
{
   PROFILE_SCOPE( ScriptedBehavior_behavior );   
   return behavior_callback( owner );
}