//-----------------------------------------------------------------------------
// Copyright (c) 2013 GarageGames, LLC
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

#include "taml.h"

#ifndef _TAML_XMLWRITER_H_
#include "persistence/taml/xml/tamlXmlWriter.h"
#endif

#ifndef _TAML_XMLREADER_H_
#include "persistence/taml/xml/tamlXmlReader.h"
#endif

#ifndef _TAML_XMLPARSER_H_
#include "persistence/taml/xml/tamlXmlParser.h"
#endif

#ifndef _TAML_BINARYWRITER_H_
#include "persistence/taml/binary/tamlBinaryWriter.h"
#endif

#ifndef _TAML_BINARYREADER_H_
#include "persistence/taml/binary/tamlBinaryReader.h"
#endif

/*#ifndef _TAML_JSONWRITER_H_
#include "taml/json/tamlJSONWriter.h"
#endif

#ifndef _TAML_JSONREADER_H_
#include "taml/json/tamlJSONReader.h"
#endif

#ifndef _TAML_JSONPARSER_H_
#include "taml/json/tamlJSONParser.h"
#endif*/

#ifndef _FRAMEALLOCATOR_H_
#include "core/frameAllocator.h"
#endif

#ifndef _SIMBASE_H_
#include "console/simBase.h"
#endif

#ifndef _MATHTYPES_H_
#include "math/mathTypes.h"
#endif

#ifndef _MPOINT2_H_
#include "math/mPoint2.h"
#endif

#ifndef _ASSET_BASE_H_
#include "assets/assetBase.h"
#endif

// Script bindings.
#include "taml_ScriptBinding.h"

// Debug Profiling.
#include "platform/profiler.h"

//-----------------------------------------------------------------------------

IMPLEMENT_CONOBJECT( Taml );

//-----------------------------------------------------------------------------

StringTableEntry tamlRefIdName          = StringTable->insert( "TamlId" );
StringTableEntry tamlRefToIdName        = StringTable->insert( "TamlRefId" );
StringTableEntry tamlNamedObjectName    = StringTable->insert( "Name" );

//-----------------------------------------------------------------------------

typedef Taml::TamlFormatMode _TamlFormatMode;
ImplementEnumType( _TamlFormatMode,
   "")
   { Taml::XmlFormat, "xml" },
   { Taml::BinaryFormat, "binary" }//,
   //{ Taml::JSONFormat, "json" }
EndImplementEnumType;

//-----------------------------------------------------------------------------

Taml::TamlFormatMode Taml::getFormatModeEnum(const char* label)
{
    // Search for Mnemonic.
   for (U32 i = 0; i < (sizeof(__TamlFormatMode::_sEnums) / sizeof(EnumTable::Value)); i++)
    {
       if( dStricmp(__TamlFormatMode::_sEnumTable[i].getName(), label) == 0)
          return (TamlFormatMode)__TamlFormatMode::_sEnumTable[i].getInt();
    }

    // Warn.
    Con::warnf( "Taml::getFormatModeEnum() - Invalid format of '%s'.", label );

    return Taml::InvalidFormat;
}

//-----------------------------------------------------------------------------

const char* Taml::getFormatModeDescription(const Taml::TamlFormatMode formatMode)
{
    // Search for Mnemonic.
    for (U32 i = 0; i < (sizeof(__TamlFormatMode::_sEnums) / sizeof(EnumTable::Value)); i++)
    {
       if( __TamlFormatMode::_sEnumTable[i].getInt() == (S32)formatMode )
          return __TamlFormatMode::_sEnumTable[i].getName();
    }

    // Warn.
    Con::warnf( "Taml::getFormatModeDescription() - Invalid format mode." );

    return StringTable->EmptyString();
}

//-----------------------------------------------------------------------------

// The string-table-entries are set to string literals below because Taml is used in a static scope and the string-table cannot currently be used like that.
Taml::Taml() :
    mFormatMode(XmlFormat),
    mJSONStrict( true ),
    mBinaryCompression(true),
    mWriteDefaults(false),
    mProgenitorUpdate(true),    
    mAutoFormat(true),
    mAutoFormatXmlExtension("taml"),    
    mAutoFormatBinaryExtension("baml"),
    mAutoFormatJSONExtension("json")
{
    // Reset the file-path buffer.
    mFilePathBuffer[0] = 0;
}

//-----------------------------------------------------------------------------

void Taml::initPersistFields()
{
    // Call parent.
    Parent::initPersistFields();

    addField("Format", TYPEID<_TamlFormatMode>(), Offset(mFormatMode, Taml), "The read/write format that should be used.");
    addField("JSONStrict", TypeBool, Offset(mBinaryCompression, Taml), "Whether to write JSON that is strictly compatible with RFC4627 or not.\n");
    addField("BinaryCompression", TypeBool, Offset(mBinaryCompression, Taml), "Whether ZIP compression is used on binary formatting or not.\n");
    addField("WriteDefaults", TypeBool, Offset(mWriteDefaults, Taml), "Whether to write static fields that are at their default or not.\n");
    addField("ProgenitorUpdate", TypeBool, Offset(mProgenitorUpdate, Taml), "Whether to update each type instances file-progenitor or not.\n");
    addField("AutoFormat", TypeBool, Offset(mAutoFormat, Taml), "Whether the format type is automatically determined by the filename extension or not.\n");
    addField("AutoFormatXmlExtension", TypeString, Offset(mAutoFormatXmlExtension, Taml), "When using auto-format, this is the extension (end of filename) used to detect the XML format.\n");
    addField("AutoFormatBinaryExtension", TypeString, Offset(mAutoFormatBinaryExtension, Taml), "When using auto-format, this is the extension (end of filename) used to detect the BINARY format.\n");
    addField("AutoFormatJSONExtension", TypeString, Offset(mAutoFormatJSONExtension, Taml), "When using auto-format, this is the extension (end of filename) used to detect the JSON format.\n");
}

//-----------------------------------------------------------------------------

bool Taml::onAdd()
{
    // Call parent.
    if ( !Parent::onAdd() )
        return false;

    // Set JSON strict mode.
    mJSONStrict = Con::getBoolVariable( TAML_JSON_STRICT_VARIBLE, true );

    // Reset the compilation.
    resetCompilation();

    return true;
}

//-----------------------------------------------------------------------------

void Taml::onRemove()
{
    // Reset the compilation.
    resetCompilation();

    // Call parent.
    Parent::onRemove();
}

//-----------------------------------------------------------------------------

bool Taml::write( SimObject* pSimObject, const char* pFilename )
{
    // Debug Profiling.
    PROFILE_SCOPE(Taml_Write);

    // Sanity!
    AssertFatal( pSimObject != NULL, "Cannot write a NULL object." );
    AssertFatal( pFilename != NULL, "Cannot write to a NULL filename." );

    // Expand the file-name into the file-path buffer.
    Con::expandToolScriptFilename( mFilePathBuffer, sizeof(mFilePathBuffer), pFilename );

    FileStream stream;

    // File opened?
    if ( !stream.open( mFilePathBuffer, Torque::FS::File::Write ) )
    {
        // No, so warn.
        Con::warnf("Taml::writeFile() - Could not open filename '%s' for write.", mFilePathBuffer );
        return false;
    }

    // Get the file auto-format mode.
    const TamlFormatMode formatMode = getFileAutoFormatMode( mFilePathBuffer );

    // Reset the compilation.
    resetCompilation();

    // Write object.
    const bool status = write( stream, pSimObject, formatMode );

    // Close file.
    stream.close();

    // Reset the compilation.
    resetCompilation();

    return status;
}

