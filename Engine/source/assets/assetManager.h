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

#ifndef _ASSET_MANAGER_H_
#define _ASSET_MANAGER_H_

#ifndef _SIMBASE_H_
#include "console/sim.h"
#endif

#ifndef _TAML_H_
#include "persistence/taml/taml.h"
#endif

#ifndef _MODULE_DEFINITION_H
#include "module/moduleDefinition.h"
#endif

#ifndef _MODULE_CALLBACKS_H_
#include "module/moduleCallbacks.h"
#endif

#ifndef _ASSET_BASE_H_
#include "assets/assetBase.h"
#endif

#ifndef _ASSET_DEFINITION_H_
#include "assets/assetDefinition.h"
#endif

#ifndef _ASSET_TAGS_MANIFEST_H_
#include "assets/assetTagsManifest.h"
#endif

#ifndef _ASSET_QUERY_H_
#include "assets/assetQuery.h"
#endif

#ifndef _ASSET_FIELD_TYPES_H_
#include "assets/assetFieldTypes.h"
#endif

// Debug Profiling.
#include "platform/profiler.h"

//-----------------------------------------------------------------------------

class AssetPtrCallback;
class AssetPtrBase;

//-----------------------------------------------------------------------------

class AssetManager : public SimObject, public ModuleCallbacks
{
private:
    typedef SimObject Parent;
    typedef StringTableEntry typeAssetId;
    typedef StringTableEntry typeAssetName;
    typedef StringTableEntry typeReferenceFilePath;
    typedef HashMap<typeAssetId, AssetDefinition*> typeDeclaredAssetsHash;
    typedef HashTable<typeAssetId, typeReferenceFilePath> typeReferencedAssetsHash;
    typedef HashTable<typeAssetId, typeAssetId> typeAssetDependsOnHash;
    typedef HashTable<typeAssetId, typeAssetId> typeAssetIsDependedOnHash;
    typedef HashMap<AssetPtrBase*, AssetPtrCallback*> typeAssetPtrRefreshHash;

    /// Declared assets.
    typeDeclaredAssetsHash              mDeclaredAssets;

    /// Referenced assets.
    typeReferencedAssetsHash            mReferencedAssets;

    /// Asset dependencies.
    typeAssetDependsOnHash              mAssetDependsOn;
    typeAssetIsDependedOnHash           mAssetIsDependedOn;

    /// Asset tags.
    SimObjectPtr<AssetTagsManifest>     mAssetTagsManifest;
    SimObjectPtr<ModuleDefinition>      mAssetTagsModuleDefinition;

    /// Asset pointer refresh notifications.
    typeAssetPtrRefreshHash             mAssetPtrRefreshNotifications;

    /// Miscellaneous.
    bool                                mEchoInfo;
    bool                                mIgnoreAutoUnload;
    U32                                 mLoadedInternalAssetsCount;
    U32                                 mLoadedExternalAssetsCount;
    U32                                 mLoadedPrivateAssetsCount;
    U32                                 mAcquiredReferenceCount;
    U32                                 mMaxLoadedInternalAssetsCount;
    U32                                 mMaxLoadedExternalAssetsCount;
    U32                                 mMaxLoadedPrivateAssetsCount;
    Taml                                mTaml;

public:
    AssetManager();
    virtual ~AssetManager() {}

    /// SimObject overrides
    virtual bool onAdd();
    virtual void onRemove();
    static void initPersistFields();

