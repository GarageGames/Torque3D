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
#include "util/undo.h"

#include "console/console.h"
#include "console/consoleTypes.h"

//-----------------------------------------------------------------------------
// UndoAction
//-----------------------------------------------------------------------------
IMPLEMENT_CONOBJECT(UndoAction);
IMPLEMENT_CONOBJECT(UndoScriptAction);

ConsoleDocClass( UndoAction,
				"@brief An event which signals the editors to undo the last action\n\n"
				"Not intended for game development, for editors or internal use only.\n\n "
				"@internal");

ConsoleDocClass( UndoScriptAction,
				"@brief Undo actions which can be created as script objects.\n\n"
				"Not intended for game development, for editors or internal use only.\n\n "
				"@internal");

UndoAction::UndoAction(const UTF8 *actionName)
{
   mActionName = actionName;
   mUndoManager = NULL;
}

UndoAction::~UndoAction()
{
   // If we are registered to an undo manager, make sure
   // we get off its lists.
   if( mUndoManager )
      mUndoManager->removeAction( this, true );

   clearAllNotifications();
}

//-----------------------------------------------------------------------------
void UndoAction::initPersistFields()
{
   addField("actionName", TypeRealString, Offset(mActionName, UndoAction), 
      "A brief description of the action, for UI representation of this undo/redo action.");

   Parent::initPersistFields();
}

//-----------------------------------------------------------------------------
void UndoAction::addToManager(UndoManager* theMan)
{
   if(theMan)
   {
      mUndoManager = theMan;
      (*theMan).addAction(this);
   }
   else
   {
      mUndoManager = &UndoManager::getDefaultManager();
      mUndoManager->addAction(this);
   }
}

//-----------------------------------------------------------------------------
// CompoundUndoAction
//-----------------------------------------------------------------------------
IMPLEMENT_CONOBJECT( CompoundUndoAction );

ConsoleDocClass( CompoundUndoAction,
				"@brief An undo action that is comprised of other undo actions.\n\n"
				"Not intended for game development, for editors or internal use only.\n\n "
				"@internal");

CompoundUndoAction::CompoundUndoAction( const UTF8 *actionName )
 : Parent( actionName )
{
}

CompoundUndoAction::~CompoundUndoAction()
{
   while( !mChildren.empty() )
   {
      UndoAction* action = mChildren.last();
      if( action->isProperlyAdded() )
         action->deleteObject();
      else
      {
         clearNotify( action );  // need to clear the delete notification manually in this case
         delete action;
      }

      mChildren.pop_back();
   }
}

void CompoundUndoAction::addAction( UndoAction *action )
{
   //AssertFatal( action->mUndoManager == NULL, "CompoundUndoAction::addAction, action already had an UndoManager." );
   mChildren.push_back( action );
   deleteNotify( action );
}

void CompoundUndoAction::undo()
{
   Vector<UndoAction*>::iterator itr = mChildren.end() - 1;
   for ( ; itr != mChildren.begin() - 1; itr-- )   
      (*itr)->undo();
}

void CompoundUndoAction::redo()
{
   Vector<UndoAction*>::iterator itr = mChildren.begin();
   for ( ; itr != mChildren.end(); itr++ )   
      (*itr)->redo();   
}

void CompoundUndoAction::onDeleteNotify( SimObject* object )
{      
   for( U32 i = 0; i < mChildren.size(); ++ i )
      if( mChildren[ i ] == object )
         mChildren.erase( i );

   Parent::onDeleteNotify( object );
}

ConsoleMethod( CompoundUndoAction, addAction, void, 3, 3, "addAction( UndoAction )" )
{
   UndoAction *action;
   if ( Sim::findObject( argv[2], action ) )
      object->addAction( action );
}

//-----------------------------------------------------------------------------
// UndoManager
//-----------------------------------------------------------------------------
IMPLEMENT_CONOBJECT(UndoManager);

