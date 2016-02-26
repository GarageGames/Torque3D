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

#include "persistence/taml/xml/tamlXmlWriter.h"

// Debug Profiling.
#include "platform/profiler.h"
#include "persistence/taml/fsTinyXml.h"

//-----------------------------------------------------------------------------

bool TamlXmlWriter::write( FileStream& stream, const TamlWriteNode* pTamlWriteNode )
{
    // Debug Profiling.
    PROFILE_SCOPE(TamlXmlWriter_Write);

    // Create document.
    fsTiXmlDocument xmlDocument;

    // Compile the root element.
    TiXmlElement* pRootElement = compileElement( pTamlWriteNode );

    // Fetch any TAML Schema file reference.
    const char* pTamlSchemaFile = Con::getVariable( TAML_SCHEMA_VARIABLE );

    // Do we have a schema file reference?
    if ( pTamlSchemaFile != NULL && *pTamlSchemaFile != 0 )
    {
        // Yes, so add namespace attribute to root.
        pRootElement->SetAttribute( "xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance" );

        // Expand the file-path reference.
        char schemaFilePathBuffer[1024];
        Con::expandToolScriptFilename( schemaFilePathBuffer, sizeof(schemaFilePathBuffer), pTamlSchemaFile );

        // Fetch the output path for the Taml file.
        char outputFileBuffer[1024];
        dSprintf( outputFileBuffer, sizeof(outputFileBuffer), "%s", mpTaml->getFilePathBuffer() );
        char* pFileStart = dStrrchr( outputFileBuffer, '/' );
        if ( pFileStart == NULL )
            *outputFileBuffer = 0;
        else
            *pFileStart = 0;

        // Fetch the schema file-path relative to the output file.
        StringTableEntry relativeSchemaFilePath = Platform::makeRelativePathName( schemaFilePathBuffer, outputFileBuffer );

        // Add schema location attribute to root.
        pRootElement->SetAttribute( "xsi:noNamespaceSchemaLocation", relativeSchemaFilePath );
    }

    // Link the root element.
    xmlDocument.LinkEndChild( pRootElement );

    // Save document to stream.
    return xmlDocument.SaveFile( stream );
}

//-----------------------------------------------------------------------------

TiXmlElement* TamlXmlWriter::compileElement( const TamlWriteNode* pTamlWriteNode )
{
    // Debug Profiling.
    PROFILE_SCOPE(TamlXmlWriter_CompileElement);

    // Fetch object.
    SimObject* pSimObject = pTamlWriteNode->mpSimObject;

    // Fetch element name.
    const char* pElementName = pSimObject->getClassName();

    // Create element.
    TiXmlElement* pElement = new fsTiXmlElement( pElementName );

    // Fetch reference Id.
    const U32 referenceId = pTamlWriteNode->mRefId;

    // Do we have a reference Id?
    if ( referenceId != 0 )
    {
        // Yes, so set reference Id attribute.
        pElement->SetAttribute( tamlRefIdName, referenceId );
    }

    // Do we have a reference to node?
    else if ( pTamlWriteNode->mRefToNode != NULL )
    {
        // Yes, so fetch reference to Id.
        const U32 referenceToId = pTamlWriteNode->mRefToNode->mRefId;

        // Sanity!
        AssertFatal( referenceToId != 0, "Taml: Invalid reference to Id." );

        // Set reference to Id attribute.
        pElement->SetAttribute( tamlRefToIdName, referenceToId );

        // Finish because we're a reference to another object.
        return pElement;
    }

    // Fetch object name.
    const char* pObjectName = pTamlWriteNode->mpObjectName;

    // Do we have a name?
    if ( pObjectName != NULL )
    {
        // Yes, so set name attribute.
        pElement->SetAttribute( tamlNamedObjectName, pObjectName );
    }

    // Compile attributes.
    compileAttributes( pElement, pTamlWriteNode );

    // Fetch children.
    Vector<TamlWriteNode*>* pChildren = pTamlWriteNode->mChildren;

    // Do we have any children?
    if ( pChildren )
    {
        // Yes, so iterate children.
        for( Vector<TamlWriteNode*>::iterator itr = pChildren->begin(); itr != pChildren->end(); ++itr )
        {
            // Write child element.
            pElement->LinkEndChild( compileElement( (*itr) ) );
        }
    }

    // Compile custom elements.
    compileCustomElements( pElement, pTamlWriteNode );

    return pElement;
}

//-----------------------------------------------------------------------------

void TamlXmlWriter::compileAttributes( TiXmlElement* pXmlElement, const TamlWriteNode* pTamlWriteNode )
{
    // Debug Profiling.
    PROFILE_SCOPE(TamlXmlWriter_CompileAttributes);

    // Fetch fields.
    const Vector<TamlWriteNode::FieldValuePair*>& fields = pTamlWriteNode->mFields;

    // Ignore if no fields.
    if ( fields.size() == 0 )
        return;

    // Iterate fields.
    for( Vector<TamlWriteNode::FieldValuePair*>::const_iterator itr = fields.begin(); itr != fields.end(); ++itr )
    {
        // Fetch field/value pair.
        TamlWriteNode::FieldValuePair* pFieldValue = (*itr);

        // Set field attribute.
        pXmlElement->SetAttribute( pFieldValue->mName, pFieldValue->mpValue );
    }
}

