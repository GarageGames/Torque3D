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

#ifndef _SIMSET_H_
#define _SIMSET_H_

#ifndef _SIMOBJECT_H_
#include "console/simObject.h"
#endif

#ifndef _SIMOBJECTLIST_H_
#include "console/simObjectList.h"
#endif

#ifndef _SIMDICTIONARY_H_
#include "console/simDictionary.h"
#endif

#ifndef _TSIGNAL_H_
#include "core/util/tSignal.h"
#endif


//---------------------------------------------------------------------------
/// A set of SimObjects.
///
/// It is often necessary to keep track of an arbitrary set of SimObjects.
/// For instance, Torque's networking code needs to not only keep track of
/// the set of objects which need to be ghosted, but also the set of objects
/// which must <i>always</i> be ghosted. It does this by working with two
/// sets. The first of these is the RootGroup (which is actually a SimGroup)
/// and the second is the GhostAlwaysSet, which contains objects which must
/// always be ghosted to the client.
///
/// Some general notes on SimSets:
///     - Membership is not exclusive. A SimObject may be a member of multiple
///       SimSets.
///     - A SimSet does not destroy subobjects when it is destroyed.
///     - A SimSet may hold an arbitrary number of objects.
///
/// Using SimSets, the code to work with these two sets becomes
/// relatively straightforward:
///
/// @code
///        // (Example from netObject.cc)
///        // To iterate over all the objects in the Sim:
///        for (SimSetIterator obj(Sim::getRootGroup()); *obj; ++obj)
///        {
///                  NetObject* nobj = dynamic_cast<NetObject*>(*obj);
///
///                 if (nobj)
///                   {
///                     // ... do things ...
///                 }
///         }
///
///         // (Example from netGhost.cc)
///         // To iterate over the ghostAlways set.
///         SimSet* ghostAlwaysSet = Sim::getGhostAlwaysSet();
///         SimSet::iterator i;
///
///         U32 sz = ghostAlwaysSet->size();
///         S32 j;
///
///         for(i = ghostAlwaysSet->begin(); i != ghostAlwaysSet->end(); i++)
///         {
///             NetObject *obj = (NetObject *)(*i);
///
///             /// ... do things with obj...
///         }
/// @endcode
///
class SimSet: public SimObject
{
   public:

      typedef SimObject Parent;

      enum SetModification
      {
         SetCleared,
         SetObjectAdded,
         SetObjectRemoved
      };

      /// Signal for letting observers know when objects are added to or removed from
      /// the set.
      ///
      /// @param modification In what way the set has been modified.
      /// @param set The set that has been modified.
      /// @param object If #modification is #SetObjectAdded or #SetObjectRemoved, this is
      ///   the object that has been added or removed.  Otherwise NULL.
      typedef Signal< void( SetModification modification, SimSet* set, SimObject* object ) > SetModificationSignal;

   protected:

      SimObjectList objectList;
      void *mMutex;

      /// Signal that is triggered when objects are added or removed from the set.
      SetModificationSignal mSetModificationSignal;
      
      /// @name Callbacks
      /// @{
      
      DECLARE_CALLBACK( void, onObjectAdded, ( SimObject* object ) );
      DECLARE_CALLBACK( void, onObjectRemoved, ( SimObject* object ) );
      
      /// @}

   public:

      SimSet();
      ~SimSet();

      /// Return the signal that is triggered when an object is added to or removed
      /// from the set.
      const SetModificationSignal& getSetModificationSignal() const { return mSetModificationSignal; }
      SetModificationSignal& getSetModificationSignal() { return mSetModificationSignal; }

      /// @name STL Interface
      /// @{

      ///
      typedef SimObjectList::iterator iterator;
      typedef SimObjectList::value_type value;
      SimObject* front() { return objectList.front(); }
      SimObject* first() { return objectList.first(); }
      SimObject* last()  { return objectList.last(); }
      bool       empty() const { return objectList.empty();   }
      S32        size() const  { return objectList.size(); }
      iterator   begin() { return objectList.begin(); }
      iterator   end()   { return objectList.end(); }
      value operator[] (S32 index) { return objectList[U32(index)]; }

      iterator find( iterator first, iterator last, SimObject *obj)
      { return ::find(first, last, obj); }

