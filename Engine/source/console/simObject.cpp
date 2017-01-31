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
#include "platform/platformMemory.h"
#include "console/simObject.h"
#include "console/console.h"
#include "console/consoleInternal.h"
#include "console/engineAPI.h"
#include "console/simFieldDictionary.h"
#include "console/simPersistID.h"
#include "console/typeValidators.h"
#include "console/arrayObject.h"
#include "console/codeBlock.h"
#include "core/frameAllocator.h"
#include "core/stream/fileStream.h"
#include "core/fileObject.h"
#include "persistence/taml/tamlCustom.h"

IMPLEMENT_CONOBJECT( SimObject );

// See full description in the new CHM manual
ConsoleDocClass( SimObject,
   "@brief Base class for almost all objects involved in the simulation.\n\n"

   "@ingroup Console\n"
);

bool SimObject::smForceId = false;
SimObjectId SimObject::smForcedId = 0;


namespace Sim
{
   // Defined in simManager.cpp
   extern SimGroup *gRootGroup;
   extern SimManagerNameDictionary *gNameDictionary;
   extern SimIdDictionary *gIdDictionary;
   extern U32 gNextObjectId;
}


//-----------------------------------------------------------------------------

SimObject::SimObject()
{
   objectName            = NULL;
   mOriginalName         = NULL;
   mInternalName         = NULL;
   nextNameObject        = (SimObject*)-1;
   nextManagerNameObject = (SimObject*)-1;
   nextIdObject          = NULL;

   mFilename             = NULL;
   mDeclarationLine      = -1;

   mId           = 0;
   mIdString[ 0 ] = '\0';
   mGroup        = 0;
   mNameSpace    = NULL;
   mNotifyList   = NULL;
   mFlags.set( ModStaticFields | ModDynamicFields );

   mFieldDictionary = NULL;
   mCanSaveFieldDictionary =  true;

   mClassName = NULL;
   mSuperClassName = NULL;

   mCopySource = NULL;
   mPersistentId = NULL;
}

//-----------------------------------------------------------------------------

SimObject::~SimObject()
{
   if( mFieldDictionary )
   {
      delete mFieldDictionary;
      mFieldDictionary = NULL;
   }

   // Release persistent ID.
   if( mPersistentId )
   {
      mPersistentId->unresolve();
      mPersistentId->decRefCount();
      mPersistentId = NULL;
   }

   if( mCopySource )
      mCopySource->unregisterReference( &mCopySource );

   AssertFatal(nextNameObject == (SimObject*)-1,avar(
      "SimObject::~SimObject:  Not removed from dictionary: name %s, id %i",
      objectName, mId));
   AssertFatal(nextManagerNameObject == (SimObject*)-1,avar(
      "SimObject::~SimObject:  Not removed from manager dictionary: name %s, id %i",
      objectName,mId));
   AssertFatal(mFlags.test(Added) == 0, "SimObject::object "
      "missing call to SimObject::onRemove");
}

//-----------------------------------------------------------------------------

bool SimObject::processArguments(S32 argc, ConsoleValueRef *argv)
{
   return argc == 0;
}

//-----------------------------------------------------------------------------

void SimObject::initPersistFields()
{
   addGroup( "Ungrouped" );

      addProtectedField( "name", TypeName, Offset(objectName, SimObject), &setProtectedName, &defaultProtectedGetFn, 
         "Optional global name of this object." );
                  
   endGroup( "Ungrouped" );

   addGroup( "Object" );

      addField( "internalName", TypeString, Offset(mInternalName, SimObject), 
         "Optional name that may be used to lookup this object within a SimSet.");

      addProtectedField( "parentGroup", TYPEID< SimObject >(), Offset(mGroup, SimObject), &setProtectedParent, &defaultProtectedGetFn, 
         "Group hierarchy parent of the object." );

      addProtectedField( "class", TypeString, Offset(mClassName, SimObject), &setClass, &defaultProtectedGetFn,
         "Script class of object." );

      addProtectedField( "superClass", TypeString, Offset(mSuperClassName, SimObject), &setSuperClass, &defaultProtectedGetFn,
         "Script super-class of object." );

      // For legacy support
      addProtectedField( "className", TypeString, Offset(mClassName, SimObject), &setClass, &defaultProtectedGetFn,
         "Script class of object.", AbstractClassRep::FIELD_HideInInspectors );

   endGroup( "Object" );
   
   addGroup( "Editing" );
   
      addProtectedField( "hidden", TypeBool, NULL,
         &_setHidden, &_getHidden,
         "Whether the object is visible." );
      addProtectedField( "locked", TypeBool, NULL,
         &_setLocked, &_getLocked,
         "Whether the object can be edited." );
   
   endGroup( "Editing" );
   
   addGroup( "Persistence" );

      addProtectedField( "canSave", TypeBool, Offset( mFlags, SimObject ),
         &_setCanSave, &_getCanSave,
         "Whether the object can be saved out. If false, the object is purely transient in nature." );

      addField( "canSaveDynamicFields", TypeBool, Offset(mCanSaveFieldDictionary, SimObject), 
         "True if dynamic fields (added at runtime) should be saved. Defaults to true." );
   
      addProtectedField( "persistentId", TypePID, Offset( mPersistentId, SimObject ),
         &_setPersistentID, &defaultProtectedGetFn,
         "The universally unique identifier for the object." );
   
   endGroup( "Persistence" );

   Parent::initPersistFields();
}

//-----------------------------------------------------------------------------

String SimObject::describeSelf() const
{
   String desc = Parent::describeSelf();
   
   if( mId != 0 )
      desc = avar( "%s|id: %i", desc.c_str(), mId );
   if( objectName )
      desc = avar( "%s|name: %s", desc.c_str(), objectName );
   if( mInternalName )
      desc = avar( "%s|internal: %s", desc.c_str(), mInternalName );
   if( mNameSpace )
      desc = avar( "%s|nspace: %s", desc.c_str(), mNameSpace->mName );
   if( mGroup )
      desc = avar( "%s|group: %s", desc.c_str(), mGroup->getName() );
   if( mCopySource )
      desc = avar( "%s|copy: %s", desc.c_str(), mCopySource->getName() );
   if( mPersistentId )
      desc = avar( "%s|pid: %s", desc.c_str(), mPersistentId->getUUID().toString().c_str() );

   return desc;
}

//=============================================================================
//    Persistence.
//=============================================================================
// MARK: ---- Persistence ----

//-----------------------------------------------------------------------------

bool SimObject::writeField(StringTableEntry fieldname, const char* value)
{
   // Don't write empty fields.
   if (!value || !*value)
      return false;

   // Don't write owner field for components
   static StringTableEntry sOwner = StringTable->insert( "owner" );
   if( fieldname == sOwner )
      return false;

   // Don't write ParentGroup
   static StringTableEntry sParentGroup = StringTable->insert( "parentGroup" );
   if( fieldname == sParentGroup )
      return false;

   // Don't write name, is within the parenthesis already
   static StringTableEntry sName = StringTable->insert( "name" );
   if( fieldname == sName )
      return false;

   // Don't write className, it is read for legacy support but we
   // write it out as class.
   static StringTableEntry sClassName = StringTable->insert( "className" );
   if( fieldname == sClassName )
      return false;

   // Write persistent ID only if present.
   static StringTableEntry sPersistentId = StringTable->insert( "persistentId" );
   if( fieldname == sPersistentId && ( !value || !value[ 0 ] ) )
      return false;
      
   // Don't write hidden and locked flags if they are at their default value.
      
   static StringTableEntry sHidden = StringTable->insert( "hidden" );
   static StringTableEntry sLocked = StringTable->insert( "locked" );

   if( fieldname == sHidden && !dAtob( value ) )
      return false;
   if( fieldname == sLocked && !dAtob( value ) )
      return false;

   return true;
}

//-----------------------------------------------------------------------------

void SimObject::writeFields(Stream &stream, U32 tabStop)
{
   // Write static fields.
   
   const AbstractClassRep::FieldList &list = getFieldList();

   for(U32 i = 0; i < list.size(); i++)
   {
      const AbstractClassRep::Field* f = &list[i];

      // Skip the special field types.
      if ( f->type >= AbstractClassRep::ARCFirstCustomField )
         continue;

      for(U32 j = 0; S32(j) < f->elementCount; j++)
      {
         char array[8];
         dSprintf( array, 8, "%d", j );
         const char *val = getDataField(StringTable->insert( f->pFieldname ), array );

         // Make a copy for the field check.
         if (!val)
            continue;

         U32 nBufferSize = dStrlen( val ) + 1;
         FrameTemp<char> valCopy( nBufferSize );
         dStrcpy( (char *)valCopy, val );

         if (!writeField(f->pFieldname, valCopy))
            continue;

         val = valCopy;

         U32 expandedBufferSize = ( nBufferSize  * 2 ) + dStrlen(f->pFieldname) + 32;
         FrameTemp<char> expandedBuffer( expandedBufferSize );
         if(f->elementCount == 1)
            dSprintf(expandedBuffer, expandedBufferSize, "%s = \"", f->pFieldname);
         else
            dSprintf(expandedBuffer, expandedBufferSize, "%s[%d] = \"", f->pFieldname, j);

         // detect and collapse relative path information
         char fnBuf[1024];
         if (f->type == TypeFilename ||
             f->type == TypeStringFilename ||
             f->type == TypeImageFilename ||
             f->type == TypePrefabFilename ||
             f->type == TypeShapeFilename)
         {
            Con::collapseScriptFilename(fnBuf, 1024, val);
            val = fnBuf;
         }

         expandEscape((char*)expandedBuffer + dStrlen(expandedBuffer), val);
         dStrcat(expandedBuffer, "\";\r\n");

         stream.writeTabs(tabStop);
         stream.write(dStrlen(expandedBuffer),expandedBuffer);
      }
   }
   
   // Write dynamic fields, if enabled.
   
   if(mFieldDictionary && mCanSaveFieldDictionary)
      mFieldDictionary->writeFields(this, stream, tabStop);
}

//-----------------------------------------------------------------------------

void SimObject::write(Stream &stream, U32 tabStop, U32 flags)
{
   if( !getCanSave() && !( flags & IgnoreCanSave ) )
      return;
      
   // Only output selected objects if they want that.
   if((flags & SelectedOnly) && !isSelected())
      return;

   stream.writeTabs(tabStop);
   char buffer[1024];
   dSprintf(buffer, sizeof(buffer), "new %s(%s) {\r\n", getClassName(), getName() && !(flags & NoName) ? getName() : "");
   stream.write(dStrlen(buffer), buffer);
   writeFields(stream, tabStop + 1);

   stream.writeTabs(tabStop);
   stream.write(4, "};\r\n");
}

//-----------------------------------------------------------------------------

