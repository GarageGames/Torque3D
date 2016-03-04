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
#define _ASSET_QUERY_H_

#ifndef _SIMBASE_H_
#include "console/simBase.h"
#endif

#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif

#ifndef _STRINGUNIT_H_
#include "core/strings/stringUnit.h"
#endif

#ifndef _TAML_CUSTOM_H_
#include "persistence/taml/tamlCustom.h"
#endif

//-----------------------------------------------------------------------------

#define ASSETQUERY_RESULTS_NODE_NAME     "Results"
#define ASSETQUERY_ASSETNODE_NAME        "Asset"
#define ASSETQUERY_ASSETID_FIELD_NAME   "AssetId"

//-----------------------------------------------------------------------------

class AssetQuery : public SimObject 
{
private:
    typedef SimObject Parent;

protected:
    virtual void onTamlCustomWrite( TamlCustomNodes& customNodes );
    virtual void onTamlCustomRead( const TamlCustomNodes& customNodes );

    static const char* getCount(void* obj, const char* data) { return Con::getIntArg(static_cast<AssetQuery*>(obj)->mAssetList.size()); }
    static bool writeCount( void* obj, StringTableEntry pFieldName ) { return false; }

public:
    AssetQuery() {}
    virtual ~AssetQuery() {}

    /// SimObject overrides
    static void initPersistFields();

    Vector<StringTableEntry> mAssetList;

    /// Whether asset is contained or not.
    inline bool containsAsset( StringTableEntry assetId )
    {
       for (Vector<StringTableEntry>::const_iterator assetItr = mAssetList.begin(); assetItr != mAssetList.end(); ++assetItr)
        {
            if ( *assetItr == assetId )
                return true;
        }
        return false;
    }

    /// Set assets.
    inline void set( const AssetQuery& assetQuery ) { this->mAssetList = assetQuery.mAssetList; }

    /// Declare Console Object.
    DECLARE_CONOBJECT( AssetQuery );
};

#endif // _ASSET_QUERY_H_

