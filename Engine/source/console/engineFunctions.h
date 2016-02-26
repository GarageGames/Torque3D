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

#ifndef _ENGINEFUNCTIONS_H_
#define _ENGINEFUNCTIONS_H_

#ifndef _ENGINEEXPORTS_H_
   #include "console/engineExports.h"
#endif
#ifndef _ENGINETYPEINFO_H_
   #include "console/engineTypeInfo.h"
#endif


/// @file
/// Structures for function-type engine export information.


#ifdef TORQUE_COMPILER_VISUALC
   #define TORQUE_API extern "C" __declspec( dllexport )
#elif defined( TORQUE_COMPILER_GCC )
   #define TORQUE_API extern "C" __attribute__( ( visibility( "default" ) ) )
#else
   #error Unsupported compiler.
#endif


// #pragma pack is bugged in GCC in that the packing in place at the template instantiation
// sites rather than their definition sites is used.  Enable workarounds.
#ifdef TORQUE_COMPILER_GCC
   #define _PACK_BUG_WORKAROUNDS
#endif



/// Structure storing the default argument values for a function invocation
/// frame.
struct EngineFunctionDefaultArguments
{
   /// Number of default arguments for the function call frame.
   ///
   /// @warn This is @b NOT the size of the memory block returned by getArgs() and also
   ///   not the number of elements it contains.
   U32 mNumDefaultArgs;
   
   /// Return a pointer to the variable-sized array of default argument values.
   ///
   /// @warn The arguments must be stored @b IMMEDIATELY after #mNumDefaultArgs.
   /// @warn This is a @b FULL frame and not just the default arguments, i.e. it starts with the
   ///   first argument that the function takes and ends with the last argument it takes.
   /// @warn If the compiler's #pragma pack is buggy, the elements in this structure are allowed
   ///   to be 4-byte aligned rather than byte-aligned as they should be.
   const U8* getArgs() const
   {
      return ( const U8* ) &( mNumDefaultArgs ) + sizeof( mNumDefaultArgs );
   }
};


// Need byte-aligned packing for the default argument structures.
#ifdef _WIN64
#pragma pack( push, 4 )
#else
#pragma pack( push, 1 )
#endif
   

// Structure encapsulating default arguments to an engine API function.
template< typename T >
struct _EngineFunctionDefaultArguments {};
template<>
struct _EngineFunctionDefaultArguments< void() > : public EngineFunctionDefaultArguments
{
   _EngineFunctionDefaultArguments()
      { mNumDefaultArgs = 0; }
};
template< typename A >
struct _EngineFunctionDefaultArguments< void( A ) > : public EngineFunctionDefaultArguments
{
   typename EngineTypeTraits< A >::DefaultArgumentValueStoreType a;
   
   _EngineFunctionDefaultArguments()
      { mNumDefaultArgs = 0; }
   _EngineFunctionDefaultArguments( A a )
      : a( a )
      { mNumDefaultArgs = 1; }
};
template< typename A, typename B >
struct _EngineFunctionDefaultArguments< void( A, B ) > : public EngineFunctionDefaultArguments
{
   typename EngineTypeTraits< A >::DefaultArgumentValueStoreType a;
   typename EngineTypeTraits< B >::DefaultArgumentValueStoreType b;
   
   _EngineFunctionDefaultArguments()
      { mNumDefaultArgs = 0; }
   _EngineFunctionDefaultArguments( B b )
      : b( b )
      { mNumDefaultArgs = 1; }
   _EngineFunctionDefaultArguments( A a, B b )
      : a( a ),
        b( b )
      { mNumDefaultArgs = 2; }
};
template< typename A, typename B, typename C >
struct _EngineFunctionDefaultArguments< void( A, B, C ) > : public EngineFunctionDefaultArguments
{
   typename EngineTypeTraits< A >::DefaultArgumentValueStoreType a;
   typename EngineTypeTraits< B >::DefaultArgumentValueStoreType b;
   typename EngineTypeTraits< C >::DefaultArgumentValueStoreType c;
   
   _EngineFunctionDefaultArguments()
      { mNumDefaultArgs = 0; }
   _EngineFunctionDefaultArguments( C c )
      : c( c )
      { mNumDefaultArgs = 1; }
   _EngineFunctionDefaultArguments( B b, C c )
      : b( b ),
        c( c )
      { mNumDefaultArgs = 2; }
   _EngineFunctionDefaultArguments( A a, B b, C c )
      : a( a ),
        b( b ),
        c( c )
      { mNumDefaultArgs = 3; }
};
template< typename A, typename B, typename C, typename D >
struct _EngineFunctionDefaultArguments< void( A, B, C, D ) > : public EngineFunctionDefaultArguments
{
   typename EngineTypeTraits< A >::DefaultArgumentValueStoreType a;
   typename EngineTypeTraits< B >::DefaultArgumentValueStoreType b;
   typename EngineTypeTraits< C >::DefaultArgumentValueStoreType c;
   typename EngineTypeTraits< D >::DefaultArgumentValueStoreType d;
   
