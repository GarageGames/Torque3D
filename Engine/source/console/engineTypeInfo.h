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

#ifndef _ENGINETYPEINFO_H_
#define _ENGINETYPEINFO_H_

#ifndef _ENGINEEXPORTS_H_
   #include "console/engineExports.h"
#endif


class EngineTypeInfo;


/// Kinding for engine types.  Engine types are segregated into kinds which
/// are to types what types are to values, i.e. a value is an instance of a type
/// and a type is an instance of a kind.
enum EngineTypeKind
{
   EngineTypeKindPrimitive,      ///< Any kind of atomic data.  Passed by value.
   EngineTypeKindEnum,           ///< Enumeration.  Passed by value.
   EngineTypeKindBitfield,       ///< Bitfield.  Passed by value.
   EngineTypeKindFunction,       ///< Function pointer.
   EngineTypeKindStruct,         ///< Structured value.  Passed by reference.
   EngineTypeKindClass           ///< Pointer to opaque EngineObject.
};

DECLARE_ENUM_R( EngineTypeKind );

/// Flags for an EngineTypeInfo.
enum EngineTypeFlags
{
   EngineTypeAbstract      = BIT( 0 ),    ///< Type is abstract.
   EngineTypeInstantiable  = BIT( 1 ),    ///< Type can be instantiated through API.
   EngineTypeDisposable    = BIT( 2 ),    ///< Instances can be disposed by the engine.
   EngineTypeSingleton     = BIT( 3 ),    ///< Class type with only a single instance.
   EngineTypeVariadic      = BIT( 4 ),    ///< Variadic function type.
};



/// Table of values for an enumeration or bitfield type.
class EngineEnumTable
{
   public:
   
      /// A value in an enumeration.
      ///
      /// The order of the fields in this structure is important as it is meant to be
      /// initialized with { ... } in code.
      struct Value
      {
         /// Integer value.  If the enumeration is a bit field,
         /// this is the bit value.
         S32 mInt;

         /// Name of the value.
         const char* mName;
         
         /// Documentation string.
         const char* mDocString;
         
         /// Return the name of this enum value.
         const char* getName() const { return mName; }
         
         /// Return the documentation string of this enum value.
         const char* getDocString() const { return mDocString; }
         
         /// Return the integer value of this enum value.
         S32 getInt() const { return mInt; }
                  
         operator S32() const
         {
            return getInt();
         }
      };
      
   protected:
   
      /// Number of values in this enumeration.
      U32 mNumValues;
      
      /// Records for all the enum values.
      const Value* mValues;
      
   public:
   
      ///
      EngineEnumTable( U32 numValues, const Value* values )
         :  mNumValues( numValues ),
            mValues( values ) {}
      
      /// Return the number of Values in this enumeration/bitfield.
      U32 getNumValues() const { return mNumValues; }
      
      /// Get the enum value at the given index.
      const Value& operator []( U32 index ) const
      {
         AssertFatal( index < getNumValues(), "" );
         return mValues[ index ];
      }
};


/// Table of fields for a struct type.
class EngineFieldTable
{
   public:
   
      /// A field descriptor in a field table.
      struct Field
      {
         /// Name of the field or group.
         const char* mName;

         /// Documentation string.
         const char* mDocString;

         /// Indexed size of this field.  Must be >=1.
         U32 mNumElements;

         /// Type of the field.
         const EngineTypeInfo* mType;
         
         /// Offset of the field in instances.
         U32 mOffset;

         ///
         const char* getName() const { return mName; }
         
         ///
         const char* getDocString() const { return mDocString; }
         
         ///
         U32 getNumElements() const { return mNumElements; }
         
         ///
         const EngineTypeInfo* getType() const { return mType; }
         
         ///
         U32 getOffset() const { return mOffset; }
      };   
      
   protected:
   
      /// Number of fields in this table.
      U32 mNumFields;
      
      ///
      const Field* mFields;

   public:
   
      /// Construct a field table from a NULL-terminated array of Field
      /// records.
      EngineFieldTable( const Field* fields )
         : mNumFields( 0 ),
           mFields( fields )
      {
         while( fields[ mNumFields ].getName() )
            mNumFields ++;
      }
   
      ///
      EngineFieldTable( U32 numFields, const Field* fields )
         :  mNumFields( numFields ),
            mFields( fields ) {}
      
      ///
      U32 getNumFields() const { return mNumFields; }
      
      ///
      const Field& operator []( U32 index ) const
      {
         AssertFatal( index <= getNumFields(), "EngineFieldTable - index out of range" );
         return mFields[ index ];
      }
};


/// Flags for property descriptors.
enum EnginePropertyFlags
{
   EnginePropertyTransient             = BIT( 0 ),       ///< Exclude from serializations.
   EnginePropertyConstant              = BIT( 1 ),       ///< Property value is constant once object has been constructed.
   EnginePropertyHideInInspectors      = BIT( 2 ),       ///< Don't make the property visible in property sheets in the editor.
   EnginePropertyGroupBegin            = BIT( 3 ),       ///< Special property to mark the beginning of a group; does not define a real property on the object.
   EnginePropertyGroupEnd              = BIT( 4 ),       ///< Special property to mark the end of a group; does not define a real property on the object.
};

