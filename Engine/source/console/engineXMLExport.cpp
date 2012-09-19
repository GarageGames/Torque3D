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

#include "console/engineExports.h"
#include "console/engineAPI.h"
#include "console/engineTypes.h"
#include "console/engineFunctions.h"
#include "console/SimXMLDocument.h"


/// @file
/// A generator that will dump all export structures contained in an engine
/// DLL to an XML file which may then be used by wrapper generators to create a
/// language-specific binding for the engine API.  Using XML as an intermediary
/// format allows the generators to use all of the export structures without
/// actually having to access them directly in the DLL as native entities.


static void exportScope( const EngineExportScope* scope, SimXMLDocument* xml, bool addNode = false );


static String getTypeName( const EngineTypeInfo* type )
{
   if( !type )
   {
      static String sVoid( "void" );
      return sVoid;
   }
      
   return type->getFullyQualifiedExportName();
}

static const char* getDocString( const EngineExport* exportInfo )
{
   if( !exportInfo->getDocString() )
      return "";
   
   return exportInfo->getDocString();
}

template< typename T >
inline T getArgValue( const EngineFunctionDefaultArguments* defaultArgs, U32 offset )
{
   return *reinterpret_cast< const T* >( defaultArgs->getArgs() + offset );
}


// List of exports that we want filtered out.  This will only be needed as long
// as the console system is still around.
static const char* sExportFilterList[] =
{
   "Console", // Console namespace
};

static bool isExportFiltered( const EngineExport* exportInfo )
{
   String qualifiedName = exportInfo->getFullyQualifiedExportName();
   
   for( U32 i = 0; i < ( sizeof( sExportFilterList ) / sizeof( sExportFilterList[ 0 ] ) ); ++ i )
      if( qualifiedName.compare( sExportFilterList[ i ] ) == 0 )
         return true;
         
   return false;
}

//=============================================================================
//    Functions.
//=============================================================================
// MARK: ---- Functions ----

//-----------------------------------------------------------------------------

/// Helper to parse argument names out of a prototype string.
static Vector< String > parseFunctionArgumentNames( const EngineFunctionInfo* function )
{
   Vector< String > argNames;
   
   const char* prototype = function->getPrototypeString();
   if( !prototype )
      return argNames;
      
   const U32 prototypeLength = dStrlen( prototype );
   const char* prototypeEnd = &prototype[ prototypeLength ];
   const char* ptr = prototypeEnd - 1;
   
   // Search for right parenthesis.
   while( ptr >= prototype && *ptr != ')' )
      ptr --;
      
   if( ptr < prototype )
      return argNames;
   ptr --;
   
   while( ptr >= prototype && *ptr != '(' )
   {
      // Skip back over spaces.
      
      while( ptr >= prototype && dIsspace( *ptr ) )
         ptr --;
      if( ptr < prototype )
         return argNames;
         
      // Parse out name.
      
      const char* end = ptr + 1;
      while( ptr > prototype && dIsalnum( *ptr ) )
         ptr --;
      const char* start = ptr + 1;
               
      // Skip back over spaces.

      while( ptr >= prototype && dIsspace( *ptr ) )
         ptr --;
         
      // If we're sure we don't have just a type name without an
      // argument name, copy out the argument name name. 

      if( ptr >= prototype && *ptr != ',' && *ptr != '(' && end > start )
         argNames.push_front( String( start, end - start ) );
      else
         argNames.push_front( "" );
      
      // Skip back to comma or opening parenthesis.
      
      U32 parenNestingCount = 0;
      while( ptr >= prototype )
      {
         if( *ptr == ')' )
            parenNestingCount ++;
         else if( *ptr == '(' )
            parenNestingCount --;
         else if( *ptr == ',' && parenNestingCount == 0 )
         {
            ptr --;
            break;
         }
         else if( *ptr == '(' && parenNestingCount == 0 )
            break;
            
         ptr --;
      }
   }
   
   // Add 'this' parameter if this is a method.
      
   if( dStrncmp( prototype, "virtual ", sizeof( "virtual " ) - 1 ) == 0 )
      argNames.push_front( "this" );

   return argNames;
}

