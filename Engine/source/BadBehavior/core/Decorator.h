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

#ifndef _BB_DECORATOR_H_
#define _BB_DECORATOR_H_

#ifndef _BB_CORE_H_
#include "BadBehavior/core/Core.h"
#endif

namespace BadBehavior
{
   //---------------------------------------------------------------------------
   // Decorator node base class
   //---------------------------------------------------------------------------
   class DecoratorNode : public Node
   {
      typedef Node Parent;

   public:
      // only allow 1 child node to be added
      virtual void addObject(SimObject *obj);
      virtual bool acceptsAsChild( SimObject *object ) const;
   };

   //---------------------------------------------------------------------------
   // Decorator task base class
   //---------------------------------------------------------------------------
   class DecoratorTask : public Task
   {
      typedef Task Parent;

   protected:
      Task* mChild;

      virtual Task* update();
      virtual void onInitialize();
      virtual void onTerminate();

   public:
      DecoratorTask(Node &node, SimObject &owner, BehaviorTreeRunner &runner);
      virtual ~DecoratorTask();

      virtual void onChildComplete(Status s);
   };
   
} // namespace BadBehavior

#endif
