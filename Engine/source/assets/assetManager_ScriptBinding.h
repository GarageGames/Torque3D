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
#include "assetBase.h"
#include "assetManager.h"
#include "module/moduleDefinition.h"
#include "console/sim.h"

DefineEngineMethod(AssetManager, compileReferencedAssets, bool, (const char* moduleDefinition), (""),
   "Compile the referenced assets determined by the specified module definition.\n"
   "@param moduleDefinition The module definition specifies the asset manifest.\n"
   "@return Whether the compilation was successful or not.\n")
{
    // Fetch module definition.
   ModuleDefinition* pModuleDefinition;
   Sim::findObject(moduleDefinition, pModuleDefinition);

    // Did we find the module definition?
    if ( pModuleDefinition == NULL )
    {
        // No, so warn.
       Con::warnf("AssetManager::compileReferencedAssets() - Could not find the module definition '%s'.", moduleDefinition);
        return false;
    }

    // Compile referenced assets.
    return object->compileReferencedAssets( pModuleDefinition );
}

//-----------------------------------------------------------------------------

DefineEngineMethod(AssetManager, addModuleDeclaredAssets, bool, (const char* moduleDefinition), (""),
   "Add any the declared assets specified by the module definition.\n"
   "@param moduleDefinition The module definition specifies the asset manifest.\n"
   "@return Whether adding declared assets was successful or not.\n")
{
   // Fetch module definition.
   ModuleDefinition* pModuleDefinition;
   Sim::findObject(moduleDefinition, pModuleDefinition);

    // Did we find the module definition?
    if ( pModuleDefinition == NULL )
    {
        // No, so warn.
       Con::warnf("AssetManager::addDeclaredAssets() - Could not find the module definition '%s'.", moduleDefinition);
        return false;
    }

    // Add module declared assets.
    return object->addModuleDeclaredAssets( pModuleDefinition );
}

//-----------------------------------------------------------------------------

DefineEngineMethod(AssetManager, addDeclaredAsset, bool, (const char* moduleDefinition, const char* assetFilePath), ("", ""),
   "Add the specified asset against the specified module definition.\n"
   "@param moduleDefinition The module definition that may contain declared assets.\n"
   "@return Whether adding declared assets was successful or not.\n")
{
   // Fetch module definition.
   ModuleDefinition* pModuleDefinition;
   Sim::findObject(moduleDefinition, pModuleDefinition);

    // Did we find the module definition?
    if ( pModuleDefinition == NULL )
    {
        // No, so warn.
       Con::warnf("AssetManager::addDeclaredAsset() - Could not find the module definition '%s'.", moduleDefinition);
        return false;
    }

    // Fetch asset file-path.
    const char* pAssetFilePath = assetFilePath;

    // Add declared asset.
    return object->addDeclaredAsset( pModuleDefinition, pAssetFilePath );
}

//-----------------------------------------------------------------------------

DefineEngineMethod(AssetManager, addPrivateAsset, String, (const char* assetObject), (""),
   "Adds a private asset object.\n"
   "@param assetObject The asset object to add as a private asset.\n"
   "@return The allocated private asset Id.\n")
{
    // Fetch asset.
   AssetBase* pAssetBase;
   Sim::findObject(assetObject, pAssetBase);

    // Did we find the asset?
    if ( pAssetBase == NULL )
    {
        // No, so warn.
       Con::warnf("AssetManager::addPrivateAsset() - Could not find the asset '%s'.", assetObject);
        return StringTable->EmptyString();
    }

    // Add private asset.
    return object->addPrivateAsset( pAssetBase );
}

//-----------------------------------------------------------------------------

