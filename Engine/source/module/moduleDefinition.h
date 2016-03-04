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

#ifndef _MODULE_DEFINITION_H
#define _MODULE_DEFINITION_H

#ifndef _ASSET_DEFINITION_H_
#include "assets/assetDefinition.h"
#endif

#ifndef _SIMSET_H_
#include "console/simSet.h"
#endif

#ifndef _SIMBASE_H_
#include "console/simBase.h"
#endif

#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif

#ifndef _STRINGUNIT_H_
#include "core/strings/stringUnit.h"
#endif

//-----------------------------------------------------------------------------

class ModuleManager;

//-----------------------------------------------------------------------------

/// @ingroup moduleGroup
/// @see moduleGroup
class ModuleDefinition : public SimSet
{
    friend class ModuleManager;

private:
    typedef SimSet Parent;

public:
    /// Module dependency.
    struct ModuleDependency
    {
        ModuleDependency() :
            mModuleId( StringTable->EmptyString() ),
            mVersionId( 0 )        
        {
        }

        ModuleDependency( StringTableEntry moduleId, const U32 versionId ) :
            mModuleId( moduleId ),
            mVersionId( versionId )
        {
        }

        StringTableEntry    mModuleId;
        U32                 mVersionId;
    };
    typedef Vector<ModuleDependency> typeModuleDependencyVector;
    typedef Vector<AssetDefinition*> typeModuleAssetsVector;

private:
    /// Module definition.
    StringTableEntry                mModuleId;
    U32                             mVersionId;
    U32                             mBuildId;
    bool                            mEnabled;
    bool                            mSynchronized;
    bool                            mDeprecated;
    bool                            mCriticalMerge;
    StringTableEntry                mModuleDescription;
    StringTableEntry                mAuthor;;
    StringTableEntry                mModuleGroup;
    StringTableEntry                mModuleType;
    typeModuleDependencyVector      mDependencies;
    StringTableEntry                mScriptFile;
    StringTableEntry                mCreateFunction;
    StringTableEntry                mDestroyFunction;

    /// Modules assets.
    StringTableEntry                mAssetTagsManifest;
    typeModuleAssetsVector          mModuleAssets;

    /// Module location.
    StringTableEntry                mModulePath;
    StringTableEntry                mModuleFile;
    StringTableEntry                mModuleFilePath;
    StringTableEntry                mModuleScriptFilePath;

    /// Miscellaneous.
    StringTableEntry                mSignature;
    S32                             mLoadCount;
    SimObjectId                     mScopeSet;
    bool                            mLocked;
    ModuleManager*                  mpModuleManager;

private:
    inline bool             checkUnlocked( void ) const { if ( mLocked )        { Con::warnf("Ignoring changes for locked module definition."); } return !mLocked; }
    inline void             setModuleManager( ModuleManager* pModuleManager )   { mpModuleManager = pModuleManager; }

public:
    ModuleDefinition();
    virtual ~ModuleDefinition() {}

    /// Engine.
    static void             initPersistFields();