///
///
///
/// - Read-only properties only have a getXXX and no setXXX method.
/// - Static properties (value shared by all instances) don't take a 'this' parameter.
/// - 
class EnginePropertyTable
{
   public:
   
      struct Property
      {
         /// Name of the property.
         const char* mName;
         
         /// Doc string using Javadoc markup.
         const char* mDocString;
         
         /// Indexed size of the property.  If 0, the property array is variable-sized.  If 1, the property
         /// is not indexed.  If >1, the property is a fixed-size array.
         U32 mNumElements;

         /// Combination of EnginePropertyFlags.
         U32 mFlags;
         
         /// Return the name of the property.
         const char* getName() const { return mName; }
         
         /// Return the number of indexed elements of the property.
         U32 getNumElements() const { return mNumElements; }
         
         /// Return the documentation string for this property.
         const char* getDocString() const { return mDocString; }
                           
         /// Test whether the property has a constant value.
         bool isConstant() const { return ( mFlags & EnginePropertyConstant ); }
         
         /// Test whether the property value is transient, i.e. should not be serialized.
         bool isTransient() const { return ( mFlags & EnginePropertyTransient ); }
         
         /// Test whether this property begins a group of properties.
         bool isGroupBegin() const { return ( mFlags & EnginePropertyGroupBegin ); }
         
         /// Test whether this property ends a group of properties.
         bool isGroupEnd() const { return ( mFlags & EnginePropertyGroupEnd ); }
         
         ///
         bool hideInInspectors() const { return ( mFlags & EnginePropertyHideInInspectors ); }
      };
      
   protected:
   
      /// Number of properties in this table.
      U32 mNumProperties;
      
      /// Array of property definitions.
      const Property* mProperties;
      
   public:
   
      ///
      EnginePropertyTable( U32 numProperties, const Property* properties )
         :  mNumProperties( numProperties ),
            mProperties( properties ) {}
      
      ///
      U32 getNumProperties() const { return mNumProperties; }
      
      ///
      const Property& operator []( U32 index ) const
      {
         AssertFatal( index <= getNumProperties(), "EnginePropertyTable - index out of range" );
         return mProperties[ index ];
      }
};


/// Information about the return and argument types of a function type.
class EngineArgumentTypeTable
{
   protected:
   
      /// Return type of the function type.
      const EngineTypeInfo* mReturnType;
      
      /// Number of argument types of the function type.
      U32 mNumArguments;
      
      /// Array of argument types of the function type.
      const EngineTypeInfo* const* mArgumentTypes;
      
   public:
      
      ///
      EngineArgumentTypeTable( const EngineTypeInfo* returnType,
                               U32 numArguments,
                               const EngineTypeInfo* const* argumentTypes )
         :  mReturnType( returnType ),
            mNumArguments( numArguments ),
            mArgumentTypes( argumentTypes ) {}
            
      /// Return the return type of the function type.
      const EngineTypeInfo* getReturnType() const { return mReturnType; }
      
      /// Return the number of argument types of the function type.
      U32 getNumArguments() const { return mNumArguments; }
      
      /// Get the argument type at the given index.
      const EngineTypeInfo* operator []( U32 index ) const
      {
         AssertFatal( index <= getNumArguments(), "EngineArgumentTypeTable - Index out of range!" );
         return mArgumentTypes[ index ];
      }
};


/// Networking related information for an engine API type.
struct EngineTypeNetInfo
{   
   S32 mNetGroupMask;
   S32 mNetType;
   S32 mNetEventDir;

   #ifdef TORQUE_NET_STATS
   struct NetStatInstance
   {
   };
   #endif
   
   EngineTypeNetInfo()
      : mNetGroupMask( 0 ),
        mNetType( 0 ),
        mNetEventDir( 0 ) {}
};


/// Information about an engine type.
///
/// This class is used to store run-time type information about engine types.
///
/// Once created, type info objects must persist for the entire duration the engine
/// is running.
///
/// All types are implicitly export scopes and may thus contain other exports
/// within them.
class EngineTypeInfo : public EngineExportScope
{
   public:
   
      DECLARE_CLASS( EngineTypeInfo, EngineExportScope );
      
      // While we still have the old ConsoleObject system around, allow
      // them to retroactively install property tables.  Will be removed
      // when the console interop is removed and all classes are migrated
      // to the new system.
      template< typename T > friend class ConcreteAbstractClassRep;
      
   protected:
      
      /// Kind of type.
      EngineTypeKind mTypeKind;

      /// Size of an instance of this type.
      U32 mInstanceSize;
      
      /// Combination of EngineTypeFlags.
      BitSet32 mTypeFlags;
                              
      /// If this is an enumeration or bitfield type, this is the pointer to the enum table.
      const EngineEnumTable* mEnumTable;
      
      /// If this is a struct type, this is the pointer to the field table.
      const EngineFieldTable* mFieldTable;
      
      /// If this is a class type, this is the pointer to the property table.
      const EnginePropertyTable* mPropertyTable;
      
      /// If this is a function type, this is the pointer to the argument type table.
      const EngineArgumentTypeTable* mArgumentTypeTable;
      
