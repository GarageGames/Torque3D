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

#ifndef _ENGINEAPI_H_
#define _ENGINEAPI_H_

#ifndef _CONSOLETYPES_H_
#include "console/consoleTypes.h"
#endif

#ifndef _CONSOLE_H_
#include "console/console.h"
#endif

#ifndef _STRINGFUNCTIONS_H_
#include "core/strings/stringFunctions.h"
#endif

#ifndef _SIMOBJECT_H_
#include "console/simObject.h"
#endif

#ifndef _ENGINEFUNCTIONS_H_
#include "console/engineFunctions.h"
#endif

// Whatever types are used in API definitions, their DECLAREs must be visible to the
// macros.  We include the basic primitive and struct types here.

#ifndef _ENGINEPRIMITIVES_H_
   #include "console/enginePrimitives.h"
#endif
#ifndef _ENGINESTRUCTS_H_
   #include "console/engineStructs.h"
#endif


/// @file
/// Definitions for exposing engine functionality to the control layer.
///
/// This file provides a convenience layer around the underlying engine interop system (which at
/// the moment still includes the legacy TorqueScript interop a.k.a. "console system").  The
/// macros exposed here will automatically take care of all marshalling, value type constraints,
/// reflection info instancing, etc. involved in defining engine API call-ins and call-outs.
///
/// @note At the moment, this file supplies both the legacy TorqueScript console system as well
///   as the new engine export system with the structures and information they need.  In the
///   near future, the console-based parts will get purged.  This will not result in visible
///   changes to users of the functionality here except for the string-based marshalling
///   functions currently exposed (which will also disappear).



//TODO: Disable warning for extern "C" functions returning UDTs for now; need to take a closer look at this
#pragma warning( disable : 4190 )



// Disable some VC warnings that are irrelevant to us.
#pragma warning( disable : 4510 ) // default constructor could not be generated; all the Args structures are never constructed by us
#pragma warning( disable : 4610 ) // can never be instantiated; again Args is never constructed by us


namespace engineAPI {

   /// Flag for enabling legacy console behavior in the interop system while
   /// we still have it around.  Will disappear along with console.
   extern bool gUseConsoleInterop;
   
   /// Flag to allow engine functions to detect whether the engine had been
   /// initialized or shut down.
   extern bool gIsInitialized;
}


//FIXME: this allows const char* to be used as a struct field type

// Temp support for allowing const char* to remain in the API functions as long as we
// still have the console system around.  When that is purged, these definitions should
// be deleted and all const char* uses be replaced with String.
template<> struct EngineTypeTraits< const char* > : public EngineTypeTraits< String > {};
template<> inline const EngineTypeInfo* TYPE< const char* >() { return TYPE< String >(); }


/// @name Marshalling
///
/// Functions for converting to/from string-based data representations.
///
/// @note This functionality is specific to the console interop.
/// @{

/// Marshal a single piece of data from native into client form.
template< typename T >
inline const char* EngineMarshallData( const T& value )
{
   return castConsoleTypeToString( value );
}
inline const char* EngineMarshallData( bool value )
{
   if( value )
      return "1";
   else
      return "0";
}
inline const char* EngineMarshallData( const char* str )
{
   // The API assumes that if you pass a plain "const char*" through it, then you are referring
   // to string storage with non-local lifetime that can be safely passed to the control layer.
   return str;
}
template< typename T >
inline const char* EngineMarshallData( T* object )
{
   return ( object ? object->getIdString() : "0" );
}
template< typename T >
inline const char* EngineMarshallData( const T* object )
{
   return ( object ? object->getIdString() : "0" );
}
inline const char* EngineMarshallData( U32 value )
{
   return EngineMarshallData( S32( value ) );
}

/// Marshal data from native into client form stored directly in
/// client function invocation vector.
template< typename T >
inline void EngineMarshallData( const T& arg, S32& argc, const char** argv )
{
   argv[ argc ] = Con::getStringArg( castConsoleTypeToString( arg ) );
   argc ++;
}
inline void EngineMarshallData( bool arg, S32& argc, const char** argv )
{
   if( arg )
      argv[ argc ] = "1";
   else
      argv[ argc ] = "0";
   argc ++;
}
inline void EngineMarshallData( S32 arg, S32& argc, const char** argv )
{
   argv[ argc ] = Con::getIntArg( arg );
   argc ++;
}
inline void EngineMarshallData( U32 arg, S32& argc, const char** argv )
{
   EngineMarshallData( S32( arg ), argc, argv );
}
inline void EngineMarshallData( F32 arg, S32& argc, const char** argv )
{
   argv[ argc ] = Con::getFloatArg( arg );
   argc ++;
}
inline void EngineMarshallData( const char* arg, S32& argc, const char** argv )
{
   argv[ argc ] = arg;
   argc ++;
}
template< typename T >
inline void EngineMarshallData( T* object, S32& argc, const char** argv )
{
   argv[ argc ] = ( object ? object->getIdString() : "0" );
   argc ++;
}
template< typename T >
inline void EngineMarshallData( const T* object, S32& argc, const char** argv )
{
   argv[ argc ] = ( object ? object->getIdString() : "0" );
   argc ++;
}

/// Unmarshal data from client form to engine form.
///
/// This is wrapped in an a struct as partial specializations on function
/// templates are not allowed in C++.
template< typename T >
struct EngineUnmarshallData
{
   T operator()( const char* str ) const
   {
      T value;
      castConsoleTypeFromString( value, str );
      return value;
   }
};
template<>
struct EngineUnmarshallData< S32 >
{
   S32 operator()( const char* str ) const
   {
      return dAtoi( str );
   }
};
template<>
struct EngineUnmarshallData< U32 >
{
   U32 operator()( const char* str ) const
   {
      return dAtoui( str );
   }
};
template<>
struct EngineUnmarshallData< F32 >
{
   F32 operator()( const char* str ) const
   {
      return dAtof( str );
   }
};
template<>
struct EngineUnmarshallData< const char* >
{
   const char* operator()( const char* str ) const
   {
      return str;
   }
};
template< typename T >
struct EngineUnmarshallData< T* >
{
   T* operator()( const char* str ) const
   {
      return dynamic_cast< T* >( Sim::findObject( str ) );
   }
};
template<>
struct EngineUnmarshallData< void >
{
   void operator()( const char* ) const {}
};

/// @}


/// @name C to C++ Trampolines
///
/// The trampolines serve two purposes:
///
/// For one, they ensure that no matter what argument types are specified by users of the engine API macros, the correct
/// argument value types are enforced on the functions exported by the engine.  Let's say, for example, the user writes
/// a function that takes a "Point3F direction" argument, then the template machinery here will automatically expose an
/// API function that takes a "Point3F& direction" argument.
///
/// Secondly, the templates jump the incoming calls from extern "C" space into C++ space.  This is mostly relevant for
/// methods only as they will need an implicit object type argument.
///
/// @{

// Helper type to factor out commonalities between function and method trampolines.
template< typename T >
struct _EngineTrampoline
{
   struct Args {};
};
template< typename R, typename A >
struct _EngineTrampoline< R( A ) >
{
   struct Args
   {
      char data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) ];
      
      typename EngineTypeTraits< A >::ValueType a() const
      {
         return EngineTypeTraits< A >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< A >::ArgumentValueType* >( &data[ 0 ] ) ) );
      }
   };
};
template< typename R, typename A, typename B >
struct _EngineTrampoline< R( A, B ) >
{
   struct Args
   {
      char data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) +
                 sizeof( typename EngineTypeTraits< B >::ArgumentValueType ) ];

      typename EngineTypeTraits< A >::ValueType a() const
      {
         return EngineTypeTraits< A >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< A >::ArgumentValueType* >( &data[ 0 ] ) ) );
      }
      typename EngineTypeTraits< B >::ValueType b() const
      {
         return EngineTypeTraits< B >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< B >::ArgumentValueType* >
               ( &data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) ] ) ) );
      }
   };
};
template< typename R, typename A, typename B, typename C >
struct _EngineTrampoline< R( A, B, C ) >
{
   struct Args
   {
      char data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) +
                 sizeof( typename EngineTypeTraits< B >::ArgumentValueType ) +
                 sizeof( typename EngineTypeTraits< C >::ArgumentValueType ) ];

      typename EngineTypeTraits< A >::ValueType a() const
      {
         return EngineTypeTraits< A >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< A >::ArgumentValueType* >( &data[ 0 ] ) ) );
      }
      typename EngineTypeTraits< B >::ValueType b() const
      {
         return EngineTypeTraits< B >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< B >::ArgumentValueType* >
               ( &data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) ] ) ) );
      }
      typename EngineTypeTraits< C >::ValueType c() const
      {
         return EngineTypeTraits< C >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< C >::ArgumentValueType* >
               ( &data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< B >::ArgumentValueType ) ] ) ) );
      }
   };
};
template< typename R, typename A, typename B, typename C, typename D >
struct _EngineTrampoline< R( A, B, C, D ) >
{
   struct Args
   {
      char data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) +
                 sizeof( typename EngineTypeTraits< B >::ArgumentValueType ) +
                 sizeof( typename EngineTypeTraits< C >::ArgumentValueType ) +
                 sizeof( typename EngineTypeTraits< D >::ArgumentValueType ) ];

      typename EngineTypeTraits< A >::ValueType a() const
      {
         return EngineTypeTraits< A >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< A >::ArgumentValueType* >( &data[ 0 ] ) ) );
      }
      typename EngineTypeTraits< B >::ValueType b() const
      {
         return EngineTypeTraits< B >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< B >::ArgumentValueType* >
               ( &data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) ] ) ) );
      }
      typename EngineTypeTraits< C >::ValueType c() const
      {
         return EngineTypeTraits< C >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< C >::ArgumentValueType* >
               ( &data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< B >::ArgumentValueType ) ] ) ) );
      }
      typename EngineTypeTraits< D >::ValueType d() const
      {
         return EngineTypeTraits< D >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< D >::ArgumentValueType* >
               ( &data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< B >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< C >::ArgumentValueType ) ] ) ) );
      }
   };
};
template< typename R, typename A, typename B, typename C, typename D, typename E >
struct _EngineTrampoline< R( A, B, C, D, E ) >
{
   struct Args
   {
      char data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) +
                 sizeof( typename EngineTypeTraits< B >::ArgumentValueType ) +
                 sizeof( typename EngineTypeTraits< C >::ArgumentValueType ) +
                 sizeof( typename EngineTypeTraits< D >::ArgumentValueType ) +
                 sizeof( typename EngineTypeTraits< E >::ArgumentValueType ) ];

      typename EngineTypeTraits< A >::ValueType a() const
      {
         return EngineTypeTraits< A >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< A >::ArgumentValueType* >( &data[ 0 ] ) ) );
      }
      typename EngineTypeTraits< B >::ValueType b() const
      {
         return EngineTypeTraits< B >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< B >::ArgumentValueType* >
               ( &data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) ] ) ) );
      }
      typename EngineTypeTraits< C >::ValueType c() const
      {
         return EngineTypeTraits< C >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< C >::ArgumentValueType* >
               ( &data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< B >::ArgumentValueType ) ] ) ) );
      }
      typename EngineTypeTraits< D >::ValueType d() const
      {
         return EngineTypeTraits< D >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< D >::ArgumentValueType* >
               ( &data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< B >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< C >::ArgumentValueType ) ] ) ) );
      }
      typename EngineTypeTraits< E >::ValueType e() const
      {
         return EngineTypeTraits< E >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< E >::ArgumentValueType* >
               ( &data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< B >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< C >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< D >::ArgumentValueType ) ] ) ) );
      }
   };
};
template< typename R, typename A, typename B, typename C, typename D, typename E, typename F >
struct _EngineTrampoline< R( A, B, C, D, E, F ) >
{
   struct Args
   {
      char data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) +
                 sizeof( typename EngineTypeTraits< B >::ArgumentValueType ) +
                 sizeof( typename EngineTypeTraits< C >::ArgumentValueType ) +
                 sizeof( typename EngineTypeTraits< D >::ArgumentValueType ) +
                 sizeof( typename EngineTypeTraits< E >::ArgumentValueType ) +
                 sizeof( typename EngineTypeTraits< F >::ArgumentValueType ) ];

      typename EngineTypeTraits< A >::ValueType a() const
      {
         return EngineTypeTraits< A >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< A >::ArgumentValueType* >( &data[ 0 ] ) ) );
      }
      typename EngineTypeTraits< B >::ValueType b() const
      {
         return EngineTypeTraits< B >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< B >::ArgumentValueType* >
               ( &data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) ] ) ) );
      }
      typename EngineTypeTraits< C >::ValueType c() const
      {
         return EngineTypeTraits< C >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< C >::ArgumentValueType* >
               ( &data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< B >::ArgumentValueType ) ] ) ) );
      }
      typename EngineTypeTraits< D >::ValueType d() const
      {
         return EngineTypeTraits< D >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< D >::ArgumentValueType* >
               ( &data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< B >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< C >::ArgumentValueType ) ] ) ) );
      }
      typename EngineTypeTraits< E >::ValueType e() const
      {
         return EngineTypeTraits< E >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< E >::ArgumentValueType* >
               ( &data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< B >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< C >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< D >::ArgumentValueType ) ] ) ) );
      }
      typename EngineTypeTraits< F >::ValueType f() const
      {
         return EngineTypeTraits< F >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< F >::ArgumentValueType* >
               ( &data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< B >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< C >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< D >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< E >::ArgumentValueType ) ] ) ) );
      }
   };
};
template< typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G >
struct _EngineTrampoline< R( A, B, C, D, E, F, G ) >
{
   struct Args
   {
      char data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) +
                 sizeof( typename EngineTypeTraits< B >::ArgumentValueType ) +
                 sizeof( typename EngineTypeTraits< C >::ArgumentValueType ) +
                 sizeof( typename EngineTypeTraits< D >::ArgumentValueType ) +
                 sizeof( typename EngineTypeTraits< E >::ArgumentValueType ) +
                 sizeof( typename EngineTypeTraits< F >::ArgumentValueType ) +
                 sizeof( typename EngineTypeTraits< G >::ArgumentValueType ) ];

      typename EngineTypeTraits< A >::ValueType a() const
      {
         return EngineTypeTraits< A >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< A >::ArgumentValueType* >( &data[ 0 ] ) ) );
      }
      typename EngineTypeTraits< B >::ValueType b() const
      {
         return EngineTypeTraits< B >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< B >::ArgumentValueType* >
               ( &data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) ] ) ) );
      }
      typename EngineTypeTraits< C >::ValueType c() const
      {
         return EngineTypeTraits< C >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< C >::ArgumentValueType* >
               ( &data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< B >::ArgumentValueType ) ] ) ) );
      }
      typename EngineTypeTraits< D >::ValueType d() const
      {
         return EngineTypeTraits< D >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< D >::ArgumentValueType* >
               ( &data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< B >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< C >::ArgumentValueType ) ] ) ) );
      }
      typename EngineTypeTraits< E >::ValueType e() const
      {
         return EngineTypeTraits< E >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< E >::ArgumentValueType* >
               ( &data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< B >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< C >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< D >::ArgumentValueType ) ] ) ) );
      }
      typename EngineTypeTraits< F >::ValueType f() const
      {
         return EngineTypeTraits< F >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< F >::ArgumentValueType* >
               ( &data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< B >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< C >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< D >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< E >::ArgumentValueType ) ] ) ) );
      }
      typename EngineTypeTraits< G >::ValueType g() const
      {
         return EngineTypeTraits< G >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< G >::ArgumentValueType* >
               ( &data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< B >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< C >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< D >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< E >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< F >::ArgumentValueType ) ] ) ) );
      }
   };
};
template< typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H >
struct _EngineTrampoline< R( A, B, C, D, E, F, G, H ) >
{
   struct Args
   {
      char data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) +
                 sizeof( typename EngineTypeTraits< B >::ArgumentValueType ) +
                 sizeof( typename EngineTypeTraits< C >::ArgumentValueType ) +
                 sizeof( typename EngineTypeTraits< D >::ArgumentValueType ) +
                 sizeof( typename EngineTypeTraits< E >::ArgumentValueType ) +
                 sizeof( typename EngineTypeTraits< F >::ArgumentValueType ) +
                 sizeof( typename EngineTypeTraits< G >::ArgumentValueType ) +
                 sizeof( typename EngineTypeTraits< H >::ArgumentValueType ) ];

      typename EngineTypeTraits< A >::ValueType a() const
      {
         return EngineTypeTraits< A >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< A >::ArgumentValueType* >( &data[ 0 ] ) ) );
      }
      typename EngineTypeTraits< B >::ValueType b() const
      {
         return EngineTypeTraits< B >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< B >::ArgumentValueType* >
               ( &data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) ] ) ) );
      }
      typename EngineTypeTraits< C >::ValueType c() const
      {
         return EngineTypeTraits< C >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< C >::ArgumentValueType* >
               ( &data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< B >::ArgumentValueType ) ] ) ) );
      }
      typename EngineTypeTraits< D >::ValueType d() const
      {
         return EngineTypeTraits< D >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< D >::ArgumentValueType* >
               ( &data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< B >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< C >::ArgumentValueType ) ] ) ) );
      }
      typename EngineTypeTraits< E >::ValueType e() const
      {
         return EngineTypeTraits< E >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< E >::ArgumentValueType* >
               ( &data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< B >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< C >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< D >::ArgumentValueType ) ] ) ) );
      }
      typename EngineTypeTraits< F >::ValueType f() const
      {
         return EngineTypeTraits< F >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< F >::ArgumentValueType* >
               ( &data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< B >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< C >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< D >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< E >::ArgumentValueType ) ] ) ) );
      }
      typename EngineTypeTraits< G >::ValueType g() const
      {
         return EngineTypeTraits< G >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< G >::ArgumentValueType* >
               ( &data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< B >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< C >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< D >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< E >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< F >::ArgumentValueType ) ] ) ) );
      }
      typename EngineTypeTraits< H >::ValueType h() const
      {
         return EngineTypeTraits< H >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< H >::ArgumentValueType* >
               ( &data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< B >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< C >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< D >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< E >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< F >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< G >::ArgumentValueType ) ] ) ) );
      }
   };
};
template< typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I >
struct _EngineTrampoline< R( A, B, C, D, E, F, G, H, I ) >
{
   struct Args
   {
      char data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) +
                 sizeof( typename EngineTypeTraits< B >::ArgumentValueType ) +
                 sizeof( typename EngineTypeTraits< C >::ArgumentValueType ) +
                 sizeof( typename EngineTypeTraits< D >::ArgumentValueType ) +
                 sizeof( typename EngineTypeTraits< E >::ArgumentValueType ) +
                 sizeof( typename EngineTypeTraits< F >::ArgumentValueType ) +
                 sizeof( typename EngineTypeTraits< G >::ArgumentValueType ) +
                 sizeof( typename EngineTypeTraits< H >::ArgumentValueType ) +
                 sizeof( typename EngineTypeTraits< I >::ArgumentValueType ) ];

      typename EngineTypeTraits< A >::ValueType a() const
      {
         return EngineTypeTraits< A >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< A >::ArgumentValueType* >( &data[ 0 ] ) ) );
      }
      typename EngineTypeTraits< B >::ValueType b() const
      {
         return EngineTypeTraits< B >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< B >::ArgumentValueType* >
               ( &data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) ] ) ) );
      }
      typename EngineTypeTraits< C >::ValueType c() const
      {
         return EngineTypeTraits< C >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< C >::ArgumentValueType* >
               ( &data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< B >::ArgumentValueType ) ] ) ) );
      }
      typename EngineTypeTraits< D >::ValueType d() const
      {
         return EngineTypeTraits< D >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< D >::ArgumentValueType* >
               ( &data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< B >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< C >::ArgumentValueType ) ] ) ) );
      }
      typename EngineTypeTraits< E >::ValueType e() const
      {
         return EngineTypeTraits< E >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< E >::ArgumentValueType* >
               ( &data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< B >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< C >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< D >::ArgumentValueType ) ] ) ) );
      }
      typename EngineTypeTraits< F >::ValueType f() const
      {
         return EngineTypeTraits< F >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< F >::ArgumentValueType* >
               ( &data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< B >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< C >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< D >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< E >::ArgumentValueType ) ] ) ) );
      }
      typename EngineTypeTraits< G >::ValueType g() const
      {
         return EngineTypeTraits< G >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< G >::ArgumentValueType* >
               ( &data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< B >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< C >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< D >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< E >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< F >::ArgumentValueType ) ] ) ) );
      }
      typename EngineTypeTraits< H >::ValueType h() const
      {
         return EngineTypeTraits< H >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< H >::ArgumentValueType* >
               ( &data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< B >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< C >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< D >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< E >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< F >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< G >::ArgumentValueType ) ] ) ) );
      }
      typename EngineTypeTraits< I >::ValueType i() const
      {
         return EngineTypeTraits< I >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< I >::ArgumentValueType* >
               ( &data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< B >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< C >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< D >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< E >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< F >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< G >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< H >::ArgumentValueType ) ] ) ) );
      }
   };
};
template< typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J >
struct _EngineTrampoline< R( A, B, C, D, E, F, G, H, I, J ) >
{
   struct Args
   {
      char data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) +
                 sizeof( typename EngineTypeTraits< B >::ArgumentValueType ) +
                 sizeof( typename EngineTypeTraits< C >::ArgumentValueType ) +
                 sizeof( typename EngineTypeTraits< D >::ArgumentValueType ) +
                 sizeof( typename EngineTypeTraits< E >::ArgumentValueType ) +
                 sizeof( typename EngineTypeTraits< F >::ArgumentValueType ) +
                 sizeof( typename EngineTypeTraits< G >::ArgumentValueType ) +
                 sizeof( typename EngineTypeTraits< H >::ArgumentValueType ) +
                 sizeof( typename EngineTypeTraits< I >::ArgumentValueType ) +
                 sizeof( typename EngineTypeTraits< J >::ArgumentValueType ) ];

      typename EngineTypeTraits< A >::ValueType a() const
      {
         return EngineTypeTraits< A >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< A >::ArgumentValueType* >( &data[ 0 ] ) ) );
      }
      typename EngineTypeTraits< B >::ValueType b() const
      {
         return EngineTypeTraits< B >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< B >::ArgumentValueType* >
               ( &data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) ] ) ) );
      }
      typename EngineTypeTraits< C >::ValueType c() const
      {
         return EngineTypeTraits< C >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< C >::ArgumentValueType* >
               ( &data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< B >::ArgumentValueType ) ] ) ) );
      }
      typename EngineTypeTraits< D >::ValueType d() const
      {
         return EngineTypeTraits< D >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< D >::ArgumentValueType* >
               ( &data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< B >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< C >::ArgumentValueType ) ] ) ) );
      }
      typename EngineTypeTraits< E >::ValueType e() const
      {
         return EngineTypeTraits< E >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< E >::ArgumentValueType* >
               ( &data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< B >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< C >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< D >::ArgumentValueType ) ] ) ) );
      }
      typename EngineTypeTraits< F >::ValueType f() const
      {
         return EngineTypeTraits< F >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< F >::ArgumentValueType* >
               ( &data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< B >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< C >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< D >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< E >::ArgumentValueType ) ] ) ) );
      }
      typename EngineTypeTraits< G >::ValueType g() const
      {
         return EngineTypeTraits< G >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< G >::ArgumentValueType* >
               ( &data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< B >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< C >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< D >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< E >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< F >::ArgumentValueType ) ] ) ) );
      }
      typename EngineTypeTraits< H >::ValueType h() const
      {
         return EngineTypeTraits< H >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< H >::ArgumentValueType* >
               ( &data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< B >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< C >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< D >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< E >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< F >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< G >::ArgumentValueType ) ] ) ) );
      }
      typename EngineTypeTraits< I >::ValueType i() const
      {
         return EngineTypeTraits< I >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< I >::ArgumentValueType* >
               ( &data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< B >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< C >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< D >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< E >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< F >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< G >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< H >::ArgumentValueType ) ] ) ) );
      }
      typename EngineTypeTraits< J >::ValueType j() const
      {
         return EngineTypeTraits< J >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< J >::ArgumentValueType* >
               ( &data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< B >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< C >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< D >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< E >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< F >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< G >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< H >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< I >::ArgumentValueType ) ] ) ) );
      }
   };
};
template< typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K >
struct _EngineTrampoline< R( A, B, C, D, E, F, G, H, I, J, K ) >
{
   struct Args
   {
      char data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) +
                 sizeof( typename EngineTypeTraits< B >::ArgumentValueType ) +
                 sizeof( typename EngineTypeTraits< C >::ArgumentValueType ) +
                 sizeof( typename EngineTypeTraits< D >::ArgumentValueType ) +
                 sizeof( typename EngineTypeTraits< E >::ArgumentValueType ) +
                 sizeof( typename EngineTypeTraits< F >::ArgumentValueType ) +
                 sizeof( typename EngineTypeTraits< G >::ArgumentValueType ) +
                 sizeof( typename EngineTypeTraits< H >::ArgumentValueType ) +
                 sizeof( typename EngineTypeTraits< I >::ArgumentValueType ) +
                 sizeof( typename EngineTypeTraits< J >::ArgumentValueType ) +
                 sizeof( typename EngineTypeTraits< K >::ArgumentValueType ) ];

      typename EngineTypeTraits< A >::ValueType a() const
      {
         return EngineTypeTraits< A >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< A >::ArgumentValueType* >( &data[ 0 ] ) ) );
      }
      typename EngineTypeTraits< B >::ValueType b() const
      {
         return EngineTypeTraits< B >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< B >::ArgumentValueType* >
               ( &data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) ] ) ) );
      }
      typename EngineTypeTraits< C >::ValueType c() const
      {
         return EngineTypeTraits< C >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< C >::ArgumentValueType* >
               ( &data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< B >::ArgumentValueType ) ] ) ) );
      }
      typename EngineTypeTraits< D >::ValueType d() const
      {
         return EngineTypeTraits< D >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< D >::ArgumentValueType* >
               ( &data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< B >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< C >::ArgumentValueType ) ] ) ) );
      }
      typename EngineTypeTraits< E >::ValueType e() const
      {
         return EngineTypeTraits< E >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< E >::ArgumentValueType* >
               ( &data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< B >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< C >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< D >::ArgumentValueType ) ] ) ) );
      }
      typename EngineTypeTraits< F >::ValueType f() const
      {
         return EngineTypeTraits< F >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< F >::ArgumentValueType* >
               ( &data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< B >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< C >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< D >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< E >::ArgumentValueType ) ] ) ) );
      }
      typename EngineTypeTraits< G >::ValueType g() const
      {
         return EngineTypeTraits< G >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< G >::ArgumentValueType* >
               ( &data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< B >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< C >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< D >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< E >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< F >::ArgumentValueType ) ] ) ) );
      }
      typename EngineTypeTraits< H >::ValueType h() const
      {
         return EngineTypeTraits< H >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< H >::ArgumentValueType* >
               ( &data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< B >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< C >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< D >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< E >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< F >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< G >::ArgumentValueType ) ] ) ) );
      }
      typename EngineTypeTraits< I >::ValueType i() const
      {
         return EngineTypeTraits< I >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< I >::ArgumentValueType* >
               ( &data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< B >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< C >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< D >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< E >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< F >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< G >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< H >::ArgumentValueType ) ] ) ) );
      }
      typename EngineTypeTraits< J >::ValueType j() const
      {
         return EngineTypeTraits< J >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< J >::ArgumentValueType* >
               ( &data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< B >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< C >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< D >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< E >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< F >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< G >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< H >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< I >::ArgumentValueType ) ] ) ) );
      }
      typename EngineTypeTraits< K >::ValueType k() const
      {
         return EngineTypeTraits< K >::ArgumentToValue(
            *( reinterpret_cast< const typename EngineTypeTraits< K >::ArgumentValueType* >
               ( &data[ sizeof( typename EngineTypeTraits< A >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< B >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< C >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< D >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< E >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< F >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< G >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< H >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< I >::ArgumentValueType ) +
                        sizeof( typename EngineTypeTraits< J >::ArgumentValueType ) ] ) ) );
      }
   };
};

