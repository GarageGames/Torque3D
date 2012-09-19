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

#ifndef _ENGINEOBJECT_H_
#define _ENGINEOBJECT_H_

#ifndef _ENGINETYPES_H_
   #include "console/engineTypes.h"
#endif
#ifndef _REFBASE_H_
   #include "core/util/refBase.h"
#endif

/// Disable TMM for this file.
#include "platform/tmm_off.h"


/// @file
/// This file contains the framework for defining class types for interfacing
/// through engine API.


class EngineObject;


/// Assign the current class and all its subclasses to the given export scope.
/// May be overridden by subclasses.
///
/// @code
/// class MyClass : public EngineObject
/// {
///     DECLARE_CLASS( MyClass, EngineObject );
///     DECLARE_INSCOPE( MyAPI );
/// };
/// @endcode
#define DECLARE_INSCOPE( name )                                                        \
   typedef name __DeclScope;
   
/// Declare that this class and all its subclasses cannot be instantiated through
/// the API, i.e. objects of these classes can only be created inside the engine.
///
/// @code
/// class MyClass : public EngineObject
/// {
///     DECLARE_CLASS( MyClass, EngineObject );
///     DECLARE_NONINSTANTIABLE;
/// };
/// @endcode
#define DECLARE_NONINSTANTIABLE                                                       \
   typedef ::FalseType __IsInstantiableType;
      
/// Declare that this class and all its subclasses can be instantiated through the
/// API using their respective create() functions.
///
/// May be overridden by subclasses.
///
/// @code
/// class MyClass : public EngineObject
/// {
///     DECLARE_CLASS( MyClass, EngineObject );
///     DECLARE_INSTANTIABLE;
/// };
/// @endcode
#define DECLARE_INSTANTIABLE                                                           \
   typedef ::TrueType __IsInstantiableType;

/// Declare a class for which the object's may be forcefully disposed of by the engine.
/// In essense, this means that such objects can only be weak-referenced from within the
/// control layer as their lifetime is ultimately controlled by the engine.  This is
/// used for all resource type objects that are owned by their devices.
///
/// An alternative to disposable types would be to shield the control layer from direct
/// access to these objects and rather expose only intermediary objects.  However, at some
/// point this just gets crazy.  The control layer wraps around engine objects that wrap
/// around internal objects.  The internal object goes away, invalidates the native wrapper
/// which invalidates the managed wrapper.
///
/// By exposing the control layer directly to in-engine resources using disposable objects,
/// intermediary steps and code are made unnecessary while the control layer is still made
/// aware of the fact that ownership of these objects ultimately and firmly belongs with
/// the engine.
///
/// One important guarantee for disposable types is that an object will <em>not</em> go
/// away <em>unless</em> the control layer calls into the engine.  This means that these
/// objects will not magically disappear while managed code is running.
///
/// @note This macro automatically redefines destroySelf().
/// @warn Reference-counting is still used for disposable types!!  This means that if the
///   reference-count drops to zero, the object will be deleted as any other object.
///
/// @see IMPLEMENT_DISPOSABLE
#define DECLARE_DISPOSABLE                                                             \
   protected:                                                                          \
      typedef ::TrueType __IsDisposableType;                                           \
      DECLARE_CALLBACK( void, onDispose, () );                                         \
   public:                                                                             \
      virtual void destroySelf()                                                       \
      {                                                                                \
         onDispose_callback();                                                         \
         SuperType::destroySelf();                                                     \
      }

/// Matching implement for DECLARE_DISPOSABLE.
///
/// @param type The disposable C++ class type.
#define IMPLEMENT_DISPOSABLE( type )                                                   \
   IMPLEMENT_NEW_CALLBACK( type, onDispose, void, (), (),                              \
      "Called before the instance is disposed." );
      