DefineEngineMethod(AssetManager, removeDeclaredAssets, bool, (const char* moduleDefinition), (""),
   "Remove any the declared assets specified by the module definition.\n"
   "@param moduleDefinition The module definition that may contain declared assets.\n"
   "@return Whether removing declared assets was successful or not.\n")
{
    // Fetch module definition.
   ModuleDefinition* pModuleDefinition;
   Sim::findObject(moduleDefinition, pModuleDefinition);

    // Did we find the module definition?
    if ( pModuleDefinition == NULL )
    {
        // No, so warn.
       Con::warnf("AssetManager::removeDeclaredAssets() - Could not find the module definition '%s'.", moduleDefinition);
        return false;
    }

    // Remove declared assets.
    return object->removeDeclaredAssets( pModuleDefinition );
}

//-----------------------------------------------------------------------------

DefineEngineMethod(AssetManager, removeDeclaredAsset, bool, (const char* assetId), (""),
   "Remove the specified declared asset Id.\n"
   "@param assetId The selected asset Id.\n"
   "@return Whether removing the declared asset was successful or not.\n")
{
    // Remove the declared asset Id.
   return object->removeDeclaredAsset(assetId);
}

//-----------------------------------------------------------------------------

DefineEngineMethod(AssetManager, getAssetName, String, (const char* assetId), (""),
   "Gets the asset name from the specified asset Id.\n"
   "@param assetId The selected asset Id.\n"
   "@return The asset name from the specified asset Id.\n")
{
   return object->getAssetName(assetId);
}

//-----------------------------------------------------------------------------

DefineEngineMethod(AssetManager, getAssetDescription, String, (const char* assetId), (""),
   "Gets the asset description from the specified asset Id.\n"
   "@param assetId The selected asset Id.\n"
   "@return The asset description from the specified asset Id.\n")
{
   return object->getAssetDescription(assetId);
}

//-----------------------------------------------------------------------------

DefineEngineMethod(AssetManager, getAssetCategory, String, (const char* assetId), (""),
   "Gets the asset category from the specified asset Id.\n"
   "@param assetId The selected asset Id.\n"
   "@return The asset category from the specified asset Id.\n")
{
   return object->getAssetCategory(assetId);
}

//-----------------------------------------------------------------------------

DefineEngineMethod(AssetManager, getAssetType, String, (const char* assetId), (""),
   "Gets the asset type from the specified asset Id.\n"
   "@param assetId The selected asset Id.\n"
   "@return The asset type from the specified asset Id.\n")
{
   return object->getAssetType(assetId);
}

//-----------------------------------------------------------------------------

DefineEngineMethod(AssetManager, getAssetFilePath, String, (const char* assetId), (""),
   "Gets the asset file-path from the specified asset Id.\n"
   "@param assetId The selected asset Id.\n"
   "@return The asset file - path from the specified asset Id.\n")
{
   return object->getAssetFilePath(assetId);
}

//-----------------------------------------------------------------------------

DefineEngineMethod(AssetManager, getAssetPath, String, (const char* assetId), (""),
   "Gets the asset path (not including the asset file) from the specified asset Id.\n"
   "@param assetId The selected asset Id.\n"
   "@return The asset path(not including the asset file) from the specified asset Id.\n")
{
   return object->getAssetPath(assetId);
}

//-----------------------------------------------------------------------------

DefineEngineMethod(AssetManager, getAssetModule, String, (const char* assetId), (""),
   "Gets the module definition where the the specified asset Id is located.\n"
   "@param assetId The selected asset Id.\n"
   "@return The module definition where the the specified asset Id is located.\n")
{
    // Fetch module definition.
   ModuleDefinition* pModuleDefinition = object->getAssetModuleDefinition(assetId);

    return pModuleDefinition == NULL ? StringTable->EmptyString() : pModuleDefinition->getIdString();
}

//-----------------------------------------------------------------------------

DefineEngineMethod(AssetManager, isAssetInternal, bool, (const char* assetId), (""),
   "Check whether the specified asset Id is internal or not.\n"
   "@param assetId The selected asset Id.\n"
   "@return Whether the specified asset Id is internal or not.\n")
{
   return object->isAssetInternal(assetId);
}