//-----------------------------------------------------------------------------

void TamlXmlWriter::compileCustomElements( TiXmlElement* pXmlElement, const TamlWriteNode* pTamlWriteNode )
{
    // Debug Profiling.
    PROFILE_SCOPE(TamlXmlWriter_CompileCustomElements);

    // Fetch custom nodes.
    const TamlCustomNodes& customNodes = pTamlWriteNode->mCustomNodes;

    // Fetch custom nodes.
    const TamlCustomNodeVector& nodes = customNodes.getNodes();

    // Finish if no custom nodes to process.
    if ( nodes.size() == 0 )
        return;

    // Iterate custom nodes.
    for( TamlCustomNodeVector::const_iterator customNodesItr = nodes.begin(); customNodesItr != nodes.end(); ++customNodesItr )
    {
        // Fetch the custom node.
        TamlCustomNode* pCustomNode = *customNodesItr;

        // Format extended element name.
        char extendedElementNameBuffer[256];
        dSprintf( extendedElementNameBuffer, sizeof(extendedElementNameBuffer), "%s.%s", pXmlElement->Value(), pCustomNode->getNodeName() );
        StringTableEntry extendedElementName = StringTable->insert( extendedElementNameBuffer );

        // Create element.
        TiXmlElement* pExtendedPropertyElement = new fsTiXmlElement( extendedElementName );

        // Fetch node children.
        const TamlCustomNodeVector& nodeChildren = pCustomNode->getChildren();

        // Iterate children nodes.
        for( TamlCustomNodeVector::const_iterator childNodeItr = nodeChildren.begin(); childNodeItr != nodeChildren.end(); ++childNodeItr )
        {
            // Fetch child node.
            const TamlCustomNode* pChildNode = *childNodeItr;

            // Compile the custom node.
            compileCustomNode( pExtendedPropertyElement, pChildNode );
        }

        // Finish if the node is set to ignore if empty and it is empty.
        if ( pCustomNode->getIgnoreEmpty() && pExtendedPropertyElement->NoChildren() )
        {
            // Yes, so delete the extended element.
            delete pExtendedPropertyElement;
            pExtendedPropertyElement = NULL;
        }
        else
        {
            // No, so add elementt as child.
            pXmlElement->LinkEndChild( pExtendedPropertyElement );
        }
    }
}

//-----------------------------------------------------------------------------

void TamlXmlWriter::compileCustomNode( TiXmlElement* pXmlElement, const TamlCustomNode* pCustomNode )
{
    // Finish if the node is set to ignore if empty and it is empty.
    if ( pCustomNode->getIgnoreEmpty() && pCustomNode->isEmpty() )
        return;

    // Is the node a proxy object?
    if ( pCustomNode->isProxyObject() )
    {
        // Yes, so write the proxy object.
        pXmlElement->LinkEndChild( compileElement( pCustomNode->getProxyWriteNode() ) );
        return;
    }

    // Create element.
    TiXmlElement* pNodeElement = new fsTiXmlElement( pCustomNode->getNodeName() );

    // Is there any node text?
    if ( !pCustomNode->getNodeTextField().isValueEmpty() )
    {
        // Yes, so add a text node.
        pNodeElement->LinkEndChild( new TiXmlText( pCustomNode->getNodeTextField().getFieldValue() ) );
    }

    // Fetch fields.
    const TamlCustomFieldVector& fields = pCustomNode->getFields();

    // Iterate fields.
    for ( TamlCustomFieldVector::const_iterator fieldItr = fields.begin(); fieldItr != fields.end(); ++fieldItr )
    {
        // Fetch field.
        const TamlCustomField* pField = *fieldItr;

        // Set field.
        pNodeElement->SetAttribute( pField->getFieldName(), pField->getFieldValue() );
    }

    // Fetch node children.
    const TamlCustomNodeVector& nodeChildren = pCustomNode->getChildren();

    // Iterate children nodes.
    for( TamlCustomNodeVector::const_iterator childNodeItr = nodeChildren.begin(); childNodeItr != nodeChildren.end(); ++childNodeItr )
    {
        // Fetch child node.
        const TamlCustomNode* pChildNode = *childNodeItr;

        // Compile the child node.
        compileCustomNode( pNodeElement, pChildNode );
    }

    // Finish if the node is set to ignore if empty and it is empty (including fields).
    if ( pCustomNode->getIgnoreEmpty() && fields.size() == 0 && pNodeElement->NoChildren() )
    {
        // Yes, so delete the extended element.
        delete pNodeElement;
        pNodeElement = NULL;
    }
    else
    {
        // Add node element as child.
        pXmlElement->LinkEndChild( pNodeElement );
    }
}
