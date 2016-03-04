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
#include "console/simSet.h"

#include "core/stringTable.h"
#include "console/console.h"
#include "console/engineAPI.h"
#include "core/stream/fileStream.h"
#include "sim/actionMap.h"
#include "core/fileObject.h"
#include "console/consoleInternal.h"
#include "console/engineAPI.h"
#include "platform/profiler.h"
#include "console/typeValidators.h"
#include "core/frameAllocator.h"
#include "math/mMathFn.h"


IMPLEMENT_CONOBJECT( SimSet );
IMPLEMENT_CONOBJECT( SimGroup );

ConsoleDocClass( SimSet,
   "@brief A collection of SimObjects.\n\n"

   "It is often necessary to keep track of an arbitrary set of SimObjects. "
   "For instance, Torque's networking code needs to not only keep track of "
   "the set of objects which need to be ghosted, but also the set of objects "
   "which must <i>always</i> be ghosted. It does this by working with two "
   "sets. The first of these is the RootGroup (which is actually a SimGroup) "
   "and the second is the GhostAlwaysSet, which contains objects which must "
   "always be ghosted to the client.\n\n"

   "Some general notes on SimSets:\n\n"
   "- Membership is not exclusive. A SimObject may be a member of multiple "
   "SimSets.\n\n"
   "- A SimSet does not destroy subobjects when it is destroyed.\n\n"
   "- A SimSet may hold an arbitrary number of objects.\n\n"

   "@ingroup Console\n"
   "@ingroup Scripting"
);
ConsoleDocClass( SimGroup,
   "@brief A collection of SimObjects that are owned by the group.\n\n"
   
   "A SimGroup is a stricter form of SimSet. SimObjects may only be a member "
   "of a single SimGroup at a time. The SimGroup will automatically enforce "
   "the single-group-membership rule (ie. adding an object to a SimGroup will "
   "cause it to be removed from its current SimGroup, if any).\n\n"

   "Deleting a SimGroup will also delete all SimObjects in the SimGroup.\n\n"

   "@tsexample\n"
   "// Create a SimGroup for particle emitters\n"
   "new SimGroup(Emitters)\n"
   "{\n"
   "   canSaveDynamicFields = \"1\";\n\n"
   "   new ParticleEmitterNode(CrystalEmmiter) {\n"
   "      active = \"1\";\n"
   "      emitter = \"dustEmitter\";\n"
   "      velocity = \"1\";\n"
   "      dataBlock = \"GenericSmokeEmitterNode\";\n"
   "      position = \"-61.6276 2.1142 4.45027\";\n"
   "      rotation = \"1 0 0 0\";\n"
   "      scale = \"1 1 1\";\n"
   "      canSaveDynamicFields = \"1\";\n"
   "   };\n\n"
   "   new ParticleEmitterNode(Steam1) {\n"
   "      active = \"1\";\n"
   "      emitter = \"SlowSteamEmitter\";\n"
   "      velocity = \"1\";\n"
   "      dataBlock = \"GenericSmokeEmitterNode\";\n"
   "      position = \"-25.0458 1.55289 2.51308\";\n"
   "      rotation = \"1 0 0 0\";\n"
   "      scale = \"1 1 1\";\n"
   "      canSaveDynamicFields = \"1\";\n"
   "   };\n"
   "};\n\n"
   "@endtsexample\n\n"
   "@ingroup Console\n"
   "@ingroup Scripting"
);

IMPLEMENT_CALLBACK( SimSet, onObjectAdded, void, ( SimObject* object ), ( object ),
   "Called when an object is added to the set.\n"
   "@param object The object that was added." );
IMPLEMENT_CALLBACK( SimSet, onObjectRemoved, void, ( SimObject* object ), ( object ),
   "Called when an object is removed from the set.\n"
   "@param object The object that was removed." );
   

//=============================================================================
//    SimSet.
//=============================================================================
// MARK: ---- SimSet ----

//-----------------------------------------------------------------------------

SimSet::SimSet()
{
   VECTOR_SET_ASSOCIATION( objectList );
   mMutex = Mutex::createMutex();
}

//-----------------------------------------------------------------------------

