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

#include <tuple>
#include <utility>

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

// Needed for the executef macros. Blame GCC.
#ifndef _SIMEVENTS_H_
#include "console/simEvents.h"
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
#pragma warning( push )
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
inline void EngineMarshallData( const T& arg, S32& argc, ConsoleValueRef *argv )
{
   argv[ argc ] = castConsoleTypeToString( arg );
   argc ++;
}
inline void EngineMarshallData( bool arg, S32& argc, ConsoleValueRef *argv )
{
   if( arg )
      argv[ argc ] = 1;
   else
      argv[ argc ] = 0;
   argc ++;
}
inline void EngineMarshallData( S32 arg, S32& argc, ConsoleValueRef *argv )
{
   argv[ argc ] = arg;
   argc ++;
}
inline void EngineMarshallData( U32 arg, S32& argc, ConsoleValueRef *argv )
{
   EngineMarshallData( S32( arg ), argc, argv );
}
inline void EngineMarshallData( F32 arg, S32& argc, ConsoleValueRef *argv )
{
   argv[ argc ] = arg;
   argc ++;
}
inline void EngineMarshallData( const char* arg, S32& argc, ConsoleValueRef *argv )
{
   argv[ argc ] = arg;
   argc ++;
}
inline void EngineMarshallData( char* arg, S32& argc, ConsoleValueRef *argv )
{
   argv[ argc ] = arg;
   argc ++;
}

template< typename T >
inline void EngineMarshallData( T* object, S32& argc, ConsoleValueRef *argv )
{
   argv[ argc ] = object ? object->getId() : 0;
   argc ++;
}
template< typename T >
inline void EngineMarshallData( const T* object, S32& argc, ConsoleValueRef *argv )
{
   argv[ argc ] = object ? object->getId() : 0;
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
   S32 operator()( ConsoleValueRef &ref ) const
   {
      return (S32)ref;
   }

   S32 operator()( const char* str ) const
   {
      return dAtoi( str );
   }
};
template<>
struct EngineUnmarshallData< U32 >
{
   U32 operator()( ConsoleValueRef &ref ) const
   {
      return (U32)((S32)ref);
   }

   U32 operator()( const char* str ) const
   {
      return dAtoui( str );
   }
};
template<>
struct EngineUnmarshallData< F32 >
{
   F32 operator()( ConsoleValueRef &ref ) const
   {
      return (F32)ref;
   }

   F32 operator()( const char* str ) const
   {
      return dAtof( str );
   }
};
template<>
struct EngineUnmarshallData< U8 >
{
   U8 operator()( ConsoleValueRef &ref ) const
   {
      return (U8)((S32)ref);
   }

   U8 operator()( const char* str ) const
   {
      return dAtoui( str );
   }
};
template<>
struct EngineUnmarshallData< const char* >
{
   const char* operator()( ConsoleValueRef &ref ) const
   {
      return ref.getStringValue();
   }

   const char* operator()( const char* str ) const
   {
      return str;
   }
};
template< typename T >
struct EngineUnmarshallData< T* >
{
   T* operator()( ConsoleValueRef &ref ) const
   {
      return dynamic_cast< T* >( Sim::findObject( ref.getStringValue() ) );
   }

   T* operator()( const char* str ) const
   {
      return dynamic_cast< T* >( Sim::findObject( str ) );
   }
};
template<>
struct EngineUnmarshallData< void >
{
   void operator()( ConsoleValueRef& ) const {}
   void operator()( const char* ) const {}
};


