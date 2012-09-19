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
#include "console/simObject.h"
#include "console/consoleTypes.h"
#include "component/simComponent.h"
#include "core/stream/stream.h"

SimComponent::SimComponent() : mOwner( NULL )
{
   mComponentList.clear();
   mMutex = Mutex::createMutex();
   
   mEnabled = true;
   mTemplate = false;
}

SimComponent::~SimComponent()
{
   Mutex::destroyMutex( mMutex );
   mMutex = NULL;
}

IMPLEMENT_CO_NETOBJECT_V1(SimComponent);

ConsoleDocClass( SimComponent,
				"@brief Legacy component system, soon to be deprecated.\n\n"
				"Not intended for game development, for editors or internal use only.\n\n "
				"@internal");

bool SimComponent::onAdd()
{
   if( !Parent::onAdd() )
      return false;

   // Register
   _registerInterfaces( this );

   if( !_registerComponents( this ) )
      return false;

   //Con::executef( this, 1, "onAdd" );

   return true;
}

void SimComponent::_registerInterfaces( SimComponent *owner )
{
   // First call this to expose the interfaces that this component will cache
   // before examining the list of subcomponents
   registerInterfaces( owner );

   // Early out to avoid mutex lock and such
   if( !hasComponents() )
      return;

   VectorPtr<SimComponent *> &components = lockComponentList();
   for( SimComponentIterator i = components.begin(); i != components.end(); i++ )
   {
      (*i)->mOwner = owner;

      // Tell the component itself to register it's interfaces
      (*i)->registerInterfaces( owner );

      (*i)->mOwner = NULL; // This tests to see if the object's onComponentRegister call will call up to the parent.

      // Recurse
      (*i)->_registerInterfaces( owner );
   }

   unlockComponentList();
}

bool SimComponent::_registerComponents( SimComponent *owner )
{
   // This method will return true if the object contains no components. See the
   // documentation for SimComponent::onComponentRegister for more information
   // on this behavior.
   bool ret =  true;

   // If this doesn't contain components, don't even lock the list.
   if( hasComponents() )
   {
      VectorPtr<SimComponent *> &components = lockComponentList();
      for( SimComponentIterator i = components.begin(); i != components.end(); i++ )
      {
         if( !(*i)->onComponentRegister( owner ) )
         {
            ret = false;
            break;
         }

         AssertFatal( (*i)->mOwner == owner, "Component failed to call parent onComponentRegister!" );

         // Recurse
         if( !(*i)->_registerComponents( owner ) )
         {
            ret = false;
            break;
         }
      }

      unlockComponentList();
   }

   return ret;
}

void SimComponent::_unregisterComponents()
{
   if( !hasComponents() )
      return;

   VectorPtr<SimComponent *> &components = lockComponentList();
   for( SimComponentIterator i = components.begin(); i != components.end(); i++ )
   {
      (*i)->onComponentUnRegister();

      AssertFatal( (*i)->mOwner == NULL, "Component failed to call parent onUnRegister" );

      // Recurse
      (*i)->_unregisterComponents();
   }

   unlockComponentList();
}

void SimComponent::onRemove()
{
   //Con::executef(this, 1, "onRemove");

   _unregisterComponents();

   // Delete all components
   VectorPtr<SimComponent *>&componentList = lockComponentList();
   while(componentList.size() > 0)
   {
      SimComponent *c = componentList[0];
      componentList.erase( componentList.begin() );

      if( c->isProperlyAdded() )
         c->deleteObject();
      else if( !c->isRemoved() && !c->isDeleted() )
         delete c;
      // else, something else is deleting this, don't mess with it
   }
   unlockComponentList();

   Parent::onRemove();
}

//////////////////////////////////////////////////////////////////////////

bool SimComponent::processArguments(S32 argc, const char **argv)
{
   for(S32 i = 0; i < argc; i++)
   {
      SimComponent *obj = dynamic_cast<SimComponent*> (Sim::findObject(argv[i]) );
      if(obj)
         addComponent(obj);
      else
         Con::printf("SimComponent::processArguments - Invalid Component Object \"%s\"", argv[i]);
   }
   return true;
}

//////////////////////////////////////////////////////////////////////////

void SimComponent::initPersistFields()
{
    addGroup("Component");

        addProtectedField( "Template", TypeBool, Offset(mTemplate, SimComponent), 
           &setIsTemplate, &defaultProtectedGetFn, 
           "Places the object in a component set for later use in new levels." );

    endGroup("Component");

    // Call Parent.
    Parent::initPersistFields();
} 