//-----------------------------------------------------------------------------

SimObject* Taml::read( const char* pFilename )
{
    // Debug Profiling.
    PROFILE_SCOPE(Taml_Read);

    // Sanity!
    AssertFatal( pFilename != NULL, "Cannot read from a NULL filename." );

    // Expand the file-name into the file-path buffer.
    Con::expandToolScriptFilename( mFilePathBuffer, sizeof(mFilePathBuffer), pFilename );

    FileStream stream;

    // File opened?
    if ( !stream.open( mFilePathBuffer, Torque::FS::File::Read ) )
    {
        // No, so warn.
        Con::warnf("Taml::read() - Could not open filename '%s' for read.", mFilePathBuffer );
        return NULL;
    }

    // Get the file auto-format mode.
    const TamlFormatMode formatMode = getFileAutoFormatMode( mFilePathBuffer );

    // Reset the compilation.
    resetCompilation();

    // Write object.
    SimObject* pSimObject = read( stream, formatMode );

    // Close file.
    stream.close();

    // Reset the compilation.
    resetCompilation();

    // Did we generate an object?
    if ( pSimObject == NULL )
    {
        // No, so warn.
        Con::warnf( "Taml::read() - Failed to load an object from the file '%s'.", mFilePathBuffer );
    }

    return pSimObject;
}

//-----------------------------------------------------------------------------

bool Taml::write( FileStream& stream, SimObject* pSimObject, const TamlFormatMode formatMode )
{
    // Sanity!
    AssertFatal( pSimObject != NULL, "Cannot write a NULL object." );

    // Compile nodes.
    TamlWriteNode* pRootNode = compileObject( pSimObject );

    // Format appropriately.
    switch( formatMode )
    {
        /// Xml.
        case XmlFormat:
        {
            // Create writer.
            TamlXmlWriter writer( this );
            // Write.
            return writer.write( stream, pRootNode );
        }

        /// Binary.
        case BinaryFormat:
        {
            // Create writer.
            TamlBinaryWriter writer( this );

            // Write.
            return writer.write( stream, pRootNode, mBinaryCompression );
        }

        /// JSON.
        case JSONFormat:
        {
            // Create writer.
            //TamlJSONWriter writer( this );

            // Write.
            //return writer.write( stream, pRootNode );
           return NULL;
        }

        /// Invalid.
        case InvalidFormat:
        {
            // Warn.
            Con::warnf("Taml::write() - Cannot write, invalid format.");
            return false;
        }
    }

    // Warn.
    Con::warnf("Taml::write() - Unknown format.");
    return false;
}

//-----------------------------------------------------------------------------

SimObject* Taml::read( FileStream& stream, const TamlFormatMode formatMode )
{
    // Format appropriately.
    switch( formatMode )
    {
        /// Xml.
        case XmlFormat:
        {
            // Create reader.
            TamlXmlReader reader( this );

            // Read.
            return reader.read( stream );
        }

        /// Binary.
        case BinaryFormat:
        {
            // Create reader.
            TamlBinaryReader reader( this );

            // Read.
            return reader.read( stream );
        }

        /// JSON.
        case JSONFormat:
        {
            // Create reader.
            //TamlJSONReader reader( this );

            // Read.
            //return reader.read( stream );
           return NULL;
        }
        
        /// Invalid.
        case InvalidFormat:
        {
            // Warn.
            Con::warnf("Taml::read() - Cannot read, invalid format.");
            return NULL;
        }
    }

    // Warn.
    Con::warnf("Taml::read() - Unknown format.");
    return NULL;
}

//-----------------------------------------------------------------------------

bool Taml::parse( const char* pFilename, TamlVisitor& visitor )
{
    // Debug Profiling.
    PROFILE_SCOPE(Taml_Parse);

    // Sanity!
    AssertFatal( pFilename != NULL, "Taml::parse() - Cannot parse a NULL filename." );

    // Fetch format mode.
    const TamlFormatMode formatMode = getFileAutoFormatMode( pFilename );

    // Handle format mode appropriately.
    switch( formatMode )
    {
        case XmlFormat:
        {
            // Parse with the visitor.
            TamlXmlParser parser;

            // Are property changes needed but not supported?
            if ( visitor.wantsPropertyChanges() && !parser.canChangeProperty() )
            {
                // Yes, so warn.
                Con::warnf( "Taml::parse() - Cannot parse '%s' file-type for filename '%s' as a specified visitor requires property changes which are not supported by the parser.", getFormatModeDescription(formatMode), pFilename );
                return false;
            }

            return parser.accept( pFilename, visitor );            
        }

        case JSONFormat:
        {
            // Parse with the visitor.
            /*TamlJSONParser parser;

            // Are property changes needed but not supported?
            if ( visitor.wantsPropertyChanges() && !parser.canChangeProperty() )
            {
                // Yes, so warn.
                Con::warnf( "Taml::parse() - Cannot parse '%s' file-type for filename '%s' as a specified visitor requires property changes which are not supported by the parser.", getFormatModeDescription(formatMode), pFilename );
                return false;
            }

            return parser.accept( pFilename, visitor );       */
           return false;
        }

        case BinaryFormat:
        default:
            break;
    }

    // Warn.
    Con::warnf( "Taml::parse() - Cannot parse '%s' file-type for filename '%s' as a required parser is not available.", getFormatModeDescription(formatMode), pFilename );
    return false;
}

//-----------------------------------------------------------------------------

void Taml::resetCompilation( void )
{
    // Debug Profiling.
    PROFILE_SCOPE(Taml_ResetCompilation);

    // Clear compiled nodes.
    for( typeNodeVector::iterator itr = mCompiledNodes.begin(); itr != mCompiledNodes.end(); ++itr )
    {
        // Fetch node.
        TamlWriteNode* pNode = (*itr);

        // Reset node.
        pNode->resetNode();

        // Delete node.
        delete pNode;
    }
    mCompiledNodes.clear();

    // Clear compiled objects.
    mCompiledObjects.clear();

    // Reset master node Id.
    mMasterNodeId = 0;
}

//-----------------------------------------------------------------------------

Taml::TamlFormatMode Taml::getFileAutoFormatMode( const char* pFilename )
{
    // Sanity!
    AssertFatal( pFilename != NULL, "Taml::getFileAutoFormatMode() - Cannot auto-format using a NULL filename." );

    // Is auto-format active?
    if ( mAutoFormat )
    {
        // Yes, so fetch the extension lengths.
        const U32 xmlExtensionLength = dStrlen( mAutoFormatXmlExtension );
        const U32 binaryExtensionLength = dStrlen( mAutoFormatBinaryExtension );
        const U32 jsonExtensionLength = dStrlen( mAutoFormatJSONExtension );

        // Fetch filename length.
        const U32 filenameLength = dStrlen( pFilename );

        // Fetch end of filename,
        const char* pEndOfFilename = pFilename + filenameLength;

        // Check for the XML format.
        if ( xmlExtensionLength <= filenameLength && dStricmp( pEndOfFilename - xmlExtensionLength, mAutoFormatXmlExtension ) == 0 )
            return Taml::XmlFormat;

        // Check for the Binary format.
        if ( binaryExtensionLength <= filenameLength && dStricmp( pEndOfFilename - xmlExtensionLength, mAutoFormatBinaryExtension ) == 0 )
            return Taml::BinaryFormat;  

        // Check for the XML format.
        if ( jsonExtensionLength <= filenameLength && dStricmp( pEndOfFilename - jsonExtensionLength, mAutoFormatJSONExtension ) == 0 )
            return Taml::JSONFormat;
    }

    // Use the explicitly specified format mode.
    return mFormatMode;
}