   _EngineFunctionDefaultArguments()
      { mNumDefaultArgs = 0; }
   _EngineFunctionDefaultArguments( D d )
      : d( d )
      { mNumDefaultArgs = 1; }
   _EngineFunctionDefaultArguments( C c, D d )
      : c( c ),
        d( d )
      { mNumDefaultArgs = 2; }
   _EngineFunctionDefaultArguments( B b, C c, D d )
      : b( b ),
        c( c ),
        d( d )
      { mNumDefaultArgs = 3; }
   _EngineFunctionDefaultArguments( A a, B b, C c, D d )
      : a( a ),
        b( b ),
        c( c ),
        d( d )
      { mNumDefaultArgs = 4; }
};
template< typename A, typename B, typename C, typename D, typename E >
struct _EngineFunctionDefaultArguments< void( A, B, C, D, E ) > : public EngineFunctionDefaultArguments
{
   typename EngineTypeTraits< A >::DefaultArgumentValueStoreType a;
   typename EngineTypeTraits< B >::DefaultArgumentValueStoreType b;
   typename EngineTypeTraits< C >::DefaultArgumentValueStoreType c;
   typename EngineTypeTraits< D >::DefaultArgumentValueStoreType d;
   typename EngineTypeTraits< E >::DefaultArgumentValueStoreType e;
   
   _EngineFunctionDefaultArguments()
      { mNumDefaultArgs = 0; }
   _EngineFunctionDefaultArguments( E e )
      : e( e )
      { mNumDefaultArgs = 1; }
   _EngineFunctionDefaultArguments( D d, E e )
      : d( d ),
        e( e )
      { mNumDefaultArgs = 2; }
   _EngineFunctionDefaultArguments( C c, D d, E e )
      : c( c ),
        d( d ),
        e( e )
      { mNumDefaultArgs = 3; }
   _EngineFunctionDefaultArguments( B b, C c, D d, E e )
      : b( b ),
        c( c ),
        d( d ),
        e( e )
      { mNumDefaultArgs = 4; }
   _EngineFunctionDefaultArguments( A a, B b, C c, D d, E e )
      : a( a ),
        b( b ),
        c( c ),
        d( d ),
        e( e )
      { mNumDefaultArgs = 5; }
};
template< typename A, typename B, typename C, typename D, typename E, typename F >
struct _EngineFunctionDefaultArguments< void( A, B, C, D, E, F ) > : public EngineFunctionDefaultArguments
{
   typename EngineTypeTraits< A >::DefaultArgumentValueStoreType a;
   typename EngineTypeTraits< B >::DefaultArgumentValueStoreType b;
   typename EngineTypeTraits< C >::DefaultArgumentValueStoreType c;
   typename EngineTypeTraits< D >::DefaultArgumentValueStoreType d;
   typename EngineTypeTraits< E >::DefaultArgumentValueStoreType e;
   typename EngineTypeTraits< F >::DefaultArgumentValueStoreType f;
   
   _EngineFunctionDefaultArguments()
      { mNumDefaultArgs = 0; }
   _EngineFunctionDefaultArguments( F f )
      : f( f )
      { mNumDefaultArgs = 1; }
   _EngineFunctionDefaultArguments( E e, F f )
      : e( e ),
        f( f )
      { mNumDefaultArgs = 2; }
   _EngineFunctionDefaultArguments( D d, E e, F f )
      : d( d ),
        e( e ),
        f( f )
      { mNumDefaultArgs = 3; }
   _EngineFunctionDefaultArguments( C c, D d, E e, F f )
      : c( c ),
        d( d ),
        e( e ),
        f( f )
      { mNumDefaultArgs = 4; }
   _EngineFunctionDefaultArguments( B b, C c, D d, E e, F f )
      : b( b ),
        c( c ),
        d( d ),
        e( e ),
        f( f )
      { mNumDefaultArgs = 5; }
   _EngineFunctionDefaultArguments( A a, B b, C c, D d, E e, F f )
      : a( a ),
        b( b ),
        c( c ),
        d( d ),
        e( e ),
        f( f )
      { mNumDefaultArgs = 6; }
};
template< typename A, typename B, typename C, typename D, typename E, typename F, typename G >
struct _EngineFunctionDefaultArguments< void( A, B, C, D, E, F, G ) > : public EngineFunctionDefaultArguments
{
   typename EngineTypeTraits< A >::DefaultArgumentValueStoreType a;
   typename EngineTypeTraits< B >::DefaultArgumentValueStoreType b;
   typename EngineTypeTraits< C >::DefaultArgumentValueStoreType c;
   typename EngineTypeTraits< D >::DefaultArgumentValueStoreType d;
   typename EngineTypeTraits< E >::DefaultArgumentValueStoreType e;
   typename EngineTypeTraits< F >::DefaultArgumentValueStoreType f;
   typename EngineTypeTraits< G >::DefaultArgumentValueStoreType g;
   