//-----------------------------------------------------------------------------

DefineEngineMethod(AssetManager, isAssetPrivate, bool, (const char* assetId), (""),
   "Check whether the specified asset Id is private or not.\n"
   "@param assetId The selected asset Id.\n"
   "@return Whether the specified asset Id is private or not.\n")
{
   return object->isAssetPrivate(assetId);
}

//-----------------------------------------------------------------------------

DefineEngineMethod(AssetManager, isAssetAutoUnload, bool, (const char* assetId), (""),
   "Check whether the specified asset Id is auto - unload or not.\n"
   "@param assetId The selected asset Id.\n"
   "@return Whether the specified asset Id is auto-unload or not.\n")
{
   return object->isAssetAutoUnload(assetId);
}

//-----------------------------------------------------------------------------

DefineEngineMethod(AssetManager, isAssetLoaded, bool, (const char* assetId), (""),
   "Check whether the specified asset Id is loaded or not.\n"
   "@param assetId The selected asset Id.\n"
   "@return Whether the specified asset Id is loaded or not.\n")
{
   return object->isAssetLoaded(assetId);
}

//-----------------------------------------------------------------------------

DefineEngineMethod(AssetManager, isDeclaredAsset, bool, (const char* assetId), (""),
   "Check whether the specified asset Id is declared or not.\n"
   "@param assetId The selected asset Id.\n"
   "@return Whether the specified asset Id is declared or not.\n")
{
   return object->isDeclaredAsset(assetId);
}

//-----------------------------------------------------------------------------

DefineEngineMethod(AssetManager, isReferencedAsset, bool, (const char* assetId), (""),
   "Check whether the specified asset Id is referenced or not.\n"
   "@param assetId The selected asset Id.\n"
   "@return Whether the specified asset Id is referenced or not.\n")
{
   return object->isReferencedAsset(assetId);
}

//-----------------------------------------------------------------------------

DefineEngineMethod(AssetManager, renameDeclaredAsset, bool, (const char* assetIdFrom, const char* assetIdTo), ("", ""),
   "Rename declared asset Id.\n"
   "@param assetIdFrom The selected asset Id to rename from.\n"
   "@param assetIdFrom The selected asset Id to rename to.\n"
   "@return Whether the rename was successful or not.\n")
{
   return object->renameDeclaredAsset(assetIdFrom, assetIdTo);
}

//-----------------------------------------------------------------------------

DefineEngineMethod(AssetManager, renameReferencedAsset, bool, (const char* assetIdFrom, const char* assetIdTo), ("", ""),
   "Rename referenced asset Id. \n"
   "@param assetIdFrom The selected asset Id to rename from.\n"
   "@param assetIdFrom The selected asset Id to rename to.\n"
   "@return Whether the rename was successful or not.\n")
{
   return object->renameReferencedAsset(assetIdFrom, assetIdTo);
}

//-----------------------------------------------------------------------------

DefineEngineMethod(AssetManager, acquireAsset, String, (const char* assetId, bool asPrivate), ("", false),
   "Acquire the specified asset Id.\n"
   "You must release the asset once you're finish with it using 'releaseAsset'.\n"
   "@param assetId The selected asset Id.\n"
   "@param asPrivate Whether to acquire the asset Id as a private asset.\n"
   "@return The acquired asset or NULL if not acquired.\n")
{
    // Fetch asset Id.
   const char* pAssetId = assetId;

    // Reset asset reference.
    AssetBase* pAssetBase = NULL;

    // Acquire private asset?
    if ( asPrivate )
    {
        // Acquire private asset.
        pAssetBase = object->acquireAsPrivateAsset<AssetBase>( pAssetId );
    }
    else
    {
        // Acquire public asset.
        pAssetBase = object->acquireAsset<AssetBase>( pAssetId );
    }

    return pAssetBase != NULL ? pAssetBase->getIdString() : StringTable->EmptyString();
}