//-----------------------------------------------------------------------------

TamlWriteNode* Taml::compileObject( SimObject* pSimObject, const bool forceId )
{
    // Debug Profiling.
    PROFILE_SCOPE(Taml_CompileObject);

    // Sanity!
    AssertFatal( pSimObject != NULL, "Taml::compileObject() - Cannot compile a NULL object." );

    // Fetch object Id.
    const SimObjectId objectId = pSimObject->getId();

    // Find a previously compiled node.
    typeCompiledHash::Iterator compiledItr = mCompiledObjects.find( objectId );

    // Have we already compiled this?
    if ( compiledItr != mCompiledObjects.end() )
    {
        // Yes, so sanity!
        AssertFatal( mCompiledNodes.size() != 0, "Taml::compileObject() - Found a compiled node at the root." );

        // Yes, so fetch node.
        TamlWriteNode* compiledNode = compiledItr->value;

        // Is a reference Id already present?
        if ( compiledNode->mRefId == 0 )
        {
            // No, so allocate one.
            compiledNode->mRefId = ++mMasterNodeId;
        }

        // Create write node.
        TamlWriteNode* pNewNode = new TamlWriteNode();
        pNewNode->set( pSimObject );

        // Set reference node.
        pNewNode->mRefToNode = compiledNode;

        // Push new node.
        mCompiledNodes.push_back( pNewNode );

        return pNewNode;
    }

    // No, so create write node.
    TamlWriteNode* pNewNode = new TamlWriteNode();
    pNewNode->set( pSimObject );

    // Is an Id being forced for this object?
    if ( forceId )
    {
        // Yes, so allocate one.
        pNewNode->mRefId = ++mMasterNodeId;
    }

    // Push new node.
    mCompiledNodes.push_back( pNewNode );

    // Insert compiled object.
    mCompiledObjects.insertUnique( objectId, pNewNode );

    // Are there any Taml callbacks?
    if ( pNewNode->mpTamlCallbacks != NULL )
    {
        // Yes, so call it.
        tamlPreWrite( pNewNode->mpTamlCallbacks );
    }

    // Compile static and dynamic fields.
    compileStaticFields( pNewNode );
    compileDynamicFields( pNewNode );

    // Compile children.
    compileChildren( pNewNode );

    // Compile custom state.
    compileCustomState( pNewNode );

    // Are there any Taml callbacks?
    if ( pNewNode->mpTamlCallbacks != NULL )
    {
        // Yes, so call it.
        tamlPostWrite( pNewNode->mpTamlCallbacks );
    }

    return pNewNode;
}

//-----------------------------------------------------------------------------

void Taml::compileStaticFields( TamlWriteNode* pTamlWriteNode )
{
    // Debug Profiling.
    PROFILE_SCOPE(Taml_CompileStaticFields);

    // Sanity!
    AssertFatal( pTamlWriteNode != NULL, "Cannot compile static fields on a NULL node." );
    AssertFatal( pTamlWriteNode->mpSimObject != NULL, "Cannot compile static fields on a node with no object." );

    // Fetch object.
    SimObject* pSimObject = pTamlWriteNode->mpSimObject;

    // Fetch field list.
    const AbstractClassRep::FieldList& fieldList = pSimObject->getFieldList();

    // Fetch field count.
    const U32 fieldCount = fieldList.size();

    // Iterate fields.
    U8 arrayDepth = 0;
    TamlCustomNode* currentArrayNode;
    for( U32 index = 0; index < fieldCount; ++index )
    {
        // Fetch field.
        const AbstractClassRep::Field* pField = &fieldList[index];

        // Ignore if field not appropriate.
        if( pField->type == AbstractClassRep::DeprecatedFieldType ||
            pField->type == AbstractClassRep::StartGroupFieldType ||
            pField->type == AbstractClassRep::EndGroupFieldType )
            continue;

        if( pField->type == AbstractClassRep::StartArrayFieldType )
        {
           TamlCustomNodes& pCustomNodes = pTamlWriteNode->mCustomNodes;
           currentArrayNode = pCustomNodes.addNode(pField->pGroupname);
           for(U16 idx = 0; idx < pField->elementCount; idx++)
              currentArrayNode->addNode(pField->pFieldname);
           arrayDepth++;
           continue;
        }

        if( pField->type == AbstractClassRep::EndArrayFieldType )
        {
           arrayDepth--;
           continue;
        }

        if(arrayDepth == 0 && pField->elementCount > 1)
        {
           TamlCustomNodes& pCustomNodes = pTamlWriteNode->mCustomNodes;
           char* niceFieldName = const_cast<char *>(pField->pFieldname);
           niceFieldName[0] = dToupper(niceFieldName[0]);
           String str_niceFieldName = String(niceFieldName);
           currentArrayNode = pCustomNodes.addNode(str_niceFieldName + "s");
           for(U16 idx = 0; idx < pField->elementCount; idx++)
              currentArrayNode->addNode(str_niceFieldName);
        }

        // Fetch fieldname.
        StringTableEntry fieldName = StringTable->insert( pField->pFieldname );

        // Fetch element count.
        const U32 elementCount = pField->elementCount;

        // Skip if the field should not be written.
        // For now, we only deal with non-array fields.
        if ( elementCount == 1 &&
           pField->setDataFn != NULL &&
            ( !getWriteDefaults() && pField->writeDataFn( pSimObject, fieldName ) == false) )
            continue;

        // Iterate elements.
        for( U32 elementIndex = 0; elementIndex < elementCount; ++elementIndex )
        {
            char indexBuffer[8];
            dSprintf( indexBuffer, 8, "%d", elementIndex );

            // Fetch object field value.
            const char* pFieldValue = pSimObject->getPrefixedDataField(fieldName, indexBuffer);

            if(!pFieldValue)
               pFieldValue = StringTable->EmptyString();

            if(pField->type == TypeBool)
               pFieldValue = dAtob(pFieldValue) ? "true" : "false";

            U32 nBufferSize = dStrlen( pFieldValue ) + 1;
            FrameTemp<char> valueCopy( nBufferSize );
            dStrcpy( (char *)valueCopy, pFieldValue );

            // Skip if field should not be written.
            if (!pSimObject->writeField(fieldName, valueCopy))
                continue;

            // Reassign field value.
            pFieldValue = valueCopy;

            // Detect and collapse relative path information
            char fnBuf[1024];
            if ((S32)pField->type == TypeFilename)
            {
               Con::collapseScriptFilename( fnBuf, 1024, pFieldValue );
                pFieldValue = fnBuf;
            }

            // Save field/value.
            if(arrayDepth > 0 || pField->elementCount > 1)
               currentArrayNode->getChildren()[elementIndex]->addField(fieldName, pFieldValue);
            else
            {
               TamlWriteNode::FieldValuePair* pFieldValuePair = new TamlWriteNode::FieldValuePair( fieldName, pFieldValue );
               pTamlWriteNode->mFields.push_back( pFieldValuePair );
            }
        }
    }    
}

//-----------------------------------------------------------------------------

static S32 QSORT_CALLBACK compareFieldEntries(const void* a,const void* b)
{
    // Debug Profiling.
    PROFILE_SCOPE(Taml_CompareFieldEntries);

    SimFieldDictionary::Entry *fa = *((SimFieldDictionary::Entry **)a);
    SimFieldDictionary::Entry *fb = *((SimFieldDictionary::Entry **)b);
    return dStricmp(fa->slotName, fb->slotName);
}

//-----------------------------------------------------------------------------