      /// Pointer to type info object for engine type that this type subtypes from.  NULL if none.
      const EngineTypeInfo* mSuperType;
      
      /// Networking related information for this type.
      mutable EngineTypeNetInfo mNetInfo;
      
      /// Next type in the global link chain.
      const EngineTypeInfo* mNext;
      
      /// Total number of defined types.
      static U32 smNumTypes;
      
      /// First type in the global link chain of type info instances.
      static const EngineTypeInfo* smFirst;

      ///
      EngineTypeInfo( const char* typeName, EngineExportScope* scope, EngineTypeKind kind, U32 instanceSize, const char* docString );

   public:

      /// @name List Interface
      /// Interface for accessing/traversing the list of types.
      /// @{

      /// Return the first type in the global link chain of types.
      static const EngineTypeInfo* getFirstType() { return smFirst; }

      /// Return the next type in the global link chain of types.
      const EngineTypeInfo* getNextType() const
      {
         return mNext;
      }

      /// @}

      /// Get the type info instance for the given type.
      /// @param typeName Name of a registered engine type.
      /// @return Type info instance for @a typeName or NULL if no such type exists.
      static const EngineTypeInfo* getTypeInfoByName( const char* typeName );
            
      /// Return the name of the type.
      /// @return The name of the type or an empty string if this is an anonymous type.
      const char* getTypeName() const { return getExportName(); }

      /// Return the kind this type.
      EngineTypeKind getTypeKind() const { return mTypeKind; }
      
      /// Return the type info object of the engine type that this type subtypes from.
      const EngineTypeInfo* getSuperType() const { return mSuperType; }

      /// Return the size of a single value in bytes.
      /// Be aware that the value size refers to the value as it is passed around.  For types using
      /// reference or pointer value semantics, this is thus the size of a pointer or reference and
      /// not the size of the actual instance.
      U32 getValueSize() const;
      
      /// Return the
      U32 getInstanceSize() const { return mInstanceSize; }
      
      /// Return true if the type is abstract.
      /// @note Only class and function types can be abstract.
      bool isAbstract() const { return mTypeFlags.test( EngineTypeAbstract ); }
      
      /// Return true if the type can be instantiated from outside the engine.
      bool isInstantiable() const { return mTypeFlags.test( EngineTypeInstantiable ); }
      
      /// Return true if the objects of this type can be disposed by the engine.
      bool isDisposable() const { return mTypeFlags.test( EngineTypeDisposable ); }
      
      /// Return true if the type can have only a single instance.
      bool isSingleton() const { return mTypeFlags.test( EngineTypeSingleton ); }
      
      /// Return true if the type is a variadic function type.
      bool isVariadic() const { return mTypeFlags.test( EngineTypeVariadic ); }
                                    
      /// Test whether this type is a primitive type.
      bool isPrimitive() const { return ( getTypeKind() == EngineTypeKindPrimitive ); }
      
      /// Test whether this type is an enumeration type.
      bool isEnum() const { return ( getTypeKind() == EngineTypeKindEnum ); }
      
      /// Test whether this type is a bitfield type.
      bool isBitfield() const { return ( getTypeKind() == EngineTypeKindBitfield ); }
      
      /// Test whether this type is a function type.
      bool isFunction() const { return ( getTypeKind() == EngineTypeKindFunction ); }
      
      /// Test whether this type is a struct type.
      bool isStruct() const { return ( getTypeKind() == EngineTypeKindStruct ); }

      /// Test whether this is a class type.
      bool isClass() const { return ( getTypeKind() == EngineTypeKindClass ); }
            
      /// Return the EngineEnumTable for this type (only for enumeration and bitfield types).
      const EngineEnumTable* getEnumTable() const { return mEnumTable; }
      
      /// Return the EngineFieldTable for this type (only for struct types).
      const EngineFieldTable* getFieldTable() const { return mFieldTable; }
      
      /// Return the EnginePropertyTable for this type (only for class types).
      const EnginePropertyTable* getPropertyTable() const { return mPropertyTable; }
      
      ///
      const EngineArgumentTypeTable* getArgumentTypeTable() const { return mArgumentTypeTable; }
      
      /// Return true if this type is a subtype of the given type.
      bool isSubtypeOf( const EngineTypeInfo* type ) const;
      
      ///
      EngineTypeNetInfo& getNetInfo() const { return mNetInfo; }
      
      /// @name Instancing
      /// @{
      
      /// Create a new instance at the given address.
      /// @pre Must not be called for abstract types.
      virtual bool constructInstance( void* ptr ) const;
      
      /// Destroy the instance at the given address.
      /// @pre Must not be called for abstract types.
      virtual void destructInstance( void* ptr ) const;
      
      /// @}
};

//--------------------------------------------------------------------------
//    Type Info Helper Classes.
//--------------------------------------------------------------------------


/// Template for type infos of primitive, enum, and bitfield types.
template< typename T >
class EngineSimpleTypeInfo : public EngineTypeInfo
{
   public:
   
      typedef EngineTypeInfo Parent;
      
