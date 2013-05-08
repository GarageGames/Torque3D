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

#ifndef _ENGINETYPES_H_
#define _ENGINETYPES_H_

#ifndef _TYPETRAITS_H_
   #include "platform/typetraits.h"
#endif
#ifndef _BITSET_H_
   #include "core/bitSet.h"
#endif


//TODO: documentation


/// @file
/// This file forms the basis for Torque's engine API type system.
///
/// The type system is geared towards use in a control layer in a .NET-like environment
/// and is intended to allow for all-native data exchange between the engine and its
/// control layer.


/// @defgroup engineAPI_types Engine Type System
///
///
/// Akin to C++ RTTI (though more thorough in depth), the engine type system provides a reflection layer
/// that can be used to query type properties at run-time.
///
/// @section engineAPI_kinds Type Kinds
///
/// Engine types are segregated into kinds.  A kind is to a type what a type is to a value, i.e. a value
/// is an instance of a type whereas a type is an instance of a kind.
///
/// Just as values share behavior through types, types share behavior through kinds.
///
/// The following kinds are defined:
///
/// <dl>
///    <dt>Primitive</dt>
///    <dd>An atomic piece of data like an int, float, etc.</dt>
///    <dt>Enums</dt>
///    <dd></dd>
///    <dt>Bitfields</dt>
///    <dd>A bitwise combination of enumeration values.</dd>
///    <dt>Functions</dt>
///    <dd></dd>
///    <dt>Structs</dt>
///    <dd>A structured piece of data like a Point3F or MatrixF.  Unlike class instances, structs are directly stored
///      in the memory of their owner.  Also, structs don't allow inheritance.</dd>
///    <dt>Classes</dt>
///    <dd></dd>
/// </dl>
///
/// @section engineAPI_subtyping Subtyping of Engine Types
///
///
/// At the moment, the type system is not unified such that all types are implicitly subtypes to a single root type.
/// This may change in the future.
///
/// @section engineAPI_defaultConstruction Default Construction
///
/// All engine types must be default-constructible, i.e. a parameter-less construction of a value of any type
/// in the engine must be successful.  Class and struct types are affected the most by this.  Classes and structs
/// @b may provide non-default constructors but they @b must provide a default construction routine that always
/// succeeds.
///
/// @section engineAPI_networking Engine Type Networking
///
/// @{


class EngineExportScope;
class EngineTypeInfo;
class EnginePropertyTable;

template< typename T, typename Base > class EngineClassTypeInfo;
template< typename T > class EngineFunctionTypeInfo;


//--------------------------------------------------------------------------
//    Type Traits.
//--------------------------------------------------------------------------

/// @name Engine Type Traits
///
/// Type traits allow to access the static properties of a type at compile-time.  This
/// is primarily useful for templating in order to make decisions based on static
/// typing.
///
/// @{


template< typename T >
struct _EngineTypeTraits
{
   // Default type traits corresponding to the semantics of class types.
   
   typedef T Type;
   typedef T* ValueType;
   typedef typename Type::SuperType SuperType;
   
   typedef T* ArgumentValueType;
   typedef T* ReturnValueType;
   typedef T* DefaultArgumentValueStoreType;
   
   typedef ReturnValueType ReturnValue;
   static ValueType ArgumentToValue( ArgumentValueType val ) { return val; }

   static const EngineTypeInfo* const TYPEINFO;
};
template< typename T > const EngineTypeInfo* const _EngineTypeTraits< T >::TYPEINFO = &T::_smTypeInfo;

template<>
struct _EngineTypeTraits< void >
{
   typedef void Type;
   typedef void ValueType;
   typedef void ReturnValueType;   
   typedef ReturnValueType ReturnValue;
   
   static const EngineTypeInfo* const TYPEINFO;
};


template< typename T >
struct EngineTypeTraits : public _EngineTypeTraits< T > {};

// Strip off native type modifiers.  Doing it like this allows to query type traits on types
// that are not true engine types or value types thereof (like e.g. a reference to a primitive type)
// but it simplifies matters for us.
template< typename T >
struct EngineTypeTraits< T* > : public EngineTypeTraits< T > {};
template< typename T >
struct EngineTypeTraits< const T* > : public EngineTypeTraits< T > {};
template< typename T >
struct EngineTypeTraits< T& > : public EngineTypeTraits< T > {};
template< typename T >
struct EngineTypeTraits< const T& > : public EngineTypeTraits< T > {};
template< typename T >
struct EngineTypeTraits< const T > : public EngineTypeTraits< T > {};