ConsoleDocClass( UndoManager,
				"@brief SimObject which adds, tracks, and deletes UndoAction objects.\n\n"
				"Not intended for game development, for editors or internal use only.\n\n "
				"@internal")

UndoManager::UndoManager(U32 levels)
{
   VECTOR_SET_ASSOCIATION( mUndoStack );
   VECTOR_SET_ASSOCIATION( mRedoStack );
   VECTOR_SET_ASSOCIATION( mCompoundStack );

   mNumLevels = levels;
   // levels can be arbitrarily high, so we don't really want to reserve(levels).
   mUndoStack.reserve(10);
   mRedoStack.reserve(10);
}

//-----------------------------------------------------------------------------
UndoManager::~UndoManager()
{
   clearStack(mUndoStack);
   clearStack(mRedoStack);
   clearStack( *( ( Vector< UndoAction* >* ) &mCompoundStack ) );
}

//-----------------------------------------------------------------------------
void UndoManager::initPersistFields()
{
   addField("numLevels", TypeS32, Offset(mNumLevels, UndoManager), "Number of undo & redo levels.");
   // arrange for the default undo manager to exist.
//   UndoManager &def = getDefaultManager();
//   Con::printf("def = %s undo manager created", def.getName());
   
}

//-----------------------------------------------------------------------------
UndoManager& UndoManager::getDefaultManager()
{
   // the default manager is created the first time it is asked for.
   static UndoManager *defaultMan = NULL;
   if(!defaultMan)
   {
      defaultMan = new UndoManager();
      defaultMan->assignName("DefaultUndoManager");
      defaultMan->registerObject();
   }
   return *defaultMan;
}

ConsoleMethod(UndoManager, clearAll, void, 2, 2, "Clears the undo manager.")
{
   object->clearAll();
}

void UndoManager::clearAll()
{
   clearStack(mUndoStack);
   clearStack(mRedoStack);
   
   Con::executef(this, "onClear");
}

//-----------------------------------------------------------------------------
void UndoManager::clearStack(Vector<UndoAction*> &stack)
{
   Vector<UndoAction*>::iterator itr = stack.begin();
   while (itr != stack.end())
   {
      UndoAction* undo = stack.first();
      stack.pop_front();

      // Call deleteObject() if the action was registered.
      if ( undo->isProperlyAdded() )
         undo->deleteObject();
      else
         delete undo;
   }
   stack.clear();
}

//-----------------------------------------------------------------------------
void UndoManager::clampStack(Vector<UndoAction*> &stack)
{
   while(stack.size() > mNumLevels)
   {
      UndoAction *act = stack.front();
      stack.pop_front();

      // Call deleteObject() if the action was registered.
      if ( act->isProperlyAdded() )
         act->deleteObject();
      else
         delete act;
   }
}

void UndoManager::removeAction(UndoAction *action, bool noDelete)
{
   Vector<UndoAction*>::iterator itr = mUndoStack.begin();
   while (itr != mUndoStack.end())
   {
      if ((*itr) == action)
      {
         UndoAction* deleteAction = *itr;
         mUndoStack.erase(itr);
         doRemove( deleteAction, noDelete );
         return;
      }
	  itr++;
   }

   itr = mRedoStack.begin();
   while (itr != mRedoStack.end())
   {
      if ((*itr) == action)
      {
         UndoAction* deleteAction = *itr;
         mRedoStack.erase(itr);
         doRemove( deleteAction, noDelete );
         return;
      }
	  itr++;
   }
}

void UndoManager::doRemove( UndoAction* action, bool noDelete )
{
   if( action->mUndoManager == this )
      action->mUndoManager = NULL;

   if( !noDelete )
   {
      // Call deleteObject() if the action was registered.
      if ( action->isProperlyAdded() )
         action->deleteObject();
      else
         delete action;
   }

   if( isProperlyAdded() )
      Con::executef(this, "onRemoveUndo");
}

