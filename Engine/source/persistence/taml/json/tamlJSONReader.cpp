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

#include "persistence/taml/json/tamlJSONReader.h"
#include "persistence/taml/json/tamlJSONWriter.h"
#include "core/stream/fileStream.h"
#include "core/strings/stringUnit.h"
#include "core/frameAllocator.h"

// Debug Profiling.
#include "platform/profiler.h"

//-----------------------------------------------------------------------------

SimObject* TamlJSONReader::read( FileStream& stream )
{
    // Debug Profiling.
    PROFILE_SCOPE(TamlJSONReader_Read);
   
    // Read JSON file.
    const U32 streamSize = stream.getStreamSize();
    FrameTemp<char> jsonText( streamSize + 1 );
    if ( !stream.read( streamSize, jsonText ) )
    {
        // Warn!
        Con::warnf("TamlJSONReader::read() -  Could not load Taml JSON file from stream.");
        return NULL;
    }
    jsonText[streamSize] = '\0'; 

    // Create JSON document.
    rapidjson::Document document;
    document.Parse<0>( jsonText );

    // Check the document is valid.
    if ( document.GetType() != rapidjson::kObjectType )
    {
        // Warn!
        Con::warnf("TamlJSONReader::read() -  Load Taml JSON file from stream but was invalid.");
        return NULL;
    }
    
    // Parse root value.
    SimObject* pSimObject = parseType( document.MemberBegin() );

    // Reset parse.
    resetParse();

    return pSimObject;
}

//-----------------------------------------------------------------------------

void TamlJSONReader::resetParse( void )
{
    // Debug Profiling.
    PROFILE_SCOPE(TamlJSONReader_ResetParse);

    // Clear object reference map.
    mObjectReferenceMap.clear();
}

//-----------------------------------------------------------------------------

SimObject* TamlJSONReader::parseType( const rapidjson::Value::ConstMemberIterator& memberItr )
{
    // Debug Profiling.
    PROFILE_SCOPE(TamlJSONReader_ParseType);

    // Fetch name and value.
    const rapidjson::Value& typeName = memberItr->name;
    const rapidjson::Value& typeValue = memberItr->value;

    // Is value an object?
    if ( !typeValue.IsObject() )
    {
        // No, so warn.
        Con::warnf( "TamlJSONReader::parseType() -  Cannot process type '%s' as it is not an object.", typeName.GetString() );
        return NULL;
    }

    // Fetch engine type name (demangled).
    StringTableEntry engineTypeName = getDemangledName( typeName.GetString() );

    // Fetch reference to Id.
    const U32 tamlRefToId = getTamlRefToId( typeValue );

    // Do we have a reference to Id?
    if ( tamlRefToId != 0 )
    {
        // Yes, so fetch reference.
        typeObjectReferenceHash::Iterator referenceItr = mObjectReferenceMap.find( tamlRefToId );

        // Did we find the reference?
        if ( referenceItr == mObjectReferenceMap.end() )
        {
            // No, so warn.
            Con::warnf( "TamlJSONReader::parseType() -  Could not find a reference Id of '%d'", tamlRefToId );
            return NULL;
        }

        // Return object.
        return referenceItr->value;
    }

    // No, so fetch reference Id.
    const U32 tamlRefId = getTamlRefId( typeValue );

    // Create type.
    SimObject* pSimObject = Taml::createType( engineTypeName, mpTaml );

    // Finish if we couldn't create the type.
    if ( pSimObject == NULL )
        return NULL;

    // Find Taml callbacks.
    TamlCallbacks* pCallbacks = dynamic_cast<TamlCallbacks*>( pSimObject );

    TamlCustomNodes customNodes;

    // Are there any Taml callbacks?
    if ( pCallbacks != NULL )
    {
        // Yes, so call it.
        mpTaml->tamlPreRead( pCallbacks );
    }

    // Parse field members.
    for( rapidjson::Value::ConstMemberIterator fieldMemberItr = typeValue.MemberBegin(); fieldMemberItr != typeValue.MemberEnd(); ++fieldMemberItr )
    {
        // Fetch value.
        const rapidjson::Value& fieldValue = fieldMemberItr->value;
        
        // Skip if not a field.
        if ( fieldValue.IsObject() )
            continue;

        // Parse as field.
        parseField( fieldMemberItr, pSimObject );
    }

    // Fetch object name.
    StringTableEntry objectName = StringTable->insert( getTamlObjectName( typeValue ) );

    // Does the object require a name?
    if ( objectName == StringTable->EmptyString() )
    {
        // No, so just register anonymously.
        pSimObject->registerObject();
    }
    else
    {
        // Yes, so register a named object.
        pSimObject->registerObject( objectName );

        // Was the name assigned?
        if ( pSimObject->getName() != objectName )
        {
            // No, so warn that the name was rejected.
            Con::warnf( "Taml::parseType() - Registered an instance of type '%s' but a request to name it '%s' was rejected.  This is typically because an object of that name already exists.",
                engineTypeName, objectName );
        }
    }

    // Do we have a reference Id?
    if ( tamlRefId != 0 )
    {
        // Yes, so insert reference.
        mObjectReferenceMap.insertUnique( tamlRefId, pSimObject );
    }

    // Parse children and custom node members.
    for( rapidjson::Value::ConstMemberIterator objectMemberItr = typeValue.MemberBegin(); objectMemberItr != typeValue.MemberEnd(); ++objectMemberItr )
    {
        // Fetch name and value.
        const rapidjson::Value& objectName = objectMemberItr->name;
        const rapidjson::Value& objectValue = objectMemberItr->value;
        
        // Skip if not an object.
        if ( !objectValue.IsObject() )
            continue;

        // Find the period character in the name.
        const char* pPeriod = dStrchr( objectName.GetString(), '.' );

        // Did we find the period?
        if ( pPeriod == NULL )
        {
            // No, so parse child object.
            parseChild( objectMemberItr, pSimObject );
            continue;
        }

        // Yes, so parse custom object.
        parseCustom( objectMemberItr, pSimObject, pPeriod+1, customNodes );
    }

    // Call custom read.
    if ( pCallbacks )
    {
        mpTaml->tamlCustomRead( pCallbacks, customNodes );
        mpTaml->tamlPostRead( pCallbacks, customNodes );
    }

    // Return object.
    return pSimObject;
}