/// Return the type info for the given engine type.
template< typename T >
inline const EngineTypeInfo* TYPE() { return EngineTypeTraits< T >::TYPEINFO; }


/// Return the type info for the @b static type of the given variable.
template< typename T >
inline const EngineTypeInfo* TYPE( const T& )
{
   return TYPE< T >();
}


// Base type trait definitions for different type kinds.

template< typename T >
struct _EnginePrimitiveTypeTraits
{
   typedef T Type;
   typedef T ValueType;
   typedef void SuperType;
   
   typedef ValueType ArgumentValueType;
   typedef ValueType ReturnValueType;
   typedef ValueType DefaultArgumentValueStoreType;
   
   typedef ReturnValueType ReturnValue;   
   static ValueType ArgumentToValue( ArgumentValueType val ) { return val; }

   static const EngineTypeInfo* const TYPEINFO;
};
template< typename T > const EngineTypeInfo* const _EnginePrimitiveTypeTraits< T >::TYPEINFO = TYPE< T >();

template< typename T >
struct _EngineEnumTypeTraits
{
   typedef T Type;
   typedef T ValueType;
   typedef void SuperType;

   typedef ValueType ArgumentValueType;
   typedef ValueType ReturnValueType;
   typedef ValueType DefaultArgumentValueStoreType;
   
   typedef ReturnValueType ReturnValue;
   static ValueType ArgumentToValue( ArgumentValueType val ) { return val; }

   static const EngineTypeInfo* const TYPEINFO;
};
template< typename T > const EngineTypeInfo* const _EngineEnumTypeTraits< T >::TYPEINFO = TYPE< T >();

template< typename T >
struct _EngineBitfieldTypeTraits
{
   typedef T Type;
   typedef T ValueType;
   typedef void SuperType;

   typedef ValueType ArgumentValueType;
   typedef ValueType ReturnValueType;
   typedef ValueType DefaultArgumentValueStoreType;
   
   typedef ReturnValueType ReturnValue;
   static ValueType ArgumentToValue( ArgumentValueType val ) { return val; }

   static const EngineTypeInfo* const TYPEINFO;
};
template< typename T > const EngineTypeInfo* const _EngineBitfieldTypeTraits< T >::TYPEINFO = TYPE< T >();

template< typename T >
struct _EngineStructTypeTraits
{
   typedef T Type;
   typedef const T& ValueType;
   typedef void SuperType;
   
   // Structs get passed in as pointers and passed out as full copies.
   typedef T* ArgumentValueType;
   typedef T ReturnValueType;
   typedef T DefaultArgumentValueStoreType;

   typedef ReturnValueType ReturnValue;
   static ValueType ArgumentToValue( ArgumentValueType val ) { return *val; }

   static const EngineTypeInfo* const TYPEINFO;
};
template< typename T > const EngineTypeInfo* const _EngineStructTypeTraits< T >::TYPEINFO = TYPE< T >();


template< typename T > struct _EngineArgumentTypeTable {};

template< typename T >
struct _EngineFunctionTypeTraits
{
      typedef T Type;
      typedef T* ValueType;

      typedef T* ArgumentValueType;
      typedef T* ReturnValueType;
      typedef T* DefaultArgumentValueStoreType;
      
      static const bool IS_VARIADIC = _EngineArgumentTypeTable< T >::VARIADIC;
      static const U32 NUM_ARGUMENTS  = _EngineArgumentTypeTable< T >::NUM_ARGUMENTS;

      static const EngineTypeInfo* const TYPEINFO;
   
   private:
      static const EngineFunctionTypeInfo< T > smTYPEINFO;
};
template< typename T > const EngineTypeInfo* const _EngineFunctionTypeTraits< T >::TYPEINFO = &smTYPEINFO;


// Temporary helper.  Used to strip the return or argument types supplied in a function type
// down to the value type and thus account for the discrepancy between (argument/return) value
// types and engine types.  Don't go for the base type as VC will not allow us to use abstract
// types even in return or argument positions of abstract function types (GCC allows it; not
// sure what the stance of the ANSI C++ standard towards this is).  Also, while it may be tempting
// to go for the real return and argument value types here, this would lead to errors as these
// are not guaranteed to be any meaningful value or base types to the engine type system.
#define T( x ) typename EngineTypeTraits< x >::ValueType