bool SimObject::save(const char *pcFileName, bool bOnlySelected, const char *preappend)
{
   static const char *beginMessage = "//--- OBJECT WRITE BEGIN ---";
   static const char *endMessage = "//--- OBJECT WRITE END ---";
   FileStream *stream;
   FileObject f;
   f.readMemory(pcFileName);

   // check for flags <selected, ...>
   U32 writeFlags = 0;
   if(bOnlySelected)
      writeFlags |= SimObject::SelectedOnly;

   if((stream = FileStream::createAndOpen( pcFileName, Torque::FS::File::Write )) == NULL)
      return false;

   char docRoot[256];
   char modRoot[256];

   dStrcpy(docRoot, pcFileName);
   char *p = dStrrchr(docRoot, '/');
   if (p) *++p = '\0';
   else  docRoot[0] = '\0';

   dStrcpy(modRoot, pcFileName);
   p = dStrchr(modRoot, '/');
   if (p) *++p = '\0';
   else  modRoot[0] = '\0';

   Con::setVariable("$DocRoot", docRoot);
   Con::setVariable("$ModRoot", modRoot);

   const char *buffer;
   while(!f.isEOF())
   {
      buffer = (const char *) f.readLine();
      if(!dStrcmp(buffer, beginMessage))
         break;
      stream->write(dStrlen(buffer), buffer);
      stream->write(2, "\r\n");
   }
   stream->write(dStrlen(beginMessage), beginMessage);
   stream->write(2, "\r\n");
   if ( preappend != NULL )   
      stream->write(dStrlen(preappend),preappend);   
   write(*stream, 0, writeFlags);
   stream->write(dStrlen(endMessage), endMessage);
   stream->write(2, "\r\n");
   while(!f.isEOF())
   {
      buffer = (const char *) f.readLine();
      if(!dStrcmp(buffer, endMessage))
         break;
   }
   while(!f.isEOF())
   {
      buffer = (const char *) f.readLine();
      stream->write(dStrlen(buffer), buffer);
      stream->write(2, "\r\n");
   }

   Con::setVariable("$DocRoot", NULL);
   Con::setVariable("$ModRoot", NULL);

   delete stream;

   return true;

}

//-----------------------------------------------------------------------------

SimPersistID* SimObject::getOrCreatePersistentId()
{
   if( !mPersistentId )
   {
       mPersistentId = SimPersistID::create( this );
       mPersistentId->incRefCount();
   }
   return mPersistentId;
}



void SimObject::onTamlCustomRead(TamlCustomNodes const& customNodes)
{
   // Debug Profiling.
   //PROFILE_SCOPE(SimObject_OnTamlCustomRead);

   // Fetch field list.
   const AbstractClassRep::FieldList& fieldList = getFieldList();
   const U32 fieldCount = fieldList.size();
   for (U32 index = 0; index < fieldCount; ++index)
   {
      // Fetch field.
      const AbstractClassRep::Field* pField = &fieldList[index];

      // Ignore if field not appropriate.
      if (pField->type == AbstractClassRep::StartArrayFieldType || pField->elementCount > 1)
      {
         // Find cell custom node.
         const TamlCustomNode* pCustomCellNodes = NULL;
         if (pField->pGroupname != NULL)
            pCustomCellNodes = customNodes.findNode(pField->pGroupname);
         if (!pCustomCellNodes)
         {
            char* niceFieldName = const_cast<char *>(pField->pFieldname);
            niceFieldName[0] = dToupper(niceFieldName[0]);
            String str_niceFieldName = String(niceFieldName);
            pCustomCellNodes = customNodes.findNode(str_niceFieldName + "s");
         }

         // Continue if we have explicit cells.
         if (pCustomCellNodes != NULL)
         {
            // Fetch children cell nodes.
            const TamlCustomNodeVector& cellNodes = pCustomCellNodes->getChildren();

            U8 idx = 0;
            // Iterate cells.
            for (TamlCustomNodeVector::const_iterator cellNodeItr = cellNodes.begin(); cellNodeItr != cellNodes.end(); ++cellNodeItr)
            {
               char buf[5];
               dSprintf(buf, 5, "%d", idx);

               // Fetch cell node.
               TamlCustomNode* pCellNode = *cellNodeItr;

               // Fetch node name.
               StringTableEntry nodeName = pCellNode->getNodeName();

               // Is this a valid alias?
               if (nodeName != pField->pFieldname)
               {
                  // No, so warn.
                  Con::warnf("SimObject::onTamlCustomRead() - Encountered an unknown custom name of '%s'.  Only '%s' is valid.", nodeName, pField->pFieldname);
                  continue;
               }

               // Fetch fields.
               const TamlCustomFieldVector& fields = pCellNode->getFields();

               // Iterate property fields.
               for (TamlCustomFieldVector::const_iterator fieldItr = fields.begin(); fieldItr != fields.end(); ++fieldItr)
               {
                  // Fetch field.
                  const TamlCustomField* pField = *fieldItr;

                  // Fetch field name.
                  StringTableEntry fieldName = pField->getFieldName();

                  const AbstractClassRep::Field* field = findField(fieldName);

                  // Check common fields.
                  if (field)
                  {
                     setDataField(fieldName, buf, pField->getFieldValue());
                  }
                  else
                  {
                     // Unknown name so warn.
                     Con::warnf("SimObject::onTamlCustomRead() - Encountered an unknown custom field name of '%s'.", fieldName);
                     continue;
                  }
               }

               idx++;
            }
         }
      }
   }
}

//-----------------------------------------------------------------------------

bool SimObject::_setPersistentID( void* object, const char* index, const char* data )
{
   SimObject* simObject = reinterpret_cast< SimObject* >( object );

   // Make sure we don't already have a PID.
   if( simObject->getPersistentId() )
   {
      Con::errorf( "SimObject::_setPersistentID - cannot set a persistent ID on an object that already has a persistent ID assigned." );
      return false;
   }

   SimPersistID* pid;
   Con::setData( TypePID, &pid, 0, 1, &data );
   if ( !pid )
      return false;

   // Make sure it's not already bound to an object.
   if( pid->getObject() )
   {
      AssertWarn( pid->getObject() != simObject, "Sim::_setPersistentID - PID is bound to this object yet not assigned to it!" );

      SimObject* otherObj = pid->getObject();
      Con::errorf( "SimObject::_setPersistentID - UUID is already used by another object: '%s' -> %i:%s (%s)",
         data, otherObj->getId(), otherObj->getClassName(), otherObj->getName() );

      return false;
   }

   pid->resolve( simObject );
   pid->incRefCount();
   simObject->mPersistentId = pid;

   return false;
}

//-----------------------------------------------------------------------------

void SimObject::setFilename( const char* file )
{
   if( file )
      mFilename = StringTable->insert( file );
   else
      mFilename = StringTable->EmptyString();
}

//-----------------------------------------------------------------------------

void SimObject::setDeclarationLine(U32 lineNumber)
{
   mDeclarationLine = lineNumber;
}

//=============================================================================
//    Management.
//=============================================================================
// MARK: ---- Management ----

//-----------------------------------------------------------------------------

bool SimObject::registerObject()
{
   AssertFatal( !mFlags.test( Added ), "reigsterObject - Object already registered!");
   mFlags.clear(Deleted | Removed);

   if(smForceId)
   {
      setId(smForcedId);
      smForceId = false;
   }

   if( !mId )
   {
      mId = Sim::gNextObjectId++;
      dSprintf( mIdString, sizeof( mIdString ), "%u", mId );
   }

   AssertFatal(Sim::gIdDictionary && Sim::gNameDictionary, 
      "SimObject::registerObject - tried to register an object before Sim::init()!");

   Sim::gIdDictionary->insert(this);   

   Sim::gNameDictionary->insert(this);

   // Notify object
   bool ret = onAdd();

   if(!ret)
      unregisterObject();

   AssertFatal(!ret || isProperlyAdded(), "Object did not call SimObject::onAdd()");
   return ret;
}

//-----------------------------------------------------------------------------

void SimObject::unregisterObject()
{
   mFlags.set(Removed);

   // Notify object first
   onRemove();

   // Clear out any pending notifications before
   // we call our own, just in case they delete
   // something that we have referenced.
   clearAllNotifications();

   // Notify all objects that are waiting for delete
   // messages
   if (getGroup())
      getGroup()->removeObject(this);

   processDeleteNotifies();

   // Do removals from the Sim.
   Sim::gNameDictionary->remove(this);
   Sim::gIdDictionary->remove(this);
   Sim::cancelPendingEvents(this);
}

//-----------------------------------------------------------------------------

void SimObject::deleteObject()
{
   Parent::destroySelf();
}

//-----------------------------------------------------------------------------

void SimObject::_destroySelf()
{
   AssertFatal( !isDeleted(), "SimObject::destroySelf - Object has already been deleted" );
   AssertFatal( !isRemoved(), "SimObject::destroySelf - Object in the process of being removed" );

   mFlags.set( Deleted );

   if( mFlags.test( Added ) )
      unregisterObject();

   Parent::_destroySelf();
}

//-----------------------------------------------------------------------------

void SimObject::destroySelf()
{
   // When using the legacy console interop, we don't delete objects
   // when their reference count drops to zero but rather defer their
   // deletion until deleteObject() is called.

   if( engineAPI::gUseConsoleInterop )
      return;

   Parent::destroySelf();
}

//-----------------------------------------------------------------------------

class SimObjectDeleteEvent : public SimEvent
{
public:
   void process(SimObject *object)
   {
      object->deleteObject();
   }
};

void SimObject::safeDeleteObject()
{
   Sim::postEvent( this, new SimObjectDeleteEvent, Sim::getCurrentTime() + 1 );
}

//-----------------------------------------------------------------------------

void SimObject::setId(SimObjectId newId)
{
   if(!mFlags.test(Added))
      mId = newId;
   else
   {
      // get this object out of the id dictionary if it's in it
      Sim::gIdDictionary->remove(this);

      // Free current Id.
      // Assign new one.
      mId = newId ? newId : Sim::gNextObjectId++;
      Sim::gIdDictionary->insert(this);
   }

   dSprintf( mIdString, sizeof( mIdString ), "%u", mId );
}

//-----------------------------------------------------------------------------

void SimObject::assignName(const char *name)
{
   if( objectName && !isNameChangeAllowed() )
   {
      Con::errorf( "SimObject::assignName - not allowed to change name of object '%s'", objectName );
      return;
   }
   
   // Added this assert 3/30/2007 because it is dumb to try to name
   // a SimObject the same thing as it's class name -patw
   //AssertFatal( dStricmp( getClassName(), name ), "Attempted to assign a name to a SimObject which matches it's type name." );
   if( dStricmp( getClassName(), name ) == 0 )
      Con::errorf( "SimObject::assignName - Assigning name '%s' to instance of object with type '%s'."
      " This can cause namespace linking issues.", getClassName(), name  );

   StringTableEntry newName = NULL;
   if(name[0])
      newName = StringTable->insert(name);

   onNameChange( newName );

   if( mGroup )
      mGroup->mNameDictionary.remove( this );
   if( isProperlyAdded() )
   {
      unlinkNamespaces();
      Sim::gNameDictionary->remove( this );
   }
      
   objectName = newName;
   
   if( mGroup )
      mGroup->mNameDictionary.insert( this );
   if( isProperlyAdded() )
   {
      Sim::gNameDictionary->insert( this );
      linkNamespaces();
   }
}

//-----------------------------------------------------------------------------

bool SimObject::registerObject(U32 id)
{
   setId(id);
   return registerObject();
}

//-----------------------------------------------------------------------------

bool SimObject::registerObject(const char *name)
{
   assignName(name);
   return registerObject();
}

//-----------------------------------------------------------------------------

bool SimObject::registerObject(const char *name, U32 id)
{
   setId(id);
   assignName(name);
   return registerObject();
}

//=============================================================================
//    Introspection.
//=============================================================================
// MARK: ---- Introspection ----