//-----------------------------------------------------------------------------

DefineEngineMethod(AssetManager, releaseAsset, bool, (const char* assetId), (""),
   "Release the specified asset Id.\n"
   "The asset should have been acquired using 'acquireAsset'.\n"
   "@param assetId The selected asset Id.\n"
   "@return Whether the asset was released or not.\n")
{
    // Release asset.
   return object->releaseAsset(assetId);
}

//-----------------------------------------------------------------------------

DefineEngineMethod(AssetManager, purgeAssets, void, (),,
   "Purge all assets that are not referenced even if they are set to not auto-unload.\n"
   "Assets can be in this state because they are either set to not auto-unload or the asset manager has/is disabling auto-unload.\n"
   "@return No return value.\n")
{
    // Purge assets.
    object->purgeAssets();
}

//-----------------------------------------------------------------------------

DefineEngineMethod(AssetManager, deleteAsset, bool, (const char* assetId, bool deleteLooseFiles, bool deleteDependencies), ("", false, false),
   "Deletes the specified asset Id and optionally its loose files and asset dependencies.\n"
   "@param assetId The selected asset Id.\n"
   "@param deleteLooseFiles Whether to delete an assets loose files or not.\n"
   "@param deleteDependencies Whether to delete assets that depend on this asset or not.\n"
   "@return Whether the asset deletion was successful or not.  A failure only indicates that the specified asset was not deleted but dependent assets and their loose files may have being deleted.\n")
{
    // Fetch asset Id.
   const char* pAssetId = assetId;

    // Delete asset.
    return object->deleteAsset( pAssetId, deleteLooseFiles, deleteDependencies );
}

//-----------------------------------------------------------------------------

DefineEngineMethod(AssetManager, refreshAsset, void, (const char* assetId), (""),
   "Refresh the specified asset Id.\n"
   "@param assetId The selected asset Id.\n"
   "@return No return value.\n")
{
   object->refreshAsset(assetId);
}

//-----------------------------------------------------------------------------

DefineEngineMethod(AssetManager, refreshAllAssets, void, (bool includeUnloaded), (false),
   "Refresh all declared assets.\n"
   "@param Whether to include currently unloaded assets in the refresh or not.  Optional: Defaults to false.\n"
   "Refreshing all assets can be an expensive (time-consuming) operation to perform.\n"
   "@return No return value.\n")
{
    // Refresh assets
    object->refreshAllAssets(includeUnloaded);
}

//-----------------------------------------------------------------------------

DefineEngineMethod(AssetManager, saveAssetTags, bool, (),,
   "Save the currently loaded asset tags manifest.\n"
   "@return Whether the save was successful or not.\n")
{
    // Save asset tags.
    return object->saveAssetTags();
}

//-----------------------------------------------------------------------------

DefineEngineMethod(AssetManager, restoreAssetTags, bool, (),,
   "Restore the currently loaded asset tags manifest from disk (replace anything in memory).\n"
   "@return Whether the restore was successful or not.\n")
{
    // Restore asset tags.
    return object->restoreAssetTags();
}

//-----------------------------------------------------------------------------

DefineEngineMethod(AssetManager, getAssetTags, S32, (), ,
   "Gets the currently loaded asset tags manifest.\n"
   "@return The currently loaded asset tags manifest or zero if not loaded.\n")
{
    // Fetch the asset tags manifest.
    AssetTagsManifest* pAssetTagsManifest = object->getAssetTags();

    return pAssetTagsManifest == NULL ? 0 : pAssetTagsManifest->getId();
}

//-----------------------------------------------------------------------------