//-----------------------------------------------------------------------------

static String getDefaultArgumentValue( const EngineFunctionInfo* function, const EngineTypeInfo* type, U32 offset )
{
   String value;
   const EngineFunctionDefaultArguments* defaultArgs = function->getDefaultArguments();
   
   switch( type->getTypeKind() )
   {
      case EngineTypeKindPrimitive:
      {
         #define PRIMTYPE( tp )                                               \
            if( TYPE< tp >() == type )                                        \
            {                                                                 \
               tp val = getArgValue< tp >( defaultArgs, offset );             \
               value = String::ToString( val );                               \
            }
            
         PRIMTYPE( bool );
         PRIMTYPE( S8 );
         PRIMTYPE( U8 );
         PRIMTYPE( S32 );
         PRIMTYPE( U32 );
         PRIMTYPE( F32 );
         PRIMTYPE( F64 );
         
         //TODO: for now we store string literals in ASCII; needs to be sorted out
         if( TYPE< const char* >() == type )
         {
            const char* val = getArgValue< const char* >( defaultArgs, offset );
            value = val;
         }
            
         #undef PRIMTYPE
         break;
      }
         
      case EngineTypeKindEnum:
      {
         S32 val = getArgValue< S32 >( defaultArgs, offset );
         AssertFatal( type->getEnumTable(), "engineXMLExport - Enum type without table!" );
         
         const EngineEnumTable& table = *( type->getEnumTable() );
         const U32 numValues = table.getNumValues();
         
         for( U32 i = 0; i < numValues; ++ i )
            if( table[ i ].getInt() == val )
            {
               value = table[ i ].getName();
               break;
            }
               
         break;
      }

      case EngineTypeKindBitfield:
      {
         S32 val = getArgValue< S32 >( defaultArgs, offset );
         AssertFatal( type->getEnumTable(), "engineXMLExport - Bitfield type without table!" );
         
         const EngineEnumTable& table = *( type->getEnumTable() );
         const U32 numValues = table.getNumValues();
         
         bool isFirst = true;
         for( U32 i = 0; i < numValues; ++ i )
            if( table[ i ].getInt() & val )
            {
               if( !isFirst )
                  value += '|';
                  
               value = table[ i ].getName();
               isFirst = false;
            }

         break;
      }
      
      case EngineTypeKindStruct:
      {
         //TODO: struct type default argument values
         break;
      }
         
      case EngineTypeKindClass:
      case EngineTypeKindFunction:
      {
         // For these two kinds, we support "null" as the only valid
         // default value.
         
         const void* ptr = getArgValue< const void* >( defaultArgs, offset );
         if( !ptr )
            value = "null";
         break;
      }
      
      default:
         break;
   }
   
   return value;
}

//-----------------------------------------------------------------------------

static void exportFunction( const EngineFunctionInfo* function, SimXMLDocument* xml )
{
   if( isExportFiltered( function ) )
      return;
      
   xml->pushNewElement( "EngineFunction" );
      
      xml->setAttribute( "name", function->getExportName() );
      xml->setAttribute( "returnType", getTypeName( function->getReturnType() ) );
      xml->setAttribute( "symbol", function->getBindingName() );
      xml->setAttribute( "isCallback", function->isCallout() ? "1" : "0" );
      xml->setAttribute( "isVariadic", function->getFunctionType()->isVariadic() ? "1" : "0" );
      xml->setAttribute( "docs", getDocString( function ) );
      
      xml->pushNewElement( "arguments" );
      
         const U32 numArguments = function->getNumArguments();
         const U32 numDefaultArguments = ( function->getDefaultArguments() ? function->getDefaultArguments()->mNumDefaultArgs : 0 );
         const U32 firstDefaultArg = numArguments - numDefaultArguments;
         
         Vector< String > argumentNames = parseFunctionArgumentNames( function );
         const U32 numArgumentNames = argumentNames.size();
         
         // Accumulated offset in function argument frame vector.
         U32 argFrameOffset = 0;
         
         for( U32 i = 0; i < numArguments; ++ i )
         {
            xml->pushNewElement( "EngineFunctionArgument" );
            const EngineTypeInfo* type = function->getArgumentType( i );
            AssertFatal( type != NULL, "exportFunction - Argument cannot have type void!" );
            
            String argName;
            if( i < numArgumentNames )
               argName = argumentNames[ i ];
               
            xml->setAttribute( "name", argName );
            xml->setAttribute( "type", getTypeName( type ) );
            
            if( i >= firstDefaultArg )
            {
               String defaultValue = getDefaultArgumentValue( function, type, argFrameOffset );
               xml->setAttribute( "defaultValue", defaultValue );
            }

            xml->popElement();
            
            if( type->getTypeKind() == EngineTypeKindStruct )
               argFrameOffset += type->getInstanceSize();
            else
               argFrameOffset += type->getValueSize();
               
            #ifdef _PACK_BUG_WORKAROUNDS
            if( argFrameOffset % 4 > 0 )
               argFrameOffset += 4 - ( argFrameOffset % 4 );
            #endif
         }
         
      xml->popElement();
         
   xml->popElement();
}


