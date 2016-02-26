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

#include "persistence/taml/json/tamlJSONWriter.h"

#include "core/stringTable.h"
#include "core/stream/fileStream.h"

// Debug Profiling.
#include "platform/profiler.h"

//-----------------------------------------------------------------------------

// These are the characters allowed that separate the type-name from the type.
// These separators can be used to ensure that each member in an object has
// a unique name.
//
// It is important to understand that TAML does not require entries to be unique
// but technically the RFC4627 spec states it as "should" be seems to be taken
// as MUST.  See here: http://www.ietf.org/rfc/rfc4627.txt
//
// They can be placed as a suffix to the type-name.  The very first occurance
// is used to split the type-name from the suffix so you can form whatever
// suffix you like to make entries unique.
// Examples are "Sprite 0", "Sprite*0", "Sprite[0]", "Sprite,0" etc.
//
// Type-names can legally consist of a-z, A-Z, 0-9 and "_" (underscore) so these
// characters cannot be used for mangling.  Feel free to add any characters you
// require to this list.
StringTableEntry JSON_RFC4627_NAME_MANGLING_CHARACTERS = StringTable->insert(" !$%^&*()-+{}[]@:~#|\\/?<>,.\n\r\t");

// This is the "dSprintf" format that the JSON writer uses to encode each
// member if JSON_RFC4627 mode is on.  You are free to change this as long as
// you ensure that it starts with the "%s" character (which represents the type name)
// and is immediately followed by at least a single mangling character and that the
// "%d" is present as that represents the automatically-added member index.
StringTableEntry JSON_RFC4627_NAME_MANGLING_FORMAT = StringTable->insert( "%s[%d]" );

//-----------------------------------------------------------------------------

bool TamlJSONWriter::write( FileStream& stream, const TamlWriteNode* pTamlWriteNode )
{
    // Debug Profiling.
    PROFILE_SCOPE(TamlJSONWriter_Write);

    // Create document.
    rapidjson::Document document;
    document.SetObject();

    // Compile the root type.
    rapidjson::Value rootValue(rapidjson::kObjectType);
    compileType( document, &rootValue, NULL, pTamlWriteNode, -1 );

    // Write document to stream.
    rapidjson::PrettyWriter<FileStream> jsonStreamWriter( stream );
    document.Accept( jsonStreamWriter );

    return true;
}

//-----------------------------------------------------------------------------

void TamlJSONWriter::compileType( rapidjson::Document& document, rapidjson::Value* pTypeValue, rapidjson::Value* pParentValue, const TamlWriteNode* pTamlWriteNode, const S32 memberIndex )
{
    // Debug Profiling.
    PROFILE_SCOPE(TamlJSONWriter_CompileType);

    // Fetch the json document allocator.
    rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

    // Fetch object.
    SimObject* pSimObject = pTamlWriteNode->mpSimObject;

    // Fetch JSON strict flag (don't use it if member index is set to not use it).
    const bool jsonStrict = memberIndex == -1 ? false : mpTaml->getJSONStrict();
   
    // Fetch element name (mangled or not).
    StringTableEntry elementName = jsonStrict ? getManagedName( pSimObject->getClassName(), memberIndex ) : pSimObject->getClassName();
    
    // Is there a parent value?
    if ( pParentValue == NULL )
    {        
        // No, so add as document root value member.
        pTypeValue = &((document.AddMember( elementName, *pTypeValue, allocator ).MemberEnd()-1)->value);
    }
    else
    {
        // Yes, so add as a parent value member.
        pTypeValue = &((pParentValue->AddMember( elementName, *pTypeValue, allocator ).MemberEnd()-1)->value);
    }

    // Fetch reference Id.
    const U32 referenceId = pTamlWriteNode->mRefId;

    // Do we have a reference Id?
    if ( referenceId != 0 )
    {
        // Yes, so set reference Id.
        rapidjson::Value value;
        value.SetInt( referenceId );
        pTypeValue->AddMember( tamlRefIdName, value, allocator );
    }
    // Do we have a reference to node?
    else if ( pTamlWriteNode->mRefToNode != NULL )
    {
        // Yes, so fetch reference to Id.
        const U32 referenceToId = pTamlWriteNode->mRefToNode->mRefId;

        // Sanity!
        AssertFatal( referenceToId != 0, "Taml: Invalid reference to Id." );

        // Set reference to Id.
        rapidjson::Value value;
        value.SetInt( referenceToId );
        pTypeValue->AddMember( tamlRefToIdName, value, allocator );

        // Finish because we're a reference to another object.
        return;
    }

    // Fetch object name.
    const char* pObjectName = pTamlWriteNode->mpObjectName;

    // Do we have a name?
    if ( pObjectName != NULL )
    {
        // Yes, so set name.
        rapidjson::Value value;
        value.SetString( pObjectName, dStrlen(pObjectName), allocator );
        pTypeValue->AddMember( tamlNamedObjectName, value, allocator );
    }

    // Compile field.
    compileFields( document, pTypeValue, pTamlWriteNode );

    // Fetch children.
    Vector<TamlWriteNode*>* pChildren = pTamlWriteNode->mChildren;

    // Do we have any children?
    if ( pChildren )
    {
        S32 childMemberIndex = 0;

        // Yes, so iterate children.
        for( Vector<TamlWriteNode*>::iterator itr = pChildren->begin(); itr != pChildren->end(); ++itr )
        {
            // Compile child type.
            rapidjson::Value childValue(rapidjson::kObjectType);
            compileType( document, &childValue, pTypeValue, (*itr), childMemberIndex++ );
        }
    }

    // Compile custom.
    compileCustom( document, pTypeValue, pTamlWriteNode );
}