   _EngineFunctionDefaultArguments()
      { mNumDefaultArgs = 0; }
   _EngineFunctionDefaultArguments( G g )
      : g( g )
      { mNumDefaultArgs = 1; }
   _EngineFunctionDefaultArguments( F f, G g )
      : f( f ),
        g( g )
      { mNumDefaultArgs = 2; }
   _EngineFunctionDefaultArguments( E e, F f, G g )
      : e( e ),
        f( f ),
        g( g )
      { mNumDefaultArgs = 3; }
   _EngineFunctionDefaultArguments( D d, E e, F f, G g )
      : d( d ),
        e( e ),
        f( f ),
        g( g )
      { mNumDefaultArgs = 4; }
   _EngineFunctionDefaultArguments( C c, D d, E e, F f, G g )
      : c( c ),
        d( d ),
        e( e ),
        f( f ),
        g( g )
      { mNumDefaultArgs = 5; }
   _EngineFunctionDefaultArguments( B b, C c, D d, E e, F f, G g )
      : b( b ),
        c( c ),
        d( d ),
        e( e ),
        f( f ),
        g( g )
      { mNumDefaultArgs = 6; }
   _EngineFunctionDefaultArguments( A a, B b, C c, D d, E e, F f, G g )
      : a( a ),
        b( b ),
        c( c ),
        d( d ),
        e( e ),
        f( f ),
        g( g )
      { mNumDefaultArgs = 7; }
};
template< typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H >
struct _EngineFunctionDefaultArguments< void( A, B, C, D, E, F, G, H ) > : public EngineFunctionDefaultArguments
{
   typename EngineTypeTraits< A >::DefaultArgumentValueStoreType a;
   typename EngineTypeTraits< B >::DefaultArgumentValueStoreType b;
   typename EngineTypeTraits< C >::DefaultArgumentValueStoreType c;
   typename EngineTypeTraits< D >::DefaultArgumentValueStoreType d;
   typename EngineTypeTraits< E >::DefaultArgumentValueStoreType e;
   typename EngineTypeTraits< F >::DefaultArgumentValueStoreType f;
   typename EngineTypeTraits< G >::DefaultArgumentValueStoreType g;
   typename EngineTypeTraits< H >::DefaultArgumentValueStoreType h;
   
   _EngineFunctionDefaultArguments()
      { mNumDefaultArgs = 0; }
   _EngineFunctionDefaultArguments( H h )
      : h( h )
      { mNumDefaultArgs = 1; }
   _EngineFunctionDefaultArguments( G g, H h )
      : g( g ),
        h( h )
      { mNumDefaultArgs = 2; }
   _EngineFunctionDefaultArguments( F f, G g, H h )
      : f( f ),
        g( g ),
        h( h )
      { mNumDefaultArgs = 3; }
   _EngineFunctionDefaultArguments( E e, F f, G g, H h )
      : e( e ),
        f( f ),
        g( g ),
        h( h )
      { mNumDefaultArgs = 4; }
   _EngineFunctionDefaultArguments( D d, E e, F f, G g, H h )
      : d( d ),
        e( e ),
        f( f ),
        g( g ),
        h( h )
      { mNumDefaultArgs = 5; }
   _EngineFunctionDefaultArguments( C c, D d, E e, F f, G g, H h )
      : c( c ),
        d( d ),
        e( e ),
        f( f ),
        g( g ),
        h( h )
      { mNumDefaultArgs = 6; }
   _EngineFunctionDefaultArguments( B b, C c, D d, E e, F f, G g, H h )
      : b( b ),
        c( c ),
        d( d ),
        e( e ),
        f( f ),
        g( g ),
        h( h )
      { mNumDefaultArgs = 7; }
   _EngineFunctionDefaultArguments( A a, B b, C c, D d, E e, F f, G g, H h )
      : a( a ),
        b( b ),
        c( c ),
        d( d ),
        e( e ),
        f( f ),
        g( g ),
        h( h )
      { mNumDefaultArgs = 8; }
};
template< typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I >
struct _EngineFunctionDefaultArguments< void( A, B, C, D, E, F, G, H, I ) > : public EngineFunctionDefaultArguments
{
   typename EngineTypeTraits< A >::DefaultArgumentValueStoreType a;
   typename EngineTypeTraits< B >::DefaultArgumentValueStoreType b;
   typename EngineTypeTraits< C >::DefaultArgumentValueStoreType c;
   typename EngineTypeTraits< D >::DefaultArgumentValueStoreType d;
   typename EngineTypeTraits< E >::DefaultArgumentValueStoreType e;
   typename EngineTypeTraits< F >::DefaultArgumentValueStoreType f;
   typename EngineTypeTraits< G >::DefaultArgumentValueStoreType g;
   typename EngineTypeTraits< H >::DefaultArgumentValueStoreType h;
   typename EngineTypeTraits< I >::DefaultArgumentValueStoreType i;
   