//-----------------------------------------------------------------------------

bool SimObject::isMethod( const char* methodName )
{
   if( !methodName || !methodName[0] )
      return false;

   StringTableEntry stname = StringTable->insert( methodName );

   if( getNamespace() )
      return ( getNamespace()->lookup( stname ) != NULL );

   return false;
}

//-----------------------------------------------------------------------------

bool SimObject::isField( const char* fieldName, bool includeStatic, bool includeDynamic )
{
   const char* strFieldName = StringTable->insert( fieldName );
   
   if( includeStatic && getClassRep()->findField( strFieldName ) )
      return true;
   
   if( includeDynamic && getFieldDictionary() && getFieldDictionary()->findDynamicField( strFieldName ) )
      return true;
   
   return false;
}

//-----------------------------------------------------------------------------

void SimObject::assignDynamicFieldsFrom(SimObject* parent)
{
   if(parent->mFieldDictionary)
   {
      if( mFieldDictionary == NULL )
         mFieldDictionary = new SimFieldDictionary;
      mFieldDictionary->assignFrom(parent->mFieldDictionary);
   }
}

//-----------------------------------------------------------------------------

void SimObject::assignFieldsFrom(SimObject *parent)
{
   // Only allow field assigns from objects of the same class or
   // a superclass.
   
   if( getClassRep()->isClass( parent->getClassRep() ) )
   {
      const AbstractClassRep::FieldList &list = parent->getFieldList();

      // copy out all the fields:
      for(U32 i = 0; i < list.size(); i++)
      {
         const AbstractClassRep::Field* f = &list[i];

         // Skip the special field types.
         if ( f->type >= AbstractClassRep::ARCFirstCustomField )
            continue;
            
         // Skip certain fields that we don't want to see copied so we don't
         // get error messages from their setters.
            
         static StringTableEntry sName = StringTable->insert( "name" );
         static StringTableEntry sPersistentId = StringTable->insert( "persistentId" );
         
         if( f->pFieldname == sName || f->pFieldname == sPersistentId )
            continue;

         S32 lastField = f->elementCount - 1;
         for(S32 j = 0; j <= lastField; j++)
         {
            const char* fieldVal = (*f->getDataFn)( parent,  Con::getData(f->type, (void *) (((const char *)parent) + f->offset), j, f->table, f->flag));

            // Don't assign the field is the pointer is null or if
            // the field is not empty and writing it was disallowed.
            if ( !fieldVal || ( fieldVal[0] && !writeField( f->pFieldname, fieldVal ) ) )
               continue;

            // code copied from SimObject::setDataField().
            // TODO: paxorr: abstract this into a better setData / getData that considers prot fields.
            FrameTemp<char> buffer(2048);
            FrameTemp<char> bufferSecure(2048); // This buffer is used to make a copy of the data
            ConsoleBaseType *cbt = ConsoleBaseType::getType( f->type );
            const char* szBuffer = cbt->prepData( fieldVal, buffer, 2048 );
            dMemset( bufferSecure, 0, 2048 );
            dMemcpy( bufferSecure, szBuffer, dStrlen( szBuffer ) );

            if((*f->setDataFn)( this, NULL, bufferSecure ) )
               Con::setData(f->type, (void *) (((const char *)this) + f->offset), j, 1, &fieldVal, f->table);
         }
      }
   }
   else
   {
      Con::errorf( "SimObject::assignFieldsFrom() - cannot assigned fields from object of type '%s' to object of type '%s'",
         parent->getClassName(), getClassName()
      );
   }

   assignDynamicFieldsFrom(parent);
}

//-----------------------------------------------------------------------------

void SimObject::setDataField(StringTableEntry slotName, const char *array, const char *value)
{
   // first search the static fields if enabled
   if(mFlags.test(ModStaticFields))
   {
      const AbstractClassRep::Field *fld = findField(slotName);
      if(fld)
      {
         // Skip the special field types as they are not data.
         if ( fld->type >= AbstractClassRep::ARCFirstCustomField )
            return;

         S32 array1 = array ? dAtoi(array) : 0;

         if(array1 >= 0 && array1 < fld->elementCount && fld->elementCount >= 1)
         {
            // If the set data notify callback returns true, then go ahead and
            // set the data, otherwise, assume the set notify callback has either
            // already set the data, or has deemed that the data should not
            // be set at all.
            FrameTemp<char> buffer(2048);
            FrameTemp<char> bufferSecure(2048); // This buffer is used to make a copy of the data
            // so that if the prep functions or any other functions use the string stack, the data
            // is not corrupted.

            ConsoleBaseType *cbt = ConsoleBaseType::getType( fld->type );
            AssertFatal( cbt != NULL, "Could not resolve Type Id." );

            const char* szBuffer = cbt->prepData( value, buffer, 2048 );
            dMemset( bufferSecure, 0, 2048 );
            dMemcpy( bufferSecure, szBuffer, dStrlen( szBuffer ) );

            if( (*fld->setDataFn)( this, array, bufferSecure ) )
               Con::setData(fld->type, (void *) (((const char *)this) + fld->offset), array1, 1, &value, fld->table);

            if(fld->validator)
               fld->validator->validateType(this, (void *) (((const char *)this) + fld->offset));

            onStaticModified( slotName, value );

            return;
         }

         if(fld->validator)
            fld->validator->validateType(this, (void *) (((const char *)this) + fld->offset));

         onStaticModified( slotName, value );
         return;
      }
   }

   if(mFlags.test(ModDynamicFields))
   {
      if(!mFieldDictionary)
         mFieldDictionary = new SimFieldDictionary;

      if(!array)
      {
         mFieldDictionary->setFieldValue(slotName, value);
         onDynamicModified( slotName, value );
      }
      else
      {
         char buf[256];
         dStrcpy(buf, slotName);
         dStrcat(buf, array);
         StringTableEntry permanentSlotName = StringTable->insert(buf);
         mFieldDictionary->setFieldValue(permanentSlotName, value);
         onDynamicModified( permanentSlotName, value );
      }
   }
}

//-----------------------------------------------------------------------------

const char *SimObject::getDataField(StringTableEntry slotName, const char *array)
{
   if(mFlags.test(ModStaticFields))
   {
      S32 array1 = array ? dAtoi(array) : -1;
      const AbstractClassRep::Field *fld = findField(slotName);

      if(fld)
      {
         if(array1 == -1 && fld->elementCount == 1)
            return (*fld->getDataFn)( this, Con::getData(fld->type, (void *) (((const char *)this) + fld->offset), 0, fld->table, fld->flag) );
         if(array1 >= 0 && array1 < fld->elementCount)
            return (*fld->getDataFn)( this, Con::getData(fld->type, (void *) (((const char *)this) + fld->offset), array1, fld->table, fld->flag) );// + typeSizes[fld.type] * array1));
         return "";
      }
   }

   if(mFlags.test(ModDynamicFields))
   {
      if(!mFieldDictionary)
         return "";

      if(!array)
      {
         if (const char* val = mFieldDictionary->getFieldValue(slotName))
            return val;
      }
      else
      {
         static char buf[256];
         dStrcpy(buf, slotName);
         dStrcat(buf, array);
         if (const char* val = mFieldDictionary->getFieldValue(StringTable->insert(buf)))
            return val;
      }
   }

   return "";
}


const char *SimObject::getPrefixedDataField(StringTableEntry fieldName, const char *array)
{
   // Sanity!
   AssertFatal(fieldName != NULL, "Cannot get field value with NULL field name.");

   // Fetch field value.
   const char* pFieldValue = getDataField(fieldName, array);

   // Sanity.
   //AssertFatal(pFieldValue != NULL, "Field value cannot be NULL.");
   if (!pFieldValue)
      return NULL;

   // Return without the prefix if there's no value.
   if (*pFieldValue == 0)
      return StringTable->EmptyString();

   // Fetch the field prefix.
   StringTableEntry fieldPrefix = getDataFieldPrefix(fieldName);

   // Sanity!
   AssertFatal(fieldPrefix != NULL, "Field prefix cannot be NULL.");

   // Calculate a buffer size including prefix.
   const U32 valueBufferSize = dStrlen(fieldPrefix) + dStrlen(pFieldValue) + 1;

   // Fetch a buffer.
   char* pValueBuffer = Con::getReturnBuffer(valueBufferSize);

   // Format the value buffer.
   dSprintf(pValueBuffer, valueBufferSize, "%s%s", fieldPrefix, pFieldValue);

   return pValueBuffer;
}

//-----------------------------------------------------------------------------

void SimObject::setPrefixedDataField(StringTableEntry fieldName, const char *array, const char *value)
{
   // Sanity!
   AssertFatal(fieldName != NULL, "Cannot set object field value with NULL field name.");
   AssertFatal(value != NULL, "Field value cannot be NULL.");

   // Set value without prefix if there's no value.
   if (*value == 0)
   {
      setDataField(fieldName, NULL, value);
      return;
   }

   // Fetch the field prefix.
   StringTableEntry fieldPrefix = getDataFieldPrefix(fieldName);

   // Sanity.
   AssertFatal(fieldPrefix != NULL, "Field prefix cannot be NULL.");

   // Do we have a field prefix?
   if (fieldPrefix == StringTable->EmptyString())
   {
      // No, so set the data field in the usual way.
      setDataField(fieldName, NULL, value);
      return;
   }

   // Yes, so fetch the length of the field prefix.
   const U32 fieldPrefixLength = dStrlen(fieldPrefix);

   // Yes, so does it start with the object field prefix?
   if (dStrnicmp(value, fieldPrefix, fieldPrefixLength) != 0)
   {
      // No, so set the data field in the usual way.
      setDataField(fieldName, NULL, value);
      return;
   }

   // Yes, so set the data excluding the prefix.
   setDataField(fieldName, NULL, value + fieldPrefixLength);
}

//-----------------------------------------------------------------------------

const char *SimObject::getPrefixedDynamicDataField(StringTableEntry fieldName, const char *array, const S32 fieldType)
{
   // Sanity!
   AssertFatal(fieldName != NULL, "Cannot get field value with NULL field name.");

   // Fetch field value.
   const char* pFieldValue = getDataField(fieldName, array);

   // Sanity.
   AssertFatal(pFieldValue != NULL, "Field value cannot be NULL.");

   // Return the field if no field type is specified.
   if (fieldType == -1)
      return pFieldValue;

   // Return without the prefix if there's no value.
   if (*pFieldValue == 0)
      return StringTable->EmptyString();

   // Fetch the console base type.
   ConsoleBaseType* pConsoleBaseType = ConsoleBaseType::getType(fieldType);

   // Did we find the console base type?
   if (pConsoleBaseType == NULL)
   {
      // No, so warn.
      Con::warnf("getPrefixedDynamicDataField() - Invalid field type '%d' specified for field '%s' with value '%s'.",
         fieldType, fieldName, pFieldValue);
   }

   // Fetch the field prefix.
   StringTableEntry fieldPrefix = pConsoleBaseType->getTypePrefix();

   // Sanity!
   AssertFatal(fieldPrefix != NULL, "Field prefix cannot be NULL.");

   // Calculate a buffer size including prefix.
   const U32 valueBufferSize = dStrlen(fieldPrefix) + dStrlen(pFieldValue) + 1;

   // Fetch a buffer.
   char* pValueBuffer = Con::getReturnBuffer(valueBufferSize);

   // Format the value buffer.
   dSprintf(pValueBuffer, valueBufferSize, "%s%s", fieldPrefix, pFieldValue);

   return pValueBuffer;
}