//------------------------------------------------------------------------------

bool SimComponent::getInterfaces( ComponentInterfaceList *list, const char *type /* = NULL */, const char *name /* = NULL  */,
                                 const SimComponent *owner /* = NULL */, bool notOwner /* = false */ )
{
   AssertFatal( list != NULL, "Passing NULL for a list is not supported functionality for SimComponents." );
   return ( mInterfaceCache.enumerate( list, type, name, owner, notOwner ) > 0 );
}

bool SimComponent::registerCachedInterface( const char *type, const char *name, SimComponent *interfaceOwner, ComponentInterface *cinterface )
{
   if( mInterfaceCache.add( type, name, interfaceOwner, cinterface ) )
   {
      cinterface->mOwner = interfaceOwner;

      // Recurse
      if( mOwner != NULL )
         return mOwner->registerCachedInterface( type, name, interfaceOwner, cinterface );

      return true;
   }

   // So this is not a good assert, because it will get triggered due to the recursive
   // nature of interface caching. I want to keep it here, though, just so nobody
   // else thinks, "Oh I'll add an assert here."
   //
   //AssertFatal( false, avar( "registerCachedInterface failed, probably because interface with type '%s', name '%s' and owner with SimObjectId '%d' already exists",
   //   type, name, interfaceOwner->getId() ) );

   return false;
}

//////////////////////////////////////////////////////////////////////////
// Component Management
//////////////////////////////////////////////////////////////////////////

bool SimComponent::addComponentFromField( void* obj, const char* data )
{
   SimComponent *pComponent = dynamic_cast<SimComponent*>( Sim::findObject( data ) );
   if( pComponent != NULL )
      static_cast<SimComponent*>(obj)->addComponent( pComponent ); 
   return false;
}

// Add Component to this one
bool SimComponent::addComponent( SimComponent *component )
{
   AssertFatal( dynamic_cast<SimObject*>(component), "SimComponent - Cannot add non SimObject derived components!" );

   MutexHandle mh;
   if( mh.lock( mMutex, true ) )
   {
      for( SimComponentIterator nItr = mComponentList.begin(); nItr != mComponentList.end(); nItr++ )
      {
         SimComponent *pComponent = dynamic_cast<SimComponent*>(*nItr);
         AssertFatal( pComponent, "SimComponent::addComponent - NULL component in list!" );
         if( pComponent == component )
            return true;
      }

      if(component->onComponentAdd(this))
      {
         component->mOwner = this;
         mComponentList.push_back( component );
         return true;
      }
   }

   return false;
}

// Remove Component from this one
bool SimComponent::removeComponent( SimComponent *component )
{
   MutexHandle mh;
   if( mh.lock( mMutex, true ) )
   {
      for( SimComponentIterator nItr = mComponentList.begin(); nItr != mComponentList.end(); nItr++ )
      {
         SimComponent *pComponent = dynamic_cast<SimComponent*>(*nItr);
         AssertFatal( pComponent, "SimComponent::removeComponent - NULL component in list!" );
         if( pComponent == component )
         {
            AssertFatal( component->mOwner == this, "Somehow we contain a component who doesn't think we are it's owner." );
            (*nItr)->onComponentRemove(this);
            component->mOwner = NULL;
            mComponentList.erase( nItr );
            return true;
         }
      }
   }
   return false;
}

//////////////////////////////////////////////////////////////////////////

bool SimComponent::onComponentAdd(SimComponent *target)
{
   Con::executef(this, "onComponentAdd", Con::getIntArg(target->getId()));
   return true;
}

void SimComponent::onComponentRemove(SimComponent *target)
{
   Con::executef(this, "onComponentRemove", Con::getIntArg(target->getId()));
}

//////////////////////////////////////////////////////////////////////////

ComponentInterface *SimComponent::getInterface(const char *type /* = NULL */, const char *name /* = NULL */, 
                                               const SimComponent *owner /* = NULL */, bool notOwner /* = false  */)
{
   ComponentInterfaceList iLst;

   if( getInterfaces( &iLst, type, name, owner, notOwner ) )
      return iLst[0];

   return NULL;
}

//////////////////////////////////////////////////////////////////////////

bool SimComponent::writeField(StringTableEntry fieldname, const char* value)
{
   if (!Parent::writeField(fieldname, value))
      return false;

   if( fieldname == StringTable->insert("owner") )
      return false;

   return true;
}

