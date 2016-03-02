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

#include "persistence/taml/xml/tamlXmlReader.h"

// Debug Profiling.
#include "platform/profiler.h"
#include "persistence/taml/fsTinyXml.h"

//-----------------------------------------------------------------------------

SimObject* TamlXmlReader::read( FileStream& stream )
{
    // Debug Profiling.
    PROFILE_SCOPE(TamlXmlReader_Read);

    // Create document.
    fsTiXmlDocument xmlDocument;

    // Load document from stream.
    if ( !xmlDocument.LoadFile( stream ) )
    {
        // Warn!
        Con::warnf("Taml: Could not load Taml XML file from stream.");
        return NULL;
    }

    // Parse root element.
    SimObject* pSimObject = parseElement( xmlDocument.RootElement() );

    // Reset parse.
    resetParse();

    return pSimObject;
}

//-----------------------------------------------------------------------------

void TamlXmlReader::resetParse( void )
{
    // Debug Profiling.
    PROFILE_SCOPE(TamlXmlReader_ResetParse);

    // Clear object reference map.
    mObjectReferenceMap.clear();
}

//-----------------------------------------------------------------------------

SimObject* TamlXmlReader::parseElement( TiXmlElement* pXmlElement )
{
    // Debug Profiling.
    PROFILE_SCOPE(TamlXmlReader_ParseElement);

    SimObject* pSimObject = NULL;

    // Fetch element name.
    StringTableEntry typeName = StringTable->insert( pXmlElement->Value() );

    // Fetch reference to Id.
    const U32 tamlRefToId = getTamlRefToId( pXmlElement );

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

    // No, so fetch reference Id.
    const U32 tamlRefId = getTamlRefId( pXmlElement );

#ifdef TORQUE_DEBUG
    // Format the type location.
    char typeLocationBuffer[64];
    dSprintf( typeLocationBuffer, sizeof(typeLocationBuffer), "Taml [format='xml' row=%d column=%d]", pXmlElement->Row(), pXmlElement->Column() );    

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
    parseAttributes( pXmlElement, pSimObject );

    // Fetch object name.
    StringTableEntry objectName = StringTable->insert( getTamlObjectName( pXmlElement ) );

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

    // Fetch any children.
    TiXmlNode* pChildXmlNode = pXmlElement->FirstChild();

    TamlCustomNodes customProperties;

    // Do we have any element children?
    if ( pChildXmlNode != NULL )
    {
        // Fetch the Taml children.
        TamlChildren* pChildren = dynamic_cast<TamlChildren*>( pSimObject );

        // Fetch any container child class specifier.
        AbstractClassRep* pContainerChildClass = pSimObject->getClassRep()->getContainerChildClass( true );

        // Iterate siblings.
        do
        {
            // Fetch element.
            TiXmlElement* pChildXmlElement = dynamic_cast<TiXmlElement*>( pChildXmlNode );

            // Move to next sibling.
            pChildXmlNode = pChildXmlNode->NextSibling();

            // Skip if this is not an element?
            if ( pChildXmlElement == NULL )
                continue;

            // Is this a standard child element?
            if ( dStrchr( pChildXmlElement->Value(), '.' ) == NULL )
            {
                // Is this a Taml child?
                if ( pChildren == NULL )
                {
                    // No, so warn.
                    Con::warnf("Taml: Child element '%s' found under parent '%s' but object cannot have children.",
                        pChildXmlElement->Value(),
                        pXmlElement->Value() );

                    // Skip.
                    continue;
                }

                // Yes, so parse child element.
                SimObject* pChildSimObject = parseElement( pChildXmlElement );

                // Skip if the child was not created.
                if ( pChildSimObject == NULL )
                    continue;

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
            else
            {
                // No, so parse custom element.
                parseCustomElement( pChildXmlElement, customProperties );
            }
        }
        while( pChildXmlNode != NULL );

        // Call custom read.
        mpTaml->tamlCustomRead( pCallbacks, customProperties );
    }

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

void TamlXmlReader::parseAttributes( TiXmlElement* pXmlElement, SimObject* pSimObject )
{
    // Debug Profiling.
    PROFILE_SCOPE(TamlXmlReader_ParseAttributes);

    // Sanity!
    AssertFatal( pSimObject != NULL, "Taml: Cannot parse attributes on a NULL object." );

    // Iterate attributes.
    for ( TiXmlAttribute* pAttribute = pXmlElement->FirstAttribute(); pAttribute; pAttribute = pAttribute->Next() )
    {
        // Insert attribute name.
        StringTableEntry attributeName = StringTable->insert( pAttribute->Name() );

        // Ignore if this is a Taml attribute.
        if (    attributeName == tamlRefIdName ||
                attributeName == tamlRefToIdName ||
                attributeName == tamlNamedObjectName )
            continue;

        // Set the field.
        pSimObject->setPrefixedDataField(attributeName, NULL, pAttribute->Value());
    }
}

//-----------------------------------------------------------------------------

void TamlXmlReader::parseCustomElement( TiXmlElement* pXmlElement, TamlCustomNodes& customNodes )
{
    // Debug Profiling.
    PROFILE_SCOPE(TamlXmlReader_ParseCustomElement);

    // Is this a standard child element?
    const char* pPeriod = dStrchr( pXmlElement->Value(), '.' );

    // Sanity!
    AssertFatal( pPeriod != NULL, "Parsing extended element but no period character found." );

    // Fetch any custom XML node.
    TiXmlNode* pCustomXmlNode = pXmlElement->FirstChild();

    // Finish is no XML node exists.
    if ( pCustomXmlNode == NULL )
        return;

    // Yes, so add custom node.
    TamlCustomNode* pCustomNode = customNodes.addNode( pPeriod+1 );

    do
    {
        // Fetch element.
        TiXmlElement* pCustomXmlElement = dynamic_cast<TiXmlElement*>( pCustomXmlNode );

        // Move to next sibling.
        pCustomXmlNode = pCustomXmlNode->NextSibling();

        // Skip if this is not an element.
        if ( pCustomXmlElement == NULL )
            continue;

        // Parse custom node.
        parseCustomNode( pCustomXmlElement, pCustomNode );
    }
    while ( pCustomXmlNode != NULL );
}

//-----------------------------------------------------------------------------

void TamlXmlReader::parseCustomNode( TiXmlElement* pXmlElement, TamlCustomNode* pCustomNode )
{
    // Is the node a proxy object?
    if (  getTamlRefId( pXmlElement ) != 0 || getTamlRefToId( pXmlElement ) != 0 )
    {
        // Yes, so parse proxy object.
        SimObject* pProxyObject = parseElement( pXmlElement );

        // Add child node.
        pCustomNode->addNode( pProxyObject );

        return;
    }

    // Yes, so add child node.
    TamlCustomNode* pChildNode = pCustomNode->addNode( pXmlElement->Value() );

    // Iterate attributes.
    for ( TiXmlAttribute* pAttribute = pXmlElement->FirstAttribute(); pAttribute; pAttribute = pAttribute->Next() )
    {
        // Insert attribute name.
        StringTableEntry attributeName = StringTable->insert( pAttribute->Name() );

        // Skip if a Taml reference attribute.
        if ( attributeName == tamlRefIdName || attributeName == tamlRefToIdName )
            continue;

        // Add node field.
        pChildNode->addField( attributeName, pAttribute->Value() );
    }

    // Fetch any element text.
    const char* pElementText = pXmlElement->GetText();

    // Do we have any element text?
    if ( pElementText != NULL )
    {
        // Yes, so store it.
        pChildNode->setNodeText( pElementText );
    }

    // Fetch any children.
    TiXmlNode* pChildXmlNode = pXmlElement->FirstChild();

    // Do we have any element children?
    if ( pChildXmlNode != NULL )
    {
        do
        {
            // Yes, so fetch child element.
            TiXmlElement* pChildXmlElement = dynamic_cast<TiXmlElement*>( pChildXmlNode );

            // Move to next sibling.
            pChildXmlNode = pChildXmlNode->NextSibling();

            // Skip if this is not an element.
            if ( pChildXmlElement == NULL )
                continue;

            // Parse custom node.
            parseCustomNode( pChildXmlElement, pChildNode );
        }
        while( pChildXmlNode != NULL );
    }
}

//-----------------------------------------------------------------------------

U32 TamlXmlReader::getTamlRefId( TiXmlElement* pXmlElement )
{
    // Debug Profiling.
    PROFILE_SCOPE(TamlXmlReader_GetTamlRefId);

    // Iterate attributes.
    for ( TiXmlAttribute* pAttribute = pXmlElement->FirstAttribute(); pAttribute; pAttribute = pAttribute->Next() )
    {
        // Insert attribute name.
        StringTableEntry attributeName = StringTable->insert( pAttribute->Name() );

        // Skip if not the correct attribute.
        if ( attributeName != tamlRefIdName )
            continue;

        // Return it.
        return dAtoi( pAttribute->Value() );
    }

    // Not found.
    return 0;
}

//-----------------------------------------------------------------------------

U32 TamlXmlReader::getTamlRefToId( TiXmlElement* pXmlElement )
{
    // Debug Profiling.
    PROFILE_SCOPE(TamlXmlReader_GetTamlRefToId);

    // Iterate attributes.
    for ( TiXmlAttribute* pAttribute = pXmlElement->FirstAttribute(); pAttribute; pAttribute = pAttribute->Next() )
    {
        // Insert attribute name.
        StringTableEntry attributeName = StringTable->insert( pAttribute->Name() );

        // Skip if not the correct attribute.
        if ( attributeName != tamlRefToIdName )
            continue;

        // Return it.
        return dAtoi( pAttribute->Value() );
    }

    // Not found.
    return 0;
}

//-----------------------------------------------------------------------------

const char* TamlXmlReader::getTamlObjectName( TiXmlElement* pXmlElement )
{
    // Debug Profiling.
    PROFILE_SCOPE(TamlXmlReader_GetTamlObjectName);

    // Iterate attributes.
    for ( TiXmlAttribute* pAttribute = pXmlElement->FirstAttribute(); pAttribute; pAttribute = pAttribute->Next() )
    {
        // Insert attribute name.
        StringTableEntry attributeName = StringTable->insert( pAttribute->Name() );

        // Skip if not the correct attribute.
        if ( attributeName != tamlNamedObjectName )
            continue;

        // Return it.
        return pAttribute->Value();
    }

    // Not found.
    return NULL;
}