//-----------------------------------------------------------------------------

void TamlJSONWriter::compileFields( rapidjson::Document& document, rapidjson::Value* pTypeValue, const TamlWriteNode* pTamlWriteNode )
{
    // Debug Profiling.
    PROFILE_SCOPE(TamlJSONWriter_CompileFields);

    // Fetch fields.
    const Vector<TamlWriteNode::FieldValuePair*>& fields = pTamlWriteNode->mFields;

    // Ignore if no fields.
    if ( fields.size() == 0 )
        return;

    // Fetch the json document allocator.
    rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

    // Iterate fields.
    for( Vector<TamlWriteNode::FieldValuePair*>::const_iterator itr = fields.begin(); itr != fields.end(); ++itr )
    {
        // Fetch field/value pair.
        TamlWriteNode::FieldValuePair* pFieldValue = (*itr);

        // Set field attribute.
        rapidjson::Value value;
        value.SetString( pFieldValue->mpValue, dStrlen(pFieldValue->mpValue), allocator );
        pTypeValue->AddMember( pFieldValue->mName, value, allocator );
    }
}

//-----------------------------------------------------------------------------

void TamlJSONWriter::compileCustom( rapidjson::Document& document, rapidjson::Value* pTypeValue, const TamlWriteNode* pTamlWriteNode )
{
    // Debug Profiling.
    PROFILE_SCOPE(TamlJSONWriter_CompileCustom);

    // Fetch custom nodes.
    const TamlCustomNodes& customNodes = pTamlWriteNode->mCustomNodes;

    // Fetch custom nodes.
    const TamlCustomNodeVector& nodes = customNodes.getNodes();

    // Finish if no custom nodes to process.
    if ( nodes.size() == 0 )
        return;

    // Fetch the json document allocator.
    rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

    // Fetch object.
    SimObject* pSimObject = pTamlWriteNode->mpSimObject;

    // Fetch element name.
    const char* pElementName = pSimObject->getClassName();

    // Iterate custom nodes.
    for( TamlCustomNodeVector::const_iterator customNodesItr = nodes.begin(); customNodesItr != nodes.end(); ++customNodesItr )
    {
        // Fetch the custom node.
        TamlCustomNode* pCustomNode = *customNodesItr;

        // Format extended element name.
        char extendedElementNameBuffer[256];
        dSprintf( extendedElementNameBuffer, sizeof(extendedElementNameBuffer), "%s.%s", pElementName, pCustomNode->getNodeName() );
        StringTableEntry elementNameEntry = StringTable->insert( extendedElementNameBuffer );

        rapidjson::Value elementValue(rapidjson::kObjectType);
        rapidjson::Value* pElementValue = &((pTypeValue->AddMember( elementNameEntry, elementValue, allocator ).MemberEnd()-1)->value);

        // Fetch node children.
        const TamlCustomNodeVector& nodeChildren = pCustomNode->getChildren();

        S32 childMemberIndex = 0;

        // Iterate children nodes.
        for( TamlCustomNodeVector::const_iterator childNodeItr = nodeChildren.begin(); childNodeItr != nodeChildren.end(); ++childNodeItr )
        {
            // Fetch child node.
            const TamlCustomNode* pChildNode = *childNodeItr;

            // Compile the custom node.
            compileCustomNode( document, pElementValue, pChildNode, childMemberIndex++ );
        }

        // Finish if the node is set to ignore if empty and it is empty.
        if ( pCustomNode->getIgnoreEmpty() && pTypeValue->MemberBegin() == pTypeValue->MemberEnd() )
        {
            // Yes, so delete the member.
            pElementValue->SetNull();
        }
    }
}