    /// Declared assets.
    bool addModuleDeclaredAssets( ModuleDefinition* pModuleDefinition );
    bool addDeclaredAsset( ModuleDefinition* pModuleDefinition, const char* pAssetFilePath );
    StringTableEntry addPrivateAsset( AssetBase* pAssetBase );
    bool removeDeclaredAssets( ModuleDefinition* pModuleDefinition );
    bool removeDeclaredAsset( const char* pAssetId );
    bool renameDeclaredAsset( const char* pAssetIdFrom, const char* pAssetIdTo );
    StringTableEntry getAssetName( const char* pAssetId );
    StringTableEntry getAssetDescription( const char* pAssetId );
    StringTableEntry getAssetCategory( const char* pAssetId );
    StringTableEntry getAssetType( const char* pAssetId );
    StringTableEntry getAssetFilePath( const char* pAssetId );
    StringTableEntry getAssetPath( const char* pAssetId );
    ModuleDefinition* getAssetModuleDefinition( const char* pAssetId );
    bool isAssetInternal( const char* pAssetId );
    bool isAssetPrivate( const char* pAssetId );
    bool isAssetAutoUnload( const char* pAssetId );
    bool isAssetLoaded( const char* pAssetId );
    bool isDeclaredAsset( const char* pAssetId );
    bool doesAssetDependOn( const char* pAssetId, const char* pDependsOnAssetId );
    bool isAssetDependedOn( const char* pAssetId, const char* pDependedOnByAssetId );

    /// Referenced assets.
    bool compileReferencedAssets( ModuleDefinition* pModuleDefinition );
    bool isReferencedAsset( const char* pAssetId );
    bool renameReferencedAsset( const char* pAssetIdFrom, const char* pAssetIdTo );

    /// Public asset acquisition.
    template<typename T> T* acquireAsset( const char* pAssetId )
    {
        // Sanity!
        AssertFatal( pAssetId != NULL, "Cannot acquire NULL asset Id." );

        // Is this an empty asset Id?
        if ( *pAssetId == 0 )
        {
            // Yes, so return nothing.
            return NULL;
        }

        // Find asset.
        AssetDefinition* pAssetDefinition = findAsset( pAssetId );

        // Did we find the asset?
        if ( pAssetDefinition == NULL )
        {
            // No, so warn.
            Con::warnf( "Asset Manager: Failed to acquire asset Id '%s' as it does not exist.", pAssetId );
            return NULL;
        }

        // Is asset loading?
        if ( pAssetDefinition->mAssetLoading == true )
        {
            // Yes, so we've got a circular loop which we cannot resolve!
            Con::warnf( "Asset Manager: Failed to acquire asset Id '%s' as loading it involves a cyclic dependency on itself which cannot be resolved.", pAssetId );
            return NULL;
        }

        // Info.
        if ( mEchoInfo )
        {
            Con::printSeparator();
            Con::printf( "Asset Manager: Started acquiring Asset Id '%s'...", pAssetId );
        }

        // Is the asset already loaded?
        if ( pAssetDefinition->mpAssetBase == NULL )
        {
            // No, so info
            if ( mEchoInfo )
            {
                // Fetch asset Id.
                StringTableEntry assetId = StringTable->insert( pAssetId );

                // Find any asset dependencies.
                typeAssetDependsOnHash::Iterator assetDependenciesItr = mAssetDependsOn.find( assetId );

                // Does the asset have any dependencies?
                if ( assetDependenciesItr != mAssetDependsOn.end() )
                {
                    // Yes, so show all dependency assets.
                    Con::printf( "Asset Manager: > Found dependencies:" );

                    // Iterate all dependencies.
                    while( assetDependenciesItr != mAssetDependsOn.end() && assetDependenciesItr->key == assetId )
                    {
                        // Info.
                        Con::printf( "Asset Manager: > Asset Id '%s'", assetDependenciesItr->value );

                        // Next dependency.
                        assetDependenciesItr++;
                    }
                }
            }

            // Flag asset as loading.
            pAssetDefinition->mAssetLoading = true;

            // Generate primary asset.
            pAssetDefinition->mpAssetBase = mTaml.read<T>( pAssetDefinition->mAssetBaseFilePath );

            // Flag asset as finished loading.
            pAssetDefinition->mAssetLoading = false;

            // Did we generate the asset?
            if ( pAssetDefinition->mpAssetBase == NULL )
            {
                // No, so warn.
                Con::warnf( "Asset Manager: > Failed to acquire asset Id '%s' as loading the asset file failed to return the asset or the correct asset type: '%s'.",
                    pAssetId, pAssetDefinition->mAssetBaseFilePath );
                return NULL;
            }

            // Increase loaded count.
            pAssetDefinition->mAssetLoadedCount++;

            // Info.
            if ( mEchoInfo )
            {
                Con::printf( "Asset Manager: > Loading asset into memory as object Id '%d' from file '%s'.",
                    pAssetDefinition->mpAssetBase->getId(), pAssetDefinition->mAssetBaseFilePath );
            }

            // Set ownership by asset manager.
            pAssetDefinition->mpAssetBase->setOwned( this, pAssetDefinition );

            // Is the asset internal?
            if ( pAssetDefinition->mAssetInternal )
            {
                // Yes, so increase internal loaded asset count.
                if ( ++mLoadedInternalAssetsCount > mMaxLoadedInternalAssetsCount )
                    mMaxLoadedInternalAssetsCount = mLoadedInternalAssetsCount;
            }
            else
            {
                // No, so increase external loaded assets count.
                if ( ++mLoadedExternalAssetsCount > mMaxLoadedExternalAssetsCount )
                    mMaxLoadedExternalAssetsCount = mLoadedExternalAssetsCount;
            }
        }
        else if ( pAssetDefinition->mpAssetBase->getAcquiredReferenceCount() == 0 )
        {
            // Info.
            if ( mEchoInfo )
            {
                Con::printf( "Asset Manager: > Acquiring from idle state." );
            }
        }

        // Set acquired asset.
        T* pAcquiredAsset = dynamic_cast<T*>( (AssetBase*)pAssetDefinition->mpAssetBase );

        // Is asset the correct type?
        if ( pAcquiredAsset == NULL )
        {
            // No, so warn.
            Con::warnf( "Asset Manager: > Failed to acquire asset Id '%s' as it was not the required asset type: '%s'.", pAssetId, pAssetDefinition->mAssetBaseFilePath );
            return NULL;
        }

        // Acquire asset reference.
        pAcquiredAsset->acquireAssetReference();

        // Info.
        if ( mEchoInfo )
        {
            Con::printf( "Asset Manager: > Finished acquiring asset.  Reference count now '%d'.", pAssetDefinition->mpAssetBase->getAcquiredReferenceCount() );
            Con::printSeparator();
        }

        return pAcquiredAsset;
    }