//-----------------------------------------------------------------------------

inline void TamlJSONReader::parseField( rapidjson::Value::ConstMemberIterator& memberItr, SimObject* pSimObject )
{
    // Debug Profiling.
    PROFILE_SCOPE(TamlJSONReader_ParseField);

    // Fetch name and value.
    const rapidjson::Value& name = memberItr->name;
    const rapidjson::Value& value = memberItr->value;

    // Insert the field name.
    StringTableEntry fieldName = StringTable->insert( name.GetString() );

    // Ignore if this is a Taml attribute.
    if (    fieldName == tamlRefIdName ||
            fieldName == tamlRefToIdName ||
            fieldName == tamlNamedObjectName )
            return;

    // Get field value.
    char valueBuffer[4096];
    if ( !parseStringValue( valueBuffer, sizeof(valueBuffer), value, fieldName ) )
    {
        // Warn.
        Con::warnf( "Taml::parseField() Could not interpret value for field '%s'", fieldName );
        return;
    }

    // Set field.
    pSimObject->setPrefixedDataField(fieldName, NULL, valueBuffer);
}

//-----------------------------------------------------------------------------

inline void TamlJSONReader::parseChild( rapidjson::Value::ConstMemberIterator& memberItr, SimObject* pSimObject )
{
    // Debug Profiling.
    PROFILE_SCOPE(TamlJSONReader_ParseChild);

    // Fetch name.
    const rapidjson::Value& name = memberItr->name;

    // Fetch the Taml children.
    TamlChildren* pChildren = dynamic_cast<TamlChildren*>( pSimObject );

    // Is this a Taml child?
    if ( pChildren == NULL )
    {
        // No, so warn.
        Con::warnf("Taml::parseChild() - Child member '%s' found under parent '%s' but object cannot have children.",
            name.GetString(),
            pSimObject->getClassName() );

        return;
    }

    // Fetch any container child class specifier.
    AbstractClassRep* pContainerChildClass = pSimObject->getClassRep()->getContainerChildClass( true );

    // Parse child member.
    SimObject* pChildSimObject = parseType( memberItr );

    // Finish if the child was not created.
    if ( pChildSimObject == NULL )
        return;

    // Do we have a container child class?
    if ( pContainerChildClass != NULL )
    {
        // Yes, so is the child object the correctly derived type?
        if ( !pChildSimObject->getClassRep()->isClass( pContainerChildClass ) )
        {
            // No, so warn.
            Con::warnf("Taml::parseChild() - Child element '%s' found under parent '%s' but object is restricted to children of type '%s'.",
                pChildSimObject->getClassName(),
                pSimObject->getClassName(),
                pContainerChildClass->getClassName() );

            // NOTE: We can't delete the object as it may be referenced elsewhere!
            pChildSimObject = NULL;

            return;
        }
    }

    // Add child.
    pChildren->addTamlChild( pChildSimObject );

    // Find Taml callbacks for child.
    TamlCallbacks* pChildCallbacks = dynamic_cast<TamlCallbacks*>( pChildSimObject );

    // Do we have callbacks on the child?
    if ( pChildCallbacks != NULL )
    {
        // Yes, so perform callback.
        mpTaml->tamlAddParent( pChildCallbacks, pSimObject );
    }
}