DefineEngineMethod(AssetManager, findAllAssets, S32, (const char* assetQuery, bool ignoreInternal, bool ignorePrivate), ("", true, true),
   "Performs an asset query searching for all assets optionally ignoring internal assets.\n"
   "@param assetQuery The asset query object that will be populated with the results.\n"
   "@param ignoreInternal Whether to ignore internal assets or not.  Optional: Defaults to true.\n"
   "@param ignorePrivate Whether to ignore private assets or not.  Optional: Defaults to true.\n"
   "@return The number of asset Ids found or (-1) if an error occurred.\n")
{
   // Fetch asset query.
   AssetQuery* pAssetQuery;
   Sim::findObject(assetQuery, pAssetQuery);

    // Did we find the asset query?
    if ( pAssetQuery == NULL )
    {
        // No, so warn.
       Con::warnf("AssetManager::findAllAssets() - Could not find the asset query object '%s'.", assetQuery);
        return -1;
    }

    // Perform query.
    return object->findAllAssets( pAssetQuery, ignoreInternal, ignorePrivate );
}

//-----------------------------------------------------------------------------

DefineEngineMethod(AssetManager, findAssetName, S32, (const char* assetQuery, const char* assetName, bool partialName), ("", "", false),
   "Performs an asset query searching for the specified asset name.\n"
   "@param assetQuery The asset query object that will be populated with the results.\n"
   "@param assetName The asset name to search for.  This may be a partial name if 'partialName' is true.\n"
   "@param partialName Whether the asset name is to be used as a partial name or not.  Optional: Defaults to false.\n"
   "@return The number of asset Ids found or (-1) if an error occurred.\n")
{
   // Fetch asset query.
   AssetQuery* pAssetQuery;
   Sim::findObject(assetQuery, pAssetQuery);

    // Did we find the asset query?
    if ( pAssetQuery == NULL )
    {
        // No, so warn.
       Con::warnf("AssetManager::findAssetName() - Could not find the asset query object '%s'.", assetQuery);
        return -1;
    }

    // Fetch asset name.
    const char* pAssetName = assetName;

    // Perform query.
    return object->findAssetName( pAssetQuery, pAssetName, partialName );
}

//-----------------------------------------------------------------------------

DefineEngineMethod(AssetManager, findAssetCategory, S32, (const char* assetQuery, const char* assetCategory, bool assetQueryAsSource), ("", "", false),
   "Performs an asset query searching for the specified asset category.\n"
   "@param assetQuery The asset query object that will be populated with the results.\n"
   "@param assetCategory The asset category to search for.\n"
   "@param assetQueryAsSource Whether to use the asset query as the data-source rather than the asset managers database or not.  Doing this effectively filters the asset query.  Optional: Defaults to false.\n"
   "@return The number of asset Ids found or (-1) if an error occurred.\n")
{
   // Fetch asset query.
   AssetQuery* pAssetQuery;
   Sim::findObject(assetQuery, pAssetQuery);

    // Did we find the asset query?
    if ( pAssetQuery == NULL )
    {
        // No, so warn.
       Con::warnf("AssetManager::findAssetCategory() - Could not find the asset query object '%s'.", assetQuery);
        return -1;
    }

    // Fetch asset category.
    const char* pAssetCategory = assetCategory;

    // Perform query.
    return object->findAssetCategory( pAssetQuery, pAssetCategory, assetQueryAsSource );
}

//-----------------------------------------------------------------------------

DefineEngineMethod(AssetManager, findAssetAutoUnload, S32, (const char* assetQuery, bool assetAutoUnload, bool assetQueryAsSource), ("", false, false),
   "Performs an asset query searching for the specified asset auto-unload flag.\n"
   "@param assetQuery The asset query object that will be populated with the results.\n"
   "@param assetInternal The asset internal flag to search for.\n"
   "@param assetQueryAsSource Whether to use the asset query as the data-source rather than the asset managers database or not.  Doing this effectively filters the asset query.  Optional: Defaults to false.\n"
   "@return The number of asset Ids found or (-1) if an error occurred.\n")
{
   // Fetch asset query.
   AssetQuery* pAssetQuery;
   Sim::findObject(assetQuery, pAssetQuery);

    // Did we find the asset query?
    if ( pAssetQuery == NULL )
    {
        // No, so warn.
       Con::warnf("AssetManager::findAssetAutoUnload() - Could not find the asset query object '%s'.", assetQuery);
        return -1;
    }

    // Perform query.
    return object->findAssetAutoUnload( pAssetQuery, assetAutoUnload, assetQueryAsSource );
}

