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

#ifndef _SIMCOMPONENT_H_
#define _SIMCOMPONENT_H_

#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif
#ifndef _STRINGTABLE_H_
#include "core/stringTable.h"
#endif
#ifndef _NETOBJECT_H_
#include "sim/netObject.h"
#endif
#ifndef _COMPONENTINTERFACE_H_
#include "component/componentInterface.h"
#endif
#ifndef _PLATFORM_THREADS_MUTEX_H_
#include "platform/threads/mutex.h"
#endif
#ifndef _STRINGFUNCTIONS_H_
#include "core/strings/stringFunctions.h"
#endif

// Forward refs
class Stream;
class ComponentInterface;
class ComponentInterfaceCache;

class SimComponent : public NetObject
{
   typedef NetObject Parent;

private:
   VectorPtr<SimComponent *> mComponentList; ///< The Component List
   void *mMutex;                             ///< Component List Mutex

   SimObjectPtr<SimComponent> mOwner;        ///< The component which owns this one.

   /// This is called internally to instruct the component to iterate over it's
   // list of components and recursively call _registerInterfaces on their lists
   // of components.
   void _registerInterfaces( SimComponent *owner );

   bool _registerComponents( SimComponent *owner );
   void _unregisterComponents();

protected:
   ComponentInterfaceCache mInterfaceCache;  ///< Stores the interfaces exposed by this component. 
   
   bool mEnabled;
   
   bool mTemplate;

   // Non-const getOwner for derived classes
   SimComponent *_getOwner() { return mOwner; }

   /// Returns a const reference to private mComponentList
   typedef VectorPtr<SimComponent *>::iterator SimComponentIterator;
   VectorPtr<SimComponent *> &lockComponentList()
   {
      Mutex::lockMutex( mMutex );
      return mComponentList;
   };

   void unlockComponentList()
   {
      Mutex::unlockMutex( mMutex );
   }

   /// onComponentRegister is called on each component by it's owner. If a component
   /// has no owner, onComponentRegister will not be called on it. The purpose
   /// of onComponentRegister is to allow a component to check for any external
   /// interfaces, or other dependencies which it needs to function. If any component
   /// in a component hierarchy returns false from it's onComponentRegister call
   /// the entire hierarchy is invalid, and SimObject::onAdd will fail on the
   /// top-level component. To put it another way, if a component contains other
   /// components, it will be registered successfully with Sim iff each subcomponent
   /// returns true from onComponentRegister. If a component does not contain
   /// other components, it will not receive an onComponentRegister call.
   ///
   /// Overloads of this method must pass the call along to their parent, as is
   /// shown in the example below. 
   ///
   /// @code
   /// bool FooComponent::onComponentRegister( SimComponent *owner )
   /// {
   ///    if( !Parent::onComponentRegister( owner ) )
   ///       return false;
   ///    ...
   /// }
   /// @endcode
   virtual bool onComponentRegister( SimComponent *owner )
   {
      mOwner = owner;
      return true;
   }

   /// onUnregister is called when the owner is unregistering. Your object should
   /// do cleanup here, as well as pass a call up the chain to the parent.
   virtual void onComponentUnRegister()
   {
      mOwner = NULL;
   }

   /// registerInterfaces is called on each component as it's owner is registering
   /// it's interfaces. This is called before onComponentRegister, and should be used to
   /// register all interfaces exposed by your component, as well as all callbacks
   /// needed by your component.
   virtual void registerInterfaces( SimComponent *owner )
   {

   }

public:
   DECLARE_CONOBJECT(SimComponent);

   /// Constructor
   /// Add this component
   SimComponent();

   /// Destructor
   /// Remove this component and destroy child references
   virtual ~SimComponent();

public:

   virtual bool onAdd();
   virtual void onRemove();

   static void initPersistFields();

   virtual bool processArguments(S32 argc, const char **argv);
   
   bool isEnabled() const { return mEnabled; }
   
   void setEnabled( bool value ) { mEnabled = value; }

   /// Will return true if this object contains components.
   bool hasComponents() const { return ( mComponentList.size() > 0 ); };