template< typename T >
struct _EngineFunctionTrampolineBase : public _EngineTrampoline< T >
{
   typedef T FunctionType;
};

// Trampolines for any call-ins that aren't methods.
template< typename T >
struct _EngineFunctionTrampoline {};

template< typename R >
struct _EngineFunctionTrampoline< R() > : public _EngineFunctionTrampolineBase< R() >
{
   static R jmp( R ( *fn )(), const typename _EngineFunctionTrampolineBase< R() >::Args& args )
   {
      return R( fn() );
   }
};
template< typename R, typename A >
struct _EngineFunctionTrampoline< R( A ) > : public _EngineFunctionTrampolineBase< R( A ) >
{
   static R jmp( R ( *fn )( A ), const typename _EngineFunctionTrampolineBase< R( A ) >::Args& args )
   {
      return R( fn( args.a() ) );
   }
};
template< typename R, typename A, typename B >
struct _EngineFunctionTrampoline< R( A, B ) > : public _EngineFunctionTrampolineBase< R( A, B ) >
{
   static R jmp( R ( *fn )( A, B ), const typename _EngineFunctionTrampolineBase< R( A, B ) >::Args& args )
   {
      return R( fn( args.a(), args.b() ) );
   }
};
template< typename R, typename A, typename B, typename C >
struct _EngineFunctionTrampoline< R( A, B, C ) > : public _EngineFunctionTrampolineBase< R( A, B, C ) >
{
   static R jmp( R ( *fn )( A, B, C ), const typename _EngineFunctionTrampolineBase< R( A, B, C ) >::Args& args )
   {
      return R( fn( args.a(), args.b(), args.c() ) );
   }
};
template< typename R, typename A, typename B, typename C, typename D >
struct _EngineFunctionTrampoline< R( A, B, C, D ) > : public _EngineFunctionTrampolineBase< R( A, B, C, D ) >
{
   static R jmp( R ( *fn )( A, B, C, D ), const typename _EngineFunctionTrampolineBase< R( A, B, C, D ) >::Args& args )
   {
      return R( fn( args.a(), args.b(), args.c(), args.d() ) );
   }
};
template< typename R, typename A, typename B, typename C, typename D, typename E >
struct _EngineFunctionTrampoline< R( A, B, C, D, E ) > : public _EngineFunctionTrampolineBase< R( A, B, C, D, E ) >
{
   static R jmp( R ( *fn )( A, B, C, D, E ), const typename _EngineFunctionTrampolineBase< R( A, B, C, D, E ) >::Args& args )
   {
      return R( fn( args.a(), args.b(), args.c(), args.d(), args.e() ) );
   }
};
template< typename R, typename A, typename B, typename C, typename D, typename E, typename F >
struct _EngineFunctionTrampoline< R( A, B, C, D, E, F ) > : public _EngineFunctionTrampolineBase< R( A, B, C, D, E, F ) >
{
   static R jmp( R ( *fn )( A, B, C, D, E, F ), const typename _EngineFunctionTrampolineBase< R( A, B, C, D, E, F ) >::Args& args )
   {
      return R( fn( args.a(), args.b(), args.c(), args.d(), args.e(), args.f() ) );
   }
};
template< typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G >
struct _EngineFunctionTrampoline< R( A, B, C, D, E, F, G ) > : public _EngineFunctionTrampolineBase< R( A, B, C, D, E, F, G ) >
{
   static R jmp( R ( *fn )( A, B, C, D, E, F, G ), const typename _EngineFunctionTrampolineBase< R( A, B, C, D, E, F, G ) >::Args& args )
   {
      return R( fn( args.a(), args.b(), args.c(), args.d(), args.e(), args.f(), args.g() ) );
   }
};
template< typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H >
struct _EngineFunctionTrampoline< R( A, B, C, D, E, F, G, H ) > : public _EngineFunctionTrampolineBase< R( A, B, C, D, E, F, G, H ) >
{
   static R jmp( R ( *fn )( A, B, C, D, E, F, G, H ), const typename _EngineFunctionTrampolineBase< R( A, B, C, D, E, F, G, H ) >::Args& args )
   {
      return R( fn( args.a(), args.b(), args.c(), args.d(), args.e(), args.f(), args.g(), args.h() ) );
   }
};
template< typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I >
struct _EngineFunctionTrampoline< R( A, B, C, D, E, F, G, H, I ) > : public _EngineFunctionTrampolineBase< R( A, B, C, D, E, F, G, H, I ) >
{
   static R jmp( R ( *fn )( A, B, C, D, E, F, G, H, I ), const typename _EngineFunctionTrampolineBase< R( A, B, C, D, E, F, G, H, I ) >::Args& args )
   {
      return R( fn( args.a(), args.b(), args.c(), args.d(), args.e(), args.f(), args.g(), args.h(), args.i() ) );
   }
};
template< typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J >
struct _EngineFunctionTrampoline< R( A, B, C, D, E, F, G, H, I, J ) > : public _EngineFunctionTrampolineBase< R( A, B, C, D, E, F, G, H, I, J ) >
{
   static R jmp( R ( *fn )( A, B, C, D, E, F, G, H, I, J ), const typename _EngineFunctionTrampolineBase< R( A, B, C, D, E, F, G, H, I, J ) >::Args& args )
   {
      return R( fn( args.a(), args.b(), args.c(), args.d(), args.e(), args.f(), args.g(), args.h(), args.i(), args.j() ) );
   }
};
template< typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K >
struct _EngineFunctionTrampoline< R( A, B, C, D, E, F, G, H, I, J, K ) > : public _EngineFunctionTrampolineBase< R( A, B, C, D, E, F, G, H, I, J, K ) >
{
   static R jmp( R ( *fn )( A, B, C, D, E, F, G, H, I, J, K ), const typename _EngineFunctionTrampolineBase< R( A, B, C, D, E, F, G, H, I, J, K ) >::Args& args )
   {
      return R( fn( args.a(), args.b(), args.c(), args.d(), args.e(), args.f(), args.g(), args.h(), args.i(), args.j(), args.k() ) );
   }
};

template< typename T >
struct _EngineMethodTrampolineBase : public _EngineTrampoline< T > {};

template< typename Frame, typename T >
struct _EngineMethodTrampoline {};

