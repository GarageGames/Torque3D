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

#ifndef _DYNAMIC_CONSOLETYPES_H_
#define _DYNAMIC_CONSOLETYPES_H_

#ifndef _CONSOLE_H_
#include "console/console.h"
#endif

#ifndef _STRINGFUNCTIONS_H_
#include "core/strings/stringFunctions.h"
#endif

#ifndef _ENGINETYPEINFO_H_
#include "console/engineTypeInfo.h"
#endif


/// @file
/// Support for legacy TorqueScript console types.


/// Information about a console type.
class ConsoleBaseType
{
   protected:

      /// Unique numeric type ID.
      S32 mTypeID;
            
      /// Size of a single data value point.
      dsize_t mTypeSize;
      
      /// Name of this console type (TypeXXX).
      StringTableEntry mTypeName;
      
      /// Name of GuiInspectorField subclass to instantiate for field edit controls in
      /// inspectors.
      const char* mInspectorFieldType;

      /// Next item in the list of all console types.
      ConsoleBaseType* mListNext;

      /// Type info object for the engine type corresponding to this console type.
      /// Since several console types may be mapped to a single native type, this type info
      /// instance may be shared by multiple ConsoleBaseType instances.
      /// NULL if the console type is not mapped to an engine API type.
      const EngineTypeInfo* mTypeInfo;
      
      /// Total number of defined console types.  This is used to generate unique IDs for each type.
      static S32 smConsoleTypeCount;

      /// We maintain a linked list of all console types; this is its head.
      static ConsoleBaseType* smListHead;

      /// The constructor is responsible for linking an element into the
      /// master list, registering the type ID, etc.
      ConsoleBaseType( const S32 size, S32 *idPtr, const char *aTypeName );

      /// Destructor is private to avoid people mucking up the list.
      ~ConsoleBaseType() {}
 
   public:

      /// @name cbt_list List Interface
      ///
      /// Interface for accessing/traversing the list of types.

      /// Get the head of the list.
      static ConsoleBaseType *getListHead();

      /// Get the item that follows this item in the list.
      ConsoleBaseType *getListNext() const
      {
         return mListNext;
      }

      /// @}

      /// Called once to initialize the console type system.
      static void initialize();

      /// Call me to get a pointer to a type's info.
      static ConsoleBaseType *getType( const S32 typeID );

      /// Call to get a pointer to a type's info
      static ConsoleBaseType *getTypeByName( const char *typeName );
      static ConsoleBaseType *getTypeByClassName( const char *typeName );
      
      /// Return the unique numeric ID of this type.
      S32 getTypeID() const { return mTypeID; }
      
      /// Return the size of a single value in bytes.
      S32 getTypeSize() const { return mTypeSize; }
      
      /// Return the console type name (TypeXXX).
      StringTableEntry getTypeName() const { return mTypeName; }
      
      /// Return the type info for the engine type corresponding to this console type or NULL if
      /// there is no mapping for the console type.
      const EngineTypeInfo* getTypeInfo() const { return mTypeInfo; }
            
      /// Return the documentation string for this type.
      const char* getDocString() const { return getTypeInfo() ? getTypeInfo()->getDocString() : ""; }
      
      /// Return the EnumTable for this type (only for enumeration types).
      const EngineEnumTable* getEnumTable() const { return getTypeInfo() ? getTypeInfo()->getEnumTable() : NULL; }
      
      /// Return the name of the GuiInspectorField subclass that fields of this
      /// type should use for editing.
      const char* getInspectorFieldType() { return mInspectorFieldType; }
      
      /// Set the name of the GuiInspectorField subclass that fields of this type
      /// should use for editing.
      void setInspectorFieldType( const char* type ) { mInspectorFieldType = type; }
      
      /// @name Value Handling Interface
      /// @{

      virtual void setData( void* dptr, S32 argc, const char** argv, const EnumTable* tbl, BitSet32 flag ) = 0;
      virtual const char* getData( void* dptr, const EnumTable* tbl, BitSet32 flag ) = 0;
      virtual const char* getTypeClassName() = 0;
      
      /// Allocate a single value.
      virtual void* getNativeVariable() = 0;
      
      /// Delete a single value allocated with getNativeVariable().
      virtual void deleteNativeVariable(void* var) = 0;
      
      /// Return true if this is datablock object type.
      virtual const bool isDatablock() { return false; };
      
      virtual const char* prepData( const char* data, char* buffer, U32 bufferLen ) { return data; };
      
