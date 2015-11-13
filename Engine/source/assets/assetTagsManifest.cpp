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

#ifndef _ASSET_TAGS_MANIFEST_H_
#include "assetTagsManifest.h"
#endif

#ifndef _ASSET_MANAGER_H_
#include "assetManager.h"
#endif

#ifndef _CONSOLETYPES_H_
#include "console/consoleTypes.h"
#endif

// Script binding.
#include "assetTagsManifest_ScriptBinding.h"

//-----------------------------------------------------------------------------

IMPLEMENT_CONOBJECT( AssetTagsManifest );

//-----------------------------------------------------------------------------

AssetTagsManifest::AssetTagsManifest()
{
}

//-----------------------------------------------------------------------------

AssetTagsManifest::~AssetTagsManifest()
{
    // Iterate tags.
    for( typeTagNameHash::iterator tagNameItr = mTagNameDatabase.begin(); tagNameItr != mTagNameDatabase.end(); ++tagNameItr )
    {
        // Delete asset tag.
        delete tagNameItr->value;
    }

    // Clear database.
    mTagNameDatabase.clear();
    mAssetToTagDatabase.clear();
}

//-----------------------------------------------------------------------------

StringTableEntry AssetTagsManifest::fetchTagName( const char* pTagName )
{
    // Sanity!
    AssertFatal( pTagName != NULL, "Cannot use a NULL tag name." );

    return StringTable->insert( pTagName, true );
}

//-----------------------------------------------------------------------------

AssetTagsManifest::AssetTag* AssetTagsManifest::findAssetTag( const char* pTagName )
{
    // Fetch tag name.
    StringTableEntry tagName = fetchTagName( pTagName );

    // Finish if the tag already exists.
    typeTagNameHash::iterator tagNameItr = mTagNameDatabase.find( tagName );

    return tagNameItr == mTagNameDatabase.end() ? NULL : tagNameItr->value;
}

//-----------------------------------------------------------------------------

void AssetTagsManifest::renameAssetId( const char* pAssetIdFrom, const char* pAssetIdTo )
{
    // Sanity!
    AssertFatal( pAssetIdFrom != NULL, "Cannot rename from NULL asset Id." );
    AssertFatal( pAssetIdTo != NULL, "Cannot rename to NULL asset Id." );

    // Fetch asset Ids.
    StringTableEntry assetIdFrom = StringTable->insert( pAssetIdFrom );
    StringTableEntry assetIdTo   = StringTable->insert( pAssetIdTo );

    while( true )
    {
        // Find asset Id.
        typeAssetToTagHash::Iterator assetIdItr = mAssetToTagDatabase.find( assetIdFrom );

        // Finish if no asset Id to rename.
        if ( assetIdItr == mAssetToTagDatabase.end() )
            return;

        // Fetch asset tag.
        AssetTag* pAssetTag = assetIdItr->value;

        // Untag old asset Id.
        untag( assetIdFrom, pAssetTag->mTagName );

        // Tag new asset Id.
        tag( assetIdTo, pAssetTag->mTagName );
    }
}

//-----------------------------------------------------------------------------

void AssetTagsManifest::onTamlCustomWrite( TamlCustomNodes& customNodes )
{
    // Call parent.
    Parent::onTamlCustomWrite( customNodes );

    // Finish if no tags.
    if ( mTagNameDatabase.size() == 0 )
        return;

    // Add node.
    TamlCustomNode* pTagsNode = customNodes.addNode( ASSETTAGS_TAGS_NODE_NAME );

    // Iterate tags.
    for( typeTagNameHash::iterator tagItr = mTagNameDatabase.begin(); tagItr != mTagNameDatabase.end(); ++tagItr )
    {
        // Add tag node.
        TamlCustomNode* pTagNode = pTagsNode->addNode( ASSETTAGS_TAGS_TYPE_NAME );

        // Add fields.
        pTagNode->addField( ASSETTAGS_TAGS_NAME_FIELD, tagItr->key );
    }

    // Add node.
    TamlCustomNode* pAssetTagsNode = customNodes.addNode( ASSETTAGS_ASSETS_NODE_NAME );

    // Iterate asset locations.
    for( typeAssetToTagHash::Iterator assetTagItr = mAssetToTagDatabase.begin(); assetTagItr != mAssetToTagDatabase.end(); ++assetTagItr )
    {
        // Add asset tag node.
        TamlCustomNode* pAssetNode = pAssetTagsNode->addNode( ASSETTAGS_ASSETS_TYPE_NAME );

        // Add fields.
        pAssetNode->addField( ASSETTAGS_ASSETS_ASSETID_FIELD, assetTagItr->key );
        pAssetNode->addField( ASSETTAGS_ASSETS_TAG_FIELD, assetTagItr->value->mTagName );
    }
}