template< typename Frame, typename R >
struct _EngineMethodTrampoline< Frame, R() > : public _EngineMethodTrampolineBase< R() >
{
   typedef R( FunctionType )( typename Frame::ObjectType* );
   static R jmp( typename Frame::ObjectType* object, const typename _EngineFunctionTrampolineBase< R() >::Args& args )
   {
      Frame f;
      f.object = object;
      return R( f._exec() );
   }
};
template< typename Frame, typename R, typename A >
struct _EngineMethodTrampoline< Frame, R( A ) > : public _EngineMethodTrampolineBase< R( A ) >
{
   typedef R( FunctionType )( typename Frame::ObjectType*, A );
   static R jmp( typename Frame::ObjectType* object, const typename _EngineFunctionTrampolineBase< R( A ) >::Args& args )
   {
      Frame f;
      f.object = object;
      return R( f._exec( args.a() ) );
   }
};
template< typename Frame, typename R, typename A, typename B >
struct _EngineMethodTrampoline< Frame, R( A, B ) > : public _EngineMethodTrampolineBase< R( A, B ) >
{
   typedef R( FunctionType )( typename Frame::ObjectType*, A, B );
   static R jmp( typename Frame::ObjectType* object, const typename _EngineFunctionTrampolineBase< R( A, B ) >::Args& args )
   {
      Frame f;
      f.object = object;
      return R( f._exec( args.a(), args.b() ) );
   }
};
template< typename Frame, typename R, typename A, typename B, typename C >
struct _EngineMethodTrampoline< Frame, R( A, B, C ) > : public _EngineMethodTrampolineBase< R( A, B, C ) >
{
   typedef R( FunctionType )( typename Frame::ObjectType*, A, B, C );
   static R jmp( typename Frame::ObjectType* object, const typename _EngineFunctionTrampolineBase< R( A, B, C ) >::Args& args )
   {
      Frame f;
      f.object = object;
      return R( f._exec( args.a(), args.b(), args.c() ) );
   }
};
template< typename Frame, typename R, typename A, typename B, typename C, typename D >
struct _EngineMethodTrampoline< Frame, R( A, B, C, D ) > : public _EngineMethodTrampolineBase< R( A, B, C, D ) >
{
   typedef R( FunctionType )( typename Frame::ObjectType*, A, B, C, D );
   static R jmp( typename Frame::ObjectType* object, const typename _EngineFunctionTrampolineBase< R( A, B, C, D ) >::Args& args )
   {
      Frame f;
      f.object = object;
      return R( f._exec( args.a(), args.b(), args.c(), args.d() ) );
   }
};
template< typename Frame, typename R, typename A, typename B, typename C, typename D, typename E >
struct _EngineMethodTrampoline< Frame, R( A, B, C, D, E ) > : public _EngineMethodTrampolineBase< R( A, B, C, D, E ) >
{
   typedef R( FunctionType )( typename Frame::ObjectType*, A, B, C, D, E );
   static R jmp( typename Frame::ObjectType* object, const typename _EngineFunctionTrampolineBase< R( A, B, C, D, E ) >::Args& args )
   {
      Frame f;
      f.object = object;
      return R( f._exec( args.a(), args.b(), args.c(), args.d(), args.e() ) );
   }
};
template< typename Frame, typename R, typename A, typename B, typename C, typename D, typename E, typename F >
struct _EngineMethodTrampoline< Frame, R( A, B, C, D, E, F ) > : public _EngineMethodTrampolineBase< R( A, B, C, D, E, F ) >
{
   typedef R( FunctionType )( typename Frame::ObjectType*, A, B, C, D, E, F );
   static R jmp( typename Frame::ObjectType* object, const typename _EngineFunctionTrampolineBase< R( A, B, C, D, E, F ) >::Args& args )
   {
      Frame f;
      f.object = object;
      return R( f._exec( args.a(), args.b(), args.c(), args.d(), args.e(), args.f() ) );
   }
};
template< typename Frame, typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G >
struct _EngineMethodTrampoline< Frame, R( A, B, C, D, E, F, G ) > : public _EngineMethodTrampolineBase< R( A, B, C, D, E, F, G ) >
{
   typedef R( FunctionType )( typename Frame::ObjectType*, A, B, C, D, E, F, G );
   static R jmp( typename Frame::ObjectType* object, const typename _EngineFunctionTrampolineBase< R( A, B, C, D, E, F, G ) >::Args& args )
   {
      Frame f;
      f.object = object;
      return R( f._exec( args.a(), args.b(), args.c(), args.d(), args.e(), args.f(), args.g() ) );
   }
};
template< typename Frame, typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H >
struct _EngineMethodTrampoline< Frame, R( A, B, C, D, E, F, G, H ) > : public _EngineMethodTrampolineBase< R( A, B, C, D, E, F, G, H ) >
{
   typedef R( FunctionType )( typename Frame::ObjectType*, A, B, C, D, E, F, G, H );
   static R jmp( typename Frame::ObjectType* object, const typename _EngineFunctionTrampolineBase< R( A, B, C, D, E, F, G, H ) >::Args& args )
   {
      Frame f;
      f.object = object;
      return R( f._exec( args.a(), args.b(), args.c(), args.d(), args.e(), args.f(), args.g(), args.h() ) );
   }
};
template< typename Frame, typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I >
struct _EngineMethodTrampoline< Frame, R( A, B, C, D, E, F, G, H, I ) > : public _EngineMethodTrampolineBase< R( A, B, C, D, E, F, G, H, I ) >
{
   typedef R( FunctionType )( typename Frame::ObjectType*, A, B, C, D, E, F, G, H, I );
   static R jmp( typename Frame::ObjectType* object, const typename _EngineFunctionTrampolineBase< R( A, B, C, D, E, F, G, H, I ) >::Args& args )
   {
      Frame f;
      f.object = object;
      return R( f._exec( args.a(), args.b(), args.c(), args.d(), args.e(), args.f(), args.g(), args.h(), args.i() ) );
   }
};
template< typename Frame, typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J >
struct _EngineMethodTrampoline< Frame, R( A, B, C, D, E, F, G, H, I, J ) > : public _EngineMethodTrampolineBase< R( A, B, C, D, E, F, G, H, I, J ) >
{
   typedef R( FunctionType )( typename Frame::ObjectType*, A, B, C, D, E, F, G, H, I, J );
   static R jmp( typename Frame::ObjectType* object, const typename _EngineFunctionTrampolineBase< R( A, B, C, D, E, F, G, H, I, J ) >::Args& args )
   {
      Frame f;
      f.object = object;
      return R( f._exec( args.a(), args.b(), args.c(), args.d(), args.e(), args.f(), args.g(), args.h(), args.i(), args.j() ) );
   }
};
template< typename Frame, typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K >
struct _EngineMethodTrampoline< Frame, R( A, B, C, D, E, F, G, H, I, J, K ) > : public _EngineMethodTrampolineBase< R( A, B, C, D, E, F, G, H, I, J, K ) >
{
   typedef R( FunctionType )( typename Frame::ObjectType*, A, B, C, D, E, F, G, H, I, J, K );
   static R jmp( typename Frame::ObjectType* object, const typename _EngineFunctionTrampolineBase< R( A, B, C, D, E, F, G, H, I, J, K ) >::Args& args )
   {
      Frame f;
      f.object = object;
      return R( f._exec( args.a(), args.b(), args.c(), args.d(), args.e(), args.f(), args.g(), args.h(), args.i(), args.j(), args.k() ) );
   }
};

/// @}


/// @name Thunking
///
/// Internal functionality for thunks placed between TorqueScript calls of engine functions and their native
/// implementations.
///
/// @note The functionality in this group is specific to the console interop system.
/// @{


// Helper function to return data from a thunk.
template< typename T >
inline const char* _EngineConsoleThunkReturnValue( const T& value )
{
   return EngineMarshallData( value );
}
inline bool _EngineConsoleThunkReturnValue( bool value )
{
   return value;
}
inline S32 _EngineConsoleThunkReturnValue( S32 value )
{
   return value;
}
inline F32 _EngineConsoleThunkReturnValue( F32 value )
{
   return value;
}
inline const char* _EngineConsoleThunkReturnValue( const String& str )
{
   return Con::getReturnBuffer( str );
}
inline const char* _EngineConsoleThunkReturnValue( const char* value )
{
   return EngineMarshallData( value );
}
template< typename T >
inline const char* _EngineConsoleThunkReturnValue( T* value )
{
   return ( value ? value->getIdString() : "" );
}
template< typename T >
inline const char* _EngineConsoleThunkReturnValue( const T* value )
{
   return ( value ? value->getIdString() : "" );
}



// Helper class to determine the type of callback registered with the console system.
template< typename R >
struct _EngineConsoleThunkType
{
   typedef const char* ReturnType;
   typedef StringCallback CallbackType;
};
template<>
struct _EngineConsoleThunkType< S32 >
{
   typedef S32 ReturnType;
   typedef IntCallback CallbackType;
};
template<>
struct _EngineConsoleThunkType< U32 >
{
   typedef U32 ReturnType;
   typedef IntCallback CallbackType;
};
template<>
struct _EngineConsoleThunkType< F32 >
{
   typedef F32 ReturnType;
   typedef FloatCallback CallbackType;
};
template<>
struct _EngineConsoleThunkType< bool >
{
   typedef bool ReturnType;
   typedef BoolCallback CallbackType;
};
template<>
struct _EngineConsoleThunkType< void >
{
   typedef void ReturnType;
   typedef VoidCallback CallbackType;
};


// Helper struct to count the number of parameters in a function list.
// The setup through operator () allows omitting the the argument list entirely.
struct _EngineConsoleThunkCountArgs
{
   template< typename A >
   U32 operator ()( A a )
   {
      return 1;
   }
   template< typename A, typename B >
   U32 operator ()( A a, B b )
   {
      return 2;
   }
   template< typename A, typename B, typename C >
   U32 operator ()( A a, B b, C c )
   {
      return 3;
   }
   template< typename A, typename B, typename C, typename D >
   U32 operator ()( A a, B b, C c, D d )
   {
      return 4;
   }
   template< typename A, typename B, typename C, typename D, typename E >
   U32 operator ()( A a, B b, C c, D d, E e )
   {
      return 5;
   }
   template< typename A, typename B, typename C, typename D, typename E, typename F >
   U32 operator ()( A a, B b, C c, D d, E e, F f )
   {
      return 6;
   }
   template< typename A, typename B, typename C, typename D, typename E, typename F, typename G >
   U32 operator ()( A a, B b, C c, D d, E e, F f, G g )
   {
      return 7;
   }
   template< typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H >
   U32 operator ()( A a, B b, C c, D d, E e, F f, G g, H h )
   {
      return 8;
   }
   template< typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I >
   U32 operator ()( A a, B b, C c, D d, E e, F f, G g, H h, I i )
   {
      return 9;
   }
   template< typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J >
   U32 operator ()( A a, B b, C c, D d, E e, F f, G g, H h, I i, J j )
   {
      return 10;
   }
   template< typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K >
   U32 operator ()( A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k )
   {
      return 11;
   }
   
   operator U32() const
   {
      return 0;
   }
};


// Encapsulation of a legacy console function invocation.

template< int startArgc, typename T >
struct _EngineConsoleThunk {};

template< int startArgc, typename R >
struct _EngineConsoleThunk< startArgc, R() >
{
   typedef typename _EngineConsoleThunkType< R >::ReturnType ReturnType;
   static const int NUM_ARGS = 0;
   static ReturnType thunk( S32 argc, const char** argv, R ( *fn )(), const _EngineFunctionDefaultArguments< void() >& )
   {
      return _EngineConsoleThunkReturnValue( fn() );
   }
   template< typename Frame >
   static ReturnType thunk( S32 argc, const char** argv, R ( Frame::*fn )() const, Frame* frame, const _EngineFunctionDefaultArguments< void( typename Frame::ObjectType* ) >& )
   {
      return _EngineConsoleThunkReturnValue( ( frame->*fn )() );
   }
};
template< int startArgc >
struct _EngineConsoleThunk< startArgc, void() >
{
   typedef void ReturnType;
   static const int NUM_ARGS = 0;
   static void thunk( S32 argc, const char** argv, void ( *fn )(), const _EngineFunctionDefaultArguments< void() >& )
   {
      fn();
   }
   template< typename Frame >
   static void thunk( S32 argc, const char** argv, void ( Frame::*fn )() const, Frame* frame, const _EngineFunctionDefaultArguments< void( typename Frame::ObjectType* ) >& )
   {
      return ( frame->*fn )();
   }
};

template< int startArgc, typename R, typename A >
struct _EngineConsoleThunk< startArgc, R( A ) >
{
   typedef typename _EngineConsoleThunkType< R >::ReturnType ReturnType;
   static const int NUM_ARGS = 1 + startArgc;
   static ReturnType thunk( S32 argc, const char** argv, R ( *fn )( A ), const _EngineFunctionDefaultArguments< void( A ) >& defaultArgs )
   {
      A a = ( startArgc < argc ? EngineUnmarshallData< A >()( argv[ startArgc ] ) : A( defaultArgs.a ) );
      return _EngineConsoleThunkReturnValue( fn( a ) );
   }
   template< typename Frame >
   static ReturnType thunk( S32 argc, const char** argv, R ( Frame::*fn )( A ) const, Frame* frame, const _EngineFunctionDefaultArguments< void( typename Frame::ObjectType*, A ) >& defaultArgs )
   {
      A a = ( startArgc < argc ? EngineUnmarshallData< A >()( argv[ startArgc ] ) : A( defaultArgs.b ) );
      return _EngineConsoleThunkReturnValue( ( frame->*fn )( a ) );
   }
};
template< int startArgc, typename A >
struct _EngineConsoleThunk< startArgc, void( A ) >
{
   typedef void ReturnType;
   static const int NUM_ARGS = 1 + startArgc;
   static void thunk( S32 argc, const char** argv, void ( *fn )( A ), const _EngineFunctionDefaultArguments< void( A ) >& defaultArgs )
   {
      A a = ( startArgc < argc ? EngineUnmarshallData< A >()( argv[ startArgc ] ) : A( defaultArgs.a ) );
      fn( a );
   }
   template< typename Frame >
   static void thunk( S32 argc, const char** argv, void ( Frame::*fn )( A ) const, Frame* frame, const _EngineFunctionDefaultArguments< void( typename Frame::ObjectType*, A ) >& defaultArgs )
   {
      A a = ( startArgc < argc ? EngineUnmarshallData< A >()( argv[ startArgc ] ) : A( defaultArgs.b ) );
      ( frame->*fn )( a );
   }
};

template< int startArgc, typename R, typename A, typename B >
struct _EngineConsoleThunk< startArgc, R( A, B ) >
{
   typedef typename _EngineConsoleThunkType< R >::ReturnType ReturnType;
   static const int NUM_ARGS = 2 + startArgc;
   static ReturnType thunk( S32 argc, const char** argv, R ( *fn )( A, B ), const _EngineFunctionDefaultArguments< void( A, B ) >& defaultArgs )
   {
      A a = ( startArgc < argc ? EngineUnmarshallData< A >()( argv[ startArgc ] ) : A( defaultArgs.a ) );
      B b = ( startArgc + 1 < argc ? EngineUnmarshallData< B >()( argv[ startArgc + 1 ] ) : B( defaultArgs.b ) );
      return _EngineConsoleThunkReturnValue( fn( a, b ) );
   }
   template< typename Frame >
   static ReturnType thunk( S32 argc, const char** argv, R ( Frame::*fn )( A, B ) const, Frame* frame, const _EngineFunctionDefaultArguments< void( typename Frame::ObjectType*, A, B ) >& defaultArgs )
   {
      A a = ( startArgc < argc ? EngineUnmarshallData< A >()( argv[ startArgc ] ) : A( defaultArgs.b ) );
      B b = ( startArgc + 1 < argc ? EngineUnmarshallData< B >()( argv[ startArgc + 1 ] ) : B( defaultArgs.c ) );
      return _EngineConsoleThunkReturnValue( ( frame->*fn )( a, b ) );
   }
};
template< int startArgc, typename A, typename B >
struct _EngineConsoleThunk< startArgc, void( A, B ) >
{
   typedef void ReturnType;
   static const int NUM_ARGS = 2 + startArgc;
   static void thunk( S32 argc, const char** argv, void ( *fn )( A, B ), const _EngineFunctionDefaultArguments< void( A, B ) >& defaultArgs )
   {
      A a = ( startArgc < argc ? EngineUnmarshallData< A >()( argv[ startArgc ] ) : A( defaultArgs.a ) );
      B b = ( startArgc + 1 < argc ? EngineUnmarshallData< B >()( argv[ startArgc + 1 ] ) : B( defaultArgs.b ) );
      fn( a, b );
   }
   template< typename Frame >
   static void thunk( S32 argc, const char** argv, void ( Frame::*fn )( A, B ) const, Frame* frame, const _EngineFunctionDefaultArguments< void( typename Frame::ObjectType*, A, B ) >& defaultArgs )
   {
      A a = ( startArgc < argc ? EngineUnmarshallData< A >()( argv[ startArgc ] ) : A( defaultArgs.b ) );
      B b = ( startArgc + 1 < argc ? EngineUnmarshallData< B >()( argv[ startArgc + 1 ] ) : B( defaultArgs.c ) );
      ( frame->*fn )( a, b );
   }
};

template< int startArgc, typename R, typename A, typename B, typename C >
struct _EngineConsoleThunk< startArgc, R( A, B, C ) >
{
   typedef typename _EngineConsoleThunkType< R >::ReturnType ReturnType;
   static const int NUM_ARGS = 3 + startArgc;
   static ReturnType thunk( S32 argc, const char** argv, R ( *fn )( A, B, C ), const _EngineFunctionDefaultArguments< void( A, B, C ) >& defaultArgs )
   {
      A a = ( startArgc < argc ? EngineUnmarshallData< A >()( argv[ startArgc ] ) : A( defaultArgs.a ) );
      B b = ( startArgc + 1 < argc ? EngineUnmarshallData< B >()( argv[ startArgc + 1 ] ) : B( defaultArgs.b ) );
      C c = ( startArgc + 2 < argc ? EngineUnmarshallData< C >()( argv[ startArgc + 2 ] ) : C( defaultArgs.c ) );
      return _EngineConsoleThunkReturnValue( fn( a, b, c ) );
   }
   template< typename Frame >
   static ReturnType thunk( S32 argc, const char** argv, R ( Frame::*fn )( A, B, C ) const, Frame* frame, const _EngineFunctionDefaultArguments< void( typename Frame::ObjectType*, A, B, C ) >& defaultArgs )
   {
      A a = ( startArgc < argc ? EngineUnmarshallData< A >()( argv[ startArgc ] ) : A( defaultArgs.b ) );
      B b = ( startArgc + 1 < argc ? EngineUnmarshallData< B >()( argv[ startArgc + 1 ] ) : B( defaultArgs.c ) );
      C c = ( startArgc + 2 < argc ? EngineUnmarshallData< C >()( argv[ startArgc + 2 ] ) : C( defaultArgs.d ) );
      return _EngineConsoleThunkReturnValue( ( frame->*fn )( a, b, c ) );
   }
};
template< int startArgc, typename A, typename B, typename C >
struct _EngineConsoleThunk< startArgc, void( A, B, C ) >
{
   typedef void ReturnType;
   static const int NUM_ARGS = 3 + startArgc;
   static void thunk( S32 argc, const char** argv, void ( *fn )( A, B, C ), const _EngineFunctionDefaultArguments< void( A, B, C ) >& defaultArgs )
   {
      A a = ( startArgc < argc ? EngineUnmarshallData< A >()( argv[ startArgc ] ) : A( defaultArgs.a ) );
      B b = ( startArgc + 1 < argc ? EngineUnmarshallData< B >()( argv[ startArgc + 1 ] ) : B( defaultArgs.b ) );
      C c = ( startArgc + 2 < argc ? EngineUnmarshallData< C >()( argv[ startArgc + 2 ] ) : C( defaultArgs.c ) );
      fn( a, b, c );
   }
   template< typename Frame >
   static void thunk( S32 argc, const char** argv, void ( Frame::*fn )( A, B, C ) const, Frame* frame, const _EngineFunctionDefaultArguments< void( typename Frame::ObjectType*, A, B, C ) >& defaultArgs )
   {
      A a = ( startArgc < argc ? EngineUnmarshallData< A >()( argv[ startArgc ] ) : A( defaultArgs.b ) );
      B b = ( startArgc + 1 < argc ? EngineUnmarshallData< B >()( argv[ startArgc + 1 ] ) : B( defaultArgs.c ) );
      C c = ( startArgc + 2 < argc ? EngineUnmarshallData< C >()( argv[ startArgc + 2 ] ) : C( defaultArgs.d ) );
      ( frame->*fn )( a, b, c );
   }
};