   _EngineFunctionDefaultArguments()
      { mNumDefaultArgs = 0; }
   _EngineFunctionDefaultArguments( I i )
      : i( i )
      { mNumDefaultArgs = 1; }
   _EngineFunctionDefaultArguments( H h, I i )
      : h( h ),
        i( i )
      { mNumDefaultArgs = 2; }
   _EngineFunctionDefaultArguments( G g, H h, I i )
      : g( g ),
        h( h ),
        i( i )
      { mNumDefaultArgs = 3; }
   _EngineFunctionDefaultArguments( F f, G g, H h, I i )
      : f( f ),
        g( g ),
        h( h ),
        i( i )
      { mNumDefaultArgs = 4; }
   _EngineFunctionDefaultArguments( E e, F f, G g, H h, I i )
      : e( e ),
        f( f ),
        g( g ),
        h( h ),
        i( i )
      { mNumDefaultArgs = 5; }
   _EngineFunctionDefaultArguments( D d, E e, F f, G g, H h, I i )
      : d( d ),
        e( e ),
        f( f ),
        g( g ),
        h( h ),
        i( i )
      { mNumDefaultArgs = 6; }
   _EngineFunctionDefaultArguments( C c, D d, E e, F f, G g, H h, I i )
      : c( c ),
        d( d ),
        e( e ),
        f( f ),
        g( g ),
        h( h ),
        i( i )
      { mNumDefaultArgs = 7; }
   _EngineFunctionDefaultArguments( B b, C c, D d, E e, F f, G g, H h, I i )
      : b( b ),
        c( c ),
        d( d ),
        e( e ),
        f( f ),
        g( g ),
        h( h ),
        i( i )
      { mNumDefaultArgs = 8; }
   _EngineFunctionDefaultArguments( A a, B b, C c, D d, E e, F f, G g, H h, I i )
      : a( a ),
        b( b ),
        c( c ),
        d( d ),
        e( e ),
        f( f ),
        g( g ),
        h( h ),
        i( i )
      { mNumDefaultArgs = 9; }
};
template< typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J >
struct _EngineFunctionDefaultArguments< void( A, B, C, D, E, F, G, H, I, J ) > : public EngineFunctionDefaultArguments
{
   typename EngineTypeTraits< A >::DefaultArgumentValueStoreType a;
   typename EngineTypeTraits< B >::DefaultArgumentValueStoreType b;
   typename EngineTypeTraits< C >::DefaultArgumentValueStoreType c;
   typename EngineTypeTraits< D >::DefaultArgumentValueStoreType d;
   typename EngineTypeTraits< E >::DefaultArgumentValueStoreType e;
   typename EngineTypeTraits< F >::DefaultArgumentValueStoreType f;
   typename EngineTypeTraits< G >::DefaultArgumentValueStoreType g;
   typename EngineTypeTraits< H >::DefaultArgumentValueStoreType h;
   typename EngineTypeTraits< I >::DefaultArgumentValueStoreType i;
   typename EngineTypeTraits< J >::DefaultArgumentValueStoreType j;
   