    /// Module definition.
    inline void             setModuleId( const char* pModuleId )                { if ( checkUnlocked() ) { mModuleId = StringTable->insert(pModuleId); } }
    inline StringTableEntry getModuleId( void ) const                           { return mModuleId; }
    inline void             setVersionId( const U32 versionId )                 { if ( checkUnlocked() ) { mVersionId = versionId; } }
    inline U32              getVersionId( void ) const                          { return mVersionId; }
    inline void             setBuildId( const U32 buildId )                     { if ( checkUnlocked() ) { mBuildId = buildId; } }
    inline U32              getBuildId( void ) const                            { return mBuildId; }
    inline void             setEnabled( const bool enabled )                    { if ( checkUnlocked() ) { mEnabled = enabled; } }
    inline bool             getEnabled( void ) const                            { return mEnabled; }
    inline void             setSynchronized( const bool synchronized )          { if ( checkUnlocked() ) { mSynchronized = synchronized; } }
    inline bool             getSynchronized( void ) const                       { return mSynchronized; }
    inline void             setDeprecated( const bool deprecated )              { if ( checkUnlocked() ) { mDeprecated = deprecated; } }
    inline bool             getDeprecated( void ) const                         { return mDeprecated; }
    inline void             setCriticalMerge( const bool mergeCritical )        { if ( checkUnlocked() ) { mCriticalMerge = mergeCritical; } }
    inline bool             getCriticalMerge( void ) const                      { return mCriticalMerge; }
    inline void             setModuleDescription( const char* pModuleDescription ) { if ( checkUnlocked() ) { mModuleDescription = StringTable->insert(pModuleDescription); } }
    inline StringTableEntry getModuleDescription( void ) const                  { return mModuleDescription; }
    inline void             setAuthor( const char* pAuthor )                    { if ( checkUnlocked() ) { mAuthor = StringTable->insert(pAuthor); } }
    inline StringTableEntry getAuthor( void ) const                             { return mAuthor; }
    inline void             setModuleGroup( const char* pModuleGroup )          { if ( checkUnlocked() ) { mModuleGroup = StringTable->insert(pModuleGroup); } }
    inline StringTableEntry getModuleGroup( void ) const                        { return mModuleGroup; }
    inline void             setModuleType( const char* pModuleType )            { if ( checkUnlocked() ) { mModuleType = StringTable->insert(pModuleType); } }
    inline StringTableEntry getModuleType( void ) const                         { return mModuleType; }
    inline void             setDependencies( const typeModuleDependencyVector& dependencies ) { if ( checkUnlocked() ) { mDependencies.clear(); mDependencies.merge(dependencies); } }
    inline const typeModuleDependencyVector& getDependencies( void ) const      { return mDependencies; }
    inline void             setScriptFile( const char* pScriptFile )            { if ( checkUnlocked() ) { mScriptFile = StringTable->insert(pScriptFile); } }
    inline StringTableEntry getScriptFile( void ) const                         { return mScriptFile; }
    inline void             setCreateFunction( const char* pCreateFunction )    { if ( checkUnlocked() ) { mCreateFunction = StringTable->insert(pCreateFunction); } }
    inline StringTableEntry getCreateFunction( void ) const                     { return mCreateFunction; }
    inline void             setDestroyFunction( const char* pDestroyFunction )  { if ( checkUnlocked() ) { mDestroyFunction = StringTable->insert(pDestroyFunction); } }
    inline StringTableEntry getDestroyFunction( void ) const                    { return mDestroyFunction; }
    inline SimObjectId      getScopeSet( void ) const                           { return mScopeSet; }

    /// Module assets.
    inline void             setAssetTagsManifest( const char* pTagsAssetManifest ) { if ( checkUnlocked() ) { mAssetTagsManifest = StringTable->insert(pTagsAssetManifest); } }
    inline StringTableEntry getAssetTagsManifest( void ) const                  { return mAssetTagsManifest; }
    inline typeModuleAssetsVector& getModuleAssets( void )                      { return mModuleAssets; }

    /// Module location.
    inline void             setModulePath( const char* pModulePath )            { if ( checkUnlocked() ) { mModulePath = StringTable->insert(pModulePath); } }
    inline StringTableEntry getModulePath( void ) const                         { return mModulePath; }
    inline void             setModuleFile( const char* pModuleDefinitionFile )  { if ( checkUnlocked() ) { mModuleFile = StringTable->insert(pModuleDefinitionFile); } }
    inline StringTableEntry getModuleFile( void ) const                         { return mModuleFile; }
    inline void             setModuleFilePath( const char* pModuleDefinitionFilePath ) { if ( checkUnlocked() ) { mModuleFilePath = StringTable->insert(pModuleDefinitionFilePath); } }
    inline StringTableEntry getModuleFilePath( void ) const                     { return mModuleFilePath; }
    inline void             setModuleScriptFilePath( const char* pModuleScriptFilePath ) { if ( checkUnlocked() ) { mModuleScriptFilePath = StringTable->insert(pModuleScriptFilePath); } }
    inline StringTableEntry getModuleScriptFilePath( void ) const               { return mModuleScriptFilePath; }

    /// Specialized dependency control.
    inline U32              getDependencyCount( void ) const                    { return mDependencies.size(); }
    bool                    getDependency( const U32 dependencyIndex, ModuleDependency& dependency ) const;
    bool                    addDependency( const char* pModuleId, const U32 versionId );
    bool                    removeDependency( const char* pModuleId );

    /// Miscellaneous.
    inline void             setSignature( const char* pSignature )              { if ( checkUnlocked() ) { mSignature = StringTable->insert(pSignature); } }
    inline StringTableEntry getSignature( void ) const                          { return mSignature; }
    inline void             increaseLoadCount( void )                           { ++mLoadCount; }
    inline void             reduceLoadCount( void )                             { --mLoadCount; }
    inline S32              getLoadCount( void ) const                          { return mLoadCount; }
    inline void             setLocked( const bool status )                      { mLocked = status; }
    inline bool             getLocked( void ) const                             { return mLocked; }
    inline ModuleManager*   getModuleManager( void ) const                      { return mpModuleManager; }
    bool                    save( void );