      EngineSimpleTypeInfo( const char* name, EngineExportScope* scope, EngineTypeKind kind, const char* docString, EngineEnumTable* enumTable = NULL )
         : Parent( name, scope, kind, sizeof( T ), docString )
      {
         mEnumTable = enumTable;
         mTypeFlags.set( EngineTypeInstantiable );
      }
      
      virtual bool constructInstance( void* ptr ) const
      {
         T* p = reinterpret_cast< T* >( ptr );
         *p = T();
         return true;
      }
      
      virtual void destructInstance( void* ptr ) const
      {
         // Nothing to do.
      }
};


/// Template for struct type infos.
template< typename T >
class EngineStructTypeInfo : public EngineTypeInfo
{
   public:
   
      typedef EngineTypeInfo Parent;
      
      EngineStructTypeInfo( const char* name, EngineExportScope* scope, const char* docString, EngineFieldTable* fieldTable )
         : Parent( name, scope, EngineTypeKindStruct, sizeof( T ), docString )
      {
         mFieldTable = fieldTable;
         mTypeFlags.set( EngineTypeInstantiable );
      }
      
      virtual bool constructInstance( void* ptr ) const
      {
         T* p = reinterpret_cast< T* >( ptr );
         *p = T();
         return true;
      }
   
      virtual void destructInstance( void* ptr ) const
      {
         T* p = reinterpret_cast< T* >( ptr );
         destructInPlace( p );
      }
};


/// Template for class type infos.
template< typename T, typename Base >
class EngineClassTypeInfo : public EngineTypeInfo
{
   public:
   
      typedef EngineTypeInfo Parent;
      
      /// The documentation string set by CLASSDOC (if any).
      static const char* smDocString;

      EngineClassTypeInfo( const char* name, EngineExportScope* scope, const char* docString = NULL )
         : Parent( name, scope, EngineTypeKindClass, sizeof( T ), docString ? docString : smDocString )
      {
         mPropertyTable = &T::_smPropertyTable;
         mSuperType = TYPE< typename T::SuperType >();
         if( IsTrueType< typename Base::IsAbstractType >() )
            mTypeFlags.set( EngineTypeAbstract );
         else if( IsTrueType< typename T::__IsInstantiableType >() )
            mTypeFlags.set( EngineTypeInstantiable );
            
         if( IsTrueType< typename T::__IsDisposableType >() )
            mTypeFlags.set( EngineTypeDisposable );
         if( IsTrueType< typename T::__IsSingletonType >() )
            mTypeFlags.set( EngineTypeSingleton );
      }
      
      virtual bool constructInstance( void* ptr ) const
      {
         return Base::_construct( ptr );
      }
   
      virtual void destructInstance( void* ptr ) const
      {
         return Base::_destruct( ptr );
      }
};

template< typename T, typename Base > const char* EngineClassTypeInfo< T, Base >::smDocString;


/// Template for function type infos.
template< typename T >
class EngineFunctionTypeInfo : public EngineTypeInfo
{
   public:
   
      typedef EngineTypeInfo Parent;
      
      static _EngineArgumentTypeTable< T > ARGTYPES;
      
      EngineFunctionTypeInfo()
         : Parent( "", &_SCOPE<>()(), EngineTypeKindFunction, sizeof( T* ), "" )
      {
         mArgumentTypeTable = &ARGTYPES;
         
         if( ARGTYPES.VARIADIC )
            mTypeFlags.set( EngineTypeVariadic );
         
         // Function types cannot be instantiated.
         mTypeFlags.set( EngineTypeAbstract );
      }      
};

template< typename T > _EngineArgumentTypeTable< T > EngineFunctionTypeInfo< T >::ARGTYPES;
template< typename T > const EngineFunctionTypeInfo< T > _EngineFunctionTypeTraits< T >::smTYPEINFO;


//--------------------------------------------------------------------------
//    Function Argument Type Infos.
//--------------------------------------------------------------------------

template< typename R >
struct _EngineArgumentTypeTable< R() > : public EngineArgumentTypeTable
{
   static const U32 NUM_ARGUMENTS = 0;
   static const bool VARIADIC = false;
   static const EngineTypeInfo* const RETURN;
#ifdef TORQUE_COMPILER_GCC
   static const EngineTypeInfo* const ARGS[ 0 ];
#else
   static const EngineTypeInfo* const ARGS[ 1 ];
#endif

   _EngineArgumentTypeTable()
      : EngineArgumentTypeTable( TYPE< typename EngineTypeTraits< R >::Type >(), NUM_ARGUMENTS, ARGS ) {}
};
template< typename R > const EngineTypeInfo* const _EngineArgumentTypeTable< R() >::RETURN = TYPE< typename EngineTypeTraits< R >::Type >();
#ifdef TORQUE_COMPILER_GCC
template< typename R > const EngineTypeInfo* const _EngineArgumentTypeTable< R() >::ARGS[ 0 ] = {};
#else
template< typename R > const EngineTypeInfo* const _EngineArgumentTypeTable< R() >::ARGS[ 1 ] = {};
#endif
template< typename R >
struct _EngineArgumentTypeTable< R( ... ) > : public _EngineArgumentTypeTable< R() >
{
   static const bool VARIADIC = true;
   _EngineArgumentTypeTable() {}
};

