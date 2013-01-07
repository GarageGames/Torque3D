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

#ifndef _SIMPERSISTSET_H_
#define _SIMPERSISTSET_H_

#ifndef _SIMSET_H_
   #include "console/simSet.h"
#endif


/// A SimSet that can be safely persisted.  Uses SimPersistIDs to reference
/// objects in the set while persisted on disk.  This allows the set to resolve
/// its references no matter whether they are loaded before or after the set
/// is created.
///
class SimPersistSet : public SimSet
{
   public:
   
      typedef SimSet Parent;
      
   protected:
   
      /// List of unresolved persistent IDs.
      Vector< SimPersistID* > mUnresolvedPIDs;
      
      /// If true, the set is currently resolving persistent IDs.
      bool mIsResolvingPIDs;

   public:
   
      ///
      SimPersistSet();
   
      /// Try to resolve all persistent IDs that as of yet are still unresolved.
      void resolvePIDs();

      // SimSet.
      virtual void addObject( SimObject* );
      virtual void write( Stream &stream, U32 tabStop, U32 flags = 0 );
      virtual bool processArguments( S32 argc, ConsoleValueRef *argv );
      
      DECLARE_CONOBJECT( SimPersistSet );
      DECLARE_CATEGORY( "Console" );
      DECLARE_DESCRIPTION( "A SimSet that can be safely persisted." );
};

#endif // !_SIMPERSISTSET_H_