template< int startArgc, typename R, typename A, typename B, typename C, typename D >
struct _EngineConsoleThunk< startArgc, R( A, B, C, D ) >
{
   typedef typename _EngineConsoleThunkType< R >::ReturnType ReturnType;
   static const int NUM_ARGS = 4 + startArgc;
   static ReturnType thunk( S32 argc, const char** argv, R ( *fn )( A, B, C, D ), const _EngineFunctionDefaultArguments< void( A, B, C, D ) >& defaultArgs )
   {
      A a = ( startArgc < argc ? EngineUnmarshallData< A >()( argv[ startArgc ] ) : A( defaultArgs.a ) );
      B b = ( startArgc + 1 < argc ? EngineUnmarshallData< B >()( argv[ startArgc + 1 ] ) : B( defaultArgs.b ) );
      C c = ( startArgc + 2 < argc ? EngineUnmarshallData< C >()( argv[ startArgc + 2 ] ) : C( defaultArgs.c ) );
      D d = ( startArgc + 3 < argc ? EngineUnmarshallData< D >()( argv[ startArgc + 3 ] ) : D( defaultArgs.d ) );
      return _EngineConsoleThunkReturnValue( fn( a, b, c, d ) );
   }
   template< typename Frame >
   static ReturnType thunk( S32 argc, const char** argv, R ( Frame::*fn )( A, B, C, D ) const, Frame* frame, const _EngineFunctionDefaultArguments< void( typename Frame::ObjectType*, A, B, C, D ) >& defaultArgs )
   {
      A a = ( startArgc < argc ? EngineUnmarshallData< A >()( argv[ startArgc ] ) : A( defaultArgs.b ) );
      B b = ( startArgc + 1 < argc ? EngineUnmarshallData< B >()( argv[ startArgc + 1 ] ) : B( defaultArgs.c ) );
      C c = ( startArgc + 2 < argc ? EngineUnmarshallData< C >()( argv[ startArgc + 2 ] ) : C( defaultArgs.d ) );
      D d = ( startArgc + 3 < argc ? EngineUnmarshallData< D >()( argv[ startArgc + 3 ] ) : D( defaultArgs.e ) );
      return _EngineConsoleThunkReturnValue( ( frame->*fn )( a, b, c, d ) );
   }
};
template< int startArgc, typename A, typename B, typename C, typename D >
struct _EngineConsoleThunk< startArgc, void( A, B, C, D ) >
{
   typedef void ReturnType;
   static const int NUM_ARGS = 4 + startArgc;
   static void thunk( S32 argc, const char** argv, void ( *fn )( A, B, C, D ), const _EngineFunctionDefaultArguments< void( A, B, C, D ) >& defaultArgs )
   {
      A a = ( startArgc < argc ? EngineUnmarshallData< A >()( argv[ startArgc ] ) : A( defaultArgs.a ) );
      B b = ( startArgc + 1 < argc ? EngineUnmarshallData< B >()( argv[ startArgc + 1 ] ) : B( defaultArgs.b ) );
      C c = ( startArgc + 2 < argc ? EngineUnmarshallData< C >()( argv[ startArgc + 2 ] ) : C( defaultArgs.c ) );
      D d = ( startArgc + 3 < argc ? EngineUnmarshallData< D >()( argv[ startArgc + 3 ] ) : D( defaultArgs.d ) );
      fn( a, b, c, d );
   }
   template< typename Frame >
   static void thunk( S32 argc, const char** argv, void ( Frame::*fn )( A, B, C, D ) const, Frame* frame, const _EngineFunctionDefaultArguments< void( typename Frame::ObjectType*, A, B, C, D ) >& defaultArgs )
   {
      A a = ( startArgc < argc ? EngineUnmarshallData< A >()( argv[ startArgc ] ) : A( defaultArgs.b ) );
      B b = ( startArgc + 1 < argc ? EngineUnmarshallData< B >()( argv[ startArgc + 1 ] ) : B( defaultArgs.c ) );
      C c = ( startArgc + 2 < argc ? EngineUnmarshallData< C >()( argv[ startArgc + 2 ] ) : C( defaultArgs.d ) );
      D d = ( startArgc + 3 < argc ? EngineUnmarshallData< D >()( argv[ startArgc + 3 ] ) : D( defaultArgs.e ) );
      ( frame->*fn )( a, b, c, d );
   }
};

template< int startArgc, typename R, typename A, typename B, typename C, typename D, typename E >
struct _EngineConsoleThunk< startArgc, R( A, B, C, D, E ) >
{
   typedef typename _EngineConsoleThunkType< R >::ReturnType ReturnType;
   static const int NUM_ARGS = 5 + startArgc;
   static ReturnType thunk( S32 argc, const char** argv, R ( *fn )( A, B, C, D, E ), const _EngineFunctionDefaultArguments< void( A, B, C, D, E ) >& defaultArgs )
   {
      A a = ( startArgc < argc ? EngineUnmarshallData< A >()( argv[ startArgc ] ) : A( defaultArgs.a ) );
      B b = ( startArgc + 1 < argc ? EngineUnmarshallData< B >()( argv[ startArgc + 1 ] ) : B( defaultArgs.b ) );
      C c = ( startArgc + 2 < argc ? EngineUnmarshallData< C >()( argv[ startArgc + 2 ] ) : C( defaultArgs.c ) );
      D d = ( startArgc + 3 < argc ? EngineUnmarshallData< D >()( argv[ startArgc + 3 ] ) : D( defaultArgs.d ) );
      E e = ( startArgc + 4 < argc ? EngineUnmarshallData< E >()( argv[ startArgc + 4 ] ) : E( defaultArgs.e ) );
      return _EngineConsoleThunkReturnValue( fn( a, b, c, d, e ) );
   }
   template< typename Frame >
   static ReturnType thunk( S32 argc, const char** argv, R ( Frame::*fn )( A, B, C, D, E ) const, Frame* frame, const _EngineFunctionDefaultArguments< void( typename Frame::ObjectType*, A, B, C, D, E ) >& defaultArgs )
   {
      A a = ( startArgc < argc ? EngineUnmarshallData< A >()( argv[ startArgc ] ) : A( defaultArgs.b ) );
      B b = ( startArgc + 1 < argc ? EngineUnmarshallData< B >()( argv[ startArgc + 1 ] ) : B( defaultArgs.c ) );
      C c = ( startArgc + 2 < argc ? EngineUnmarshallData< C >()( argv[ startArgc + 2 ] ) : C( defaultArgs.d ) );
      D d = ( startArgc + 3 < argc ? EngineUnmarshallData< D >()( argv[ startArgc + 3 ] ) : D( defaultArgs.e ) );
      E e = ( startArgc + 4 < argc ? EngineUnmarshallData< E >()( argv[ startArgc + 4 ] ) : E( defaultArgs.f ) );
      return _EngineConsoleThunkReturnValue( ( frame->*fn )( a, b, c, d, e ) );
   }
};
template< int startArgc, typename A, typename B, typename C, typename D, typename E >
struct _EngineConsoleThunk< startArgc, void( A, B, C, D, E ) >
{
   typedef void ReturnType;
   static const int NUM_ARGS = 5 + startArgc;
   static void thunk( S32 argc, const char** argv, void ( *fn )( A, B, C, D, E ), const _EngineFunctionDefaultArguments< void( A, B, C, D, E ) >& defaultArgs )
   {
      A a = ( startArgc < argc ? EngineUnmarshallData< A >()( argv[ startArgc ] ) : A( defaultArgs.a ) );
      B b = ( startArgc + 1 < argc ? EngineUnmarshallData< B >()( argv[ startArgc + 1 ] ) : B( defaultArgs.b ) );
      C c = ( startArgc + 2 < argc ? EngineUnmarshallData< C >()( argv[ startArgc + 2 ] ) : C( defaultArgs.c ) );
      D d = ( startArgc + 3 < argc ? EngineUnmarshallData< D >()( argv[ startArgc + 3 ] ) : D( defaultArgs.d ) );
      E e = ( startArgc + 4 < argc ? EngineUnmarshallData< E >()( argv[ startArgc + 4 ] ) : E( defaultArgs.e ) );
      fn( a, b, c, d, e );
   }
   template< typename Frame >
   static void thunk( S32 argc, const char** argv, void ( Frame::*fn )( A, B, C, D, E ) const, Frame* frame, const _EngineFunctionDefaultArguments< void( typename Frame::ObjectType*, A, B, C, D, E ) >& defaultArgs )
   {
      A a = ( startArgc < argc ? EngineUnmarshallData< A >()( argv[ startArgc ] ) : A( defaultArgs.b ) );
      B b = ( startArgc + 1 < argc ? EngineUnmarshallData< B >()( argv[ startArgc + 1 ] ) : B( defaultArgs.c ) );
      C c = ( startArgc + 2 < argc ? EngineUnmarshallData< C >()( argv[ startArgc + 2 ] ) : C( defaultArgs.d ) );
      D d = ( startArgc + 3 < argc ? EngineUnmarshallData< D >()( argv[ startArgc + 3 ] ) : D( defaultArgs.e ) );
      E e = ( startArgc + 4 < argc ? EngineUnmarshallData< E >()( argv[ startArgc + 4 ] ) : E( defaultArgs.f ) );
      ( frame->*fn )( a, b, c, d, e );
   }
};

template< int startArgc, typename R, typename A, typename B, typename C, typename D, typename E, typename F >
struct _EngineConsoleThunk< startArgc, R( A, B, C, D, E, F ) >
{
   typedef typename _EngineConsoleThunkType< R >::ReturnType ReturnType;
   static const int NUM_ARGS = 6 + startArgc;
   static ReturnType thunk( S32 argc, const char** argv, R ( *fn )( A, B, C, D, E, F ), const _EngineFunctionDefaultArguments< void( A, B, C, D, E, F ) >& defaultArgs )
   {
      A a = ( startArgc < argc ? EngineUnmarshallData< A >()( argv[ startArgc ] ) : A( defaultArgs.a ) );
      B b = ( startArgc + 1 < argc ? EngineUnmarshallData< B >()( argv[ startArgc + 1 ] ) : B( defaultArgs.b ) );
      C c = ( startArgc + 2 < argc ? EngineUnmarshallData< C >()( argv[ startArgc + 2 ] ) : C( defaultArgs.c ) );
      D d = ( startArgc + 3 < argc ? EngineUnmarshallData< D >()( argv[ startArgc + 3 ] ) : D( defaultArgs.d ) );
      E e = ( startArgc + 4 < argc ? EngineUnmarshallData< E >()( argv[ startArgc + 4 ] ) : E( defaultArgs.e ) );
      F f = ( startArgc + 5 < argc ? EngineUnmarshallData< F >()( argv[ startArgc + 5 ] ) : F( defaultArgs.f ) );
      return _EngineConsoleThunkReturnValue( fn( a, b, c, d, e, f ) );
   }
   template< typename Frame >
   static ReturnType thunk( S32 argc, const char** argv, R ( Frame::*fn )( A, B, C, D, E, F ) const, Frame* frame, const _EngineFunctionDefaultArguments< void( typename Frame::ObjectType*, A, B, C, D, E, F ) >& defaultArgs )
   {
      A a = ( startArgc < argc ? EngineUnmarshallData< A >()( argv[ startArgc ] ) : A( defaultArgs.b ) );
      B b = ( startArgc + 1 < argc ? EngineUnmarshallData< B >()( argv[ startArgc + 1 ] ) : B( defaultArgs.c ) );
      C c = ( startArgc + 2 < argc ? EngineUnmarshallData< C >()( argv[ startArgc + 2 ] ) : C( defaultArgs.d ) );
      D d = ( startArgc + 3 < argc ? EngineUnmarshallData< D >()( argv[ startArgc + 3 ] ) : D( defaultArgs.e ) );
      E e = ( startArgc + 4 < argc ? EngineUnmarshallData< E >()( argv[ startArgc + 4 ] ) : E( defaultArgs.f ) );
      F f = ( startArgc + 5 < argc ? EngineUnmarshallData< F >()( argv[ startArgc + 5 ] ) : F( defaultArgs.g ) );
      return _EngineConsoleThunkReturnValue( ( frame->*fn )( a, b, c, d, e, f ) );
   }
};
template< int startArgc, typename A, typename B, typename C, typename D, typename E, typename F >
struct _EngineConsoleThunk< startArgc, void( A, B, C, D, E, F ) >
{
   typedef void ReturnType;
   static const int NUM_ARGS = 6 + startArgc;
   static void thunk( S32 argc, const char** argv, void ( *fn )( A, B, C, D, E, F ), const _EngineFunctionDefaultArguments< void( A, B, C, D, E, F ) >& defaultArgs )
   {
      A a = ( startArgc < argc ? EngineUnmarshallData< A >()( argv[ startArgc ] ) : A( defaultArgs.a ) );
      B b = ( startArgc + 1 < argc ? EngineUnmarshallData< B >()( argv[ startArgc + 1 ] ) : B( defaultArgs.b ) );
      C c = ( startArgc + 2 < argc ? EngineUnmarshallData< C >()( argv[ startArgc + 2 ] ) : C( defaultArgs.c ) );
      D d = ( startArgc + 3 < argc ? EngineUnmarshallData< D >()( argv[ startArgc + 3 ] ) : D( defaultArgs.d ) );
      E e = ( startArgc + 4 < argc ? EngineUnmarshallData< E >()( argv[ startArgc + 4 ] ) : E( defaultArgs.e ) );
      F f = ( startArgc + 5 < argc ? EngineUnmarshallData< F >()( argv[ startArgc + 5 ] ) : F( defaultArgs.f ) );
      fn( a, b, c, d, e, f );
   }
   template< typename Frame >
   static void thunk( S32 argc, const char** argv, void ( Frame::*fn )( A, B, C, D, E, F ) const, Frame* frame, const _EngineFunctionDefaultArguments< void( typename Frame::ObjectType*, A, B, C, D, E, F ) >& defaultArgs )
   {
      A a = ( startArgc < argc ? EngineUnmarshallData< A >()( argv[ startArgc ] ) : A( defaultArgs.b ) );
      B b = ( startArgc + 1 < argc ? EngineUnmarshallData< B >()( argv[ startArgc + 1 ] ) : B( defaultArgs.c ) );
      C c = ( startArgc + 2 < argc ? EngineUnmarshallData< C >()( argv[ startArgc + 2 ] ) : C( defaultArgs.d ) );
      D d = ( startArgc + 3 < argc ? EngineUnmarshallData< D >()( argv[ startArgc + 3 ] ) : D( defaultArgs.e ) );
      E e = ( startArgc + 4 < argc ? EngineUnmarshallData< E >()( argv[ startArgc + 4 ] ) : E( defaultArgs.f ) );
      F f = ( startArgc + 5 < argc ? EngineUnmarshallData< F >()( argv[ startArgc + 5 ] ) : F( defaultArgs.g ) );
      ( frame->*fn )( a, b, c, d, e, f );
   }
};

template< int startArgc, typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G >
struct _EngineConsoleThunk< startArgc, R( A, B, C, D, E, F, G ) >
{
   typedef typename _EngineConsoleThunkType< R >::ReturnType ReturnType;
   static const int NUM_ARGS = 7 + startArgc;
   static ReturnType thunk( S32 argc, const char** argv, R ( *fn )( A, B, C, D, E, F, G ), const _EngineFunctionDefaultArguments< void( A, B, C, D, E, F, G ) >& defaultArgs )
   {
      A a = ( startArgc < argc ? EngineUnmarshallData< A >()( argv[ startArgc ] ) : A( defaultArgs.a ) );
      B b = ( startArgc + 1 < argc ? EngineUnmarshallData< B >()( argv[ startArgc + 1 ] ) : B( defaultArgs.b ) );
      C c = ( startArgc + 2 < argc ? EngineUnmarshallData< C >()( argv[ startArgc + 2 ] ) : C( defaultArgs.c ) );
      D d = ( startArgc + 3 < argc ? EngineUnmarshallData< D >()( argv[ startArgc + 3 ] ) : D( defaultArgs.d ) );
      E e = ( startArgc + 4 < argc ? EngineUnmarshallData< E >()( argv[ startArgc + 4 ] ) : E( defaultArgs.e ) );
      F f = ( startArgc + 5 < argc ? EngineUnmarshallData< F >()( argv[ startArgc + 5 ] ) : F( defaultArgs.f ) );
      G g = ( startArgc + 6 < argc ? EngineUnmarshallData< G >()( argv[ startArgc + 6 ] ) : G( defaultArgs.g ) );
      return _EngineConsoleThunkReturnValue( fn( a, b, c, d, e, f, g ) );
   }
   template< typename Frame >
   static ReturnType thunk( S32 argc, const char** argv, R ( Frame::*fn )( A, B, C, D, E, F, G ) const, Frame* frame, const _EngineFunctionDefaultArguments< void( typename Frame::ObjectType*, A, B, C, D, E, F, G ) >& defaultArgs )
   {
      A a = ( startArgc < argc ? EngineUnmarshallData< A >()( argv[ startArgc ] ) : A( defaultArgs.b ) );
      B b = ( startArgc + 1 < argc ? EngineUnmarshallData< B >()( argv[ startArgc + 1 ] ) : B( defaultArgs.c ) );
      C c = ( startArgc + 2 < argc ? EngineUnmarshallData< C >()( argv[ startArgc + 2 ] ) : C( defaultArgs.d ) );
      D d = ( startArgc + 3 < argc ? EngineUnmarshallData< D >()( argv[ startArgc + 3 ] ) : D( defaultArgs.e ) );
      E e = ( startArgc + 4 < argc ? EngineUnmarshallData< E >()( argv[ startArgc + 4 ] ) : E( defaultArgs.f ) );
      F f = ( startArgc + 5 < argc ? EngineUnmarshallData< F >()( argv[ startArgc + 5 ] ) : F( defaultArgs.g ) );
      G g = ( startArgc + 6 < argc ? EngineUnmarshallData< G >()( argv[ startArgc + 6 ] ) : G( defaultArgs.h ) );
      return _EngineConsoleThunkReturnValue( ( frame->*fn )( a, b, c, d, e, f, g ) );
   }
};
template< int startArgc, typename A, typename B, typename C, typename D, typename E, typename F, typename G >
struct _EngineConsoleThunk< startArgc, void( A, B, C, D, E, F, G ) >
{
   typedef void ReturnType;
   static const int NUM_ARGS = 7 + startArgc;
   static void thunk( S32 argc, const char** argv, void ( *fn )( A, B, C, D, E, F, G ), const _EngineFunctionDefaultArguments< void( A, B, C, D, E, F, G ) >& defaultArgs )
   {
      A a = ( startArgc < argc ? EngineUnmarshallData< A >()( argv[ startArgc ] ) : A( defaultArgs.a ) );
      B b = ( startArgc + 1 < argc ? EngineUnmarshallData< B >()( argv[ startArgc + 1 ] ) : B( defaultArgs.b ) );
      C c = ( startArgc + 2 < argc ? EngineUnmarshallData< C >()( argv[ startArgc + 2 ] ) : C( defaultArgs.c ) );
      D d = ( startArgc + 3 < argc ? EngineUnmarshallData< D >()( argv[ startArgc + 3 ] ) : D( defaultArgs.d ) );
      E e = ( startArgc + 4 < argc ? EngineUnmarshallData< E >()( argv[ startArgc + 4 ] ) : E( defaultArgs.e ) );
      F f = ( startArgc + 5 < argc ? EngineUnmarshallData< F >()( argv[ startArgc + 5 ] ) : F( defaultArgs.f ) );
      G g = ( startArgc + 6 < argc ? EngineUnmarshallData< G >()( argv[ startArgc + 6 ] ) : G( defaultArgs.g ) );
      fn( a, b, c, d, e, f, g );
   }
   template< typename Frame >
   static void thunk( S32 argc, const char** argv, void ( Frame::*fn )( A, B, C, D, E, F, G ) const, Frame* frame, const _EngineFunctionDefaultArguments< void( typename Frame::ObjectType*, A, B, C, D, E, F, G ) >& defaultArgs )
   {
      A a = ( startArgc < argc ? EngineUnmarshallData< A >()( argv[ startArgc ] ) : A( defaultArgs.b ) );
      B b = ( startArgc + 1 < argc ? EngineUnmarshallData< B >()( argv[ startArgc + 1 ] ) : B( defaultArgs.c ) );
      C c = ( startArgc + 2 < argc ? EngineUnmarshallData< C >()( argv[ startArgc + 2 ] ) : C( defaultArgs.d ) );
      D d = ( startArgc + 3 < argc ? EngineUnmarshallData< D >()( argv[ startArgc + 3 ] ) : D( defaultArgs.e ) );
      E e = ( startArgc + 4 < argc ? EngineUnmarshallData< E >()( argv[ startArgc + 4 ] ) : E( defaultArgs.f ) );
      F f = ( startArgc + 5 < argc ? EngineUnmarshallData< F >()( argv[ startArgc + 5 ] ) : F( defaultArgs.g ) );
      G g = ( startArgc + 6 < argc ? EngineUnmarshallData< G >()( argv[ startArgc + 6 ] ) : G( defaultArgs.h ) );
      ( frame->*fn )( a, b, c, d, e, f, g );
   }
};

