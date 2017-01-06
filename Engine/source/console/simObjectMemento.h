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

#ifndef _CONSOLE_SIMOBJECTMEMENTO_H_
#define _CONSOLE_SIMOBJECTMEMENTO_H_

#ifndef _SIM_H_
#include "console/sim.h"
#endif


/// This simple class is used to store an SimObject and 
/// its state so it can be recreated at a later time.
///
/// The success of restoring the object completely depends
/// on the results from SimObject::write().
class SimObjectMemento
{
protected:

   /// The captured object state.
   UTF8 *mState;

   /// The captured object's name.
   String mObjectName;
   bool mIsDatablock;

public:

   SimObjectMemento();
   virtual ~SimObjectMemento();

   /// Returns true if we have recorded state.
   bool hasState() const { return mState; }

   ///
   void save( SimObject *object );

   ///
   SimObject *restore() const;

};


#endif // _CONSOLE_SIMOBJECTMEMENTO_H_