//-----------------------------------------------------------------------------
void UndoManager::undo()
{
   // make sure we have an action available
   if(mUndoStack.size() < 1)
      return;

   // pop the action off the undo stack
   UndoAction *act = mUndoStack.last();
   mUndoStack.pop_back();
   
   // add it to the redo stack
   mRedoStack.push_back(act);
   if(mRedoStack.size() > mNumLevels)
      mRedoStack.pop_front();
   
   Con::executef(this, "onUndo");

   // perform the undo, whatever it may be.
   (*act).undo();
}

//-----------------------------------------------------------------------------
void UndoManager::redo()
{
   // make sure we have an action available
   if(mRedoStack.size() < 1)
      return;

   // pop the action off the redo stack
   UndoAction *react = mRedoStack.last();
   mRedoStack.pop_back();
   
   // add it to the undo stack
   mUndoStack.push_back(react);
   if(mUndoStack.size() > mNumLevels)
      mUndoStack.pop_front();
   
   Con::executef(this, "onRedo");
   
   // perform the redo, whatever it may be.
   (*react).redo();
}

ConsoleMethod(UndoManager, getUndoCount, S32, 2, 2, "")
{
   return object->getUndoCount();
}

S32 UndoManager::getUndoCount()
{
   return mUndoStack.size();
}

ConsoleMethod(UndoManager, getUndoName, const char*, 3, 3, "(index)")
{
   return object->getUndoName(dAtoi(argv[2]));
}

const char* UndoManager::getUndoName(S32 index)
{
   if ((index < getUndoCount()) && (index >= 0))
      return mUndoStack[index]->mActionName;

   return NULL;
}

ConsoleMethod(UndoManager, getUndoAction, S32, 3, 3, "(index)")
{
   UndoAction * action = object->getUndoAction(dAtoi(argv[2]));
   if ( !action )
      return -1;
   
   if ( !action->isProperlyAdded() )
      action->registerObject();

   return action->getId();
}

UndoAction* UndoManager::getUndoAction(S32 index)
{
   if ((index < getUndoCount()) && (index >= 0))
      return mUndoStack[index];
   return NULL;
}

ConsoleMethod(UndoManager, getRedoCount, S32, 2, 2, "")
{
   return object->getRedoCount();
}

S32 UndoManager::getRedoCount()
{
   return mRedoStack.size();
}

ConsoleMethod(UndoManager, getRedoName, const char*, 3, 3, "(index)")
{
   return object->getRedoName(dAtoi(argv[2]));
}

const char* UndoManager::getRedoName(S32 index)
{
   if ((index < getRedoCount()) && (index >= 0))
      return mRedoStack[getRedoCount() - index - 1]->mActionName;

   return NULL;
}

ConsoleMethod(UndoManager, getRedoAction, S32, 3, 3, "(index)")
{
   UndoAction * action = object->getRedoAction(dAtoi(argv[2]));

   if ( !action )
      return -1;

   if ( !action->isProperlyAdded() )
      action->registerObject();

   return action->getId();
}

UndoAction* UndoManager::getRedoAction(S32 index)
{
   if ((index < getRedoCount()) && (index >= 0))
      return mRedoStack[index];
   return NULL;
}

//-----------------------------------------------------------------------------
const char* UndoManager::getNextUndoName()
{
   if(mUndoStack.size() < 1)
      return NULL;
      
   UndoAction *act = mUndoStack.last();
   return (*act).mActionName;
}

//-----------------------------------------------------------------------------
const char* UndoManager::getNextRedoName()
{
   if(mRedoStack.size() < 1)
      return NULL;

   UndoAction *act = mRedoStack.last();
   return (*act).mActionName;
}

