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

#include "component/simpleComponent.h"

IMPLEMENT_CONOBJECT(SimpleComponent);

ConsoleDocClass( SimpleComponent,
				"@brief The purpose of this component is to provide a minimalistic component that "
				"exposes a simple, cached interface\n\n"
				"Soon to be deprecated, internal only.\n\n "
				"@internal");

//////////////////////////////////////////////////////////////////////////
// It may seem like some weak sauce to use a unit test for this, however
// it is very, very easy to set breakpoints in a unit test, and trace 
// execution in the debugger, so I will use a unit test.
//
// Note I am not using much actual 'test' functionality, just providing
// an easy place to examine the functionality that was described and implemented
// in the header file.
//
// If you want to run this code, simply run Torque, pull down the console, and
// type:
//    unitTest_runTests("Components/SimpleComponent");

#include "unit/test.h"
using namespace UnitTesting;

CreateUnitTest(TestSimpleComponent, "Components/SimpleComponent")
{
   void run()
   {
      // When instantiating, and working with a SimObject in C++ code, such as
      // a unit test, you *may not* allocate a SimObject off of the stack. 
      //
      // For example:
      //    SimpleComponent sc; 
      // is a stack allocation. This memory is allocated off of the program stack
      // when the function is called. SimObject deletion is done via SimObject::deleteObject()
      // and the last command of this method is 'delete this;' That command will
      // cause an assert if it is called on stack-allocated memory. Therefor, when
      // instantiating SimObjects in C++ code, it is imperitive that you keep in
      // mind that if any script calls 'delete()' on that SimObject, or any other
      // C++ code calls 'deleteObject()' on that SimObject, it will crash.
      SimpleComponent *sc = new SimpleComponent();

      // SimObject::registerObject must be called on a SimObject before it is
      // fully 'hooked in' to the engine. 
      //
      // Tracing execution of this function will let you see onAdd get called on
      // the component, and you will see it cache the interface we exposed.
      sc->registerObject();

      // It is *not* required that a component always be owned by a component (obviously)
      // however I am using an owner so that you can trace execution of recursive
      // calls to cache interfaces and such.
      SimComponent *testOwner = new SimComponent();

      // Add the test component to it's owner. This will set the 'mOwner' field
      // of 'sc' to the address of 'testOwner'
      testOwner->addComponent( sc );

      // If you step-into this registerObject the same way as the previous one, 
      // you will be able to see the recursive caching of the exposed interface.
      testOwner->registerObject();

      // Now to prove that object composition is working properly, lets ask 
      // both of these components for their interface lists...

      // The ComponentInterfaceList is a typedef for type 'VectorPtr<ComponentInterface *>'
      // and it will be used by getInterfaces() to store the results of the interface
      // query. This is the "complete" way to obtain an interface, and it is too
      // heavy-weight for most cases. A simplified query will be performed next, 
      // to demonstrate the usage of both.
      ComponentInterfaceList iLst;

      // This query requests all interfaces, on all components, regardless of name
      // or owner.
      sc->getInterfaces( &iLst, 
         // This is the type field. I am passing NULL here to signify that the query
         // should match all values of 'type' in the list.
         NULL,

         // The name field, let's pass NULL again just so when you trace execution
         // you can see how queries work in the simple case, first.
         NULL );

      // Lets process the list that we've gotten back, and find the interface that
      // we want.
      SimpleComponentInterface *scQueriedInterface = NULL;

      for( ComponentInterfaceListIterator i = iLst.begin(); i != iLst.end(); i++ )
      {
         scQueriedInterface = dynamic_cast<SimpleComponentInterface *>( *i );

         if( scQueriedInterface != NULL )
            break;
      }

      AssertFatal( scQueriedInterface != NULL, "No valid SimpleComponentInterface was found in query" );

      // Lets do it again, only we will execute the query on the parent instead,
      // in a simplified way. Remember the parent component doesn't expose any 
      // interfaces at all, so the success of this behavior is entirely dependent 
      // on the recursive registration that occurs in registerInterfaces()
      SimpleComponentInterface *ownerQueriedInterface = testOwner->getInterface<SimpleComponentInterface>();

      AssertFatal( ownerQueriedInterface != NULL, "No valid SimpleComponentInterface was found in query" );

      // We should now have two pointers to the same interface obtained by querying
      // different components.
      test( ownerQueriedInterface == scQueriedInterface, "This really shouldn't be possible to fail given the setup of the test" );

      // Lets call the method that was exposed on the component via the interface.
      // Trace the execution of this function, if you wish.
      test( ownerQueriedInterface->isFortyTwo( 42 ), "Don't panic, but it's a bad day in the component system." );
      test( scQueriedInterface->isFortyTwo( 42 ), "Don't panic, but it's a bad day in the component system." );

      // So there you have it. Writing a simple component that exposes a cached
      // interface, and testing it. It's time to clean up.
      testOwner->removeComponent( sc );

      sc->deleteObject();
      testOwner->deleteObject();

      // Interfaces do not need to be freed. In Juggernaught, these will be ref-counted
      // for more robust behavior. Right now, however, the values of our two interface
      // pointers, scQueriedInterface and ownerQueriedInterface, reference invalid
      // memory. 
   }
};