   _EngineFunctionDefaultArguments()
      { mNumDefaultArgs = 0; }
   _EngineFunctionDefaultArguments( J j )
      : j( j )
      { mNumDefaultArgs = 1; }
   _EngineFunctionDefaultArguments( I i, J j )
      : i( i ),
        j( j )
      { mNumDefaultArgs = 2; }
   _EngineFunctionDefaultArguments( H h, I i, J j )
      : h( h ),
        i( i ),
        j( j )
      { mNumDefaultArgs = 3; }
   _EngineFunctionDefaultArguments( G g, H h, I i, J j )
      : g( g ),
        h( h ),
        i( i ),
        j( j )
      { mNumDefaultArgs = 4; }
   _EngineFunctionDefaultArguments( F f, G g, H h, I i, J j )
      : f( f ),
        g( g ),
        h( h ),
        i( i ),
        j( j )
      { mNumDefaultArgs = 5; }
   _EngineFunctionDefaultArguments( E e, F f, G g, H h, I i, J j )
      : e( e ),
        f( f ),
        g( g ),
        h( h ),
        i( i ),
        j( j )
      { mNumDefaultArgs = 6; }
   _EngineFunctionDefaultArguments( D d, E e, F f, G g, H h, I i, J j )
      : d( d ),
        e( e ),
        f( f ),
        g( g ),
        h( h ),
        i( i ),
        j( j )
      { mNumDefaultArgs = 7; }
   _EngineFunctionDefaultArguments( C c, D d, E e, F f, G g, H h, I i, J j )
      : c( c ),
        d( d ),
        e( e ),
        f( f ),
        g( g ),
        h( h ),
        i( i ),
        j( j )
      { mNumDefaultArgs = 8; }
   _EngineFunctionDefaultArguments( B b, C c, D d, E e, F f, G g, H h, I i, J j )
      : b( b ),
        c( c ),
        d( d ),
        e( e ),
        f( f ),
        g( g ),
        h( h ),
        i( i ),
        j( j )
      { mNumDefaultArgs = 9; }
   _EngineFunctionDefaultArguments( A a, B b, C c, D d, E e, F f, G g, H h, I i, J j )
      : a( a ),
        b( b ),
        c( c ),
        d( d ),
        e( e ),
        f( f ),
        g( g ),
        h( h ),
        i( i ),
        j( j )
      { mNumDefaultArgs = 10; }
};
template< typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K >
struct _EngineFunctionDefaultArguments< void( A, B, C, D, E, F, G, H, I, J, K ) > : public EngineFunctionDefaultArguments
{
   typename EngineTypeTraits< A >::DefaultArgumentValueStoreType a;
   typename EngineTypeTraits< B >::DefaultArgumentValueStoreType b;
   typename EngineTypeTraits< C >::DefaultArgumentValueStoreType c;
   typename EngineTypeTraits< D >::DefaultArgumentValueStoreType d;
   typename EngineTypeTraits< E >::DefaultArgumentValueStoreType e;
   typename EngineTypeTraits< F >::DefaultArgumentValueStoreType f;
   typename EngineTypeTraits< G >::DefaultArgumentValueStoreType g;
   typename EngineTypeTraits< H >::DefaultArgumentValueStoreType h;
   typename EngineTypeTraits< I >::DefaultArgumentValueStoreType i;
   typename EngineTypeTraits< J >::DefaultArgumentValueStoreType j;
   typename EngineTypeTraits< K >::DefaultArgumentValueStoreType k;