//-----------------------------------------------------------------------------

void SimObject::setPrefixedDynamicDataField(StringTableEntry fieldName, const char *array, const char *value, const S32 fieldType)
{
   // Sanity!
   AssertFatal(fieldName != NULL, "Cannot set object field value with NULL field name.");
   AssertFatal(value != NULL, "Field value cannot be NULL.");

   // Set value without prefix if no field type was specified.
   if (fieldType == -1)
   {
      setDataField(fieldName, NULL, value);
      return;
   }

   // Fetch the console base type.
   ConsoleBaseType* pConsoleBaseType = ConsoleBaseType::getType(fieldType);

   // Did we find the console base type?
   if (pConsoleBaseType == NULL)
   {
      // No, so warn.
      Con::warnf("setPrefixedDynamicDataField() - Invalid field type '%d' specified for field '%s' with value '%s'.",
         fieldType, fieldName, value);
   }

   // Set value without prefix if there's no value or we didn't find the console base type.
   if (*value == 0 || pConsoleBaseType == NULL)
   {
      setDataField(fieldName, NULL, value);
      return;
   }

   // Fetch the field prefix.
   StringTableEntry fieldPrefix = pConsoleBaseType->getTypePrefix();

   // Sanity.
   AssertFatal(fieldPrefix != NULL, "Field prefix cannot be NULL.");

   // Do we have a field prefix?
   if (fieldPrefix == StringTable->EmptyString())
   {
      // No, so set the data field in the usual way.
      setDataField(fieldName, NULL, value);
      return;
   }

   // Yes, so fetch the length of the field prefix.
   const U32 fieldPrefixLength = dStrlen(fieldPrefix);

   // Yes, so does it start with the object field prefix?
   if (dStrnicmp(value, fieldPrefix, fieldPrefixLength) != 0)
   {
      // No, so set the data field in the usual way.
      setDataField(fieldName, NULL, value);
      return;
   }

   // Yes, so set the data excluding the prefix.
   setDataField(fieldName, NULL, value + fieldPrefixLength);
}

//-----------------------------------------------------------------------------

StringTableEntry SimObject::getDataFieldPrefix(StringTableEntry fieldName)
{
   // Sanity!
   AssertFatal(fieldName != NULL, "Cannot get field prefix with NULL field name.");

   // Find the field.
   const AbstractClassRep::Field* pField = findField(fieldName);

   // Return nothing if field was not found.
   if (pField == NULL)
      return StringTable->EmptyString();

   // Yes, so fetch the console base type.
   ConsoleBaseType* pConsoleBaseType = ConsoleBaseType::getType(pField->type);

   // Fetch the type prefix.
   return pConsoleBaseType->getTypePrefix();
}


//-----------------------------------------------------------------------------

U32 SimObject::getDataFieldType( StringTableEntry slotName, const char* array )
{
   const AbstractClassRep::Field* field = findField( slotName );
   if(field)
      return field->type;

   // Check dynamic fields
   if(!mFieldDictionary)
      return 0;

   if(array == NULL || *array == 0)
      return mFieldDictionary->getFieldType( slotName );
   else
   {
      static char buf[256];
      dStrcpy( buf, slotName );
      dStrcat( buf, array );

      return mFieldDictionary->getFieldType( StringTable->insert( buf ) );
   }
}

//-----------------------------------------------------------------------------

void SimObject::setDataFieldType(const U32 fieldTypeId, StringTableEntry slotName, const char *array)
{
   // This only works on dynamic fields, bail if we have no field dictionary
   if(!mFieldDictionary)
      return;

   if(array == NULL || *array == 0)
   {
      mFieldDictionary->setFieldType( slotName, fieldTypeId );
      onDynamicModified( slotName, mFieldDictionary->getFieldValue(slotName) );
   }
   else
   {
      static char buf[256];
      dStrcpy( buf, slotName );
      dStrcat( buf, array );

      mFieldDictionary->setFieldType( StringTable->insert( buf ), fieldTypeId );
      onDynamicModified( slotName, mFieldDictionary->getFieldValue(slotName) );
   }
}

//-----------------------------------------------------------------------------

void SimObject::setDataFieldType(const char *typeName, StringTableEntry slotName, const char *array)
{
   // This only works on dynamic fields, bail if we have no field dictionary
   if(!mFieldDictionary)
      return;

   if(array == NULL || *array == 0)
      mFieldDictionary->setFieldType( slotName, typeName );
   else
   {
      static char buf[256];
      dStrcpy( buf, slotName );
      dStrcat( buf, array );
      StringTableEntry permanentSlotName = StringTable->insert(buf);

      mFieldDictionary->setFieldType( permanentSlotName, typeName );
      onDynamicModified( permanentSlotName, mFieldDictionary->getFieldValue(permanentSlotName) );
   }
}

//-----------------------------------------------------------------------------

void SimObject::dumpClassHierarchy()
{
   AbstractClassRep* pRep = getClassRep();
   while(pRep)
   {
      Con::warnf("%s ->", pRep->getClassName());
      pRep  =  pRep->getParentClass();
   }
}

//-----------------------------------------------------------------------------

SimObject* SimObject::clone()
{
   if( !getClassRep() )
      return NULL;
      
   ConsoleObject* conObject = getClassRep()->create();
   if( !conObject )
      return NULL;
      
   SimObject* simObject = dynamic_cast< SimObject* >( conObject );
   if( !simObject )
   {
      delete conObject;
      return NULL;
   }
   
   simObject->assignFieldsFrom( this );

   String name = Sim::getUniqueName( getName() );
   if( !simObject->registerObject( name ) )
   {
      delete simObject;
      return NULL;
   }
      
   if( getGroup() )
      getGroup()->addObject( simObject );
   
   return simObject;
}

//-----------------------------------------------------------------------------

SimObject* SimObject::deepClone()
{
   return clone();
}

//=============================================================================
//    Grouping.
//=============================================================================
// MARK: ---- Grouping ----

//-----------------------------------------------------------------------------

SimObject* SimObject::findObject( const char* )
{
   return NULL;
}

//-----------------------------------------------------------------------------

bool SimObject::isChildOfGroup(SimGroup* pGroup)
{
   if(!pGroup)
      return false;

   //if we *are* the group in question,
   //return true:
   if(pGroup == dynamic_cast<SimGroup*>(this))
      return true;

   SimGroup* temp =  mGroup;
   while(temp)
   {
      if(temp == pGroup)
         return true;
      temp = temp->mGroup;
   }

   return false;
}

//-----------------------------------------------------------------------------

bool SimObject::addToSet(SimObjectId spid)
{
   if (mFlags.test(Added) == false)
      return false;

   SimObject* ptr = Sim::findObject(spid);
   if (ptr)
   {
      SimSet* sp = dynamic_cast<SimSet*>(ptr);
      AssertFatal(sp != 0,
         "SimObject::addToSet: "
         "ObjectId does not refer to a set object");
      sp->addObject(this);
      return true;
   }
   return false;
}

//-----------------------------------------------------------------------------

bool SimObject::addToSet(const char *ObjectName)
{
   if (mFlags.test(Added) == false)
      return false;

   SimObject* ptr = Sim::findObject(ObjectName);
   if (ptr)
   {
      SimSet* sp = dynamic_cast<SimSet*>(ptr);
      AssertFatal(sp != 0,
         "SimObject::addToSet: "
         "ObjectName does not refer to a set object");
      sp->addObject(this);
      return true;
   }
   return false;
}

//-----------------------------------------------------------------------------

bool SimObject::removeFromSet(SimObjectId sid)
{
   if (mFlags.test(Added) == false)
      return false;

   SimSet *set;
   if(Sim::findObject(sid, set))
   {
      set->removeObject(this);
      return true;
   }
   return false;
}

//-----------------------------------------------------------------------------

bool SimObject::removeFromSet(const char *objectName)
{
   if (mFlags.test(Added) == false)
      return false;

   SimSet *set;
   if(Sim::findObject(objectName, set))
   {
      set->removeObject(this);
      return true;
   }
   return false;
}

//-----------------------------------------------------------------------------

void SimObject::dumpGroupHierarchy()
{
   String className( getClassName() );
   String objectName( getName() );

   Con::warnf( "[%i] %s - %s ->", getId(), className.c_str(), objectName.c_str() );
   
   if ( mGroup )
      mGroup->dumpGroupHierarchy();
}

//=============================================================================
//    Events.
//=============================================================================
// MARK: ---- Events ----

//-----------------------------------------------------------------------------

bool SimObject::onAdd()
{
   mFlags.set(Added);

   linkNamespaces();

   return true;
}

//-----------------------------------------------------------------------------

void SimObject::onRemove()
{
   mFlags.clear(Added);

   unlinkNamespaces();
}

//-----------------------------------------------------------------------------

void SimObject::onGroupAdd()
{
}

//-----------------------------------------------------------------------------

void SimObject::onGroupRemove()
{
}

//-----------------------------------------------------------------------------

void SimObject::onDeleteNotify(SimObject*)
{
}

//-----------------------------------------------------------------------------

void SimObject::onNameChange(const char*)
{
}

//-----------------------------------------------------------------------------

void SimObject::onStaticModified(const char* slotName, const char* newValue)
{
}

//-----------------------------------------------------------------------------

void SimObject::onDynamicModified(const char* slotName, const char* newValue)
{
}

//=============================================================================
//    Notifications.
//=============================================================================
// MARK: ---- Notifications ----

static Chunker<SimObject::Notify> notifyChunker(128000);
SimObject::Notify *SimObject::mNotifyFreeList = NULL;

//-----------------------------------------------------------------------------

SimObject::Notify *SimObject::allocNotify()
{
   if(mNotifyFreeList)
   {
      SimObject::Notify *ret = mNotifyFreeList;
      mNotifyFreeList = ret->next;
      return ret;
   }
   return notifyChunker.alloc();
}

//-----------------------------------------------------------------------------

void SimObject::freeNotify(SimObject::Notify* note)
{
   AssertFatal(note->type != SimObject::Notify::Invalid, "Invalid notify");
   note->type = SimObject::Notify::Invalid;
   note->next = mNotifyFreeList;
   mNotifyFreeList = note;
}

//-----------------------------------------------------------------------------

SimObject::Notify* SimObject::removeNotify(void *ptr, SimObject::Notify::Type type)
{
   Notify **list = &mNotifyList;
   while(*list)
   {
      if((*list)->ptr == ptr && (*list)->type == type)
      {
         SimObject::Notify *ret = *list;
         *list = ret->next;
         return ret;
      }
      list = &((*list)->next);
   }
   return NULL;
}

//-----------------------------------------------------------------------------

void SimObject::deleteNotify(SimObject* obj)
{
   AssertFatal(!obj->isDeleted(),
      "SimManager::deleteNotify: Object is being deleted");
   Notify *note = allocNotify();
   note->ptr = (void *) this;
   note->next = obj->mNotifyList;
   note->type = Notify::DeleteNotify;
   obj->mNotifyList = note;

   note = allocNotify();
   note->ptr = (void *) obj;
   note->next = mNotifyList;
   note->type = Notify::ClearNotify;
   mNotifyList = note;

   //obj->deleteNotifyList.pushBack(this);
   //clearNotifyList.pushBack(obj);
}