//-----------------------------------------------------------------------------

void TamlJSONWriter::compileCustomNode( rapidjson::Document& document, rapidjson::Value* pParentValue, const TamlCustomNode* pCustomNode, const S32 memberIndex )
{
    // Debug Profiling.
    PROFILE_SCOPE(TamlJSONWriter_CompileCustomNode);

    // Finish if the node is set to ignore if empty and it is empty.
    if ( pCustomNode->getIgnoreEmpty() && pCustomNode->isEmpty() )
        return;

    // Is the node a proxy object?
    if ( pCustomNode->isProxyObject() )
    {
        // Yes, so write the proxy object.
        rapidjson::Value proxyValue(rapidjson::kObjectType);
        compileType( document, &proxyValue, pParentValue, pCustomNode->getProxyWriteNode(), memberIndex );
        return;
    }

    // Fetch the json document allocator.
    rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

    // Fetch fields.
    const TamlCustomFieldVector& fields = pCustomNode->getFields();

    // Fetch JSON strict flag (don't use it if member index is set to not use it).
    const bool jsonStrict = memberIndex == -1 ? false : mpTaml->getJSONStrict();
   
    // Fetch element name (mangled or not).
    StringTableEntry nodeName = jsonStrict ? getManagedName( pCustomNode->getNodeName(), memberIndex ) : pCustomNode->getNodeName();

    // Is there any node text?
    if ( !pCustomNode->getNodeTextField().isValueEmpty() )
    {
        // Yes, so fetch text.
        const char* pNodeText = pCustomNode->getNodeTextField().getFieldValue();

        // Create custom value.
        rapidjson::Value customTextValue(rapidjson::kArrayType);
        customTextValue.PushBack( pNodeText, allocator );
        pParentValue->AddMember( nodeName, customTextValue, allocator );
        return;
    }

    // Create custom value.
    rapidjson::Value customValue(rapidjson::kObjectType);
    rapidjson::Value* pCustomValue = &((pParentValue->AddMember( nodeName, customValue, allocator ).MemberEnd()-1)->value);

    // Iterate fields.
    for ( TamlCustomFieldVector::const_iterator fieldItr = fields.begin(); fieldItr != fields.end(); ++fieldItr )
    {
        // Fetch field.
        const TamlCustomField* pField = *fieldItr;

        // Add a field.
        rapidjson::Value fieldValue;
        fieldValue.SetString( pField->getFieldValue(), dStrlen(pField->getFieldValue()), allocator );
        pCustomValue->AddMember( pField->getFieldName(), fieldValue, allocator );
    }

    // Fetch node children.
    const TamlCustomNodeVector& nodeChildren = pCustomNode->getChildren();

    S32 childMemberIndex = 0;

    // Iterate children nodes.
    for( TamlCustomNodeVector::const_iterator childNodeItr = nodeChildren.begin(); childNodeItr != nodeChildren.end(); ++childNodeItr )
    {
        // Fetch child node.
        const TamlCustomNode* pChildNode = *childNodeItr;

        // Compile the child node.
        compileCustomNode( document, pCustomValue, pChildNode, childMemberIndex++ );
    }

    // Finish if the node is set to ignore if empty and it is empty (including fields).
    if ( pCustomNode->getIgnoreEmpty() && fields.size() == 0 && pCustomValue->MemberBegin() == pCustomValue->MemberEnd() )
    {
        // Yes, so delete the member.
        pCustomValue->SetNull();
    }
}

//-----------------------------------------------------------------------------

inline StringTableEntry TamlJSONWriter::getManagedName( const char* pName, const S32 memberIndex )
{
    // Debug Profiling.
    PROFILE_SCOPE(TamlJSONWriter_getManagedName);

    char nameBuffer[1024];

    // Format mangled name.
    dSprintf( nameBuffer, sizeof(nameBuffer), JSON_RFC4627_NAME_MANGLING_FORMAT, pName, memberIndex );

    return StringTable->insert( nameBuffer );
}