template< typename R >
struct _EngineTypeTraits< R() > : public _EngineFunctionTypeTraits< T( R )() > {};
template< typename R >
struct _EngineTypeTraits< R( ... ) > : public _EngineFunctionTypeTraits< T( R )( ... ) > {};
template< typename R, typename A >
struct _EngineTypeTraits< R( A ) > : public _EngineFunctionTypeTraits< T( R )( T( A ) ) > {};
template< typename R, typename A >
struct _EngineTypeTraits< R( A, ... ) > : public _EngineFunctionTypeTraits< T( R )( T( A ), ... ) > {};
template< typename R, typename A, typename B >
struct _EngineTypeTraits< R( A, B ) > : public _EngineFunctionTypeTraits< T( R )( T( A ), T( B ) ) > {};
template< typename R, typename A, typename B >
struct _EngineTypeTraits< R( A, B, ... ) > : public _EngineFunctionTypeTraits< T( R )( T( A ), T( B ), ... ) > {};
template< typename R, typename A, typename B, typename C >
struct _EngineTypeTraits< R( A, B, C ) > : public _EngineFunctionTypeTraits< T( R )( T( A ), T( B ), T( C ) ) > {};
template< typename R, typename A, typename B, typename C >
struct _EngineTypeTraits< R( A, B, C, ... ) > : public _EngineFunctionTypeTraits< T( R )( T( A ), T( B ), T( C ), ... ) > {};
template< typename R, typename A, typename B, typename C, typename D >
struct _EngineTypeTraits< R( A, B, C, D ) > : public _EngineFunctionTypeTraits< T( R )( T( A ), T( B ), T( C ), T( D ) ) > {};
template< typename R, typename A, typename B, typename C, typename D >
struct _EngineTypeTraits< R( A, B, C, D, ... ) > : public _EngineFunctionTypeTraits< T( R )( T( A ), T( B ), T( C ), T( D ), ... ) > {};
template< typename R, typename A, typename B, typename C, typename D, typename E >
struct _EngineTypeTraits< R( A, B, C, D, E ) > : public _EngineFunctionTypeTraits< T( R )( T( A ), T( B ), T( C ), T( D ), T( E ) ) > {};
template< typename R, typename A, typename B, typename C, typename D, typename E >
struct _EngineTypeTraits< R( A, B, C, D, E, ... ) > : public _EngineFunctionTypeTraits< T( R )( T( A ), T( B ), T( C ), T( D ), T( E ), ... ) > {};
template< typename R, typename A, typename B, typename C, typename D, typename E, typename F >
struct _EngineTypeTraits< R( A, B, C, D, E, F ) > : public _EngineFunctionTypeTraits< T( R )( T( A ), T( B ), T( C ), T( D ), T( E ), T( F ) ) > {};
template< typename R, typename A, typename B, typename C, typename D, typename E, typename F >
struct _EngineTypeTraits< R( A, B, C, D, E, F, ... ) > : public _EngineFunctionTypeTraits< T( R )( T( A ), T( B ), T( C ), T( D ), T( E ), T( F ), ... ) > {};
template< typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G >
struct _EngineTypeTraits< R( A, B, C, D, E, F, G ) > : public _EngineFunctionTypeTraits< T( R )( T( A ), T( B ), T( C ), T( D ), T( E ), T( F ), T( G ) ) > {};
template< typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G >
struct _EngineTypeTraits< R( A, B, C, D, E, F, G, ... ) > : public _EngineFunctionTypeTraits< T( R )( T( A ), T( B ), T( C ), T( D ), T( E ), T( F ), T( G ), ... ) > {};
template< typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H >
struct _EngineTypeTraits< R( A, B, C, D, E, F, G, H ) > : public _EngineFunctionTypeTraits< T( R )( T( A ), T( B ), T( C ), T( D ), T( E ), T( F ), T( G ), T( H ) ) > {};
template< typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H >
struct _EngineTypeTraits< R( A, B, C, D, E, F, G, H, ... ) > : public _EngineFunctionTypeTraits< T( R )( T( A ), T( B ), T( C ), T( D ), T( E ), T( F ), T( G ), T( H ), ... ) > {};
template< typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I >
struct _EngineTypeTraits< R( A, B, C, D, E, F, G, H, I ) > : public _EngineFunctionTypeTraits< T( R )( T( A ), T( B ), T( C ), T( D ), T( E ), T( F ), T( G ), T( H ), T( I ) ) > {};
template< typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I >
struct _EngineTypeTraits< R( A, B, C, D, E, F, G, H, I, ... ) > : public _EngineFunctionTypeTraits< T( R )( T( A ), T( B ), T( C ), T( D ), T( E ), T( F ), T( G ), T( H ), T( I ), ... ) > {};
template< typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J >
struct _EngineTypeTraits< R( A, B, C, D, E, F, G, H, I, J ) > : public _EngineFunctionTypeTraits< T( R )( T( A ), T( B ), T( C ), T( D ), T( E ), T( F ), T( G ), T( H ), T( I ), T( J ) ) > {};
template< typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J >
struct _EngineTypeTraits< R( A, B, C, D, E, F, G, H, I, J, ... ) > : public _EngineFunctionTypeTraits< T( R )( T( A ), T( B ), T( C ), T( D ), T( E ), T( F ), T( G ), T( H ), T( I ), T( J ), ... ) > {};
template< typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K >
struct _EngineTypeTraits< R( A, B, C, D, E, F, G, H, I, J, K ) > : public _EngineFunctionTypeTraits< T( R )( T( A ), T( B ), T( C ), T( D ), T( E ), T( F ), T( G ), T( H ), T( I ), T( J ), T( K ) ) > {};
template< typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K >
struct _EngineTypeTraits< R( A, B, C, D, E, F, G, H, I, J, K, ... ) > : public _EngineFunctionTypeTraits< T( R )( T( A ), T( B ), T( C ), T( D ), T( E ), T( F ), T( G ), T( H ), T( I ), T( J ), T( K ), ... ) > {};
template< typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L >
struct _EngineTypeTraits< R( A, B, C, D, E, F, G, H, I, J, K, L ) > : public _EngineFunctionTypeTraits< T( R )( T( A ), T( B ), T( C ), T( D ), T( E ), T( F ), T( G ), T( H ), T( I ), T( J ), T( K ), T( L ) ) > {};
template< typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L >
struct _EngineTypeTraits< R( A, B, C, D, E, F, G, H, I, J, K, L, ... ) > : public _EngineFunctionTypeTraits< T( R )( T( A ), T( B ), T( C ), T( D ), T( E ), T( F ), T( G ), T( H ), T( I ), T( J ), T( K ), T( L ), ... ) > {};