void Taml::compileDynamicFields( TamlWriteNode* pTamlWriteNode )
{
    // Debug Profiling.
    PROFILE_SCOPE(Taml_CompileDynamicFields);

     // Sanity!
    AssertFatal( pTamlWriteNode != NULL, "Cannot compile dynamic fields on a NULL node." );
    AssertFatal( pTamlWriteNode->mpSimObject != NULL, "Cannot compile dynamic fields on a node with no object." );

    // Fetch object.
    SimObject* pSimObject = pTamlWriteNode->mpSimObject;

    // Fetch field dictionary.
    SimFieldDictionary* pFieldDictionary = pSimObject->getFieldDictionary();

    // Ignore if not writing dynamic fields.
    if ( !pFieldDictionary || !pSimObject->getCanSaveDynamicFields() )
        return;

    // Fetch field list.
    const AbstractClassRep::FieldList& fieldList = pSimObject->getFieldList();

    // Fetch field count.
    const U32 fieldCount = fieldList.size();

    Vector<SimFieldDictionary::Entry*> dynamicFieldList(__FILE__, __LINE__);

    // Ensure the dynamic field doesn't conflict with static field.
    for( U32 hashIndex = 0; hashIndex < SimFieldDictionary::HashTableSize; ++hashIndex )
    {
        for( SimFieldDictionary::Entry* pEntry = pFieldDictionary->mHashTable[hashIndex]; pEntry; pEntry = pEntry->next )
        {
            // Iterate static fields.
            U32 fieldIndex;
            for( fieldIndex = 0; fieldIndex < fieldCount; ++fieldIndex )
            {
                if( fieldList[fieldIndex].pFieldname == pEntry->slotName)
                    break;
            }

            // Skip if found.
            if( fieldIndex != (U32)fieldList.size() )
                continue;

            // Skip if not writing field.
            if ( !pSimObject->writeField( pEntry->slotName, pEntry->value) )
                continue;

            dynamicFieldList.push_back( pEntry );
        }
    }

    // Sort Entries to prevent version control conflicts
    if ( dynamicFieldList.size() > 1 )
        dQsort(dynamicFieldList.address(), dynamicFieldList.size(), sizeof(SimFieldDictionary::Entry*), compareFieldEntries);

    // Save the fields.
    for( Vector<SimFieldDictionary::Entry*>::iterator entryItr = dynamicFieldList.begin(); entryItr != dynamicFieldList.end(); ++entryItr )
    {
        // Fetch entry.
        SimFieldDictionary::Entry* pEntry = *entryItr;

        // Save field/value.
        TamlWriteNode::FieldValuePair*  pFieldValuePair = new TamlWriteNode::FieldValuePair( pEntry->slotName, pEntry->value );
        pTamlWriteNode->mFields.push_back( pFieldValuePair );
    }
}

//-----------------------------------------------------------------------------

void Taml::compileChildren( TamlWriteNode* pTamlWriteNode )
{
    // Debug Profiling.
    PROFILE_SCOPE(Taml_CompileChildren);

    // Sanity!
    AssertFatal( pTamlWriteNode != NULL, "Cannot compile children on a NULL node." );
    AssertFatal( pTamlWriteNode->mpSimObject != NULL, "Cannot compile children on a node with no object." );

    // Fetch object.
    SimObject* pSimObject = pTamlWriteNode->mpSimObject;

    // Fetch the Taml children.
    TamlChildren* pChildren = dynamic_cast<TamlChildren*>( pSimObject );

    // Finish if object does not contain Taml children.
    if ( pChildren == NULL || pChildren->getTamlChildCount() == 0 )
        return;

    // Create children vector.
    pTamlWriteNode->mChildren = new typeNodeVector();

    // Fetch the child count.
    const U32 childCount = pChildren->getTamlChildCount();

    // Iterate children.
    for ( U32 childIndex = 0; childIndex < childCount; childIndex++ )
    {
        // Compile object.
        TamlWriteNode* pChildTamlWriteNode = compileObject( pChildren->getTamlChild(childIndex) );

        // Save node.
        pTamlWriteNode->mChildren->push_back( pChildTamlWriteNode );
    }
}

//-----------------------------------------------------------------------------

void Taml::compileCustomState( TamlWriteNode* pTamlWriteNode )
{
    // Debug Profiling.
    PROFILE_SCOPE(Taml_CompileCustomProperties);

    // Sanity!
    AssertFatal( pTamlWriteNode != NULL, "Cannot compile custom state on a NULL node." );
    AssertFatal( pTamlWriteNode->mpSimObject != NULL, "Cannot compile custom state on a node with no object." );

    // Fetch the custom node on the write node.
    TamlCustomNodes& customNodes = pTamlWriteNode->mCustomNodes;

    // Are there any Taml callbacks?
    if ( pTamlWriteNode->mpTamlCallbacks != NULL )
    {
        // Yes, so call it.
        tamlCustomWrite( pTamlWriteNode->mpTamlCallbacks, customNodes );
    }

    // Fetch custom nodes.
    const TamlCustomNodeVector& nodes = customNodes.getNodes();

    // Finish if no custom nodes to process.
    if ( nodes.size() == 0 )
        return;
    
    // Iterate custom properties.
    for( TamlCustomNodeVector::const_iterator customNodesItr = nodes.begin(); customNodesItr != nodes.end(); ++customNodesItr )
    {
        // Fetch the custom node.
        TamlCustomNode* pCustomNode = *customNodesItr;

        // Compile custom node state.
        compileCustomNodeState( pCustomNode );
    }
}

//-----------------------------------------------------------------------------

void Taml::compileCustomNodeState( TamlCustomNode* pCustomNode )
{
    // Sanity!
    AssertFatal( pCustomNode != NULL, "Taml: Cannot compile NULL custom node state." );

    // Fetch children.
    const TamlCustomNodeVector& children = pCustomNode->getChildren();

    // Fetch proxy object.
    SimObject* pProxyObject = pCustomNode->getProxyObject<SimObject>(false);

    // Do we have a proxy object?
    if ( pProxyObject != NULL )
    {
        // Yes, so sanity!
        AssertFatal( children.size() == 0, "Taml: Cannot compile a proxy object on a custom node that has children." );

        // Yes, so compile it.
        // NOTE: We force an Id for custom compiled objects so we guarantee an Id.  The reason for this is fairly
        // weak in that the XML parser currently has no way of distinguishing between a compiled object node
        // and a custom node.  If the node has an Id or an Id-Ref then it's obviously an object and should be parsed as such.
        pCustomNode->setWriteNode( compileObject( pProxyObject, true ) );
        return;
    }

    // Finish if no children.
    if ( children.size() == 0 )
        return;

    // Iterate children.
    for( TamlCustomNodeVector::const_iterator childItr = children.begin(); childItr != children.end(); ++childItr )
    {
        // Fetch shape node.
        TamlCustomNode* pChildNode = *childItr;

        // Compile the child.
        compileCustomNodeState( pChildNode );
    }
}

//-----------------------------------------------------------------------------