   _EngineFunctionDefaultArguments()
      { mNumDefaultArgs = 0; }
   _EngineFunctionDefaultArguments( K k )
      : k( k )
      { mNumDefaultArgs = 1; }
   _EngineFunctionDefaultArguments( J j, K k )
      : j( j ),
        k( k )
      { mNumDefaultArgs = 2; }
   _EngineFunctionDefaultArguments( I i, J j, K k )
      : i( i ),
        j( j ),
        k( k )
      { mNumDefaultArgs = 3; }
   _EngineFunctionDefaultArguments( H h, I i, J j, K k )
      : h( h ),
        i( i ),
        j( j ),
        k( k )
      { mNumDefaultArgs = 4; }
   _EngineFunctionDefaultArguments( G g, H h, I i, J j, K k )
      : g( g ),
        h( h ),
        i( i ),
        j( j ),
        k( k )
      { mNumDefaultArgs = 5; }
   _EngineFunctionDefaultArguments( F f, G g, H h, I i, J j, K k )
      : f( f ),
        g( g ),
        h( h ),
        i( i ),
        j( j ),
        k( k )
      { mNumDefaultArgs = 6; }
   _EngineFunctionDefaultArguments( E e, F f, G g, H h, I i, J j, K k )
      : e( e ),
        f( f ),
        g( g ),
        h( h ),
        i( i ),
        j( j ),
        k( k )
      { mNumDefaultArgs = 7; }
   _EngineFunctionDefaultArguments( D d, E e, F f, G g, H h, I i, J j, K k )
      : d( d ),
        e( e ),
        f( f ),
        g( g ),
        h( h ),
        i( i ),
        j( j ),
        k( k )
      { mNumDefaultArgs = 8; }
   _EngineFunctionDefaultArguments( C c, D d, E e, F f, G g, H h, I i, J j, K k )
      : c( c ),
        d( d ),
        e( e ),
        f( f ),
        g( g ),
        h( h ),
        i( i ),
        j( j ),
        k( k )
      { mNumDefaultArgs = 9; }
   _EngineFunctionDefaultArguments( B b, C c, D d, E e, F f, G g, H h, I i, J j, K k )
      : b( b ),
        c( c ),
        d( d ),
        e( e ),
        f( f ),
        g( g ),
        h( h ),
        i( i ),
        j( j ),
        k( k )
      { mNumDefaultArgs = 10; }
   _EngineFunctionDefaultArguments( A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k )
      : a( a ),
        b( b ),
        c( c ),
        d( d ),
        e( e ),
        f( f ),
        g( g ),
        h( h ),
        i( i ),
        j( j ),
        k( k )
      { mNumDefaultArgs = 11; }
};
template< typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L >
struct _EngineFunctionDefaultArguments< void( A, B, C, D, E, F, G, H, I, J, K, L ) > : public EngineFunctionDefaultArguments
{
   typename EngineTypeTraits< A >::DefaultArgumentValueStoreType a;
   typename EngineTypeTraits< B >::DefaultArgumentValueStoreType b;
   typename EngineTypeTraits< C >::DefaultArgumentValueStoreType c;
   typename EngineTypeTraits< D >::DefaultArgumentValueStoreType d;
   typename EngineTypeTraits< E >::DefaultArgumentValueStoreType e;
   typename EngineTypeTraits< F >::DefaultArgumentValueStoreType f;
   typename EngineTypeTraits< G >::DefaultArgumentValueStoreType g;
   typename EngineTypeTraits< H >::DefaultArgumentValueStoreType h;
   typename EngineTypeTraits< I >::DefaultArgumentValueStoreType i;
   typename EngineTypeTraits< J >::DefaultArgumentValueStoreType j;
   typename EngineTypeTraits< K >::DefaultArgumentValueStoreType k;
   typename EngineTypeTraits< L >::DefaultArgumentValueStoreType l;

   _EngineFunctionDefaultArguments()
      { mNumDefaultArgs = 0; }
   _EngineFunctionDefaultArguments( L l )
      : l( l )
      { mNumDefaultArgs = 1; }
   _EngineFunctionDefaultArguments( K k, L l )
      : k( k ),
        l( l )
      { mNumDefaultArgs = 2; }
   _EngineFunctionDefaultArguments( J j, K k, L l )
      : j( j ),
        k( k ),
        l( l )
      { mNumDefaultArgs = 3; }
   _EngineFunctionDefaultArguments( I i, J j, K k, L l )
      : i( i ),
        j( j ),
        k( k ),
        l( l )
      { mNumDefaultArgs = 4; }
   _EngineFunctionDefaultArguments( H h, I i, J j, K k, L l )
      : h( h ),
        i( i ),
        j( j ),
        k( k ),
        l( l )
      { mNumDefaultArgs = 5; }
   _EngineFunctionDefaultArguments( G g, H h, I i, J j, K k, L l )
      : g( g ),
        h( h ),
        i( i ),
        j( j ),
        k( k ),
        l( l )
      { mNumDefaultArgs = 6; }
   _EngineFunctionDefaultArguments( F f, G g, H h, I i, J j, K k, L l )
      : f( f ),
        g( g ),
        h( h ),
        i( i ),
        j( j ),
        k( k ),
        l( l )
      { mNumDefaultArgs = 7; }
   _EngineFunctionDefaultArguments( E e, F f, G g, H h, I i, J j, K k, L l )
      : e( e ),
        f( f ),
        g( g ),
        h( h ),
        i( i ),
        j( j ),
        k( k ),
        l( l )
      { mNumDefaultArgs = 8; }
   _EngineFunctionDefaultArguments( D d, E e, F f, G g, H h, I i, J j, K k, L l )
      : d( d ),
        e( e ),
        f( f ),
        g( g ),
        h( h ),
        i( i ),
        j( j ),
        k( k ),
        l( l )
      { mNumDefaultArgs = 9; }
   _EngineFunctionDefaultArguments( C c, D d, E e, F f, G g, H h, I i, J j, K k, L l )
      : c( c ),
        d( d ),
        e( e ),
        f( f ),
        g( g ),
        h( h ),
        i( i ),
        j( j ),
        k( k ),
        l( l )
      { mNumDefaultArgs = 10; }
   _EngineFunctionDefaultArguments( B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l )
      : b( b ),
        c( c ),
        d( d ),
        e( e ),
        f( f ),
        g( g ),
        h( h ),
        i( i ),
        j( j ),
        k( k ),
        l( l )
      { mNumDefaultArgs = 11; }
   _EngineFunctionDefaultArguments( A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l )
      : a( a ),
        b( b ),
        c( c ),
        d( d ),
        e( e ),
        f( f ),
        g( g ),
        h( h ),
        i( i ),
        j( j ),
        k( k ),
        l( l )
      { mNumDefaultArgs = 12; }
};

