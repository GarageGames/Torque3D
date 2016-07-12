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
#include "console/consoleObject.h"

#include "core/stringTable.h"
#include "core/crc.h"
#include "core/dataChunker.h"
#include "console/console.h"
#include "console/consoleInternal.h"
#include "console/typeValidators.h"
#include "console/simObject.h"
#include "console/engineTypes.h"
#include "console/engineAPI.h"


IMPLEMENT_SCOPE( ConsoleAPI, Console,,
   "Functionality related to the legacy TorqueScript console system." );

IMPLEMENT_NONINSTANTIABLE_CLASS( ConsoleObject,
   "Legacy console system root class.  Will disappear." )
END_IMPLEMENT_CLASS;


AbstractClassRep *                 AbstractClassRep::classLinkList = NULL;
AbstractClassRep::FieldList        sg_tempFieldList( __FILE__, __LINE__ );
U32                                AbstractClassRep::NetClassCount  [NetClassGroupsCount][NetClassTypesCount] = {{0, },};
U32                                AbstractClassRep::NetClassBitSize[NetClassGroupsCount][NetClassTypesCount] = {{0, },};

AbstractClassRep **                AbstractClassRep::classTable[NetClassGroupsCount][NetClassTypesCount];

U32                                AbstractClassRep::classCRC[NetClassGroupsCount] = {CRC::INITIAL_CRC_VALUE, };
bool                               AbstractClassRep::initialized = false;



//-----------------------------------------------------------------------------
AbstractClassRep* AbstractClassRep::findFieldRoot(StringTableEntry fieldName)
{
   // Find the field.
   const Field* pField = findField(fieldName);

   // Finish if not found.
   if (pField == NULL)
      return NULL;

   // We're the root if we have no parent.
   if (getParentClass() == NULL)
      return this;

   // Find the field root via the parent.
   AbstractClassRep* pFieldRoot = getParentClass()->findFieldRoot(fieldName);

   // We're the root if the parent does not have it else return the field root.
   return pFieldRoot == NULL ? this : pFieldRoot;
}

void AbstractClassRep::init()
{
   // Only add the renderable and selectable globals for
   // classes derived from SceneObject which are the only
   // objects for which these work.
   if ( isSubclassOf( "SceneObject" ) )
   {
      Con::addVariable( avar( "$%s::isRenderable", getClassName() ), TypeBool, &mIsRenderEnabled,
         "@brief Disables rendering of all instances of this type.\n\n" );

      Con::addVariable( avar( "$%s::isSelectable", getClassName() ), TypeBool, &mIsSelectionEnabled,
         "@brief Disables selection of all instances of this type.\n\n" );
   }
}

const AbstractClassRep::Field *AbstractClassRep::findField(StringTableEntry name) const
{
   for(U32 i = 0; i < mFieldList.size(); i++)
      if(mFieldList[i].pFieldname == name)
         return &mFieldList[i];

   return NULL;
}

AbstractClassRep* AbstractClassRep::findClassRep(const char* in_pClassName)
{
   AssertFatal(initialized,
      "AbstractClassRep::findClassRep() - Tried to find an AbstractClassRep before AbstractClassRep::initialize().");

   for (AbstractClassRep *walk = classLinkList; walk; walk = walk->nextClass)
      if (!dStricmp(walk->getClassName(), in_pClassName))
         return walk;

   return NULL;
}

AbstractClassRep* AbstractClassRep::findClassRep( U32 groupId, U32 typeId, U32 classId )
{
   AssertFatal(initialized,
      "AbstractClassRep::findClasRep() - Tried to create an object before AbstractClassRep::initialize().");
   AssertFatal(classId < NetClassCount[groupId][typeId],
      "AbstractClassRep::findClassRep() - Class id out of range.");
   AssertFatal(classTable[groupId][typeId][classId] != NULL,
      "AbstractClassRep::findClassRep() - No class with requested ID type.");

   // Look up the specified class and create it.
   if(classTable[groupId][typeId][classId])
      return classTable[groupId][typeId][classId];
      
   return NULL;
}