//-----------------------------------------------------------------------------

DefineEngineMethod(AssetManager, findAssetInternal, S32, (const char* assetQuery, bool assetInternal, bool assetQueryAsSource), ("", false, false),
   "Performs an asset query searching for the specified asset internal flag.\n"
   "@param assetQuery The asset query object that will be populated with the results.\n"
   "@param assetInternal The asset internal flag to search for.\n"
   "@param assetQueryAsSource Whether to use the asset query as the data-source rather than the asset managers database or not.  Doing this effectively filters the asset query.  Optional: Defaults to false.\n"
   "@return The number of asset Ids found or (-1) if an error occurred.\n")
{
   // Fetch asset query.
   AssetQuery* pAssetQuery;
   Sim::findObject(assetQuery, pAssetQuery);

    // Did we find the asset query?
    if ( pAssetQuery == NULL )
    {
        // No, so warn.
       Con::warnf("AssetManager::findAssetInternal() - Could not find the asset query object '%s'.", assetQuery);
        return -1;
    }

    // Perform query.
    return object->findAssetInternal( pAssetQuery, assetInternal, assetQueryAsSource );
}

//-----------------------------------------------------------------------------

DefineEngineMethod(AssetManager, findAssetPrivate, S32, (const char* assetQuery, bool assetPrivate, bool assetQueryAsSource), ("", false, false),
   "Performs an asset query searching for the specified asset private flag.\n"
   "@param assetQuery The asset query object that will be populated with the results.\n"
   "@param assetPrivate The asset private flag to search for.\n"
   "@param assetQueryAsSource Whether to use the asset query as the data-source rather than the asset managers database or not.  Doing this effectively filters the asset query.  Optional: Defaults to false.\n"
   "@return The number of asset Ids found or (-1) if an error occurred.\n")
{
   // Fetch asset query.
   AssetQuery* pAssetQuery;
   Sim::findObject(assetQuery, pAssetQuery);

    // Did we find the asset query?
    if ( pAssetQuery == NULL )
    {
        // No, so warn.
       Con::warnf("AssetManager::findAssetPrivate() - Could not find the asset query object '%s'.", assetQuery);
        return -1;
    }

    // Perform query.
    return object->findAssetInternal( pAssetQuery, assetPrivate, assetQueryAsSource );
}

//-----------------------------------------------------------------------------

DefineEngineMethod(AssetManager, findAssetType, S32, (const char* assetQuery, const char* assetType, bool assetQueryAsSource), ("", "", false),
   "Performs an asset query searching for the specified asset type.\n"
   "@param assetQuery The asset query object that will be populated with the results.\n"
   "@param assetType The asset type to search for.\n"
   "@param assetQueryAsSource Whether to use the asset query as the data-source rather than the asset managers database or not.  Doing this effectively filters the asset query.  Optional: Defaults to false.\n"
   "@return The number of asset Ids found or (-1) if an error occurred.\n")
{
   // Fetch asset query.
   AssetQuery* pAssetQuery;
   Sim::findObject(assetQuery, pAssetQuery);

    // Did we find the asset query?
    if ( pAssetQuery == NULL )
    {
        // No, so warn.
       Con::warnf("AssetManager::findAssetType() - Could not find the asset query object '%s'.", assetQuery);
        return -1;
    }

    // Fetch asset type.
    const char* pAssetType = assetType;

    // Perform query.
    return object->findAssetType( pAssetQuery, pAssetType, assetQueryAsSource );
}

