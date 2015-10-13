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

#include "persistence/taml/binary/tamlBinaryWriter.h"

#ifndef _ZIPSUBSTREAM_H_
#include "core/util/zip/zipSubStream.h"
#endif

// Debug Profiling.
#include "platform/profiler.h"

//-----------------------------------------------------------------------------

bool TamlBinaryWriter::write( FileStream& stream, const TamlWriteNode* pTamlWriteNode, const bool compressed )
{
    // Debug Profiling.
    PROFILE_SCOPE(TamlBinaryWriter_Write);
 
    // Write Taml signature.
    stream.writeString( StringTable->insert( TAML_SIGNATURE ) );

    // Write version Id.
    stream.write( mVersionId );

    // Write compressed flag.
    stream.write( compressed );

    // Are we compressed?
    if ( compressed )
    {
        // yes, so attach zip stream.
        ZipSubWStream zipStream;
        zipStream.attachStream( &stream );

        // Write element.
        writeElement( zipStream, pTamlWriteNode );

        // Detach zip stream.
        zipStream.detachStream();
    }
    else
    {
        // No, so write element.
        writeElement( stream, pTamlWriteNode );
    }

    return true;
}

//-----------------------------------------------------------------------------

void TamlBinaryWriter::writeElement( Stream& stream, const TamlWriteNode* pTamlWriteNode )
{
    // Debug Profiling.
    PROFILE_SCOPE(TamlBinaryWriter_WriteElement);

    // Fetch object.
    SimObject* pSimObject = pTamlWriteNode->mpSimObject;

    // Fetch element name.
    const char* pElementName = pSimObject->getClassName();

    // Write element name.
    stream.writeString( pElementName );

    // Fetch object name.
    const char* pObjectName = pTamlWriteNode->mpObjectName;

    // Write object name.
    stream.writeString( pObjectName != NULL ? pObjectName : StringTable->EmptyString() );

    // Fetch reference Id.
    const U32 tamlRefId = pTamlWriteNode->mRefId;

    // Write reference Id.
    stream.write( tamlRefId );

    // Do we have a reference to node?
    if ( pTamlWriteNode->mRefToNode != NULL )
    {
        // Yes, so fetch reference to Id.
        const U32 tamlRefToId = pTamlWriteNode->mRefToNode->mRefId;

        // Sanity!
        AssertFatal( tamlRefToId != 0, "Taml: Invalid reference to Id." );

        // Write reference to Id.
        stream.write( tamlRefToId );

        // Finished.
        return;
    }

    // No, so write no reference to Id.
    stream.write( 0 );

    // Write attributes.
    writeAttributes( stream, pTamlWriteNode );

    // Write children.
    writeChildren( stream, pTamlWriteNode );

    // Write custom elements.
    writeCustomElements( stream, pTamlWriteNode );
}

//-----------------------------------------------------------------------------

void TamlBinaryWriter::writeAttributes( Stream& stream, const TamlWriteNode* pTamlWriteNode )
{
    // Debug Profiling.
    PROFILE_SCOPE(TamlBinaryWriter_WriteAttributes);

    // Fetch fields.
    const Vector<TamlWriteNode::FieldValuePair*>& fields = pTamlWriteNode->mFields;

    // Write placeholder attribute count.
    stream.write( (U32)fields.size() );

    // Finish if no fields.
    if ( fields.size() == 0 )
        return;

    // Iterate fields.
    for( Vector<TamlWriteNode::FieldValuePair*>::const_iterator itr = fields.begin(); itr != fields.end(); ++itr )
    {
        // Fetch field/value pair.
        TamlWriteNode::FieldValuePair* pFieldValue = (*itr);

        // Write attribute.
        stream.writeString( pFieldValue->mName );
        stream.writeLongString( 4096, pFieldValue->mpValue );
    }
}

