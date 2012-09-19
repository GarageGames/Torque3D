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

#include "unit/test.h"
#include "unit/memoryTester.h"
#include "component/simComponent.h"

using namespace UnitTesting;

//////////////////////////////////////////////////////////////////////////

class CachedInterfaceExampleComponent : public SimComponent
{
   typedef SimComponent Parent;

   ComponentProperty<U32> mMyId;
   static U32 smNumInstances;
   ComponentProperty<U32> *mpU32; // CodeReview [patw, 2, 17, 2007] Make ref objects when this is in Jugg

public:
   DECLARE_CONOBJECT( CachedInterfaceExampleComponent );

   CachedInterfaceExampleComponent() : mpU32( NULL )
   {
      mMyId = ( ( 1 << 24 ) | smNumInstances++ );
   }
   virtual ~CachedInterfaceExampleComponent()
   {
      smNumInstances--;
   }

public:
   //////////////////////////////////////////////////////////////////////////

   virtual void registerInterfaces( const SimComponent *owner )
   {
      // Register a cached interface for this 
      registerCachedInterface( NULL, "aU32", this, &mMyId );
   }

   //////////////////////////////////////////////////////////////////////////

   bool onComponentRegister( SimComponent *owner )
   {
      // Call up to the parent first
      if( !Parent::onComponentRegister( owner ) )
         return false;

      // We want to get an interface from another object in our containing component
      // to simulate component interdependency. 
      ComponentInterfaceList list;

      // Enumerate the interfaces on the owner, only ignore interfaces that this object owns
      if( !_getOwner()->getInterfaces( &list, NULL, "aU32", this, true ) )
         return false;

      // Sanity check before just assigning all willy-nilly
      for( ComponentInterfaceListIterator i = list.begin(); i != list.end(); i++ )
      {
         mpU32 = dynamic_cast<ComponentProperty<U32> *>( (*i) );

         if( mpU32 != NULL )
            return true;
      }

      return false;
   }

   //////////////////////////////////////////////////////////////////////////

   // CodeReview [patw, 2, 17, 2007] I'm going to make another lightweight interface
   // for this functionality later
   void unit_test( UnitTest *test )
   {
      test->test( mpU32 != NULL, "Pointer to dependent interface is NULL" );
      test->test( *(*mpU32) & ( 1 << 24 ), "Pointer to interface data is bogus." );
      test->test( *(*mpU32) != *mMyId, "Two of me have the same ID, bad!" );
   }
};

IMPLEMENT_CONOBJECT( CachedInterfaceExampleComponent );
U32 CachedInterfaceExampleComponent::smNumInstances = 0;

ConsoleDocClass( CachedInterfaceExampleComponent,
				"@brief Legacy from older component system.\n\n"
				"Not intended for game development, for editors or internal use only.\n\n "
				"@internal");

//////////////////////////////////////////////////////////////////////////

CreateUnitTest(TestComponentInterfacing, "Components/Composition")
{
   void run()
   {
      MemoryTester m;
      m.mark();

      SimComponent *testComponent = new SimComponent();
      CachedInterfaceExampleComponent *componentA = new CachedInterfaceExampleComponent();
      CachedInterfaceExampleComponent *componentB = new CachedInterfaceExampleComponent();

      // Register sub-components
      test( componentA->registerObject(), "Failed to register componentA" );
      test( componentB->registerObject(), "Failed to register componentB" );

      // Add the components
      test( testComponent->addComponent( componentA ), "Failed to add component a to testComponent" );
      test( testComponent->addComponent( componentB ), "Failed to add component b to testComponent" );

      test( componentA->getOwner() == testComponent, "testComponent did not properly set the mOwner field of componentA to NULL." );
      test( componentB->getOwner() == testComponent, "testComponent did not properly set the mOwner field of componentB to NULL." );

      // Register the object with the simulation, kicking off the interface registration
      test( testComponent->registerObject(), "Failed to register testComponent" );

      // Interface tests
      componentA->unit_test( this );
      componentB->unit_test( this );

      testComponent->deleteObject();

      test( m.check(), "Component composition test leaked memory." );
   }
};