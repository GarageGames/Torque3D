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

#ifndef _MOREADVANCEDCOMPONENT_H_
#define _MOREADVANCEDCOMPONENT_H_

#ifndef _SIMPLECOMPONENT_H_
#include "component/simpleComponent.h"
#endif

/// This is a slightly more advanced component which will be used to demonstrate
/// components which are dependent on other components.
class MoreAdvancedComponent : public SimComponent
{
   typedef SimComponent Parent;

protected:
   // This component is going to be dependent on a SimpleComponentInterface being
   // queried off of it's parent object. This will store that interface that
   // will get queried during onComponentRegister()
   SimpleComponentInterface *mSCInterface;

public:
   DECLARE_CONOBJECT(MoreAdvancedComponent);

   // Firstly, take a look at the documentation for this function in simComponent.h.
   // We will be overloading this method to query the component heirarchy for our
   // dependent interface, as noted above.
   virtual bool onComponentRegister( SimComponent *owner );

   // This function will try to execute a function through the interface that this
   // component is dependent on.
   virtual bool testDependentInterface();
};

#endif