//-----------------------------------------------------------------------------

void SimObject::registerReference(SimObject **ptr)
{
   Notify *note = allocNotify();
   note->ptr = (void *) ptr;
   note->next = mNotifyList;
   note->type = Notify::ObjectRef;
   mNotifyList = note;
}

//-----------------------------------------------------------------------------

void SimObject::unregisterReference(SimObject **ptr)
{
   Notify *note = removeNotify((void *) ptr, Notify::ObjectRef);
   if(note)
   {
      freeNotify(note);

      if( mFlags.test( AutoDelete ) )
      {
         for( Notify* n = mNotifyList; n != NULL; n = n->next )
            if( n->type == Notify::ObjectRef )
               return;

         deleteObject();
      }
   }
}

//-----------------------------------------------------------------------------

void SimObject::clearNotify(SimObject* obj)
{
   Notify *note = obj->removeNotify((void *) this, Notify::DeleteNotify);
   if(note)
      freeNotify(note);

   note = removeNotify((void *) obj, Notify::ClearNotify);
   if(note)
      freeNotify(note);
}

//-----------------------------------------------------------------------------

void SimObject::processDeleteNotifies()
{
   // clear out any delete notifies and
   // object refs.

   while(mNotifyList)
   {
      Notify *note = mNotifyList;
      mNotifyList = note->next;

      AssertFatal(note->type != Notify::ClearNotify, "Clear notes should be all gone.");

      if(note->type == Notify::DeleteNotify)
      {
         SimObject *obj = (SimObject *) note->ptr;
         Notify *cnote = obj->removeNotify((void *)this, Notify::ClearNotify);
         obj->onDeleteNotify(this);
         freeNotify(cnote);
      }
      else
      {
         // it must be an object ref - a pointer refs this object
         *((SimObject **) note->ptr) = NULL;
      }
      freeNotify(note);
   }
}

//-----------------------------------------------------------------------------

void SimObject::clearAllNotifications()
{
   for(Notify **cnote = &mNotifyList; *cnote; )
   {
      Notify *temp = *cnote;
      if(temp->type == Notify::ClearNotify)
      {
         *cnote = temp->next;
         Notify *note = ((SimObject *) temp->ptr)->removeNotify((void *) this, Notify::DeleteNotify);
         freeNotify(temp);
         if ( note )
            freeNotify(note);
      }
      else
         cnote = &(temp->next);
   }
}

//=============================================================================
//    Namespaces.
//=============================================================================
// MARK: ---- Namespaces ----

//-----------------------------------------------------------------------------

void SimObject::linkNamespaces()
{
   // Don't link if we already have a namespace linkage in place.
   // If you want to change namespace linking, first call unlinkNamespaces()
   // while still having the class namespace fields matching the current
   // setup.

   if (mNameSpace)
   {
      Con::warnf("SimObject::linkNamespaces -- Namespace linkage already in place %s", mNameSpace->getName());
      return;
   }
   // Get the namespace for the C++ class.

   Namespace* cppNamespace = getClassRep()->getNameSpace();

   // Parent namespace defaults to namespace of C++ class.

   Namespace* parentNamespace = cppNamespace;

   // Perform superclass linking, if requested.

   if( mSuperClassName && mSuperClassName[ 0 ] )
   {
      // Look up the superclass namespace.

      Namespace* superClassNamespace = Con::lookupNamespace( mSuperClassName );

      // If packages are active and adding to the superclass namespace, then we will
      // have multiple packages in a parent chain that all have the same name.
      // Con::lookupNamespace returns the bottom-most package in the chain to us so
      // in order to properly link namespace here without conflicting with the package
      // mechanism, we need to properly link child namespaces to the bottom-most namespace
      // while linking parent namespaces to the topmost namespace.  To find the latter
      // one, we walk up the hierarchy here.

      Namespace* superClassNamespacePackageRoot = superClassNamespace->getPackageRoot();

      // Link the superclass namespace to the C++ class namespace.

      if( superClassNamespacePackageRoot->getParent() == NULL )
      {
         // The superclass namespace isn't linked yet so we just
         // link it to the C++ class namespace and make that our parent.
         // No increasing parent reference counts is needed in this case.

         bool ok = superClassNamespacePackageRoot->classLinkTo( cppNamespace );
         AssertFatal( ok, "SimObject::linkNamespaces - failed to link new namespace to c++ class name" );
         parentNamespace = superClassNamespace;
      }
      else
      {
         // In debug builds, make sure the namespace hierarchy that's been
         // put into place actually makes sense and leads back to the C++
         // class namespace.

#ifdef TORQUE_DEBUG

         bool foundClassNameNS = false;
         for(  Namespace* linkWalk = superClassNamespacePackageRoot->getParent(); linkWalk != NULL;
               linkWalk = linkWalk->getParent() )
         {
            if( linkWalk == cppNamespace )
            {
               foundClassNameNS = true;
               break;
            }
         }

         if( !foundClassNameNS )
         {
            // C++ class namespace not in parent link chain.  Warn about it.

            Con::errorf(
               "SimObject::linkNamespaces - cannot link object to superclass %s because c++ class %s is not in the parent namespace chain.  Linking object to c++ class.",
               mSuperClassName,
               getClassName()
            );
            
            // Clear out superclass name so we don't come across it during
            // unlinking.

            mSuperClassName = NULL;
         }
         else

#endif
         {
            // Super link is ok.

            parentNamespace = superClassNamespace;

            // Now increase the reference count of all namespaces in the parent hierarchy
            // (up to the C++ class).

            for( Namespace* linkWalk = parentNamespace;
                 linkWalk != NULL && linkWalk != cppNamespace && linkWalk->getParent() != NULL;
                 linkWalk = linkWalk->getParent() )
            {
               // Skip namespaces coming from packages.
               if( linkWalk->getPackage() != NULL )
                  continue;

               linkWalk->incRefCountToParent();
            }
         }
      }
   }

   // If class name is set, link it in as the new parent
   // which itself inherits from the current parent.

   if( mClassName && mClassName[ 0 ] )
   {
      Namespace* classNamespace = Con::lookupNamespace( mClassName );
      if( classNamespace && classNamespace->classLinkTo( parentNamespace ) )
      {
         parentNamespace = classNamespace;
      }
      else
      {
         // Clear out class name so we don't perform a bogus unlink
         // in unlinkNamespaces().
         mClassName = NULL;
      }
   }

   // Finally, if we have an object name, link its namespace
   // as the child to the current parent namespace and let it
   // become the final namespace of this object.

   StringTableEntry objectName = getName();
   if( objectName && objectName[ 0 ] )
   {
      Namespace* objectNamespace = Con::lookupNamespace( objectName );
      if( objectNamespace && objectNamespace->classLinkTo( parentNamespace ) )
      {
         parentNamespace = objectNamespace;
      }
   }

   // Store our namespace.

   mNameSpace = parentNamespace;
}

//-----------------------------------------------------------------------------

void SimObject::unlinkNamespaces()
{
   if( !mNameSpace )
      return;

   Namespace* cppNamespace = getClassRep()->getNameSpace();
   Namespace* parentNamespace = cppNamespace;

   // Handle superclass.

   if( mSuperClassName && mSuperClassName[ 0 ] )
   {
      // Get the superclass namespace.

      Namespace* superClassNamespace = Con::lookupNamespace( mSuperClassName );

      // Make it the parent namespace.
      
      parentNamespace = superClassNamespace;

      // Decrease parent refcounts on the superclass hierarchy.

      for( Namespace* linkWalk = superClassNamespace;
           linkWalk != NULL && linkWalk != cppNamespace && linkWalk->getParent() != NULL; )
      {
         // Store the parent link since it may disappear once we
         // decrease the reference count.
         Namespace* parent = linkWalk->getParent();

         // Decrease the refcount.
         if( linkWalk->getPackage() == NULL ) // Skip namespaces coming from packages.
            linkWalk->decRefCountToParent();

         // Walk up.
         linkWalk = parent;
      }
   }

   // Handle class.

   if( mClassName && mClassName[ 0 ] )
   {
      Namespace* classNamespace = Con::lookupNamespace( mClassName );
      if( classNamespace )
      {
         classNamespace->decRefCountToParent();
         parentNamespace = classNamespace;
      }
   }

   // Handle object name.

   StringTableEntry objectName = getName();
   if( objectName && objectName[ 0 ] )
      mNameSpace->decRefCountToParent();

   mNameSpace = NULL;
}

//-----------------------------------------------------------------------------

void SimObject::setClassNamespace( const char *classNamespace )
{
   StringTableEntry oldClassNamespace = mClassName;
   StringTableEntry newClassNamespace = StringTable->insert( classNamespace );
   
   if( oldClassNamespace == newClassNamespace )
      return;
      
   if( isProperlyAdded() )
      unlinkNamespaces();

   mClassName = newClassNamespace;
   
   if( isProperlyAdded() )
   {
      linkNamespaces();
      
      // Restore old namespace setup if linkage failed.
      
      if( mClassName != newClassNamespace )
      {
         mClassName = oldClassNamespace;
         linkNamespaces();
      }
   }
}

//-----------------------------------------------------------------------------

void SimObject::setSuperClassNamespace( const char *superClassNamespace )
{
   StringTableEntry oldSuperClassNamespace = mSuperClassName;
   StringTableEntry newSuperClassNamespace = StringTable->insert( superClassNamespace );
   
   if( oldSuperClassNamespace == newSuperClassNamespace )
      return;
      
   if( isProperlyAdded() )
      unlinkNamespaces();
   
   mSuperClassName = newSuperClassNamespace;
   
   if( isProperlyAdded() )
   {
      linkNamespaces();
      
      // Restore old setup if linkage failed.
      
      if( mSuperClassName != newSuperClassNamespace )
      {
         mSuperClassName = oldSuperClassNamespace;
         linkNamespaces();
      }
   }
}

//=============================================================================
//    Misc.
//=============================================================================
// MARK: ---- Misc ----

//-----------------------------------------------------------------------------

void SimObject::setInternalName( const char* newname )
{
   if( newname )
      mInternalName = StringTable->insert( newname );
   else
      mInternalName = StringTable->EmptyString();
}

//-----------------------------------------------------------------------------

void SimObject::setOriginalName( const char* originalName )
{
   if( originalName )
      mOriginalName = StringTable->insert( originalName );
   else
      mOriginalName = StringTable->EmptyString();
}

//-----------------------------------------------------------------------------

const char *SimObject::tabComplete(const char *prevText, S32 baseLen, bool fForward)
{
   return mNameSpace->tabComplete(prevText, baseLen, fForward);
}

//-----------------------------------------------------------------------------

void SimObject::setSelected( bool sel )
{
   if( mFlags.test( Selected ) == sel )
      return; // No change.

   if( sel )
   {
      mFlags.set( Selected );
      _onSelected();
   }
   else
   {
      mFlags.clear( Selected );
      _onUnselected();
   }
}

//-----------------------------------------------------------------------------

bool SimObject::isSelectedRecursive() const
{
   const SimObject *walk = this;
   while ( walk )
   {
      if ( walk->isSelected() )
         return true;
      walk = walk->getGroup();
   }

   return false;   
}

//-----------------------------------------------------------------------------