//-----------------------------------------------------------------------------

DefineEngineMethod(AssetManager, findAssetDependsOn, S32, (const char* assetQuery, const char* assetId), ("", ""),
   "Performs an asset query searching for asset Ids that the specified asset Id depends on.\n"
   "@param assetQuery The asset query object that will be populated with the results.\n"
   "@param assetId The asset Id to query for any asset Ids that it depends on.\n"
   "@return The number of asset Ids found or (-1) if an error occurred.\n")
{
   // Fetch asset query.
   AssetQuery* pAssetQuery;
   Sim::findObject(assetQuery, pAssetQuery);

    // Did we find the asset query?
    if ( pAssetQuery == NULL )
    {
        // No, so warn.
       Con::warnf("AssetManager::findAssetDependsOn() - Could not find the asset query object '%s'.", assetQuery);
        return -1;
    }

    // Fetch asset Id.
    const char* pAssetId = assetId;

    // Perform query.
    return object->findAssetDependsOn( pAssetQuery, pAssetId );
}

//-----------------------------------------------------------------------------

DefineEngineMethod(AssetManager, findAssetIsDependedOn, S32, (const char* assetQuery, const char* assetId), ("", ""),
   "Performs an asset query searching for asset Ids that depend on the specified asset Id.\n"
   "@param assetQuery The asset query object that will be populated with the results.\n"
   "@param assetId The asset Id to query for any asset Ids that may depend on it.\n"
   "@return The number of asset Ids found or (-1) if an error occurred.\n")
{
   // Fetch asset query.
   AssetQuery* pAssetQuery;
   Sim::findObject(assetQuery, pAssetQuery);

    // Did we find the asset query?
    if ( pAssetQuery == NULL )
    {
        // No, so warn.
       Con::warnf("AssetManager::findAssetIsDependedOn() - Could not find the asset query object '%s'.", assetQuery);
        return -1;
    }

    // Fetch asset Id.
    const char* pAssetId = assetId;

    // Perform query.
    return object->findAssetIsDependedOn( pAssetQuery, pAssetId );
}

//-----------------------------------------------------------------------------

DefineEngineMethod(AssetManager, findInvalidAssetReferences, S32, (const char* assetQuery), (""),
   "Performs an asset query searching for invalid asset references.\n"
   "@param assetQuery The asset query object that will be populated with the results.\n"
   "@return The number of asset Ids found that are invalid or (-1) if an error occurred.\n")
{
   // Fetch asset query.
   AssetQuery* pAssetQuery;
   Sim::findObject(assetQuery, pAssetQuery);

    // Did we find the asset query?
    if ( pAssetQuery == NULL )
    {
        // No, so warn.
       Con::warnf("AssetManager::findInvalidAssetReferences() - Could not find the asset query object '%s'.", assetQuery);
        return -1;
    }

    // Perform query.
    return object->findInvalidAssetReferences( pAssetQuery );
}

//-----------------------------------------------------------------------------

DefineEngineMethod(AssetManager, findTaggedAssets, S32, (const char* assetQuery, const char* assetTagNames, bool assetQueryAsSource), ("", "", false),
   "Performs an asset query searching for the specified asset tag name(s).\n"
   "@param assetQuery The asset query object that will be populated with the results.\n"
   "@param assetTagNames The asset tag name or names to search for.  Multiple names can be specified using comma, space, tab or newline separation.  Tags use an OR operation i.e. only assets tagged with ANY of the specified tags will be returned.\n"
   "@param assetQueryAsSource Whether to use the asset query as the data-source rather than the asset managers database or not.  Doing this effectively filters the asset query.  Optional: Defaults to false.\n"
   "@return The number of asset Ids found or (-1) if an error occurred.\n")
{
   // Fetch asset query.
   AssetQuery* pAssetQuery;
   Sim::findObject(assetQuery, pAssetQuery);

    // Did we find the asset query?
    if ( pAssetQuery == NULL )
    {
        // No, so warn.
       Con::warnf("AssetManager::findTaggedAssets() - Could not find the asset query object '%s'.", assetQuery);
        return -1;
    }

    // Fetch asset tag name(s).
    const char* pAssetTagNames = assetTagNames;

    // Perform query.
    return object->findTaggedAssets( pAssetQuery, pAssetTagNames, assetQueryAsSource );
}

