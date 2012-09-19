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

#include "component/moreAdvancedComponent.h"
#include "unit/test.h"

// unitTest_runTests("Component/MoreAdvancedComponent");

//////////////////////////////////////////////////////////////////////////

IMPLEMENT_CONOBJECT(MoreAdvancedComponent);

ConsoleDocClass( MoreAdvancedComponent,
				"@brief This is a slightly more advanced component which will be used to demonstrate "
				"components which are dependent on other components.\n\n"
				"Not intended for game development, for editors or internal use only.\n\n "
				"@internal");

bool MoreAdvancedComponent::onComponentRegister( SimComponent *owner )
{
   if( !Parent::onComponentRegister( owner ) )
      return false;

   // This will return the first interface of type SimpleComponent that is cached
   // on the parent object. 
   mSCInterface = owner->getInterface<SimpleComponentInterface>();

   // If we can't find this interface, our component can't function, so false
   // will be returned, and this will signify, to the top-level component, that it
   // should fail the onAdd call.
   return ( mSCInterface != NULL );
}

bool MoreAdvancedComponent::testDependentInterface()
{
   // These two requirements must be met in order for the test to proceed, so
   // lets check them.
   if( mSCInterface == NULL || !mSCInterface->isValid() )
      return false;

   return mSCInterface->isFortyTwo( 42 );
}

//////////////////////////////////////////////////////////////////////////

using namespace UnitTesting;

CreateUnitTest(MoreAdvancedComponentTest, "Component/MoreAdvancedComponent")
{
   void run()
   {
      // Create component instances and compose them.
      SimComponent *parentComponent = new SimComponent();
      SimpleComponent *simpleComponent = new SimpleComponent();
      MoreAdvancedComponent *moreAdvComponent = new MoreAdvancedComponent();
      // CodeReview note that the interface pointer isn't initialized in a ctor
      //  on the components, so it's bad memory against which you might
      //  be checking in testDependentInterface [3/3/2007 justind]
      parentComponent->addComponent( simpleComponent );
      parentComponent->addComponent( moreAdvComponent );

      simpleComponent->registerObject();
      moreAdvComponent->registerObject();

      // Put a break-point here, follow the onAdd call, and observe the order in
      // which the SimComponent::onAdd function executes. You will see the interfaces
      // get cached, and the dependent interface query being made.
      parentComponent->registerObject();

      // If the MoreAdvancedComponent found an interface, than the parentComponent
      // should have returned true, from onAdd, and should therefore be registered
      // properly with the Sim
      test( parentComponent->isProperlyAdded(), "Parent component not properly added!" );

      // Now lets test the interface. You can step through this, as well.
      test( moreAdvComponent->testDependentInterface(), "Dependent interface test failed." ); 

      // CodeReview is there a reason we can't just delete the parentComponent here? [3/3/2007 justind]
      //
      // Clean up
      parentComponent->removeComponent( simpleComponent );
      parentComponent->removeComponent( moreAdvComponent );

      parentComponent->deleteObject();
      moreAdvComponent->deleteObject();
      simpleComponent->deleteObject();
   }
};