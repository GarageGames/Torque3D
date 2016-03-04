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

#ifndef _ASSET_PTR_H_
#define _ASSET_PTR_H_

#ifndef _ASSET_MANAGER_H_
#include "assetManager.h"
#endif

//-----------------------------------------------------------------------------

class AssetPtrCallback
{
    friend class AssetManager;

protected:
    virtual void onAssetRefreshed( AssetPtrBase* pAssetPtrBase ) = 0;    
};

//-----------------------------------------------------------------------------

class AssetPtrBase
{
public:
    AssetPtrBase() {};
    virtual ~AssetPtrBase()
    {
        // Un-register any notifications.
        unregisterRefreshNotify();
    };

    /// Referencing.
    virtual void clear( void ) = 0;
    virtual void setAssetId( const char* pAssetId ) = 0;
    virtual StringTableEntry getAssetId( void ) const = 0;
    virtual StringTableEntry getAssetType( void ) const = 0;
    virtual bool isAssetId( const char* pAssetId ) const = 0;

    /// Validity.
    virtual bool isNull( void ) const = 0;
    virtual bool notNull( void ) const = 0;

    /// Notification.
    inline void registerRefreshNotify( AssetPtrCallback* pCallback )
    {
        // Sanity!
        AssertFatal( AssetDatabase.isProperlyAdded(), "AssetPtrBase::registerRefreshNotify() - Cannot register an asset pointer with the asset system." );

        // register refresh notify.
        AssetDatabase.registerAssetPtrRefreshNotify( this, pCallback );
    }

    void unregisterRefreshNotify( void )
    {
        // Un-register the refresh notify if the asset system is available.
        if ( AssetDatabase.isProperlyAdded() )
            AssetDatabase.unregisterAssetPtrRefreshNotify( this );
    }
};

//-----------------------------------------------------------------------------

template<typename T> class AssetPtr : public AssetPtrBase
{
private:
    SimObjectPtr<T> mpAsset;

public:
    AssetPtr() {}
    AssetPtr( const char* pAssetId )
    {
        // Finish if this is an invalid asset Id.
        if ( pAssetId == NULL || *pAssetId == 0 )
            return;

        // Acquire asset.
        mpAsset = AssetDatabase.acquireAsset<T>( pAssetId );
    }
    AssetPtr( const AssetPtr<T>& assetPtr )
    {
        // Does the asset pointer have an asset?
        if ( assetPtr.notNull() )
        {
            // Yes, so acquire the asset.
            mpAsset = AssetDatabase.acquireAsset<T>( assetPtr->getAssetId() );
        }
    }
    virtual ~AssetPtr()
    {
        // Do we have an asset?
        if ( notNull() )
        {
            // Yes, so release it.
            AssetDatabase.releaseAsset( mpAsset->getAssetId() );
        }
    }

    /// Assignment.
    AssetPtr<T>& operator=( const char* pAssetId )
    {
        // Do we have an asset?
        if ( notNull() )
        {
            // Yes, so finish if the asset Id is already assigned.
            if ( isAssetId( pAssetId ) )
                return *this;

            // No, so release it.
            AssetDatabase.releaseAsset( mpAsset->getAssetId() );
        }

        // Is the asset Id at least okay to attempt to acquire the asset?
        if ( pAssetId != NULL && *pAssetId != 0 )
        {
            // Yes, so acquire the asset.
            mpAsset = AssetDatabase.acquireAsset<T>( pAssetId );
        }
        else
        {
            // No, so remove reference.
            mpAsset = NULL;
        }

        // Return Reference.
        return *this;
    }

    AssetPtr<T>& operator=( const AssetPtr<T>& assetPtr )
    {
        // Set asset pointer.
        *this = assetPtr->getAssetId();

        // Return Reference.
        return *this;
    }

    /// Referencing.
    virtual void clear( void )
    {
        // Do we have an asset?
        if ( notNull() )
        {
            // Yes, so release it.
            AssetDatabase.releaseAsset( mpAsset->getAssetId() );
        }

        // Reset the asset reference.
        mpAsset = NULL;
    }

    T* operator->( void ) const { return mpAsset; }
    T& operator*( void ) const { return *mpAsset; }
    operator T*( void ) const { return mpAsset; }
    virtual void setAssetId( const char* pAssetId ) { *this = pAssetId; }
    virtual StringTableEntry getAssetId( void ) const { return isNull() ? StringTable->EmptyString() : mpAsset->getAssetId(); }
    virtual StringTableEntry getAssetType(void) const { return isNull() ? StringTable->EmptyString() : mpAsset->getClassName(); }
    virtual bool isAssetId( const char* pAssetId ) const { return pAssetId == NULL ? isNull() : getAssetId() == StringTable->insert(pAssetId); }

    /// Validity.
    virtual bool isNull( void ) const { return mpAsset.isNull(); }
    virtual bool notNull( void ) const { return !mpAsset.isNull(); }
};

#endif // _ASSET_PTR_H_