//--------------------------------------
void AbstractClassRep::registerClassRep(AbstractClassRep* in_pRep)
{
   AssertFatal(in_pRep != NULL, "AbstractClassRep::registerClassRep was passed a NULL pointer!");

#ifdef TORQUE_DEBUG  // assert if this class is already registered.
   for(AbstractClassRep *walk = classLinkList; walk; walk = walk->nextClass)
   {
      AssertFatal(dStrcmp(in_pRep->mClassName, walk->mClassName),
         "Duplicate class name registered in AbstractClassRep::registerClassRep()");
   }
#endif

   in_pRep->nextClass = classLinkList;
   classLinkList = in_pRep;
}

//--------------------------------------
void AbstractClassRep::removeClassRep(AbstractClassRep* in_pRep)
{
   for( AbstractClassRep *walk = classLinkList; walk; walk = walk->nextClass )
   {
      // This is the case that will most likely get hit.
      if( walk->nextClass == in_pRep ) 
         walk->nextClass = walk->nextClass->nextClass;
      else if( walk == in_pRep )
      {
         AssertFatal( in_pRep == classLinkList, "Pat failed in his logic for un linking RuntimeClassReps from the class linked list" );
         classLinkList = in_pRep->nextClass;
      }
   }
}

//--------------------------------------

ConsoleObject* AbstractClassRep::create(const char* in_pClassName)
{
   AssertFatal(initialized,
      "AbstractClassRep::create() - Tried to create an object before AbstractClassRep::initialize().");

   const AbstractClassRep *rep = AbstractClassRep::findClassRep(in_pClassName);
   if(rep)
      return rep->create();

   AssertWarn(0, avar("Couldn't find class rep for dynamic class: %s", in_pClassName));
   return NULL;
}

//--------------------------------------
ConsoleObject* AbstractClassRep::create(const U32 groupId, const U32 typeId, const U32 in_classId)
{
   AbstractClassRep* classRep = findClassRep( groupId, typeId, in_classId );
   if( !classRep )
      return NULL;

   return classRep->create();
}

//--------------------------------------

static S32 QSORT_CALLBACK ACRCompare(const void *aptr, const void *bptr)
{
   const AbstractClassRep *a = *((const AbstractClassRep **) aptr);
   const AbstractClassRep *b = *((const AbstractClassRep **) bptr);

   if(a->mClassType != b->mClassType)
      return a->mClassType - b->mClassType;
   return dStrnatcasecmp(a->getClassName(), b->getClassName());
}

void AbstractClassRep::initialize()
{
   AssertFatal(!initialized, "Duplicate call to AbstractClassRep::initialize()!");
   Vector<AbstractClassRep *> dynamicTable(__FILE__, __LINE__);

   AbstractClassRep *walk;

   // Initialize namespace references...
   for (walk = classLinkList; walk; walk = walk->nextClass)
   {
      walk->mNamespace = Con::lookupNamespace(StringTable->insert(walk->getClassName()));
      walk->mNamespace->mUsage = walk->getDocString();
      walk->mNamespace->mClassRep = walk;
   }

   // Initialize field lists... (and perform other console registration).
   for (walk = classLinkList; walk; walk = walk->nextClass)
   {
      // sg_tempFieldList is used as a staging area for field lists
      // (see addField, addGroup, etc.)
      sg_tempFieldList.setSize(0);

      walk->init();

      // So if we have things in it, copy it over...
      if (sg_tempFieldList.size() != 0)
         walk->mFieldList = sg_tempFieldList;

      // And of course delete it every round.
      sg_tempFieldList.clear();
   }

   // Calculate counts and bit sizes for the various NetClasses.
   for (U32 group = 0; group < NetClassGroupsCount; group++)
   {
      U32 groupMask = 1 << group;

      // Specifically, for each NetClass of each NetGroup...
      for(U32 type = 0; type < NetClassTypesCount; type++)
      {
         // Go through all the classes and find matches...
         for (walk = classLinkList; walk; walk = walk->nextClass)
         {
            if(walk->mClassType == type && walk->mClassGroupMask & groupMask)
               dynamicTable.push_back(walk);
         }

         // Set the count for this NetGroup and NetClass
         NetClassCount[group][type] = dynamicTable.size();
         if(!NetClassCount[group][type])
            continue; // If no classes matched, skip to next.

         // Sort by type and then by name.
         dQsort((void *) &dynamicTable[0], dynamicTable.size(), sizeof(AbstractClassRep *), ACRCompare);

         // Allocate storage in the classTable
         classTable[group][type] = new AbstractClassRep*[NetClassCount[group][type]];

         // Fill this in and assign class ids for this group.
         for(U32 i = 0; i < NetClassCount[group][type];i++)
         {
            classTable[group][type][i] = dynamicTable[i];
            dynamicTable[i]->mClassId[group] = i;
         }

         // And calculate the size of bitfields for this group and type.
         NetClassBitSize[group][type] =
               getBinLog2(getNextPow2(NetClassCount[group][type] + 1));
         AssertFatal(NetClassCount[group][type] < (1 << NetClassBitSize[group][type]), "NetClassBitSize too small!");

         dynamicTable.clear();
      }
   }

   // Ok, we're golden!
   initialized = true;
}