SimObject* Taml::createType( StringTableEntry typeName, const Taml* pTaml, const char* pProgenitorSuffix )
{
    // Debug Profiling.
    PROFILE_SCOPE(Taml_CreateType);

    typedef HashTable<StringTableEntry, AbstractClassRep*> typeClassHash;
    static typeClassHash mClassMap;

    // Sanity!
    AssertFatal( typeName != NULL, "Taml: Type cannot be NULL" );

    // Find type.
    typeClassHash::Iterator typeItr = mClassMap.find( typeName );

    // Found type?
    if ( typeItr == mClassMap.end() )
    {
        // No, so find type.
        AbstractClassRep* pClassRep = AbstractClassRep::getClassList();
        while( pClassRep )
        {
            // Is this the type?
            if( dStricmp( pClassRep->getClassName(), typeName ) == 0 )
            {
                // Yes, so insert it.
                typeItr = mClassMap.insertUnique( typeName, pClassRep );
                break;
            }

            // Next type.
            pClassRep = pClassRep->getNextClass();
        }

        // Did we find the type?
        if ( typeItr == mClassMap.end() )
        {
            // No, so warn and fail.
            Con::warnf( "Taml: Failed to create type '%s' as such a registered type could not be found.", typeName );
            return NULL;
        }
    }

    // Create the object.
    ConsoleObject* pConsoleObject = typeItr->value->create();

    // NOTE: It is important that we don't register the object here as many objects rely on the fact that
    // fields are set prior to the object being registered.  Registering here will invalid those assumptions.

    // Fetch the SimObject.
    SimObject* pSimObject = dynamic_cast<SimObject*>( pConsoleObject );

    // Was it a SimObject?
    if ( pSimObject == NULL )
    {
        // No, so warn.
        Con::warnf( "Taml: Failed to create type '%s' as it is not a SimObject.", typeName );

        // Destroy object and fail.
        delete pConsoleObject;
        return NULL;
    }

    // Are we updating the file-progenitor?
    if ( pTaml->getProgenitorUpdate() )
    {
        // Yes, so do we have a progenitor suffix?
        if ( pProgenitorSuffix == NULL )
        {
            // No, so just set it to the progenitor file.
            pSimObject->setProgenitorFile( pTaml->getFilePathBuffer() );
        }
        else
        {
            // Yes, so format the progenitor buffer.
            char progenitorBuffer[2048];
            dSprintf( progenitorBuffer, sizeof(progenitorBuffer), "%s,%s", pTaml->getFilePathBuffer(), pProgenitorSuffix );

            // Set the progenitor file.
            pSimObject->setProgenitorFile( progenitorBuffer );
        }
    }

    return pSimObject;
}

//-----------------------------------------------------------------------------