template<>
struct EngineUnmarshallData< ConsoleValueRef >
{
   ConsoleValueRef operator()( ConsoleValueRef ref ) const
   {
      return ref;
   }
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


template<typename T> struct _EngineTrampoline {
   struct Args {};
};

template< typename R, typename ...ArgTs >
struct _EngineTrampoline< R( ArgTs ... ) >
{
   typedef std::tuple<ArgTs ...> Args;
   std::tuple<ArgTs ...> argT;
};

template< typename T >
struct _EngineFunctionTrampolineBase : public _EngineTrampoline< T >
{
   typedef T FunctionType;
};

// Trampolines for any call-ins that aren't methods.
template< typename T >
struct _EngineFunctionTrampoline {};

template< typename R, typename ...ArgTs >
struct _EngineFunctionTrampoline< R(ArgTs...) > : public _EngineFunctionTrampolineBase< R(ArgTs...) >
{
private:
   using Super = _EngineFunctionTrampolineBase< R(ArgTs...) >;
   using ArgsType = typename Super::Args;
   
   template<size_t ...> struct Seq {};
   template<size_t N, size_t ...S> struct Gens : Gens<N-1, N-1, S...> {};
   template<size_t ...I> struct Gens<0, I...>{ typedef Seq<I...> type; };
   
   template<size_t ...I>
   static R dispatchHelper(typename Super::FunctionType fn, const ArgsType& args, Seq<I...>)  {
      return R( fn(std::get<I>(args) ...) );
   }

   using SeqType = typename Gens<sizeof...(ArgTs)>::type;
public:
   static R jmp(typename Super::FunctionType fn, const ArgsType& args )
   {
      return dispatchHelper(fn, args, SeqType());
   }
};

// Trampolines for engine methods

template< typename T >
struct _EngineMethodTrampolineBase : public _EngineTrampoline< T > {};

template< typename Frame, typename T >
struct _EngineMethodTrampoline {};

template< typename Frame, typename R, typename ...ArgTs >
struct _EngineMethodTrampoline< Frame, R(ArgTs ...) > : public _EngineMethodTrampolineBase< R(ArgTs ...) >
{
   using FunctionType = R( typename Frame::ObjectType*, ArgTs ...);
private:
   using Super = _EngineMethodTrampolineBase< R(ArgTs ...) >;
   using ArgsType = typename _EngineFunctionTrampolineBase< R(ArgTs ...) >::Args;
   
   template<size_t ...> struct Seq {};
   template<size_t N, size_t ...S> struct Gens : Gens<N-1, N-1, S...> {};
   template<size_t ...I> struct Gens<0, I...>{ typedef Seq<I...> type; };
   
   template<size_t ...I>
   static R dispatchHelper(Frame f, const ArgsType& args, Seq<I...>)  {
      return R( f._exec(std::get<I>(args) ...) );
   }
   
   using SeqType = typename Gens<sizeof...(ArgTs)>::type;
public:
   static R jmp( typename Frame::ObjectType* object, const ArgsType& args )
   {
      
      Frame f;
      f.object = object;
      return dispatchHelper(f, args, SeqType());
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

   template<typename ...ArgTs> U32 operator()(ArgTs... args){
      return sizeof...(ArgTs);
   }
   
   operator U32() const{ // FIXME: WHAT IS THIS?? I'm pretty sure it's incorrect, and it's the version that is invoked by all the macros
      return 0;
   }
};




// Encapsulation of a legacy console function invocation.
namespace engineAPI{
   namespace detail{
      template<S32 startArgc, typename R, typename ...ArgTs>
      struct ThunkHelpers {
         using SelfType = ThunkHelpers<startArgc, R, ArgTs...>;
         using FunctionType = R(*)(ArgTs...);
         template<typename Frame> using MethodType = R(Frame::*)(ArgTs ...) const;
         template<size_t I> using IthArgType = typename std::tuple_element<I, std::tuple<ArgTs ...> >::type;
         
         template<size_t ...> struct Seq {};
         template<size_t N, size_t ...S> struct Gens : Gens<N-1, N-1, S...> {};
         template<size_t ...I> struct Gens<0, I...>{ typedef Seq<I...> type; };
         
         typedef typename _EngineConsoleThunkType< R >::ReturnType ReturnType;
         static const S32 NUM_ARGS = sizeof...(ArgTs) + startArgc;
         
