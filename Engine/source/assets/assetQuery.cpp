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

#ifndef _ASSET_QUERY_H_
#include "assetQuery.h"
#endif

#ifndef _CONSOLETYPES_H_
#include "console/consoleTypes.h"
#endif

#ifndef _TAML_CUSTOM_H_
#include "persistence/taml/tamlCustom.h"
#endif

// Script bindings.
#include "assetQuery_ScriptBinding.h"

//-----------------------------------------------------------------------------

IMPLEMENT_CONOBJECT( AssetQuery );

//-----------------------------------------------------------------------------

void AssetQuery::initPersistFields()
{
    // Call parent.
    Parent::initPersistFields();

    addProtectedField("Count", TypeS32, 0, &defaultProtectedSetFn, &getCount, &writeCount, "Gets the number of results in the asset query.");
}

//-----------------------------------------------------------------------------

void AssetQuery::onTamlCustomWrite( TamlCustomNodes& customNodes )
{
    // Call parent.
    Parent::onTamlCustomWrite( customNodes );

    // Add node.
    TamlCustomNode* pCustomNode = customNodes.addNode( ASSETQUERY_RESULTS_NODE_NAME );

    // Finish if no assets.
    if (mAssetList.size() == 0)
        return;

    // Iterate asset.
    for (Vector<StringTableEntry>::iterator assetItr = mAssetList.begin(); assetItr != mAssetList.end(); ++assetItr)
    {
        // Add asset node.
        TamlCustomNode* pAssetNode = pCustomNode->addNode( ASSETQUERY_ASSETNODE_NAME );

        // Add field.
        pAssetNode->addField( ASSETQUERY_ASSETID_FIELD_NAME, *assetItr );
    }
}

//-----------------------------------------------------------------------------

void AssetQuery::onTamlCustomRead( const TamlCustomNodes& customNodes )
{
    // Call parent.
    Parent::onTamlCustomRead( customNodes );

    // Find custom node name.
    const TamlCustomNode* pResultsNode = customNodes.findNode( ASSETQUERY_RESULTS_NODE_NAME );

    // Finish if we don't have a results name.
    if ( pResultsNode == NULL )
        return;

    // Fetch node name.
    StringTableEntry assetNodeName = StringTable->insert( ASSETQUERY_ASSETNODE_NAME );

    // Fetch children asset nodes.
    const TamlCustomNodeVector& assetNodes = pResultsNode->getChildren();

    // Iterate asset nodes.
    for( TamlCustomNodeVector::const_iterator assetNodeItr = assetNodes.begin(); assetNodeItr != assetNodes.end(); ++assetNodeItr )
    {
        // Fetch asset node.
        const TamlCustomNode* pAssetNode = *assetNodeItr;

        // Skip if an unknown node name.
        if ( pAssetNode->getNodeName() != assetNodeName )
            continue;

        // Fetch field.
        const TamlCustomField* pField = pAssetNode->findField( ASSETQUERY_ASSETID_FIELD_NAME );

        // Do we find the field?
        if ( pField == NULL )
        {
            // No, so warn.
            Con::warnf( "AssetQuery::onTamlCustomRead() - Could not find '%s' field.", ASSETQUERY_ASSETID_FIELD_NAME );
            continue;
        }

        // Store asset.
        mAssetList.push_back(pField->getFieldValue());
    }
}