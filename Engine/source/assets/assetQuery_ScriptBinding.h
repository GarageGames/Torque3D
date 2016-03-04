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
#include "assets/assetQuery.h"
#endif

#include "console/engineAPI.h"

DefineConsoleMethod(AssetQuery, clear, void, (),,"Clears all asset Id results."
   "Clears all asset Id results.\n"
   "@return () No return value.\n")
{
   object->mAssetList.clear();
}

//-----------------------------------------------------------------------------


DefineConsoleMethod(AssetQuery, set, bool, (S32 queryId), ,
   "Sets the asset query to a copy of the specified asset query.\n"
   "@param assetQuery The asset query to copy.\n"
   "@return Whether the operation succeeded or not.\n")
{
    // Find asset query.
    AssetQuery* pAssetQuery;

    if (!Sim::findObject(queryId, pAssetQuery))
    {
       // No, so warn.
       Con::warnf("AssetQuery::set() - Could not find asset query '%i'.", queryId);
       return false;
    }

    // Set asset query.
    object->set( *pAssetQuery );

    return true;
}

//-----------------------------------------------------------------------------

DefineConsoleMethod(AssetQuery, getCount, S32, (), , 
   "Gets the count of asset Id results.\n"
   "@return (int)The count of asset Id results.\n")
{
   return object->mAssetList.size();
}

//-----------------------------------------------------------------------------

DefineConsoleMethod(AssetQuery, getAsset, const char*, (S32 resultIndex), (-1), 
   "Gets the asset Id at the specified query result index.\n"
   "@param resultIndex The query result index to use.\n"
   "@return (assetId)The asset Id at the specified index or NULL if not valid.\n")
{
    // Is index within bounds?
   if (resultIndex >= object->mAssetList.size())
    {
        // No, so warn.
        Con::warnf("AssetQuery::getAsset() - Result index '%s' is out of bounds.", resultIndex);
        return StringTable->EmptyString();
    }

    return object->mAssetList[resultIndex];
}