//=============================================================================
//    Types.
//=============================================================================
// MARK: ---- Types ----

//-----------------------------------------------------------------------------

static void exportType( const EngineTypeInfo* type, SimXMLDocument* xml )
{
   // Don't export anonymous types.
   if( !type->getTypeName()[ 0 ] )
      return;
      
   if( isExportFiltered( type ) )
      return;
      
   const char* nodeName = NULL;
   switch( type->getTypeKind() )
   {
      case EngineTypeKindPrimitive:
         nodeName = "EnginePrimitiveType";
         break;
         
      case EngineTypeKindEnum:
         nodeName = "EngineEnumType";
         break;
         
      case EngineTypeKindBitfield:
         nodeName = "EngineBitfieldType";
         break;
         
      case EngineTypeKindStruct:
         nodeName = "EngineStructType";
         break;
         
      case EngineTypeKindClass:
         nodeName = "EngineClassType";
         break;
      
      default:
         return;
   }
   
   xml->pushNewElement( nodeName );

      xml->setAttribute( "name", type->getTypeName() );
      xml->setAttribute( "size", String::ToString( type->getInstanceSize() ) );
      xml->setAttribute( "isAbstract", type->isAbstract() ? "1" : "0" );
      xml->setAttribute( "isInstantiable", type->isInstantiable() ? "1" : "0" );
      xml->setAttribute( "isDisposable", type->isDisposable() ? "1" : "0" );
      xml->setAttribute( "isSingleton", type->isSingleton() ? "1" : "0" );
      xml->setAttribute( "docs", getDocString( type ) );
      
      if( type->getSuperType() )
         xml->setAttribute( "superType", getTypeName( type->getSuperType() ) );
      
      if( type->getEnumTable() )
      {
         xml->pushNewElement( "enums" );
         
            const EngineEnumTable& table = *( type->getEnumTable() );
            const U32 numValues = table.getNumValues();
            
            for( U32 i = 0; i < numValues; ++ i )
            {
               xml->pushNewElement( "EngineEnum" );
               
                  xml->setAttribute( "name", table[ i ].getName() );
                  xml->setAttribute( "value", String::ToString( table[ i ].getInt() ) );
                  xml->setAttribute( "docs", table[ i ].getDocString() ? table[ i ].getDocString() : "" );
                  
               xml->popElement();
            }
            
         xml->popElement();
      }
      else if( type->getFieldTable() )
      {
         xml->pushNewElement( "fields" );
         
            const EngineFieldTable& table = *( type->getFieldTable() );
            const U32 numFields = table.getNumFields();
            
            for( U32 i = 0; i < numFields; ++ i )
            {
               const EngineFieldTable::Field& field = table[ i ];
               
               xml->pushNewElement( "EngineField" );
               
                  xml->setAttribute( "name", field.getName() );
                  xml->setAttribute( "type", getTypeName( field.getType() ) );
                  xml->setAttribute( "offset", String::ToString( field.getOffset() ) );
                  xml->setAttribute( "indexedSize", String::ToString( field.getNumElements() ) );
                  xml->setAttribute( "docs", field.getDocString() ? field.getDocString() : "" );
               
               xml->popElement();
            }
         
         xml->popElement();
      }
      else if( type->getPropertyTable() )
      {
         xml->pushNewElement( "properties" );
         
            const EnginePropertyTable& table = *( type->getPropertyTable() );
            const U32 numProperties = table.getNumProperties();
            U32 groupNestingDepth = 0;
            
            for( U32 i = 0; i < numProperties; ++ i )
            {
               const EnginePropertyTable::Property& property = table[ i ];
               
               if( property.isGroupBegin() )
               {
                  groupNestingDepth ++;
                  xml->pushNewElement( "EnginePropertyGroup" );
                  
                     xml->setAttribute( "name", property.getName() );
                     xml->setAttribute( "indexedSize", String::ToString( property.getNumElements() ) );
                     xml->setAttribute( "docs", property.getDocString() ? property.getDocString() : "" );
                     
                     xml->pushNewElement( "properties" );
               }
               else if( property.isGroupEnd() )
               {
                  groupNestingDepth --;
                  xml->popElement();
                  xml->popElement();
               }
               else
               {
                  xml->pushNewElement( "EngineProperty" );
                  
                     xml->setAttribute( "name", property.getName() );
                     xml->setAttribute( "indexedSize", String::ToString( property.getNumElements() ) );
                     xml->setAttribute( "isConstant", property.isConstant() ? "1" : "0" );
                     xml->setAttribute( "isTransient", property.isTransient() ? "1" : "0" );
                     xml->setAttribute( "isVisible", property.hideInInspectors() ? "0" : "1" );
                     xml->setAttribute( "docs", property.getDocString() ? property.getDocString() : "" );
                  
                  xml->popElement();
               }
            }
         
         AssertFatal( !groupNestingDepth, "exportType - Property group nesting mismatch!" );
         xml->popElement();
      }      
      exportScope( type, xml );
      
   xml->popElement();
}


