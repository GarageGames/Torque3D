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

#include "platform/platform.h"
#include "console/console.h"

#include "console/engineAPI.h"
#include "core/stream/fileStream.h"
#include "console/consoleInternal.h"
#include "console/compiler.h"

#define USE_UNDOCUMENTED_GROUP

/// @file
/// Documentation generator for the current TorqueScript-based engine API.
///
/// Be aware that this generator is solely for the legacy console system and
/// is and will not be useful to the new engine API system.  It will go away
/// when the console interop is removed.


/// Used to track unique groups encountered during
/// the dump process.
static HashTable<String,U32> smDocGroups;


static void dumpDoc( Stream& stream, const char* text, bool checkUngrouped = true )
{
   // Extract brief.
   
   String brief;
   
   if( text )
   {
      const char* briefTag = dStrstr( text, "@brief" );
      if( !briefTag )
      {
         const char* newline = dStrchr( text, '\n' );
         if( newline )
         {
            brief = String( text, newline - text );
            text = newline + 1;
         }
         else
         {
            brief = text;
            text = NULL;
         }
      }
   }

   // Write doc comment.
   
   if( !brief.isEmpty() )
   {
      stream.writeText( "@brief " );
      stream.writeText( brief );
      stream.writeText( "\r\n\r\n" );
   }
   
   if( text )
      stream.writeText( text );
#ifdef USE_UNDOCUMENTED_GROUP
   if( checkUngrouped && ( !text || !dStrstr( text, "@ingroup" ) ) )
   {
      smDocGroups.insertUnique( "UNDOCUMENTED", 0 );
      stream.writeText( "\r\n@ingroup UNDOCUMENTED\r\n" );
   }
#endif
}

static void dumpFragment( Stream& stream, ConsoleDocFragment* fragment )
{
   if( !fragment->mText || !fragment->mText[ 0 ] )
      return;
      
   // Emit doc text in comment.
      
   stream.writeText( "/*!\r\n" );
   stream.writeText( fragment->mText );
   stream.writeText( "*/\r\n\r\n" );
   
   // Emit definition, if any.
   
   if( fragment->mDefinition )
   {
      stream.writeText( fragment->mDefinition );
      stream.writeText( "\r\n" );
   }
}

static void dumpVariable(  Stream& stream,
                           Dictionary::Entry* entry,
                           const char* inClass = NULL )
{
   // Skip variables defined in script.
   
   if( entry->type < 0 )
      return;
         
   // Skip internals... don't export them.
   if (  entry->mUsage &&
         ( dStrstr( entry->mUsage, "@hide" ) || dStrstr( entry->mUsage, "@internal" ) ) )
      return;

   // Split up qualified name.

   Vector< String > nameComponents;
   String( entry->name ).split( "::", nameComponents );
   if( !nameComponents.size() ) // Safety check.
      return;
      
   // Match filter.
   
   if( inClass )
   {
      // Make sure first qualifier in name components is a
      // namespace qualifier matching the given class name.
      
      if( nameComponents.size() <= 1 || dStricmp( nameComponents.first().c_str() + 1, inClass ) != 0 ) // Skip '$'.
         return;
   }
   else
   {
      // Make sure, this is *not* in a class namespace.
      
      if( nameComponents.size() > 1 && Con::lookupNamespace( nameComponents.first().c_str() + 1 )->mClassRep )
         return;
   }
            
   // Skip variables for which we can't decipher their type.

   ConsoleBaseType* type = ConsoleBaseType::getType( entry->type );
   if( !type )
   {
      Con::errorf( "Can't find type for variable '%s'", entry->name );
      return;
   }

   // Write doc comment.
   
   stream.writeText( "/*!\r\n" );
   
   if( !inClass )
   {
      stream.writeText( "@var " );
      stream.writeText( type->getTypeClassName() );
      stream.writeText( " " );
      stream.writeText( entry->name );
      stream.writeText( ";\r\n" );
   }
   
   dumpDoc( stream, entry->mUsage );
   
   stream.writeText( "*/\r\n" );
   
   // Write definition.
   
   const U32 numNameComponents = nameComponents.size();
   if( !inClass && numNameComponents > 1 )
      for( U32 i = 0; i < ( numNameComponents - 1 ); ++ i )
      {
         stream.writeText( "namespace " );
         stream.writeText( nameComponents[ i ] );
         stream.writeText( " { " );
      }
   
   if( inClass )
      stream.writeText( "static " );
      
   if( entry->mIsConstant )
      stream.writeText( "const " );
      
   stream.writeText( type->getTypeClassName() );
   stream.writeText( " " );
   stream.writeText( nameComponents.last() );
   stream.writeText( ";" );
   
   if( !inClass && numNameComponents > 1 )
      for( U32 i = 0; i < ( numNameComponents - 1 ); ++ i )
         stream.writeText( " } " );
         
   stream.writeText( "\r\n" );
}