template< typename R, typename A >
struct _EngineArgumentTypeTable< R( A ) > : public EngineArgumentTypeTable
{
   static const U32 NUM_ARGUMENTS = 1;
   static const bool VARIADIC = false;
   static const EngineTypeInfo* const RETURN;
   static const EngineTypeInfo* const ARGS[ 1 ];

   _EngineArgumentTypeTable()
      : EngineArgumentTypeTable( TYPE< typename EngineTypeTraits< R >::Type >(), NUM_ARGUMENTS, ARGS ) {}
};
template< typename R, typename A >
const EngineTypeInfo* const _EngineArgumentTypeTable< R( A ) >::RETURN = TYPE< typename EngineTypeTraits< R >::Type >();
template< typename R, typename A >
const EngineTypeInfo* const _EngineArgumentTypeTable< R( A ) >::ARGS[ 1 ] =
{
   TYPE< typename EngineTypeTraits< A >::Type >()
};
template< typename R, typename A >
struct _EngineArgumentTypeTable< R( A, ... ) > : public _EngineArgumentTypeTable< R( A ) >
{
   static const bool VARIADIC = true;
   _EngineArgumentTypeTable() {}
};

template< typename R, typename A, typename B >
struct _EngineArgumentTypeTable< R( A, B ) > : public EngineArgumentTypeTable
{
   static const U32 NUM_ARGUMENTS = 2;
   static const bool VARIADIC = false;
   static const EngineTypeInfo* const RETURN;
   static const EngineTypeInfo* const ARGS[ 2 ];

   _EngineArgumentTypeTable()
      : EngineArgumentTypeTable( TYPE< typename EngineTypeTraits< R >::Type >(), NUM_ARGUMENTS, ARGS ) {}
};
template< typename R, typename A, typename B >
const EngineTypeInfo* const _EngineArgumentTypeTable< R( A, B ) >::RETURN = TYPE< typename EngineTypeTraits< R >::Type >();
template< typename R, typename A, typename B >
const EngineTypeInfo* const _EngineArgumentTypeTable< R( A, B ) >::ARGS[ 2 ] =
{
   TYPE< typename EngineTypeTraits< A >::Type >(),
   TYPE< typename EngineTypeTraits< B >::Type >()
};
template< typename R, typename A, typename B >
struct _EngineArgumentTypeTable< R( A, B, ... ) > : public _EngineArgumentTypeTable< R( A, B ) >
{
   static const bool VARIADIC = true;
   _EngineArgumentTypeTable() {}
};

template< typename R, typename A, typename B, typename C >
struct _EngineArgumentTypeTable< R( A, B, C ) > : public EngineArgumentTypeTable
{
   static const U32 NUM_ARGUMENTS = 3;
   static const bool VARIADIC = false;
   static const EngineTypeInfo* const RETURN;
   static const EngineTypeInfo* const ARGS[ 3 ];

   _EngineArgumentTypeTable()
      : EngineArgumentTypeTable( TYPE< typename EngineTypeTraits< R >::Type >(), NUM_ARGUMENTS, ARGS ) {}
};
template< typename R, typename A, typename B, typename C >
const EngineTypeInfo* const _EngineArgumentTypeTable< R( A, B, C ) >::RETURN = TYPE< typename EngineTypeTraits< R >::Type >();
template< typename R, typename A, typename B, typename C >
const EngineTypeInfo* const _EngineArgumentTypeTable< R( A, B, C ) >::ARGS[ 3 ] =
{
   TYPE< typename EngineTypeTraits< A >::Type >(),
   TYPE< typename EngineTypeTraits< B >::Type >(),
   TYPE< typename EngineTypeTraits< C >::Type >()
};
template< typename R, typename A, typename B, typename C >
struct _EngineArgumentTypeTable< R( A, B, C, ... ) > : public _EngineArgumentTypeTable< R( A, B, C ) >
{
   static const bool VARIADIC = true;
   _EngineArgumentTypeTable() {}
};

template< typename R, typename A, typename B, typename C, typename D >
struct _EngineArgumentTypeTable< R( A, B, C, D ) > : public EngineArgumentTypeTable
{
   static const U32 NUM_ARGUMENTS = 4;
   static const bool VARIADIC = false;
   static const EngineTypeInfo* const RETURN;
   static const EngineTypeInfo* const ARGS[ 4 ];

   _EngineArgumentTypeTable()
      : EngineArgumentTypeTable( TYPE< typename EngineTypeTraits< R >::Type >(), NUM_ARGUMENTS, ARGS ) {}
};
template< typename R, typename A, typename B, typename C, typename D >
const EngineTypeInfo* const _EngineArgumentTypeTable< R( A, B, C, D ) >::RETURN = TYPE< typename EngineTypeTraits< R >::Type >();
template< typename R, typename A, typename B, typename C, typename D >
const EngineTypeInfo* const _EngineArgumentTypeTable< R( A, B, C, D ) >::ARGS[ 4 ] =
{
   TYPE< typename EngineTypeTraits< A >::Type >(),
   TYPE< typename EngineTypeTraits< B >::Type >(),
   TYPE< typename EngineTypeTraits< C >::Type >(),
   TYPE< typename EngineTypeTraits< D >::Type >()
};
template< typename R, typename A, typename B, typename C, typename D >
struct _EngineArgumentTypeTable< R( A, B, C, D, ... ) > : public _EngineArgumentTypeTable< R( A, B, C, D ) >
{
   static const bool VARIADIC = true;
   _EngineArgumentTypeTable() {}
};