#pragma pack( pop )


// Helper to allow flags argument to DEFINE_FUNCTION to be empty.
struct _EngineFunctionFlags
{
   U32 val;
   _EngineFunctionFlags()
      : val( 0 ) {}
   _EngineFunctionFlags( U32 val )
      : val( val ) {}
   operator U32() const { return val; }
};


///
enum EngineFunctionFlags
{
   /// Function is a callback into the control layer.  If this flag is not set,
   /// the function is a call-in.
   EngineFunctionCallout = BIT( 0 ),
};


/// A function exported by the engine for interfacing with the control layer.
///
/// A function can either be a call-in, transfering control flow from the control layer to the engine, or a call-out,
/// transfering control flow from the engine to the control layer.
///
/// All engine API functions use the native C (@c cdecl) calling convention.
///
/// Be aware that there a no implicit parameters to functions.  This, for example, means that methods will simply
/// list an object type parameter as their first argument but otherwise be indistinguishable from other functions.
///
/// Variadic functions are supported.
///
/// @section engineFunction_strings String Arguments and Return Values
///
/// Strings passed through the API are assumed to be owned by the caller.  They must persist for the entire duration
/// of a call.
///
/// Strings returned by a function are assumed to be in transient storage that will be overwritten by subsequent API
/// calls.  If the caller wants to preserve a string, it is responsible to copying strings to its own memory.  This will
/// happen with most higher-level control layers anyway.
///
/// @section engineFunction_defaultargs Default Arguments
///
/// As the engine API export system is set up to not require hand-written code in generated wrappers per se, the
/// export system seeks to include a maximum possible amount of information in the export structures.
/// To this end, where applicable, information about suggested default values for arguments to the engine API
/// functions is stored in the export structures.  It is up to the wrapper generator if and how it makes use of
/// this information.
///
/// Default arguments are represented by capturing raw stack frame vectors of the arguments to functions.  These
/// frames could be used as default images for passing arguments in stack frames, though wrapper generators
/// may actually want to read out individual argument values and include them in function prototypes within
/// the generated code.
///
/// @section engineFunction_callin Call-ins
///
/// Call-ins are exposed as native entry points.  The control layer must be able to natively
/// marshall arguments and call DLL function exports using C calling conventions.
///
/// @section engineFunction_callout Call-outs
///
/// Call-outs are exposed as pointer-sized memory locations into which the control layer needs
/// to install addresses of functions that receive the call from the engine back into the control
/// layer.  The function has to follow C calling conventions and 
///
/// A call-out will initially be set to NULL and while being NULL, will simply cause the engine
/// to skip and ignore the call-out.  This allows the control layer to only install call-outs
/// it is actually interested in.
///
class EngineFunctionInfo : public EngineExport
{
   public:
   
      DECLARE_CLASS( EngineFunctionInfo, EngineExport );
   
   protected:
   
      /// A combination of EngineFunctionFlags.
      BitSet32 mFunctionFlags;
      
      /// The type of the function.
      const EngineTypeInfo* mFunctionType;
         
      /// Default values for the function arguments.
      const EngineFunctionDefaultArguments* mDefaultArgumentValues;
            
      /// Name of the DLL symbol denoting the address of the exported entity.
      const char* mBindingName;
      
      /// Full function prototype string.  Useful for quick printing and most importantly,
      /// this will be the only place containing information about the argument names.
      const char* mPrototypeString;
      
      /// Address of either the function implementation or the variable taking the address
      /// of a call-out.
      void* mAddress;
      