void TamlBinaryWriter::writeChildren( Stream& stream, const TamlWriteNode* pTamlWriteNode )
{
    // Debug Profiling.
    PROFILE_SCOPE(TamlBinaryWriter_WriteChildren);

    // Fetch children.
    Vector<TamlWriteNode*>* pChildren = pTamlWriteNode->mChildren;

    // Do we have any children?
    if ( pChildren == NULL )
    {
        // No, so write no children.
        stream.write( (U32)0 );
        return;
    }

    // Write children count.
    stream.write( (U32)pChildren->size() );

    // Iterate children.
    for( Vector<TamlWriteNode*>::iterator itr = pChildren->begin(); itr != pChildren->end(); ++itr )
    {
        // Write child.
        writeElement( stream, (*itr) );
    }
}

//-----------------------------------------------------------------------------

void TamlBinaryWriter::writeCustomElements( Stream& stream, const TamlWriteNode* pTamlWriteNode )
{
    // Debug Profiling.
    PROFILE_SCOPE(TamlBinaryWriter_WriteCustomElements);

    // Fetch custom nodes.
    const TamlCustomNodes& customNodes = pTamlWriteNode->mCustomNodes;

    // Fetch custom nodes.
    const TamlCustomNodeVector& nodes = customNodes.getNodes();

    // Write custom node count.
    stream.write( (U32)nodes.size() );

    // Finish if there are no nodes.
    if ( nodes.size() == 0 )
        return;

    // Iterate custom nodes.
    for( TamlCustomNodeVector::const_iterator customNodesItr = nodes.begin(); customNodesItr != nodes.end(); ++customNodesItr )
    {
        // Fetch the custom node.
        TamlCustomNode* pCustomNode = *customNodesItr;

        // Write custom node name.
        stream.writeString( pCustomNode->getNodeName() );

        // Fetch node children.
        const TamlCustomNodeVector& nodeChildren = pCustomNode->getChildren();

        // Iterate children nodes.
        for( TamlCustomNodeVector::const_iterator childNodeItr = nodeChildren.begin(); childNodeItr != nodeChildren.end(); ++childNodeItr )
        {
            // Fetch child node.
            const TamlCustomNode* pChildNode = *childNodeItr;

            // Write the custom node.
            writeCustomNode( stream, pChildNode );
        }
    }
}

//-----------------------------------------------------------------------------

void TamlBinaryWriter::writeCustomNode( Stream& stream, const TamlCustomNode* pCustomNode )
{
    // Is the node a proxy object?
    if ( pCustomNode->isProxyObject() )
    {
        // Yes, so flag as proxy object.
        stream.write( true );

        // Write the element.
        writeElement( stream, pCustomNode->getProxyWriteNode() );
        return;
    }

    // No, so flag as custom node.
    stream.write( false );

    // Write custom node name.
    stream.writeString( pCustomNode->getNodeName() );

    // Write custom node text.
    stream.writeLongString(MAX_TAML_NODE_FIELDVALUE_LENGTH, pCustomNode->getNodeTextField().getFieldValue());

    // Fetch node children.
    const TamlCustomNodeVector& nodeChildren = pCustomNode->getChildren();

    // Fetch child node count.
    const U32 childNodeCount = (U32)nodeChildren.size();

    // Write custom node count.
    stream.write( childNodeCount );

    // Do we have any children nodes.
    if ( childNodeCount > 0 )
    {
        // Yes, so iterate children nodes.
        for( TamlCustomNodeVector::const_iterator childNodeItr = nodeChildren.begin(); childNodeItr != nodeChildren.end(); ++childNodeItr )
        {
            // Fetch child node.
            const TamlCustomNode* pChildNode = *childNodeItr;

            // Write the custom node.
            writeCustomNode( stream, pChildNode );
        }
    }

    // Fetch fields.
    const TamlCustomFieldVector& fields = pCustomNode->getFields();

    // Fetch child field count.
    const U32 childFieldCount = (U32)fields.size();

    // Write custom field count.
    stream.write( childFieldCount );

    // Do we have any child fields?
    if ( childFieldCount > 0 )
    {
        // Yes, so iterate  fields.
        for ( TamlCustomFieldVector::const_iterator fieldItr = fields.begin(); fieldItr != fields.end(); ++fieldItr )
        {
            // Fetch node field.
            const TamlCustomField* pField = *fieldItr;

            // Write the node field.
            stream.writeString( pField->getFieldName() );
            stream.writeLongString( MAX_TAML_NODE_FIELDVALUE_LENGTH, pField->getFieldValue() );
        }
    }
}