static void dumpVariables( Stream& stream, const char* inClass = NULL )
{
   const U32 hashTableSize = gEvalState.globalVars.hashTable->size;
   for( U32 i = 0; i < hashTableSize; ++ i )
      for( Dictionary::Entry* entry = gEvalState.globalVars.hashTable->data[ i ]; entry != NULL; entry = entry->nextEntry )
         dumpVariable( stream, entry, inClass );
}

static void dumpFunction(  Stream &stream,
                           bool isClassMethod,
                           Namespace::Entry* entry )
{
   String doc = entry->getDocString().trim();
   String prototype = entry->getPrototypeString().trim();
   
   // If the doc string contains @hide, skip this function.
   
   if( dStrstr( doc.c_str(), "@hide" ) || dStrstr( doc.c_str(), "@internal" ) )
      return;
   
   // Make sure we have a valid function prototype.
   
   if( prototype.isEmpty() )
   {
      Con::errorf( "Function '%s::%s' has no prototype!", entry->mNamespace->mName, entry->mFunctionName );
      return;
   }
   
   // See if it's a static method.
   
   bool isStaticMethod = false;
   if( entry->mHeader )
      isStaticMethod = entry->mHeader->mIsStatic;
      
   // Emit the doc comment.

   if( !doc.isEmpty() )
   {
      stream.writeText( "/*!\r\n" );

      // If there's no @brief, take the first line of the doc text body
      // as the description.

      const char* brief = dStrstr( doc, "@brief" );
      if( !brief )
      {
         String brief = entry->getBriefDescription( &doc );
         
         brief.trim();
         if( !brief.isEmpty() )
         {
            stream.writeText( "@brief " );
            stream.writeText( brief );
            stream.writeText( "\r\n\r\n" );
         }
      }

      stream.writeText( doc );
      
      // Emit @ingroup if it's not a class method.

      if ( !isClassMethod && !isStaticMethod ) // Extra static method check for static classes (which will come out as non-class namespaces).
      {
         const char *group = dStrstr( doc, "@ingroup" );
         if( group )
         {
            char groupName[ 256 ] = { 0 };
            dSscanf( group, "@ingroup %s", groupName );
            smDocGroups.insertUnique( groupName, 0 );
         }
#ifdef USE_UNDOCUMENTED_GROUP
         else
         {
            smDocGroups.insertUnique( "UNDOCUMENTED", 0 );
            stream.writeText( "\r\n@ingroup UNDOCUMENTED\r\n" );
         }
#endif
      }
      
      stream.writeText( "*/\r\n" );
   }
#ifdef USE_UNDOCUMENTED_GROUP
   else if( !isClassMethod )
   {
      smDocGroups.insertUnique( "UNDOCUMENTED", 0 );
      stream.writeText( "/*! UNDOCUMENTED!\r\n@ingroup UNDOCUMENTED\r\n */\r\n" );
   }
#endif
      
   if( isStaticMethod )
      stream.writeText( "static " );
      
   stream.writeText( prototype );
   stream.writeText( ";\r\n" );
}

static void dumpNamespaceEntries( Stream &stream, Namespace *g, bool callbacks = false )
{
   /// Only print virtual on methods that are members of
   /// classes as this allows doxygen to properly do overloads.
   const bool isClassMethod = g->mClassRep != NULL;

   // Go through all the entries in the namespace.
   for ( Namespace::Entry *ewalk = g->mEntryList; ewalk; ewalk = ewalk->mNext )
   {
      int eType = ewalk->mType;

      // We do not dump script defined functions... only engine exports.
      if(    eType == Namespace::Entry::ConsoleFunctionType
          || eType == Namespace::Entry::GroupMarker )
         continue;

      if( eType == Namespace::Entry::ScriptCallbackType )
      {
         if( !callbacks )
            continue;
      }
      else if( callbacks )
         continue;

      dumpFunction( stream, isClassMethod, ewalk );
   }
}

