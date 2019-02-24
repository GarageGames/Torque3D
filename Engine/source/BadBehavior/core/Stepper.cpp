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

#include "Stepper.h"

using namespace BadBehavior;

Status BehaviorTreeStepper::stepThrough(VectorPtr<Task *> &taskVector) 
{
   if(taskVector.empty()) return INVALID;
         
   if(taskVector.back()->getStatus() == SUSPENDED)
      return SUSPENDED;

   Status status = INVALID;

   // loop through the tasks in the task list
   while(!taskVector.empty())
   {
      // get a task
      Task *currentTask = taskVector.back();

      // tick the task
      Task *nextTask = currentTask->tick();
      
      // if task returned no children, it has completed
      if(!nextTask)
      {
         // stop if it's RUNNING or SUSPENED
         status = currentTask->getStatus();
         if(status == RUNNING || status == SUSPENDED)
            break;
               
         // otherwise, remove it from the list
         taskVector.pop_back();
         if(!taskVector.empty())
            // and tell its parent that it completed
            taskVector.back()->onChildComplete(currentTask->getStatus());

         // complete the task
         currentTask->finish();
      }
      else
      {
         // add the child as a task
         nextTask->setup();
         taskVector.push_back(nextTask);
      }
   }

   return status;
}