SimSet::~SimSet()
{
   Mutex::destroyMutex( mMutex );
}

//-----------------------------------------------------------------------------

void SimSet::addObject( SimObject* obj )
{
   // Prevent SimSet being added to itself.
   if( obj == this )
      return;
      
   lock();
   
   const bool added = objectList.pushBack( obj );
   if( added )
      deleteNotify( obj );
   
   unlock();

   if( added )
   {
      getSetModificationSignal().trigger( SetObjectAdded, this, obj );
      if( obj->isProperlyAdded() )
         onObjectAdded_callback( obj );
   }
}

//-----------------------------------------------------------------------------

void SimSet::removeObject( SimObject* obj )
{
   lock();
   
   const bool removed = objectList.remove( obj );
   if( removed )
      clearNotify( obj );
   
   unlock();

   if( removed )
   {
      getSetModificationSignal().trigger( SetObjectRemoved, this, obj );
      if( obj->isProperlyAdded() )
         onObjectRemoved_callback( obj );
   }
}

//-----------------------------------------------------------------------------

void SimSet::pushObject( SimObject* obj )
{
   if( obj == this )
      return;
      
   lock();
   
   bool added = objectList.pushBackForce( obj );
   if( added )
      deleteNotify( obj );
      
   unlock();

   if( added )
   {
      getSetModificationSignal().trigger( SetObjectAdded, this, obj );
      if( obj->isProperlyAdded() )
         onObjectAdded_callback( obj );
   }
}

//-----------------------------------------------------------------------------

void SimSet::popObject()
{
   if( objectList.empty() )
   {
      AssertWarn(false, "Stack underflow in SimSet::popObject");
      return;
   }

   lock();
   SimObject* object = objectList.last();
   objectList.pop_back();

   clearNotify( object );
   unlock();
   
   getSetModificationSignal().trigger( SetObjectRemoved, this, object );
   if( object->isProperlyAdded() )
      onObjectRemoved_callback( object );
}

//-----------------------------------------------------------------------------

void SimSet::scriptSort( const String &scriptCallbackFn )
{
   lock();
   objectList.scriptSort( scriptCallbackFn );
   unlock();
}

//-----------------------------------------------------------------------------

void SimSet::callOnChildren( const String &method, S32 argc, ConsoleValueRef argv[], bool executeOnChildGroups )
{
   // Prep the arguments for the console exec...
   // Make sure and leave args[1] empty.
   ConsoleValueRef args[21] = { };
   ConsoleValue name_method;
   name_method.setStackStringValue(method.c_str());
   args[0] = ConsoleValueRef::fromValue(&name_method);

   for (S32 i = 0; i < argc; i++)
      args[i + 2] = argv[i];

   for( iterator i = begin(); i != end(); i++ )
   {
      SimObject *childObj = static_cast<SimObject*>(*i);

      if( childObj->isMethod( method.c_str() ) )
         Con::execute(childObj, argc + 2, args);

      if( executeOnChildGroups )
      {
         SimSet* childSet = dynamic_cast<SimSet*>(*i);
         if ( childSet )
            childSet->callOnChildren( method, argc, argv, executeOnChildGroups );
      }
   }
}

//-----------------------------------------------------------------------------

U32 SimSet::sizeRecursive()
{
   U32 count = 0;

   for ( iterator i = begin(); i != end(); i++ )
   {
      count++;

      SimSet* childSet = dynamic_cast<SimSet*>(*i);
      if ( childSet )
         count += childSet->sizeRecursive();
   }

   return count;
}

//-----------------------------------------------------------------------------

bool SimSet::reOrder( SimObject *obj, SimObject *target )
{
   MutexHandle handle;
   handle.lock(mMutex);

   iterator itrS, itrD;
   if ( (itrS = find(begin(),end(),obj)) == end() )
   {
      // object must be in list
      return false; 
   }

   if ( obj == target )
   {
      // don't reorder same object but don't indicate error
      return true;   
   }

   if ( !target )    
   {
      // if no target, then put to back of list

      // don't move if already last object
      if ( itrS != (end()-1) )
      {
         // remove object from its current location and push to back of list
         objectList.erase(itrS);    
         objectList.push_back(obj);
      }
   }
   else
   {
      // if target, insert object in front of target
      if ( (itrD = find(begin(),end(),target)) == end() )
         // target must be in list
         return false;

      objectList.erase(itrS);

      // once itrS has been erased, itrD won't be pointing at the 
      // same place anymore - re-find...
      itrD = find(begin(),end(),target);
      objectList.insert(itrD, obj);
   }

   return true;
}   