template< int startArgc, typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H >
struct _EngineConsoleThunk< startArgc, R( A, B, C, D, E, F, G, H ) >
{
   typedef typename _EngineConsoleThunkType< R >::ReturnType ReturnType;
   static const int NUM_ARGS = 8 + startArgc;
   static ReturnType thunk( S32 argc, const char** argv, R ( *fn )( A, B, C, D, E, F, G, H ), const _EngineFunctionDefaultArguments< void( A, B, C, D, E, F, G, H ) >& defaultArgs )
   {
      A a = ( startArgc < argc ? EngineUnmarshallData< A >()( argv[ startArgc ] ) : A( defaultArgs.a ) );
      B b = ( startArgc + 1 < argc ? EngineUnmarshallData< B >()( argv[ startArgc + 1 ] ) : B( defaultArgs.b ) );
      C c = ( startArgc + 2 < argc ? EngineUnmarshallData< C >()( argv[ startArgc + 2 ] ) : C( defaultArgs.c ) );
      D d = ( startArgc + 3 < argc ? EngineUnmarshallData< D >()( argv[ startArgc + 3 ] ) : D( defaultArgs.d ) );
      E e = ( startArgc + 4 < argc ? EngineUnmarshallData< E >()( argv[ startArgc + 4 ] ) : E( defaultArgs.e ) );
      F f = ( startArgc + 5 < argc ? EngineUnmarshallData< F >()( argv[ startArgc + 5 ] ) : F( defaultArgs.f ) );
      G g = ( startArgc + 6 < argc ? EngineUnmarshallData< G >()( argv[ startArgc + 6 ] ) : G( defaultArgs.g ) );
      H h = ( startArgc + 7 < argc ? EngineUnmarshallData< H >()( argv[ startArgc + 7 ] ) : H( defaultArgs.h ) );
      return _EngineConsoleThunkReturnValue( fn( a, b, c, d, e, f, g, h ) );
   }
   template< typename Frame >
   static ReturnType thunk( S32 argc, const char** argv, R ( Frame::*fn )( A, B, C, D, E, F, G, H ) const, Frame* frame, const _EngineFunctionDefaultArguments< void( typename Frame::ObjectType*, A, B, C, D, E, F, G, H ) >& defaultArgs )
   {
      A a = ( startArgc < argc ? EngineUnmarshallData< A >()( argv[ startArgc ] ) : A( defaultArgs.b ) );
      B b = ( startArgc + 1 < argc ? EngineUnmarshallData< B >()( argv[ startArgc + 1 ] ) : B( defaultArgs.c ) );
      C c = ( startArgc + 2 < argc ? EngineUnmarshallData< C >()( argv[ startArgc + 2 ] ) : C( defaultArgs.d ) );
      D d = ( startArgc + 3 < argc ? EngineUnmarshallData< D >()( argv[ startArgc + 3 ] ) : D( defaultArgs.e ) );
      E e = ( startArgc + 4 < argc ? EngineUnmarshallData< E >()( argv[ startArgc + 4 ] ) : E( defaultArgs.f ) );
      F f = ( startArgc + 5 < argc ? EngineUnmarshallData< F >()( argv[ startArgc + 5 ] ) : F( defaultArgs.g ) );
      G g = ( startArgc + 6 < argc ? EngineUnmarshallData< G >()( argv[ startArgc + 6 ] ) : G( defaultArgs.h ) );
      H h = ( startArgc + 7 < argc ? EngineUnmarshallData< H >()( argv[ startArgc + 7 ] ) : H( defaultArgs.i ) );
      return _EngineConsoleThunkReturnValue( ( frame->*fn )( a, b, c, d, e, f, g, h ) );
   }
};
template< int startArgc, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H >
struct _EngineConsoleThunk< startArgc, void( A, B, C, D, E, F, G, H ) >
{
   typedef void ReturnType;
   static const int NUM_ARGS = 8 + startArgc;
   static void thunk( S32 argc, const char** argv, void ( *fn )( A, B, C, D, E, F, G, H ), const _EngineFunctionDefaultArguments< void( A, B, C, D, E, F, G, H ) >& defaultArgs )
   {
      A a = ( startArgc < argc ? EngineUnmarshallData< A >()( argv[ startArgc ] ) : A( defaultArgs.a ) );
      B b = ( startArgc + 1 < argc ? EngineUnmarshallData< B >()( argv[ startArgc + 1 ] ) : B( defaultArgs.b ) );
      C c = ( startArgc + 2 < argc ? EngineUnmarshallData< C >()( argv[ startArgc + 2 ] ) : C( defaultArgs.c ) );
      D d = ( startArgc + 3 < argc ? EngineUnmarshallData< D >()( argv[ startArgc + 3 ] ) : D( defaultArgs.d ) );
      E e = ( startArgc + 4 < argc ? EngineUnmarshallData< E >()( argv[ startArgc + 4 ] ) : E( defaultArgs.e ) );
      F f = ( startArgc + 5 < argc ? EngineUnmarshallData< F >()( argv[ startArgc + 5 ] ) : F( defaultArgs.f ) );
      G g = ( startArgc + 6 < argc ? EngineUnmarshallData< G >()( argv[ startArgc + 6 ] ) : G( defaultArgs.g ) );
      H h = ( startArgc + 7 < argc ? EngineUnmarshallData< H >()( argv[ startArgc + 7 ] ) : H( defaultArgs.h ) );
      fn( a, b, c, d, e, f, g, h );
   }
   template< typename Frame >
   static void thunk( S32 argc, const char** argv, void ( Frame::*fn )( A, B, C, D, E, F, G, H ) const, Frame* frame, const _EngineFunctionDefaultArguments< void( typename Frame::ObjectType*, A, B, C, D, E, F, G, H ) >& defaultArgs )
   {
      A a = ( startArgc < argc ? EngineUnmarshallData< A >()( argv[ startArgc ] ) : A( defaultArgs.b ) );
      B b = ( startArgc + 1 < argc ? EngineUnmarshallData< B >()( argv[ startArgc + 1 ] ) : B( defaultArgs.c ) );
      C c = ( startArgc + 2 < argc ? EngineUnmarshallData< C >()( argv[ startArgc + 2 ] ) : C( defaultArgs.d ) );
      D d = ( startArgc + 3 < argc ? EngineUnmarshallData< D >()( argv[ startArgc + 3 ] ) : D( defaultArgs.e ) );
      E e = ( startArgc + 4 < argc ? EngineUnmarshallData< E >()( argv[ startArgc + 4 ] ) : E( defaultArgs.f ) );
      F f = ( startArgc + 5 < argc ? EngineUnmarshallData< F >()( argv[ startArgc + 5 ] ) : F( defaultArgs.g ) );
      G g = ( startArgc + 6 < argc ? EngineUnmarshallData< G >()( argv[ startArgc + 6 ] ) : G( defaultArgs.h ) );
      H h = ( startArgc + 7 < argc ? EngineUnmarshallData< H >()( argv[ startArgc + 7 ] ) : H( defaultArgs.i ) );
      ( frame->*fn )( a, b, c, d, e, f, g, h );
   }
};

template< int startArgc, typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I >
struct _EngineConsoleThunk< startArgc, R( A, B, C, D, E, F, G, H, I ) >
{
   typedef typename _EngineConsoleThunkType< R >::ReturnType ReturnType;
   static const int NUM_ARGS = 9 + startArgc;
   static ReturnType thunk( S32 argc, const char** argv, R ( *fn )( A, B, C, D, E, F, G, H, I ), const _EngineFunctionDefaultArguments< void( A, B, C, D, E, F, G, H, I ) >& defaultArgs )
   {
      A a = ( startArgc < argc ? EngineUnmarshallData< A >()( argv[ startArgc ] ) : A( defaultArgs.a ) );
      B b = ( startArgc + 1 < argc ? EngineUnmarshallData< B >()( argv[ startArgc + 1 ] ) : B( defaultArgs.b ) );
      C c = ( startArgc + 2 < argc ? EngineUnmarshallData< C >()( argv[ startArgc + 2 ] ) : C( defaultArgs.c ) );
      D d = ( startArgc + 3 < argc ? EngineUnmarshallData< D >()( argv[ startArgc + 3 ] ) : D( defaultArgs.d ) );
      E e = ( startArgc + 4 < argc ? EngineUnmarshallData< E >()( argv[ startArgc + 4 ] ) : E( defaultArgs.e ) );
      F f = ( startArgc + 5 < argc ? EngineUnmarshallData< F >()( argv[ startArgc + 5 ] ) : F( defaultArgs.f ) );
      G g = ( startArgc + 6 < argc ? EngineUnmarshallData< G >()( argv[ startArgc + 6 ] ) : G( defaultArgs.g ) );
      H h = ( startArgc + 7 < argc ? EngineUnmarshallData< H >()( argv[ startArgc + 7 ] ) : H( defaultArgs.h ) );
      I i = ( startArgc + 8 < argc ? EngineUnmarshallData< I >()( argv[ startArgc + 8 ] ) : I( defaultArgs.i ) );
      return _EngineConsoleThunkReturnValue( fn( a, b, c, d, e, f, g, h, i ) );
   }
   template< typename Frame >
   static ReturnType thunk( S32 argc, const char** argv, R ( Frame::*fn )( A, B, C, D, E, F, G, H, I ) const, Frame* frame, const _EngineFunctionDefaultArguments< void( typename Frame::ObjectType*, A, B, C, D, E, F, G, H, I ) >& defaultArgs )
   {
      A a = ( startArgc < argc ? EngineUnmarshallData< A >()( argv[ startArgc ] ) : A( defaultArgs.b ) );
      B b = ( startArgc + 1 < argc ? EngineUnmarshallData< B >()( argv[ startArgc + 1 ] ) : B( defaultArgs.c ) );
      C c = ( startArgc + 2 < argc ? EngineUnmarshallData< C >()( argv[ startArgc + 2 ] ) : C( defaultArgs.d ) );
      D d = ( startArgc + 3 < argc ? EngineUnmarshallData< D >()( argv[ startArgc + 3 ] ) : D( defaultArgs.e ) );
      E e = ( startArgc + 4 < argc ? EngineUnmarshallData< E >()( argv[ startArgc + 4 ] ) : E( defaultArgs.f ) );
      F f = ( startArgc + 5 < argc ? EngineUnmarshallData< F >()( argv[ startArgc + 5 ] ) : F( defaultArgs.g ) );
      G g = ( startArgc + 6 < argc ? EngineUnmarshallData< G >()( argv[ startArgc + 6 ] ) : G( defaultArgs.h ) );
      H h = ( startArgc + 7 < argc ? EngineUnmarshallData< H >()( argv[ startArgc + 7 ] ) : H( defaultArgs.i ) );
      I i = ( startArgc + 8 < argc ? EngineUnmarshallData< I >()( argv[ startArgc + 8 ] ) : I( defaultArgs.j ) );
      return _EngineConsoleThunkReturnValue( ( frame->*fn )( a, b, c, d, e, f, g, h, i ) );
   }
};
template< int startArgc, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I >
struct _EngineConsoleThunk< startArgc, void( A, B, C, D, E, F, G, H, I ) >
{
   typedef void ReturnType;
   static const int NUM_ARGS = 9 + startArgc;
   static void thunk( S32 argc, const char** argv, void ( *fn )( A, B, C, D, E, F, G, H, I ), const _EngineFunctionDefaultArguments< void( A, B, C, D, E, F, G, H, I ) >& defaultArgs )
   {
      A a = ( startArgc < argc ? EngineUnmarshallData< A >()( argv[ startArgc ] ) : A( defaultArgs.a ) );
      B b = ( startArgc + 1 < argc ? EngineUnmarshallData< B >()( argv[ startArgc + 1 ] ) : B( defaultArgs.b ) );
      C c = ( startArgc + 2 < argc ? EngineUnmarshallData< C >()( argv[ startArgc + 2 ] ) : C( defaultArgs.c ) );
      D d = ( startArgc + 3 < argc ? EngineUnmarshallData< D >()( argv[ startArgc + 3 ] ) : D( defaultArgs.d ) );
      E e = ( startArgc + 4 < argc ? EngineUnmarshallData< E >()( argv[ startArgc + 4 ] ) : E( defaultArgs.e ) );
      F f = ( startArgc + 5 < argc ? EngineUnmarshallData< F >()( argv[ startArgc + 5 ] ) : F( defaultArgs.f ) );
      G g = ( startArgc + 6 < argc ? EngineUnmarshallData< G >()( argv[ startArgc + 6 ] ) : G( defaultArgs.g ) );
      H h = ( startArgc + 7 < argc ? EngineUnmarshallData< H >()( argv[ startArgc + 7 ] ) : H( defaultArgs.h ) );
      I i = ( startArgc + 8 < argc ? EngineUnmarshallData< I >()( argv[ startArgc + 8 ] ) : I( defaultArgs.i ) );
      fn( a, b, c, d, e, f, g, h, i );
   }
   template< typename Frame >
   static void thunk( S32 argc, const char** argv, void ( Frame::*fn )( A, B, C, D, E, F, G, H, I ) const, Frame* frame, const _EngineFunctionDefaultArguments< void( typename Frame::ObjectType*, A, B, C, D, E, F, G, H, I ) >& defaultArgs )
   {
      A a = ( startArgc < argc ? EngineUnmarshallData< A >()( argv[ startArgc ] ) : A( defaultArgs.b ) );
      B b = ( startArgc + 1 < argc ? EngineUnmarshallData< B >()( argv[ startArgc + 1 ] ) : B( defaultArgs.c ) );
      C c = ( startArgc + 2 < argc ? EngineUnmarshallData< C >()( argv[ startArgc + 2 ] ) : C( defaultArgs.d ) );
      D d = ( startArgc + 3 < argc ? EngineUnmarshallData< D >()( argv[ startArgc + 3 ] ) : D( defaultArgs.e ) );
      E e = ( startArgc + 4 < argc ? EngineUnmarshallData< E >()( argv[ startArgc + 4 ] ) : E( defaultArgs.f ) );
      F f = ( startArgc + 5 < argc ? EngineUnmarshallData< F >()( argv[ startArgc + 5 ] ) : F( defaultArgs.g ) );
      G g = ( startArgc + 6 < argc ? EngineUnmarshallData< G >()( argv[ startArgc + 6 ] ) : G( defaultArgs.h ) );
      H h = ( startArgc + 7 < argc ? EngineUnmarshallData< H >()( argv[ startArgc + 7 ] ) : H( defaultArgs.i ) );
      I i = ( startArgc + 8 < argc ? EngineUnmarshallData< I >()( argv[ startArgc + 8 ] ) : I( defaultArgs.j ) );
      ( frame->*fn )( a, b, c, d, e, f, g, h, i );
   }
};