//-----------------------------------------------------------------------------

void AssetTagsManifest::onTamlCustomRead( const TamlCustomNodes& customNodes )
{
    // Call parent.
    Parent::onTamlCustomRead( customNodes );

    // Find tags nodes.
    const TamlCustomNode* pTagProperty = customNodes.findNode( ASSETTAGS_TAGS_NODE_NAME );

    // Finish if we don't have a tags node name.
    if ( pTagProperty == NULL )
        return;

    // Fetch node name.
    StringTableEntry tagNodeName = StringTable->insert( ASSETTAGS_TAGS_TYPE_NAME );

    // Fetch children asset nodes.
    const TamlCustomNodeVector& tagNodes = pTagProperty->getChildren();

    // Iterate tag nodes.
    for( TamlCustomNodeVector::const_iterator tagNodeItr = tagNodes.begin(); tagNodeItr != tagNodes.end(); ++tagNodeItr )
    {
        // Fetch tag node.
        const TamlCustomNode* pTagNode = *tagNodeItr;

        // Skip if an unknown node name.
        if ( pTagNode->getNodeName() != tagNodeName )
            continue;

        // Fetch "Name" field.
        const TamlCustomField* pTagNameField = pTagNode->findField( ASSETTAGS_TAGS_NAME_FIELD );

        // Do we find the field?
        if ( pTagNameField == NULL )
        {
            // No, so warn.
            Con::warnf( "AssetTagsManifest::onTamlCustomRead() - Could not find '%s' field.", ASSETTAGS_TAGS_NAME_FIELD );
            continue;
        }

        // Create the tag.
        createTag( pTagNameField->getFieldValue() );
    }

    // Find asset tags node.
    const TamlCustomNode* pAssetTagProperty = customNodes.findNode( ASSETTAGS_ASSETS_NODE_NAME );

    // Finish if we don't have an asset tags node name.
    if ( pAssetTagProperty == NULL )
        return;

    // Fetch node name.
    StringTableEntry assetTagNodeName = StringTable->insert( ASSETTAGS_ASSETS_TYPE_NAME );

    // Fetch children asset tag nodes.
    const TamlCustomNodeVector& assetTagNodes = pAssetTagProperty->getChildren();

    // Iterate property alias.
    for( TamlCustomNodeVector::const_iterator assetTagNodeItr = assetTagNodes.begin(); assetTagNodeItr != assetTagNodes.end(); ++assetTagNodeItr )
    {
        // Fetch asset node.
        const TamlCustomNode* pAssetTagNode = *assetTagNodeItr;

        // Skip if an unknown node name.
        if ( pAssetTagNode->getNodeName() != assetTagNodeName )
            continue;

        // Fetch "AssetId" field.
        const TamlCustomField* pAssetIdField = pAssetTagNode->findField( ASSETTAGS_ASSETS_ASSETID_FIELD );

        // Do we find the field?
        if ( pAssetIdField == NULL )
        {
            // No, so warn.
            Con::warnf( "AssetTagsManifest::onTamlCustomRead() - Could not find '%s' field.", ASSETTAGS_ASSETS_ASSETID_FIELD );
            continue;
        }

        // Fetch "Tag" field.
        const TamlCustomField* pTagField = pAssetTagNode->findField( ASSETTAGS_ASSETS_TAG_FIELD );

        // Do we find the field?
        if ( pTagField == NULL )
        {
            // No, so warn.
            Con::warnf( "AssetTagsManifest::onTamlCustomRead() - Could not find '%s' field.", ASSETTAGS_ASSETS_TAG_FIELD );
            continue;
        }

        // Tag the asset.
        tag( pAssetIdField->getFieldValue(), pTagField->getFieldValue() );
    }
}

//-----------------------------------------------------------------------------

const AssetTagsManifest::AssetTag* AssetTagsManifest::createTag( const char* pTagName )
{
    // Sanity!
    AssertFatal( pTagName != NULL, "Cannot use a NULL tag name." );

    // Finish if the tag already exists.
    AssetTag* pAssetTag = findAssetTag( pTagName );

    // Return asset tag if already created.
    if ( pAssetTag != NULL )
        return pAssetTag;

    // Fetch tag name.
    StringTableEntry tagName = fetchTagName( pTagName );

    // Generate the asset tag.
    pAssetTag = new AssetTag( tagName );

    // Add the tag.
    mTagNameDatabase.insert( tagName, pAssetTag );

    return pAssetTag;
}

//-----------------------------------------------------------------------------