void AbstractClassRep::shutdown()
{
   AssertFatal( initialized, "AbstractClassRep::shutdown - not initialized" );

   // Release storage allocated to the class table.

   for (U32 group = 0; group < NetClassGroupsCount; group++)
      for(U32 type = 0; type < NetClassTypesCount; type++)
         if( classTable[ group ][ type ] )
            SAFE_DELETE_ARRAY( classTable[ group ][ type ] );

   initialized = false;
}

AbstractClassRep *AbstractClassRep::getCommonParent( const AbstractClassRep *otherClass ) const
{
   // CodeReview: This may be a noob way of doing it. There may be some kind of
   // super-spiffy algorithm to do what the code below does, but this appeared
   // to make sense to me, and it is pretty easy to see what it is doing [6/23/2007 Pat]

   static VectorPtr<AbstractClassRep *> thisClassHeirarchy;
   thisClassHeirarchy.clear();

   AbstractClassRep *walk = const_cast<AbstractClassRep *>( this );

   while( walk != NULL )
   {
      thisClassHeirarchy.push_front( walk );
      walk = walk->getParentClass();
   }

   static VectorPtr<AbstractClassRep *> compClassHeirarchy;
   compClassHeirarchy.clear();
   walk = const_cast<AbstractClassRep *>( otherClass );
   while( walk != NULL )
   {
      compClassHeirarchy.push_front( walk );
      walk = walk->getParentClass();
   }

   // Make sure we only iterate over the list the number of times we can
   S32 maxIterations = getMin( compClassHeirarchy.size(), thisClassHeirarchy.size() );

   U32 i = 0;
   for( ; i < maxIterations; i++ )
   {
      if( compClassHeirarchy[i] != thisClassHeirarchy[i] )
         break;
   }

   return compClassHeirarchy[i];
}

//------------------------------------------------------------------------------
//-------------------------------------- ConsoleObject

static char replacebuf[1024];
static char* suppressSpaces(const char* in_pname)
{
	U32 i = 0;
	char chr;
	do
	{
		chr = in_pname[i];
		replacebuf[i++] = (chr != 32) ? chr : '_';
	} while(chr);

	return replacebuf;
}

void ConsoleObject::addGroup(const char* in_pGroupname, const char* in_pGroupDocs)
{
   // Remove spaces.
   char* pFieldNameBuf = suppressSpaces(in_pGroupname);

   // Append group type to fieldname.
   dStrcat(pFieldNameBuf, "_begingroup");

   // Create Field.
   AbstractClassRep::Field f;
   f.pFieldname   = StringTable->insert(pFieldNameBuf);
   f.pGroupname   = in_pGroupname;

   if(in_pGroupDocs)
      f.pFieldDocs   = in_pGroupDocs;

   f.type         = AbstractClassRep::StartGroupFieldType;
   f.elementCount = 0;
   f.groupExpand  = false;
   f.validator    = NULL;
   f.setDataFn    = &defaultProtectedSetFn;
   f.getDataFn    = &defaultProtectedGetFn;
   f.writeDataFn = &defaultProtectedWriteFn;

   // Add to field list.
   sg_tempFieldList.push_back(f);
}