template< int startArgc, typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J >
struct _EngineConsoleThunk< startArgc, R( A, B, C, D, E, F, G, H, I, J ) >
{
   typedef typename _EngineConsoleThunkType< R >::ReturnType ReturnType;
   static const int NUM_ARGS = 10 + startArgc;
   static ReturnType thunk( S32 argc, const char** argv, R ( *fn )( A, B, C, D, E, F, G, H, I, J ), const _EngineFunctionDefaultArguments< void( A, B, C, D, E, F, G, H, I, J ) >& defaultArgs )
   {
      A a = ( startArgc < argc ? EngineUnmarshallData< A >()( argv[ startArgc ] ) : A( defaultArgs.a ) );
      B b = ( startArgc + 1 < argc ? EngineUnmarshallData< B >()( argv[ startArgc + 1 ] ) : B( defaultArgs.b ) );
      C c = ( startArgc + 2 < argc ? EngineUnmarshallData< C >()( argv[ startArgc + 2 ] ) : C( defaultArgs.c ) );
      D d = ( startArgc + 3 < argc ? EngineUnmarshallData< D >()( argv[ startArgc + 3 ] ) : D( defaultArgs.d ) );
      E e = ( startArgc + 4 < argc ? EngineUnmarshallData< E >()( argv[ startArgc + 4 ] ) : E( defaultArgs.e ) );
      F f = ( startArgc + 5 < argc ? EngineUnmarshallData< F >()( argv[ startArgc + 5 ] ) : F( defaultArgs.f ) );
      G g = ( startArgc + 6 < argc ? EngineUnmarshallData< G >()( argv[ startArgc + 6 ] ) : G( defaultArgs.g ) );
      H h = ( startArgc + 7 < argc ? EngineUnmarshallData< H >()( argv[ startArgc + 7 ] ) : H( defaultArgs.h ) );
      I i = ( startArgc + 8 < argc ? EngineUnmarshallData< I >()( argv[ startArgc + 8 ] ) : I( defaultArgs.i ) );
      J j = ( startArgc + 9 < argc ? EngineUnmarshallData< J >()( argv[ startArgc + 9 ] ) : J( defaultArgs.j ) );
      return _EngineConsoleThunkReturnValue( fn( a, b, c, d, e, f, g, h, i, j ) );
   }
   template< typename Frame >
   static ReturnType thunk( S32 argc, const char** argv, R ( Frame::*fn )( A, B, C, D, E, F, G, H, I, J ) const, Frame* frame, const _EngineFunctionDefaultArguments< void( typename Frame::ObjectType*, A, B, C, D, E, F, G, H, I, J ) >& defaultArgs )
   {
      A a = ( startArgc < argc ? EngineUnmarshallData< A >()( argv[ startArgc ] ) : A( defaultArgs.b ) );
      B b = ( startArgc + 1 < argc ? EngineUnmarshallData< B >()( argv[ startArgc + 1 ] ) : B( defaultArgs.c ) );
      C c = ( startArgc + 2 < argc ? EngineUnmarshallData< C >()( argv[ startArgc + 2 ] ) : C( defaultArgs.d ) );
      D d = ( startArgc + 3 < argc ? EngineUnmarshallData< D >()( argv[ startArgc + 3 ] ) : D( defaultArgs.e ) );
      E e = ( startArgc + 4 < argc ? EngineUnmarshallData< E >()( argv[ startArgc + 4 ] ) : E( defaultArgs.f ) );
      F f = ( startArgc + 5 < argc ? EngineUnmarshallData< F >()( argv[ startArgc + 5 ] ) : F( defaultArgs.g ) );
      G g = ( startArgc + 6 < argc ? EngineUnmarshallData< G >()( argv[ startArgc + 6 ] ) : G( defaultArgs.h ) );
      H h = ( startArgc + 7 < argc ? EngineUnmarshallData< H >()( argv[ startArgc + 7 ] ) : H( defaultArgs.i ) );
      I i = ( startArgc + 8 < argc ? EngineUnmarshallData< I >()( argv[ startArgc + 8 ] ) : I( defaultArgs.j ) );
      J j = ( startArgc + 9 < argc ? EngineUnmarshallData< J >()( argv[ startArgc + 9 ] ) : J( defaultArgs.k ) );
      return _EngineConsoleThunkReturnValue( ( frame->*fn )( a, b, c, d, e, f, g, h, i, j ) );
   }
};
template< int startArgc, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J >
struct _EngineConsoleThunk< startArgc, void( A, B, C, D, E, F, G, H, I, J ) >
{
   typedef void ReturnType;
   static const int NUM_ARGS = 10 + startArgc;
   static void thunk( S32 argc, const char** argv, void ( *fn )( A, B, C, D, E, F, G, H, I, J ), const _EngineFunctionDefaultArguments< void( A, B, C, D, E, F, G, H, I, J ) >& defaultArgs )
   {
      A a = ( startArgc < argc ? EngineUnmarshallData< A >()( argv[ startArgc ] ) : A( defaultArgs.a ) );
      B b = ( startArgc + 1 < argc ? EngineUnmarshallData< B >()( argv[ startArgc + 1 ] ) : B( defaultArgs.b ) );
      C c = ( startArgc + 2 < argc ? EngineUnmarshallData< C >()( argv[ startArgc + 2 ] ) : C( defaultArgs.c ) );
      D d = ( startArgc + 3 < argc ? EngineUnmarshallData< D >()( argv[ startArgc + 3 ] ) : D( defaultArgs.d ) );
      E e = ( startArgc + 4 < argc ? EngineUnmarshallData< E >()( argv[ startArgc + 4 ] ) : E( defaultArgs.e ) );
      F f = ( startArgc + 5 < argc ? EngineUnmarshallData< F >()( argv[ startArgc + 5 ] ) : F( defaultArgs.f ) );
      G g = ( startArgc + 6 < argc ? EngineUnmarshallData< G >()( argv[ startArgc + 6 ] ) : G( defaultArgs.g ) );
      H h = ( startArgc + 7 < argc ? EngineUnmarshallData< H >()( argv[ startArgc + 7 ] ) : H( defaultArgs.h ) );
      I i = ( startArgc + 8 < argc ? EngineUnmarshallData< I >()( argv[ startArgc + 8 ] ) : I( defaultArgs.i ) );
      J j = ( startArgc + 9 < argc ? EngineUnmarshallData< J >()( argv[ startArgc + 9 ] ) : J( defaultArgs.j ) );
      fn( a, b, c, d, e, f, g, h, i, j );
   }
   template< typename Frame >
   static void thunk( S32 argc, const char** argv, void ( Frame::*fn )( A, B, C, D, E, F, G, H, I, J ) const, Frame* frame, const _EngineFunctionDefaultArguments< void( typename Frame::ObjectType*, A, B, C, D, E, F, G, H, I, J ) >& defaultArgs )
   {
      A a = ( startArgc < argc ? EngineUnmarshallData< A >()( argv[ startArgc ] ) : A( defaultArgs.b ) );
      B b = ( startArgc + 1 < argc ? EngineUnmarshallData< B >()( argv[ startArgc + 1 ] ) : B( defaultArgs.c ) );
      C c = ( startArgc + 2 < argc ? EngineUnmarshallData< C >()( argv[ startArgc + 2 ] ) : C( defaultArgs.d ) );
      D d = ( startArgc + 3 < argc ? EngineUnmarshallData< D >()( argv[ startArgc + 3 ] ) : D( defaultArgs.e ) );
      E e = ( startArgc + 4 < argc ? EngineUnmarshallData< E >()( argv[ startArgc + 4 ] ) : E( defaultArgs.f ) );
      F f = ( startArgc + 5 < argc ? EngineUnmarshallData< F >()( argv[ startArgc + 5 ] ) : F( defaultArgs.g ) );
      G g = ( startArgc + 6 < argc ? EngineUnmarshallData< G >()( argv[ startArgc + 6 ] ) : G( defaultArgs.h ) );
      H h = ( startArgc + 7 < argc ? EngineUnmarshallData< H >()( argv[ startArgc + 7 ] ) : H( defaultArgs.i ) );
      I i = ( startArgc + 8 < argc ? EngineUnmarshallData< I >()( argv[ startArgc + 8 ] ) : I( defaultArgs.j ) );
      J j = ( startArgc + 9 < argc ? EngineUnmarshallData< J >()( argv[ startArgc + 9 ] ) : J( defaultArgs.k ) );
      ( frame->*fn )( a, b, c, d, e, f, g, h, i, j );
   }
};
template< int startArgc, typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K >
struct _EngineConsoleThunk< startArgc, R( A, B, C, D, E, F, G, H, I, J, K ) >
{
   typedef typename _EngineConsoleThunkType< R >::ReturnType ReturnType;
   static const int NUM_ARGS = 11 + startArgc;
   static ReturnType thunk( S32 argc, const char** argv, R ( *fn )( A, B, C, D, E, F, G, H, I, J, K ), const _EngineFunctionDefaultArguments< void( A, B, C, D, E, F, G, H, I, J, K ) >& defaultArgs )
   {
      A a = ( startArgc < argc ? EngineUnmarshallData< A >()( argv[ startArgc ] ) : A( defaultArgs.a ) );
      B b = ( startArgc + 1 < argc ? EngineUnmarshallData< B >()( argv[ startArgc + 1 ] ) : B( defaultArgs.b ) );
      C c = ( startArgc + 2 < argc ? EngineUnmarshallData< C >()( argv[ startArgc + 2 ] ) : C( defaultArgs.c ) );
      D d = ( startArgc + 3 < argc ? EngineUnmarshallData< D >()( argv[ startArgc + 3 ] ) : D( defaultArgs.d ) );
      E e = ( startArgc + 4 < argc ? EngineUnmarshallData< E >()( argv[ startArgc + 4 ] ) : E( defaultArgs.e ) );
      F f = ( startArgc + 5 < argc ? EngineUnmarshallData< F >()( argv[ startArgc + 5 ] ) : F( defaultArgs.f ) );
      G g = ( startArgc + 6 < argc ? EngineUnmarshallData< G >()( argv[ startArgc + 6 ] ) : G( defaultArgs.g ) );
      H h = ( startArgc + 7 < argc ? EngineUnmarshallData< H >()( argv[ startArgc + 7 ] ) : H( defaultArgs.h ) );
      I i = ( startArgc + 8 < argc ? EngineUnmarshallData< I >()( argv[ startArgc + 8 ] ) : I( defaultArgs.i ) );
      J j = ( startArgc + 9 < argc ? EngineUnmarshallData< J >()( argv[ startArgc + 9 ] ) : J( defaultArgs.j ) );
      K k = ( startArgc + 10 < argc ? EngineUnmarshallData< K >()( argv[ startArgc + 10 ] ) : K( defaultArgs.k ) );
      return _EngineConsoleThunkReturnValue( fn( a, b, c, d, e, f, g, h, i, j, k ) );
   }
   template< typename Frame >
   static ReturnType thunk( S32 argc, const char** argv, R ( Frame::*fn )( A, B, C, D, E, F, G, H, I, J, K ) const, Frame* frame, const _EngineFunctionDefaultArguments< void( typename Frame::ObjectType*, A, B, C, D, E, F, G, H, I, J, K ) >& defaultArgs )
   {
      A a = ( startArgc < argc ? EngineUnmarshallData< A >()( argv[ startArgc ] ) : A( defaultArgs.b ) );
      B b = ( startArgc + 1 < argc ? EngineUnmarshallData< B >()( argv[ startArgc + 1 ] ) : B( defaultArgs.c ) );
      C c = ( startArgc + 2 < argc ? EngineUnmarshallData< C >()( argv[ startArgc + 2 ] ) : C( defaultArgs.d ) );
      D d = ( startArgc + 3 < argc ? EngineUnmarshallData< D >()( argv[ startArgc + 3 ] ) : D( defaultArgs.e ) );
      E e = ( startArgc + 4 < argc ? EngineUnmarshallData< E >()( argv[ startArgc + 4 ] ) : E( defaultArgs.f ) );
      F f = ( startArgc + 5 < argc ? EngineUnmarshallData< F >()( argv[ startArgc + 5 ] ) : F( defaultArgs.g ) );
      G g = ( startArgc + 6 < argc ? EngineUnmarshallData< G >()( argv[ startArgc + 6 ] ) : G( defaultArgs.h ) );
      H h = ( startArgc + 7 < argc ? EngineUnmarshallData< H >()( argv[ startArgc + 7 ] ) : H( defaultArgs.i ) );
      I i = ( startArgc + 8 < argc ? EngineUnmarshallData< I >()( argv[ startArgc + 8 ] ) : I( defaultArgs.j ) );
      J j = ( startArgc + 9 < argc ? EngineUnmarshallData< J >()( argv[ startArgc + 9 ] ) : J( defaultArgs.k ) );
      K k = ( startArgc + 10 < argc ? EngineUnmarshallData< K >()( argv[ startArgc + 10 ] ) : K( defaultArgs.l ) );
      return _EngineConsoleThunkReturnValue( ( frame->*fn )( a, b, c, d, e, f, g, h, i, j, k ) );
   }
};
template< int startArgc, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K >
struct _EngineConsoleThunk< startArgc, void( A, B, C, D, E, F, G, H, I, J, K ) >
{
   typedef void ReturnType;
   static const int NUM_ARGS = 11 + startArgc;
   static void thunk( S32 argc, const char** argv, void ( *fn )( A, B, C, D, E, F, G, H, I, J, K ), const _EngineFunctionDefaultArguments< void( A, B, C, D, E, F, G, H, I, J, K ) >& defaultArgs )
   {
      A a = ( startArgc < argc ? EngineUnmarshallData< A >()( argv[ startArgc ] ) : A( defaultArgs.a ) );
      B b = ( startArgc + 1 < argc ? EngineUnmarshallData< B >()( argv[ startArgc + 1 ] ) : B( defaultArgs.b ) );
      C c = ( startArgc + 2 < argc ? EngineUnmarshallData< C >()( argv[ startArgc + 2 ] ) : C( defaultArgs.c ) );
      D d = ( startArgc + 3 < argc ? EngineUnmarshallData< D >()( argv[ startArgc + 3 ] ) : D( defaultArgs.d ) );
      E e = ( startArgc + 4 < argc ? EngineUnmarshallData< E >()( argv[ startArgc + 4 ] ) : E( defaultArgs.e ) );
      F f = ( startArgc + 5 < argc ? EngineUnmarshallData< F >()( argv[ startArgc + 5 ] ) : F( defaultArgs.f ) );
      G g = ( startArgc + 6 < argc ? EngineUnmarshallData< G >()( argv[ startArgc + 6 ] ) : G( defaultArgs.g ) );
      H h = ( startArgc + 7 < argc ? EngineUnmarshallData< H >()( argv[ startArgc + 7 ] ) : H( defaultArgs.h ) );
      I i = ( startArgc + 8 < argc ? EngineUnmarshallData< I >()( argv[ startArgc + 8 ] ) : I( defaultArgs.i ) );
      J j = ( startArgc + 9 < argc ? EngineUnmarshallData< J >()( argv[ startArgc + 9 ] ) : J( defaultArgs.j ) );
      K k = ( startArgc + 10 < argc ? EngineUnmarshallData< K >()( argv[ startArgc + 10 ] ) : K( defaultArgs.k ) );
      fn( a, b, c, d, e, f, g, h, i, j, k );
   }
   template< typename Frame >
   static void thunk( S32 argc, const char** argv, void ( Frame::*fn )( A, B, C, D, E, F, G, H, I, J, K ) const, Frame* frame, const _EngineFunctionDefaultArguments< void( typename Frame::ObjectType*, A, B, C, D, E, F, G, H, I, J, K ) >& defaultArgs )
   {
      A a = ( startArgc < argc ? EngineUnmarshallData< A >()( argv[ startArgc ] ) : A( defaultArgs.b ) );
      B b = ( startArgc + 1 < argc ? EngineUnmarshallData< B >()( argv[ startArgc + 1 ] ) : B( defaultArgs.c ) );
      C c = ( startArgc + 2 < argc ? EngineUnmarshallData< C >()( argv[ startArgc + 2 ] ) : C( defaultArgs.d ) );
      D d = ( startArgc + 3 < argc ? EngineUnmarshallData< D >()( argv[ startArgc + 3 ] ) : D( defaultArgs.e ) );
      E e = ( startArgc + 4 < argc ? EngineUnmarshallData< E >()( argv[ startArgc + 4 ] ) : E( defaultArgs.f ) );
      F f = ( startArgc + 5 < argc ? EngineUnmarshallData< F >()( argv[ startArgc + 5 ] ) : F( defaultArgs.g ) );
      G g = ( startArgc + 6 < argc ? EngineUnmarshallData< G >()( argv[ startArgc + 6 ] ) : G( defaultArgs.h ) );
      H h = ( startArgc + 7 < argc ? EngineUnmarshallData< H >()( argv[ startArgc + 7 ] ) : H( defaultArgs.i ) );
      I i = ( startArgc + 8 < argc ? EngineUnmarshallData< I >()( argv[ startArgc + 8 ] ) : I( defaultArgs.j ) );
      J j = ( startArgc + 9 < argc ? EngineUnmarshallData< J >()( argv[ startArgc + 9 ] ) : J( defaultArgs.k ) );
      K k = ( startArgc + 10 < argc ? EngineUnmarshallData< K >()( argv[ startArgc + 10 ] ) : K( defaultArgs.l ) );
      ( frame->*fn )( a, b, c, d, e, f, g, h, i, j, k );
   }
};


/// @}

/// @name API Definition Macros
///
/// The macros in this group allow to create engine API functions that work both with the
/// legacy console system as well as with the new engine export system.  As such, they only
/// support those function features that are available in both systems.  This means that for
/// console-style variadic functions, the ConsoleXXX must be used and that for overloaded
/// and/or C-style variadic functions as well as for placing functions in export scopes,
/// DEFINE_CALLIN must be used directly.
///
/// When the console system is removed, the console thunking functionality will be removed
/// from these macros but otherwise they will remain unchanged and in place.
///
/// @{


// Helpers to implement initialization checks.  Pulled out into separate macros so this can be deactivated easily.
// Especially important for the initialize() function itself.