//-----------------------------------------------------------------------------

void SimSet::onDeleteNotify(SimObject *object)
{
   removeObject(object);
   Parent::onDeleteNotify(object);
}

//-----------------------------------------------------------------------------

void SimSet::onRemove()
{
   MutexHandle handle;
   handle.lock( mMutex );

   if( !objectList.empty() )
   {
      objectList.sortId();
      
      // This backwards iterator loop doesn't work if the
      // list is empty, check the size first.
      
      for( SimObjectList::iterator ptr = objectList.end() - 1;
            ptr >= objectList.begin(); ptr -- )
         clearNotify( *ptr );
   }

   handle.unlock();

   Parent::onRemove();
}

//-----------------------------------------------------------------------------

void SimSet::write(Stream &stream, U32 tabStop, U32 flags)
{
   MutexHandle handle;
   handle.lock(mMutex);

   // export selected only?
   if((flags & SelectedOnly) && !isSelected())
   {
      for(U32 i = 0; i < size(); i++)
         (*this)[i]->write(stream, tabStop, flags);

      return;

   }

   stream.writeTabs( tabStop );
   char buffer[ 2048 ];
   const U32 bufferWriteLen = dSprintf( buffer, sizeof( buffer ), "new %s(%s) {\r\n", getClassName(), getName() && !( flags & NoName ) ? getName() : "" );
   stream.write( bufferWriteLen, buffer );
   writeFields( stream, tabStop + 1 );

   if(size())
   {
      stream.write(2, "\r\n");
      for(U32 i = 0; i < size(); i++)
      {
         SimObject* child = ( *this )[ i ];
         if( child->getCanSave() )
            child->write(stream, tabStop + 1, flags);
      }
   }

   stream.writeTabs(tabStop);
   stream.write(4, "};\r\n");
}

//-----------------------------------------------------------------------------

void SimSet::clear()
{
   lock();
   
   while( !empty() )
      popObject();
      
   unlock();

   getSetModificationSignal().trigger( SetCleared, this, NULL );
}

//-----------------------------------------------------------------------------

//UNSAFE
void SimSet::deleteAllObjects()
{
   lock();
   while( !empty() )
   {
      SimObject* object = objectList.last();
      objectList.pop_back();

      object->deleteObject();
   }
   unlock();
}

//-----------------------------------------------------------------------------

SimObject* SimSet::findObject( SimObject* object )
{
   bool found = false;
   lock();
   for( SimSet::iterator iter = begin(); iter != end(); ++ iter )
      if( *iter == object )
      {
         found = true;
         break;
      }
   unlock();
   
   if( found )
      return object;
      
   return NULL;
}

//-----------------------------------------------------------------------------

SimObject* SimSet::findObject( const char *namePath )
{
   // find the end of the object name
   S32 len;
   for(len = 0; namePath[len] != 0 && namePath[len] != '/'; len++)
      ;

   StringTableEntry stName = StringTable->lookupn(namePath, len);
   if(!stName)
      return NULL;

   lock();
   for(SimSet::iterator i = begin(); i != end(); i++)
   {
      if((*i)->getName() == stName)
      {
         unlock();
         if(namePath[len] == 0)
            return *i;
         return (*i)->findObject(namePath + len + 1);
      }
   }
   unlock();
   return NULL;
}

//-----------------------------------------------------------------------------

SimObject* SimSet::findObjectByInternalName(StringTableEntry internalName, bool searchChildren)
{
   iterator i;
   for (i = begin(); i != end(); i++)
   {
      SimObject *childObj = static_cast<SimObject*>(*i);
      if(childObj->getInternalName() == internalName)
         return childObj;
      else if (searchChildren)
      {
         SimSet* childSet = dynamic_cast<SimSet*>(*i);
         if (childSet)
         {
            SimObject* found = childSet->findObjectByInternalName(internalName, searchChildren);
            if (found) return found;
         }
      }
   }

   return NULL;
}