    /// Private asset acquisition.
    template<typename T> T* acquireAsPrivateAsset( const char* pAssetId )
    {
        // Acquire the asset normally.
        T* pAsset = acquireAsset<T>( pAssetId );

        // Finish if the asset was not acquired.
        if ( pAsset == NULL )
            return NULL;

        // Clone the asset.
        T* pAssetClone = dynamic_cast<T*>( pAsset->clone() );

        // Sanity!
        AssertFatal( pAssetClone != NULL, "acquireAsPrivateAsset() - Failed to clone asset type." );

        // Release the public asset.
        releaseAsset( pAssetId );

        // Add as a private asset.
        addPrivateAsset( pAssetClone );

        return pAssetClone;
    }

    bool releaseAsset( const char* pAssetId );
    void purgeAssets( void );

    /// Asset deletion.
    bool deleteAsset( const char* pAssetId, const bool deleteLooseFiles, const bool deleteDependencies );

    // Asset refresh notification.
    bool refreshAsset( const char* pAssetId );
    void refreshAllAssets( const bool includeUnloaded = false );
    void registerAssetPtrRefreshNotify( AssetPtrBase* pAssetPtrBase, AssetPtrCallback* pCallback );
    void unregisterAssetPtrRefreshNotify( AssetPtrBase* pAssetPtrBase );

    /// Asset tags.
    bool loadAssetTags( ModuleDefinition* pModuleDefinition );
    bool saveAssetTags( void );
    bool restoreAssetTags( void );
    inline AssetTagsManifest* getAssetTags( void ) const { return mAssetTagsManifest; }