template< typename R, typename A, typename B, typename C, typename D, typename E >
struct _EngineArgumentTypeTable< R( A, B, C, D, E ) > : public EngineArgumentTypeTable
{
   static const U32 NUM_ARGUMENTS = 5;
   static const bool VARIADIC = false;
   static const EngineTypeInfo* const RETURN;
   static const EngineTypeInfo* const ARGS[ 5 ];

   _EngineArgumentTypeTable()
      : EngineArgumentTypeTable( TYPE< typename EngineTypeTraits< R >::Type >(), NUM_ARGUMENTS, ARGS ) {}
};
template< typename R, typename A, typename B, typename C, typename D, typename E >
const EngineTypeInfo* const _EngineArgumentTypeTable< R( A, B, C, D, E ) >::RETURN = TYPE< typename EngineTypeTraits< R >::Type >();
template< typename R, typename A, typename B, typename C, typename D, typename E >
const EngineTypeInfo* const _EngineArgumentTypeTable< R( A, B, C, D, E ) >::ARGS[ 5 ] =
{
   TYPE< typename EngineTypeTraits< A >::Type >(),
   TYPE< typename EngineTypeTraits< B >::Type >(),
   TYPE< typename EngineTypeTraits< C >::Type >(),
   TYPE< typename EngineTypeTraits< D >::Type >(),
   TYPE< typename EngineTypeTraits< E >::Type >()
};
template< typename R, typename A, typename B, typename C, typename D, typename E >
struct _EngineArgumentTypeTable< R( A, B, C, D, E, ... ) > : public _EngineArgumentTypeTable< R( A, B, C, D, E ) >
{
   static const bool VARIADIC = true;
   _EngineArgumentTypeTable() {}
};

template< typename R, typename A, typename B, typename C, typename D, typename E, typename F >
struct _EngineArgumentTypeTable< R( A, B, C, D, E, F ) > : public EngineArgumentTypeTable
{
   static const U32 NUM_ARGUMENTS = 6;
   static const bool VARIADIC = false;
   static const EngineTypeInfo* const RETURN;
   static const EngineTypeInfo* const ARGS[ 6 ];

   _EngineArgumentTypeTable()
      : EngineArgumentTypeTable( TYPE< typename EngineTypeTraits< R >::Type >(), NUM_ARGUMENTS, ARGS ) {}
};
template< typename R, typename A, typename B, typename C, typename D, typename E, typename F >
const EngineTypeInfo* const _EngineArgumentTypeTable< R( A, B, C, D, E, F ) >::RETURN = TYPE< typename EngineTypeTraits< R >::Type >();
template< typename R, typename A, typename B, typename C, typename D, typename E, typename F >
const EngineTypeInfo* const _EngineArgumentTypeTable< R( A, B, C, D, E, F ) >::ARGS[ 6 ] =
{
   TYPE< typename EngineTypeTraits< A >::Type >(),
   TYPE< typename EngineTypeTraits< B >::Type >(),
   TYPE< typename EngineTypeTraits< C >::Type >(),
   TYPE< typename EngineTypeTraits< D >::Type >(),
   TYPE< typename EngineTypeTraits< E >::Type >(),
   TYPE< typename EngineTypeTraits< F >::Type >()
};
template< typename R, typename A, typename B, typename C, typename D, typename E, typename F >
struct _EngineArgumentTypeTable< R( A, B, C, D, E, F, ... ) > : public _EngineArgumentTypeTable< R( A, B, C, D, E, F ) >
{
   static const bool VARIADIC = true;
   _EngineArgumentTypeTable() {}
};

template< typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G >
struct _EngineArgumentTypeTable< R( A, B, C, D, E, F, G ) > : public EngineArgumentTypeTable
{
   static const U32 NUM_ARGUMENTS = 7;
   static const bool VARIADIC = false;
   static const EngineTypeInfo* const RETURN;
   static const EngineTypeInfo* const ARGS[ 7 ];

   _EngineArgumentTypeTable()
      : EngineArgumentTypeTable( TYPE< typename EngineTypeTraits< R >::Type >(), NUM_ARGUMENTS, ARGS ) {}
};
template< typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G >
const EngineTypeInfo* const _EngineArgumentTypeTable< R( A, B, C, D, E, F, G ) >::RETURN = TYPE< typename EngineTypeTraits< R >::Type >();
template< typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G >
const EngineTypeInfo* const _EngineArgumentTypeTable< R( A, B, C, D, E, F, G ) >::ARGS[ 7 ] =
{
   TYPE< typename EngineTypeTraits< A >::Type >(),
   TYPE< typename EngineTypeTraits< B >::Type >(),
   TYPE< typename EngineTypeTraits< C >::Type >(),
   TYPE< typename EngineTypeTraits< D >::Type >(),
   TYPE< typename EngineTypeTraits< E >::Type >(),
   TYPE< typename EngineTypeTraits< F >::Type >(),
   TYPE< typename EngineTypeTraits< G >::Type >()
};
template< typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G >
struct _EngineArgumentTypeTable< R( A, B, C, D, E, F, G, ... ) > : public _EngineArgumentTypeTable< R( A, B, C, D, E, F, G ) >
{
   static const bool VARIADIC = true;
   _EngineArgumentTypeTable() {}
};

template< typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H >
struct _EngineArgumentTypeTable< R( A, B, C, D, E, F, G, H ) > : public EngineArgumentTypeTable
{
   static const U32 NUM_ARGUMENTS = 8;
   static const bool VARIADIC = false;
   static const EngineTypeInfo* const RETURN;
   static const EngineTypeInfo* const ARGS[ 8 ];

   _EngineArgumentTypeTable()
      : EngineArgumentTypeTable( TYPE< typename EngineTypeTraits< R >::Type >(), NUM_ARGUMENTS, ARGS ) {}
};
template< typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H >
const EngineTypeInfo* const _EngineArgumentTypeTable< R( A, B, C, D, E, F, G, H ) >::RETURN = TYPE< typename EngineTypeTraits< R >::Type >();
template< typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H >
const EngineTypeInfo* const _EngineArgumentTypeTable< R( A, B, C, D, E, F, G, H ) >::ARGS[ 8 ] =
{
   TYPE< typename EngineTypeTraits< A >::Type >(),
   TYPE< typename EngineTypeTraits< B >::Type >(),
   TYPE< typename EngineTypeTraits< C >::Type >(),
   TYPE< typename EngineTypeTraits< D >::Type >(),
   TYPE< typename EngineTypeTraits< E >::Type >(),
   TYPE< typename EngineTypeTraits< F >::Type >(),
   TYPE< typename EngineTypeTraits< G >::Type >(),
   TYPE< typename EngineTypeTraits< H >::Type >()
};
template< typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H >
struct _EngineArgumentTypeTable< R( A, B, C, D, E, F, G, H, ... ) > : public _EngineArgumentTypeTable< R( A, B, C, D, E, F, G, H ) >
{
   static const bool VARIADIC = true;
   _EngineArgumentTypeTable() {}
};

template< typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I >
struct _EngineArgumentTypeTable< R( A, B, C, D, E, F, G, H, I ) > : public EngineArgumentTypeTable
{
   static const U32 NUM_ARGUMENTS = 9;
   static const bool VARIADIC = false;
   static const EngineTypeInfo* const RETURN;
   static const EngineTypeInfo* const ARGS[ 9 ];

   _EngineArgumentTypeTable()
      : EngineArgumentTypeTable( TYPE< typename EngineTypeTraits< R >::Type >(), NUM_ARGUMENTS, ARGS ) {}
};
template< typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I >
const EngineTypeInfo* const _EngineArgumentTypeTable< R( A, B, C, D, E, F, G, H, I ) >::RETURN = TYPE< typename EngineTypeTraits< R >::Type >();
template< typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I >
const EngineTypeInfo* const _EngineArgumentTypeTable< R( A, B, C, D, E, F, G, H, I ) >::ARGS[ 9 ] =
{
   TYPE< typename EngineTypeTraits< A >::Type >(),
   TYPE< typename EngineTypeTraits< B >::Type >(),
   TYPE< typename EngineTypeTraits< C >::Type >(),
   TYPE< typename EngineTypeTraits< D >::Type >(),
   TYPE< typename EngineTypeTraits< E >::Type >(),
   TYPE< typename EngineTypeTraits< F >::Type >(),
   TYPE< typename EngineTypeTraits< G >::Type >(),
   TYPE< typename EngineTypeTraits< H >::Type >(),
   TYPE< typename EngineTypeTraits< I >::Type >()
};
template< typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I >
struct _EngineArgumentTypeTable< R( A, B, C, D, E, F, G, H, I, ... ) > : public _EngineArgumentTypeTable< R( A, B, C, D, E, F, G, H, I ) >
{
   static const bool VARIADIC = true;
   _EngineArgumentTypeTable() {}
};

template< typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J >
struct _EngineArgumentTypeTable< R( A, B, C, D, E, F, G, H, I, J ) > : public EngineArgumentTypeTable
{
   static const U32 NUM_ARGUMENTS = 10;
   static const bool VARIADIC = false;
   static const EngineTypeInfo* const RETURN;
   static const EngineTypeInfo* const ARGS[ 10 ];