//-----------------------------------------------------------------------------

SimObject* SimSet::findObjectByLineNumber(const char* fileName, S32 declarationLine, bool searchChildren)
{
   if (!fileName)
      return NULL;

   if (declarationLine < 0)
      return NULL;

   StringTableEntry fileEntry = StringTable->insert(fileName);

   for (iterator i = begin(); i != end(); i++)
   {
      SimObject *childObj = static_cast<SimObject*>(*i);

      if(childObj->getFilename() == fileEntry && childObj->getDeclarationLine() == declarationLine)
         return childObj;
      else if (searchChildren)
      {
         SimSet* childSet = dynamic_cast<SimSet*>(*i);

         if (childSet)
         {
            SimObject* found = childSet->findObjectByLineNumber(fileName, declarationLine, searchChildren);
            if (found)
               return found;
         }
      }
   }

   return NULL;
}

//-----------------------------------------------------------------------------

SimObject* SimSet::getRandom()
{
   if (size() > 0)
      return objectList[mRandI(0, size() - 1)];

   return NULL;
}

//-----------------------------------------------------------------------------

SimSet* SimSet::clone()
{
   // Clone the set object.
   
   SimObject* object = Parent::clone();
   SimSet* set = dynamic_cast< SimSet* >( object );
   if( !set )
   {
      object->deleteObject();
      return NULL;
   }
   
   // Add all object in the set.
   
   for( iterator iter = begin(); iter != end(); ++ iter )
      set->addObject( *iter );
   
   return set;
}

//-----------------------------------------------------------------------------

inline void SimSetIterator::Stack::push_back(SimSet* set)
{
   increment();
   last().set = set;
   last().itr = set->begin();
}

//-----------------------------------------------------------------------------

SimSetIterator::SimSetIterator(SimSet* set)
{
   VECTOR_SET_ASSOCIATION(stack);

   if (!set->empty())
      stack.push_back(set);
}

//-----------------------------------------------------------------------------

SimObject* SimSetIterator::operator++()
{
   SimSet* set;
   if ((set = dynamic_cast<SimSet*>(*stack.last().itr)) != 0)
   {
      if (!set->empty())
      {
         stack.push_back(set);
         return *stack.last().itr;
      }
   }

   while (++stack.last().itr == stack.last().set->end())
   {
      stack.pop_back();
      if (stack.empty())
         return 0;
   }
   return *stack.last().itr;
}

//=============================================================================
//    SimGroup.
//=============================================================================
// MARK: ---- SimGroup ----

//-----------------------------------------------------------------------------

SimGroup::~SimGroup()
{
   for( iterator itr = begin(); itr != end(); itr ++ )
      mNameDictionary.remove(*itr);
}

//-----------------------------------------------------------------------------

void SimGroup::_addObject( SimObject* obj, bool forcePushBack )
{
   // Make sure we aren't adding ourself.  This isn't the most robust check
   // but it should be good enough to prevent some self-foot-shooting.
   if( obj == this )
   {
      Con::errorf( "SimGroup::addObject - (%d) can't add self!", getIdString() );
      return;
   }
   
   if( obj->getGroup() == this )
      return;

   lock();
   
   obj->incRefCount();
   
   if( obj->getGroup() )
      obj->getGroup()->removeObject( obj );
      
   if( forcePushBack ? objectList.pushBack( obj ) : objectList.pushBackForce( obj ) )
   {
      mNameDictionary.insert( obj );
      obj->mGroup = this;
      
      obj->onGroupAdd();
      
      getSetModificationSignal().trigger( SetObjectAdded, this, obj );
      if( obj->isProperlyAdded() )
         onObjectAdded_callback( obj );
   }
   else
      obj->decRefCount();
   
   unlock();

   // SimObjects will automatically remove them from their group
   // when deleted so we don't hook up a delete notification.
}

//-----------------------------------------------------------------------------

void SimGroup::addObject( SimObject* obj )
{
   _addObject( obj );
}

//-----------------------------------------------------------------------------