#define _CHECK_ENGINE_INITIALIZED_IMPL( fnName, returnType )                                                                     \
      if( !engineAPI::gIsInitialized )                                                                                           \
      {                                                                                                                          \
         Con::errorf( "EngineAPI: Engine not initialized when calling " #fnName );                                               \
         return EngineTypeTraits< returnType >::ReturnValue( EngineTypeTraits< returnType >::ReturnValueType() );                \
      }

#define _CHECK_ENGINE_INITIALIZED( fnName, returnType ) _CHECK_ENGINE_INITIALIZED_IMPL( fnName, returnType )


/// Define a call-in point for calling into the engine.
///
/// @param name The name of the function as it should be seen by the control layer.
/// @param returnType The value type returned to the control layer.
/// @param args The argument list as it would appear on the function definition
/// @param defaultArgs The list of default argument values.
/// @param usage The usage doc string for the engine API reference.
///
/// @code
/// DefineEngineFunction( myFunction, int, ( float f, const String& s ), ( "value for s" ), "This is my function." )
/// {
///    return int( f ) + dAtoi( s );
/// }
/// @endcode
#define DefineEngineFunction( name, returnType, args, defaultArgs, usage )                                                       \
   static inline returnType _fn ## name ## impl args;                                                                            \
   TORQUE_API EngineTypeTraits< returnType >::ReturnValueType fn ## name                                                         \
      ( _EngineFunctionTrampoline< returnType args >::Args a )                                                                   \
   {                                                                                                                             \
      _CHECK_ENGINE_INITIALIZED( name, returnType );                                                                             \
      return EngineTypeTraits< returnType >::ReturnValue(                                                                        \
         _EngineFunctionTrampoline< returnType args >::jmp( _fn ## name ## impl, a )                                             \
      );                                                                                                                         \
   }                                                                                                                             \
   static _EngineFunctionDefaultArguments< void args > _fn ## name ## DefaultArgs defaultArgs;                                   \
   static EngineFunctionInfo _fn ## name ## FunctionInfo(                                                                        \
      #name,                                                                                                                     \
      &_SCOPE<>()(),                                                                                                             \
      usage,                                                                                                                     \
      #returnType " " #name #args,                                                                                               \
      "fn" #name,                                                                                                                \
      TYPE< returnType args >(),                                                                                                 \
      &_fn ## name ## DefaultArgs,                                                                                               \
      ( void* ) &fn ## name,                                                                                                     \
      0                                                                                                                          \
   );                                                                                                                            \
   static _EngineConsoleThunkType< returnType >::ReturnType _ ## name ## caster( SimObject*, S32 argc, const char** argv )       \
   {                                                                                                                             \
      return _EngineConsoleThunkType< returnType >::ReturnType( _EngineConsoleThunk< 1, returnType args >::thunk(                \
         argc, argv, &_fn ## name ## impl, _fn ## name ## DefaultArgs                                                            \
      ) );                                                                                                                       \
   }                                                                                                                             \
   static ConsoleFunctionHeader _ ## name ## header                                                                              \
      ( #returnType, #args, #defaultArgs );                                                                                      \
   static ConsoleConstructor                                                                                                     \
      _ ## name ## obj( NULL, #name, _EngineConsoleThunkType< returnType >::CallbackType( _ ## name ## caster ), usage,          \
         _EngineConsoleThunk< 1, returnType args >::NUM_ARGS - _EngineConsoleThunkCountArgs() defaultArgs,                       \
         _EngineConsoleThunk< 1, returnType args >::NUM_ARGS,                                                                    \
         false, &_ ## name ## header                                                                                             \
      );                                                                                                                         \
   static inline returnType _fn ## name ## impl args
   
   
// The next thing is a bit tricky.  DefineEngineMethod allows to make the 'object' (=this) argument to the function
// implicit which presents quite an obstacle for the macro internals as the engine export system requires the
// name of a DLL symbol that represents an extern "C" function with an explicit first object pointer argument.
//
// Even if we ignored the fact that we don't have a guarantee how the various C++ compilers implement implicit 'this' arguments,
// we could still not just use a C++ method for this as then we would have to get past the C++ compiler's mangling to
// get to the function symbol name (let alone the fact that typing this method correctly would be tricky).
//
// So, the trick employed here is to package all but the implicit 'this' argument in a structure and then define an
// extern "C" function that takes the object pointer as a first argument and the struct type as the second argument.
// This will result in a function with an identical stack call frame layout to the function we want.
//
// Unfortunately, that still requires that function to chain on to the real user-defined function.  To do this
// cleanly and portably, _EngineMethodTrampoline is used to unpack and jump the call from extern "C" into C++ space.
// In optimized builds, the compiler should be smart enough to pretty much optimize all our trickery here away.

#define _DefineMethodTrampoline( className, name, returnType, args ) \
   TORQUE_API EngineTypeTraits< returnType >::ReturnValueType \
      fn ## className ## _ ## name ( className* object, _EngineMethodTrampoline< _ ## className ## name ## frame, returnType args >::Args a )   \
   {                                                                                                                                            \
      _CHECK_ENGINE_INITIALIZED( className::name, returnType );                                                                                 \
      return EngineTypeTraits< returnType >::ReturnValue(                                                                                       \
         _EngineMethodTrampoline< _ ## className ## name ## frame, returnType args >::jmp( object, a )                                          \
      );                                                                                                                                        \
   }


/// Define a call-in point for calling a method on an engine object.
///
/// @param name The name of the C++ class.
/// @param name The name of the method as it should be seen by the control layer.
/// @param returnType The value type returned to the control layer.
/// @param args The argument list as it would appear on the function definition
/// @param defaultArgs The list of default argument values.
/// @param usage The usage doc string for the engine API reference.
///
/// @code
/// DefineEngineMethod( MyClass, myMethod, int, ( float f, const String& s ), ( "value for s" ), "This is my method." )
/// {
///    return object->someMethod( f, s );
/// }
/// @endcode
#define DefineEngineMethod( className, name, returnType, args, defaultArgs, usage )                                                             \
   struct _ ## className ## name ## frame                                                                                                       \
   {                                                                                                                                            \
      typedef className ObjectType;                                                                                                             \
      className* object;                                                                                                                        \
      inline returnType _exec args const;                                                                                                       \
   };                                                                                                                                           \
   _DefineMethodTrampoline( className, name, returnType, args );                                                                                \
   static _EngineFunctionDefaultArguments< _EngineMethodTrampoline< _ ## className ## name ## frame, void args >::FunctionType >                \
      _fn ## className ## name ## DefaultArgs defaultArgs;                                                                                      \
   static EngineFunctionInfo _fn ## className ## name ## FunctionInfo(                                                                          \
      #name,                                                                                                                                    \
      &_SCOPE< className >()(),                                                                                                                 \
      usage,                                                                                                                                    \
      "virtual " #returnType " " #name #args,                                                                                                   \
      "fn" #className "_" #name,                                                                                                                \
      TYPE< _EngineMethodTrampoline< _ ## className ## name ## frame, returnType args >::FunctionType >(),                                      \
      &_fn ## className ## name ## DefaultArgs,                                                                                                 \
      ( void* ) &fn ## className ## _ ## name,                                                                                                  \
      0                                                                                                                                         \
   );                                                                                                                                           \
   static _EngineConsoleThunkType< returnType >::ReturnType _ ## className ## name ## caster( SimObject* object, S32 argc, const char** argv )  \
   {                                                                                                                                            \
      _ ## className ## name ## frame frame;                                                                                                    \
      frame.object = static_cast< className* >( object );                                                                                       \
      return _EngineConsoleThunkType< returnType >::ReturnType( _EngineConsoleThunk< 2, returnType args >::thunk(                               \
         argc, argv, &_ ## className ## name ## frame::_exec, &frame, _fn ## className ## name ## DefaultArgs                                   \
      ) );                                                                                                                                      \
   }                                                                                                                                            \
   static ConsoleFunctionHeader _ ## className ## name ## header                                                                                \
      ( #returnType, #args, #defaultArgs );                                                                                                     \
   static ConsoleConstructor                                                                                                                    \
      className ## name ## obj( #className, #name,                                                                                              \
         _EngineConsoleThunkType< returnType >::CallbackType( _ ## className ## name ## caster ), usage,                                        \
         _EngineConsoleThunk< 2, returnType args >::NUM_ARGS - _EngineConsoleThunkCountArgs() defaultArgs,                                      \
         _EngineConsoleThunk< 2, returnType args >::NUM_ARGS,                                                                                   \
         false, &_ ## className ## name ## header                                                                                               \
      );                                                                                                                                        \
   returnType _ ## className ## name ## frame::_exec args const
   
   
/// Define a call-in point for calling into the engine.  Unlike with DefineEngineFunction, the statically
/// callable function will be confined to the namespace of the given class.
///
/// @param name The name of the C++ class (or a registered export scope).
/// @param name The name of the method as it should be seen by the control layer.
/// @param returnType The value type returned to the control layer.
/// @param args The argument list as it would appear on the function definition
/// @param defaultArgs The list of default argument values.
/// @param usage The usage doc string for the engine API reference.
///
/// @code
/// DefineEngineStaticMethod( MyClass, myMethod, int, ( float f, string s ), ( "value for s" ), "This is my method." )
/// {
/// }
/// @endcode
#define DefineEngineStaticMethod( className, name, returnType, args, defaultArgs, usage )                                              \
   static inline returnType _fn ## className ## name ## impl args;                                                                     \
   TORQUE_API EngineTypeTraits< returnType >::ReturnValueType fn ## className ## _ ## name                                             \
      ( _EngineFunctionTrampoline< returnType args >::Args a )                                                                         \
   {                                                                                                                                   \
      _CHECK_ENGINE_INITIALIZED( className::name, returnType );                                                                        \
      return EngineTypeTraits< returnType >::ReturnValue(                                                                              \
         _EngineFunctionTrampoline< returnType args >::jmp( _fn ## className ## name ## impl, a )                                      \
      );                                                                                                                               \
   }                                                                                                                                   \
   static _EngineFunctionDefaultArguments< void args > _fn ## className ## name ## DefaultArgs defaultArgs;                            \
   static EngineFunctionInfo _fn ## name ## FunctionInfo(                                                                              \
      #name,                                                                                                                           \
      &_SCOPE< className >()(),                                                                                                        \
      usage,                                                                                                                           \
      #returnType " " #name #args,                                                                                                     \
      "fn" #className "_" #name,                                                                                                       \
      TYPE< returnType args >(),                                                                                                       \
      &_fn ## className ## name ## DefaultArgs,                                                                                        \
      ( void* ) &fn ## className ## _ ## name,                                                                                         \
      0                                                                                                                                \
   );                                                                                                                                  \
   static _EngineConsoleThunkType< returnType >::ReturnType _ ## className ## name ## caster( SimObject*, S32 argc, const char** argv )\
   {                                                                                                                                   \
      return _EngineConsoleThunkType< returnType >::ReturnType( _EngineConsoleThunk< 1, returnType args >::thunk(                      \
         argc, argv, &_fn ## className ## name ## impl, _fn ## className ## name ## DefaultArgs                                        \
      ) );                                                                                                                             \
   }                                                                                                                                   \
   static ConsoleFunctionHeader _ ## className ## name ## header                                                                       \
      ( #returnType, #args, #defaultArgs, true );                                                                                      \
   static ConsoleConstructor                                                                                                           \
      _ ## className ## name ## obj( #className, #name, _EngineConsoleThunkType< returnType >::CallbackType( _ ## className ## name ## caster ), usage, \
         _EngineConsoleThunk< 1, returnType args >::NUM_ARGS - _EngineConsoleThunkCountArgs() defaultArgs,                             \
         _EngineConsoleThunk< 1, returnType args >::NUM_ARGS,                                                                          \
         false, &_ ## className ## name ## header                                                                                      \
      );                                                                                                                               \
   static inline returnType _fn ## className ## name ## impl args


// Convenience macros to allow defining functions that use the new marshalling features
// while being only visible in the console interop.  When we drop the console system,
// these macros can be removed and all definitions that make use of them can be removed
// as well.
#define DefineConsoleFunction( name, returnType, args, defaultArgs, usage )                                                      \
   static inline returnType _fn ## name ## impl args;                                                                            \
   static _EngineFunctionDefaultArguments< void args > _fn ## name ## DefaultArgs defaultArgs;                                   \
   static _EngineConsoleThunkType< returnType >::ReturnType _ ## name ## caster( SimObject*, S32 argc, const char** argv )       \
   {                                                                                                                             \
      return _EngineConsoleThunkType< returnType >::ReturnType( _EngineConsoleThunk< 1, returnType args >::thunk(                \
         argc, argv, &_fn ## name ## impl, _fn ## name ## DefaultArgs                                                            \
      ) );                                                                                                                       \
   }                                                                                                                             \
   static ConsoleFunctionHeader _ ## name ## header                                                                              \
      ( #returnType, #args, #defaultArgs );                                                                                      \
   static ConsoleConstructor                                                                                                     \
      _ ## name ## obj( NULL, #name, _EngineConsoleThunkType< returnType >::CallbackType( _ ## name ## caster ), usage,          \
         _EngineConsoleThunk< 1, returnType args >::NUM_ARGS - _EngineConsoleThunkCountArgs() defaultArgs,                       \
         _EngineConsoleThunk< 1, returnType args >::NUM_ARGS,                                                                    \
         false, &_ ## name ## header                                                                                             \
      );                                                                                                                         \
   static inline returnType _fn ## name ## impl args

#define DefineConsoleMethod( className, name, returnType, args, defaultArgs, usage )                                                            \
   struct _ ## className ## name ## frame                                                                                                       \
   {                                                                                                                                            \
      typedef className ObjectType;                                                                                                             \
      className* object;                                                                                                                        \
      inline returnType _exec args const;                                                                                                       \
   };                                                                                                                                           \
   static _EngineFunctionDefaultArguments< _EngineMethodTrampoline< _ ## className ## name ## frame, void args >::FunctionType >                \
      _fn ## className ## name ## DefaultArgs defaultArgs;                                                                                      \
   static _EngineConsoleThunkType< returnType >::ReturnType _ ## className ## name ## caster( SimObject* object, S32 argc, const char** argv )  \
   {                                                                                                                                            \
      _ ## className ## name ## frame frame;                                                                                                    \
      frame.object = static_cast< className* >( object );                                                                                       \
      return _EngineConsoleThunkType< returnType >::ReturnType( _EngineConsoleThunk< 2, returnType args >::thunk(                               \
         argc, argv, &_ ## className ## name ## frame::_exec, &frame, _fn ## className ## name ## DefaultArgs                                   \
      ) );                                                                                                                                      \
   }                                                                                                                                            \
   static ConsoleFunctionHeader _ ## className ## name ## header                                                                                \
      ( #returnType, #args, #defaultArgs );                                                                                                     \
   static ConsoleConstructor                                                                                                                    \
      className ## name ## obj( #className, #name,                                                                                              \
         _EngineConsoleThunkType< returnType >::CallbackType( _ ## className ## name ## caster ), usage,                                        \
         _EngineConsoleThunk< 2, returnType args >::NUM_ARGS - _EngineConsoleThunkCountArgs() defaultArgs,                                      \
         _EngineConsoleThunk< 2, returnType args >::NUM_ARGS,                                                                                   \
         false, &_ ## className ## name ## header                                                                                               \
      );                                                                                                                                        \
   returnType _ ## className ## name ## frame::_exec args const

#define DefineConsoleStaticMethod( className, name, returnType, args, defaultArgs, usage )                                             \
   static inline returnType _fn ## className ## name ## impl args;                                                                     \
   static _EngineFunctionDefaultArguments< void args > _fn ## className ## name ## DefaultArgs defaultArgs;                            \
   static _EngineConsoleThunkType< returnType >::ReturnType _ ## className ## name ## caster( SimObject*, S32 argc, const char** argv )\
   {                                                                                                                                   \
      return _EngineConsoleThunkType< returnType >::ReturnType( _EngineConsoleThunk< 1, returnType args >::thunk(                      \
         argc, argv, &_fn ## className ## name ## impl, _fn ## className ## name ## DefaultArgs                                        \
      ) );                                                                                                                             \
   }                                                                                                                                   \
   static ConsoleFunctionHeader _ ## className ## name ## header                                                                       \
      ( #returnType, #args, #defaultArgs, true );                                                                                      \
   static ConsoleConstructor                                                                                                           \
      _ ## className ## name ## obj( #className, #name, _EngineConsoleThunkType< returnType >::CallbackType( _ ## className ## name ## caster ), usage, \
         _EngineConsoleThunk< 1, returnType args >::NUM_ARGS - _EngineConsoleThunkCountArgs() defaultArgs,                             \
         _EngineConsoleThunk< 1, returnType args >::NUM_ARGS,                                                                          \
         false, &_ ## className ## name ## header                                                                                      \
      );                                                                                                                               \
   static inline returnType _fn ## className ## name ## impl args


// The following three macros are only temporary.  They allow to define engineAPI functions using the framework
// here in this file while being visible only in the new API.  When the console interop is removed, these macros
// can be removed and all their uses be replaced with their corresponding versions that now still include support
// for the console (e.g. DefineNewEngineFunction should become DefineEngineFunction).
#define DefineNewEngineFunction( name, returnType, args, defaultArgs, usage )                                                    \
   static inline returnType _fn ## name ## impl args;                                                                            \
   TORQUE_API EngineTypeTraits< returnType >::ReturnValueType fn ## name                                                         \
      ( _EngineFunctionTrampoline< returnType args >::Args a )                                                                   \
   {                                                                                                                             \
      _CHECK_ENGINE_INITIALIZED( name, returnType );                                                                             \
      return EngineTypeTraits< returnType >::ReturnValue(                                                                        \
         _EngineFunctionTrampoline< returnType args >::jmp( _fn ## name ## impl, a )                                             \
      );                                                                                                                         \
   }                                                                                                                             \
   static _EngineFunctionDefaultArguments< void args > _fn ## name ## DefaultArgs defaultArgs;                                   \
   static EngineFunctionInfo _fn ## name ## FunctionInfo(                                                                        \
      #name,                                                                                                                     \
      &_SCOPE<>()(),                                                                                                             \
      usage,                                                                                                                     \
      #returnType " " #name #args,                                                                                               \
      "fn" #name,                                                                                                                \
      TYPE< returnType args >(),                                                                                                 \
      &_fn ## name ## DefaultArgs,                                                                                               \
      ( void* ) &fn ## name,                                                                                                     \
      0                                                                                                                          \
   );                                                                                                                            \
   static inline returnType _fn ## name ## impl args

#define DefineNewEngineMethod( className, name, returnType, args, defaultArgs, usage )                                                          \
   struct _ ## className ## name ## frame                                                                                                       \
   {                                                                                                                                            \
      typedef className ObjectType;                                                                                                             \
      className* object;                                                                                                                        \
      inline returnType _exec args const;                                                                                                       \
   };                                                                                                                                           \
   _DefineMethodTrampoline( className, name, returnType, args );                                                                                \
   static _EngineFunctionDefaultArguments< _EngineMethodTrampoline< _ ## className ## name ## frame, void args >::FunctionType >                \
      _fn ## className ## name ## DefaultArgs defaultArgs;                                                                                      \
   static EngineFunctionInfo _fn ## className ## name ## FunctionInfo(                                                                          \
      #name,                                                                                                                                    \
      &_SCOPE< className >()(),                                                                                                                 \
      usage,                                                                                                                                    \
      "virtual " #returnType " " #name #args,                                                                                                   \
      "fn" #className "_" #name,                                                                                                                \
      TYPE< _EngineMethodTrampoline< _ ## className ## name ## frame, returnType args >::FunctionType >(),                                      \
      &_fn ## className ## name ## DefaultArgs,                                                                                                 \
      ( void* ) &fn ## className ## _ ## name,                                                                                                  \
      0                                                                                                                                         \
   );                                                                                                                                           \
   returnType _ ## className ## name ## frame::_exec args const

#define DefineNewEngineStaticMethod( className, name, returnType, args, defaultArgs, usage )                                           \
   static inline returnType _fn ## className ## name ## impl args;                                                                     \
   TORQUE_API EngineTypeTraits< returnType >::ReturnValueType fn ## className ## _ ## name                                             \
      ( _EngineFunctionTrampoline< returnType args >::Args a )                                                                         \
   {                                                                                                                                   \
      _CHECK_ENGINE_INITIALIZED( className::name, returnType );                                                                        \
      return EngineTypeTraits< returnType >::ReturnValue(                                                                              \
         _EngineFunctionTrampoline< returnType args >::jmp( _fn ## className ## name ## impl, a )                                      \
      );                                                                                                                               \
   }                                                                                                                                   \
   static _EngineFunctionDefaultArguments< void args > _fn ## className ## name ## DefaultArgs defaultArgs;                            \
   static EngineFunctionInfo _fn ## name ## FunctionInfo(                                                                              \
      #name,                                                                                                                           \
      &_SCOPE< className >()(),                                                                                                        \
      usage,                                                                                                                           \
      #returnType " " #name #args,                                                                                                     \
      "fn" #className "_" #name,                                                                                                       \
      TYPE< returnType args >(),                                                                                                       \
      &_fn ## className ## name ## DefaultArgs,                                                                                        \
      ( void* ) &fn ## className ## _ ## name,                                                                                         \
      0                                                                                                                                \
   );                                                                                                                                  \
   static inline returnType _fn ## className ## name ## impl args

/// @}


//=============================================================================
//    Callbacks.
//=============================================================================

/// Matching implement for DECLARE_CALLBACK.
///
///
/// @warn With the new interop system, method-style callbacks <em>must not</em> be triggered on object
///   that are being created!  This is because the control layer will likely not yet have a fully valid wrapper
///   object in place for the EngineObject under construction.
#define IMPLEMENT_CALLBACK( class, name, returnType, args, argNames, usageString )                                                           \
   struct _ ## class ## name ## frame { typedef class ObjectType; };                                                                         \
   TORQUE_API _EngineMethodTrampoline< _ ## class ## name ## frame, returnType args >::FunctionType* cb ## class ## _ ## name;               \
   TORQUE_API void set_cb ## class ## _ ## name(                                                                                             \
      _EngineMethodTrampoline< _ ## class ## name ## frame, returnType args >::FunctionType fn )                                             \
      { cb ## class ## _ ## name = fn; }                                                                                                     \
   _EngineMethodTrampoline< _ ## class ## name ## frame, returnType args >::FunctionType* cb ## class ## _ ## name;                          \
   namespace {                                                                                                                               \
      ::EngineFunctionInfo _cb ## class ## name(                                                                                             \
         #name,                                                                                                                              \
         &::_SCOPE< class >()(),                                                                                                             \
         usageString,                                                                                                                        \
         "virtual " #returnType " " #name #args,                                                                                             \
         "cb" #class "_" #name,                                                                                                              \
         ::TYPE< _EngineMethodTrampoline< _ ## class ## name ## frame, returnType args >::FunctionType >(),                                  \
         NULL,                                                                                                                               \
         ( void* ) &cb ## class ## _ ## name,                                                                                                \
         EngineFunctionCallout                                                                                                               \
      );                                                                                                                                     \
   }                                                                                                                                         \
   returnType class::name ## _callback args                                                                                                  \
   {                                                                                                                                         \
      if( cb ## class ## _ ## name ) {                                                                                                       \
         _EngineCallbackHelper cbh( this, reinterpret_cast< const void* >( cb ## class ## _ ## name ) );                                     \
         return returnType( cbh.call< returnType > argNames );                                                                               \
      }                                                                                                                                      \
      if( engineAPI::gUseConsoleInterop )                                                                                                    \
      {                                                                                                                                      \
         static StringTableEntry sName = StringTable->insert( #name );                                                                       \
         _EngineConsoleCallbackHelper cbh( sName, this );                                                                                    \
         return returnType( cbh.call< returnType > argNames );                                                                               \
      }                                                                                                                                      \
      return returnType();                                                                                                                   \
   }                                                                                                                                         \
   namespace {                                                                                                                               \
      ConsoleFunctionHeader _ ## class ## name ## header(                                                                                    \
         #returnType, #args, "" );                                                                                                           \
      ConsoleConstructor _ ## class ## name ## obj( #class, #name, usageString, &_ ## class ## name ## header );                             \
   }


/// Used to define global callbacks not associated with 
/// any particular class or namespace.
#define IMPLEMENT_GLOBAL_CALLBACK( name, returnType, args, argNames, usageString )                                                           \
   DEFINE_CALLOUT( cb ## name, name,, returnType, args, 0, usageString );                                                                    \
   returnType name ## _callback args                                                                                                         \
   {                                                                                                                                         \
      if( cb ## name )                                                                                                                       \
         return returnType( cb ## name argNames );                                                                                           \
      if( engineAPI::gUseConsoleInterop )                                                                                                    \
      {                                                                                                                                      \
         static StringTableEntry sName = StringTable->insert( #name );                                                                       \
         _EngineConsoleCallbackHelper cbh( sName, NULL );                                                                                    \
         return returnType( cbh.call< returnType > argNames );                                                                               \
      }                                                                                                                                      \
      return returnType();                                                                                                                   \
   }                                                                                                                                         \
   namespace {                                                                                                                               \
      ConsoleFunctionHeader _ ## name ## header(                                                                                             \
         #returnType, #args, "" );                                                                                                           \
      ConsoleConstructor _ ## name ## obj( NULL, #name, usageString, &_ ## name ## header );                                                 \
   }
   
   
// Again, temporary macros to allow splicing the API while we still have the console interop around.

#define IMPLEMENT_CONSOLE_CALLBACK( class, name, returnType, args, argNames, usageString )                                                   \
   returnType class::name ## _callback args                                                                                                  \
   {                                                                                                                                         \
      if( engineAPI::gUseConsoleInterop )                                                                                                    \
      {                                                                                                                                      \
         static StringTableEntry sName = StringTable->insert( #name );                                                                       \
         _EngineConsoleCallbackHelper cbh( sName, this );                                                                                    \
         return returnType( cbh.call< returnType > argNames );                                                                               \
      }                                                                                                                                      \
      return returnType();                                                                                                                   \
   }                                                                                                                                         \
   namespace {                                                                                                                               \
      ConsoleFunctionHeader _ ## class ## name ## header(                                                                                    \
         #returnType, #args, "" );                                                                                                           \
      ConsoleConstructor _ ## class ## name ## obj( #class, #name, usageString, &_ ## class ## name ## header );                             \
   }
   
#define IMPLEMENT_NEW_CALLBACK( class, name, returnType, args, argNames, usageString )                                                       \
   struct _ ## class ## name ## frame { typedef class ObjectType; };                                                                         \
   TORQUE_API _EngineMethodTrampoline< _ ## class ## name ## frame, returnType args >::FunctionType* cb ## class ## _ ## name;               \
   TORQUE_API void set_cb ## class ## _ ## name(                                                                                             \
      _EngineMethodTrampoline< _ ## class ## name ## frame, returnType args >::FunctionType fn )                                             \
      { cb ## class ## _ ## name = fn; }                                                                                                     \
   _EngineMethodTrampoline< _ ## class ## name ## frame, returnType args >::FunctionType* cb ## class ## _ ## name;                          \
   namespace {                                                                                                                               \
      ::EngineFunctionInfo _cb ## class ## name(                                                                                             \
         #name,                                                                                                                              \
         &::_SCOPE< class >()(),                                                                                                             \
         usageString,                                                                                                                        \
         "virtual " #returnType " " #name #args,                                                                                             \
         "cb" #class "_" #name,                                                                                                              \
         ::TYPE< _EngineMethodTrampoline< _ ## class ## name ## frame, returnType args >::FunctionType >(),                                  \
         NULL,                                                                                                                               \
         &cb ## class ## _ ## name,                                                                                                          \
         EngineFunctionCallout                                                                                                               \
      );                                                                                                                                     \
   }                                                                                                                                         \
   returnType class::name ## _callback args                                                                                                  \
   {                                                                                                                                         \
      if( cb ## class ## _ ## name ) {                                                                                                       \
         _EngineCallbackHelper cbh( this, reinterpret_cast< const void* >( cb ## class ## _ ## name ) );                                     \
         return returnType( cbh.call< returnType > argNames );                                                                               \
      }                                                                                                                                      \
      return returnType();                                                                                                                   \
   }


// Internal helper class for doing call-outs in the new interop.
struct _EngineCallbackHelper
{
   protected:
    
      EngineObject* mThis;
      const void* mFn;
            
   public:

      _EngineCallbackHelper( EngineObject* pThis, const void* fn )
         : mThis( pThis ),
           mFn( fn ) {}
      
      template< typename R >
      R call() const
      {
         typedef R( FunctionType )( EngineObject* );
         return R( reinterpret_cast< FunctionType* >( mFn )( mThis ) );
      }
      template< typename R, typename A >
      R call( A a ) const
      {
         typedef R( FunctionType )( EngineObject*, A );
         return R( reinterpret_cast< FunctionType* >( mFn )( mThis, a ) );
      }
      template< typename R, typename A, typename B >
      R call( A a, B b ) const
      {
         typedef R( FunctionType )( EngineObject*, A, B );
         return R( reinterpret_cast< FunctionType* >( mFn )( mThis, a, b ) );
      }
      template< typename R, typename A, typename B, typename C >
      R call( A a, B b, C c ) const
      {
         typedef R( FunctionType )( EngineObject*, A, B, C );
         return R( reinterpret_cast< FunctionType* >( mFn )( mThis, a, b, c ) );
      }
      template< typename R, typename A, typename B, typename C, typename D >
      R call( A a, B b, C c, D d ) const
      {
         typedef R( FunctionType )( EngineObject*, A, B, C, D );
         return R( reinterpret_cast< FunctionType* >( mFn )( mThis, a, b, c, d ) );
      }
      template< typename R, typename A, typename B, typename C, typename D, typename E >
      R call( A a, B b, C c, D d, E e ) const
      {
         typedef R( FunctionType )( EngineObject*, A, B, C, D, E );
         return R( reinterpret_cast< FunctionType* >( mFn )( mThis, a, b, c, d, e ) );
      }
      template< typename R, typename A, typename B, typename C, typename D, typename E, typename F >
      R call( A a, B b, C c, D d, E e, F f ) const
      {
         typedef R( FunctionType )( EngineObject*, A, B, C, D, E, F );
         return R( reinterpret_cast< FunctionType* >( mFn )( mThis, a, b, c, d, e, f ) );
      }
      template< typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G >
      R call( A a, B b, C c, D d, E e, F f, G g ) const
      {
         typedef R( FunctionType )( EngineObject*, A, B, C, D, E, F, G );
         return R( reinterpret_cast< FunctionType* >( mFn )( mThis, a, b, c, d, e, f, g ) );
      }
      template< typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H >
      R call( A a, B b, C c, D d, E e, F f, G g, H h ) const
      {
         typedef R( FunctionType )( EngineObject*, A, B, C, D, E, F, G, H );
         return R( reinterpret_cast< FunctionType* >( mFn )( mThis, a, b, c, d, e, f, g, h ) );
      }
      template< typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I >
      R call( A a, B b, C c, D d, E e, F f, G g, H h, I i ) const
      {
         typedef R( FunctionType )( EngineObject*, A, B, C, D, E, F, G, H, I );
         return R( reinterpret_cast< FunctionType* >( mFn )( mThis, a, b, c, d, e, f, g, h, i ) );
      }
      template< typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J >
      R call( A a, B b, C c, D d, E e, F f, G g, H h, I i, J j ) const
      {
         typedef R( FunctionType )( EngineObject*, A, B, C, D, E, F, G, H, I, J );
         return R( reinterpret_cast< FunctionType* >( mFn )( mThis, a, b, c, d, e, f, g, h, i, j ) );
      }
      template< typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K >
      R call( A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k ) const
      {
         typedef R( FunctionType )( EngineObject*, A, B, C, D, E, F, G, H, I, J, K );
         return R( reinterpret_cast< FunctionType* >( mFn )( mThis, a, b, c, d, e, f, g, h, i, j, k ) );
      }
      template< typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L >
      R call( A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l ) const
      {
         typedef R( FunctionType )( EngineObject*, A, B, C, D, E, F, G, H, I, J, K, L l );
         return R( reinterpret_cast< FunctionType* >( mFn )( mThis, a, b, c, d, e, f, g, h, i, j, k, l ) );
      }
};

// Internal helper for callback support in legacy console system.
struct _EngineConsoleCallbackHelper
{

   protected:
   
      /// Matches up to storeArgs.
      static const U32 MAX_ARGUMENTS = 11;

      SimObject* mThis;
      S32 mArgc;
      const char* mArgv[ MAX_ARGUMENTS + 2 ];
      
      const char* _exec()
      {
         if( mThis )
         {
            // Cannot invoke callback until object has been registered
            return mThis->isProperlyAdded() ? Con::execute( mThis, mArgc, mArgv ) : "";
         }
         else
            return Con::execute( mArgc, mArgv );
      }
      
   public:

      _EngineConsoleCallbackHelper( StringTableEntry callbackName, SimObject* pThis )
         : mThis( pThis ),
           mArgc( pThis ? 2 : 1 )
      {
         mArgv[ 0 ] = callbackName;
      }
      
      template< typename R >
      R call()
      {
         return R( EngineUnmarshallData< R >()( _exec() ) );
      }
      template< typename R, typename A >
      R call( A a )
      {
         EngineMarshallData( a, mArgc, mArgv );
         return R( EngineUnmarshallData< R >()( _exec() ) );
      }
      template< typename R, typename A, typename B >
      R call( A a, B b )
      {
         EngineMarshallData( a, mArgc, mArgv );
         EngineMarshallData( b, mArgc, mArgv );
         return R( EngineUnmarshallData< R >()( _exec() ) );
      }
      template< typename R, typename A, typename B, typename C >
      R call( A a, B b, C c )
      {
         EngineMarshallData( a, mArgc, mArgv );
         EngineMarshallData( b, mArgc, mArgv );
         EngineMarshallData( c, mArgc, mArgv );
         return R( EngineUnmarshallData< R >()( _exec() ) );
      }
      template< typename R, typename A, typename B, typename C, typename D >
      R call( A a, B b, C c, D d )
      {
         EngineMarshallData( a, mArgc, mArgv );
         EngineMarshallData( b, mArgc, mArgv );
         EngineMarshallData( c, mArgc, mArgv );
         EngineMarshallData( d, mArgc, mArgv );
         return R( EngineUnmarshallData< R >()( _exec() ) );
      }
      template< typename R, typename A, typename B, typename C, typename D, typename E >
      R call( A a, B b, C c, D d, E e )
      {
         EngineMarshallData( a, mArgc, mArgv );
         EngineMarshallData( b, mArgc, mArgv );
         EngineMarshallData( c, mArgc, mArgv );
         EngineMarshallData( d, mArgc, mArgv );
         EngineMarshallData( e, mArgc, mArgv );
         return R( EngineUnmarshallData< R >()( _exec() ) );
      }
      template< typename R, typename A, typename B, typename C, typename D, typename E, typename F >
      R call( A a, B b, C c, D d, E e, F f )
      {
         EngineMarshallData( a, mArgc, mArgv );
         EngineMarshallData( b, mArgc, mArgv );
         EngineMarshallData( c, mArgc, mArgv );
         EngineMarshallData( d, mArgc, mArgv );
         EngineMarshallData( e, mArgc, mArgv );
         EngineMarshallData( f, mArgc, mArgv );
         return R( EngineUnmarshallData< R >()( _exec() ) );
      }
      template< typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G >
      R call( A a, B b, C c, D d, E e, F f, G g )
      {
         EngineMarshallData( a, mArgc, mArgv );
         EngineMarshallData( b, mArgc, mArgv );
         EngineMarshallData( c, mArgc, mArgv );
         EngineMarshallData( d, mArgc, mArgv );
         EngineMarshallData( e, mArgc, mArgv );
         EngineMarshallData( f, mArgc, mArgv );
         EngineMarshallData( g, mArgc, mArgv );
         return R( EngineUnmarshallData< R >()( _exec() ) );
      }
      template< typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H >
      R call( A a, B b, C c, D d, E e, F f, G g, H h )
      {
         EngineMarshallData( a, mArgc, mArgv );
         EngineMarshallData( b, mArgc, mArgv );
         EngineMarshallData( c, mArgc, mArgv );
         EngineMarshallData( d, mArgc, mArgv );
         EngineMarshallData( e, mArgc, mArgv );
         EngineMarshallData( f, mArgc, mArgv );
         EngineMarshallData( g, mArgc, mArgv );
         EngineMarshallData( h, mArgc, mArgv );
         return R( EngineUnmarshallData< R >()( _exec() ) );
      }
      template< typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I >
      R call( A a, B b, C c, D d, E e, F f, G g, H h, I i )
      {
         EngineMarshallData( a, mArgc, mArgv );
         EngineMarshallData( b, mArgc, mArgv );
         EngineMarshallData( c, mArgc, mArgv );
         EngineMarshallData( d, mArgc, mArgv );
         EngineMarshallData( e, mArgc, mArgv );
         EngineMarshallData( f, mArgc, mArgv );
         EngineMarshallData( g, mArgc, mArgv );
         EngineMarshallData( h, mArgc, mArgv );
         EngineMarshallData( i, mArgc, mArgv );
         return R( EngineUnmarshallData< R >()( _exec() ) );
      }
      template< typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J >
      R call( A a, B b, C c, D d, E e, F f, G g, H h, I i, J j )
      {
         EngineMarshallData( a, mArgc, mArgv );
         EngineMarshallData( b, mArgc, mArgv );
         EngineMarshallData( c, mArgc, mArgv );
         EngineMarshallData( d, mArgc, mArgv );
         EngineMarshallData( e, mArgc, mArgv );
         EngineMarshallData( f, mArgc, mArgv );
         EngineMarshallData( g, mArgc, mArgv );
         EngineMarshallData( h, mArgc, mArgv );
         EngineMarshallData( i, mArgc, mArgv );
         EngineMarshallData( j, mArgc, mArgv );
         return R( EngineUnmarshallData< R >()( _exec() ) );
      }
      template< typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K >
      R call( A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k )
      {
         EngineMarshallData( a, mArgc, mArgv );
         EngineMarshallData( b, mArgc, mArgv );
         EngineMarshallData( c, mArgc, mArgv );
         EngineMarshallData( d, mArgc, mArgv );
         EngineMarshallData( e, mArgc, mArgv );
         EngineMarshallData( f, mArgc, mArgv );
         EngineMarshallData( g, mArgc, mArgv );
         EngineMarshallData( h, mArgc, mArgv );
         EngineMarshallData( i, mArgc, mArgv );
         EngineMarshallData( j, mArgc, mArgv );
         EngineMarshallData( k, mArgc, mArgv );
         return R( EngineUnmarshallData< R >()( _exec() ) );
      }
      template< typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L >
      R call( A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l )
      {
         EngineMarshallData( a, mArgc, mArgv );
         EngineMarshallData( b, mArgc, mArgv );
         EngineMarshallData( c, mArgc, mArgv );
         EngineMarshallData( d, mArgc, mArgv );
         EngineMarshallData( e, mArgc, mArgv );
         EngineMarshallData( f, mArgc, mArgv );
         EngineMarshallData( g, mArgc, mArgv );
         EngineMarshallData( h, mArgc, mArgv );
         EngineMarshallData( i, mArgc, mArgv );
         EngineMarshallData( j, mArgc, mArgv );
         EngineMarshallData( k, mArgc, mArgv );
         EngineMarshallData( l, mArgc, mArgv );
         return R( EngineUnmarshallData< R >()( _exec() ) );
      }
};


// Re-enable some VC warnings we disabled for this file.
#pragma warning( default : 4510 )
#pragma warning( default : 4610 )

#endif // !_ENGINEAPI_H_