//-----------------------------------------------------------------------------

inline void TamlJSONReader::parseCustom( rapidjson::Value::ConstMemberIterator& memberItr, SimObject* pSimObject, const char* pCustomNodeName, TamlCustomNodes& customNodes )
{
    // Debug Profiling.
    PROFILE_SCOPE(TamlJSONReader_ParseCustom);

    // Fetch value.
    const rapidjson::Value& value = memberItr->value;

    // Add custom node.
    TamlCustomNode* pCustomNode = customNodes.addNode( pCustomNodeName );

    // Iterate members.
    for( rapidjson::Value::ConstMemberIterator customMemberItr = value.MemberBegin(); customMemberItr != value.MemberEnd(); ++customMemberItr )
    {
        // Fetch value.
        const rapidjson::Value& customValue = customMemberItr->value;

        // Is the member an object?
        if ( !customValue.IsObject() && !customValue.IsArray() )
        {
            // No, so warn.
            Con::warnf( "Taml::parseCustom() - Cannot process custom node name '%s' member as child value is not an object or array.", pCustomNodeName );
            return;
        }

        // Parse custom node.
        parseCustomNode( customMemberItr, pCustomNode );
    }
}

//-----------------------------------------------------------------------------

inline void TamlJSONReader::parseCustomNode( rapidjson::Value::ConstMemberIterator& memberItr, TamlCustomNode* pCustomNode )
{
    // Debug Profiling.
    PROFILE_SCOPE(TamlJSONReader_ParseCustomNode);

    // Fetch name and value.
    const rapidjson::Value& name = memberItr->name;
    const rapidjson::Value& value = memberItr->value;

    // Is the value an object?
    if ( value.IsObject() )
    {
        // Yes, so is the node a proxy object?
        if (  getTamlRefId( value ) != 0 || getTamlRefToId( value ) != 0 )
        {
            // Yes, so parse proxy object.
            SimObject* pProxyObject = parseType( memberItr );

            // Add child node.
            pCustomNode->addNode( pProxyObject );

            return;
        }
    }

    char valueBuffer[4096];

    // Fetch the node name.
    StringTableEntry nodeName = getDemangledName( name.GetString() );

    // Yes, so add child node.
    TamlCustomNode* pChildNode = pCustomNode->addNode( nodeName );

    // Is the value an array?
    if ( value.IsArray() )
    {
        // Yes, so does it have a single entry?
        if ( value.Size() == 1 )
        {
            // Yes, so parse the node text.
            if ( parseStringValue( valueBuffer, sizeof(valueBuffer), value.Begin(), nodeName ) )
            {
                pChildNode->setNodeText( valueBuffer );
            }
            else
            {
                // Warn.
                Con::warnf( "Taml::parseCustomNode() - Encountered text in the custom node '%s' but could not interpret the value.", nodeName );
            }
        }
        else
        {
            // No, so warn.
            Con::warnf( "Taml::parseCustomNode() - Encountered text in the custom node '%s' but more than a single element was found in the array.", nodeName );
        }

        return;
    }

    // Iterate child members.
    for( rapidjson::Value::ConstMemberIterator childMemberItr = value.MemberBegin(); childMemberItr != value.MemberEnd(); ++childMemberItr )
    {
        // Fetch name and value.
        const rapidjson::Value& childName = childMemberItr->name;
        const rapidjson::Value& childValue = childMemberItr->value;

        // Fetch the field name.
        StringTableEntry fieldName = StringTable->insert( childName.GetString() );

        // Is the value an array?
        if ( childValue.IsArray() )
        {
            // Yes, so does it have a single entry?
            if ( childValue.Size() == 1 )
            {
                // Yes, so parse the node text.
                if ( parseStringValue( valueBuffer, sizeof(valueBuffer), *childValue.Begin(), fieldName ) )
                {
                    // Yes, so add sub-child node.
                    TamlCustomNode* pSubChildNode = pChildNode->addNode( fieldName );

                    // Set sub-child text.
                    pSubChildNode->setNodeText( valueBuffer );
                    continue;
                }

                // Warn.
                Con::warnf( "Taml::parseCustomNode() - Encountered text in the custom node '%s' but could not interpret the value.", fieldName );
                return;
            }

            // No, so warn.
            Con::warnf( "Taml::parseCustomNode() - Encountered text in the custom node '%s' but more than a single element was found in the array.", fieldName );
            return;
        }

        // Is the member an object?
        if ( childValue.IsObject() )
        {
            // Yes, so parse custom node.
            parseCustomNode( childMemberItr, pChildNode );
            continue;
        }

        // Ignore if this is a Taml attribute.
        if (    fieldName == tamlRefIdName ||
                fieldName == tamlRefToIdName ||
                fieldName == tamlNamedObjectName )
                continue;

        // Parse string value.
        if ( !parseStringValue( valueBuffer, sizeof(valueBuffer), childValue, childName.GetString() ) )
        {
            // Warn.
            Con::warnf( "Taml::parseCustomNode() - Could not interpret value for field '%s'", fieldName );
            continue;
        }
        
        // Add node field.
        pChildNode->addField( fieldName, valueBuffer );
    }
}

