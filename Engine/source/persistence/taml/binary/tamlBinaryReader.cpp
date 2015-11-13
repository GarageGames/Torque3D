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

#include "persistence/taml/binary/tamlBinaryReader.h"

#ifndef _ZIPSUBSTREAM_H_
#include "core/util/zip/zipSubStream.h"
#endif

// Debug Profiling.
#include "platform/profiler.h"

//-----------------------------------------------------------------------------

SimObject* TamlBinaryReader::read( FileStream& stream )
{
    // Debug Profiling.
    PROFILE_SCOPE(TamlBinaryReader_Read);

    // Read Taml signature.
    StringTableEntry tamlSignature = stream.readSTString();

    // Is the signature correct?
    if ( tamlSignature != StringTable->insert( TAML_SIGNATURE ) )
    {
        // Warn.
        Con::warnf("Taml: Cannot read binary file as signature is incorrect '%s'.", tamlSignature );
        return NULL;
    }

    // Read version Id.
    U32 versionId;
    stream.read( &versionId );

    // Read compressed flag.
    bool compressed;
    stream.read( &compressed );

    SimObject* pSimObject = NULL;

    // Is the stream compressed?
    if ( compressed )
    {
        // Yes, so attach zip stream.
        ZipSubRStream zipStream;
        zipStream.attachStream( &stream );

        // Parse element.
        pSimObject = parseElement( zipStream, versionId );

        // Detach zip stream.
        zipStream.detachStream();
    }
    else
    {
        // No, so parse element.
        pSimObject = parseElement( stream, versionId );
    }

    return pSimObject;
}

//-----------------------------------------------------------------------------

void TamlBinaryReader::resetParse( void )
{
    // Debug Profiling.
    PROFILE_SCOPE(TamlBinaryReader_ResetParse);

    // Clear object reference map.
    mObjectReferenceMap.clear();
}

//-----------------------------------------------------------------------------

SimObject* TamlBinaryReader::parseElement( Stream& stream, const U32 versionId )
{
    // Debug Profiling.
    PROFILE_SCOPE(TamlBinaryReader_ParseElement);

    SimObject* pSimObject = NULL;

#ifdef TORQUE_DEBUG
    // Format the type location.
    char typeLocationBuffer[64];
    dSprintf( typeLocationBuffer, sizeof(typeLocationBuffer), "Taml [format='binary' offset=%u]", stream.getPosition() );
#endif

    // Fetch element name.    
    StringTableEntry typeName = stream.readSTString();

    // Fetch object name.
    StringTableEntry objectName = stream.readSTString();

    // Read references.
    U32 tamlRefId;
    U32 tamlRefToId;
    stream.read( &tamlRefId );
    stream.read( &tamlRefToId );

    // Do we have a reference to Id?
    if ( tamlRefToId != 0 )
    {
        // Yes, so fetch reference.
        typeObjectReferenceHash::Iterator referenceItr = mObjectReferenceMap.find( tamlRefToId );

        // Did we find the reference?
        if ( referenceItr == mObjectReferenceMap.end() )
        {
            // No, so warn.
            Con::warnf( "Taml: Could not find a reference Id of '%d'", tamlRefToId );
            return NULL;
        }

        // Return object.
        return referenceItr->value;
    }

#ifdef TORQUE_DEBUG
    // Create type.
    pSimObject = Taml::createType( typeName, mpTaml, typeLocationBuffer );
#else
    // Create type.
    pSimObject = Taml::createType( typeName, mpTaml );
#endif

    // Finish if we couldn't create the type.
    if ( pSimObject == NULL )
        return NULL;

    // Find Taml callbacks.
    TamlCallbacks* pCallbacks = dynamic_cast<TamlCallbacks*>( pSimObject );

    // Are there any Taml callbacks?
    if ( pCallbacks != NULL )
    {
        // Yes, so call it.
        mpTaml->tamlPreRead( pCallbacks );
    }

    // Parse attributes.
    parseAttributes( stream, pSimObject, versionId );

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
#ifdef TORQUE_DEBUG
            Con::warnf( "Taml::parseElement() - Registered an instance of type '%s' but a request to name it '%s' was rejected.  This is typically because an object of that name already exists.  '%s'", typeName, objectName, typeLocationBuffer );
#else
            Con::warnf( "Taml::parseElement() - Registered an instance of type '%s' but a request to name it '%s' was rejected.  This is typically because an object of that name already exists.", typeName, objectName );
#endif
        }
    }

    // Do we have a reference Id?
    if ( tamlRefId != 0 )
    {
        // Yes, so insert reference.
        mObjectReferenceMap.insertUnique( tamlRefId, pSimObject );
    }

    // Parse custom elements.
    TamlCustomNodes customProperties;

    // Parse children.
    parseChildren( stream, pCallbacks, pSimObject, versionId );

    // Parse custom elements.
    parseCustomElements( stream, pCallbacks, customProperties, versionId );

    // Are there any Taml callbacks?
    if ( pCallbacks != NULL )
    {
        // Yes, so call it.
        mpTaml->tamlPostRead( pCallbacks, customProperties );
    }

    // Return object.
    return pSimObject;
}

//-----------------------------------------------------------------------------

void TamlBinaryReader::parseAttributes( Stream& stream, SimObject* pSimObject, const U32 versionId )
{
    // Debug Profiling.
    PROFILE_SCOPE(TamlBinaryReader_ParseAttributes);

    // Sanity!
    AssertFatal( pSimObject != NULL, "Taml: Cannot parse attributes on a NULL object." );

    // Fetch attribute count.
    U32 attributeCount;
    stream.read( &attributeCount );

    // Finish if no attributes.
    if ( attributeCount == 0 )
        return;

    char valueBuffer[4096];

    // Iterate attributes.
    for ( U32 index = 0; index < attributeCount; ++index )
    {
        // Fetch attribute.
        StringTableEntry attributeName = stream.readSTString();
        stream.readLongString( 4096, valueBuffer );

        // We can assume this is a field for now.
        pSimObject->setPrefixedDataField(attributeName, NULL, valueBuffer);
    }
}