      /// @}
};


class EnumConsoleBaseType : public ConsoleBaseType
{
   public:
   
      typedef ConsoleBaseType Parent;
      
   protected:
   
      EnumConsoleBaseType( S32 size, S32* idPtr, const char* typeName )
         : Parent( size, idPtr, typeName ) {}
         
   public:

      virtual const char* getData(void *dptr, const EnumTable *tbl, BitSet32 flag)
      {
         S32 dptrVal = *( S32* ) dptr;
         if( !tbl ) tbl = getEnumTable();
         const U32 numEnums = tbl->getNumValues();
         for( U32 i = 0; i < numEnums; ++ i )
            if( dptrVal == ( *tbl )[ i ].mInt )
               return ( *tbl )[ i ].mName;
         return "";
      }
      virtual void setData(void *dptr, S32 argc, const char **argv, const EnumTable *tbl, BitSet32 flag)
      {
         if( argc != 1 ) return;
         if( !tbl ) tbl = getEnumTable();
         S32 val = 0;
         const U32 numEnums = tbl->getNumValues();
         for( U32 i = 0; i < numEnums; ++ i )
            if( dStricmp( argv[ 0 ], ( *tbl )[ i ].mName ) == 0 )
            {
               val = ( *tbl )[ i ].mInt;
               break;
            }
         *( ( S32* ) dptr ) = val;
      }
};


class BitfieldConsoleBaseType : public ConsoleBaseType
{
   public:
   
      typedef ConsoleBaseType Parent;
      
   protected:
   
      BitfieldConsoleBaseType( S32 size, S32* idPtr, const char* typeName )
         : Parent( size, idPtr, typeName ) {}
         
   public:

      virtual const char* getData( void* dptr, const EnumTable*, BitSet32 )
      {
         char* returnBuffer = Con::getReturnBuffer(256);
         dSprintf(returnBuffer, 256, "0x%08x", *((S32 *) dptr) );
         return returnBuffer;
      }
      virtual void setData( void* dptr, S32 argc, const char** argv, const EnumTable*, BitSet32 )
      {
         if( argc != 1 ) return; \
         *((S32 *) dptr) = dAtoui(argv[0],0); \
      }
};



template< typename T >
struct _ConsoleConstType
{
   typedef const T ConstType;
};

/// Return the type ID for the primary console type associated with the given native type.
///
/// There can only be one console type associated with a C++ type.  This is referred to as the primary
/// console type.
///
/// @return The type ID of the primary console type for "T".
template< typename T >
S32 TYPEID() { return T::_smTypeId; } // Default assumes a structured type storing its ID in a static member variable.


// Helper to allow to override certain mappings.
template< typename T >
const EngineTypeInfo* _MAPTYPE() { return TYPE< T >(); }


/// @name Console Type Macros
/// @{

#define DefineConsoleType( type, nativeType ) \
   extern S32 type; \
   extern const char* castConsoleTypeToString( _ConsoleConstType< nativeType >::ConstType &arg ); \
   extern bool castConsoleTypeFromString( nativeType &arg, const char *str ); \
   template<> S32 TYPEID< nativeType >();
   
#define DefineUnmappedConsoleType( type, nativeType ) \
   DefineConsoleType( type, nativeType ) \
   template<> inline const EngineTypeInfo* _MAPTYPE< nativeType >() { return NULL; }

