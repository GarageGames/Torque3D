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

#ifndef _TORQUE_CORE_UTIL_NAMEDSINGLETON_H_
#define _TORQUE_CORE_UTIL_NAMEDSINGLETON_H_

#include "platform/platform.h"
#include "core/util/safeCast.h"
#include "console/console.h"
#include "core/stringTable.h"

//--------------------------------------------------------------------------
//    StaticNamedSingleton.
//--------------------------------------------------------------------------

/// Collection of statically registered named singletons.
///
/// This class is useful as a mix-in for classes that are supposed to
/// represent a range of named singleton instances from which a specific
/// instance is then selected at run-time.
///
/// @param T Arbitrary type parameter; identical types will share
///    static data.

template< class T >
struct StaticNamedSingleton
{
   typedef StaticNamedSingleton This;

   StaticNamedSingleton( const char* name );
   virtual ~StaticNamedSingleton() {}

   const char*       getName();
   T*                getNext();

   static T*         staticGetFirst();
   static T*         staticFindSingleton( const char* name );
   static EnumTable* staticCreateEnumTable();
   static U32        staticGetNumSingletons();

private:
   const char*       mName;
   This*             mNext;

   static This*      smSingletons;
};

template< class T >
StaticNamedSingleton< T >* StaticNamedSingleton< T >::smSingletons;

template< class T >
StaticNamedSingleton< T >::StaticNamedSingleton( const char* name )
   : mName( name )
{
   mNext = smSingletons;
   smSingletons = this;
}

template< class T >
inline const char* StaticNamedSingleton< T >::getName()
{
   return mName;
}

template< class T >
inline T* StaticNamedSingleton< T >::getNext()
{
   return static_cast< T* >( mNext );
}

template< class T >
T* StaticNamedSingleton< T >::staticGetFirst()
{
   return static_cast< T* >( smSingletons );
}

/// Find the instance with the given name.  Returns NULL if no such
/// instance exists.

template< class T >
T* StaticNamedSingleton< T >::staticFindSingleton( const char* name )
{
   for( This* ptr = smSingletons; ptr != 0; ptr = ptr->mNext )
      if( dStricmp( name, ptr->mName ) == 0 )
         return static_cast< T* >( ptr );

   return 0;
}

/// Create a TorqueScript EnumTable that contains all registered
/// instance names.

template< class T >
EnumTable* StaticNamedSingleton< T >::staticCreateEnumTable()
{
   U32 numSingletons = staticGetNumSingletons();

   // Create the enums.
   
   EnumTable::Enums* enums = new EnumTable::Enums[ numSingletons ];
   This* ptr = smSingletons;
   for( U32 i = 0; i < numSingletons; ++ i )
   {
      enums[ i ].index = i;
      enums[ i ].label = StringTable->insert( ptr->getName() );

      ptr = ptr->mNext;
   }

   // Create the table.

   return new EnumTable( numSingletons, enums );
}

/// Return the number of registered named singletons.

template< class T >
U32 StaticNamedSingleton< T >::staticGetNumSingletons()
{
   U32 numSingletons = 0;
   for( This* ptr = smSingletons; ptr != 0; ptr = ptr->mNext )
      numSingletons ++;
   return numSingletons;
}

#endif // _TORQUE_CORE_UTIL_NAMEDSINGLETON_H_