//-----------------------------------------------------------------------------

inline StringTableEntry TamlJSONReader::getDemangledName( const char* pMangledName )
{
    // Debug Profiling.
    PROFILE_SCOPE(TamlJSONReader_GetDemangledName);

    // Is the type name mangled?
    if ( StringUnit::getUnitCount( pMangledName, JSON_RFC4627_NAME_MANGLING_CHARACTERS ) > 1 )
    {
        // Yes, so fetch type name portion.
        return StringTable->insert( StringUnit::getUnit( pMangledName, 0, JSON_RFC4627_NAME_MANGLING_CHARACTERS ) );
    }

    // No, so use all the type name.
    return StringTable->insert( pMangledName );
}

//-----------------------------------------------------------------------------

inline bool TamlJSONReader::parseStringValue( char* pBuffer, const S32 bufferSize, const rapidjson::Value& value, const char* pName )
{
    // Debug Profiling.
    PROFILE_SCOPE(TamlJSONReader_ParseStringValue);

    // Handle field value appropriately.

    if ( value.IsString() )
    {
        dSprintf( pBuffer, bufferSize, "%s", value.GetString() );
        return true;
    }

    if ( value.IsNumber() )
    {
        if ( value.IsInt() )
        {
            dSprintf( pBuffer, bufferSize, "%d", value.GetInt() );
            return true;
        }

        if ( value.IsUint() )
        {
            dSprintf( pBuffer, bufferSize, "%d", value.GetUint() );
            return true;
        }

        if ( value.IsInt64() )
        {
            dSprintf( pBuffer, bufferSize, "%d", value.GetInt64() );
            return true;
        }

        if ( value.IsUint64() )
        {
            dSprintf( pBuffer, bufferSize, "%d", value.GetUint64() );
            return true;
        }

        if ( value.IsDouble() )
        {
            dSprintf( pBuffer, bufferSize, "%f", value.GetDouble() );
            return true;
        }
    }

    if ( value.IsBool() )
    {
        dSprintf( pBuffer, bufferSize, "%d", value.GetBool() );
        return true;
    }

    // Failed to get value type.
    Con::warnf( "Taml: Encountered a field '%s' but its value is an unknown type.", pName );
    return false;
}