    /// Declare Console Object.
    DECLARE_CONOBJECT( ModuleDefinition );

protected:
    static bool             setModuleId(void* obj, const char* index, const char* data)                    { static_cast<ModuleDefinition*>(obj)->setModuleId( data ); return false; }
    static bool             setVersionId(void* obj, const char* index, const char* data)                   { static_cast<ModuleDefinition*>(obj)->setVersionId((U32)dAtoi(data)); return false; }
    static bool             setBuildId(void* obj, const char* index, const char* data)                     { static_cast<ModuleDefinition*>(obj)->setBuildId((U32)dAtoi(data)); return false; }
    static bool             writeBuildId( void* obj, StringTableEntry pFieldName )      { return static_cast<ModuleDefinition*>(obj)->getBuildId() != 0; }
    static bool             setEnabled(void* obj, const char* index, const char* data)                     { static_cast<ModuleDefinition*>(obj)->setEnabled(dAtob(data)); return false; }
    static bool             writeEnabled( void* obj, StringTableEntry pFieldName )      { return static_cast<ModuleDefinition*>(obj)->getEnabled() == false; }
    static bool             setSynchronized(void* obj, const char* index, const char* data)                { static_cast<ModuleDefinition*>(obj)->setSynchronized(dAtob(data)); return false; }
    static bool             writeSynchronized( void* obj, StringTableEntry pFieldName ) { return static_cast<ModuleDefinition*>(obj)->getSynchronized() == true; }
    static bool             setDeprecated(void* obj, const char* index, const char* data)                  { static_cast<ModuleDefinition*>(obj)->setDeprecated(dAtob(data)); return false; }
    static bool             writeDeprecated( void* obj, StringTableEntry pFieldName )   { return static_cast<ModuleDefinition*>(obj)->getDeprecated() == true; }
    static bool             writeCriticalMerge( void* obj, StringTableEntry pFieldName ){ return static_cast<ModuleDefinition*>(obj)->getCriticalMerge() == true; }    
    static bool             setModuleDescription(void* obj, const char* index, const char* data)           { static_cast<ModuleDefinition*>(obj)->setModuleDescription(data); return false; }
    static bool             writeModuleDescription( void* obj, StringTableEntry pFieldName ) { return static_cast<ModuleDefinition*>(obj)->getModuleDescription() != StringTable->EmptyString(); }
    static bool             setAuthor(void* obj, const char* index, const char* data)                      { static_cast<ModuleDefinition*>(obj)->setAuthor(data); return false; }
    static bool             writeAuthor(void* obj, StringTableEntry pFieldName)       { return static_cast<ModuleDefinition*>(obj)->getAuthor() != StringTable->EmptyString(); }
    static bool             setModuleGroup(void* obj, const char* index, const char* data)                 { static_cast<ModuleDefinition*>(obj)->setModuleGroup(data); return false; }
    static bool             setModuleType(void* obj, const char* index, const char* data)                  { static_cast<ModuleDefinition*>(obj)->setModuleType(data); return false; }
    static bool             writeModuleType(void* obj, StringTableEntry pFieldName)   { return static_cast<ModuleDefinition*>(obj)->getModuleType() != StringTable->EmptyString(); }
    static bool             setScriptFile(void* obj, const char* index, const char* data)                  { static_cast<ModuleDefinition*>(obj)->setScriptFile(data); return false; }
    static bool             writeScriptFile(void* obj, StringTableEntry pFieldName)   { return static_cast<ModuleDefinition*>(obj)->getScriptFile() != StringTable->EmptyString(); }
    static bool             setCreateFunction(void* obj, const char* index, const char* data)              { static_cast<ModuleDefinition*>(obj)->setCreateFunction(data); return false; }
    static bool             writeCreateFunction(void* obj, StringTableEntry pFieldName) { return static_cast<ModuleDefinition*>(obj)->getCreateFunction() != StringTable->EmptyString(); }
    static bool             setDestroyFunction(void* obj, const char* index, const char* data)             { static_cast<ModuleDefinition*>(obj)->setDestroyFunction(data); return false; }
    static bool             writeDestroyFunction(void* obj, StringTableEntry pFieldName) { return static_cast<ModuleDefinition*>(obj)->getDestroyFunction() != StringTable->EmptyString(); }

