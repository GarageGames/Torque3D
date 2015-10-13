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

#ifndef _MODULE_MANAGER_H
#define _MODULE_MANAGER_H

#ifndef _SIMBASE_H_
#include "console/simBase.h"
#endif

#ifndef _TVECTOR_H_
#include "core/util/tvector.h"
#endif

#ifndef _TDICTIONARY_H_
#include "core/util/tDictionary.h"
#endif

#ifndef _TAML_H_
#include "persistence/taml/taml.h"
#endif

#ifndef _MODULE_DEFINITION_H
#include "moduleDefinition.h"
#endif

//-----------------------------------------------------------------------------

#define MODULE_MANAGER_MERGE_FILE                   "module.merge"
#define MODULE_MANAGER_MODULE_DEFINITION_EXTENSION  "module.taml"

//-----------------------------------------------------------------------------

/// @ingroup moduleGroup
/// @see moduleGroup
class ModuleManager : public SimObject
{
private:
    typedef SimObject Parent;

public:
    /// Module definitions.
    typedef Vector<ModuleDefinition*> typeModuleDefinitionVector;
    typedef Vector<const ModuleDefinition*> typeConstModuleDefinitionVector;

private:
    /// Database locking.
    struct LockDatabase
    {
    public:
        LockDatabase( ModuleManager* pManager ) :
          mpManager( pManager )
          {
              mpManager->mDatabaseLocks++;
          }

        ~LockDatabase()
        {
            mpManager->mDatabaseLocks--;

            // Sanity!
            AssertFatal( mpManager->mDatabaseLocks >= 0, "Module Manager: Cannot unlock database as it is already unlocked." );
        }

    private:
        ModuleManager* mpManager;
    };

    /// Loaded module entry.
    struct ModuleLoadEntry
    {
        ModuleLoadEntry( ModuleDefinition* pModuleDefinition, const bool strictVersionId ) :
            mpModuleDefinition( pModuleDefinition ),
            mStrictVersionId( strictVersionId )
        {
        }

        ModuleLoadEntry()
        {
           mpModuleDefinition = NULL;
           mStrictVersionId = false;
        }

        ModuleDefinition*   mpModuleDefinition;
        bool                mStrictVersionId;
    };

    /// Module loading.
    typedef Vector<StringTableEntry> typeModuleIdVector;
    typedef Vector<StringTableEntry> typeGroupVector;
    typedef HashMap<StringTableEntry, typeModuleIdVector*> typeGroupModuleHash;
    typedef Vector<ModuleLoadEntry> typeModuleLoadEntryVector;
    typeGroupModuleHash         mGroupModules;
    typeGroupVector             mGroupsLoaded;
    typeModuleLoadEntryVector   mModulesLoaded;

    /// Miscellaneous.
    bool                        mEnforceDependencies;
    bool                        mEchoInfo;
    S32                         mDatabaseLocks;
    char                        mModuleExtension[256];
    Taml                        mTaml;
    SimSet                      mNotificationListeners;

    // Module definition entry.
    struct ModuleDefinitionEntry : public typeModuleDefinitionVector
    {
    public:
        ModuleDefinitionEntry( StringTableEntry moduleId, StringTableEntry moduleGroup, StringTableEntry moduleType ) :
            mModuleId( moduleId ),
            mModuleGroup( moduleGroup ),
            mModuleType( moduleType )
        {
        }

        const StringTableEntry  mModuleId;
        const StringTableEntry  mModuleGroup;
        const StringTableEntry  mModuleType;
    };

    /// Module databases.
    typedef HashMap<StringTableEntry, ModuleDefinitionEntry*> typeModuleIdDatabaseHash;
    typeModuleIdDatabaseHash mModuleIdDatabase;

public:
    ModuleManager();
    virtual ~ModuleManager() {}

    /// SimObject overrides
    virtual bool onAdd();
    virtual void onRemove();
    virtual void onDeleteNotify( SimObject *object );
    static void initPersistFields();

    /// Declare Console Object.
    DECLARE_CONOBJECT( ModuleManager );

    /// Module definitions.
    bool setModuleExtension( const char* pExtension );

    /// Module discovery.
    bool scanModules( const char* pPath, const bool rootOnly = false );

    /// Module unregister.
    bool unregisterModule( const char* pModuleId, const U32 versionId );

    /// Module (un)loading.
    bool loadModuleGroup( const char* pModuleGroup );
    bool unloadModuleGroup( const char* pModuleGroup );
    bool loadModuleExplicit( const char* pModuleId, const U32 versionId = 0 );
    bool unloadModuleExplicit( const char* pModuleId );

    /// Module type enumeration.
    ModuleDefinition* findModule( const char* pModuleId, const U32 versionId );
    ModuleDefinition* findLoadedModule( const char* pModuleId );
    void findModules( const bool loadedOnly, typeConstModuleDefinitionVector& moduleDefinitions );
    void findModuleTypes( const char* pModuleType, const bool loadedOnly, typeConstModuleDefinitionVector& moduleDefinitions );

    /// Module synchronization.
    StringTableEntry copyModule( ModuleDefinition* pSourceModuleDefinition, const char* pTargetModuleId, const char* pTargetPath, const bool useVersionPathing );
    bool synchronizeDependencies( ModuleDefinition* pRootModuleDefinition, const char* pTargetDependencyPath );

    /// Module updates.
    inline bool isModuleMergeAvailable( void ) const { return Platform::isFile( getModuleMergeFilePath() ); }
    bool canMergeModules( const char* pMergeSourcePath );
    bool mergeModules( const char* pMergeTargetPath, const bool removeMergeDefinition, const bool registerNewModules );

    /// Module notifications.
    void addListener( SimObject* pListener );
    void removeListener( SimObject* pListener );

private:
    void clearDatabase( void );
    bool removeModuleDefinition( ModuleDefinition* pModuleDefinition );
    bool registerModule( const char* pModulePath, const char* pModuleFile );

    void raiseModulePreLoadNotifications( ModuleDefinition* pModuleDefinition );
    void raiseModulePostLoadNotifications( ModuleDefinition* pModuleDefinition );
    void raiseModulePreUnloadNotifications( ModuleDefinition* pModuleDefinition );
    void raiseModulePostUnloadNotifications( ModuleDefinition* pModuleDefinition );

    ModuleDefinitionEntry* findModuleId( StringTableEntry moduleId );
    ModuleDefinitionEntry::iterator findModuleDefinition( StringTableEntry moduleId, const U32 versionId );
    bool resolveModuleDependencies( StringTableEntry moduleId, const U32 versionId, StringTableEntry moduleGroup, bool synchronizedOnly, typeModuleLoadEntryVector& moduleResolvingQueue, typeModuleLoadEntryVector& moduleReadyQueue );
    ModuleLoadEntry* findModuleResolving( StringTableEntry moduleId, typeModuleLoadEntryVector& moduleResolvingQueue );
    ModuleLoadEntry* findModuleReady( StringTableEntry moduleId, typeModuleLoadEntryVector& moduleReadyQueue );
    typeModuleLoadEntryVector::iterator findModuleLoaded( StringTableEntry moduleId, const U32 versionId = 0 );
    typeGroupVector::iterator findGroupLoaded( StringTableEntry moduleGroup );
    StringTableEntry getModuleMergeFilePath( void ) const;
};

//-----------------------------------------------------------------------------

extern ModuleManager ModuleDatabase;

#endif // _MODULE_MANAGER_H