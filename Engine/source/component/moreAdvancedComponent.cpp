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