#undef T

/// @}

//--------------------------------------------------------------------------
//    Helpers.
//--------------------------------------------------------------------------

namespace _Private {
   template< typename T >
   struct _InstantiableConcreteClass
   {
      static bool _construct( void* ptr )
      {
         T* p = reinterpret_cast< T* >( ptr );
         constructInPlace( p );
         return true;
      }
   
      static void _destruct( void* ptr )
      {
         T* p = reinterpret_cast< T* >( ptr );
         destructInPlace( p );
      }
   };
   template< typename T >
   struct _NonInstantiableConcreteClass
   {
      static bool _construct( void* ptr )
      {
         AssertFatal( false, "EngineClassTypeInfo - constructInstance called on non-instantiable class" );
         return false;
      }
   
      static void _destruct( void* ptr )
      {
         AssertFatal( false, "EngineClassTypeInfo - destructInstance called on non-instantiable class" );
      }
   };
   template< typename T >
   struct _ConcreteClassBase : public IfTrueType< typename T::__IsInstantiableType, _InstantiableConcreteClass< T >, _NonInstantiableConcreteClass< T > >
   {
      typedef ::FalseType IsAbstractType;
   };
   template< typename T >
   struct _AbstractClassBase
   {
      typedef ::TrueType IsAbstractType;
      static bool _construct( void* ptr )
      {
         AssertFatal( false, "EngineClassTypeInfo - constructInstance called on abstract class" );
         return false;
      }
   
      static void _destruct( void* ptr )
      {
         AssertFatal( false, "EngineClassTypeInfo - destructInstance called on abstract class" );
      }
   };
}

//--------------------------------------------------------------------------
//    Macros.
//--------------------------------------------------------------------------

///
///
#define DECLARE_SCOPE( name )                                                                \
   struct name {                                                                             \
      static EngineExportScope __engineExportScopeInst;                                      \
      static EngineExportScope& __engineExportScope() { return __engineExportScopeInst; }    \
   };
   
