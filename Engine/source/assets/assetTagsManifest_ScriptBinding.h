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
#include "console/engineAPI.h"
#include "assetTagsManifest.h"

DefineEngineMethod(AssetTagsManifest, createTag, void, (const char* tagName), (""),
   "Creates an asset tag.\n"
   "@param tagName The tag name to create.\n"
   "@return No return value.\n")
{
   object->createTag(tagName);
}

//-----------------------------------------------------------------------------

DefineEngineMethod(AssetTagsManifest, renameTag, bool, (const char* oldTagName, const char* newTagName),,
   "Renames an existing asset tag.\n"
   "@param tagName The tag name to rename.\n"
   "@param newTagName The new tag name to assign.\n"
   "@return Whether the asset tag was renamed or not.\n")
{
   return object->renameTag(oldTagName, newTagName);
}

//-----------------------------------------------------------------------------

DefineEngineMethod(AssetTagsManifest, deleteTag, bool, (const char* tagName),,
   "Deletes an asset tag.\n"
   "@param tagName The tag name to delete.\n"
   "@return Whether the asset tag was deleted or not.\n")
{
   return object->deleteTag(tagName);
}

//-----------------------------------------------------------------------------

DefineEngineMethod(AssetTagsManifest, isTag, bool, (const char* tagName),,
   "Checks whether the specified asset tag exists or not.\n"
   "@param tagName The tag name to check.\n"
   "@return Whether the specified asset tag exists or not.\n")
{
   return object->isTag(tagName);
}

//-----------------------------------------------------------------------------

DefineEngineMethod(AssetTagsManifest, getTagCount, S32, (),,
   "Gets the total asset tag count.\n"
   "@return The total asset tag count.\n")
{
    return object->getTagCount();
}

//-----------------------------------------------------------------------------

DefineEngineMethod(AssetTagsManifest, getTag, const char*, (S32 tagIndex),,
   "Gets the asset tag at the specified index.\n"
   "@param tagIndex The asset tag index.This must be 0 to the asset tag count less one.\n"
   "@return The asset tag at the specified index or NULL if invalid.\n")
{
    // Is the tag index out-of-bounds?
    if ( tagIndex >= object->getTagCount() )
    {
        // Yes, so warn.
        Con::warnf( "AssetTagsManifest: Asset tag index '%d' is out of bounds.  Asset tag count is '%d'", tagIndex, object->getTagCount() );
        return StringTable->EmptyString();
    }

    return object->getTag( tagIndex );
}

//-----------------------------------------------------------------------------

DefineEngineMethod(AssetTagsManifest, getAssetTagCount, S32, (const char* assetId),,
   "Gets the asset tag count on the specified asset Id.\n"
   "@param assetId The asset Id to count tags on.\n"
   "@return The asset tag count on the specified asset Id.\n")
{
   return object->getAssetTagCount(assetId);
}

//-----------------------------------------------------------------------------

DefineEngineMethod(AssetTagsManifest, getAssetTag, const char*, (const char* assetId, S32 tagIndex), ,
   "Gets the asset tag on the specified asset Id at the specified index.\n"
   "@param assetId The asset Id to count tags on.\n"
   "@param tagIndex The asset tag index.This must be 0 to the asset tag count less one.\n"
   "@return The asset tag on the specified asset Id at the specified index or NULL if invalid.\n")
{
   // Is the tag index out-of-bounds?
   if (tagIndex >= object->getAssetTagCount(assetId))
   {
      // Yes, so warn.
      Con::warnf("AssetTagsManifest: Asset tag index '%d' is out of bounds.  Asset tag count is '%d'", tagIndex, object->getAssetTagCount(assetId));
      return StringTable->EmptyString();
   }

   return object->getAssetTag(assetId, tagIndex);
}

//-----------------------------------------------------------------------------

DefineEngineMethod(AssetTagsManifest, tag, bool, (const char* assetId, const char* tagName),,
   "Tags the asset Id with the specified asset tag.\n"
   "@param assetId The asset Id to tag.\n"
   "@param tagName The tag name to assign.\n"
   "@return Whether the tag operation was successful or not.\n")
{
   return object->tag(assetId, tagName);
}

//-----------------------------------------------------------------------------

DefineEngineMethod(AssetTagsManifest, untag, bool, (const char* assetId, const char* tagName),,
   "Un-tags the asset Id from the specified asset tag.\n"
   "@param assetId The asset Id to un - tag.\n"
   "@param tagName The tag name to un - assign.\n"
   "@return Whether the un - tag operation was successful or not.\n")
{
   return object->untag(assetId, tagName);
}

//-----------------------------------------------------------------------------

DefineEngineMethod(AssetTagsManifest, hasTag, bool, (const char* assetId, const char* tagName), ,
   "Checks whether the asset Id is tagged with the specified asset tag.\n"
   "@param assetId The asset Id to check.\n"
   "@param tagName The tag name to check.\n"
   "@return Whether the asset Id is tagged with the specified asset tag or not.\n")
{
   return object->hasTag(assetId, tagName);
}