void ConsoleObject::endGroup(const char*  in_pGroupname)
{
   // Remove spaces.
   char* pFieldNameBuf = suppressSpaces(in_pGroupname);

   // Append group type to fieldname.
   dStrcat(pFieldNameBuf, "_endgroup");

   // Create Field.
   AbstractClassRep::Field f;
   f.pFieldname   = StringTable->insert(pFieldNameBuf);
   f.pGroupname   = in_pGroupname;
   f.type         = AbstractClassRep::EndGroupFieldType;
   f.groupExpand  = false;
   f.validator    = NULL;
   f.setDataFn    = &defaultProtectedSetFn;
   f.getDataFn    = &defaultProtectedGetFn;
   f.writeDataFn = &defaultProtectedWriteFn;
   f.elementCount = 0;

   // Add to field list.
   sg_tempFieldList.push_back(f);
}

void ConsoleObject::addArray( const char *arrayName, S32 count )
{
   char *nameBuff = suppressSpaces(arrayName);
   dStrcat(nameBuff, "_beginarray");

   // Create Field.
   AbstractClassRep::Field f;
   f.pFieldname   = StringTable->insert(nameBuff);
   f.pGroupname   = arrayName;

   f.type         = AbstractClassRep::StartArrayFieldType;
   f.elementCount = count;
   f.groupExpand  = false;
   f.validator    = NULL;
   f.setDataFn    = &defaultProtectedSetFn;
   f.getDataFn    = &defaultProtectedGetFn;
   f.writeDataFn = &defaultProtectedWriteFn;

   // Add to field list.
   sg_tempFieldList.push_back(f);
}

void ConsoleObject::endArray( const char *arrayName )
{
   char *nameBuff = suppressSpaces(arrayName);
   dStrcat(nameBuff, "_endarray");

   // Create Field.
   AbstractClassRep::Field f;
   f.pFieldname   = StringTable->insert(nameBuff);
   f.pGroupname   = arrayName;
   f.type         = AbstractClassRep::EndArrayFieldType;
   f.groupExpand  = false;
   f.validator    = NULL;
   f.setDataFn    = &defaultProtectedSetFn;
   f.getDataFn    = &defaultProtectedGetFn;
   f.writeDataFn = &defaultProtectedWriteFn;
   f.elementCount = 0;

   // Add to field list.
   sg_tempFieldList.push_back(f);
}

void ConsoleObject::addField(const char*  in_pFieldname,
                       const U32 in_fieldType,
                       const dsize_t in_fieldOffset,
                       const char* in_pFieldDocs,
                       U32 flags )
{
   addField(
      in_pFieldname,
      in_fieldType,
      in_fieldOffset,
      1,
      in_pFieldDocs,
      flags );
}

void ConsoleObject::addField(const char*  in_pFieldname,
   const U32 in_fieldType,
   const dsize_t in_fieldOffset,
   AbstractClassRep::WriteDataNotify in_writeDataFn,
   const char* in_pFieldDocs,
   U32 flags)
{
   addField(
      in_pFieldname,
      in_fieldType,
      in_fieldOffset,
      in_writeDataFn,
      1,
      in_pFieldDocs,
      flags);
}

void ConsoleObject::addField(const char*  in_pFieldname,
   const U32 in_fieldType,
   const dsize_t in_fieldOffset,
   const U32 in_elementCount,
   const char* in_pFieldDocs,
   U32 flags)
{
   addField(in_pFieldname,
      in_fieldType,
      in_fieldOffset,
      &defaultProtectedWriteFn,
      in_elementCount,
      in_pFieldDocs,
      flags);
}

void ConsoleObject::addField(const char*  in_pFieldname,
   const U32 in_fieldType,
   const dsize_t in_fieldOffset,
   AbstractClassRep::WriteDataNotify in_writeDataFn,
   const U32 in_elementCount,
   const char* in_pFieldDocs,
   U32 flags)
{
   AbstractClassRep::Field f;
   f.pFieldname = StringTable->insert(in_pFieldname);

   if (in_pFieldDocs)
      f.pFieldDocs = in_pFieldDocs;

   f.type = in_fieldType;
   f.offset = in_fieldOffset;
   f.elementCount = in_elementCount;
   f.validator = NULL;
   f.flag = flags;

   f.setDataFn = &defaultProtectedSetFn;
   f.getDataFn = &defaultProtectedGetFn;
   f.writeDataFn = in_writeDataFn;

   ConsoleBaseType* conType = ConsoleBaseType::getType(in_fieldType);
   AssertFatal(conType, "ConsoleObject::addField - invalid console type");
   f.table = conType->getEnumTable();

   sg_tempFieldList.push_back(f);
}