      /// Next function in the global link chain of engine functions.
      EngineFunctionInfo* mNextFunction;
      
      /// First function in the global link chain of engine functions.
      static EngineFunctionInfo* smFirstFunction;
      
   public:
   
      ///
      EngineFunctionInfo(  const char* name,
                           EngineExportScope* scope,
                           const char* docString,
                           const char* protoypeString,
                           const char* bindingName,
                           const EngineTypeInfo* functionType,
                           const EngineFunctionDefaultArguments* defaultArgs,
                           void* address,
                           U32 flags );
      
      /// Return the name of the function.
      const char* getFunctionName() const { return getExportName(); }
      
      /// Return the function's full prototype string including the return type, function name,
      /// and argument list.
      const char* getPrototypeString() const { return mPrototypeString; }
      
      /// Return the DLL export symbol name.
      const char* getBindingName() const { return mBindingName; }
      
      /// Test whether this is a callout function.
      bool isCallout() const { return mFunctionFlags.test( EngineFunctionCallout ); }
      
      /// Test whether the function is variadic, i.e. takes a variable number of arguments.
      bool isVariadic() const { return mFunctionType->isVariadic(); }
         
      /// Return the type of this function.
      const EngineTypeInfo* getFunctionType() const { return mFunctionType; }
      
      /// Return the return type of the function.
      const EngineTypeInfo* getReturnType() const { return getFunctionType()->getArgumentTypeTable()->getReturnType(); }
      
      /// Return the number of arguments that this function takes.  If the function is variadic,
      /// this is the number of fixed arguments.
      U32 getNumArguments() const { return getFunctionType()->getArgumentTypeTable()->getNumArguments(); }
      
      ///
      const EngineTypeInfo* getArgumentType( U32 index ) const { return ( *( getFunctionType()->getArgumentTypeTable() ) )[ index ]; }
      
      /// Return the vector storing the default argument values.
      const EngineFunctionDefaultArguments* getDefaultArguments() const { return mDefaultArgumentValues; }
      
      /// Reset all callout function pointers back to NULL.  This deactivates all callbacks.
      static void resetAllCallouts();
};


///
///
/// Due to the given argument types and return type being directly used as is, it is not possible
/// to use this macro with engine types that have more complex value passing semantics (like e.g. 
/// String).  Use engineAPI in this case.
///
/// @note The method of defining functions exposed by this macro is very low-level.  To more
///   conveniently define API functions and methods, use the facilities provided in engineAPI.h.
///
/// @see engineAPI.h
#define DEFINE_CALLIN( bindingName, exportName, scope, returnType, args, defaultArgs, flags, doc )       \
   TORQUE_API returnType bindingName args;                                                               \
   namespace { namespace _ ## bindingName {                                                              \
      _EngineFunctionDefaultArguments< void args > sDefaultArgs defaultArgs;                             \
      EngineFunctionInfo sFunctionInfo(                                                                  \
         #exportName,                                                                                    \
         &_SCOPE< scope >()(),                                                                           \
         doc,                                                                                            \
         #returnType " " #exportName #args,                                                              \
         #bindingName,                                                                                   \
         TYPE< returnType args >(),                                                                      \
         &sDefaultArgs,                                                                                  \
         ( void* ) &bindingName,                                                                         \
         _EngineFunctionFlags( flags )                                                                   \
      );                                                                                                 \
   } }                                                                                                   \
   TORQUE_API returnType bindingName args
   
   
///
///
/// Not all control layers may be able to access data variables in a DLL so this macro exposes
/// both the variable and a set_XXX function to set the variable programmatically.
#define DEFINE_CALLOUT( bindingName, exportName, scope, returnType, args, flags, doc )                   \
   TORQUE_API returnType ( *bindingName ) args;                                                          \
   TORQUE_API void set_ ## bindingName( returnType ( *fn ) args )                                        \
      { bindingName = fn; }                                                                              \
   returnType ( *bindingName ) args;                                                                     \
   namespace {                                                                                           \
      ::EngineFunctionInfo _cb ## bindingName(                                                           \
         #exportName,                                                                                    \
         &::_SCOPE< scope >()(),                                                                         \
         doc,                                                                                            \
         #returnType " " #exportName #args,                                                              \
         #bindingName,                                                                                   \
         ::TYPE< returnType args >(),                                                                    \
         NULL,                                                                                           \
         ( void* ) &bindingName,                                                                         \
         EngineFunctionCallout | EngineFunctionFlags( flags )                                            \
      );                                                                                                 \
   }
   

#endif // !_ENGINEFUNCTIONS_H_
