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

#ifndef _SIMPLECOMPONENT_H_
#define _SIMPLECOMPONENT_H_

#ifndef _SIMCOMPONENT_H_
#include "component/simComponent.h"
#endif

#ifndef _COMPONENTINTERFACE_H_
#include "component/componentInterface.h"
#endif

/// This is a very simple interface. Interfaces provide ways for components to 
/// interact with each-other, and query each-other for functionality. It makes it
/// possible for two components to be interdependent on one another, as well. An
/// interface should make accessor calls to it's owner for functionality, and 
/// generally be as thin of a layer as possible. 
class SimpleComponentInterface : public ComponentInterface
{
public:
   bool isFortyTwo( const U32 test );
};

//////////////////////////////////////////////////////////////////////////
/// The purpose of this component is to provide a minimalistic component that 
/// exposes a simple, cached interface
class SimpleComponent : public SimComponent
{
   typedef SimComponent Parent;

protected:
   SimpleComponentInterface mSCInterface;

public:
   // Components are still SimObjects, and need to be declared and implemented
   // with the standard macros
   DECLARE_CONOBJECT(SimpleComponent);

   //////////////////////////////////////////////////////////////////////////
   // SimComponent overloads. 

   // This method is called on each component as it's parent is getting onAdd
   // called. The purpose of overloading this method is to expose cached interfaces
   // before onComponentRegister is called, so that other components can depend on the
   // interfaces you expose in order to register properly. This functionality
   // will be demonstrated in a more advanced example.
   virtual void registerInterfaces( SimComponent *owner )
   {
      // While it is not imperative that we pass this call to the Parent in this
      // example, if there existed a class-heirarchy of components, it would be
      // critical, so for good practice, call up to the Parent.
      Parent::registerInterfaces( owner );

      // Now we should go ahead and register our cached interface. What we are doing
      // is telling the component which contains this component (if it exists)
      // all of the interfaces that we expose. When this call is made, it will 
      // recurse up the owner list. 
      //
      // For example, there exists components A, B, and C. 
      // A owns B, and B owns C.
      // 
      // If C exposes a cached interface, it will expose it via registerCachedInterface
      // when registerInterfaces is recursively called. It will add it's interface to 
      // it's cache list, and then pass the register call up to it's parent. The parent
      // will also cache the interface, and continue to pass the cache call up the 
      // child->parent chain until there exists no parent. 
      //
      // The result is that, if C exposes an interface 'foo', and A owns B, and 
      // B owns C, an interface request for 'foo' given to component A will result
      // in 'foo' being returned, even though A does not expose 'foo'. This makes
      // it possible for a component to query it's owner for an interface, and
      // not care where that interface is exposed. It also allows for game code
      // to work with any SimComponent and query that component for any interface
      // it wants without knowing or caring exactly where it is coming from.
      //
      // registerCachedInterface returns a boolean value if it was successful. 
      // Success results in the caching of this interface throughout the full
      // child->parent chain. An interface will be added to a cache list 
      // successfully iff there exists no entry in that list that has matching
      // values for 'type', 'name' and 'owner'. 
      owner->registerCachedInterface( 
         // The first parameter is the 'type' of the interface, this is not to be
         // confused with any kind of existing console, or c++ type. It is simply
         // a string which is can be set to any value
         "example", 

         // The next parameter is the 'name' of the interface. This is also a string
         // which can be set to any value
         "isfortytwo", 
         
         // The owner of the interface. Note that the value being assigned here
         // is this instance of SimpleComponent, and not the 'owner' argument
         // of the function registerInterfaces that we are calling from.
         this, 
         
         // And finally the interface; a pointer to an object with type ComponentInterface
         &mSCInterface );
   }

   //////////////////////////////////////////////////////////////////////////
   // Specific functionality

   /// This is the test method, it will return true if the number provided
   /// is forty two
   bool isFortyTwo( const U32 test ) const
   {
      return ( test == 42 );
   }
};

//////////////////////////////////////////////////////////////////////////
// Interface implementation
//
// Since interfaces themselves implement very little functionality, it is a good
// idea to inline them if at all possible. Interdependent components will be using
// these interfaces constantly, and so putting as thin of a layer between the 
// functionality they expose, and the functionality the component implements is
// a good design practice.
inline bool SimpleComponentInterface::isFortyTwo( const U32 test )
{
   // This code block will test for a valid owner in a debug build before
   // performing operations on it's owner. It is worth noting that the
   // ComponentInterface::isValid() method can be overridden to include
   // validation specific to your interface and/or component.
   AssertFatal( isValid(), "SimpleComponentInterface has not been registered properly by the component which exposes it." );

   // This is a sanity check. The owner of this interface should have the type
   // SimpleComponent, otherwise this won't work. (See further interface examples
   // for some ways around this)
   AssertFatal( dynamic_cast<SimpleComponent *>( getOwner() ) != NULL, "Owner of SimpleComponentInterface is not a SimpleComponent" );

   // Component interfaces rely on being registered to set their mOwner
   // field. This field is initialized to NULL, and then gets set by
   // SimComponent when the interface is registered.
   return static_cast<const SimpleComponent *>( getOwner() )->isFortyTwo( test );
}

#endif