static void dumpClassHeader(  Stream &stream, 
                              const char *usage, 
                              const char *className, 
                              const char *superClassName )
{
   if ( usage )
   {
      stream.writeText( "/*!\r\n" );
      stream.writeText( usage );

      const char *group = dStrstr( usage, "@ingroup" );
      if ( group )
      {
         char groupName[256] = { 0 };
         dSscanf( group, "@ingroup %s", groupName );
         smDocGroups.insertUnique( groupName, 0 );
      }
#ifdef USE_UNDOCUMENTED_GROUP
      else
      {
         smDocGroups.insertUnique( "UNDOCUMENTED", 0 );
         stream.writeText( "\r\n@ingroup UNDOCUMENTED\r\n" );
      }
#endif
      
      stream.writeText( "\r\n*/\r\n" );
   }
   else
   {
      // No documentation string.  Check whether ther is a separate
      // class doc fragement.
      
      bool haveClassDocFragment = false;
      if( className )
      {
         char buffer[ 1024 ];
         dSprintf( buffer, sizeof( buffer ), "@class %s", className );
         
         for(  ConsoleDocFragment* fragment = ConsoleDocFragment::smFirst;
               fragment != NULL; fragment = fragment->mNext )
            if( !fragment->mClass && dStrstr( fragment->mText, buffer ) != NULL )
            {
               haveClassDocFragment = true;
               break;
            }
      }
#ifdef USE_UNDOCUMENTED_GROUP
      if( !haveClassDocFragment )
      {
         smDocGroups.insertUnique( "UNDOCUMENTED", 0 );
         stream.writeText( "/*! UNDOCUMENTED!\r\n@ingroup UNDOCUMENTED\r\n */\r\n" );
      }
#endif
   }

   // Print out appropriate class header
   if ( superClassName )
      stream.writeText( String::ToString( "class %s : public %s {\r\npublic:\r\n", className, superClassName ? superClassName : "" ) );
   else if ( className )
      stream.writeText( String::ToString( "class %s {\r\npublic:\r\n", className ) );
   else
      stream.writeText( "namespace {\r\n" );
}

static void dumpClassMember(  Stream &stream,
                              const AbstractClassRep::Field& field )
{
   stream.writeText( "/*!\r\n" );

   if( field.pFieldDocs && field.pFieldDocs[ 0 ] )
   {
      stream.writeText( "@brief " );
      
      String docs( field.pFieldDocs );
      S32 newline = docs.find( '\n' );
      if( newline == -1 )
         stream.writeText( field.pFieldDocs );
      else
      {
         String brief = docs.substr( 0, newline );
         String body = docs.substr( newline + 1 );
         
         stream.writeText( brief );
         stream.writeText( "\r\n\r\n" );
         stream.writeText( body );
      }
      
      stream.writeText( "\r\n" );
   }

   const bool isDeprecated = ( field.type == AbstractClassRep::DeprecatedFieldType );
   if( isDeprecated )
      stream.writeText( "@deprecated This member is deprecated and its value is always undefined.\r\n" );

   stream.writeText( "*/\r\n" );
  
   ConsoleBaseType* cbt = ConsoleBaseType::getType( field.type );
   const char* type = ( cbt ? cbt->getTypeClassName() : "" );
   
   if( field.elementCount > 1 )
      stream.writeText( String::ToString( "%s %s[ %i ];\r\n", isDeprecated ? "deprecated" : type, field.pFieldname, field.elementCount ) );
   else
      stream.writeText( String::ToString( "%s %s;\r\n", isDeprecated ? "deprecated" : type, field.pFieldname ) );
}

static void dumpClassFooter( Stream &stream )
{
   stream.writeText( "};\r\n\r\n" );
}

static void dumpGroupStart(   Stream &stream,
                              const char *aName, 
                              const char *aDocs = NULL )
{
   stream.writeText( String::ToString( "\r\n/*! @name %s\r\n", aName ) );

   if ( aDocs )
   {
      stream.writeText( aDocs );
      stream.writeText( "\r\n" );
   }

   stream.writeText( "@{ */\r\n" );

   // Add a blank comment in order to make sure groups are parsed properly.
   //Con::printf("   /*! */");
}