///
#define IMPLEMENT_SCOPE( name, exportName, scope, doc )                                      \
   EngineExportScope name::__engineExportScopeInst( #exportName, &_SCOPE< scope >()(), doc );


#define _DECLARE_TYPE( type )                                                                \
   template<> const EngineTypeInfo* TYPE< type >();                                          \
   template<> struct _SCOPE< type > {                                                        \
      EngineExportScope& operator()() const {                                                \
         return *reinterpret_cast< EngineExportScope* >(                                     \
            const_cast< EngineTypeInfo* >( ( TYPE< type >() ) )                              \
         );                                                                                  \
      }                                                                                      \
   };

// Unlike the other types, primitives directly specialize on EngineTypeTraits instead
// of _EngineTypeTraits so as to prevent any stripping from taking place.  Otherwise,
// pointers could not be primitive types.
#define _DECLARE_PRIMITIVE( type )                                                           \
   _DECLARE_TYPE( type )                                                                     \
   template<>                                                                                \
   struct EngineTypeTraits< type > : public _EnginePrimitiveTypeTraits< type > {};
   
#define _DECLARE_ENUM( type )                                                                \
   _DECLARE_TYPE( type )                                                                     \
   template<>                                                                                \
   struct _EngineTypeTraits< type > : public _EngineEnumTypeTraits< type > {};
   
#define _DECLARE_BITFIELD( type )                                                            \
   _DECLARE_TYPE( type )                                                                     \
   template<>                                                                                \
   struct _EngineTypeTraits< type > : public _EngineBitfieldTypeTraits< type > {};

#define _DECLARE_STRUCT( type )                                                              \
   _DECLARE_TYPE( type )                                                                     \
   template<>                                                                                \
   struct _EngineTypeTraits< type > : public _EngineStructTypeTraits< type > {};


#define _IMPLEMENT_TYPE( type, exportName )        \
   template<>                                      \
   const EngineTypeInfo* TYPE< type >()            \
   {                                               \
      return &_ ## exportName::gsTypeInfo;         \
   }


#define _IMPLEMENT_PRIMITIVE( type, exportName, scope, doc )                                                                           \
   namespace { namespace _ ## exportName {                                                                                             \
      static EngineSimpleTypeInfo< type > gsTypeInfo( #exportName, &_SCOPE< scope >()(), EngineTypeKindPrimitive, doc );               \
   } }                                                                                                                                 \
   _IMPLEMENT_TYPE( type, exportName )
   
#define _IMPLEMENT_ENUM( type, exportName, scope, doc )                                                                                \
   namespace { namespace _ ## exportName {                                                                                             \
      typedef type EnumType;                                                                                                           \
      extern EngineSimpleTypeInfo< EnumType > gsTypeInfo;                                                                              \
   } }                                                                                                                                 \
   _IMPLEMENT_TYPE( type, exportName );                                                                                                \
   namespace { namespace _ ## exportName {                                                                                             \
      static const char* const _sEnumName = #exportName;                                                                               \
      static const char* const _sDoc = doc;                                                                                            \
      static EngineExportScope& _sScope = _SCOPE< scope >()();                                                                         \
      static EngineEnumTable::Value _sEnums[] = {
   
#define _END_IMPLEMENT_ENUM                                                                                                            \
      };                                                                                                                               \
      static EngineEnumTable _sEnumTable( sizeof( _sEnums ) / sizeof( _sEnums[ 0 ] ), _sEnums );                                       \
      EngineSimpleTypeInfo< EnumType > gsTypeInfo( _sEnumName, &_sScope, EngineTypeKindEnum, _sDoc, &_sEnumTable );                    \
   } }

#define _IMPLEMENT_BITFIELD( type, exportName, scope, doc )                                                                            \
   namespace { namespace _ ## exportName {                                                                                             \
      extern EngineSimpleTypeInfo< type > gsTypeInfo;                                                                                  \
   } }                                                                                                                                 \
   _IMPLEMENT_TYPE( type, exportName );                                                                                                \
   namespace { namespace _ ## exportName {                                                                                             \
      typedef type BitfieldType;                                                                                                       \
      static const char* const _sBitfieldName = #exportName;                                                                           \
      static const char* const _sDoc = doc;                                                                                            \
      static EngineExportScope& _sScope = _SCOPE< scope >()();                                                                         \
      static EngineEnumTable::Value _sEnums[] = {
   
#define _END_IMPLEMENT_BITFIELD                                                                                                        \
      };                                                                                                                               \
      static EngineEnumTable _sEnumTable( sizeof( _sEnums ) / sizeof( _sEnums[ 0 ] ), _sEnums );                                       \
      EngineSimpleTypeInfo< BitfieldType > gsTypeInfo( _sBitfieldName, &_sScope, EngineTypeKindBitfield, _sDoc, &_sEnumTable );        \
   } }
   
