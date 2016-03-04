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
#define _ASSET_TAGS_MANIFEST_H_

#ifndef _SIMBASE_H_
#include "console/simBase.h"
#endif

#ifndef _TDICTIONARY_H_
#include "core/util/tDictionary.h"
#endif

#ifndef _TVECTOR_H_
#include "core/util/tvector.h"
#endif

#ifndef _STRINGUNIT_H_
#include "core/strings/stringUnit.h"
#endif

//-----------------------------------------------------------------------------

#define ASSETTAGS_TAGS_NODE_NAME                "Tags"
#define ASSETTAGS_TAGS_TYPE_NAME                "Tag"
#define ASSETTAGS_TAGS_NAME_FIELD               "Name"

#define ASSETTAGS_ASSETS_NODE_NAME              "Assets"
#define ASSETTAGS_ASSETS_TYPE_NAME              "Tag"
#define ASSETTAGS_ASSETS_ASSETID_FIELD          "AssetId"
#define ASSETTAGS_ASSETS_TAG_FIELD              "Name"

//-----------------------------------------------------------------------------

class AssetManager;

//-----------------------------------------------------------------------------

class AssetTagsManifest : public SimObject
{
    friend class AssetManager;

private:
    typedef SimObject Parent;
    typedef StringTableEntry typeAssetId;
    typedef StringTableEntry typeAssetTagName;

public:
    /// Asset location.
    class AssetTag
    {
    public:
        AssetTag( StringTableEntry tagName )
        {
            // Sanity!
            AssertFatal( tagName != NULL, "Cannot use a NULL tag name." );

            // Case sensitive tag name.
            mTagName = tagName;
        }

        bool containsAsset( typeAssetId assetId )
        {
            for ( Vector<typeAssetId>::iterator assetIdItr = mAssets.begin(); assetIdItr != mAssets.end(); ++assetIdItr )
            {
                if ( *assetIdItr == assetId )
                    return true;
            }

            return false;
        }

        void removeAsset( typeAssetId assetId )
        {
            for ( Vector<typeAssetId>::iterator assetIdItr = mAssets.begin(); assetIdItr != mAssets.end(); ++assetIdItr )
            {
                if ( *assetIdItr == assetId )
                {
                    mAssets.erase( assetIdItr );
                    return;
                }
            }
        }

        typeAssetTagName mTagName;
        Vector<typeAssetId> mAssets;
    };

    /// Asset/Tag database.
    typedef HashMap<typeAssetTagName, AssetTag*> typeTagNameHash;
    typedef HashTable<typeAssetId, AssetTag*> typeAssetToTagHash;

private:
    typeTagNameHash mTagNameDatabase;
    typeAssetToTagHash mAssetToTagDatabase;

private:
    StringTableEntry fetchTagName( const char* pTagName );
    AssetTag* findAssetTag( const char* pTagName );
    void renameAssetId( const char* pAssetIdFrom, const char* pAssetIdTo );

protected:
    virtual void onTamlCustomWrite( TamlCustomNodes& customNodes );
    virtual void onTamlCustomRead( const TamlCustomNodes& customNodes );

public:
    AssetTagsManifest();
    virtual ~AssetTagsManifest();

    /// Tagging.
    const AssetTag* createTag( const char* pTagName );
    bool renameTag( const char* pOldTagName, const char* pNewTagName );
    bool deleteTag( const char* pTagName );
    bool isTag( const char* pTagName );
    inline U32 getTagCount( void ) const { return (U32)mTagNameDatabase.size(); }
    StringTableEntry getTag( const U32 tagIndex );
    U32 getAssetTagCount( const char* pAssetId );
    StringTableEntry getAssetTag( const char* pAssetId, const U32 tagIndex );
    bool tag( const char* pAssetId, const char* pTagName );
    bool untag( const char* pAssetId, const char* pTagName );
    bool hasTag( const char* pAssetId, const char* pTagName );

    /// Declare Console Object.
    DECLARE_CONOBJECT( AssetTagsManifest );
};

#endif // _ASSET_TAGS_MANIFEST_H_