void SimObject::setLocked( bool b )
{
   if( b )
      mFlags.set( Locked );
   else
      mFlags.clear( Locked );
}

//-----------------------------------------------------------------------------

void SimObject::setHidden( bool b )
{
   if( b )
      mFlags.set( Hidden );
   else
      mFlags.clear( Hidden );
}

//-----------------------------------------------------------------------------

void SimObject::setCopySource( SimObject* object )
{
   if( mCopySource )
      mCopySource->unregisterReference( &mCopySource );
   mCopySource = object;
   if( mCopySource )
      mCopySource->registerReference( &mCopySource );
}

//---------------------------------------------------------------------------

bool SimObject::_setCanSave( void* object, const char* index, const char* data )
{
   SimObject* obj = reinterpret_cast< SimObject* >( object );
   obj->setCanSave( dAtob( data ) );
   return false;
}

//-----------------------------------------------------------------------------

const char* SimObject::_getCanSave( void* object, const char* data )
{
   SimObject* obj = reinterpret_cast< SimObject* >( object );
   if( obj->getCanSave() )
      return "1";
   else
      return "0";
}

//---------------------------------------------------------------------------

// Copy SimObject to another SimObject (Originally designed for T2D).
void SimObject::copyTo( SimObject* object  )
{
   object->mClassName = mClassName;
   object->mSuperClassName = mSuperClassName;

   linkNamespaces();
}

//-----------------------------------------------------------------------------

bool SimObject::setProtectedParent( void *obj, const char *index, const char *data )
{
   SimGroup *parent = NULL;
   SimObject *object = static_cast<SimObject*>(obj);

   if(Sim::findObject(data, parent))
      parent->addObject(object);

   // always return false, because we've set mGroup when we called addObject
   return false;
}

//-----------------------------------------------------------------------------

bool SimObject::setProtectedName(void *obj, const char *index, const char *data)
{   
   SimObject *object = static_cast<SimObject*>(obj);
   
   if ( object->isProperlyAdded() )
      object->assignName( data );   

   // always return false because we assign the name here
   return false;
}

//-----------------------------------------------------------------------------

void SimObject::inspectPreApply()
{
}

//-----------------------------------------------------------------------------

void SimObject::inspectPostApply()
{
}

//-----------------------------------------------------------------------------

String SimObject::_getLogMessage(const char* fmt, va_list args) const
{
   String objClass = "UnknownClass";
   if(getClassRep())
      objClass = getClassRep()->getClassName();
      
   String objName = getName();
   if(objName.isEmpty())
      objName = "Unnamed";
   
   String formattedMessage = String::VToString(fmt, args);
   return String::ToString("%s - %s(%i) - %s", 
      objClass.c_str(), objName.c_str(), getId(), formattedMessage.c_str());
}

//=============================================================================
//    API.
//=============================================================================
// MARK: ---- API ----

//-----------------------------------------------------------------------------

DefineEngineMethod( SimObject, dumpGroupHierarchy, void, (),,
   "Dump the hierarchy of this object up to RootGroup to the console." )
{
   object->dumpGroupHierarchy();
}

//-----------------------------------------------------------------------------

DefineConsoleMethod( SimObject, isMethod, bool, ( const char* methodName ),,
   "Test whether the given method is defined on this object.\n"
   "@param The name of the method.\n"
   "@return True if the object implements the given method." )
{
   return object->isMethod( methodName );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SimObject, isChildOfGroup, bool, ( SimGroup* group ),,
   "Test whether the object belongs directly or indirectly to the given group.\n"
   "@param group The SimGroup object.\n"
   "@return True if the object is a child of the given group or a child of a group that the given group is directly or indirectly a child to." )
{
   return object->isChildOfGroup( group );
}

//-----------------------------------------------------------------------------

DefineConsoleMethod( SimObject, getClassNamespace, const char*, (),,
   "Get the name of the class namespace assigned to this object.\n"
   "@return The name of the 'class' namespace." )
{
   return object->getClassNamespace();
}

//-----------------------------------------------------------------------------

DefineConsoleMethod( SimObject, getSuperClassNamespace, const char*, (),,
   "Get the name of the superclass namespace assigned to this object.\n"
   "@return The name of the 'superClass' namespace." )
{
   return object->getSuperClassNamespace();
}

//-----------------------------------------------------------------------------

DefineConsoleMethod( SimObject, setClassNamespace, void, ( const char* name ),,
   "Assign a class namespace to this object.\n"
   "@param name The name of the 'class' namespace for this object." )
{
   object->setClassNamespace( name );
}

//-----------------------------------------------------------------------------

DefineConsoleMethod( SimObject, setSuperClassNamespace, void, ( const char* name ),,
   "Assign a superclass namespace to this object.\n"
   "@param name The name of the 'superClass' namespace for this object." )
{
   object->setSuperClassNamespace( name );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SimObject, isSelected, bool, (),,
   "Get whether the object has been marked as selected. (in editor)\n"
   "@return True if the object is currently selected." )
{
   return object->isSelected();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SimObject, setIsSelected, void, ( bool state ), ( true ),
   "Set whether the object has been marked as selected. (in editor)\n"
   "@param state True if object is to be marked selected; false if not." )
{
   object->setSelected( state );
}

//-----------------------------------------------------------------------------

DefineConsoleMethod( SimObject, isExpanded, bool, (),,
   "Get whether the object has been marked as expanded. (in editor)\n"
   "@return True if the object is marked expanded." )
{
   return object->isExpanded();
}

//-----------------------------------------------------------------------------

DefineConsoleMethod( SimObject, setIsExpanded, void, ( bool state ), ( true ),
   "Set whether the object has been marked as expanded. (in editor)\n"
   "@param state True if the object is to be marked expanded; false if not." )
{
   object->setExpanded( state );
}

//-----------------------------------------------------------------------------

DefineConsoleMethod( SimObject, getFilename, const char*, (),,
   "Returns the filename the object is attached to.\n"
   "@return The name of the file the object is associated with; usually the file the object was loaded from." )
{
   return object->getFilename();
}

//-----------------------------------------------------------------------------

DefineConsoleMethod( SimObject, setFilename, void, ( const char* fileName ),,
   "Sets the object's file name and path\n"
   "@param fileName The name of the file to associate this object with." )
{
   return object->setFilename( fileName );
}

//-----------------------------------------------------------------------------

DefineConsoleMethod( SimObject, getDeclarationLine, S32, (),,
   "Get the line number at which the object is defined in its file.\n\n"
   "@return The line number of the object's definition in script.\n"
   "@see getFilename()")
{
   return object->getDeclarationLine();
}

//-----------------------------------------------------------------------------

#ifdef TORQUE_DEBUG

static const char* sEnumCallbackFunction;
static void sEnumCallback( EngineObject* object )
{
   SimObject* simObject = dynamic_cast< SimObject* >( object );
   if( !simObject )
      return;
      
   Con::evaluatef( "%s( %i );", sEnumCallbackFunction, simObject->getId() );
}

DefineEngineFunction( debugEnumInstances, void, ( const char* className, const char* functionName ),,
   "Call the given function for each instance of the given class.\n"
   "@param className Name of the class for which to enumerate instances.\n"
   "@param functionName Name of function to call and pass each instance of the given class.\n"
   "@note This function is only available in debug builds and primarily meant as an aid in debugging."
   "@ingroup Console")
{
   sEnumCallbackFunction = functionName;
   ConsoleObject::debugEnumInstances( className, sEnumCallback );
}

#endif

//-----------------------------------------------------------------------------

DefineConsoleMethod( SimObject, assignFieldsFrom, void, ( SimObject* fromObject ),,
   "Copy fields from another object onto this one.  The objects must "
   "be of same type. Everything from the object will overwrite what's "
   "in this object; extra fields in this object will remain. This "
   "includes dynamic fields.\n"
   "@param fromObject The object from which to copy fields." )
{
   if( fromObject )
      object->assignFieldsFrom( fromObject );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SimObject, assignPersistentId, void, (),,
   "Assign a persistent ID to the object if it does not already have one." )
{
   object->getOrCreatePersistentId();
}

//-----------------------------------------------------------------------------

DefineConsoleMethod( SimObject, getCanSave, bool, (),,
   "Get whether the object will be included in saves.\n"
   "@return True if the object will be saved; false otherwise." )
{
   return object->getCanSave();
}

//-----------------------------------------------------------------------------

DefineConsoleMethod( SimObject, setCanSave, void, ( bool value ), ( true ),
   "Set whether the object will be included in saves.\n"
   "@param value If true, the object will be included in saves; if false, it will be excluded." )
{
   object->setCanSave( value );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SimObject, isEditorOnly, bool, (),,
   "Return true if the object is only used by the editor.\n"
   "@return True if this object exists only for the sake of editing." )
{
   return object->isEditorOnly();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SimObject, setEditorOnly, void, ( bool value ), ( true ),
   "Set/clear the editor-only flag on this object.\n"
   "@param value If true, the object is marked as existing only for the editor." )
{
   object->setEditorOnly( value );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SimObject, isNameChangeAllowed, bool, (),,
   "Get whether this object may be renamed.\n"
   "@return True if this object can be renamed; false otherwise." )
{
   return object->isNameChangeAllowed();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SimObject, setNameChangeAllowed, void, ( bool value ), ( true ),
   "Set whether this object can be renamed from its first name.\n"
   "@param value If true, renaming is allowed for this object; if false, trying to change the name of the object will generate a console error." )
{
   object->setNameChangeAllowed( value );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SimObject, clone, SimObject*, (),,
   "Create a copy of this object.\n"
   "@return An exact duplicate of this object." )
{
   return object->clone();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SimObject, deepClone, SimObject*, (),,
   "Create a copy of this object and all its subobjects.\n"
   "@return An exact duplicate of this object and all objects it references." )
{
   return object->deepClone();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SimObject, setLocked, void, ( bool value ), ( true ),
   "Lock/unlock the object in the editor.\n"
   "@param value If true, the object will be locked; if false, the object will be unlocked." )
{
   object->setLocked( value );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SimObject, setHidden, void, ( bool value ), ( true ),
   "Hide/unhide the object.\n"
   "@param value If true, the object will be hidden; if false, the object will be unhidden." )
{
   object->setHidden( value );
}

//-----------------------------------------------------------------------------

DefineConsoleMethod( SimObject, dumpMethods, ArrayObject*, (),,
   "List the methods defined on this object.\n\n"
   "Each description is a newline-separated vector with the following elements:\n"
   "- Minimum number of arguments.\n"
   "- Maximum number of arguments.\n"
   "- Prototype string.\n"
   "- Full script file path (if script method).\n"
   "- Line number of method definition in script (if script method).\n\n"
   "- Documentation string (not including prototype).  This takes up the remainder of the vector.\n"
   "@return An ArrayObject populated with (name,description) pairs of all methods defined on the object." )
{
   Namespace *ns = object->getNamespace();
   if( !ns )
      return 0;
      
   ArrayObject* dictionary = new ArrayObject();
   dictionary->registerObject();
   
   VectorPtr<Namespace::Entry *> vec(__FILE__, __LINE__);
   ns->getEntryList(&vec);

   for(Vector< Namespace::Entry* >::iterator j = vec.begin(); j != vec.end(); j++)
   {
      Namespace::Entry* e = *j;

      if( e->mType < 0 )
         continue;
         
      StringBuilder str;
      
      str.append( String::ToString( e->mMinArgs ) );
      str.append( '\n' );
      str.append( String::ToString( e->mMaxArgs ) );
      str.append( '\n' );
      str.append( e->getPrototypeString() );

      str.append( '\n' );
      if( e->mCode && e->mCode->fullPath )
         str.append( e->mCode->fullPath );
      str.append( '\n' );
      if( e->mCode )
         str.append( String::ToString( e->mFunctionLineNumber ) );

      str.append( '\n' );
      String docs = e->getDocString();
      if( !docs.isEmpty() )
         str.append( docs );

      dictionary->push_back( e->mFunctionName, str.end() );
   }
   
   return dictionary;
}