#define _IMPLEMENT_STRUCT( type, exportName, scope, doc )                                                                              \
   namespace { namespace _ ## exportName {                                                                                             \
      extern EngineStructTypeInfo< type > gsTypeInfo;                                                                                  \
   } }                                                                                                                                 \
   _IMPLEMENT_TYPE( type, exportName );                                                                                                \
   namespace { namespace _ ## exportName {                                                                                             \
      typedef type StructType;                                                                                                         \
      typedef StructType ThisType;                                                                                                     \
      static const char* const _sStructName = #exportName;                                                                             \
      static const char* const _sDoc = doc;                                                                                            \
      static EngineExportScope& _sScope = _SCOPE< scope >()();                                                                         \
      static EngineFieldTable::Field _sFields[] = {

#define _END_IMPLEMENT_STRUCT                                                                                                          \
         { NULL }                                                                                                                      \
      };                                                                                                                               \
      static EngineFieldTable _sFieldTable( sizeof( _sFields ) / sizeof( _sFields[ 0 ] ) - 1, _sFields );                              \
      EngineStructTypeInfo< StructType > gsTypeInfo( _sStructName, &_sScope, _sDoc, &_sFieldTable );                                   \
   } }


///
#define DECLARE_PRIMITIVE( type ) \
   _DECLARE_PRIMITIVE( type )

///
#define IMPLEMENT_PRIMITIVE( type, exportName, scope, doc ) \
   _IMPLEMENT_PRIMITIVE( type, exportName, scope, doc )

///
#define DECLARE_ENUM( type ) \
   _DECLARE_ENUM( type )
   
///
#define DECLARE_BITFIELD( type ) \
   _DECLARE_BITFIELD( type )

///
#define IMPLEMENT_ENUM( type, exportName, scope, doc ) \
   _IMPLEMENT_ENUM( type, exportName, scope, doc )
   
///
#define IMPLEMENT_BITFIELD( type, exportName, scope, doc ) \
   _IMPLEMENT_BITFIELD( type, exportName, scope, doc )

///
#define END_IMPLEMENT_ENUM \
   _END_IMPLEMENT_ENUM
   
///
#define END_IMPLEMENT_BITFIELD \
   _END_IMPLEMENT_BITFIELD

///
#define DECLARE_STRUCT( type ) \
   _DECLARE_STRUCT( type )

///
#define IMPLEMENT_STRUCT( type, exportName, scope, doc ) \
   _IMPLEMENT_STRUCT( type, exportName, scope, doc )

///
#define END_IMPLEMENT_STRUCT \
   _END_IMPLEMENT_STRUCT


///
#define FIELD( fieldName, exportName, numElements, doc ) \
   { #exportName, doc, numElements, TYPE( ( ( ThisType* ) 16 )->fieldName ), FIELDOFFSET( fieldName ) }, // Artificial offset to avoid compiler warnings.

///
#define FIELD_AS( type, fieldName, exportName, numElements, doc ) \
   { #exportName, doc, numElements, TYPE( *( ( type* ) &( ( ThisType* ) 16 )->fieldName ) ), FIELDOFFSET( fieldName ) }, // Artificial offset to avoid compiler warnings.
   
///
#define FIELDOFFSET( fieldName ) \
   U32( ( ( const char* ) &( ( ( ThisType* ) 16 )->fieldName ) ) - 16 ) // Artificial offset to avoid compiler warnings.
   
///
#define CLASSDOC( className, doc ) \
   template<> const char* EngineClassTypeInfo< className, className::_ClassBase >::smDocString = doc;
   

/// @}


struct _GLOBALSCOPE
{
   static EngineExportScope& __engineExportScope();
};

// Helper to retrieve a scope instance through a C++ type.  The setup here is a little
// contrived to allow the scope parameters to the various macros to be empty.  What makes
// this difficult is that T need not necessarily be a structured type.
template< typename T = _GLOBALSCOPE >
struct _SCOPE
{
   EngineExportScope& operator()() const { return T::__engineExportScope(); }
};

#endif // !_ENGINETYPES_H_