//=============================================================================
//    Scopes.
//=============================================================================
// MARK: ---- Scopes ----

//-----------------------------------------------------------------------------

static void exportScope( const EngineExportScope* scope, SimXMLDocument* xml, bool addNode )
{
   if( addNode )
   {
      if( isExportFiltered( scope ) )
         return;
         
      xml->pushNewElement( "EngineExportScope" );
      
         xml->setAttribute( "name", scope->getExportName() );
         xml->setAttribute( "docs", getDocString( scope ) );
   }

   // Dump all contained exports.
   
   xml->pushNewElement( "exports" );
   
      for( const EngineExport* exportInfo = scope->getExports(); exportInfo != NULL; exportInfo = exportInfo->getNextExport() )
      {
         switch( exportInfo->getExportKind() )
         {
            case EngineExportKindScope:
               exportScope( static_cast< const EngineExportScope* >( exportInfo ), xml, true );
               break;
               
            case EngineExportKindFunction:
               exportFunction( static_cast< const EngineFunctionInfo* >( exportInfo ), xml );
               break;
               
            case EngineExportKindType:
               exportType( static_cast< const EngineTypeInfo* >( exportInfo ), xml );
               break;
               
            default:
               break;
         }
      }
      
   xml->popElement();
   
   if( addNode )
      xml->popElement();
}

//-----------------------------------------------------------------------------

DefineEngineFunction( exportEngineAPIToXML, SimXMLDocument*, (),,
   "Create a XML document containing a dump of the entire exported engine API.\n\n"
   "@return A SimXMLDocument containing a dump of the engine's export information or NULL if the operation failed.\n\n"
   "@ingroup Console" )
{
   SimXMLDocument* xml = new SimXMLDocument;
   xml->registerObject();
   Sim::getRootGroup()->addObject( xml );
   xml->addHeader();
   
   exportScope( EngineExportScope::getGlobalScope(), xml, true );
   
   return xml;
}
