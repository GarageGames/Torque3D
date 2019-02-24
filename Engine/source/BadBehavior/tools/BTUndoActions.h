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

#ifndef _BT_UNDO_ACTIONS_H_
#define _BT_UNDO_ACTIONS_H_

#ifndef _UNDO_H_
#include "util/undo.h"
#endif
#ifndef _CONSOLE_SIMOBJECTMEMENTO_H_
#include "console/simObjectMemento.h"
#endif

S32 getNextObjectInSet(SimObject *obj, SimGroup &group);

class BTDeleteUndoAction : public UndoAction
{
   typedef UndoAction Parent;

protected:

   struct ObjectState
   {
      /// The object we deleted and will restore in undo.
      SimObjectId id;

      /// The captured object state.
      SimObjectMemento memento;

      /// Keep track of the parent group.
      SimObjectId groupId;

      /// Keep track of the position within the parent group
      SimObjectId nextId;
   };

   /// The object we're deleting.
   ObjectState mObject;

public:

   DECLARE_CONOBJECT( BTDeleteUndoAction );
   static void initPersistFields();
   
   BTDeleteUndoAction( const UTF8* actionName = "Delete Object" );
   virtual ~BTDeleteUndoAction();

   ///
   void deleteObject( SimObject *object );
   
   // UndoAction
   virtual void undo();
   virtual void redo();
};



#endif // _BT_UNDO_ACTIONS_H_