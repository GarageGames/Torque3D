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

#include <tuple>

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

template<typename ...ArgTs>
struct _EngineFunctionDefaultArguments< void(ArgTs...) > : public EngineFunctionDefaultArguments
{
   template<typename T> using DefVST = typename EngineTypeTraits<T>::DefaultArgumentValueStoreType;
   std::tuple<DefVST<ArgTs>  ...> mArgs;
private:
   using SelfType = _EngineFunctionDefaultArguments< void(ArgTs...) >;
   
   template<size_t ...> struct Seq {};
   template<size_t N, size_t ...S> struct Gens : Gens<N-1, N-1, S...> {};
   
   template<size_t ...I> struct Gens<0, I...>{ typedef Seq<I...> type; };
   
   template<typename ...TailTs, size_t ...I>
   static void copyHelper(std::tuple<DefVST<ArgTs> ...> &args, std::tuple<DefVST<TailTs> ...> &defaultArgs, Seq<I...>)  {
      std::tie(std::get<I + (sizeof...(ArgTs) - sizeof...(TailTs))>(args)...) = defaultArgs;
   }
   
   template<typename ...TailTs> using MaybeSelfEnabled = typename std::enable_if<sizeof...(TailTs) <= sizeof...(ArgTs), decltype(mArgs)>::type;
   
   template<typename ...TailTs> static MaybeSelfEnabled<TailTs...> tailInit(TailTs ...tail) {
      std::tuple<DefVST<ArgTs>...> argsT;
      std::tuple<DefVST<TailTs>...> tailT = std::make_tuple(tail...);
      SelfType::copyHelper<TailTs...>(argsT, tailT, typename Gens<sizeof...(TailTs)>::type());
      return argsT;
   };
   
public:
   template<typename ...TailTs> _EngineFunctionDefaultArguments(TailTs ...tail)
   : EngineFunctionDefaultArguments({sizeof...(TailTs)}), mArgs(SelfType::tailInit(tail...))
   {}
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