void SimGroup::removeObject( SimObject* obj )
{
   lock();
   _removeObjectNoLock( obj );
   unlock();
}

//-----------------------------------------------------------------------------

void SimGroup::_removeObjectNoLock( SimObject* obj )
{
   if( obj->mGroup == this )
   {
      obj->onGroupRemove();
      
      mNameDictionary.remove( obj );
      objectList.remove( obj );
      obj->mGroup = 0;

      getSetModificationSignal().trigger( SetObjectRemoved, this, obj );
      if( obj->isProperlyAdded() )
         onObjectRemoved_callback( obj );
      obj->decRefCount();
   }
}

//-----------------------------------------------------------------------------

void SimGroup::pushObject( SimObject* object )
{
   _addObject( object, true );
}

//-----------------------------------------------------------------------------

void SimGroup::popObject()
{
   MutexHandle handle;
   handle.lock( mMutex );

   if( objectList.empty() )
   {
      AssertWarn( false, "SimGroup::popObject - Stack underflow" );
      return;
   }

   SimObject* object = objectList.last();
   objectList.pop_back();

   object->onGroupRemove();
   object->mGroup = NULL;

   clearNotify( object );
   mNameDictionary.remove( object );
      
   getSetModificationSignal().trigger( SetObjectAdded, this, object );
   if( object->isProperlyAdded() )
      onObjectRemoved_callback( object );
   
   object->decRefCount();
}

//-----------------------------------------------------------------------------

void SimGroup::onRemove()
{
   lock();
   if( !objectList.empty() )
   {
      objectList.sortId();
      clear();
   }
   SimObject::onRemove();
   unlock();
}

//-----------------------------------------------------------------------------

void SimGroup::clear()
{
   lock();
   while( size() > 0 )
   {
      SimObject* object = objectList.last();
      object->onGroupRemove();
      
      objectList.pop_back();
      mNameDictionary.remove( object );
      object->mGroup = 0;

      getSetModificationSignal().trigger( SetObjectRemoved, this, object );
      if( object->isProperlyAdded() )
         onObjectRemoved_callback( object );

      if( engineAPI::gUseConsoleInterop )
         object->deleteObject();
      else
         object->decRefCount();      
   }
   unlock();

   getSetModificationSignal().trigger( SetCleared, this, NULL );
}

//-----------------------------------------------------------------------------

SimObject *SimGroup::findObject(const char *namePath)
{
   // find the end of the object name
   S32 len;
   for(len = 0; namePath[len] != 0 && namePath[len] != '/'; len++)
      ;

   StringTableEntry stName = StringTable->lookupn(namePath, len);
   if(!stName)
      return NULL;

   SimObject *root = mNameDictionary.find( stName );
   if( !root )
      return NULL;

   if(namePath[len] == 0)
      return root;

   return root->findObject(namePath + len + 1);
}

//-----------------------------------------------------------------------------

SimGroup* SimGroup::clone()
{
   // Skip SimSet::clone since we do not want to steal the child objects
   // from this group.
   
   SimObject* object = SimObject::clone();
   SimGroup* group = dynamic_cast< SimGroup* >( object );
   if( !group )
   {
      object->deleteObject();
      return NULL;
   }
   
   return group;
}

//-----------------------------------------------------------------------------

SimGroup* SimGroup::deepClone()
{
   // Clone the group object.
   
   SimObject* object = Parent::deepClone();
   SimGroup* group = dynamic_cast< SimGroup* >( object );
   if( !group )
   {
      object->deleteObject();
      return NULL;
   }
   
   // Clone all child objects.
   
   for( iterator iter = begin(); iter != end(); ++ iter )
      group->addObject( ( *iter )->deepClone() );
   
   return group;
}

//-----------------------------------------------------------------------------

bool SimGroup::processArguments(S32, ConsoleValueRef *argv)
{
   return true;
}

//-----------------------------------------------------------------------------

SimObject* SimGroupIterator::operator++()
{
   SimGroup* set;
   if ((set = dynamic_cast<SimGroup*>(*stack.last().itr)) != 0)
   {
      if (!set->empty())
      {
         stack.push_back(set);
         return *stack.last().itr;
      }
   }

   while (++stack.last().itr == stack.last().set->end())
   {
      stack.pop_back();
      if (stack.empty())
         return 0;
   }
   return *stack.last().itr;
}