         template<size_t index, size_t method_offset = 0, typename ...RealArgTs>
         static IthArgType<index> getRealArgValue(S32 argc, ConsoleValueRef *argv, const _EngineFunctionDefaultArguments< void(RealArgTs...) >& defaultArgs)
         {
            if((startArgc + index) < argc)
            {
               return EngineUnmarshallData< IthArgType<index> >()( argv[ startArgc + index ] );
            } else {
               return std::get<index + method_offset>(defaultArgs.mArgs);
            }
         }
         
         template<size_t ...I>
         static R dispatchHelper(S32 argc, ConsoleValueRef *argv, FunctionType fn, const _EngineFunctionDefaultArguments< void(ArgTs...) >& defaultArgs, Seq<I...>){
            return fn(SelfType::getRealArgValue<I>(argc, argv, defaultArgs) ...);
         }
         
         template<typename Frame, size_t ...I>
         static R dispatchHelper(S32 argc, ConsoleValueRef *argv, MethodType<Frame> fn, Frame* frame, const _EngineFunctionDefaultArguments< void( typename Frame::ObjectType*, ArgTs...) >& defaultArgs, Seq<I...>){
            return (frame->*fn)(SelfType::getRealArgValue<I,1>(argc, argv, defaultArgs) ...);
         }
         
         using SeqType = typename Gens<sizeof...(ArgTs)>::type;
      };
      
      template<typename ArgVT> struct MarshallHelpers {
         template<typename ...ArgTs> static void marshallEach(S32 &argc, ArgVT *argv, const ArgTs& ...args){}
         template<typename H, typename ...Tail> static void marshallEach(S32 &argc, ArgVT *argv, const H& head, const Tail& ...tail){
            argv[argc++] = EngineMarshallData(head);
            marshallEach(argc, argv, tail...);
         }
      };
      
      template<> struct MarshallHelpers<ConsoleValueRef> {
         template<typename ...ArgTs> static void marshallEach(S32 &argc, ConsoleValueRef *argv, const ArgTs& ...args){}
         template<typename H, typename ...Tail> static void marshallEach(S32 &argc, ConsoleValueRef *argv, const H& head, const Tail& ...tail){
            EngineMarshallData(head, argc, argv);
            marshallEach(argc, argv, tail...);
         }
      };
   }
}

template< S32 startArgc, typename T >
struct _EngineConsoleThunk {};

template< S32 startArgc, typename R, typename ...ArgTs >
struct _EngineConsoleThunk< startArgc, R(ArgTs...) >
{
private:
   using Helper = engineAPI::detail::ThunkHelpers<startArgc, R, ArgTs...>;
   using SeqType = typename Helper::SeqType;
public:
   typedef typename Helper::FunctionType FunctionType;
   typedef typename Helper::ReturnType ReturnType;
   template<typename Frame> using MethodType = typename Helper::template MethodType<Frame>;
   static const S32 NUM_ARGS = Helper::NUM_ARGS;
   
   static ReturnType thunk( S32 argc, ConsoleValueRef *argv, FunctionType fn, const _EngineFunctionDefaultArguments< void(ArgTs...) >& defaultArgs)
   {
      return _EngineConsoleThunkReturnValue( Helper::dispatchHelper(argc, argv, fn, defaultArgs, SeqType()));
   }
   template< typename Frame >
   static ReturnType thunk( S32 argc, ConsoleValueRef *argv, MethodType<Frame> fn, Frame* frame, const _EngineFunctionDefaultArguments< void( typename Frame::ObjectType*, ArgTs...) >& defaultArgs)
   {
      return _EngineConsoleThunkReturnValue( Helper::dispatchHelper(argc, argv, fn, frame, defaultArgs, SeqType()));
   }
};

// Have to do a partial specialization for void-returning functions :(
template<S32 startArgc, typename ...ArgTs>
struct _EngineConsoleThunk<startArgc, void(ArgTs...)> {
private:
   using Helper = engineAPI::detail::ThunkHelpers<startArgc, void, ArgTs...>;
   using SeqType = typename Helper::SeqType;
public:
   typedef typename Helper::FunctionType FunctionType;
   typedef typename Helper::ReturnType ReturnType;
   template<typename Frame> using MethodType = typename Helper::template MethodType<Frame>;
   static const S32 NUM_ARGS = Helper::NUM_ARGS;
   