   _EngineArgumentTypeTable()
      : EngineArgumentTypeTable( TYPE< typename EngineTypeTraits< R >::Type >(), NUM_ARGUMENTS, ARGS ) {}
};
template< typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J >
const EngineTypeInfo* const _EngineArgumentTypeTable< R( A, B, C, D, E, F, G, H, I, J ) >::RETURN = TYPE< typename EngineTypeTraits< R >::Type >();
template< typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J >
const EngineTypeInfo* const _EngineArgumentTypeTable< R( A, B, C, D, E, F, G, H, I, J ) >::ARGS[ 10 ] =
{
   TYPE< typename EngineTypeTraits< A >::Type >(),
   TYPE< typename EngineTypeTraits< B >::Type >(),
   TYPE< typename EngineTypeTraits< C >::Type >(),
   TYPE< typename EngineTypeTraits< D >::Type >(),
   TYPE< typename EngineTypeTraits< E >::Type >(),
   TYPE< typename EngineTypeTraits< F >::Type >(),
   TYPE< typename EngineTypeTraits< G >::Type >(),
   TYPE< typename EngineTypeTraits< H >::Type >(),
   TYPE< typename EngineTypeTraits< I >::Type >(),
   TYPE< typename EngineTypeTraits< J >::Type >()
};
template< typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J >
struct _EngineArgumentTypeTable< R( A, B, C, D, E, F, G, H, I, J, ... ) > : public _EngineArgumentTypeTable< R( A, B, C, D, E, F, G, H, I, J ) >
{
   static const bool VARIADIC = true;
   _EngineArgumentTypeTable() {}
};

template< typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K >
struct _EngineArgumentTypeTable< R( A, B, C, D, E, F, G, H, I, J, K ) > : public EngineArgumentTypeTable
{
   static const U32 NUM_ARGUMENTS = 11;
   static const bool VARIADIC = false;
   static const EngineTypeInfo* const RETURN;
   static const EngineTypeInfo* const ARGS[ 11 ];

   _EngineArgumentTypeTable()
      : EngineArgumentTypeTable( TYPE< typename EngineTypeTraits< R >::Type >(), NUM_ARGUMENTS, ARGS ) {}
};
template< typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K >
const EngineTypeInfo* const _EngineArgumentTypeTable< R( A, B, C, D, E, F, G, H, I, J, K ) >::RETURN = TYPE< typename EngineTypeTraits< R >::Type >();
template< typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K >
const EngineTypeInfo* const _EngineArgumentTypeTable< R( A, B, C, D, E, F, G, H, I, J, K ) >::ARGS[ 11 ] =
{
   TYPE< typename EngineTypeTraits< A >::Type >(),
   TYPE< typename EngineTypeTraits< B >::Type >(),
   TYPE< typename EngineTypeTraits< C >::Type >(),
   TYPE< typename EngineTypeTraits< D >::Type >(),
   TYPE< typename EngineTypeTraits< E >::Type >(),
   TYPE< typename EngineTypeTraits< F >::Type >(),
   TYPE< typename EngineTypeTraits< G >::Type >(),
   TYPE< typename EngineTypeTraits< H >::Type >(),
   TYPE< typename EngineTypeTraits< I >::Type >(),
   TYPE< typename EngineTypeTraits< J >::Type >(),
   TYPE< typename EngineTypeTraits< K >::Type >()
};
template< typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K >
struct _EngineArgumentTypeTable< R( A, B, C, D, E, F, G, H, I, J, K, ... ) > : public _EngineArgumentTypeTable< R( A, B, C, D, E, F, G, H, I, J, K ) >
{
   static const bool VARIADIC = true;
   _EngineArgumentTypeTable() {}
};

template< typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L >
struct _EngineArgumentTypeTable< R( A, B, C, D, E, F, G, H, I, J, K, L ) > : public EngineArgumentTypeTable
{
   static const U32 NUM_ARGUMENTS = 12;
   static const bool VARIADIC = false;
   static const EngineTypeInfo* const RETURN;
   static const EngineTypeInfo* const ARGS[ 12 ];

   _EngineArgumentTypeTable()
      : EngineArgumentTypeTable( TYPE< typename EngineTypeTraits< R >::Type >(), NUM_ARGUMENTS, ARGS ) {}
};
template< typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L >
const EngineTypeInfo* const _EngineArgumentTypeTable< R( A, B, C, D, E, F, G, H, I, J, K, L ) >::RETURN = TYPE< typename EngineTypeTraits< R >::Type >();
template< typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L >
const EngineTypeInfo* const _EngineArgumentTypeTable< R( A, B, C, D, E, F, G, H, I, J, K, L ) >::ARGS[ 12 ] =
{
   TYPE< typename EngineTypeTraits< A >::Type >(),
   TYPE< typename EngineTypeTraits< B >::Type >(),
   TYPE< typename EngineTypeTraits< C >::Type >(),
   TYPE< typename EngineTypeTraits< D >::Type >(),
   TYPE< typename EngineTypeTraits< E >::Type >(),
   TYPE< typename EngineTypeTraits< F >::Type >(),
   TYPE< typename EngineTypeTraits< G >::Type >(),
   TYPE< typename EngineTypeTraits< H >::Type >(),
   TYPE< typename EngineTypeTraits< I >::Type >(),
   TYPE< typename EngineTypeTraits< J >::Type >(),
   TYPE< typename EngineTypeTraits< K >::Type >(),
   TYPE< typename EngineTypeTraits< L >::Type >()
};
template< typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L >
struct _EngineArgumentTypeTable< R( A, B, C, D, E, F, G, H, I, J, K, L, ... ) > : public _EngineArgumentTypeTable< R( A, B, C, D, E, F, G, H, I, J, K, L ) >
{
   static const bool VARIADIC = true;
   _EngineArgumentTypeTable() {}
};

#endif // !_ENGINETYPEINFO_H_