void ConsoleObject::addProtectedField(const char*  in_pFieldname,
   const U32 in_fieldType,
   const dsize_t in_fieldOffset,
   AbstractClassRep::SetDataNotify in_setDataFn,
   AbstractClassRep::GetDataNotify in_getDataFn,
   const char* in_pFieldDocs,
   U32 flags)
{
   addProtectedField(
      in_pFieldname,
      in_fieldType,
      in_fieldOffset,
      in_setDataFn,
      in_getDataFn,
      &defaultProtectedWriteFn,
      1,
      in_pFieldDocs,
      flags);
}

void ConsoleObject::addProtectedField(const char*  in_pFieldname,
   const U32 in_fieldType,
   const dsize_t in_fieldOffset,
   AbstractClassRep::SetDataNotify in_setDataFn,
   AbstractClassRep::GetDataNotify in_getDataFn,
   AbstractClassRep::WriteDataNotify in_writeDataFn,
   const char* in_pFieldDocs,
   U32 flags)
{
   addProtectedField(
      in_pFieldname,
      in_fieldType,
      in_fieldOffset,
      in_setDataFn,
      in_getDataFn,
      in_writeDataFn,
      1,
      in_pFieldDocs,
      flags);
}

void ConsoleObject::addProtectedField(const char*  in_pFieldname,
   const U32 in_fieldType,
   const dsize_t in_fieldOffset,
   AbstractClassRep::SetDataNotify in_setDataFn,
   AbstractClassRep::GetDataNotify in_getDataFn,
   const U32 in_elementCount,
   const char* in_pFieldDocs,
   U32 flags)
{
   addProtectedField(
      in_pFieldname,
      in_fieldType,
      in_fieldOffset,
      in_setDataFn,
      in_getDataFn,
      &defaultProtectedWriteFn,
      in_elementCount,
      in_pFieldDocs,
      flags);
}
void ConsoleObject::addProtectedField(const char*  in_pFieldname,
   const U32 in_fieldType,
   const dsize_t in_fieldOffset,
   AbstractClassRep::SetDataNotify in_setDataFn,
   AbstractClassRep::GetDataNotify in_getDataFn,
   AbstractClassRep::WriteDataNotify in_writeDataFn,
   const U32 in_elementCount,
   const char* in_pFieldDocs,
   U32 flags)
{
   AbstractClassRep::Field f;
   f.pFieldname = StringTable->insert(in_pFieldname);

   if (in_pFieldDocs)
      f.pFieldDocs = in_pFieldDocs;

   f.type = in_fieldType;
   f.offset = in_fieldOffset;
   f.elementCount = in_elementCount;
   f.validator = NULL;
   f.flag = flags;

   f.setDataFn = in_setDataFn;
   f.getDataFn = in_getDataFn;
   f.writeDataFn = in_writeDataFn;

   ConsoleBaseType* conType = ConsoleBaseType::getType(in_fieldType);
   AssertFatal(conType, "ConsoleObject::addProtectedField - invalid console type");
   f.table = conType->getEnumTable();

   sg_tempFieldList.push_back(f);
}

void ConsoleObject::addFieldV(const char*  in_pFieldname,
                       const U32 in_fieldType,
                       const dsize_t in_fieldOffset,
                       TypeValidator *v,
                       const char* in_pFieldDocs)
{
   AbstractClassRep::Field f;
   f.pFieldname   = StringTable->insert(in_pFieldname);
   if(in_pFieldDocs)
      f.pFieldDocs   = in_pFieldDocs;
   f.type         = in_fieldType;
   f.offset       = in_fieldOffset;
   f.elementCount = 1;
   f.table        = NULL;
   f.setDataFn    = &defaultProtectedSetFn;
   f.getDataFn    = &defaultProtectedGetFn;
   f.writeDataFn = &defaultProtectedWriteFn;
   f.validator    = v;
   v->fieldIndex  = sg_tempFieldList.size();

   sg_tempFieldList.push_back(f);
}