static void dumpGroupEnd( Stream &stream )
{
   stream.writeText( "/// @}\r\n\r\n" );
}

static void dumpClasses( Stream &stream )
{
   Namespace::trashCache();

   VectorPtr<Namespace*> vec;
   vec.reserve( 1024 );

   // We use mHashSequence to mark if we have traversed...
   // so mark all as zero to start.
   for ( Namespace *walk = Namespace::mNamespaceList; walk; walk = walk->mNext )
      walk->mHashSequence = 0;

   for(Namespace *walk = Namespace::mNamespaceList; walk; walk = walk->mNext)
   {
      VectorPtr<Namespace*> stack;
      stack.reserve( 1024 );

      // Get all the parents of this namespace... (and mark them as we go)
      Namespace *parentWalk = walk;
      while(parentWalk)
      {
         if(parentWalk->mHashSequence != 0)
            break;
         if(parentWalk->mPackage == 0)
         {
            parentWalk->mHashSequence = 1;   // Mark as traversed.
            stack.push_back(parentWalk);
         }
         parentWalk = parentWalk->mParent;
      }

      // Load stack into our results vector.
      while(stack.size())
      {
         vec.push_back(stack[stack.size() - 1]);
         stack.pop_back();
      }
   }

   // Go through previously discovered classes
   U32 i;
   for(i = 0; i < vec.size(); i++)
   {
      const char *className = vec[i]->mName;
      const char *superClassName = vec[i]->mParent ? vec[i]->mParent->mName : NULL;

      // Skip the global namespace, that gets dealt with in dumpFunctions
      if(!className) 
         continue;

      // We're just dumping engine functions, then we don't want to dump
      // a class that only contains script functions. So, we iterate over 
      // all the functions.
      bool found = false;
      for( Namespace::Entry *ewalk = vec[i]->mEntryList; ewalk; ewalk = ewalk->mNext )
      {
         if( ewalk->mType != Namespace::Entry::ConsoleFunctionType )
         {
            found = true;
            break;
         }
      }

      // If we don't have engine functions and the namespace name
      // doesn't match the class name... then its a script class.
      if ( !found && !vec[i]->isClass() )
         continue;
  
      // If we hit a class with no members and no classRep, do clever filtering.
      if(vec[i]->mEntryList == NULL && vec[i]->mClassRep == NULL)
      {
         // Print out a short stub so we get a proper class hierarchy.
         if ( superClassName )  
         { 
            // Filter hack; we don't want non-inheriting classes...
            dumpClassHeader( stream, NULL, className, superClassName );
            dumpClassFooter( stream );
         }
         continue;
      }

      // Skip over hidden or internal classes.
      if(   vec[i]->mUsage &&
            ( dStrstr( vec[i]->mUsage, "@hide" ) || dStrstr( vec[i]->mUsage, "@internal" ) ) )
         continue;

      // Print the header for the class..
      dumpClassHeader( stream, vec[i]->mUsage, className, superClassName );
      
      // Dump all fragments for this class.
      
      for( ConsoleDocFragment* fragment = ConsoleDocFragment::smFirst; fragment != NULL; fragment = fragment->mNext )
         if( fragment->mClass && dStricmp( fragment->mClass, className ) == 0 )
            dumpFragment( stream, fragment );

      // Dump member functions.
      dumpNamespaceEntries( stream, vec[ i ], false );
      
      // Dump callbacks.
      dumpGroupStart( stream, "Callbacks" );
      dumpNamespaceEntries( stream, vec[ i ], true );
      dumpGroupEnd( stream );
      
      // Dump static member variables.
      dumpVariables( stream, className );

      // Deal with the classRep (to get members)...
      AbstractClassRep *rep = vec[i]->mClassRep;
      AbstractClassRep::FieldList emptyList;
      AbstractClassRep::FieldList *parentList = &emptyList;
      AbstractClassRep::FieldList *fieldList = &emptyList;
      if ( rep )
      {
         // Get information about the parent's fields...
         AbstractClassRep *parentRep = vec[i]->mParent ? vec[i]->mParent->mClassRep : NULL;
         if(parentRep)
            parentList = &(parentRep->mFieldList);

         // Get information about our fields
         fieldList = &(rep->mFieldList);

         // Go through all our fields...
         for(U32 j = 0; j < fieldList->size(); j++)
         {
            const AbstractClassRep::Field &field = (*fieldList)[j];

            switch( field.type )
            {
            case AbstractClassRep::StartArrayFieldType:
            case AbstractClassRep::EndArrayFieldType:
               break;
            case AbstractClassRep::StartGroupFieldType:
               dumpGroupStart( stream, field.pGroupname, field.pFieldDocs );
               break;
            case AbstractClassRep::EndGroupFieldType:
               dumpGroupEnd( stream );
               break;
            default:
            case AbstractClassRep::DeprecatedFieldType:
               // Skip over fields that are already defined in
               // our parent class.
               if ( parentRep && parentRep->findField( field.pFieldname ) )
                  continue;
                     
               dumpClassMember( stream, field );
               break;
            }
         }
      }

      // Close the class/namespace.
      dumpClassFooter( stream );
   }
}