bool Taml::generateTamlSchema()
{
    // Fetch any TAML Schema file reference.
    const char* pTamlSchemaFile = Con::getVariable( TAML_SCHEMA_VARIABLE );

    // Do we have a schema file reference?
    if ( pTamlSchemaFile == NULL || *pTamlSchemaFile == 0 )
    {
        // No, so warn.
        Con::warnf( "Taml::generateTamlSchema() - Cannot write a TAML schema as no schema variable is set ('%s').", TAML_SCHEMA_VARIABLE );
        return false;
    }

    // Expand the file-name into the file-path buffer.
    char filePathBuffer[1024];
    Con::expandToolScriptFilename( filePathBuffer, sizeof(filePathBuffer), pTamlSchemaFile );

    FileStream stream;

    // File opened?
    /*if ( !stream.open( filePathBuffer, Torque::FS::File::Write ) )
    {
        // No, so warn.
        Con::warnf("Taml::GenerateTamlSchema() - Could not open filename '%s' for write.", filePathBuffer );
        return false;
    }*/

    // Create document.
    TiXmlDocument schemaDocument;

    // Add declaration.
    TiXmlDeclaration schemaDeclaration( "1.0", "iso-8859-1", "no" );
    schemaDocument.InsertEndChild( schemaDeclaration );

    // Add schema element.
    TiXmlElement* pSchemaElement = new TiXmlElement( "xs:schema" );
    pSchemaElement->SetAttribute( "xmlns:xs", "http://www.w3.org/2001/XMLSchema" );
    schemaDocument.LinkEndChild( pSchemaElement );

    // Fetch class-rep root.
    AbstractClassRep* pRootType = AbstractClassRep::getClassList();

    // Fetch SimObject class rep.
    AbstractClassRep* pSimObjectType = AbstractClassRep::findClassRep( "SimObject" );
    // Sanity!
    AssertFatal( pSimObjectType != NULL, "Taml::GenerateTamlSchema() - Could not find SimObject class rep." );

    // Reset scratch state.
    char buffer[1024];
    HashTable<AbstractClassRep*, StringTableEntry> childGroups;

    // *************************************************************
    // Generate console type elements.
    // *************************************************************

    // Vector2.
    TiXmlComment* pVector2Comment = new TiXmlComment( "Vector2 Console Type" );
    pSchemaElement->LinkEndChild( pVector2Comment );
    TiXmlElement* pVector2TypeElement = new TiXmlElement( "xs:simpleType" );
    pVector2TypeElement->SetAttribute( "name", "Vector2_ConsoleType" );
    pSchemaElement->LinkEndChild( pVector2TypeElement );
    TiXmlElement* pVector2ElementA = new TiXmlElement( "xs:restriction" );
    pVector2ElementA->SetAttribute( "base", "xs:string" );
    pVector2TypeElement->LinkEndChild( pVector2ElementA );
    TiXmlElement* pVector2ElementB = new TiXmlElement( "xs:pattern" );
    pVector2ElementB->SetAttribute( "value", "([-]?(\\b[0-9]+)?\\.)?[0-9]+\\b ([-]?(\\b[0-9]+)?\\.)?[0-9]+\\b" );   
    pVector2ElementA->LinkEndChild( pVector2ElementB );

    // Point2F.
    TiXmlComment* pPoint2FComment = new TiXmlComment( "Point2F Console Type" );
    pSchemaElement->LinkEndChild( pPoint2FComment );
    TiXmlElement* pPoint2FTypeElement = new TiXmlElement( "xs:simpleType" );
    pPoint2FTypeElement->SetAttribute( "name", "Point2F_ConsoleType" );
    pSchemaElement->LinkEndChild( pPoint2FTypeElement );
    TiXmlElement* pPoint2FElementA = new TiXmlElement( "xs:restriction" );
    pPoint2FElementA->SetAttribute( "base", "xs:string" );
    pPoint2FTypeElement->LinkEndChild( pPoint2FElementA );
    TiXmlElement* pPoint2FElementB = new TiXmlElement( "xs:pattern" );
    pPoint2FElementB->SetAttribute( "value", "([-]?(\\b[0-9]+)?\\.)?[0-9]+\\b ([-]?(\\b[0-9]+)?\\.)?[0-9]+\\b" );   
    pPoint2FElementA->LinkEndChild( pPoint2FElementB );

    // Point2I.
    TiXmlComment* pPoint2IComment = new TiXmlComment( "Point2I Console Type" );
    pSchemaElement->LinkEndChild( pPoint2IComment );
    TiXmlElement* pPoint2ITypeElement = new TiXmlElement( "xs:simpleType" );
    pPoint2ITypeElement->SetAttribute( "name", "Point2I_ConsoleType" );
    pSchemaElement->LinkEndChild( pPoint2ITypeElement );
    TiXmlElement* pPoint2IElementA = new TiXmlElement( "xs:restriction" );
    pPoint2IElementA->SetAttribute( "base", "xs:string" );
    pPoint2ITypeElement->LinkEndChild( pPoint2IElementA );
    TiXmlElement* pPoint2IElementB = new TiXmlElement( "xs:pattern" );
    pPoint2IElementB->SetAttribute( "value", "[-]?[0-9]* [-]?[0-9]*" );   
    pPoint2IElementA->LinkEndChild( pPoint2IElementB );

    // b2AABB.
    TiXmlComment* pb2AABBComment = new TiXmlComment( "b2AABB Console Type" );
    pSchemaElement->LinkEndChild( pb2AABBComment );
    TiXmlElement* pb2AABBTypeElement = new TiXmlElement( "xs:simpleType" );
    pb2AABBTypeElement->SetAttribute( "name", "b2AABB_ConsoleType" );
    pSchemaElement->LinkEndChild( pb2AABBTypeElement );
    TiXmlElement* pb2AABBElementA = new TiXmlElement( "xs:restriction" );
    pb2AABBElementA->SetAttribute( "base", "xs:string" );
    pb2AABBTypeElement->LinkEndChild( pb2AABBElementA );
    TiXmlElement* pb2AABBElementB = new TiXmlElement( "xs:pattern" );
    pb2AABBElementB->SetAttribute( "value", "([-]?(\\b[0-9]+)?\\.)?[0-9]+\\b ([-]?(\\b[0-9]+)?\\.)?[0-9]+\\b ([-]?(\\b[0-9]+)?\\.)?[0-9]+\\b ([-]?(\\b[0-9]+)?\\.)?[0-9]+\\b" );   
    pb2AABBElementA->LinkEndChild( pb2AABBElementB );   

    // RectI.
    TiXmlComment* pRectIComment = new TiXmlComment( "RectI Console Type" );
    pSchemaElement->LinkEndChild( pRectIComment );
    TiXmlElement* pRectITypeElement = new TiXmlElement( "xs:simpleType" );
    pRectITypeElement->SetAttribute( "name", "RectI_ConsoleType" );
    pSchemaElement->LinkEndChild( pRectITypeElement );
    TiXmlElement* pRectIElementA = new TiXmlElement( "xs:restriction" );
    pRectIElementA->SetAttribute( "base", "xs:string" );
    pRectITypeElement->LinkEndChild( pRectIElementA );
    TiXmlElement* pRectIElementB = new TiXmlElement( "xs:pattern" );
    pRectIElementB->SetAttribute( "value", "[-]?[0-9]* [-]?[0-9]* [-]?[0-9]* [-]?[0-9]*" );   
    pRectIElementA->LinkEndChild( pRectIElementB );

    // RectF.
    TiXmlComment* pRectFComment = new TiXmlComment( "RectF Console Type" );
    pSchemaElement->LinkEndChild( pRectFComment );
    TiXmlElement* pRectFTypeElement = new TiXmlElement( "xs:simpleType" );
    pRectFTypeElement->SetAttribute( "name", "RectF_ConsoleType" );
    pSchemaElement->LinkEndChild( pRectFTypeElement );
    TiXmlElement* pRectFElementA = new TiXmlElement( "xs:restriction" );
    pRectFElementA->SetAttribute( "base", "xs:string" );
    pRectFTypeElement->LinkEndChild( pRectFElementA );
    TiXmlElement* pRectFElementB = new TiXmlElement( "xs:pattern" );
    pRectFElementB->SetAttribute( "value", "(\\b[-]?(b[0-9]+)?\\.)?[0-9]+\\b" );   
    pRectFElementA->LinkEndChild( pRectFElementB );

    // AssetId.
    TiXmlComment* pAssetIdComment = new TiXmlComment("AssetId Console Type");
    pSchemaElement->LinkEndChild(pAssetIdComment);
    TiXmlElement* pAssetIdTypeElement = new TiXmlElement("xs:simpleType");
    pAssetIdTypeElement->SetAttribute("name", "AssetId_ConsoleType");
    pSchemaElement->LinkEndChild(pAssetIdTypeElement);
    TiXmlElement* pAssetIdElementA = new TiXmlElement("xs:restriction");
    pAssetIdElementA->SetAttribute("base", "xs:string");
    pAssetIdTypeElement->LinkEndChild(pAssetIdElementA);
    TiXmlElement* pAssetIdElementB = new TiXmlElement("xs:pattern");
    dSprintf(buffer, sizeof(buffer), "(%s)?\\b[a-zA-Z0-9]+\\b%s\\b[a-zA-Z0-9]+\\b", ASSET_ID_FIELD_PREFIX, ASSET_SCOPE_TOKEN);
    pAssetIdElementB->SetAttribute("value", buffer);
    pAssetIdElementA->LinkEndChild(pAssetIdElementB);

    // Color Enums.
    TiXmlComment* pColorEnumsComment = new TiXmlComment( "Color Enums" );
    pSchemaElement->LinkEndChild( pColorEnumsComment );
    TiXmlElement* pColorEnumsTypeElement = new TiXmlElement( "xs:simpleType" );
    pColorEnumsTypeElement->SetAttribute( "name", "Color_Enums" );
    pSchemaElement->LinkEndChild( pColorEnumsTypeElement );
    TiXmlElement* pColorEnumsRestrictionElement = new TiXmlElement( "xs:restriction" );
    pColorEnumsRestrictionElement->SetAttribute( "base", "xs:string" );
    pColorEnumsTypeElement->LinkEndChild( pColorEnumsRestrictionElement );
    const S32 ColorEnumsCount = StockColor::getCount();
    for( S32 index = 0; index < ColorEnumsCount; ++index )
    {
        // Add enumeration element.
        TiXmlElement* pColorEnumsAttributeEnumerationElement = new TiXmlElement( "xs:enumeration" );
        pColorEnumsAttributeEnumerationElement->SetAttribute( "value", StockColor::getColorItem(index)->getColorName() );
        pColorEnumsRestrictionElement->LinkEndChild( pColorEnumsAttributeEnumerationElement );
    }

    // ColorF.
    TiXmlComment* pColorFValuesComment = new TiXmlComment( "ColorF Values" );
    pSchemaElement->LinkEndChild( pColorFValuesComment );
    TiXmlElement* pColorFValuesTypeElement = new TiXmlElement( "xs:simpleType" );
    pColorFValuesTypeElement->SetAttribute( "name", "ColorF_Values" );
    pSchemaElement->LinkEndChild( pColorFValuesTypeElement );
    TiXmlElement* pColorFValuesElementA = new TiXmlElement( "xs:restriction" );
    pColorFValuesElementA->SetAttribute( "base", "xs:string" );
    pColorFValuesTypeElement->LinkEndChild( pColorFValuesElementA );
    TiXmlElement* pColorFValuesElementB = new TiXmlElement( "xs:pattern" );
    pColorFValuesElementB->SetAttribute( "value", "([-]?(\\b[0-9]+)?\\.)?[0-9]+\\b ([-]?(\\b[0-9]+)?\\.)?[0-9]+\\b ([-]?(\\b[0-9]+)?\\.)?[0-9]+\\b ([-]?(\\b[0-9]+)?\\.)?[0-9]+\\b" );
    pColorFValuesElementA->LinkEndChild( pColorFValuesElementB );

    TiXmlComment* pColorFComment = new TiXmlComment( "ColorF Console Type" );
    pSchemaElement->LinkEndChild( pColorFComment );
    TiXmlElement* pColorFTypeElement = new TiXmlElement( "xs:simpleType" );
    pColorFTypeElement->SetAttribute( "name", "ColorF_ConsoleType" );
    pSchemaElement->LinkEndChild( pColorFTypeElement );
    TiXmlElement* pColorFUnionElement = new TiXmlElement( "xs:union" );
    pColorFUnionElement->SetAttribute( "memberTypes", "ColorF_Values Color_Enums" );
    pColorFTypeElement->LinkEndChild( pColorFUnionElement );

    // ColorI.
    TiXmlComment* pColorIValuesComment = new TiXmlComment( "ColorI Values" );
    pSchemaElement->LinkEndChild( pColorIValuesComment );
    TiXmlElement* pColorIValuesTypeElement = new TiXmlElement( "xs:simpleType" );
    pColorIValuesTypeElement->SetAttribute( "name", "ColorI_Values" );
    pSchemaElement->LinkEndChild( pColorIValuesTypeElement );
    TiXmlElement* pColorIValuesElementA = new TiXmlElement( "xs:restriction" );
    pColorIValuesElementA->SetAttribute( "base", "xs:string" );
    pColorIValuesTypeElement->LinkEndChild( pColorIValuesElementA );
    TiXmlElement* pColorIValuesElementB = new TiXmlElement( "xs:pattern" );
    pColorIValuesElementB->SetAttribute( "value", "[-]?[0-9]* [-]?[0-9]* [-]?[0-9]* [-]?[0-9]*" );
    pColorIValuesElementA->LinkEndChild( pColorIValuesElementB );

    TiXmlComment* pColorIComment = new TiXmlComment( "ColorI Console Type" );
    pSchemaElement->LinkEndChild( pColorIComment );
    TiXmlElement* pColorITypeElement = new TiXmlElement( "xs:simpleType" );
    pColorITypeElement->SetAttribute( "name", "ColorI_ConsoleType" );
    pSchemaElement->LinkEndChild( pColorITypeElement );
    TiXmlElement* pColorIUnionElement = new TiXmlElement( "xs:union" );
    pColorIUnionElement->SetAttribute( "memberTypes", "ColorI_Values Color_Enums" );
    pColorITypeElement->LinkEndChild( pColorIUnionElement );

    // *************************************************************
    // Generate engine type elements.
    // *************************************************************

    // Generate the engine type elements.
    TiXmlComment* pComment = new TiXmlComment( "Type Elements" );
    pSchemaElement->LinkEndChild( pComment );
    for ( AbstractClassRep* pType = pRootType; pType != NULL; pType = pType->getNextClass() )
    {
        // Add type.
        TiXmlElement* pTypeElement = new TiXmlElement( "xs:element" );
        pTypeElement->SetAttribute( "name", pType->getClassName() );
        dSprintf( buffer, sizeof(buffer), "%s_Type", pType->getClassName() );
        pTypeElement->SetAttribute( "type", buffer );
        pSchemaElement->LinkEndChild( pTypeElement );
    }

    // *************************************************************
    // Generate the engine complex types.
    // *************************************************************
    for ( AbstractClassRep* pType = pRootType; pType != NULL; pType = pType->getNextClass() )
    {
        // Add complex type comment.
        dSprintf( buffer, sizeof(buffer), " %s Type ", pType->getClassName() );
        TiXmlComment* pComment = new TiXmlComment( buffer );
        pSchemaElement->LinkEndChild( pComment );

        // Add complex type.
        TiXmlElement* pComplexTypeElement = new TiXmlElement( "xs:complexType" );
        dSprintf( buffer, sizeof(buffer), "%s_Type", pType->getClassName() );
        pComplexTypeElement->SetAttribute( "name", buffer );
        pSchemaElement->LinkEndChild( pComplexTypeElement );

        // Add sequence.
        TiXmlElement* pSequenceElement = new TiXmlElement( "xs:sequence" );
        pComplexTypeElement->LinkEndChild( pSequenceElement );

        // Fetch container child class.
        AbstractClassRep* pContainerChildClass = pType->getContainerChildClass( true );

        // Is the type allowed children?
        if ( pContainerChildClass != NULL )
        {
            // Yes, so add choice element.
            TiXmlElement* pChoiceElement = new TiXmlElement( "xs:choice" );
            pChoiceElement->SetAttribute( "minOccurs", 0 );
            pChoiceElement->SetAttribute( "maxOccurs", "unbounded" );
            pSequenceElement->LinkEndChild( pChoiceElement );

            // Find child group.
            HashTable<AbstractClassRep*, StringTableEntry>::Iterator childGroupItr = childGroups.find( pContainerChildClass );

            // Does the group exist?
            if ( childGroupItr == childGroups.end() )
            {
                // No, so format group name.
                dSprintf( buffer, sizeof(buffer), "%s_ChildrenTypes", pContainerChildClass->getClassName() );

                // Insert into child group hash.
                childGroupItr = childGroups.insertUnique( pContainerChildClass, StringTable->insert( buffer ) );

                // Add the group.
                TiXmlElement* pChildrenGroupElement = new TiXmlElement( "xs:group" );
                pChildrenGroupElement->SetAttribute( "name", buffer );
                pSchemaElement->LinkEndChild( pChildrenGroupElement );

                // Add choice element.
                TiXmlElement* pChildreGroupChoiceElement = new TiXmlElement( "xs:choice" );
                pChildrenGroupElement->LinkEndChild( pChildreGroupChoiceElement );

                // Add choice members.
                for ( AbstractClassRep* pChoiceType = pRootType; pChoiceType != NULL; pChoiceType = pChoiceType->getNextClass() )
                {
                    // Skip if not derived from the container child class.
                    if ( !pChoiceType->isClass( pContainerChildClass ) )
                        continue;

                    // Add choice member.
                    TiXmlElement* pChildrenMemberElement = new TiXmlElement( "xs:element" );
                    pChildrenMemberElement->SetAttribute( "name", pChoiceType->getClassName() );
                    dSprintf( buffer, sizeof(buffer), "%s_Type", pChoiceType->getClassName() );
                    pChildrenMemberElement->SetAttribute( "type", buffer );
                    pChildreGroupChoiceElement->LinkEndChild( pChildrenMemberElement );
                }

            }

            // Reference the child group.
            TiXmlElement* pChoiceGroupReferenceElement = new TiXmlElement( "xs:group" );
            pChoiceGroupReferenceElement->SetAttribute( "ref", childGroupItr->value );
            pChoiceGroupReferenceElement->SetAttribute( "minOccurs", 0 );
            pChoiceElement->LinkEndChild( pChoiceGroupReferenceElement );
        }

        // Generate the custom Taml schema.
        for ( AbstractClassRep* pCustomSchemaType = pType; pCustomSchemaType != NULL; pCustomSchemaType = pCustomSchemaType->getParentClass() )
        {
            // Fetch the types custom TAML schema function.
            AbstractClassRep::WriteCustomTamlSchema customSchemaFn = pCustomSchemaType->getCustomTamlSchema();

            // Skip if no function avilable.
            if ( customSchemaFn == NULL )
                continue;

            // Call schema generation function.
            customSchemaFn( pType, pSequenceElement );
        }

        // Generate field attribute group.
        TiXmlElement* pFieldAttributeGroupElement = new TiXmlElement( "xs:attributeGroup" );
        dSprintf( buffer, sizeof(buffer), "%s_Fields", pType->getClassName() );
        pFieldAttributeGroupElement->SetAttribute( "name", buffer );
        pSchemaElement->LinkEndChild( pFieldAttributeGroupElement );

        // Fetch field list.
        const AbstractClassRep::FieldList& fields = pType->mFieldList;

        // Fetcj field count.
        const S32 fieldCount = fields.size();

        // Iterate static fields (in reverse as most types are organized from the root-fields up).
        for( S32 index = fieldCount-1; index > 0; --index )
        {
            // Fetch field.
            const AbstractClassRep::Field& field = fields[index];

            // Skip if not a data field.
            if( field.type == AbstractClassRep::DeprecatedFieldType ||
                field.type == AbstractClassRep::StartGroupFieldType ||
                field.type == AbstractClassRep::EndGroupFieldType )
            continue;

            // Skip if the field root is not this type.
            if ( pType->findFieldRoot( field.pFieldname ) != pType )
                continue;

            // Add attribute element.
            TiXmlElement* pAttributeElement = new TiXmlElement( "xs:attribute" );
            pAttributeElement->SetAttribute( "name", field.pFieldname );

            // Handle the console type appropriately.
            const S32 fieldType = (S32)field.type;

            /*
            // Is the field an enumeration?
            if ( fieldType == TypeEnum )
            {
                // Yes, so add attribute type.
                TiXmlElement* pAttributeSimpleTypeElement = new TiXmlElement( "xs:simpleType" );
                pAttributeElement->LinkEndChild( pAttributeSimpleTypeElement );

                // Add restriction element.
                TiXmlElement* pAttributeRestrictionElement = new TiXmlElement( "xs:restriction" );
                pAttributeRestrictionElement->SetAttribute( "base", "xs:string" );
                pAttributeSimpleTypeElement->LinkEndChild( pAttributeRestrictionElement );

                // Yes, so fetch enumeration count.
                const S32 enumCount = field.table->size;

                // Iterate enumeration.
                for( S32 index = 0; index < enumCount; ++index )
                {
                    // Add enumeration element.
                    TiXmlElement* pAttributeEnumerationElement = new TiXmlElement( "xs:enumeration" );
                    pAttributeEnumerationElement->SetAttribute( "value", field.table->table[index].label );
                    pAttributeRestrictionElement->LinkEndChild( pAttributeEnumerationElement );
                }
            }
            else
            {*/
                // No, so assume it's a string type initially.
                const char* pFieldTypeDescription = "xs:string";

                // Handle known types.
                if( fieldType == TypeF32 )
                {
                    pFieldTypeDescription = "xs:float";
                }
                else if( fieldType == TypeS8 || fieldType == TypeS32 )
                {
                    pFieldTypeDescription = "xs:int";
                }
                else if( fieldType == TypeBool || fieldType == TypeFlag )
                {
                    pFieldTypeDescription = "xs:boolean";
                }
                else if( fieldType == TypePoint2F )
                {
                    pFieldTypeDescription = "Point2F_ConsoleType";
                }
                else if( fieldType == TypePoint2I )
                {
                    pFieldTypeDescription = "Point2I_ConsoleType";
                }
                else if( fieldType == TypeRectI )
                {
                    pFieldTypeDescription = "RectI_ConsoleType";
                }
                else if( fieldType == TypeRectF )
                {
                    pFieldTypeDescription = "RectF_ConsoleType";
                }
                else if( fieldType == TypeColorF )
                {
                    pFieldTypeDescription = "ColorF_ConsoleType";
                }
                else if( fieldType == TypeColorI )
                {
                    pFieldTypeDescription = "ColorI_ConsoleType";
                }
                else if (fieldType == TypeAssetId/* ||
                   fieldType == TypeImageAssetPtr ||
                   fieldType == TypeAnimationAssetPtr ||
                   fieldType == TypeAudioAssetPtr*/)
                {
                   pFieldTypeDescription = "AssetId_ConsoleType";
                }

                // Set attribute type.
                pAttributeElement->SetAttribute( "type", pFieldTypeDescription );
            //}

            pAttributeElement->SetAttribute( "use", "optional" );
            pFieldAttributeGroupElement->LinkEndChild( pAttributeElement );
        }

        // Is this the SimObject Type?
        if ( pType == pSimObjectType )
        {
            // Yes, so add reserved Taml field attributes here...

            // Add Taml "Name" attribute element.
            TiXmlElement* pNameAttributeElement = new TiXmlElement( "xs:attribute" );
            pNameAttributeElement->SetAttribute( "name", tamlNamedObjectName );
            pNameAttributeElement->SetAttribute( "type", "xs:normalizedString" );
            pFieldAttributeGroupElement->LinkEndChild( pNameAttributeElement );

            // Add Taml "TamlId" attribute element.
            TiXmlElement* pTamlIdAttributeElement = new TiXmlElement( "xs:attribute" );
            pTamlIdAttributeElement->SetAttribute( "name", tamlRefIdName );
            pTamlIdAttributeElement->SetAttribute( "type", "xs:nonNegativeInteger" );
            pFieldAttributeGroupElement->LinkEndChild( pTamlIdAttributeElement );

            // Add Taml "TamlRefId" attribute element.
            TiXmlElement* pTamlRefIdAttributeElement = new TiXmlElement( "xs:attribute" );
            pTamlRefIdAttributeElement->SetAttribute( "name", tamlRefToIdName );
            pTamlRefIdAttributeElement->SetAttribute( "type", "xs:nonNegativeInteger" );
            pFieldAttributeGroupElement->LinkEndChild( pTamlRefIdAttributeElement );
        }

        // Add attribute group types.
        for ( AbstractClassRep* pAttributeGroupsType = pType; pAttributeGroupsType != NULL; pAttributeGroupsType = pAttributeGroupsType->getParentClass() )
        {
            TiXmlElement* pFieldAttributeGroupRefElement = new TiXmlElement( "xs:attributeGroup" );
            dSprintf( buffer, sizeof(buffer), "%s_Fields", pAttributeGroupsType->getClassName() );
            pFieldAttributeGroupRefElement->SetAttribute( "ref", buffer );
            pComplexTypeElement->LinkEndChild( pFieldAttributeGroupRefElement );
        }

        // Add "any" attribute element (dynamic fields).
        TiXmlElement* pAnyAttributeElement = new TiXmlElement( "xs:anyAttribute" );
        pAnyAttributeElement->SetAttribute( "processContents", "skip" );
        pComplexTypeElement->LinkEndChild( pAnyAttributeElement );
    }

    // Write the schema document.
    schemaDocument.SaveFile( filePathBuffer );

    // Close file.
    stream.close();

    return true;
}

