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

#ifndef _TYPETRAITS_H_
#define _TYPETRAITS_H_

#ifndef _PLATFORM_H_
#  include "platform/platform.h"
#endif


/// @file
/// Template definitions for introspecting type properties.


//--------------------------------------------------------------------------
//    Type Predicating.
//--------------------------------------------------------------------------

struct TrueType {};
struct FalseType {};

template< typename T >
inline bool IsTrueType()
{
   return false;
}
template<>
inline bool IsTrueType< TrueType >()
{
   return true;
}

template< typename T >
inline bool IsFalseType()
{
   return false;
}
template<>
inline bool IsFalseType< FalseType >()
{
   return true;
}


template< typename T, typename IfTrue, typename IfFalse >
struct IfTrueType : public IfFalse {};
template< typename IfTrue, typename IfFalse >
struct IfTrueType< TrueType, IfTrue, IfFalse > : public IfTrue {};

template< typename T, typename IfTrue, typename IfFalse >
struct IfFalseType : public IfTrue {};
template< typename IfTrue, typename IfFalse >
struct IfFalseType< FalseType, IfTrue, IfFalse > : public IfFalse {};

//--------------------------------------------------------------------------
//    Construct.
//--------------------------------------------------------------------------

struct _ConstructDefault
{
   template< typename T >
   static T single()
   {
      return T();
   }
   template< typename T, typename A >
   static T single( A a )
   {
      return T( a );
   }
   template< typename T, typename A, typename B >
   static T single( A a, B b )
   {
      return T( a, b );
   }
   template< typename T >
   static void array( T* ptr, U32 num )
   {
      constructArrayInPlace< T >( ptr, num );
   }
   template< typename T, typename A >
   static void array( T* ptr, U32 num, A a )
   {
      for( U32 i = 0; i < num; ++ i )
         ptr[ i ] = single< T >( a );
   }
};
struct _ConstructPrim
{
   template< typename T >
   static T single()
   {
      return 0;
   }
   template< typename T, typename A >
   static T single( T a )
   {
      return a;
   }
   template< typename T >
   static void array( T* ptr, U32 num )
   {
      dMemset( ptr, 0, num * sizeof( T ) );
   }
   template< typename T, typename A >
   static void array( T* ptr, U32 num, T a )
   {
      for( U32 i = 0; i < num; ++ i )
         ptr[ i ] = a;
   }
};
struct _ConstructPtr
{
   template< typename T >
   static T* single()
   {
      return new T;
   }
   template< typename T, typename A >
   static T* single( A a )
   {
      return new T( a );
   }
   template< typename T, typename A, typename B >
   static T* single( A a, B b )
   {
      return new T( a, b );
   }
   template< typename  T >
   static void array( T** ptr, U32 num )
   {
      for( U32 i = 0; i < num; ++ i )
         ptr[ i ] = single< T >();
   }
   template< typename T, typename A >
   static void array( T** ptr, U32 num, A a )
   {
      for( U32 i = 0; i < num; ++ i )
         ptr[ i ] = single< T >( a );
   }
};

//--------------------------------------------------------------------------
//    Destruct.
//--------------------------------------------------------------------------

struct _DestructDefault
{
   template< typename T >
   static void single( T& val )
   {
      val.~T();
   }
   template< typename T >
   static void array( T* ptr, U32 num )
   {
      for( U32 i = 0; i < num; ++ i )
         single< T >( ptr[ i ] );
   }
};
struct _DestructPrim
{
   template< typename T >
   static void single( T& val ) {}
   template< typename T >
   static void array( T* ptr, U32 num ) {}
};
struct _DestructPtr
{
   template< typename T >
   static void single( T*& val )
   {
      delete val;
      val = NULL;
   }
   template< typename T >
   static void array( T* ptr, U32 num )
   {
      for( U32 i = 0; i < num; ++ i )
         single< T >( ptr[ i ] );
   }
};

//--------------------------------------------------------------------------
//    TypeTraits.
//--------------------------------------------------------------------------

template< typename T >
struct _TypeTraits
{
   typedef T BaseType;
   typedef const T ConstType;
   typedef _ConstructDefault Construct;
   typedef _DestructDefault Destruct;
};
template< typename T >
struct _TypeTraits< T* >
{
   typedef T BaseType;
   typedef const T ConstType;
   typedef _ConstructPtr Construct;
   typedef _DestructPtr Destruct;

   template< typename A >
   static bool isTaggedPtr( A* ptr ) { return ( U32( ptr ) & 0x1 ); } //TODO: 64bits
   template< typename A >
   static A* getTaggedPtr( A* ptr ) { return ( A* ) ( U32( ptr ) | 0x1 ); } //TODO: 64bits
   template< typename A >
   static A* getUntaggedPtr( A* ptr ) { return ( A* ) ( U32( ptr ) & 0xFFFFFFFE ); } //TODO: 64bit
};