static void dumpEnum( Stream& stream, const EngineTypeInfo* type )
{
   if( !type->getEnumTable() ) // Sanity check.
      return;
      
   // Skip internals... don't export them.
   if (  type->getDocString() &&
         ( dStrstr( type->getDocString(), "@hide" ) || dStrstr( type->getDocString(), "@internal" ) ) )
      return;

   // Write documentation.
   
   stream.writeText( "/*!\r\n" );
   dumpDoc( stream, type->getDocString() );
   stream.writeText( "*/\r\n" );
   
   // Write definition.
   
   stream.writeText( "enum " );
   stream.writeText( type->getTypeName() );
   stream.writeText( " {\r\n" );
   
   const EngineEnumTable& table = *( type->getEnumTable() );
   const U32 numValues = table.getNumValues();
   
   for( U32 i = 0; i < numValues; ++ i )
   {
      const EngineEnumTable::Value& value = table[ i ];
      
      stream.writeText( "/*!\r\n" );
      dumpDoc( stream, value.getDocString(), false );
      stream.writeText( "*/\r\n" );
      stream.writeText( value.getName() );
      stream.writeText( ",\r\n" );
   }
   
   stream.writeText( "};\r\n" );
}

static void dumpEnums( Stream& stream )
{
   for( const EngineTypeInfo* type = EngineTypeInfo::getFirstType();
        type != NULL; type = type->getNextType() )
      if( type->isEnum() || type->isBitfield() )
         dumpEnum( stream, type );
}

static bool dumpEngineDocs( const char *outputFile )
{
   // Create the output stream.
   FileStream stream;
   if ( !stream.open( outputFile, Torque::FS::File::Write ) )
   {
      Con::errorf( "dumpEngineDocs - Failed to open output file." );
      return false;
   }

   // First dump all global ConsoleDoc fragments.
   
   for( ConsoleDocFragment* fragment = ConsoleDocFragment::smFirst; fragment != NULL; fragment = fragment->mNext )
      if( !fragment->mClass )
         dumpFragment( stream, fragment );
   
   // Clear the doc groups before continuing,
   smDocGroups.clear();
   
   // Dump enumeration types.
   dumpEnums( stream );
   
   // Dump all global variables.
   dumpVariables( stream );

   // Now dump the global functions.
   Namespace *g = Namespace::find( NULL );
   while( g ) 
   {
      dumpNamespaceEntries( stream, g );
      
      // Dump callbacks.
      dumpGroupStart( stream, "Callbacks" );
      dumpNamespaceEntries( stream, g, true );
      dumpGroupEnd( stream );

      g = g->mParent;
   }

   // Now dump all the classes.
   dumpClasses( stream );

   // Dump pre-declarations for any groups we encountered
   // so that we don't have to explicitly define them.
   HashTable<String,U32>::Iterator iter = smDocGroups.begin();
   for ( ; iter != smDocGroups.end(); iter++ )
      stream.writeText( String::ToString( "/*! @addtogroup %s */\r\n\r\n", iter->key.c_str() ) );

   return true;
}

DefineEngineFunction( dumpEngineDocs, bool, ( const char* outputFile ),,
                     "Dumps the engine scripting documentation to the specified file overwriting any existing content.\n"
                     "@param outputFile The relative or absolute output file path and name.\n"
                     "@return Returns true if successful.\n"
                     "@ingroup Console")
{
   return dumpEngineDocs( outputFile );
}