//-----------------------------------------------------------------------------

void TamlBinaryReader::parseChildren( Stream& stream, TamlCallbacks* pCallbacks, SimObject* pSimObject, const U32 versionId )
{
    // Debug Profiling.
    PROFILE_SCOPE(TamlBinaryReader_ParseChildren);

    // Sanity!
    AssertFatal( pSimObject != NULL, "Taml: Cannot parse children on a NULL object." );

    // Fetch children count.
    U32 childrenCount;
    stream.read( &childrenCount );

    // Finish if no children.
    if ( childrenCount == 0 )
        return;

    // Fetch the Taml children.
    TamlChildren* pChildren = dynamic_cast<TamlChildren*>( pSimObject );

    // Is this a sim set?
    if ( pChildren == NULL )
    {
        // No, so warn.
        Con::warnf("Taml: Child element found under parent but object cannot have children." );
        return;
    }

    // Fetch any container child class specifier.
    AbstractClassRep* pContainerChildClass = pSimObject->getClassRep()->getContainerChildClass( true );

    // Iterate children.
    for ( U32 index = 0; index < childrenCount; ++ index )
    {
        // Parse child element.
        SimObject* pChildSimObject = parseElement( stream, versionId );

        // Finish if child failed.
        if ( pChildSimObject == NULL )
            return;

        // Do we have a container child class?
        if ( pContainerChildClass != NULL )
        {
            // Yes, so is the child object the correctly derived type?
            if ( !pChildSimObject->getClassRep()->isClass( pContainerChildClass ) )
            {
                // No, so warn.
                Con::warnf("Taml: Child element '%s' found under parent '%s' but object is restricted to children of type '%s'.",
                    pChildSimObject->getClassName(),
                    pSimObject->getClassName(),
                    pContainerChildClass->getClassName() );

                // NOTE: We can't delete the object as it may be referenced elsewhere!
                pChildSimObject = NULL;

                // Skip.
                continue;
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
}

//-----------------------------------------------------------------------------

void TamlBinaryReader::parseCustomElements( Stream& stream, TamlCallbacks* pCallbacks, TamlCustomNodes& customNodes, const U32 versionId )
{
    // Debug Profiling.
    PROFILE_SCOPE(TamlBinaryReader_ParseCustomElement);

    // Read custom node count.
    U32 customNodeCount;
    stream.read( &customNodeCount );

    // Finish if no custom nodes.
    if ( customNodeCount == 0 )
        return;

    // Iterate custom nodes.
    for ( U32 nodeIndex = 0; nodeIndex < customNodeCount; ++nodeIndex )
    {
        //Read custom node name.
        StringTableEntry nodeName = stream.readSTString();

        // Add custom node.
        TamlCustomNode* pCustomNode = customNodes.addNode( nodeName );

        // Parse the custom node.
        parseCustomNode( stream, pCustomNode, versionId );
    }

    // Do we have callbacks?
    if ( pCallbacks == NULL )
    {
        // No, so warn.
        Con::warnf( "Taml: Encountered custom data but object does not support custom data." );
        return;
    }

    // Custom read callback.
    mpTaml->tamlCustomRead( pCallbacks, customNodes );
}

//-----------------------------------------------------------------------------

void TamlBinaryReader::parseCustomNode( Stream& stream, TamlCustomNode* pCustomNode, const U32 versionId )
{
    // Fetch if a proxy object.
    bool isProxyObject;
    stream.read( &isProxyObject );

    // Is this a proxy object?
    if ( isProxyObject )
    {
        // Yes, so parse proxy object.
        SimObject* pProxyObject = parseElement( stream, versionId );

        // Add child node.
        pCustomNode->addNode( pProxyObject );

        return;
    }

    // No, so read custom node name.
    StringTableEntry nodeName = stream.readSTString();

    // Add child node.
    TamlCustomNode* pChildNode = pCustomNode->addNode( nodeName );

    // Read child node text.
    char childNodeTextBuffer[MAX_TAML_NODE_FIELDVALUE_LENGTH];
    stream.readLongString( MAX_TAML_NODE_FIELDVALUE_LENGTH, childNodeTextBuffer );
    pChildNode->setNodeText( childNodeTextBuffer );

    // Read child node count.
    U32 childNodeCount;
    stream.read( &childNodeCount );

    // Do we have any children nodes?
    if ( childNodeCount > 0 )
    {
        // Yes, so parse children nodes.
        for( U32 childIndex = 0; childIndex < childNodeCount; ++childIndex )
        {
            // Parse child node.
            parseCustomNode( stream, pChildNode, versionId );
        }
    }

    // Read child field count.
    U32 childFieldCount;
    stream.read( &childFieldCount );

    // Do we have any child fields?
    if ( childFieldCount > 0 )
    {
        // Yes, so parse child fields.
        for( U32 childFieldIndex = 0; childFieldIndex < childFieldCount; ++childFieldIndex )
        {
            // Read field name.
            StringTableEntry fieldName = stream.readSTString();

            // Read field value.
            char valueBuffer[MAX_TAML_NODE_FIELDVALUE_LENGTH];
            stream.readLongString( MAX_TAML_NODE_FIELDVALUE_LENGTH, valueBuffer );

            // Add field.
            pChildNode->addField( fieldName, valueBuffer );
        }
    }
}