void ConsoleObject::addDeprecatedField(const char *fieldName)
{
   AbstractClassRep::Field f;
   f.pFieldname   = StringTable->insert(fieldName);
   f.type         = AbstractClassRep::DeprecatedFieldType;
   f.offset       = 0;
   f.elementCount = 0;
   f.table        = NULL;
   f.validator    = NULL;
   f.setDataFn    = &defaultProtectedSetFn;
   f.getDataFn    = &defaultProtectedGetFn;
   f.writeDataFn = &defaultProtectedWriteFn;

   sg_tempFieldList.push_back(f);
}


bool ConsoleObject::removeField(const char* in_pFieldname)
{
   for (U32 i = 0; i < sg_tempFieldList.size(); i++) {
      if (dStricmp(in_pFieldname, sg_tempFieldList[i].pFieldname) == 0) {
         sg_tempFieldList.erase(i);
         return true;
      }
   }

   return false;
}

//--------------------------------------
void ConsoleObject::initPersistFields()
{
}

//--------------------------------------
void ConsoleObject::consoleInit()
{
}

//--------------------------------------
AbstractClassRep* ConsoleObject::getClassRep() const
{
   return NULL;
}

String ConsoleObject::_getLogMessage(const char* fmt, va_list args) const
{
   String objClass = "UnknownClass";
   if(getClassRep())
      objClass = getClassRep()->getClassName();
   
   String formattedMessage = String::VToString(fmt, args);
   return String::ToString("%s - Object at %x - %s", 
      objClass.c_str(), this, formattedMessage.c_str());
}

void ConsoleObject::logMessage(const char* fmt, ...) const
{
   va_list args;
   va_start(args, fmt);
   Con::printf(_getLogMessage(fmt, args));
   va_end(args);
}

void ConsoleObject::logWarning(const char* fmt, ...) const
{
   va_list args;
   va_start(args, fmt);
   Con::warnf(_getLogMessage(fmt, args));
   va_end(args);
}

void ConsoleObject::logError(const char* fmt, ...) const
{
   va_list args;
   va_start(args, fmt);
   Con::errorf(_getLogMessage(fmt, args));
   va_end(args);
}


//------------------------------------------------------------------------------

static const char* returnClassList( Vector< AbstractClassRep* >& classes, U32 bufSize )
{
   if( !classes.size() )
      return "";
      
   dQsort( classes.address(), classes.size(), sizeof( AbstractClassRep* ), ACRCompare );

   char* ret = Con::getReturnBuffer( bufSize );
   dStrcpy( ret, classes[ 0 ]->getClassName() );
   for( U32 i = 1; i < classes.size(); i ++ )
   {
      dStrcat( ret, "\t" );
      dStrcat( ret, classes[ i ]->getClassName() );
   }
   
   return ret;
}

//------------------------------------------------------------------------------

DefineEngineFunction( isClass, bool,  ( const char* identifier ),,
				"@brief Returns true if the passed identifier is the name of a declared class.\n\n"
				"@ingroup Console")
{
   AbstractClassRep* rep = AbstractClassRep::findClassRep( identifier );
   return rep != NULL;
}

DefineEngineFunction( isMemberOfClass, bool, ( const char* className, const char* superClassName ),,
   "@brief Returns true if the class is derived from the super class.\n\n"
   "If either class doesn't exist this returns false.\n"
   "@param className The class name.\n"
   "@param superClassName The super class to look for.\n"
   "@ingroup Console")
{
   AbstractClassRep *pRep = AbstractClassRep::findClassRep( className );
   while (pRep)
   {
      if( !dStricmp( pRep->getClassName(), superClassName ) )
         return true;
      pRep = pRep->getParentClass();
   }
   return false;
}

DefineEngineFunction( getDescriptionOfClass, const char*, ( const char* className ),,
				"@brief Returns the description string for the named class.\n\n"
				"@param className The name of the class.\n"
				"@return The class description in string format.\n"
				"@ingroup Console")
{
   AbstractClassRep* rep = AbstractClassRep::findClassRep( className );
   if( rep )
      return rep->getDescription();

   Con::errorf( "getDescriptionOfClass - no class called '%s'", className );
   return "";
}