    /// Asset manifest.
    static bool             setAssetTagsManifest(void* obj, const char* index, const char* data)     { static_cast<ModuleDefinition*>(obj)->setAssetTagsManifest(data); return false; }
    static bool             writeAssetTagsManifest(void* obj, StringTableEntry pFieldName) { return static_cast<ModuleDefinition*>(obj)->getAssetTagsManifest() != StringTable->EmptyString(); }
    static const char*      getScopeSet(void* obj, const char* data)                    { return Con::getIntArg(static_cast<ModuleDefinition*>(obj)->getScopeSet()); }

    static bool             setDependencies(void* obj, const char* index, const char* data)
    {
        // Fetch module dependencies.
        ModuleDefinition::typeModuleDependencyVector moduleDependencies;

        // Fetch dependency value.
        const char* pDependencyValue = data;

        char slotUnit[256];
        char slotName[256];
        char slotValue[256];

        // Fetch definition word count.
        const U32 dependencyWordCount = StringUnit::getUnitCount( pDependencyValue, "," );

        // Do we have any dependencies specified?
        if ( dependencyWordCount > 0 )
        {
            // Yes, so iterate dependencies.
            for ( U32 dependencyIndex = 0; dependencyIndex < dependencyWordCount; ++dependencyIndex )
            {
                // Fetch slot.
                dStrcpy( slotUnit, StringUnit::getUnit( pDependencyValue, dependencyIndex, "," ) );
        
                // Fetch slot name and value.
                dStrcpy( slotName, StringUnit::getUnit( slotUnit, 0, "=" ) );
                dStrcpy( slotValue, StringUnit::getUnit( slotUnit, 1, "=" ) );

                // Fetch module Id.
                StringTableEntry moduleId = StringTable->insert( slotName );

                // Fetch version Id.
                const U32 versionId = slotValue[0] == '*' ? 0 : dAtoi(slotValue);

                // Populate module dependency.
                ModuleDefinition::ModuleDependency dependency( moduleId, versionId );

                // Store dependency.
                moduleDependencies.push_back( dependency );
            }
        }

        // Set dependencies.
        static_cast<ModuleDefinition*>(obj)->setDependencies( moduleDependencies );

        return false;
    }
    static const char*      getDependencies(void* obj, const char* data)
    {
        // Fetch module dependencies.
        const ModuleDefinition::typeModuleDependencyVector& moduleDependencies = static_cast<ModuleDefinition*>(obj)->getDependencies();

        // Finish if no dependencies.
        if ( moduleDependencies.size() == 0 )
            return StringTable->EmptyString();

        // Get a return buffer.
        const S32 bufferSize = 1024;
        char* pReturnBuffer = Con::getReturnBuffer(bufferSize);
        pReturnBuffer[0] = '\0';

        // Set buffer limits.
        char* pValueBuffer = pReturnBuffer;
        S32 bufferLeft = bufferSize;
        U32 used;

        // Iterate module dependencies.
        for ( ModuleDefinition::typeModuleDependencyVector::const_iterator dependencyItr = moduleDependencies.begin(); dependencyItr < moduleDependencies.end(); ++dependencyItr )
        {
            // Fetch module dependency.
            const ModuleDefinition::ModuleDependency* pDependency = dependencyItr;

            // Fetch version Id.
            const char* pVersionId = pDependency->mVersionId == 0 ? "*" : avar("%d", pDependency->mVersionId );
           
            if ( dependencyItr == moduleDependencies.begin() )
            {
                // Write out a field/value pair
                used = dSprintf( pValueBuffer, bufferLeft, "%s=%s", pDependency->mModuleId, pVersionId );
                pValueBuffer += used;
                bufferLeft -= used;
            }
            else
            {
                // Write out a field/value pair
                used = dSprintf( pValueBuffer, bufferLeft, ",%s=%s", pDependency->mModuleId, pVersionId );
                pValueBuffer += used;
                bufferLeft -= used;
            }

            // Sanity.
            AssertFatal( bufferLeft > 0, "Cannot format module dependencies as we ran out of buffer." );      
        }

        return pReturnBuffer;
    }
    static bool             writeDependencies( void* obj, StringTableEntry pFieldName ) { return static_cast<ModuleDefinition*>(obj)->getDependencies().size() > 0; }
    static const char*      getSignature(void* obj, const char* data)                   { return static_cast<ModuleDefinition*>(obj)->getSignature(); }
};

#endif // _MODULE_DEFINITION_H