//=============================================================================
//    API.
//=============================================================================
// MARK: ---- API ----

//-----------------------------------------------------------------------------

DefineEngineMethod( SimSet, listObjects, void, (),,
   "Dump a list of all objects contained in the set to the console." )
{
   object->lock();
   SimSet::iterator itr;
   for(itr = object->begin(); itr != object->end(); itr++)
   {
      SimObject *obj = *itr;
      bool isSet = dynamic_cast<SimSet *>(obj) != 0;
      const char *name = obj->getName();
      if(name)
         Con::printf("   %d,\"%s\": %s %s", obj->getId(), name,
         obj->getClassName(), isSet ? "(g)":"");
      else
         Con::printf("   %d: %s %s", obj->getId(), obj->getClassName(),
         isSet ? "(g)" : "");
   }
   object->unlock();
}

//-----------------------------------------------------------------------------

DEFINE_CALLIN( fnSimSet_add, add, SimSet, void, ( SimSet* set, SimObject* object ),,,
   "Add the given object to the set.\n"
   "@param object An object." )
{
   if( object )
      set->addObject( object );
}

ConsoleMethod( SimSet, add, void, 3, 0,
   "( SimObject objects... ) Add the given objects to the set.\n"
   "@param objects The objects to add to the set." )
{
   for(S32 i = 2; i < argc; i++)
   {
      SimObject *obj = Sim::findObject( argv[ i ] );
      if(obj)
         object->addObject( obj );
      else
         Con::printf("Set::add: Object \"%s\" doesn't exist", (const char*)argv[ i ] );
   }
}

//-----------------------------------------------------------------------------

DEFINE_CALLIN( fnSimSet_remove, remove, SimSet, void, ( SimSet* set, SimObject* object ),,,
   "Remove the given object from the set.\n"
   "@param object An object." )
{
   if( object )
      set->removeObject( object );
}

ConsoleMethod( SimSet, remove, void, 3, 0,
   "( SimObject objects... ) Remove the given objects from the set.\n"
   "@param objects The objects to remove from the set." )
{
   for(S32 i = 2; i < argc; i++)
   {
      SimObject *obj = Sim::findObject(argv[i]);
      object->lock();
      if(obj && object->find(object->begin(),object->end(),obj) != object->end())
         object->removeObject(obj);
      else
         Con::printf("Set::remove: Object \"%s\" does not exist in set", (const char*)argv[i]);
      object->unlock();
   }
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SimSet, clear, void, (),,
   "Remove all objects from the set." )
{
   object->clear();
}

//-----------------------------------------------------------------------------

//UNSAFE; don't want this in the new API
DefineConsoleMethod( SimSet, deleteAllObjects, void, (), , "() Delete all objects in the set." )
{
   object->deleteAllObjects();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SimSet, getRandom, SimObject*, (),,
   "Return a random object from the set.\n"
   "@return A randomly selected object from the set or -1 if the set is empty." )
{
   return object->getRandom();
}

//-----------------------------------------------------------------------------

ConsoleMethod( SimSet, callOnChildren, void, 3, 0,
   "( string method, string args... ) Call a method on all objects contained in the set.\n\n"
   "@param method The name of the method to call.\n"
   "@param args The arguments to the method.\n\n"
   "@note This method recurses into all SimSets that are children to the set.\n\n"
   "@see callOnChildrenNoRecurse" )
{
   object->callOnChildren( (const char*)argv[2], argc - 3, argv + 3 );
}

//-----------------------------------------------------------------------------