DefineEngineFunction( getCategoryOfClass, const char*,  ( const char* className ),,
				"@brief Returns the category of the given class.\n\n"
				"@param className The name of the class.\n"
				"@ingroup Console")
{
   AbstractClassRep* rep = AbstractClassRep::findClassRep( className );
   if( rep )
      return rep->getCategory();

   Con::errorf( "getCategoryOfClass - no class called '%s'", className );
   return "";
}

DefineEngineFunction( enumerateConsoleClasses, const char*, ( const char* className ), ( "" ),
				"@brief Returns a list of classes that derive from the named class.\n\n"
            "If the named class is omitted this dumps all the classes.\n"
            "@param className The optional base class name.\n"
				"@return A tab delimited list of classes.\n"
            "@ingroup Editors\n"
				"@internal")
{
   AbstractClassRep *base = NULL;    
   if(className && *className)
   {
      base = AbstractClassRep::findClassRep(className);
      if(!base)
         return "";
   }
   
   Vector<AbstractClassRep*> classes;
   U32 bufSize = 0;
   for(AbstractClassRep *rep = AbstractClassRep::getClassList(); rep; rep = rep->getNextClass())
   {
      if( !base || rep->isClass(base))
      {
         classes.push_back(rep);
         bufSize += dStrlen(rep->getClassName()) + 1;
      }
   }

   return returnClassList( classes, bufSize );
}

DefineEngineFunction( enumerateConsoleClassesByCategory, const char*, ( String category ),,
				"@brief Provide a list of classes that belong to the given category.\n\n"
				"@param category The category name.\n"
				"@return A tab delimited list of classes.\n"
				"@ingroup Editors\n"
				"@internal")
{
   U32 categoryLength = category.length();
   
   U32 bufSize = 0;
   Vector< AbstractClassRep* > classes;
   
   for( AbstractClassRep* rep = AbstractClassRep::getClassList(); rep != NULL; rep = rep->getNextClass() )
   {
      const String& repCategory = rep->getCategory();
      
      if( repCategory.length() >= categoryLength
          && ( repCategory.compare( category, categoryLength, String::NoCase ) == 0 )
          && ( repCategory[ categoryLength ] == ' ' || repCategory[ categoryLength ] == '\0' ) )
      {
         classes.push_back( rep );
         bufSize += dStrlen( rep->getClassName() + 1 );
      }
   }

   return returnClassList( classes, bufSize );
}

