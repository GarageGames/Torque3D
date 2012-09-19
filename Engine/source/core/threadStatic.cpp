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

#include "core/threadStatic.h"

//-----------------------------------------------------------------------------
// Statics
U32 _TorqueThreadStatic::mListIndex = 0;

_TorqueThreadStaticReg *_TorqueThreadStaticReg::smFirst = NULL;
//-----------------------------------------------------------------------------

inline Vector<TorqueThreadStaticList> &_TorqueThreadStaticReg::getThreadStaticListVector()
{
   // This function assures that the static vector of ThreadStatics will get initialized
   // before first use.
   static Vector<TorqueThreadStaticList> sTorqueThreadStaticVec( __FILE__, __LINE__ );

   return sTorqueThreadStaticVec;
}

//-----------------------------------------------------------------------------

// Destructor, size should == 1 otherwise someone didn't clean up, or someone
// did horrible things to list index 0
_TorqueThreadStaticReg::~_TorqueThreadStaticReg()
{
   AssertFatal( getThreadStaticListVector().size() == 1, "Destruction of static list was not performed on program exit" );
}

//-----------------------------------------------------------------------------

void _TorqueThreadStaticReg::destroyInstances()
{
   // mThreadStaticInstances[0] does *not* need to be deallocated
   // because all members of the list are pointers to static memory
   while( getThreadStaticListVector().size() > 1 )
   {
      // Delete the members of this list
      while( getThreadStaticListVector().last().size() )
      {
         _TorqueThreadStatic *biscuit = getThreadStaticListVector().last().first();

         // Erase the vector entry
         getThreadStaticListVector().last().pop_front();

         // And finally the memory
         delete biscuit;
      }

      // Remove the entry from the list of lists
      getThreadStaticListVector().pop_back();
   }
}

//-----------------------------------------------------------------------------

void _TorqueThreadStaticReg::destroyInstance( TorqueThreadStaticList *instanceList )
{
   AssertFatal( instanceList != &getThreadStaticListVector().first(), "Cannot delete static instance list index 0" );

   while( instanceList->size() )
   {
      _TorqueThreadStatic *biscuit = getThreadStaticListVector().last().first();

      // Erase the vector entry
      getThreadStaticListVector().last().pop_front();

      // And finally the memory
      delete biscuit;
   }

   getThreadStaticListVector().erase( instanceList );
}

//-----------------------------------------------------------------------------

TorqueThreadStaticListHandle _TorqueThreadStaticReg::spawnThreadStaticsInstance()
{
   AssertFatal( getThreadStaticListVector().size() > 0, "List is not initialized somehow" );

   // Add a new list of static instances
   getThreadStaticListVector().increment();

   // Copy mThreadStaticInstances[0] (master copy) into new memory, and
   // pass it back.
   for( int i = 0; i < getThreadStaticListVector()[0].size(); i++ )
   {
      getThreadStaticListVector().last().push_back( getThreadStaticListVector()[0][i]->_createInstance() );
   }

   // Return list index of newly allocated static instance list
   return &getThreadStaticListVector().last();
}