//-----------------------------------------------------------------------------
void UndoManager::addAction(UndoAction* action)
{
   // If we are assembling a compound, redirect the action to it
   // and don't modify our current undo/redo state.
   
   if( mCompoundStack.size() )
   {
      mCompoundStack.last()->addAction( action );
      return;
   }
   
   // clear the redo stack
   clearStack(mRedoStack);

   // push the incoming action onto the stack, move old data off the end if necessary.
   mUndoStack.push_back(action);
   if(mUndoStack.size() > mNumLevels)
      mUndoStack.pop_front();

   Con::executef(this, "onAddUndo");
}

//-----------------------------------------------------------------------------

CompoundUndoAction* UndoManager::pushCompound( const String& name )
{
   mCompoundStack.push_back( new CompoundUndoAction( name ) );
   return mCompoundStack.last();
}

//-----------------------------------------------------------------------------

void UndoManager::popCompound( bool discard )
{
   AssertFatal( getCompoundStackDepth() > 0, "UndoManager::popCompound - no compound on stack!" );
   
   CompoundUndoAction* undo = mCompoundStack.last();
   mCompoundStack.pop_back();
   
   if( discard || undo->getNumChildren() == 0 )
   {
      if( undo->isProperlyAdded() )
         undo->deleteObject();
      else
         delete undo;
   }
   else
      addAction( undo );
}

//-----------------------------------------------------------------------------
ConsoleMethod(UndoAction, addToManager, void, 2, 3, "action.addToManager([undoManager])")
{
   UndoManager *theMan = NULL;
   if(argc == 3)
   {
      SimObject *obj = Sim::findObject(argv[2]);
      if(obj)
         theMan = dynamic_cast<UndoManager*> (obj);
   }
   object->addToManager(theMan);
}

//-----------------------------------------------------------------------------

ConsoleMethod( UndoAction, undo, void, 2, 2, "() - Undo action contained in undo." )
{
   object->undo();
}

//-----------------------------------------------------------------------------

ConsoleMethod( UndoAction, redo, void, 2, 2, "() - Reo action contained in undo." )
{
   object->redo();
}

//-----------------------------------------------------------------------------
ConsoleMethod(UndoManager, undo, void, 2, 2, "UndoManager.undo();")
{
   object->undo();
}

//-----------------------------------------------------------------------------
ConsoleMethod(UndoManager, redo, void, 2, 2, "UndoManager.redo();")
{
   object->redo();
}

//-----------------------------------------------------------------------------
ConsoleMethod(UndoManager, getNextUndoName, const char *, 2, 2, "UndoManager.getNextUndoName();")
{
   const char *name = object->getNextUndoName();
   if(!name)
      return NULL;
   char *ret = Con::getReturnBuffer(dStrlen(name) + 1);
   dStrcpy(ret, name);
   return ret;
}

//-----------------------------------------------------------------------------
ConsoleMethod(UndoManager, getNextRedoName, const char *, 2, 2, "UndoManager.getNextRedoName();")
{
   const char *name = object->getNextRedoName();
   if(!name)
      return NULL;
   char *ret = Con::getReturnBuffer(dStrlen(name) + 1);
   dStrcpy(ret, name);
   return ret;
}

//-----------------------------------------------------------------------------

ConsoleMethod( UndoManager, pushCompound, const char*, 2, 3, "( string name=\"\" ) - Push a CompoundUndoAction onto the compound stack for assembly." )
{
   String name;
   if( argc > 2 )
      name = argv[ 2 ];
      
   CompoundUndoAction* action = object->pushCompound( name );
   if( !action )
      return "";
      
   if( !action->isProperlyAdded() )
      action->registerObject();
      
   return action->getIdString();
}

//-----------------------------------------------------------------------------

ConsoleMethod( UndoManager, popCompound, void, 2, 3, "( bool discard=false ) - Pop the current CompoundUndoAction off the stack." )
{
   if( !object->getCompoundStackDepth() )
   {
      Con::errorf( "%s::popCompound - no compound on stack", argv[ 0 ] );
      return;
   }
   
   bool discard = false;
   if( argc > 2 )
      discard = dAtob( argv[ 2 ] );
   
   object->popCompound( discard );
}