ConsoleMethod( SimSet, callOnChildrenNoRecurse, void, 3, 0,
   "( string method, string args... ) Call a method on all objects contained in the set.\n\n"
   "@param method The name of the method to call.\n"
   "@param args The arguments to the method.\n\n"
   "@note This method does not recurse into child SimSets.\n\n"
   "@see callOnChildren" )
{
   object->callOnChildren( (const char*)argv[2], argc - 3, argv + 3, false );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SimSet, reorderChild, void, ( SimObject* child1, SimObject* child2 ),,
   "Make sure child1 is ordered right before child2 in the set.\n"
   "@param child1 The first child.  The object must already be contained in the set.\n"
   "@param child2 The second child.  The object must already be contained in the set." )
{
   SimObject* pObject = child1;
   SimObject* pTarget = child2;

   if(pObject && pTarget)
   {
      object->reOrder(pObject,pTarget);
   }
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SimSet, getCount, S32, (),,
   "Get the number of objects contained in the set.\n"
   "@return The number of objects contained in the set." )
{
   return object->size();
}

//-----------------------------------------------------------------------------

DEFINE_CALLIN( fnSimSet_getCountRecursive, getCountRecursive, SimSet, U32, ( SimSet* set ),,,
   "Get the number of direct and indirect child objects contained in the set.\n"
   "@return The number of objects contained in the set as well as in other sets contained directly or indirectly in the set." )
{
   return set->sizeRecursive();
}

DefineConsoleMethod( SimSet, getFullCount, S32, (), , "() Get the number of direct and indirect child objects contained in the set.\n"
   "@return The number of objects contained in the set as well as in other sets contained directly or indirectly in the set." )
{
   return object->sizeRecursive();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SimSet, getObject, SimObject*, ( U32 index ),,
   "Get the object at the given index.\n"
   "@param index The object index.\n"
   "@return The object at the given index or -1 if index is out of range." )
{
   if( index < 0 || index >= object->size() )
   {
      Con::errorf( "Set::getObject - index out of range." );
      return NULL;
   }
   
   return ( *object )[ index ];
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SimSet, getObjectIndex, S32, ( SimObject* obj ),,
   "Return the index of the given object in this set.\n"
   "@param obj The object for which to return the index.  Must be contained in the set.\n"
   "@return The index of the object or -1 if the object is not contained in the set." )
{
   if( !obj )
      return -1;

   object->lock();
   S32 count = 0;
   for( SimSet::iterator i = object->begin(); i != object->end(); i++)
   {
      if( *i == obj )
      {
         object->unlock();
         return count;
      }

      ++count;
   }
   object->unlock();

   return -1;
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SimSet, isMember, bool, ( SimObject* obj ),,
   "Test whether the given object belongs to the set.\n"
   "@param obj The object.\n"
   "@return True if the object is contained in the set; false otherwise." )
{
   if( !obj )
      return false;

   return ( object->find( object->begin(), object->end(), obj ) != object->end() );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SimSet, findObjectByInternalName, SimObject*, ( const char* internalName, bool searchChildren ), ( false ),
   "Find an object in the set by its internal name.\n"
   "@param internalName The internal name of the object to look for.\n"
   "@param searchChildren If true, SimSets contained in the set will be recursively searched for the object.\n"
   "@return The object with the given internal name or 0 if no match was found.\n" )
{
   StringTableEntry pcName = StringTable->insert( internalName );
   return object->findObjectByInternalName( pcName, searchChildren );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SimSet, bringToFront, void, ( SimObject* obj ),,
   "Make the given object the first object in the set.\n"
   "@param obj The object to bring to the frontmost position.  Must be contained in the set." )
{
   if( obj )
      object->bringObjectToFront( obj );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SimSet, pushToBack, void, ( SimObject* obj ),,
   "Make the given object the last object in the set.\n"
   "@param obj The object to bring to the last position.  Must be contained in the set." )
{
   if( obj )
      object->pushObjectToBack( obj );
}

//-----------------------------------------------------------------------------

DefineConsoleMethod( SimSet, sort, void, ( const char * callbackFunction ), , "( string callbackFunction ) Sort the objects in the set using the given comparison function.\n"
   "@param callbackFunction Name of a function that takes two object arguments A and B and returns -1 if A is less, 1 if B is less, and 0 if both are equal." )
{
   object->scriptSort( callbackFunction );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SimSet, acceptsAsChild, bool, ( SimObject* obj ),,
   "Test whether the given object may be added to the set.\n"
   "@param obj The object to test for potential membership.\n"
   "@return True if the object may be added to the set, false otherwise." )
{
   if( !obj )
      return false;
   
   return object->acceptsAsChild( obj );
}