//-----------------------------------------------------------------------------

inline U32 TamlJSONReader::getTamlRefId( const rapidjson::Value& value )
{
    // Debug Profiling.
    PROFILE_SCOPE(TamlJSONReader_GetTamlRefId);

    // Is the value an object?
    if ( !value.IsObject()  )
    {
        // No, so warn.
        Con::warnf( "Taml::getTamlRefId() - Cannot get '%s' member as value is not an object.", tamlRefIdName );
        return 0;
    }

    // Iterate members.
    for( rapidjson::Value::ConstMemberIterator memberItr = value.MemberBegin(); memberItr != value.MemberEnd(); ++memberItr )
    {
        // Insert member name.
        StringTableEntry attributeName = StringTable->insert( memberItr->name.GetString() );

        // Skip if not the correct attribute.
        if ( attributeName != tamlRefIdName )
            continue;

        // Is the value an integer?
        if ( !memberItr->value.IsInt() )
        {
            // No, so warn.
            Con::warnf( "Taml::getTamlRefId() - Found '%s' member but it is not an integer.", tamlRefIdName );
            return 0;
        }

        // Return it.
        return (U32)memberItr->value.GetInt();
    }

    // Not found.
    return 0;
}

//-----------------------------------------------------------------------------

inline U32 TamlJSONReader::getTamlRefToId( const rapidjson::Value& value )
{
    // Debug Profiling.
    PROFILE_SCOPE(TamlJSONReader_GetTamlRefToId);

    // Is the value an object?
    if ( !value.IsObject()  )
    {
        // No, so warn.
        Con::warnf( "Taml::getTamlRefToId() - Cannot get '%s' member as value is not an object.", tamlRefToIdName );
        return 0;
    }

    // Iterate members.
    for( rapidjson::Value::ConstMemberIterator memberItr = value.MemberBegin(); memberItr != value.MemberEnd(); ++memberItr )
    {
        // Insert member name.
        StringTableEntry attributeName = StringTable->insert( memberItr->name.GetString() );

        // Skip if not the correct attribute.
        if ( attributeName != tamlRefToIdName )
            continue;

        // Is the value an integer?
        if ( !memberItr->value.IsInt() )
        {
            // No, so warn.
            Con::warnf( "Taml::getTamlRefToId() - Found '%s' member but it is not an integer.", tamlRefToIdName );
            return 0;
        }

        // Return it.
        return (U32)memberItr->value.GetInt();
    }

    // Not found.
    return 0;
}

//-----------------------------------------------------------------------------

inline const char* TamlJSONReader::getTamlObjectName( const rapidjson::Value& value )
{
    // Debug Profiling.
    PROFILE_SCOPE(TamlJSONReader_GetTamlObjectName);

    // Is the value an object?
    if ( !value.IsObject()  )
    {
        // No, so warn.
        Con::warnf( "Taml::getTamlObjectName() - Cannot get '%s' member as value is not an object.", tamlNamedObjectName );
        return 0;
    }

    // Iterate members.
    for( rapidjson::Value::ConstMemberIterator memberItr = value.MemberBegin(); memberItr != value.MemberEnd(); ++memberItr )
    {
        // Insert member name.
        StringTableEntry attributeName = StringTable->insert( memberItr->name.GetString() );

        // Skip if not the correct attribute.
        if ( attributeName != tamlNamedObjectName )
            continue;

        // Is the value an integer?
        if ( !memberItr->value.IsString() )
        {
            // No, so warn.
            Con::warnf( "Taml::getTamlObjectName() - Found '%s' member but it is not a string.", tamlNamedObjectName );
            return NULL;
        }

        // Return it.
        return memberItr->value.GetString();
    }

    // Not found.
    return NULL;
}