//-----------------------------------------------------------------------------

namespace {
   S32 QSORT_CALLBACK compareFields( const void* a,const void* b )
   {
      const AbstractClassRep::Field* fa = *((const AbstractClassRep::Field**)a);
      const AbstractClassRep::Field* fb = *((const AbstractClassRep::Field**)b);

      return dStricmp(fa->pFieldname, fb->pFieldname);
   }
   
   struct DocString
   {
      char mPadding[ 8 ];
      String mPrototype;
      String mDescription;
      const char* mReturnType;
      
      DocString( Namespace::Entry* entry )
         : mPrototype( entry->getArgumentsString() ),
           mDescription( entry->getBriefDescription() )
      {
         mReturnType = "        ";
         mPadding[ 0 ] = 0;
         if( entry->mType == -4 )
         {
            //TODO: need to have script callbacks set up proper return type info
         }
         else
         {
            switch( entry->mType )
            {
               case Namespace::Entry::StringCallbackType:
                  mReturnType = "string";
                  mPadding[ 0 ] = ' ';
                  mPadding[ 1 ] = ' ';
                  mPadding[ 2 ] = 0;
                  break;
                  
               case Namespace::Entry::IntCallbackType:
                  mReturnType = "int";
                  mPadding[ 0 ] = ' ';
                  mPadding[ 1 ] = ' ';
                  mPadding[ 2 ] = ' ';
                  mPadding[ 3 ] = ' ';
                  mPadding[ 4 ] = ' ';
                  mPadding[ 5 ] = 0;
                  break;

               case Namespace::Entry::FloatCallbackType:
                  mReturnType = "float";
                  mPadding[ 0 ] = ' ';
                  mPadding[ 1 ] = ' ';
                  mPadding[ 2 ] = ' ';
                  mPadding[ 3 ] = 0;
                  break;

               case Namespace::Entry::VoidCallbackType:
                  mReturnType = "void";
                  mPadding[ 0 ] = ' ';
                  mPadding[ 1 ] = ' ';
                  mPadding[ 2 ] = ' ';
                  mPadding[ 3 ] = ' ';
                  mPadding[ 4 ] = 0;
                  break;

               case Namespace::Entry::BoolCallbackType:
                  mReturnType = "bool";
                  mPadding[ 0 ] = ' ';
                  mPadding[ 1 ] = ' ';
                  mPadding[ 2 ] = ' ';
                  mPadding[ 3 ] = ' ';
                  mPadding[ 4 ] = 0;
                  break;                  
            }
         }
      }
   };
}

DefineEngineMethod( SimObject, dump, void, ( bool detailed ), ( false ),
   "Dump a description of all fields and methods defined on this object to the console.\n"
   "@param detailed Whether to print detailed information about members." )
{
   Con::printf( "Class: %s", object->getClassName() );
   
   const AbstractClassRep::FieldList &list = object->getFieldList();
   char expandedBuffer[4096];

   Con::printf( "Static Fields:" );
   Vector<const AbstractClassRep::Field *> flist(__FILE__, __LINE__);

   for(U32 i = 0; i < list.size(); i++)
      flist.push_back(&list[i]);

   dQsort(flist.address(),flist.size(),sizeof(AbstractClassRep::Field *),compareFields);

   for(Vector<const AbstractClassRep::Field *>::iterator itr = flist.begin(); itr != flist.end(); itr++)
   {
      const AbstractClassRep::Field* f = *itr;

      // The special field types can be skipped.
      if ( f->type >= AbstractClassRep::ARCFirstCustomField )
         continue;

      for(U32 j = 0; S32(j) < f->elementCount; j++)
      {
         // [neo, 07/05/2007 - #3000]
         // Some objects use dummy vars and projected fields so make sure we call the get functions 
         //const char *val = Con::getData(f->type, (void *) (((const char *)object) + f->offset), j, f->table, f->flag);                          
         const char *val = (*f->getDataFn)( object, Con::getData(f->type, (void *) (((const char *)object) + f->offset), j, f->table, f->flag) );// + typeSizes[fld.type] * array1));
         
         ConsoleBaseType* conType = ConsoleBaseType::getType( f->type );
         const char* conTypeName = "<unknown>";
         if( conType )
            conTypeName = conType->getTypeClassName();

         if( !val /*|| !*val*/ )
            continue;
         if( f->elementCount == 1 )
            dSprintf( expandedBuffer, sizeof( expandedBuffer ), "  %s %s = \"", conTypeName, f->pFieldname );
         else
            dSprintf( expandedBuffer, sizeof( expandedBuffer ), "  %s %s[ %d ] = \"", conTypeName, f->pFieldname, j );
         expandEscape( expandedBuffer + dStrlen(expandedBuffer), val);
         Con::printf( "%s\"", expandedBuffer );
         
         if( detailed && f->pFieldDocs && f->pFieldDocs[ 0 ] )
            Con::printf( "    %s", f->pFieldDocs );
      }
   }

   Con::printf( "Dynamic Fields:" );
   if(object->getFieldDictionary())
      object->getFieldDictionary()->printFields(object);

   Con::printf( "Methods:" );
   Namespace *ns = object->getNamespace();
   VectorPtr<Namespace::Entry *> vec(__FILE__, __LINE__);

   if(ns)
      ns->getEntryList(&vec);

   bool sawCBs = false;

   for(Vector<Namespace::Entry *>::iterator j = vec.begin(); j != vec.end(); j++)
   {
      Namespace::Entry *e = *j;

      if(e->mType == Namespace::Entry::ScriptCallbackType)
         sawCBs = true;

      if(e->mType < 0)
         continue;
         
      DocString doc( e );
      Con::printf( "  %s%s%s%s", doc.mReturnType, doc.mPadding, e->mFunctionName, doc.mPrototype.c_str() );

      if( detailed && !doc.mDescription.isEmpty() )
         Con::printf( "    %s", doc.mDescription.c_str() );
   }

   if( sawCBs )
   {
      Con::printf( "Callbacks:" );

      for(Vector<Namespace::Entry *>::iterator j = vec.begin(); j != vec.end(); j++)
      {
         Namespace::Entry *e = *j;

         if(e->mType != Namespace::Entry::ScriptCallbackType)
            continue;

         DocString doc( e );
         Con::printf( "  %s%s%s%s", doc.mReturnType, doc.mPadding, e->cb.mCallbackName, doc.mPrototype.c_str() );

         if( detailed && !doc.mDescription.isEmpty() )
            Con::printf( "    %s", doc.mDescription.c_str() );
      }
   }
}

//-----------------------------------------------------------------------------

DefineConsoleMethod( SimObject, save, bool, ( const char* fileName, bool selectedOnly, const char* preAppendString ), ( false, "" ),
   "Save out the object to the given file.\n"
   "@param fileName The name of the file to save to."
   "@param selectedOnly If true, only objects marked as selected will be saved out.\n"
   "@param preAppendString Text which will be preprended directly to the object serialization.\n"
   "@param True on success, false on failure." )
{
   return object->save( fileName, selectedOnly, preAppendString );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SimObject, setName, void, ( const char* newName ),,
   "Set the global name of the object.\n"
   "@param newName The new global name to assign to the object.\n"
   "@note If name changing is disallowed on the object, the method will fail with a console error." )
{
   object->assignName( newName );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SimObject, getName, const char*, (),,
   "Get the global name of the object.\n"
   "@return The global name assigned to the object." )
{
   const char *ret = object->getName();
   return ret ? ret : "";
}

//-----------------------------------------------------------------------------

DefineConsoleMethod( SimObject, getClassName, const char*, (),,
   "Get the name of the C++ class which the object is an instance of.\n"
   "@return The name of the C++ class of the object." )
{
   const char *ret = object->getClassName();
   return ret ? ret : "";
}

//-----------------------------------------------------------------------------

DefineConsoleMethod( SimObject, isField, bool, ( const char* fieldName ),,
   "Test whether the given field is defined on this object.\n"
   "@param fieldName The name of the field.\n"
   "@return True if the object implements the given field." )
{
   return object->isField( fieldName );
}

//-----------------------------------------------------------------------------

DefineConsoleMethod( SimObject, getFieldValue, const char*, ( const char* fieldName, S32 index ), ( -1 ),
   "Return the value of the given field on this object.\n"
   "@param fieldName The name of the field.  If it includes a field index, the index is parsed out.\n"
   "@param index Optional parameter to specify the index of an array field separately.\n"
   "@return The value of the given field or \"\" if undefined." )
{
   char fieldNameBuffer[ 1024 ];
   char arrayIndexBuffer[ 64 ];
   
   // Parse out index if the field is given in the form of 'name[index]'.
   
   const char* arrayIndex = NULL;
   const U32 nameLen = dStrlen( fieldName );
   if( fieldName[ nameLen - 1 ] == ']' )
   {
      const char* leftBracket = dStrchr( fieldName, '[' );
      const char* rightBracket = &fieldName[ nameLen - 1 ];
      
      const U32 fieldNameLen = getMin( U32( leftBracket - fieldName ), sizeof( fieldNameBuffer ) - 1 );
      const U32 arrayIndexLen = getMin( U32( rightBracket - leftBracket - 1 ), sizeof( arrayIndexBuffer ) - 1 );
      
      dMemcpy( fieldNameBuffer, fieldName, fieldNameLen );
      dMemcpy( arrayIndexBuffer, leftBracket + 1, arrayIndexLen );
      
      fieldNameBuffer[ fieldNameLen ] = '\0';
      arrayIndexBuffer[ arrayIndexLen ] = '\0';
      
      fieldName = fieldNameBuffer;
      arrayIndex = arrayIndexBuffer;
   }

   fieldName = StringTable->insert( fieldName );
   
   if( index != -1 )
   {
      dSprintf( arrayIndexBuffer, sizeof( arrayIndexBuffer ), "%i", index );
      arrayIndex = arrayIndexBuffer;
   }
   
   return object->getDataField( fieldName, arrayIndex );
}

//-----------------------------------------------------------------------------