DefineEngineFunction( dumpNetStats, void, (),,
   "@brief Dumps network statistics for each class to the console.\n\n"

   "The returned <i>avg</i>, <i>min</i> and <i>max</i> values are in bits sent per update.  "
   "The <i>num</i> value is the total number of events collected.\n"

   "@note This method only works when TORQUE_NET_STATS is defined in torqueConfig.h.\n"
   "@ingroup Networking\n" )
{
#ifdef TORQUE_NET_STATS
   for (AbstractClassRep * rep = AbstractClassRep::getClassList(); rep; rep = rep->getNextClass())
   {
      if (rep->mNetStatPack.numEvents || rep->mNetStatUnpack.numEvents || rep->mNetStatWrite.numEvents || rep->mNetStatRead.numEvents)
      {
         Con::printf("class %s net info",rep->getClassName());
         if (rep->mNetStatPack.numEvents)
            Con::printf("   packUpdate: avg (%f), min (%i), max (%i), num (%i)",
                                       F32(rep->mNetStatPack.total)/F32(rep->mNetStatPack.numEvents),
                                       rep->mNetStatPack.min,
                                       rep->mNetStatPack.max,
                                       rep->mNetStatPack.numEvents);
         if (rep->mNetStatUnpack.numEvents)
            Con::printf("   unpackUpdate: avg (%f), min (%i), max (%i), num (%i)",
                                       F32(rep->mNetStatUnpack.total)/F32(rep->mNetStatUnpack.numEvents),
                                       rep->mNetStatUnpack.min,
                                       rep->mNetStatUnpack.max,
                                       rep->mNetStatUnpack.numEvents);
         if (rep->mNetStatWrite.numEvents)
            Con::printf("   write: avg (%f), min (%i), max (%i), num (%i)",
                                       F32(rep->mNetStatWrite.total)/F32(rep->mNetStatWrite.numEvents),
                                       rep->mNetStatWrite.min,
                                       rep->mNetStatWrite.max,
                                       rep->mNetStatWrite.numEvents);
         if (rep->mNetStatRead.numEvents)
            Con::printf("   read: avg (%f), min (%i), max (%i), num (%i)",
                                       F32(rep->mNetStatRead.total)/F32(rep->mNetStatRead.numEvents),
                                       rep->mNetStatRead.min,
                                       rep->mNetStatRead.max,
                                       rep->mNetStatRead.numEvents);
         S32 sum = 0;
         for (S32 i=0; i<32; i++)
            sum  += rep->mDirtyMaskFrequency[i];
         if (sum)
         {
            Con::printf("   Mask bits:");
            for (S32 i=0; i<8; i++)
            {
               F32 avg0  = rep->mDirtyMaskFrequency[i] ? F32(rep->mDirtyMaskTotal[i])/F32(rep->mDirtyMaskFrequency[i]) : 0.0f;
               F32 avg8  = rep->mDirtyMaskFrequency[i+8] ? F32(rep->mDirtyMaskTotal[i+8])/F32(rep->mDirtyMaskFrequency[i+8]) : 0.0f;
               F32 avg16 = rep->mDirtyMaskFrequency[i+16] ? F32(rep->mDirtyMaskTotal[i+16])/F32(rep->mDirtyMaskFrequency[i+16]) : 0.0f;
               F32 avg24 = rep->mDirtyMaskFrequency[i+24] ? F32(rep->mDirtyMaskTotal[i+24])/F32(rep->mDirtyMaskFrequency[i+24]) : 0.0f;
               Con::printf("      %2i - %4i (%6.2f)     %2i - %4i (%6.2f)     %2i - %4i (%6.2f)     %2i - %4i, (%6.2f)",
                  i   ,rep->mDirtyMaskFrequency[i],avg0,
                  i+8 ,rep->mDirtyMaskFrequency[i+8],avg8,
                  i+16,rep->mDirtyMaskFrequency[i+16],avg16,
                  i+24,rep->mDirtyMaskFrequency[i+24],avg24);
            }
         }
      }
      rep->resetNetStats();
   }
#endif
}

DefineEngineFunction( sizeof, S32, ( const char *objectOrClass ),,
				"@brief Determines the memory consumption of a class or object.\n\n"
				"@param objectOrClass The object or class being measured.\n"
				"@return Returns the total size of an object in bytes.\n"
				"@ingroup Debugging\n")
{
   AbstractClassRep *acr = NULL;
   SimObject *obj = Sim::findObject(objectOrClass);
   if(obj)
      acr = obj->getClassRep();

   if(!acr)
      acr = AbstractClassRep::findClassRep(objectOrClass);

   if(acr)
      return acr->getSizeof();

   if(dStricmp("ConsoleObject", objectOrClass) == 0)
     return sizeof(ConsoleObject);

   Con::warnf("could not find a class rep for that object or class name.");
   return 0;
}


DefineEngineFunction(linkNamespaces, bool, ( String childNSName, String parentNSName  ),,
                     "@brief Links childNS to parentNS.\n\n"
                     "Links childNS to parentNS, or nothing if parentNS is NULL.\n"
                     "Will unlink the namespace from previous namespace if a parent already exists.\n"
                     "@internal\n")
{
   StringTableEntry childNSSTE = StringTable->insert(childNSName.c_str());
   StringTableEntry parentNSSTE = StringTable->insert(parentNSName.c_str());
   
   Namespace *childNS = Namespace::find(childNSSTE);
   Namespace *parentNS = Namespace::find(parentNSSTE);
   
   if (!childNS)
   {
      return false;
   }

   Namespace *currentParent = childNS->getParent();
   
   // Link to new NS if applicable
   
   if (currentParent != parentNS)
   {
      if (currentParent != NULL)
      {
         if (!childNS->unlinkClass(currentParent))
         {
            return false;
         }
      }
      
      if (parentNS != NULL)
      {
         return childNS->classLinkTo(parentNS);
      }
   }
   
   return true;
}