#define ConsoleType( typeName, type, nativeType ) \
   S32 type; \
   class ConsoleType##type : public ConsoleBaseType \
   { \
   public: \
      typedef nativeType T; \
      ConsoleType##type() \
         : ConsoleBaseType( sizeof( nativeType ), &type, #type ) \
      { \
         mTypeInfo = _MAPTYPE< nativeType >(); \
      } \
      virtual void setData(void *dptr, S32 argc, const char **argv, const EnumTable *tbl, BitSet32 flag); \
      virtual const char *getData(void *dptr, const EnumTable *tbl, BitSet32 flag ); \
      virtual const char *getTypeClassName() { return #typeName ; } \
      virtual void       *getNativeVariable() { T* var = new T; return (void*)var; } \
      virtual void        deleteNativeVariable(void* var) { T* nativeVar = reinterpret_cast<T*>(var); delete nativeVar; } \
   }; \
   ConsoleType ## type gConsoleType ## type ## Instance;

#define ImplementConsoleTypeCasters( type, nativeType ) \
   const char *castConsoleTypeToString( _ConsoleConstType< nativeType >::ConstType &arg ) { return Con::getData(type, const_cast< nativeType* >( &arg ), 0); } \
   bool castConsoleTypeFromString( nativeType &arg, const char *str ) { Con::setData(type, const_cast< nativeType* >( &arg ), 0, 1, &str); return true; } \
   template<> S32 TYPEID< nativeType >() { return type; }


#define ConsolePrepType( typeName, type, nativeType ) \
   S32 type; \
   class ConsoleType##type : public ConsoleBaseType \
   { \
   public: \
      typedef nativeType T; \
      ConsoleType##type() \
         : ConsoleBaseType( sizeof( nativeType ), &type, #type ) \
      { \
         mTypeInfo = _MAPTYPE< nativeType >(); \
      } \
      virtual void setData(void *dptr, S32 argc, const char **argv, const EnumTable *tbl, BitSet32 flag); \
      virtual const char *getData(void *dptr, const EnumTable *tbl, BitSet32 flag ); \
      virtual const char *getTypeClassName() { return #typeName ; } \
      virtual void       *getNativeVariable() { T* var = new T; return (void*)var; } \
      virtual void        deleteNativeVariable(void* var) { T* nativeVar = reinterpret_cast<T*>(var); delete nativeVar; } \
      virtual const char *prepData(const char *data, char *buffer, U32 bufferLen); \
   }; \
   ConsoleType ## type gConsoleType ## type ## Instance;

#define ConsoleSetType( type ) \
   void ConsoleType##type::setData(void *dptr, S32 argc, const char **argv, const EnumTable *tbl, BitSet32 flag)

#define ConsoleGetType( type ) \
   const char *ConsoleType##type::getData(void *dptr, const EnumTable *tbl, BitSet32 flag)

#define ConsoleProcessData( type ) \
   const char *ConsoleType##type::prepData(const char *data, char *buffer, U32 bufferSz)
   
   
#define DefineEnumType( type ) \
   DECLARE_ENUM( type ); \
   DefineConsoleType( Type ## type, type );

#define _ConsoleEnumType( typeName, type, nativeType ) \
   S32 type; \
   ImplementConsoleTypeCasters( type, nativeType ) \
   class EnumConsoleType ## type : public EnumConsoleBaseType \
   { \
   public: \
      EnumConsoleType ## type() \
         : EnumConsoleBaseType( sizeof( nativeType ), &type, #type ) \
      { \
         mTypeInfo = _MAPTYPE< nativeType >(); \
      } \
      virtual const char *getTypeClassName() { return #typeName; } \
      virtual void       *getNativeVariable() { return new nativeType; } \
      virtual void        deleteNativeVariable(void* var) { nativeType* nativeVar = reinterpret_cast< nativeType* >( var ); delete nativeVar; }\
   }; \
   EnumConsoleType ## type gConsoleType ## type ## Instance;

#define ImplementEnumType( type, doc ) \
   _ConsoleEnumType( type, Type ## type, type ) \
   IMPLEMENT_ENUM( type, type,, doc )
   
#define EndImplementEnumType \
   END_IMPLEMENT_ENUM


#define DefineBitfieldType( type ) \
   DECLARE_BITFIELD( type ); \
   DefineConsoleType( Type ## type, type );

#define _ConsoleBitfieldType( typeName, type, nativeType ) \
   S32 type; \
   ImplementConsoleTypeCasters( type, nativeType ) \
   class BitfieldConsoleType ## type : public BitfieldConsoleBaseType \
   { \
   public: \
      BitfieldConsoleType ## type() \
         : BitfieldConsoleBaseType( sizeof( nativeType ), &type, #type ) \
      { \
         mTypeInfo = _MAPTYPE< nativeType >(); \
      } \
      virtual const char *getTypeClassName() { return #typeName; } \
      virtual void       *getNativeVariable() { return new nativeType; } \
      virtual void        deleteNativeVariable(void* var) { nativeType* nativeVar = reinterpret_cast< nativeType* >( var ); delete nativeVar; }\
   }; \
   BitfieldConsoleType ## type gConsoleType ## type ## Instance;
      
#define ImplementBitfieldType( type, doc ) \
   _ConsoleBitfieldType( type, Type ## type, type ) \
   IMPLEMENT_BITFIELD( type, type,, doc )
   
#define EndImplementBitfieldType \
   END_IMPLEMENT_BITFIELD

/// @}

#endif