   /// The component which owns this object
   const SimComponent *getOwner() const { return mOwner; };

   // Component Information
   inline virtual StringTableEntry  getComponentName() { return StringTable->insert( getClassName() ); };

   /// Protected 'Component' Field setter that will add a component to the list.
   static bool addComponentFromField(void* obj, const char* data);

   /// Add Component to this one
   virtual bool addComponent( SimComponent *component );

   /// Remove Component from this one
   virtual bool removeComponent( SimComponent *component );

   /// Clear Child components of this one
   virtual bool clearComponents() { mComponentList.clear(); return true; };

   virtual bool onComponentAdd(SimComponent *target);
   virtual void onComponentRemove(SimComponent *target);

   S32 getComponentCount()                   { return mComponentList.size(); }
   SimComponent *getComponent(S32 idx)       { return mComponentList[idx]; }

   SimComponentIterator find(SimComponentIterator first, SimComponentIterator last, SimComponent *value)
   {
      return ::find(first, last, value);
   }

   static bool setIsTemplate( void *object, const char *index, const char *data ) 
      { static_cast<SimComponent*>(object)->setIsTemplate( dAtob( data ) ); return false; };
   virtual void setIsTemplate( const bool pTemplate ) { mTemplate = pTemplate; }
   bool getIsTemplate() const { return mTemplate; }

   virtual void write(Stream &stream, U32 tabStop, U32 flags = 0);
   virtual bool writeField(StringTableEntry fieldname, const char* value);


   /// getInterfaces allows the caller to enumerate the interfaces exposed by
   /// this component. This method can be overwritten to expose interfaces
   /// which are not cached on the object, before passing the call to the Parent.
   /// This can be used delay interface creation until it is queried for, instead
   /// of creating it on initialization, and caching it. Returns false if no results
   /// were found
   ///
   /// @param list The list that this method will append search results on to.
   /// @param type An expression which the 'type' field on an added object must match to be included in results
   /// @param name An expression which the 'name' field on an added object must match to be included in results
   /// @param owner Limit results to components owned/not-owned by this SimComponent (see next param)
   /// @param notOwner If set to true, this will enumerate only interfaces NOT owned by 'owner'
   virtual bool getInterfaces( ComponentInterfaceList *list, const char *type = NULL, const char *name = NULL, const SimComponent *owner = NULL, bool notOwner = false ); // const omission intentional

   
   /// These two methods allow for easy query of component interfaces if you know
   /// exactly what you are looking for, and don't mind being passed back the first
   /// matching result.
   ComponentInterface *getInterface( const char *type = NULL, const char *name = NULL, const SimComponent *owner = NULL, bool notOwner = false );

   template <class T>
   T *getInterface( const char *type = NULL, const char *name = NULL, const SimComponent *owner = NULL, bool notOwner = false );

   /// Add an interface to the cache. This function will return true if the interface
   /// is added successfully. An interface will not be added successfully if an entry
   /// in this components cache with the same values for 'type' and 'name' is present.
   /// 
   /// @param type Type of the interface being added. If NULL is passed, it will match any type string queried.
   /// @param name Name of interface being added. If NULL is passed, it will match any name string queried.
   /// @param interfaceOwner The component which owns the interface being cached
   /// @param cinterface The ComponentInterface being cached
   bool registerCachedInterface( const char *type, const char *name, SimComponent *interfaceOwner, ComponentInterface *cinterface );
};

//////////////////////////////////////////////////////////////////////////

template <class T>
T *SimComponent::getInterface( const char *type /* = NULL */, const char *name /* = NULL */, 
                              const SimComponent *owner /* = NULL */, bool notOwner /* = false  */ )
{
   ComponentInterfaceList iLst;

   if( getInterfaces( &iLst, type, name, owner, notOwner ) )
   {
      ComponentInterfaceListIterator itr = iLst.begin();

      while( dynamic_cast<T *>( *itr ) == NULL )
         itr++;

      if( itr != iLst.end() )
         return static_cast<T *>( *itr );
   }

   return NULL;
}

#endif // _SIMCOMPONENT_H_