//-----------------------------------------------------------------------------

DefineEngineMethod(AssetManager, findAssetLooseFile, S32, (const char* assetQuery, const char* assetLooseFile, bool assetQueryAsSource), ("", "", false),
   "Performs an asset query searching for the specified loose file.\n"
   "@param assetQuery The asset query object that will be populated with the results.\n"
   "@param assetLooseFile The loose-file used by the asset to search for.\n"
   "@param assetQueryAsSource Whether to use the asset query as the data-source rather than the asset managers database or not.  Doing this effectively filters the asset query.  Optional: Defaults to false.\n"
   "@return The number of asset Ids found or (-1) if an error occurred.\n")
{
   // Fetch asset query.
   AssetQuery* pAssetQuery;
   Sim::findObject(assetQuery, pAssetQuery);

    // Did we find the asset query?
    if ( pAssetQuery == NULL )
    {
        // No, so warn.
       Con::warnf("AssetManager::findAssetLooseFile() - Could not find the asset query object '%s'.", assetQuery);
        return -1;
    }

    // Fetch asset loose file.
    const char* pAssetLooseFile = assetLooseFile;

    // Perform query.
    return object->findAssetLooseFile( pAssetQuery, pAssetLooseFile, assetQueryAsSource );
}

//-----------------------------------------------------------------------------

DefineEngineMethod(AssetManager, getDeclaredAssetCount, bool, (),,
   "Gets the number of declared assets.\n"
   "@return Returns the number of declared assets.\n")
{
    return object->getDeclaredAssetCount();
}

//-----------------------------------------------------------------------------

DefineEngineMethod(AssetManager, getReferencedAssetCount, bool, (), ,
   "Gets the number of asset referenced.\n"
   "@return Returns the number of asset references.\n")
{
    return object->getReferencedAssetCount();
}

//-----------------------------------------------------------------------------

DefineEngineMethod(AssetManager, getLoadedInternalAssetCount, bool, (), ,
   "Gets the number of loaded internal assets.\n"
   "@return Returns the number of loaded internal assets.\n")
{
    return object->getLoadedInternalAssetCount();
}

//-----------------------------------------------------------------------------

DefineEngineMethod(AssetManager, getMaxLoadedInternalAssetCount, bool, (), ,
   "Gets the maximum number of loaded internal assets.\n"
   "@return Returns the maximum number of loaded internal assets.\n")
{
    return object->getMaxLoadedInternalAssetCount();
}

//-----------------------------------------------------------------------------

DefineEngineMethod(AssetManager, getLoadedExternalAssetCount, bool, (), ,
   "Gets the number of loaded external assets.\n"
   "@return Returns the number of loaded external assets.\n")
{
    return object->getLoadedExternalAssetCount();
}

//-----------------------------------------------------------------------------

DefineEngineMethod(AssetManager, getMaxLoadedExternalAssetCount, bool, (), ,
   "Gets the maximum number of loaded external assets.\n"
   "@return Returns the maximum number of loaded external assets.\n")
{
    return object->getMaxLoadedExternalAssetCount();
}

//-----------------------------------------------------------------------------

DefineEngineMethod(AssetManager, dumpDeclaredAssets, void, (), ,
   "Dumps a breakdown of all declared assets.\n"
   "@return No return value.\n")
{
    return object->dumpDeclaredAssets();
}