bool AssetTagsManifest::renameTag( const char* pOldTagName, const char* pNewTagName )
{
    // Sanity!
    AssertFatal( pOldTagName != NULL, "Cannot use a NULL tag name." );
    AssertFatal( pNewTagName != NULL, "Cannot use a NULL tag name." );

    // Find old asset tags.
    AssetTag* pOldAssetTag = findAssetTag( pOldTagName );

    // Did we find the asset tag?
    if ( pOldAssetTag == NULL )
    {
        // No, so warn.
        Con::warnf( "AssetTagsManifest: Cannot rename tag '%s' as it does not exist.", pOldTagName );
        return false;
    }

    // Are the old and new tags the same?
    if ( pOldAssetTag->mTagName == fetchTagName( pNewTagName ) )
    {
        // Yes, so warn.
        Con::warnf( "AssetTagsManifest: Cannot rename tag '%s' to tag '%s' as they are the same.", pOldTagName, pNewTagName );
        return false;
    }

    // Create new tag.
    AssetTag* pNewAssetTag = const_cast<AssetTag*>( createTag( pNewTagName ) );

    // Transfer asset tags.
    for ( Vector<typeAssetId>::iterator assetIdItr = pOldAssetTag->mAssets.begin(); assetIdItr != pOldAssetTag->mAssets.end(); ++assetIdItr )
    {
        pNewAssetTag->mAssets.push_back( *assetIdItr );
    }

    // Delete old tag.
    deleteTag( pOldTagName );

    return true;
}

//-----------------------------------------------------------------------------

bool AssetTagsManifest::deleteTag( const char* pTagName )
{
    // Sanity!
    AssertFatal( pTagName != NULL, "Cannot use a NULL tag name." );

    // Find asset tag.
    AssetTag* pAssetTag = findAssetTag( pTagName );

    // Did we find the asset tag?
    if ( pAssetTag == NULL )
    {
        // No, so warn.
        Con::warnf( "AssetTagsManifest: Cannot delete tag '%s' as it does not exist.", pTagName );
        return false;
    }

    // Remove the tag.
    mTagNameDatabase.erase( pAssetTag->mTagName );

    // Remove the asset tags.
    for ( Vector<typeAssetId>::iterator assetIdItr = pAssetTag->mAssets.begin(); assetIdItr != pAssetTag->mAssets.end(); ++assetIdItr )
    {
        // Find asset Id tag.
        typeAssetToTagHash::Iterator assetItr = mAssetToTagDatabase.find( *assetIdItr );

        // Iterate asset Id tags.
        while( assetItr != mAssetToTagDatabase.end() && assetItr->key == *assetIdItr )
        {
            // Is this the asset tag?
            if ( assetItr->value == pAssetTag )
            {
                // Yes, so erase it.
                mAssetToTagDatabase.erase( assetItr );
                break;
            }

            // Next entry.
            assetItr++;
        }
    }

    // Delete the asset tag.
    delete pAssetTag;

    return true;
}

//-----------------------------------------------------------------------------

bool AssetTagsManifest::isTag( const char* pTagName )
{
    // Sanity!
    AssertFatal( pTagName != NULL, "Cannot use a NULL tag name." );

    // Check whether tag exists or not.
    return findAssetTag( pTagName ) != NULL;
}

//-----------------------------------------------------------------------------

StringTableEntry AssetTagsManifest::getTag( const U32 tagIndex )
{
    // Finish if invalid tag count.
    AssertFatal( tagIndex < getTagCount(), "Tag index is out of bounds." );

    // Fetch tag iterator.
    typeTagNameHash::iterator tagItr = mTagNameDatabase.begin();

    // Find asset tag by index.
    // NOTE: Unfortunately this is slow as it's not the natural access method.
    for( U32 index = 0; index  < tagIndex; ++index, ++tagItr ) {};

    // Return tag name.
    return tagItr->value->mTagName;
}

//-----------------------------------------------------------------------------

U32 AssetTagsManifest::getAssetTagCount( const char* pAssetId )
{
    // Sanity!
    AssertFatal( pAssetId != NULL, "Cannot use a NULL asset Id." );

    // Fetch asset Id.
    StringTableEntry assetId = StringTable->insert( pAssetId );

    return (U32)mAssetToTagDatabase.count( assetId );
}

//-----------------------------------------------------------------------------

StringTableEntry AssetTagsManifest::getAssetTag( const char* pAssetId, const U32 tagIndex )
{
    // Sanity!
    AssertFatal( pAssetId != NULL, "Cannot use a NULL asset Id." );

    // Fetch asset Id.
    StringTableEntry assetId = StringTable->insert( pAssetId );

    // Sanity!
    AssertFatal( tagIndex < (U32)mAssetToTagDatabase.count( assetId ), "Asset tag index is out of bounds." );

    // Find asset tag.
    typeAssetToTagHash::Iterator assetItr = mAssetToTagDatabase.find( assetId );

    // Find asset tag by index.
    // NOTE: Unfortunately this is slow as it's not the natural access method.
    for( U32 index = 0; index  < tagIndex; ++index, ++assetItr ) {};

    // Return tag name.
    return assetItr->value->mTagName;
}