      /// Reorder the position of "obj" to either be the last object in the list or, if
      /// "target" is given, to come before "target" in the list of children.
      virtual bool reOrder( SimObject *obj, SimObject *target=0 );
      
      /// Return the object at the given index.
      SimObject* at(S32 index) const { return objectList.at(index); }

      /// Remove all objects from this set.
      virtual void clear();
      
      /// @}

      /// @name Set Management
      /// @{

      /// Add the given object to the set.
      /// @param object Object to add to the set.
      virtual void addObject( SimObject* object );
      
      /// Remove the given object from the set.
      /// @param object Object to remove from the set.
      virtual void removeObject( SimObject* object );

      /// Add the given object to the end of the object list of this set.
      /// @param object Object to add to the set.
      virtual void pushObject( SimObject* object );
      
      /// Return true if this set accepts the given object as a child.
      /// This method should be overridden for set classes that restrict membership.
      virtual bool acceptsAsChild( SimObject* object ) const { return true; }
      
      /// Deletes all the objects in the set.
      void deleteAllObjects();

      /// Remove an object from the end of the list.
      virtual void popObject();

      void bringObjectToFront(SimObject* obj) { reOrder(obj, front()); }
      void pushObjectToBack(SimObject* obj) { reOrder(obj, NULL); }

      /// Performs a sort of the objects in the set using a script
      /// callback function to do the comparison.
      ///
      /// An example script sort callback:
      ///
      /// @code
      ///   function sortByName( %object1, %object2 )
      ///   {
      ///      return strcmp( %object1.getName(), %object2.getName() );
      ///   }
      /// @endcode
      ///
      /// Note: You should never modify the SimSet itself while in
      /// the sort callback function as it can cause a deadlock.
      ///
      void scriptSort( const String &scriptCallbackFn );
      
      /// @}

      void callOnChildren( const String &method, S32 argc, const char *argv[], bool executeOnChildGroups = true );

      /// Return the number of objects in this set as well as all sets that are contained
      /// in this set and its children.
      ///
      /// @note The child sets themselves count towards the total too.
      U32 sizeRecursive();

      SimObject* findObjectByInternalName(StringTableEntry internalName, bool searchChildren = false);
      SimObject* findObjectByLineNumber(const char* fileName, S32 declarationLine, bool searchChildren = false);   

      /// Find the given object in this set.  Returns NULL if the object
      /// is not part of this set.
      SimObject* findObject( SimObject* object );

      /// Add all child objects ( including children of children ) to the foundObjects
      /// Vector which are of type T.

      template< class T >
      void findObjectByType( Vector<T*> &foundObjects );

      /// Add all child objects ( including children of children ) to the foundObjects
      /// Vector which are of type T and for which DecideAddObjectCallback return true;   

      template< class T > 
      void findObjectByCallback( bool ( *fn )( T* ), Vector<T*>& foundObjects );   
      
      SimObject* getRandom();

      inline void lock()
      {
         #ifdef TORQUE_MULTITHREAD
         Mutex::lockMutex(mMutex);
         #endif
      }

      void unlock()
      {
         #ifdef TORQUE_MULTITHREAD
         Mutex::unlockMutex(mMutex);
         #endif
      }

      #ifdef TORQUE_DEBUG_GUARD
      inline void _setVectorAssoc( const char *file, const U32 line )
      {
         objectList.setFileAssociation( file, line );
      }
      #endif

      // SimObject.
      DECLARE_CONOBJECT( SimSet );

      virtual void onRemove();
      virtual void onDeleteNotify(SimObject *object);

      virtual SimObject* findObject( const char* name );

      virtual void write(Stream &stream, U32 tabStop, U32 flags = 0);
      virtual bool writeObject(Stream *stream);
      virtual bool readObject(Stream *stream);

      virtual SimSet* clone();
};

#ifdef TORQUE_DEBUG_GUARD
#  define SIMSET_SET_ASSOCIATION( x ) x._setVectorAssoc( __FILE__, __LINE__ )
#else
#  define SIMSET_SET_ASSOCIATION( x )
#endif