   static void thunk( S32 argc, ConsoleValueRef *argv, FunctionType fn, const _EngineFunctionDefaultArguments< void(ArgTs...) >& defaultArgs)
   {
      Helper::dispatchHelper(argc, argv, fn, defaultArgs, SeqType());
   }
   template< typename Frame >
   static void thunk( S32 argc, ConsoleValueRef *argv, MethodType<Frame> fn, Frame* frame, const _EngineFunctionDefaultArguments< void( typename Frame::ObjectType*, ArgTs...) >& defaultArgs)
   {
      Helper::dispatchHelper(argc, argv, fn, frame, defaultArgs, SeqType());
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
   static _EngineConsoleThunkType< returnType >::ReturnType _ ## name ## caster( SimObject*, S32 argc, ConsoleValueRef *argv )       \
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
   static _EngineConsoleThunkType< returnType >::ReturnType _ ## className ## name ## caster( SimObject* object, S32 argc, ConsoleValueRef *argv )  \
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
   static _EngineConsoleThunkType< returnType >::ReturnType _ ## className ## name ## caster( SimObject*, S32 argc, ConsoleValueRef *argv )\
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
   static _EngineConsoleThunkType< returnType >::ReturnType _ ## name ## caster( SimObject*, S32 argc, ConsoleValueRef *argv )       \
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
   static _EngineConsoleThunkType< returnType >::ReturnType _ ## className ## name ## caster( SimObject* object, S32 argc, ConsoleValueRef *argv )  \
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
   static _EngineConsoleThunkType< returnType >::ReturnType _ ## className ## name ## caster( SimObject*, S32 argc, ConsoleValueRef *argv )\
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
      
      template< typename R, typename ...ArgTs >
      R call(ArgTs ...args) const
      {
         typedef R( FunctionType )( EngineObject*, ArgTs... );
         return R( reinterpret_cast< FunctionType* >( const_cast<void*>(mFn) )( mThis, args... ) );
      }

};


#include "console/stringStack.h"

// Internal helper for callback support in legacy console system.
struct _BaseEngineConsoleCallbackHelper
{
public:

   /// Matches up to storeArgs.
   static const U32 MAX_ARGUMENTS = 11;

   SimObject* mThis;
   S32 mInitialArgc;
   S32 mArgc;
   StringTableEntry mCallbackName;
   ConsoleValueRef mArgv[ MAX_ARGUMENTS + 2 ];

   ConsoleValueRef _exec();
   ConsoleValueRef _execLater(SimConsoleThreadExecEvent *evt);

   _BaseEngineConsoleCallbackHelper() {;}
};



// Base helper for console callbacks
struct _EngineConsoleCallbackHelper : public _BaseEngineConsoleCallbackHelper
{
private:
   using Helper = engineAPI::detail::MarshallHelpers<ConsoleValueRef>;
public:

   _EngineConsoleCallbackHelper( StringTableEntry callbackName, SimObject* pThis )
   {
      mThis = pThis;
      mArgc = mInitialArgc = pThis ? 2 : 1 ;
      mCallbackName = callbackName;
   }
   