/// Declare that the current class (and any of its descendents) can only have a single instance.
///
/// @code
/// class MySingletonClass : public EngineObject
/// {
///     DECLARE_CLASS( MySingletonClass, EngineObject );
///     DECLARE_SINGLETON;
/// };
/// @endcode
///
/// @note At the moment, DECLARE_SINGLETON disallows the use of custom create methods
///   on the same class.
#define DECLARE_SINGLETON                                                              \
   private:                                                                            \
      static ThisType* _smInstance;                                                    \
      struct _clearInstance;                                                           \
      friend struct _clearInstance;                                                    \
      struct _clearInstance {                                                          \
         ~_clearInstance() { _smInstance = NULL; }                                     \
      } _clearInstanceInst;                                                            \
   public:                                                                             \
      static ThisType* createSingleton();                                              \
      static void destroySingleton();                                                  \
   protected:                                                                          \
      typedef ::TrueType __IsSingletonType;                                            \
      DEFINE_CREATE_METHOD                                                             \
      {                                                                                \
         return createSingleton();                                                     \
      }                                                                                \
   public:                                                                             \
      static ThisType* instance()                                                      \
      {                                                                                \
         return _smInstance;                                                           \
      }

/// Matching implement for DECLARE_SINGLETON.
/// @param type The C++ class type.
#define IMPLEMENT_SINGLETON( type )                                                    \
   type::ThisType* type::_smInstance;                                                  \
   type::ThisType* type::createSingleton()                                             \
   {                                                                                   \
      if( _smInstance != NULL )                                                        \
      {                                                                                \
         Con::errorf( "%s::create - Singleton instance already created",               \
            TYPE< ThisType >()->getFullyQualifiedExportName().c_str() );               \
         return NULL;                                                                  \
      }                                                                                \
      _smInstance = new ThisType();                                                    \
      _smInstance->incRefCount();                                                      \
      return _smInstance;                                                              \
   }                                                                                   \
   void type::destroySingleton()                                                       \
   {                                                                                   \
      if( !_smInstance )                                                               \
         return;                                                                       \
      if( ::IsTrueType< __IsDisposableType >() )                                       \
         _smInstance->destroySelf();                                                   \
      else                                                                             \
         _smInstance->decRefCount();                                                   \
   }                                                                                   \
   DefineNewEngineStaticMethod( type, getInstance, type*, (),,                         \
      "Get the singleton " #type " instance.\n\n"                                      \
      "@return The " #type " singleton." )                                             \
   {                                                                                   \
      return type::instance();                                                         \
   }

/// Declare a static class, i.e. a class which only acts as an export scope.  Static
/// classes cannot be instantiated, neither through the API nor inside the engine (at
/// least in a form that would be usable within the API).
///
/// @code
/// class MyFunctions
/// {
///     DECLARE_STATIC_CLASS( MyFunctions );
/// };
///
/// IMPLEMENT_STATIC_CLASS( MyFunctions,, "doc" );
/// 
/// DefineStaticEngineMethod( MyFunctions, doSomething, void, (),, "" )
/// {
///    // ...
/// }
/// @endcode
///
/// @param type The C++ class type.
///
/// @see IMPLEMENT_STATIC_CLASS
#define DECLARE_STATIC_CLASS( type )                                                   \
   private:                                                                            \
      static EngineExportScope __engineExportScopeInst;                                \
      static EngineExportScope& __engineExportScope()                                  \
         { return __engineExportScopeInst; }                                           \
   public:                                                                             \
      template< typename T > friend struct ::_SCOPE;                                   \
      
/// Matching implement for DECLARE_STATIC_CLASS.
///
/// @param type The C++ type of the static class.  Also used as export name.
/// @param scope Export scope to place the static class in.
/// @param doc Documentation string.
#define IMPLEMENT_STATIC_CLASS( type, scope, doc )                                     \
   EngineExportScope type::__engineExportScopeInst( #type, &_SCOPE< scope >()(), doc );

/// Declare a concrete class @a type derived from the class @a super.  Whether the
/// class can be instantiated by the control layer depends on its instantiability.
///
/// @code
/// class MyClass : public EngineObject
/// {
///     DECLARE_CLASS( MyClass, EngineObject );
/// };
/// @endcode
///
/// @param type C++ class type.
/// @param super C++ class type of the superclass.  May be void only for EngineObject.
///
/// @see IMPLEMENT_CLASS
/// @see IMPLEMENT_NONINSTANTIABLE_CLASS
#define DECLARE_CLASS( type, super )                                                   \
   public:                                                                             \
      typedef type ThisType;                                                           \
      typedef super SuperType;                                                         \
      template< typename T > friend struct ::_EngineTypeTraits;                        \
      template< typename T > friend struct ::_SCOPE;                                   \
      template< typename T > friend T* _CREATE();                                      \
      template< typename T, typename Base > friend class ::EngineClassTypeInfo;        \
   private:                                                                            \
      typedef ::_Private::_ConcreteClassBase< ThisType > _ClassBase;                   \
      static EngineClassTypeInfo< ThisType, _ClassBase > _smTypeInfo;                  \
      static EngineExportScope& __engineExportScope();                                 \
      static EnginePropertyTable& _smPropertyTable;                                    \
      virtual const EngineTypeInfo* __typeinfo() const;                                \
   public:
   
/// Declare an abstract class @a type derived from the class @a super.
///
/// @code
/// class MyClass : public EngineObject
/// {
///     DECLARE_ABSTRACT_CLASS( MyClass, EngineObject );
/// };
/// @endcode
///
/// @param type C++ class type.
/// @param super C++ class type of the superclass.  May be void only for EngineObject.
///
/// @see IMPLEMENT_NONINSTANTIABLE_CLASS
#define DECLARE_ABSTRACT_CLASS( type, super )                                          \
   public:                                                                             \
      typedef type ThisType;                                                           \
      typedef super SuperType;                                                         \
      template< typename T > friend struct ::_EngineTypeTraits;                        \
      template< typename T > friend struct ::_SCOPE;                                   \
      template< typename T > friend T* _CREATE();                                      \
      template< typename T, typename Base > friend class ::EngineClassTypeInfo;        \
   private:                                                                            \
      typedef ::_Private::_AbstractClassBase< ThisType > _ClassBase;                   \
      static EngineClassTypeInfo< ThisType, _ClassBase > _smTypeInfo;                  \
      static EngineExportScope& __engineExportScope();                                 \
      static EnginePropertyTable& _smPropertyTable;                                    \
      virtual const EngineTypeInfo* __typeinfo() const;                                \
   public:

/// Matching implement for DECLARE_ABSTRACT_CLASS or DECLARE_CLASS for classes
/// that are not instantiable through the API.
///
/// @code
/// class MyClass : public EngineObject
/// {
///     DECLARE_CLASS( MyClass, EngineObject );
///     DECLARE_INSCOPE( MyScope );
///     DECLARE_NONINSTANTIABLE;
/// };
///
/// IMPLEMENT_NONINSTANTIABLE_CLASS( MyClass, "My class." )
///    PROPERTY( myProperty, 1, "My property.", EnginePropertyTransient )
/// END_IMPLEMENT_CLASS;
/// @endcode
///
/// @note If you want the export name to be different to the actual class
///   name, use a typedef.
///
/// @param type The C++ class type.
/// @doc Documentation string.
#define IMPLEMENT_NONINSTANTIABLE_CLASS( type, doc )                                   \
   DEFINE_CALLIN( fn ## type ## _staticGetType, staticGetType, type, const EngineTypeInfo*, (),,, \
      "Get the type info object for the " #type " class.\n\n"                          \
      "@return The type info object for " #type )                                      \
   {                                                                                   \
      return ::TYPE< type >();                                                         \
   }                                                                                   \
   EngineClassTypeInfo< type::ThisType, type::_ClassBase >                             \
      type::_smTypeInfo( #type, &_SCOPE< __DeclScope >()(), doc );                     \
   EngineExportScope& type::__engineExportScope() { return type::_smTypeInfo; }        \
   const EngineTypeInfo* type::__typeinfo() const { return &_smTypeInfo; }             \
   namespace { namespace _ ## type {                                                   \
      extern EnginePropertyTable _propertyTable;                                       \
   } }                                                                                 \
   EnginePropertyTable& type::_smPropertyTable = _ ## type::_propertyTable;            \
   namespace { namespace _ ## type {                                                   \
      EnginePropertyTable::Property _properties[] = {

/// Matching implement to DECLARE_CLASS for classes that are instantiable
/// through the API.
///
/// @note engineFunctions.h must be included for this macro to work.
///
/// @param type The C++ class type.
/// @param doc Documentation string.
#define IMPLEMENT_CLASS( type, doc )                                                   \
   DEFINE_CALLIN( fn ## type ## _create, create, type, type*, (),,,                    \
      "Create a new " #type " instance.\n"                                             \
      "@return A new " #type " instance with a reference count of 1." )                \
   {                                                                                   \
      return ::_CREATE< type >();                                                      \
   }                                                                                   \
   IMPLEMENT_NONINSTANTIABLE_CLASS( type, doc )
   
/// Close an IMPLEMENT_CLASS or IMPLEMENT_NONINSTANTIABLE_CLASS block.
#define END_IMPLEMENT_CLASS                                                            \
         { NULL }                                                                      \
      };                                                                               \
      EnginePropertyTable _propertyTable                                               \
         ( sizeof( _properties ) / sizeof( _properties[ 0 ] ) - 1, _properties );      \
   } }
   
/// Define a property on the current class.
///
/// A property named XXX must have a corresponding "getXXX" and/or "setXXX" accessor
/// method defined on the class.  If there is only a "setXXX" method, the property is
/// write-only.  If there is only a "getXXX", the property is read-only.  If both are
/// defined, the property is read-write.
///
/// If the accessors are static methods, the property is a static property.  Otherwise
/// it is an instance property.
///
/// The type of the property is determined by its accessor methods.
///
/// A getXXX method must take no arguments and return a value.  A setXXX method must take
/// one argument and return void.
///
/// If the property is indexed (@a numElements != 1), the get and set methods take an
/// additional first argument which is the integer index (type S32).  Additionally, the get method
/// must be called "getXXXElement" and the set method must be called "setXXXElement".
///
/// Indexed properties may be either fixed-size or variable-sized.  Fixed-size indexed
/// properties have numElements count > 1.  Variable-size indexed properties have a
/// numElements count of 0.  Variable-sized indexed properties must have an additional
/// method "getXXXCount" that returns the current count of elements for the given property.
///
/// @code
/// IMPLEMENT_CLASS( MyClass, "My class." )
///    PROPERTY( myProperty, 1, "My property.", EnginePropertyTransient )
/// END_IMPLEMENT_CLASS;
/// @endcode
///
/// @note Case is ignored when matching get and set methods to property definitions.
///
/// @param name The name of the property.  Must correspond with the get and/or set methods.
/// @param numElements If this is a fixed size array property, this is the number of elements
///   the fixed array has.  Otherwise 1.
/// @param doc Documentation string.
/// @param flags A combination of EnginePropertyFlags or simply 0 if not applicable.
///
/// @see EnginePropertyFlags
#define PROPERTY( name, numElements, doc, flags )                                      \
   { #name, doc, numElements, flags },
   
///
#define PROPERTY_GROUP( name, numElements, doc )                                       \
   { #name, doc, numElements, EnginePropertyGroupBegin },

///
#define END_PROPERTY_GROUP                                                             \
   { NULL, NULL, 0, EnginePropertyGroupEnd },

/// Define a custom create function for this class and its subclasses.  Create
/// functions are used to create new instances of class types.  This code will be
/// called by the automatically generated "createXXX" functions exported through
/// the API.
///
/// The type of class being created is available to the code as the type parameter "T"
/// which is guaranteed to be a subtype of the current class type.
///
/// @code
/// class MyClass : public EngineObject
/// {
///        DECLARE_CLASS( MyClass, EngineObject );
///     protected:
///        DEFINE_CREATE_METHOD
///        {
///           T* object = new T();
///           object->incRefCount();
///           object->_registerObject();
///           return object;
///        }
///        virtual void _registerObject();
/// };
/// @endcode
///
/// @note A create method must return an object with a reference count of 1.
#define DEFINE_CREATE_METHOD                                                           \
      template< typename T >                                                           \
      static T* __create()

/// Define a method that calls back into the control layer.
///
/// @see IMPLEMENT_CALLBACK
#define DECLARE_CALLBACK( returnType, name, args )  \
   virtual returnType name ## _callback args


// Our backdoor into calling the __create method on any class even if it is
// protected or private.  This function is automatically made friends of any
// EngineObject class.  Should be used except by internal API code.
template< typename T >
inline T* _CREATE()
{
   return T::template __create< T >();
}


/// Interface for object memory allocation.
class IEngineObjectPool
{
   public:
   
      /// Allocate a new object memory block of the given size.
      /// @return Pointer to a new memory block or NULL on failure.
      virtual void* allocateObject( U32 size TORQUE_TMM_ARGS_DECL ) = 0;
      
      /// Return the member for the object at the given address to the
      /// allocator for reuse.
      /// @param ptr Pointer to an object memory block previously allocated with allocateObject().
      virtual void freeObject( void* ptr ) = 0;
      
      /// Instance of the object pool to use by default.
      static IEngineObjectPool* DEFAULT;
};


/// Singleton class that uses the C runtime memory routines for allocating objects.
class EngineCRuntimeObjectPool : public IEngineObjectPool
{
   public:
   
      typedef IEngineObjectPool Parent;
      
   protected:
   
      static EngineCRuntimeObjectPool smInstance;
      
   public:

      /// Return the singleton instance of this pool.
      static EngineCRuntimeObjectPool* instance() { return &smInstance; }
      
      // IEngineObjectPool
      virtual void* allocateObject( U32 size TORQUE_TMM_ARGS_DECL );
      virtual void freeObject( void* ptr );
};


/// Base class for all objects that may be passed to the control layer.
///
/// A set of rules applies to all EngineObject-derived classes:
///
/// - Every EngineObject class must have a default constructor.
/// - The default constructor and the destructor of every EngineObject class must be public.
/// - If an EngineObject class inherits from multiple classes, the class leading back to EngineObject
///   must be the @b first class in the list to ensure binary-compatible class layouts.
/// - EngineObjects are cooperatively reference-counted by both the engine as well as the control
///   layer.
class EngineObject : public StrongRefBase
{
   public:
   
      DECLARE_ABSTRACT_CLASS( EngineObject, void );
      DECLARE_INSCOPE( _GLOBALSCOPE );
      DECLARE_INSTANTIABLE;
   
      friend const EngineTypeInfo* TYPEOF( const EngineObject* ); // __typeinfo
      friend void*& _USERDATA( EngineObject* ); // mEngineObjectUserData
      friend class StaticEngineObject; // mEngineObjectPool
      
   protected:
   
      typedef ::FalseType __IsDisposableType;
      typedef ::FalseType __IsSingletonType;

      DEFINE_CREATE_METHOD
      {
         T* object = new T;
         object->incRefCount();
         return object; 
      }

      /// Subclasses should overload this method instead of the public destroySelf().
      virtual void _destroySelf() {}
      
      ///
      static void* _allocateObject( size_t size, IEngineObjectPool* pool TORQUE_TMM_ARGS_DECL );
            
   public:
      
      EngineObject();
      virtual ~EngineObject();

      /// Return a string that describes this instance.  Meant primarily for debugging.
      virtual String describeSelf() const;
                  
      #ifndef TORQUE_DISABLE_MEMORY_MANAGER
      // Make sure no matter what, we get the new/delete calls.
      void* operator new( size_t size );
      void* operator new( size_t size, IEngineObjectPool* pool );
      #endif
      
      /// Allocate a new object in the default object pool.
      /// @param size Size of the object in bytes.
      /// @return Memory block for new object; never NULL.
      void* operator new( size_t size TORQUE_TMM_ARGS_DECL );
      
      /// Allocate a new object in the given object pool.
      ///
      /// If the given pool's allocateObject returns NULL, the method will fall back
      /// to the default pool.
      ///
      /// @param size Size of the object in bytes.
      /// @param pool Object pool to allocate the object in.
      /// @return Memory block for the new object; never NULL.
      void* operator new( size_t size, IEngineObjectPool* pool TORQUE_TMM_ARGS_DECL );
      
      /// Placement new.
      void* operator new( size_t size, void* ptr ) { return ptr; }
            
      /// Release the given object's memory in the pool it has been allocated from.
      void operator delete( void* ptr );

      /// Return the pool of EngineObjects to which this object belongs.
      IEngineObjectPool* getEngineObjectPool() const { return mEngineObjectPool; }
      
      // StrongRefBase
      virtual void destroySelf();
            
#ifdef TORQUE_DEBUG
      
      /// @name Instance Tracking (debugging only)
      ///
      /// In debug builds, all EngineObjects are kept on a global list so that it is easy
      /// to enumerate all live objects at any time.
      ///
      /// @note This is @b NOT thread-safe.
      /// @{
      
      /// Type of callback function for iterating over EngineObject instances.
      typedef void ( *DebugEnumInstancesCallback )( EngineObject* );

      /// Dump describeSelf()s of all live ConsoleObjects to the console.
      static void debugDumpInstances();

      /// Call the given callback for all instances of the given type.
      /// Callback may also be NULL in which case the method just iterates
      /// over all instances of the given type.  This is useful for setting
      /// a breakpoint during debugging.
      static void debugEnumInstances( const std::type_info& type, DebugEnumInstancesCallback callback );
      
      /// Same as above but uses an export class name and also includes
      /// inheritance (i.e. enumerates all instances of the given class and
      /// its subclasses).
      static void debugEnumInstances( const char* className, DebugEnumInstancesCallback callback );

   private:
   
      /// Next object in global link chain of engine objects.
      /// @note Debug builds only.
      EngineObject* mNextEngineObject;
      
      /// Previous object in global link chain of engine objects.
      /// @note Debug builds only.
      EngineObject* mPrevEngineObject;

      /// Total number of engine objects currently instantiated.
      /// @note Debug builds only.
      static U32 smNumEngineObjects;
      
      /// First object in the global link chain of engine objects.
      /// @note Debug builds only.
      static EngineObject* smFirstEngineObject;
      
      /// @}
      
#endif

   private:
         
      /// Object pool to which this object belongs.  If this is NULL,
      /// the object will not deallocate itself when it is destructed.
      /// This is useful for inline allocation of objects.
      IEngineObjectPool* mEngineObjectPool;
      
      /// Opaque user data pointer that the control layer may install
      /// on any engine object.  Most importantly, this allows control layers
      /// to very easily keep track of EngineObjects that they have already
      /// created their own wrapper objects for.
      void* mEngineObjectUserData;

      // Disable array new/delete operators.
      void* operator new[]( size_t );
      void operator delete[]( void* );
};


/// A statically allocated engine object.
///
/// Static objects have an implicit initial reference count of one and will not
/// delete themselves even when asked to do so.
class StaticEngineObject : public EngineObject
{
   public:

      DECLARE_ABSTRACT_CLASS( StaticEngineObject, EngineObject );
      DECLARE_NONINSTANTIABLE;
      
      StaticEngineObject();
      
      // EngineObject.
      virtual void destroySelf();
};


typedef StrongRefPtr< EngineObject > EngineObjectRef;
typedef WeakRefPtr< EngineObject > EngineObjectWeakRef;


/// Return the type info object for the dynamic type of the given object.
/// @param object An EngineObject or NULL.
/// @return An EngineTypeInfo instance or NULL if @a object is NULL.
inline const EngineTypeInfo* TYPEOF( const EngineObject* object )
{
   if( !object )
      return NULL;
   return object->__typeinfo();
}


#include "platform/tmm_on.h"
#endif // !_ENGINEOBJECT_H_