    /// Info.
    inline U32 getDeclaredAssetCount( void ) const { return (U32)mDeclaredAssets.size(); }
    inline U32 getReferencedAssetCount( void ) const { return (U32)mReferencedAssets.size(); }
    inline U32 getLoadedInternalAssetCount( void ) const { return mLoadedInternalAssetsCount; }
    inline U32 getLoadedExternalAssetCount( void ) const { return mLoadedExternalAssetsCount; }
    inline U32 getLoadedPrivateAssetCount( void ) const { return mLoadedPrivateAssetsCount; }
    inline U32 getMaxLoadedInternalAssetCount( void ) const { return mMaxLoadedInternalAssetsCount; }
    inline U32 getMaxLoadedExternalAssetCount( void ) const { return mMaxLoadedExternalAssetsCount; }
    inline U32 getMaxLoadedPrivateAssetCount( void ) const { return mMaxLoadedPrivateAssetsCount; }
    void dumpDeclaredAssets( void ) const;

    /// Total acquired asset references.
    inline void acquireAcquiredReferenceCount( void ) { mAcquiredReferenceCount++; }
    inline void releaseAcquiredReferenceCount( void ) { AssertFatal( mAcquiredReferenceCount != 0, "AssetManager: Invalid acquired reference count." ); mAcquiredReferenceCount--; }
    inline U32 getAcquiredReferenceCount( void ) const { return mAcquiredReferenceCount; }

    /// Asset queries.
    S32 findAllAssets( AssetQuery* pAssetQuery, const bool ignoreInternal = true, const bool ignorePrivate = true );
    S32 findAssetName( AssetQuery* pAssetQuery, const char* pAssetName, const bool partialName = false );
    S32 findAssetCategory( AssetQuery* pAssetQuery, const char* pAssetCategory, const bool assetQueryAsSource = false );
    S32 findAssetAutoUnload( AssetQuery* pAssetQuery, const bool assetAutoUnload, const bool assetQueryAsSource = false );
    S32 findAssetInternal( AssetQuery* pAssetQuery, const bool assetInternal, const bool assetQueryAsSource = false );
    S32 findAssetPrivate( AssetQuery* pAssetQuery, const bool assetPrivate, const bool assetQueryAsSource = false );
    S32 findAssetType( AssetQuery* pAssetQuery, const char* pAssetType, const bool assetQueryAsSource = false );
    S32 findAssetDependsOn( AssetQuery* pAssetQuery, const char* pAssetId );
    S32 findAssetIsDependedOn( AssetQuery* pAssetQuery, const char* pAssetId );
    S32 findInvalidAssetReferences( AssetQuery* pAssetQuery );
    S32 findTaggedAssets( AssetQuery* pAssetQuery, const char* pAssetTagNames, const bool assetQueryAsSource = false );
    S32 findAssetLooseFile( AssetQuery* pAssetQuery, const char* pLooseFile, const bool assetQueryAsSource = false );

    /// Declare Console Object.
    DECLARE_CONOBJECT( AssetManager );

private:
    bool scanDeclaredAssets( const char* pPath, const char* pExtension, const bool recurse, ModuleDefinition* pModuleDefinition );
    bool scanReferencedAssets( const char* pPath, const char* pExtension, const bool recurse );
    AssetDefinition* findAsset( const char* pAssetId );
    void addReferencedAsset( StringTableEntry assetId, StringTableEntry referenceFilePath );
    void renameAssetReferences( StringTableEntry assetIdFrom, StringTableEntry assetIdTo );
    void removeAssetReferences( StringTableEntry assetId );
    void renameAssetDependencies( StringTableEntry assetIdFrom, StringTableEntry assetIdTo );
    void removeAssetDependencies( const char* pAssetId );
    void removeAssetLooseFiles( const char* pAssetId );
    void unloadAsset( AssetDefinition* pAssetDefinition );

    /// Module callbacks.
    virtual void onModulePreLoad( ModuleDefinition* pModuleDefinition );
    virtual void onModulePreUnload( ModuleDefinition* pModuleDefinition );
    virtual void onModulePostUnload( ModuleDefinition* pModuleDefinition );
};

//-----------------------------------------------------------------------------

extern AssetManager AssetDatabase;

#endif // _ASSET_MANAGER_H_