template< typename T >
struct TypeTraits : public TypeTraits< typename T::Parent >
{
   typedef T BaseType;
   typedef const T ConstType;
};
template< typename T >
struct TypeTraits< T* > : public TypeTraits< typename T::Parent* >
{
   typedef T BaseType;
   typedef const T ConstType;
};
template< typename T >
struct TypeTraits< T* const > : public TypeTraits< typename T::Parent* >
{
   typedef T BaseType;
   typedef const T ConstType;
};
template<>
struct TypeTraits< void > : public _TypeTraits< void > {};
template<>
struct TypeTraits< void* > : public _TypeTraits< void* > {};
template<>
struct TypeTraits< void* const > : public _TypeTraits< void* > {};

// Type traits for primitive types.

template<>
struct TypeTraits< bool > : public _TypeTraits< bool >
{
   typedef _ConstructPrim Construct;
   typedef _DestructPrim Destruct;
};
template<>
struct TypeTraits< S8 > : public _TypeTraits< S8 >
{
   static const S8 MIN = S8_MIN;
   static const S8 MAX = S8_MAX;
   static const S8 ZERO = 0;
   typedef _ConstructPrim Construct;
   typedef _DestructPrim Destruct;
};
template<>
struct TypeTraits< U8 > : public _TypeTraits< U8 >
{
   static const U8 MIN = 0;
   static const U8 MAX = U8_MAX;
   static const U8 ZERO = 0;
   typedef _ConstructPrim Construct;
   typedef _DestructPrim Destruct;
};
template<>
struct TypeTraits< S16 > : public _TypeTraits< S16 >
{
   static const S16 MIN = S16_MIN;
   static const S16 MAX = S16_MAX;
   static const S16 ZERO = 0;
   typedef _ConstructPrim Construct;
   typedef _DestructPrim Destruct;
};
template<>
struct TypeTraits< U16 > : public _TypeTraits< U16 >
{
   static const U16 MIN = 0;
   static const U16 MAX = U16_MAX;
   static const U16 ZERO = 0;
   typedef _ConstructPrim Construct;
   typedef _DestructPrim Destruct;
};
template<>
struct TypeTraits< S32 > : public _TypeTraits< S32 >
{
   static const S32 MIN = S32_MIN;
   static const S32 MAX = S32_MAX;
   static const S32 ZERO = 0;
   typedef _ConstructPrim Construct;
   typedef _DestructPrim Destruct;
};
template<>
struct TypeTraits< U32 > : public _TypeTraits< U32 >
{
   static const U32 MIN = 0;
   static const U32 MAX = U32_MAX;
   static const U32 ZERO = 0;
   typedef _ConstructPrim Construct;
   typedef _DestructPrim Destruct;
};
template<>
struct TypeTraits< F32 > : public _TypeTraits< F32 >
{
   static const F32 MIN;
   static const F32 MAX;
   static const F32 ZERO;
   typedef _ConstructPrim Construct;
   typedef _DestructPrim Destruct;
};

//--------------------------------------------------------------------------
//    Utilities.
//--------------------------------------------------------------------------

template< typename T >
inline T constructSingle()
{
   typedef typename TypeTraits< T >::BaseType Type;
   typedef typename TypeTraits< T >::Construct Construct;
   return Construct::template single< Type >();
}
template< typename T, typename A >
inline T constructSingle( A a )
{
   typedef typename TypeTraits< T >::BaseType BaseType;
   typedef typename TypeTraits< T >::Construct Construct;
   return Construct::template single< BaseType >( a );
}
template< typename T, typename A, typename B >
inline T constructSingle( A a, B b )
{
   typedef typename TypeTraits< T >::BaseType BaseType;
   typedef typename TypeTraits< T >::Construct Construct;
   return Construct::template single< BaseType >( a, b );
}
template< typename T >
inline void constructArray( T* ptr, U32 num )
{
   typedef typename TypeTraits< T >::BaseType BaseType;
   typedef typename TypeTraits< T >::Construct Construct;
   Construct::template array< BaseType >( ptr, num );
}
template< typename T, typename A >
inline void constructArray( T* ptr, U32 num, A a )
{
   typedef typename TypeTraits< T >::BaseType BaseType;
   typedef typename TypeTraits< T >::Construct Construct;
   Construct::template array< BaseType >( ptr, num, a );
}
template< typename T >
inline void destructSingle( T& val )
{
   typedef typename TypeTraits< T >::BaseType BaseType;
   typedef typename TypeTraits< T >::Destruct Destruct;
   Destruct::template single< BaseType >( val );
}
template< typename T >
inline void destructArray( T* ptr, U32 num )
{
   typedef typename TypeTraits< T >::BaseType BaseType;
   typedef typename TypeTraits< T >::Destruct Destruct;
   Destruct::template array< BaseType >( ptr, num );
}

template< typename T>
inline T& Deref( T& val )
{
   return val;
}
template< typename T >
inline T& Deref( T* ptr )
{
   return *ptr;
}

/// Delete a single object policy.
struct DeleteSingle
{
   template<class T>
   static void destroy(T *ptr) { delete ptr; }
};

/// Delete an array of objects policy.
struct DeleteArray
{
   template<class T>
   static void destroy(T *ptr) { delete [] ptr; }
};

///
template< typename T >
struct ValueHolder
{
   T value;
   
   ValueHolder( const T& value )
      : value( value ) {}
      
   operator T() const { return value; }
};
template<>
struct ValueHolder< void >
{
   ValueHolder() {}
};

#endif // _TYPETRAITS_H_
