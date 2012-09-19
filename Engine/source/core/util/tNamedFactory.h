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

#ifndef _TFACTORY_H_
#define _TFACTORY_H_

#ifndef _TDICTIONARY_H_
#include "core/util/tDictionary.h"
#endif
#ifndef _TORQUE_STRING_H_
#include "core/util/str.h"
#endif


/// A helper template class for registering creation 
/// methods to name strings.
template <typename Type>
class NamedFactory
{
protected:

   typedef Type*(*FactorySig)();

   typedef Map<String,FactorySig> FactoryMap;

   ///
   static FactoryMap& getFactoryMap()
   {
      static FactoryMap theMap;
      return theMap;
   }

public:

   /// Add a new creation method to the factory.
   static void add( const char *name, FactorySig func )
   {
      getFactoryMap().insert( name, func );
   }

   /// Create an object instance by name.
   static Type* create( const char *name )
   {
      typename FactoryMap::Iterator iter = getFactoryMap().find( name );
      if ( iter == getFactoryMap().end() )
         return NULL;

      return iter->value();
   }

   /// Create an object instance of the first entry in the factory.
   static Type* create()
   {
      typename FactoryMap::Iterator iter = getFactoryMap().begin();
      if ( iter == getFactoryMap().end() )
         return NULL;

      return iter->value();
   }
};


#endif //_TFACTORY_H_

