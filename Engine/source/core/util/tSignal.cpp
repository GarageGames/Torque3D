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
#include "core/util/tSignal.h"


void SignalBase::DelegateLink::insert(DelegateLink* node, float order)
{
   // Note: can only legitimately be called on list head
   DelegateLink * walk = next;
   while (order >= walk->order && walk->next != this)
      walk = walk->next;
   if (order >= walk->order)
   {
      // insert after walk
      node->prev = walk;
      node->next = walk->next;
      walk->next->prev = node;
      walk->next = node;
   }
   else
   {
      // insert before walk
      node->prev = walk->prev;
      node->next = walk;
      walk->prev->next = node;
      walk->prev = node;
   }
   node->order = order;
}

void SignalBase::DelegateLink::unlink()
{
   // Unlink this node
   next->prev = prev;
   prev->next = next;
}

SignalBase::~SignalBase()
{
   removeAll();
}

void SignalBase::removeAll()
{
   while (mList.next != &mList)
   {
      DelegateLink* ptr = mList.next;
      ptr->unlink();
      delete ptr;
   }
}