DefineConsoleMethod( SimObject, setFieldValue, bool, ( const char* fieldName, const char* value, S32 index ), ( -1 ),
   "Set the value of the given field on this object.\n"
   "@param fieldName The name of the field to assign to.  If it includes an array index, the index will be parsed out.\n"
   "@param value The new value to assign to the field.\n"
   "@param index Optional argument to specify an index for an array field.\n"
   "@return True." )
{
   char fieldNameBuffer[ 1024 ];
   char arrayIndexBuffer[ 64 ];
   
   // Parse out index if the field is given in the form of 'name[index]'.
   
   const char* arrayIndex = NULL;
   const U32 nameLen = dStrlen( fieldName );
   if( fieldName[ nameLen - 1 ] == ']' )
   {
      const char* leftBracket = dStrchr( fieldName, '[' );
      const char* rightBracket = &fieldName[ nameLen - 1 ];
      
      const U32 fieldNameLen = getMin( U32( leftBracket - fieldName ), sizeof( fieldNameBuffer ) - 1 );
      const U32 arrayIndexLen = getMin( U32( rightBracket - leftBracket - 1 ), sizeof( arrayIndexBuffer ) - 1 );
      
      dMemcpy( fieldNameBuffer, fieldName, fieldNameLen );
      dMemcpy( arrayIndexBuffer, leftBracket + 1, arrayIndexLen );
      
      fieldNameBuffer[ fieldNameLen ] = '\0';
      arrayIndexBuffer[ arrayIndexLen ] = '\0';
      
      fieldName = fieldNameBuffer;
      arrayIndex = arrayIndexBuffer;
   }

   fieldName = StringTable->insert( fieldName );

   if( index != -1 )
   {
      dSprintf( arrayIndexBuffer, sizeof( arrayIndexBuffer ), "%i", index );
      arrayIndex = arrayIndexBuffer;
   }

   object->setDataField( fieldName, arrayIndex, value );

   return true;
}

//-----------------------------------------------------------------------------

DefineConsoleMethod( SimObject, getFieldType, const char*, ( const char* fieldName ),,
   "Get the console type code of the given field.\n"
   "@return The numeric type code for the underlying console type of the given field." )
{
   U32 typeID = object->getDataFieldType( StringTable->insert( fieldName ), NULL );
   ConsoleBaseType* type = ConsoleBaseType::getType( typeID );

   if( type )
      return type->getTypeName();

   return "";
}

//-----------------------------------------------------------------------------

DefineConsoleMethod( SimObject, setFieldType, void, ( const char* fieldName, const char* type ),,
   "Set the console type code for the given field.\n"
   "@param fieldName The name of the dynamic field to change to type for.\n"
   "@param type The name of the console type.\n"
   "@note This only works for dynamic fields.  Types of static fields cannot be changed." )
{
   object->setDataFieldType( type, StringTable->insert( fieldName ), NULL );
}

//-----------------------------------------------------------------------------

ConsoleMethod( SimObject, call, const char*, 3, 0, "( string method, string args... ) Dynamically call a method on an object.\n"
   "@param method Name of method to call.\n"
   "@param args Zero or more arguments for the method.\n"
   "@return The result of the method call." )
{
   argv[1] = argv[2];
   return Con::execute( object, argc - 1, argv + 1 );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SimObject, setInternalName, void, ( const char* newInternalName ),,
   "Set the internal name of the object.\n"
   "@param newInternalName The new internal name for the object." )
{
   object->setInternalName( newInternalName );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SimObject, getInternalName, const char*, (),,
   "Get the internal name of the object.\n"
   "@return The internal name of the object." )
{
   return object->getInternalName();
}

//-----------------------------------------------------------------------------

DefineConsoleMethod( SimObject, dumpClassHierarchy, void, (),,
   "Dump the native C++ class hierarchy of this object's C++ class to the console." )
{
   object->dumpClassHierarchy();
}

//-----------------------------------------------------------------------------

DefineConsoleMethod( SimObject, isMemberOfClass, bool, ( const char* className ),,
   "Test whether this object is a member of the specified class.\n"
   "@param className Name of a native C++ class.\n"
   "@return True if this object is an instance of the given C++ class or any of its super classes." )
{
   AbstractClassRep* pRep = object->getClassRep();
   while(pRep)
   {
      if( !dStricmp(pRep->getClassName(), className ) )
      {
         //matches
         return true;
      }

      pRep  =  pRep->getParentClass();
   }

   return false;
}

//-----------------------------------------------------------------------------

DefineConsoleMethod( SimObject, isInNamespaceHierarchy, bool, ( const char* name ),,
   "Test whether the namespace of this object is a direct or indirect child to the given namespace.\n"
   "@param name The name of a namespace.\n"
   "@return True if the given namespace name is within the namespace hierarchy of this object." )
{
   Namespace* nspace = object->getNamespace();
      
   while( nspace && dStricmp( nspace->mName, name ) != 0 )
      nspace = nspace->mParent;
      
   return ( nspace != NULL );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SimObject, getId, S32, (),,
   "Get the underlying unique numeric ID of the object.\n"
   "@note Object IDs are unique only during single engine runs.\n"
   "@return The unique numeric ID of the object." )
{
   return object->getId();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SimObject, getGroup, SimGroup*, (),,
   "Get the group that this object is contained in.\n"
   "@note If not assigned to particular SimGroup, an object belongs to RootGroup.\n"
   "@return The SimGroup object to which the object belongs." )
{
   return object->getGroup();
}

//-----------------------------------------------------------------------------

DefineConsoleMethod( SimObject, delete, void, (),,
   "Delete and remove the object." )
{
   object->deleteObject();
}

//-----------------------------------------------------------------------------

ConsoleMethod( SimObject,schedule, S32, 4, 0, "( float time, string method, string args... ) Delay an invocation of a method.\n"
   "@param time The number of milliseconds after which to invoke the method.  This is a soft limit.\n"
   "@param method The method to call.\n"
   "@param args The arguments with which to call the method.\n"
   "@return The numeric ID of the created schedule.  Can be used to cancel the call.\n" )
{
   U32 timeDelta = U32(dAtof(argv[2]));
   argv[2] = argv[3];
   argv[3] = argv[1];
   SimConsoleEvent *evt = new SimConsoleEvent(argc - 2, argv + 2, true);
   S32 ret = Sim::postEvent(object, evt, Sim::getCurrentTime() + timeDelta);
   // #ifdef DEBUG
   //    Con::printf("obj %s schedule(%s) = %d", argv[3], argv[2], ret);
   //    Con::executef( "backtrace");
   // #endif
   return ret;
}

//-----------------------------------------------------------------------------

DefineConsoleMethod( SimObject, getDynamicFieldCount, S32, (),,
   "Get the number of dynamic fields defined on the object.\n"
   "@return The number of dynamic fields defined on the object." )
{
   S32 count = 0;
   SimFieldDictionary* fieldDictionary = object->getFieldDictionary();
   for (SimFieldDictionaryIterator itr(fieldDictionary); *itr; ++itr)
      count++;

   return count;
}

//-----------------------------------------------------------------------------

DefineConsoleMethod( SimObject, getDynamicField, const char*, ( S32 index ),,
   "Get a value of a dynamic field by index.\n"
   "@param index The index of the dynamic field.\n"
   "@return The value of the dynamic field at the given index or \"\"." )
{
   SimFieldDictionary* fieldDictionary = object->getFieldDictionary();
   SimFieldDictionaryIterator itr(fieldDictionary);
   for (S32 i = 0; i < index; i++)
   {
      if (!(*itr))
      {
         Con::warnf("Invalid dynamic field index passed to SimObject::getDynamicField!");
         return NULL;
      }
      ++itr;
   }

   static const U32 bufSize = 256;
   char* buffer = Con::getReturnBuffer(bufSize);
   if (*itr)
   {
      SimFieldDictionary::Entry* entry = *itr;
      dSprintf(buffer, bufSize, "%s\t%s", entry->slotName, entry->value);
      return buffer;
   }

   Con::warnf("Invalid dynamic field index passed to SimObject::getDynamicField!");
   return NULL;
}

//-----------------------------------------------------------------------------

DefineConsoleMethod( SimObject, getFieldCount, S32, (),,
   "Get the number of static fields on the object.\n"
   "@return The number of static fields defined on the object." )
{
   const AbstractClassRep::FieldList &list = object->getFieldList();
   const AbstractClassRep::Field* f;
   U32 numDummyEntries = 0;

   for(S32 i = 0; i < list.size(); i++)
   {
      f = &list[i];

      // The special field types do not need to be counted.
      if ( f->type >= AbstractClassRep::ARCFirstCustomField )
         numDummyEntries++;
   }

   return list.size() - numDummyEntries;
}

//-----------------------------------------------------------------------------

DefineConsoleMethod( SimObject, getField, const char*, ( S32 index ),,
   "Retrieve the value of a static field by index.\n"
   "@param index The index of the static field.\n"
   "@return The value of the static field with the given index or \"\"." )
{
   const AbstractClassRep::FieldList &list = object->getFieldList();
   if( ( index < 0 ) || ( index >= list.size() ) )
      return "";

   const AbstractClassRep::Field* f;
   S32 currentField = 0;
   for ( U32 i = 0; i < list.size() && currentField <= index; i++ )
   {
      f = &list[i];

      // The special field types can be skipped.
      if ( f->type >= AbstractClassRep::ARCFirstCustomField )
         continue;

      if(currentField == index)
         return f->pFieldname;

      currentField++;
   }

   // if we found nada, return nada.
   return "";
}

//-----------------------------------------------------------------------------

#ifdef TORQUE_DEBUG

DefineEngineMethod( SimObject, getDebugInfo, ArrayObject*, (),,
   "Return some behind-the-scenes information on the object.\n"
   "@return An ArrayObject filled with internal information about the object." )
{
   ArrayObject* array = new ArrayObject();
   array->registerObject();
   
   array->push_back( "C++|Address", String::ToString( "0x%x", object ) );
   array->push_back( "C++|Size", String::ToString( object->getClassRep()->getSizeof() ) );
   array->push_back( "Object|Description", object->describeSelf() );
   array->push_back( "Object|FileName", object->getFilename() );
   array->push_back( "Object|DeclarationLine", String::ToString( object->getDeclarationLine() ) );
   array->push_back( "Object|CopySource", object->getCopySource() ?
      String::ToString( "%i:%s (%s)", object->getCopySource()->getId(), object->getCopySource()->getClassName(), object->getCopySource()->getName() ) : "" );
   array->push_back( "Flag|EditorOnly", object->isEditorOnly() ? "true" : "false" );
   array->push_back( "Flag|NameChangeAllowed", object->isNameChangeAllowed() ? "true" : "false" );
   array->push_back( "Flag|AutoDelete", object->isAutoDeleted() ? "true" : "false" );
   array->push_back( "Flag|Selected", object->isSelected() ? "true" : "false" );
   array->push_back( "Flag|Expanded", object->isExpanded() ? "true" : "false" );
   array->push_back( "Flag|ModStaticFields", object->canModStaticFields() ? "true" : "false" );
   array->push_back( "Flag|ModDynamicFields", object->canModDynamicFields() ? "true" : "false" );
   array->push_back( "Flag|CanSave", object->getCanSave() ? "true" : "false" );
   
   #ifndef TORQUE_DISABLE_MEMORY_MANAGER
   Memory::Info memInfo;
   Memory::getMemoryInfo( object, memInfo );
   
   array->push_back( "Memory|AllocNumber", String::ToString( memInfo.mAllocNumber ) );
   array->push_back( "Memory|AllocSize", String::ToString( memInfo.mAllocSize ) );
   array->push_back( "Memory|AllocFile", memInfo.mFileName );
   array->push_back( "Memory|AllocLine", String::ToString( memInfo.mLineNumber ) );
   array->push_back( "Memory|IsGlobal", memInfo.mIsGlobal ? "true" : "false" );
   array->push_back( "Memory|IsStatic", memInfo.mIsStatic ? "true" : "false" );
   #endif
   
   return array;
}

#endif