   template< typename R, typename ...ArgTs >
   R call(ArgTs ...args)
   {
      if (Con::isMainThread())
      {
         ConsoleStackFrameSaver sav; sav.save();
         CSTK.reserveValues(mArgc + sizeof...(ArgTs), mArgv);
         mArgv[ 0 ].value->setStackStringValue(mCallbackName);
        
        Helper::marshallEach(mArgc, mArgv, args...);
        
         return R( EngineUnmarshallData< R >()( _exec() ) );
      }
      else
      {
         SimConsoleThreadExecCallback cb;
         SimConsoleThreadExecEvent *evt = new SimConsoleThreadExecEvent(mArgc + sizeof...(ArgTs), NULL, false, &cb);
         evt->populateArgs(mArgv);
         mArgv[ 0 ].value->setStackStringValue(mCallbackName);
        
        Helper::marshallEach(mArgc, mArgv, args...);
        
         Sim::postEvent((SimObject*)Sim::getRootGroup(), evt, Sim::getCurrentTime());

         return R( EngineUnmarshallData< R >()( cb.waitForResult() ) );
      }
   }
   
};


// Override for when first parameter is presumably a SimObject*, in which case A will be absorbed as the callback
template<typename P1> struct _EngineConsoleExecCallbackHelper : public _BaseEngineConsoleCallbackHelper
{
private:
   using Helper = engineAPI::detail::MarshallHelpers<ConsoleValueRef>;
public:

   _EngineConsoleExecCallbackHelper( SimObject* pThis )
   {
      mThis = pThis;
      mArgc = mInitialArgc = 2;
      mCallbackName = NULL;
   }

   
   template< typename R, typename SCB, typename ...ArgTs >
   R call( SCB simCB , ArgTs ...args )
   {
      if (Con::isMainThread())
      {
         ConsoleStackFrameSaver sav; sav.save();
         CSTK.reserveValues(mArgc+sizeof...(ArgTs), mArgv);
         mArgv[ 0 ].value->setStackStringValue(simCB);

        Helper::marshallEach(mArgc, mArgv, args...);

         return R( EngineUnmarshallData< R >()( _exec() ) );
      }
      else
      {
         SimConsoleThreadExecCallback cb;
         SimConsoleThreadExecEvent *evt = new SimConsoleThreadExecEvent(mArgc+sizeof...(ArgTs), NULL, true, &cb);
         evt->populateArgs(mArgv);
         mArgv[ 0 ].value->setStackStringValue(simCB);
        
        Helper::marshallEach(mArgc, mArgv, args...);

         Sim::postEvent(mThis, evt, Sim::getCurrentTime());

         return R( EngineUnmarshallData< R >()( cb.waitForResult() ) );
      }
   }
};

// Override for when first parameter is const char*
template<> struct _EngineConsoleExecCallbackHelper<const char*> : public _BaseEngineConsoleCallbackHelper
{
private:
   using Helper = engineAPI::detail::MarshallHelpers<ConsoleValueRef>;
public:
   _EngineConsoleExecCallbackHelper( const char *callbackName )
   {
      mThis = NULL;
      mArgc = mInitialArgc = 1;
      mCallbackName = StringTable->insert(callbackName);
   }

   template< typename R, typename ...ArgTs >
   R call(ArgTs ...args)
   {
      if (Con::isMainThread())
      {
         ConsoleStackFrameSaver sav; sav.save();
         CSTK.reserveValues(mArgc+sizeof...(ArgTs), mArgv);
         mArgv[ 0 ].value->setStackStringValue(mCallbackName);
        
        Helper::marshallEach(mArgc, mArgv, args...);
        
         return R( EngineUnmarshallData< R >()( _exec() ) );
      }
      else
      {
         SimConsoleThreadExecCallback cb;
         SimConsoleThreadExecEvent *evt = new SimConsoleThreadExecEvent(mArgc+sizeof...(ArgTs), NULL, false, &cb);
         evt->populateArgs(mArgv);
         mArgv[ 0 ].value->setStackStringValue(mCallbackName);
        
        Helper::marshallEach(mArgc, mArgv, args...);

         Sim::postEvent((SimObject*)Sim::getRootGroup(), evt, Sim::getCurrentTime());
         return R( EngineUnmarshallData< R >()( cb.waitForResult() ) );
      }
   }   
};

// Re-enable some VC warnings we disabled for this file.
#pragma warning( pop ) // 4510 and 4610

#endif // !_ENGINEAPI_H_
