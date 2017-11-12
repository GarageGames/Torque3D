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

#include "BTUndoActions.h"
#include "console/consoleTypes.h"
#include "console/simSet.h"

S32 getNextObjectInGroup(SimObject *object, SimGroup *group)
{
   group->lock();
   S32 nextId = -1;
   
   if(object != group->last() && group->find( group->begin(), group->end(), object ) != group->end())
   {
      for( SimSet::iterator i = group->begin(); i != group->end(); i++)
      {
         if( *i == object )
         {
            nextId = (*++i)->getId();
            break;
         }
      }
      group->unlock();
   }

   return nextId;
}

IMPLEMENT_CONOBJECT( BTDeleteUndoAction );

ConsoleDocClass( BTDeleteUndoAction,
				"@brief Behavior Tree Editor delete undo instance\n\n"
				"Not intended for game development, for editors or internal use only.\n\n "
				"@internal");

BTDeleteUndoAction::BTDeleteUndoAction( const UTF8 *actionName )
   :  UndoAction( actionName )
{
}

BTDeleteUndoAction::~BTDeleteUndoAction()
{
}

void BTDeleteUndoAction::initPersistFields()
{
   Parent::initPersistFields();
}

void BTDeleteUndoAction::deleteObject( SimObject *object )
{
   AssertFatal( object, "BTDeleteUndoAction::deleteObject() - Got null object!" );
   AssertFatal( object->isProperlyAdded(), 
      "BTDeleteUndoAction::deleteObject() - Object should be registered!" );

   // Capture the object id.
   mObject.id = object->getId();

   // Save the state.
   mObject.memento.save( object );

   // Store the group.
   SimGroup *group = object->getGroup();
   if ( group )
   {
      mObject.groupId = group->getId();

      // and the next object in the group
      mObject.nextId = getNextObjectInGroup(object, group);
   }
   
   // Now delete the object.
   object->deleteObject();
}

ConsoleMethod( BTDeleteUndoAction, deleteObject, void, 3, 3, "( SimObject obj )")
{
   SimObject *obj = NULL;
   if ( Sim::findObject( argv[2], obj ) && obj )
   	object->deleteObject( obj );
}

void BTDeleteUndoAction::undo()
{
   // Create the object.
   SimObject::setForcedId(mObject.id); // Restore the object's Id
   SimObject *object = mObject.memento.restore();
   if ( !object )
      return;

   // Now restore its group.
   SimGroup *group;
   if ( Sim::findObject( mObject.groupId, group ) )
   {
      group->addObject( object );

      // restore its position in the group
      SimObject *nextObj;
      if ( Sim::findObject( mObject.nextId, nextObj ) )
      {
         group->reOrder(object, nextObj);
      }
   }
   
   Con::executef( this, "onUndone" );
}

void BTDeleteUndoAction::redo()
{
   SimObject *object = Sim::findObject( mObject.id );
   if ( object )
      object->deleteObject();
   
   Con::executef( this, "onRedone" );
}