void SimComponent::write(Stream &stream, U32 tabStop, U32 flags /* = 0 */)
{
   MutexHandle handle;
   handle.lock(mMutex); // When this goes out of scope, it will unlock it

   // export selected only?
   if((flags & SelectedOnly) && !isSelected())
   {
      for(U32 i = 0; i < mComponentList.size(); i++)
         mComponentList[i]->write(stream, tabStop, flags);

      return;
   }

   stream.writeTabs(tabStop);
   char buffer[1024];
   dSprintf(buffer, sizeof(buffer), "new %s(%s) {\r\n", getClassName(), getName() ? getName() : "");
   stream.write(dStrlen(buffer), buffer);
   writeFields(stream, tabStop + 1);

   if(mComponentList.size())
   {
      stream.write(2, "\r\n");

      stream.writeTabs(tabStop+1);
      stream.writeLine((U8 *)"// Note: This is a list of behaviors, not arbitrary SimObjects as in a SimGroup or SimSet.\r\n");

      for(U32 i = 0; i < mComponentList.size(); i++)
         mComponentList[i]->write(stream, tabStop + 1, flags);
   }

   stream.writeTabs(tabStop);
   stream.write(4, "};\r\n");
}

//////////////////////////////////////////////////////////////////////////
// Console Methods
//////////////////////////////////////////////////////////////////////////

ConsoleMethod( SimComponent, addComponents, bool, 3, 64, "%obj.addComponents( %compObjName, %compObjName2, ... );\n"
			  "Adds additional components to current list.\n"
			  "@param Up to 62 component names\n"
			  "@return Returns true on success, false otherwise.")
{
   for(S32 i = 2; i < argc; i++)
   {
      SimComponent *obj = dynamic_cast<SimComponent*> (Sim::findObject(argv[i]) );
      if(obj)
         object->addComponent(obj);
      else
         Con::printf("SimComponent::addComponents - Invalid Component Object \"%s\"", argv[i]);
   }
   return true;
}

ConsoleMethod( SimComponent, removeComponents, bool, 3, 64, "%obj.removeComponents( %compObjName, %compObjName2, ... );\n"
			  "Removes components by name from current list.\n"
			  "@param objNamex Up to 62 component names\n"
			  "@return Returns true on success, false otherwise.")
{
   for(S32 i = 2; i < argc; i++)
   {
      SimComponent *obj = dynamic_cast<SimComponent*> (Sim::findObject(argv[i]) );
      if(obj)
         object->removeComponent(obj);
      else
         Con::printf("SimComponent::removeComponents - Invalid Component Object \"%s\"", argv[i]);
   }
   return true;
}

ConsoleMethod( SimComponent, getComponentCount, S32, 2, 2, "() Get the current component count\n"
			  "@return The number of components in the list as an integer")
{
   return object->getComponentCount();
}

ConsoleMethod( SimComponent, getComponent, S32, 3, 3, "(idx) Get the component corresponding to the given index.\n"
			  "@param idx An integer index value corresponding to the desired component.\n"
			  "@return The id of the component at the given index as an integer")
{
   S32 idx = dAtoi(argv[2]);
   if(idx < 0 || idx >= object->getComponentCount())
   {
      Con::errorf("SimComponent::getComponent - Invalid index %d", idx);
      return 0;
   }

   SimComponent *c = object->getComponent(idx);
   return c ? c->getId() : 0;
}

ConsoleMethod(SimComponent, setEnabled, void, 3, 3, "(enabled) Sets or unsets the enabled flag\n"
			  "@param enabled Boolean value\n"
			  "@return No return value")
{
   object->setEnabled(dAtob(argv[2]));
}

ConsoleMethod(SimComponent, isEnabled, bool, 2, 2, "() Check whether SimComponent is currently enabled\n"
			  "@return true if enabled and false if not")
{
   return object->isEnabled();
}

ConsoleMethod(SimComponent, setIsTemplate, void, 3, 3, "(template) Sets or unsets the template flag\n"
			  "@param template Boolean value\n"
			  "@return No return value")
{
   object->setIsTemplate(dAtob(argv[2]));
}

ConsoleMethod(SimComponent, getIsTemplate, bool, 2, 2, "() Check whether SimComponent is currently a template\n"
			  "@return true if is a template and false if not")
{
   return object->getIsTemplate();
}