template< class T >
void SimSet::findObjectByType( Vector<T*> &foundObjects )
{
   T *curObj;
   SimSet *curSet;

   lock();

   // Loop through our child objects.

   SimObjectList::iterator itr = objectList.begin();   

   for ( ; itr != objectList.end(); itr++ )
   {
      curObj = dynamic_cast<T*>( *itr );
      curSet = dynamic_cast<SimSet*>( *itr );

      // If child object is a set, call recursively into it.
      if ( curSet )
         curSet->findObjectByType( foundObjects ); 

      // Add this child object if appropriate.
      if ( curObj )
         foundObjects.push_back( curObj );      
   }

   // Add this object if appropriate.
   curObj = dynamic_cast<T*>(this);
   if ( curObj )      
      foundObjects.push_back( curObj );

   unlock();
}

template< class T > 
void SimSet::findObjectByCallback(  bool ( *fn )( T* ), Vector<T*> &foundObjects )
{
   T *curObj;
   SimSet *curSet;

   lock();

   // Loop through our child objects.

   SimObjectList::iterator itr = objectList.begin();   

   for ( ; itr != objectList.end(); itr++ )
   {
      curObj = dynamic_cast<T*>( *itr );
      curSet = dynamic_cast<SimSet*>( *itr );

      // If child object is a set, call recursively into it.
      if ( curSet )
         curSet->findObjectByCallback( fn, foundObjects ); 

      // Add this child object if appropriate.
      if ( curObj && ( fn == NULL || fn( curObj ) ) )
         foundObjects.push_back( curObj );      
   }

   // Add this object if appropriate.
   curObj = dynamic_cast<T*>(this);
   if ( curObj && ( fn == NULL || fn( curObj ) ) )      
      foundObjects.push_back( curObj );

   unlock();
}

/// An iterator that recursively and exhaustively traverses the contents
/// of a SimSet.
///
/// @see SimSet
class SimSetIterator
{
protected:
   struct Entry {
      SimSet* set;
      SimSet::iterator itr;
   };
   class Stack: public Vector<Entry> {
   public:
      void push_back(SimSet*);
   };
   Stack stack;

public:
   SimSetIterator(SimSet*);
   SimObject* operator++();
   SimObject* operator*() {
      return stack.empty()? 0: *stack.last().itr;
   }
};

//---------------------------------------------------------------------------
/// A group of SimObjects.
///
/// A SimGroup is a stricter form of SimSet. SimObjects may only be a member
/// of a single SimGroup at a time.
///
/// The SimGroup will automatically enforce the single-group-membership rule.
///
/// @code
///      // From engine/sim/simPath.cc - getting a pointer to a SimGroup
///      SimGroup* pMissionGroup = dynamic_cast<SimGroup*>(Sim::findObject("MissionGroup"));
///
///      // From game/trigger.cc:46 - iterating over a SimObject's group.
///      SimObject* trigger = ...;
///      SimGroup* pGroup = trigger->getGroup();
///      for (SimGroup::iterator itr = pGroup->begin(); itr != pGroup->end(); itr++)
///      {
///         // do something with *itr
///      }
/// @endcode
class SimGroup: public SimSet
{
   public:
   
      typedef SimSet Parent;

      friend class SimManager;
      friend class SimObject;

   private:

      SimNameDictionary mNameDictionary;

      void _addObject( SimObject* object, bool forcePushBack = false );
      void _removeObjectNoLock( SimObject* );

   public:

      ~SimGroup();

      void addObject( SimObject* object, SimObjectId id);
      void addObject( SimObject* object, const char* name );

      // SimSet.
      virtual void addObject( SimObject* object );
      virtual void removeObject( SimObject* object );
      virtual void pushObject( SimObject* object );
      virtual void popObject();
      virtual void clear();
      
      virtual SimGroup* clone();
      virtual SimGroup* deepClone();

      virtual SimObject* findObject(const char* name);
      virtual void onRemove();

      virtual bool processArguments( S32 argc, const char** argv );

      DECLARE_CONOBJECT( SimGroup );
};

inline void SimGroup::addObject(SimObject* obj, SimObjectId id)
{
   obj->mId = id;
   dSprintf( obj->mIdString, sizeof( obj->mIdString ), "%u", obj->mId );
   addObject( obj );
}

inline void SimGroup::addObject(SimObject *obj, const char *name)
{
   addObject( obj );
   obj->assignName(name);
}

/// An iterator that recursively and exhaustively traverses all objects
/// in an SimGroup object tree.
class SimGroupIterator: public SimSetIterator
{
public:
   SimGroupIterator(SimGroup* grp): SimSetIterator(grp) {}
   SimObject* operator++();
};

#endif // _SIMSET_H_