//-----------------------------------------------------------------------------

bool AssetTagsManifest::tag( const char* pAssetId, const char* pTagName )
{
    // Sanity!
    AssertFatal( pAssetId != NULL, "Cannot use a NULL asset Id." );
    AssertFatal( pTagName != NULL, "Cannot use a NULL tag name." );

    // Find asset tag.
    AssetTag* pAssetTag = findAssetTag( pTagName );

    // Does the tag exist?
    if ( pAssetTag == NULL )
    {
        // No, so warn.
        Con::warnf("AssetTagsManifest: Cannot tag asset Id '%s' with tag name '%s' as tag name does not exist.", pAssetId, pTagName );
        return false;
    }

    // Is the asset Id valid?
    if ( !AssetDatabase.isDeclaredAsset( pAssetId ) )
    {
        // No, so warn.
        Con::warnf( "AssetTagsManifest::onTamlCustomRead() - Ignoring asset Id '%s' mapped to tag name '%s' as it is not a declared asset Id.", pAssetId, pTagName );
        return false;
    }

    // Fetch asset Id.
    typeAssetId assetId = StringTable->insert( pAssetId );

    // Find asset Id tag.
    typeAssetToTagHash::Iterator assetItr = mAssetToTagDatabase.find(assetId);

    // Iterate asset Id tags.
    while( assetItr != mAssetToTagDatabase.end() && assetItr->key == assetId )
    {
        // Is the asset already tagged appropriately?
        if ( assetItr->value == pAssetTag )
            return true;

        // Next entry.
        assetItr++;
    }

    // No, so add tag.
    mAssetToTagDatabase.insertEqual( assetId, pAssetTag );    

    // Add to asset tag.
    pAssetTag->mAssets.push_back( assetId );

    return true;
}

//-----------------------------------------------------------------------------

bool AssetTagsManifest::untag( const char* pAssetId, const char* pTagName )
{
    // Sanity!
    AssertFatal( pAssetId != NULL, "Cannot use a NULL asset Id." );
    AssertFatal( pTagName != NULL, "Cannot use a NULL tag name." );

    // Find asset tag.
    AssetTag* pAssetTag = findAssetTag( pTagName );

    // Does the tag exist?
    if ( pAssetTag == NULL )
    {
        // No, so warn.
        Con::warnf("AssetTagsManifest: Cannot untag asset Id '%s' from tag name '%s' as tag name does not exist.", pAssetId, pTagName );
        return false;
    }

    // Fetch asset Id.
    typeAssetId assetId = StringTable->insert( pAssetId );

    // Is the asset tagged?
    if ( !pAssetTag->containsAsset( assetId ) )
    {
        // No, so warn.
        Con::warnf("AssetTagsManifest: Cannot untag asset Id '%s' from tag name '%s' as the asset is not tagged with the tag name.", pAssetId, pTagName );
        return false;
    }

    // Remove asset from assert tag.
    pAssetTag->removeAsset( assetId );

    // Find asset Id tag.
    typeAssetToTagHash::Iterator assetItr = mAssetToTagDatabase.find(assetId);

    // Iterate asset Id tags.
    while( assetItr != mAssetToTagDatabase.end() && assetItr->key == assetId )
    {
        // Is this the asset tag?
        if ( assetItr->value == pAssetTag )
        {
            // Yes, so remove it.
            mAssetToTagDatabase.erase( assetItr );
            break;
        }

        // Next entry.
        assetItr++;
    }

    return true;
}

//-----------------------------------------------------------------------------

bool AssetTagsManifest::hasTag( const char* pAssetId, const char* pTagName )
{
    // Sanity!
    AssertFatal( pAssetId != NULL, "Cannot use a NULL asset Id." );
    AssertFatal( pTagName != NULL, "Cannot use a NULL tag name." );

    // Find asset tag.
    AssetTag* pAssetTag = findAssetTag( pTagName );

    // Does the tag exist?
    if ( pAssetTag == NULL )
    {
        // No, so warn.
        Con::warnf("AssetTagsManifest: Cannot check if asset Id '%s' has tag name '%s' as tag name does not exist.", pAssetId, pTagName );
        return false;
    }

    // Return whether asset Id is tagged.
    return pAssetTag->containsAsset( StringTable->insert( pAssetId ) );
}