//-----------------------------------------------------------------------------

void Taml::WriteUnrestrictedCustomTamlSchema( const char* pCustomNodeName, const AbstractClassRep* pClassRep, TiXmlElement* pParentElement )
{
    // Sanity!
    AssertFatal( pCustomNodeName != NULL, "Taml::WriteDefaultCustomTamlSchema() - Node name cannot be NULL." );
    AssertFatal( pClassRep != NULL,  "Taml::WriteDefaultCustomTamlSchema() - ClassRep cannot be NULL." );
    AssertFatal( pParentElement != NULL,  "Taml::WriteDefaultCustomTamlSchema() - Parent Element cannot be NULL." );

    char buffer[1024];

    // Add custom type element.
    TiXmlElement* pCustomElement = new TiXmlElement( "xs:element" );
    dSprintf( buffer, sizeof(buffer), "%s.%s", pClassRep->getClassName(), pCustomNodeName  );
    pCustomElement->SetAttribute( "name", buffer );
    pCustomElement->SetAttribute( "minOccurs", 0 );
    pCustomElement->SetAttribute( "maxOccurs", 1 );
    pParentElement->LinkEndChild( pCustomElement );

    // Add complex type element.
    TiXmlElement* pComplexTypeElement = new TiXmlElement( "xs:complexType" );
    pCustomElement->LinkEndChild( pComplexTypeElement );

    // Add choice element.
    TiXmlElement* pChoiceElement = new TiXmlElement( "xs:choice" );
    pChoiceElement->SetAttribute( "minOccurs", 0 );
    pChoiceElement->SetAttribute( "maxOccurs", "unbounded" );
    pComplexTypeElement->LinkEndChild( pChoiceElement );

    // Add sequence element.
    TiXmlElement* pSequenceElement = new TiXmlElement( "xs:sequence" );
    pChoiceElement->LinkEndChild( pSequenceElement );

    // Add "any" element.
    TiXmlElement* pAnyElement = new TiXmlElement( "xs:any" );
    pAnyElement->SetAttribute( "processContents", "skip" );
    pSequenceElement->LinkEndChild( pAnyElement );
}
