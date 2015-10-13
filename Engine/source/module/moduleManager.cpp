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

#include "moduleManager.h"

#ifndef _MODULE_MERGE_DEFINITION_H
#include "moduleMergeDefinition.h"
#endif

#ifndef _TAML_MODULE_ID_UPDATE_VISITOR_H_
#include "tamlModuleIdUpdateVisitor.h"
#endif

#ifndef _MODULE_CALLBACKS_H_
#include "moduleCallbacks.h"
#endif

#ifndef _CONSOLETYPES_H_
#include "console/consoleTypes.h"
#endif

// Script bindings.
#include "moduleManager_ScriptBinding.h"

//-----------------------------------------------------------------------------

IMPLEMENT_CONOBJECT( ModuleManager );

//-----------------------------------------------------------------------------

ModuleManager ModuleDatabase;

//-----------------------------------------------------------------------------

S32 QSORT_CALLBACK moduleDefinitionVersionIdSort( const void* a, const void* b )
{
    // Fetch module definitions.
   ModuleDefinition* pDefinition1 = *(ModuleDefinition**)a;
   ModuleDefinition* pDefinition2 = *(ModuleDefinition**)b;

   // Fetch version Ids.
   const U32 versionId1 = pDefinition1->getVersionId();
   const U32 versionId2 = pDefinition2->getVersionId();

   // We sort higher version Id first.
   return versionId1 > versionId2 ? -1 : versionId1 < versionId2 ? 1 : 0;
}

//-----------------------------------------------------------------------------

ModuleManager::ModuleManager() :
    mEnforceDependencies(true),
    mEchoInfo(true),
    mDatabaseLocks( 0 )
{
    // Set module extension.
    dStrcpy( mModuleExtension, MODULE_MANAGER_MODULE_DEFINITION_EXTENSION );
}

//-----------------------------------------------------------------------------

bool ModuleManager::onAdd()
{
    if( !Parent::onAdd() )
        return false;

    // Register listeners.
    mNotificationListeners.registerObject();

    return true;
}

//-----------------------------------------------------------------------------

void ModuleManager::onRemove()
{
    // Clear database.
    clearDatabase();

    // Unregister object.
    mNotificationListeners.unregisterObject();

    // Call parent.
    Parent::onRemove();
}

//-----------------------------------------------------------------------------

void ModuleManager::initPersistFields()
{
    // Call parent.
    Parent::initPersistFields();

    addField( "EnforceDependencies", TypeBool, Offset(mEnforceDependencies, ModuleManager), "Whether the module manager enforces any dependencies on module definitions it discovers or not." );
    addField( "EchoInfo", TypeBool, Offset(mEchoInfo, ModuleManager), "Whether the module manager echos extra information to the console or not." );
}

//-----------------------------------------------------------------------------

void ModuleManager::onDeleteNotify( SimObject *object )
{
    // Cast to a module definition.
    ModuleDefinition* pModuleDefinition = dynamic_cast<ModuleDefinition*>( object );

    // Ignore if not appropriate.
    if ( pModuleDefinition == NULL )
        return;

    // Warn.
    Con::warnf( "Module Manager::onDeleteNotify() - Notified of a module definition deletion for module Id '%s' of version Id '%d' however this should not happen and can cause module database corruption.",
        pModuleDefinition->getModuleId(), pModuleDefinition->getVersionId() );
}

//-----------------------------------------------------------------------------

bool ModuleManager::setModuleExtension( const char* pExtension )
{
    // Sanity!
    AssertFatal( pExtension != NULL, "Cannot set module extension with NULL extension." );

    // Did we find an extension period?
    if ( *pExtension == '.' )
    {
        // Yes, so warn.
        Con::warnf("Module Manager: Failed to set extension as supplied extension contains an initial period: '%s'.", pExtension );
        return false;
    }

    // Is the extension too large?
    if ( dStrlen( pExtension ) > sizeof( mModuleExtension ) )
    {
        // Yes, so warn.
        Con::warnf("Module Manager: Failed to set extension as supplied extension is too large: '%s'.", pExtension );
        return false;
    }

    // Set module extension.
    dStrcpy( mModuleExtension, pExtension );

    return true;
}

//-----------------------------------------------------------------------------

bool ModuleManager::scanModules( const char* pPath, const bool rootOnly )
{
    // Lock database.
    LockDatabase( this );

    // Sanity!
    AssertFatal( pPath != NULL, "Cannot scan module with NULL path." );

    // Expand module location.
    char pathBuffer[1024];
    Con::expandPath( pathBuffer, sizeof(pathBuffer), pPath );

    // Info.
    if ( mEchoInfo )
    {
        Con::printSeparator();
        Con::printf( "Module Manager: Started scanning '%s'...", pathBuffer );
    }

    Vector<StringTableEntry> directories;

    // Find directories.
    if ( !Platform::dumpDirectories( pathBuffer, directories, rootOnly ? 1 : -1 ) )
    {
        // Failed so warn.
        Con::warnf( "Module Manager: Failed to scan module directories in path '%s'.", pathBuffer );
        return false;
    }

    // Fetch extension length.
    const U32 extensionLength = dStrlen( mModuleExtension );

    Vector<Platform::FileInfo> files;

    // Iterate directories.
    for( Vector<StringTableEntry>::iterator basePathItr = directories.begin(); basePathItr != directories.end(); ++basePathItr )
    {
        // Fetch base path.
        StringTableEntry basePath = *basePathItr;

        // Skip if we're only processing the root and this is not the root.
        if ( rootOnly && basePathItr != directories.begin() )
            continue;

        // Find files.
        files.clear();
        if ( !Platform::dumpPath( basePath, files, 0 ) )
        {
            // Failed so warn.
            Con::warnf( "Module Manager: Failed to scan modules files in directory '%s'.", basePath );
            return false;
        }

        // Iterate files.
        for ( Vector<Platform::FileInfo>::iterator fileItr = files.begin(); fileItr != files.end(); ++fileItr )
        {
            // Fetch file info.
            Platform::FileInfo* pFileInfo = fileItr;

            // Fetch filename.
            const char* pFilename = pFileInfo->pFileName;

            // Find filename length.
            const U32 filenameLength = dStrlen( pFilename );

            // Skip if extension is longer than filename.
            if ( extensionLength > filenameLength )
                continue;

            // Skip if extension not found.
            if ( dStricmp( pFilename + filenameLength - extensionLength, mModuleExtension ) != 0 )
                continue;

            // Register module.
            registerModule( basePath, pFileInfo->pFileName );
        }

        // Stop processing if we're only processing the root.
        if ( rootOnly )
            break;
    }

    // Info.
    if ( mEchoInfo )
    {
        Con::printf( "Module Manager: Finished scanning '%s'.", pathBuffer );
    }

    return true;
}

//-----------------------------------------------------------------------------

bool ModuleManager::loadModuleGroup( const char* pModuleGroup )
{
    // Lock database.
    LockDatabase( this );

    // Sanity!
    AssertFatal( pModuleGroup != NULL, "Cannot load module group with NULL group name." );

    typeModuleLoadEntryVector   moduleResolvingQueue;
    typeModuleLoadEntryVector   moduleReadyQueue;

    // Fetch module group.
    StringTableEntry moduleGroup = StringTable->insert( pModuleGroup );

    // Info.
    if ( mEchoInfo )
    {
        Con::printSeparator();
        Con::printf( "Module Manager: Loading group '%s':" ,moduleGroup );
    }

    // Is the module group already loaded?
    if ( findGroupLoaded( moduleGroup ) != NULL )
    {
        // Yes, so warn.
        Con::warnf( "Module Manager: Cannot load group '%s' as it is already loaded.", moduleGroup );
        return false;
    }

    // Find module group.
    typeGroupModuleHash::iterator moduleGroupItr = mGroupModules.find( moduleGroup );

    // Did we find the module group?
    if ( moduleGroupItr == mGroupModules.end() )
    {
        // No, so info.
        if ( mEchoInfo )
        {
            Con::printf( "Module Manager: No modules found for module group '%s'.", moduleGroup );
        }
        
        return true;
    }

    // Yes, so fetch the module Ids.
    typeModuleIdVector* pModuleIds = moduleGroupItr->value;

    // Iterate module groups.
    for( typeModuleIdVector::iterator moduleIdItr = pModuleIds->begin(); moduleIdItr != pModuleIds->end(); ++moduleIdItr )
    {
        // Fetch module Id.
        StringTableEntry moduleId = *moduleIdItr;

        // Finish if we could not resolve the dependencies for module Id (of any version Id).
        if ( !resolveModuleDependencies( moduleId, 0, moduleGroup, false, moduleResolvingQueue, moduleReadyQueue ) )
            return false;
    }

    // Check the modules we want to load to ensure that we do not have incompatible modules loaded already.
    for ( typeModuleLoadEntryVector::iterator moduleReadyItr = moduleReadyQueue.begin(); moduleReadyItr != moduleReadyQueue.end(); ++moduleReadyItr )
    {
        // Fetch load ready module definition.
        ModuleDefinition* pLoadReadyModuleDefinition = moduleReadyItr->mpModuleDefinition;;

        // Fetch the module Id loaded entry.
        ModuleLoadEntry* pLoadedModuleEntry = findModuleLoaded( pLoadReadyModuleDefinition->getModuleId() );

        // Did we find a loaded entry?
        if ( pLoadedModuleEntry != NULL )
        {
            // Yes, so is it the one we need to load?
            if ( pLoadedModuleEntry->mpModuleDefinition != pLoadReadyModuleDefinition )
            {
                // Yes, so warn.
                Con::warnf( "Module Manager: Cannot load module group '%s' as the module Id '%s' at version Id '%d' is required but the module Id is already loaded but at version Id '%d'.",
                    moduleGroup, pLoadReadyModuleDefinition->getModuleId(), pLoadReadyModuleDefinition->getVersionId(), pLoadedModuleEntry->mpModuleDefinition->getVersionId() );
                return false;
            }
        }
    }

    // Info.
    if ( mEchoInfo )
    {
        // Info.
        Con::printf( "Module Manager: Group '%s' and its dependencies is comprised of the following '%d' module(s):", moduleGroup, moduleReadyQueue.size() );

        // Iterate the modules echoing them.
        for ( typeModuleLoadEntryVector::iterator moduleReadyItr = moduleReadyQueue.begin(); moduleReadyItr != moduleReadyQueue.end(); ++moduleReadyItr )
        {
            // Fetch the ready entry.
            ModuleDefinition* pModuleDefinition = moduleReadyItr->mpModuleDefinition;

            // Info.
            Con::printf( "> module Id '%s' at version Id '%d':", pModuleDefinition->getModuleId(), pModuleDefinition->getVersionId() );
        }
    }

    // Add module group.
    mGroupsLoaded.push_back( moduleGroup );

    // Reset modules loaded count.
    U32 modulesLoadedCount = 0;

    // Iterate the modules, executing their script files and call their create function.
    for ( typeModuleLoadEntryVector::iterator moduleReadyItr = moduleReadyQueue.begin(); moduleReadyItr != moduleReadyQueue.end(); ++moduleReadyItr )
    {
        // Fetch the ready entry.
        ModuleLoadEntry* pReadyEntry = moduleReadyItr;

        // Fetch load ready module definition.
        ModuleDefinition* pLoadReadyModuleDefinition = pReadyEntry->mpModuleDefinition;

        // Fetch any loaded entry for the module Id.
        ModuleLoadEntry* pLoadedEntry = findModuleLoaded( pLoadReadyModuleDefinition->getModuleId() );

        // Is the module already loaded.
        if ( pLoadedEntry != NULL )
        {
            // Yes, so increase load count.
            pLoadedEntry->mpModuleDefinition->increaseLoadCount();

            // Skip.
            continue;
        }

        // No, so info.
        if ( mEchoInfo )
        {
            Con::printSeparator();
            Con::printf( "Module Manager: Loading group '%s' : module Id '%s' at version Id '%d' in group '%s' using the script file '%s'.",
                moduleGroup, pLoadReadyModuleDefinition->getModuleId(), pLoadReadyModuleDefinition->getVersionId(), pLoadReadyModuleDefinition->getModuleGroup(), pLoadReadyModuleDefinition->getModuleScriptFilePath() );
        }

        // Is the module deprecated?
        if ( pLoadReadyModuleDefinition->getDeprecated() )
        {
            // Yes, so warn.
            Con::warnf( "Module Manager: Caution: module Id '%s' at version Id '%d' in group '%s' is deprecated.  You should use a newer version!",
                pLoadReadyModuleDefinition->getModuleId(), pLoadReadyModuleDefinition->getVersionId(), pLoadReadyModuleDefinition->getModuleGroup() );
        }

        // Add the path expando for module.
        Con::addPathExpando( pLoadReadyModuleDefinition->getModuleId(), pLoadReadyModuleDefinition->getModulePath() );

        // Create a scope set.
        SimSet* pScopeSet = new SimSet;
        pScopeSet->registerObject( pLoadReadyModuleDefinition->getModuleId() );
        pReadyEntry->mpModuleDefinition->mScopeSet = pScopeSet->getId();

        // Increase load count.
        pReadyEntry->mpModuleDefinition->increaseLoadCount();

        // Queue module loaded.
        mModulesLoaded.push_back( *pReadyEntry );

        // Bump modules loaded count.
        modulesLoadedCount++;

        // Raise notifications.
        raiseModulePreLoadNotifications( pLoadReadyModuleDefinition );

        // Do we have a script file-path specified?
        if ( pLoadReadyModuleDefinition->getModuleScriptFilePath() != StringTable->EmptyString() )
        {
            // Yes, so execute the script file.
            const bool scriptFileExecuted = dAtob( Con::executef("exec", pLoadReadyModuleDefinition->getModuleScriptFilePath() ) );

            // Did we execute the script file?
            if ( scriptFileExecuted )
            {
                // Yes, so is the create method available?
                if ( pScopeSet->isMethod( pLoadReadyModuleDefinition->getCreateFunction() ) )
                {
                    // Yes, so call the create method.
                    Con::executef( pScopeSet, pLoadReadyModuleDefinition->getCreateFunction() );
                }
            }
            else
            {
                // No, so warn.
                Con::errorf( "Module Manager: Cannot load module group '%s' as the module Id '%s' at version Id '%d' as it failed to have the script file '%s' loaded.",
                    moduleGroup, pLoadReadyModuleDefinition->getModuleId(), pLoadReadyModuleDefinition->getVersionId(), pLoadReadyModuleDefinition->getModuleScriptFilePath() );
            }
        }

        // Raise notifications.
        raiseModulePostLoadNotifications( pLoadReadyModuleDefinition );
    }

    // Info.
    if ( mEchoInfo )
    {
        Con::printSeparator();
        Con::printf( "Module Manager: Finish loading '%d' module(s) for group '%s'.", modulesLoadedCount, moduleGroup );
        Con::printSeparator();
    }

    return true;
}

//-----------------------------------------------------------------------------

bool ModuleManager::unloadModuleGroup( const char* pModuleGroup )
{
    // Lock database.
    LockDatabase( this );

    // Sanity!
    AssertFatal( pModuleGroup != NULL, "Cannot unload module group with NULL group name." );

    typeModuleLoadEntryVector   moduleResolvingQueue;
    typeModuleLoadEntryVector   moduleReadyQueue;

    // Fetch module group.
    StringTableEntry moduleGroup = StringTable->insert( pModuleGroup );

    // Info.
    if ( mEchoInfo )
    {
        Con::printSeparator();
        Con::printf( "Module Manager: Unloading group '%s':" , moduleGroup );
    }

    // Find the group loaded iterator.
    typeGroupVector::iterator groupLoadedItr = findGroupLoaded( moduleGroup );

    // Is the module group already unloaded?
    if ( groupLoadedItr == NULL )
    {
        // No, so warn.
        Con::warnf( "Module Manager: Cannot unload group '%s' as it is not loaded.", moduleGroup );
        return false;
    }

    // Find module group.
    typeGroupModuleHash::iterator moduleGroupItr = mGroupModules.find( moduleGroup );

    // Did we find the module group?
    if ( moduleGroupItr == mGroupModules.end() )
    {
        // No, so info.
        if ( mEchoInfo )
        {
            Con::printf( "Module Manager: No modules found for module group '%s'.", moduleGroup );
            return true;
        }
    }

    // Yes, so fetch the module Ids.
    typeModuleIdVector* pModuleIds = moduleGroupItr->value;

    // Iterate module groups.
    for( typeModuleIdVector::iterator moduleIdItr = pModuleIds->begin(); moduleIdItr != pModuleIds->end(); ++moduleIdItr )
    {
        // Fetch module Id.
        StringTableEntry moduleId = *moduleIdItr;

        // Finish if we could not resolve the dependencies for module Id (of any version Id).
        if ( !resolveModuleDependencies( moduleId, 0, moduleGroup, false, moduleResolvingQueue, moduleReadyQueue ) )
            return false;
    }

    // Check the modules we want to load to ensure that we do not have incompatible modules loaded already.
    for ( typeModuleLoadEntryVector::iterator moduleReadyItr = moduleReadyQueue.begin(); moduleReadyItr != moduleReadyQueue.end(); ++moduleReadyItr )
    {
        // Fetch load ready module definition.
        ModuleDefinition* pLoadReadyModuleDefinition = moduleReadyItr->mpModuleDefinition;;

        // Fetch the module Id loaded entry.
        ModuleLoadEntry* pLoadedModuleEntry = findModuleLoaded( pLoadReadyModuleDefinition->getModuleId() );

        // Did we find a loaded entry?
        if ( pLoadedModuleEntry != NULL )
        {
            // Yes, so is it the one we need to load?
            if ( pLoadedModuleEntry->mpModuleDefinition != pLoadReadyModuleDefinition )
            {
                // Yes, so warn.
                Con::warnf( "Module Manager: Cannot unload module group '%s' as the module Id '%s' at version Id '%d' is required but the module Id is loaded but at version Id '%d'.",
                    moduleGroup, pLoadReadyModuleDefinition->getModuleId(), pLoadReadyModuleDefinition->getVersionId(), pLoadedModuleEntry->mpModuleDefinition->getVersionId() );
                return false;
            }
        }
    }

    // Remove module group.
    mGroupsLoaded.erase_fast( groupLoadedItr );

    // Reset modules unloaded count.
    U32 modulesUnloadedCount = 0;

    // Iterate the modules in reverse order calling their destroy function.
    for ( typeModuleLoadEntryVector::iterator moduleReadyItr = moduleReadyQueue.end()-1; moduleReadyItr >= moduleReadyQueue.begin(); --moduleReadyItr )
    {
        // Fetch the ready entry.
        ModuleLoadEntry* pReadyEntry = moduleReadyItr;

        // Fetch load ready module definition.
        ModuleDefinition* pLoadReadyModuleDefinition = pReadyEntry->mpModuleDefinition;;

        // Fetch any loaded entry for the module Id.
        ModuleLoadEntry* pLoadedEntry = findModuleLoaded( pLoadReadyModuleDefinition->getModuleId() );

        // Is the module loaded.
        if ( pLoadedEntry == NULL )
        {
            // No, so warn.
            if ( mEchoInfo )
            {
                Con::printf( "Module Manager: Unloading group '%s' but could not unload module Id '%s' at version Id '%d'.",
                    moduleGroup, pLoadedEntry->mpModuleDefinition->getModuleId(), pLoadedEntry->mpModuleDefinition->getVersionId() );
            }
            // Skip.
            continue;
        }

        // Reduce load count.
        pLoadedEntry->mpModuleDefinition->reduceLoadCount();

        // Sanity!
        AssertFatal( pLoadedEntry->mpModuleDefinition->getLoadCount() >= 0, "ModuleManager::unloadModuleGroup() - Encountered an invalid load count." );

        // Do we need to unload?
        if ( pLoadedEntry->mpModuleDefinition->getLoadCount() == 0 )
        {
            // Yes, so info.
            if ( mEchoInfo )
            {
                Con::printSeparator();
                Con::printf( "Module Manager: Unload group '%s' with module Id '%s' at version Id '%d' in group '%s'.",
                    moduleGroup, pLoadReadyModuleDefinition->getModuleId(), pLoadReadyModuleDefinition->getVersionId(), pLoadReadyModuleDefinition->getModuleGroup() );
            }

            // Raise notifications.
            raiseModulePreUnloadNotifications( pLoadReadyModuleDefinition );

            // Fetch the module Id loaded entry.
            typeModuleLoadEntryVector::iterator moduleLoadedItr = findModuleLoaded( pLoadReadyModuleDefinition->getModuleId() );

            // Sanity!
            AssertFatal( moduleLoadedItr != NULL, "ModuleManager::unloadModuleGroup() - Cannot find module to unload it." );

            // Dequeue module loaded.
            mModulesLoaded.erase_fast( moduleLoadedItr );

            // Fetch scope set.
            SimSet* pScopeSet = dynamic_cast<SimSet*>(Sim::findObject(pLoadReadyModuleDefinition->mScopeSet));

            // Is the destroy method available?
            if ( pScopeSet->isMethod( pLoadReadyModuleDefinition->getDestroyFunction() ) )
            {
                // Yes, so call the destroy method.
                Con::executef( pScopeSet, pLoadReadyModuleDefinition->getDestroyFunction() );
            }

            // Remove scope set.
            pScopeSet->deleteAllObjects();
            pScopeSet->unregisterObject();
            pLoadReadyModuleDefinition->mScopeSet = 0;

            // Remove path expando for module.
            Con::removePathExpando( pLoadReadyModuleDefinition->getModuleId() );

            // Bump modules unloaded count.
            modulesUnloadedCount++;

            // Raise notifications.
            raiseModulePostUnloadNotifications( pLoadReadyModuleDefinition );
        }
    }

    // Info.
    if ( mEchoInfo )
    {
        Con::printSeparator();
        Con::printf( "Module Manager: Finish unloading '%d' module(s) for group '%s'.", modulesUnloadedCount, moduleGroup );
        Con::printSeparator();
    }

    return true;
}

//-----------------------------------------------------------------------------

bool ModuleManager::loadModuleExplicit( const char* pModuleId, const U32 versionId )
{
    // Lock database.
    LockDatabase( this );

    // Sanity!
    AssertFatal( pModuleId != NULL, "Cannot load explicit module Id with NULL module Id." );

    typeModuleLoadEntryVector   moduleResolvingQueue;
    typeModuleLoadEntryVector   moduleReadyQueue;

    // Fetch module Id.
    StringTableEntry moduleId = StringTable->insert( pModuleId );

    // Fetch modules definitions.
    ModuleDefinitionEntry* pDefinitions = findModuleId( moduleId );

    // Did we find the module Id?
    if ( pDefinitions == NULL )
    {
        // No, so warn.
        Con::warnf( "Module Manager: Cannot load explicit module Id '%s' as it does not exist.", moduleId );
        return false;
    }

    // Fetch module group.
    StringTableEntry moduleGroup = pDefinitions->mModuleGroup;

    // Info.
    if ( mEchoInfo )
    {
        Con::printSeparator();
        Con::printf( "Module Manager: Loading explicit module Id '%s' at version Id '%d':", moduleId, versionId );
    }

    // Finish if we could not resolve the dependencies for module Id (of any version Id).
    if ( !resolveModuleDependencies( moduleId, versionId, moduleGroup, false, moduleResolvingQueue, moduleReadyQueue ) )
        return false;

    // Check the modules we want to load to ensure that we do not have incompatible modules loaded already.
    for ( typeModuleLoadEntryVector::iterator moduleReadyItr = moduleReadyQueue.begin(); moduleReadyItr != moduleReadyQueue.end(); ++moduleReadyItr )
    {
        // Fetch load ready module definition.
        ModuleDefinition* pLoadReadyModuleDefinition = moduleReadyItr->mpModuleDefinition;

        // Fetch the module Id loaded entry.
        ModuleLoadEntry* pLoadedModuleEntry = findModuleLoaded( pLoadReadyModuleDefinition->getModuleId() );

        // Did we find a loaded entry?
        if ( pLoadedModuleEntry != NULL )
        {
            // Yes, so is it the one we need to load?
            if ( pLoadedModuleEntry->mpModuleDefinition != pLoadReadyModuleDefinition )
            {
                // Yes, so warn.
                Con::warnf( "Module Manager: Cannot load explicit module Id '%s' at version Id '%d' as the module Id is already loaded but at version Id '%d'.",
                    pLoadReadyModuleDefinition->getModuleId(), pLoadReadyModuleDefinition->getVersionId(), pLoadedModuleEntry->mpModuleDefinition->getVersionId() );
                return false;
            }
        }
    }

    // Info.
    if ( mEchoInfo )
    {
        // Info.
        Con::printf( "Module Manager: Explicit load of module Id '%s' at version Id '%d' and its dependencies is comprised of the following '%d' module(s):", moduleId, versionId, moduleReadyQueue.size() );

        // Iterate the modules echoing them.
        for ( typeModuleLoadEntryVector::iterator moduleReadyItr = moduleReadyQueue.begin(); moduleReadyItr != moduleReadyQueue.end(); ++moduleReadyItr )
        {
            // Fetch the ready entry.
            ModuleDefinition* pModuleDefinition = moduleReadyItr->mpModuleDefinition;

            // Info.
            Con::printf( "> module Id '%s' at version Id '%d'", pModuleDefinition->getModuleId(), pModuleDefinition->getVersionId() );
        }
    }

    // Reset modules loaded count.
    U32 modulesLoadedCount = 0;

    // Iterate the modules, executing their script files and call their create function.
    for ( typeModuleLoadEntryVector::iterator moduleReadyItr = moduleReadyQueue.begin(); moduleReadyItr != moduleReadyQueue.end(); ++moduleReadyItr )
    {
        // Fetch the ready entry.
        ModuleLoadEntry* pReadyEntry = moduleReadyItr;

        // Fetch load ready module definition.
        ModuleDefinition* pLoadReadyModuleDefinition = pReadyEntry->mpModuleDefinition;

        // Fetch any loaded entry for the module Id.
        ModuleLoadEntry* pLoadedEntry = findModuleLoaded( pLoadReadyModuleDefinition->getModuleId() );

        // Is the module already loaded.
        if ( pLoadedEntry != NULL )
        {
            // Yes, so increase load count.
            pLoadedEntry->mpModuleDefinition->increaseLoadCount();

            // Skip.
            continue;
        }

        // No, so info.
        if ( mEchoInfo )
        {
            Con::printSeparator();
            Con::printf( "Module Manager: Loading explicit module Id '%s' at version Id '%d' using the script file '%s'.",
                pLoadReadyModuleDefinition->getModuleId(), pLoadReadyModuleDefinition->getVersionId(), pLoadReadyModuleDefinition->getModuleScriptFilePath() );
        }

        // Is the module deprecated?
        if ( pLoadReadyModuleDefinition->getDeprecated() )
        {
            // Yes, so warn.
            Con::warnf( "Module Manager: Caution: module Id '%s' at version Id '%d' is deprecated,  You should use a newer version!",
                pLoadReadyModuleDefinition->getModuleId(), pLoadReadyModuleDefinition->getVersionId() );
        }

        // Add the path expando for module.
        Con::addPathExpando( pLoadReadyModuleDefinition->getModuleId(), pLoadReadyModuleDefinition->getModulePath() );

        // Create a scope set.
        SimSet* pScopeSet = new SimSet;
        pScopeSet->registerObject( pLoadReadyModuleDefinition->getModuleId() );
        pReadyEntry->mpModuleDefinition->mScopeSet = pScopeSet->getId();

        // Increase load count.
        pReadyEntry->mpModuleDefinition->increaseLoadCount();

        // Queue module loaded.
        mModulesLoaded.push_back( *pReadyEntry );

        // Bump modules loaded count.
        modulesLoadedCount++;

        // Raise notifications.
        raiseModulePreLoadNotifications( pLoadReadyModuleDefinition );

        // Do we have a script file-path specified?
        if ( pLoadReadyModuleDefinition->getModuleScriptFilePath() != StringTable->EmptyString() )
        {
            // Yes, so execute the script file.
            const bool scriptFileExecuted = dAtob( Con::executef("exec", pLoadReadyModuleDefinition->getModuleScriptFilePath() ) );

            // Did we execute the script file?
            if ( scriptFileExecuted )
            {
                // Yes, so is the create method available?
                if ( pScopeSet->isMethod( pLoadReadyModuleDefinition->getCreateFunction() ) )
                {
                    // Yes, so call the create method.
                    Con::executef( pScopeSet, pLoadReadyModuleDefinition->getCreateFunction() );
                }
            }
            else
            {
                // No, so warn.
                Con::errorf( "Module Manager: Cannot load explicit module Id '%s' at version Id '%d' as it failed to have the script file '%s' loaded.",
                    pLoadReadyModuleDefinition->getModuleId(), pLoadReadyModuleDefinition->getVersionId(), pLoadReadyModuleDefinition->getModuleScriptFilePath() );
            }
        }

        // Raise notifications.
        raiseModulePostLoadNotifications( pLoadReadyModuleDefinition );
    }

    // Info.
    if ( mEchoInfo )
    {
        Con::printSeparator();
        Con::printf( "Module Manager: Finish loading '%d' explicit module(s).", modulesLoadedCount );
        Con::printSeparator();
    }

    return true;
}

//-----------------------------------------------------------------------------

bool ModuleManager::unloadModuleExplicit( const char* pModuleId )
{
    // Lock database.
    LockDatabase( this );

    // Sanity!
    AssertFatal( pModuleId != NULL, "Cannot unload explicit module Id with NULL module Id." );

    typeModuleLoadEntryVector   moduleResolvingQueue;
    typeModuleLoadEntryVector   moduleReadyQueue;

    // Fetch module Id.
    StringTableEntry moduleId = StringTable->insert( pModuleId );

    // Fetch modules definitions.
    ModuleDefinitionEntry* pDefinitions = findModuleId( moduleId );

    // Did we find the module Id?
    if ( pDefinitions == NULL )
    {
        // No, so warn.
        Con::warnf( "Module Manager: Cannot unload explicit module Id '%s' as it does not exist.", moduleId );
        return false;
    }

    // Find if the module is actually loaded.
    ModuleDefinition* pLoadedModule = findLoadedModule( moduleId );

    // Is the module loaded?
    if ( pLoadedModule == NULL )
    {
        // No, so warn.
        Con::warnf( "Module Manager: Cannot unload explicit module Id '%s' as it is not loaded.", moduleId );
        return false;
    }

    // Fetch module group.
    StringTableEntry moduleGroup = pDefinitions->mModuleGroup;

    // Info.
    if ( mEchoInfo )
    {
        Con::printSeparator();
        Con::printf( "Module Manager: Unloading explicit module Id '%s':" , moduleId );
    }

    // Finish if we could not resolve the dependencies for module Id (of any version Id).
    if ( !resolveModuleDependencies( moduleId, pLoadedModule->getVersionId(), moduleGroup, false, moduleResolvingQueue, moduleReadyQueue ) )
        return false;

    // Check the modules we want to unload to ensure that we do not have incompatible modules loaded already.
    for ( typeModuleLoadEntryVector::iterator moduleReadyItr = moduleReadyQueue.begin(); moduleReadyItr != moduleReadyQueue.end(); ++moduleReadyItr )
    {
        // Fetch load ready module definition.
        ModuleDefinition* pLoadReadyModuleDefinition = moduleReadyItr->mpModuleDefinition;;

        // Fetch the module Id loaded entry.
        ModuleLoadEntry* pLoadedModuleEntry = findModuleLoaded( pLoadReadyModuleDefinition->getModuleId() );

        // Did we find a loaded entry?
        if ( pLoadedModuleEntry != NULL )
        {
            // Yes, so is it the one we need to load?
            if ( pLoadedModuleEntry->mpModuleDefinition != pLoadReadyModuleDefinition )
            {
                // Yes, so warn.
                Con::warnf( "Module Manager: Cannot unload explicit module Id '%s' at version Id '%d' as the module Id is loaded but at version Id '%d'.",
                    pLoadReadyModuleDefinition->getModuleId(), pLoadReadyModuleDefinition->getVersionId(), pLoadedModuleEntry->mpModuleDefinition->getVersionId() );
                return false;
            }
        }
    }

    // Reset modules unloaded count.
    U32 modulesUnloadedCount = 0;

    // Iterate the modules in reverse order calling their destroy function.
    for ( typeModuleLoadEntryVector::iterator moduleReadyItr = moduleReadyQueue.end()-1; moduleReadyItr >= moduleReadyQueue.begin(); --moduleReadyItr )
    {
        // Fetch the ready entry.
        ModuleLoadEntry* pReadyEntry = moduleReadyItr;

        // Fetch load ready module definition.
        ModuleDefinition* pLoadReadyModuleDefinition = pReadyEntry->mpModuleDefinition;;

        // Fetch any loaded entry for the module Id.
        ModuleLoadEntry* pLoadedEntry = findModuleLoaded( pLoadReadyModuleDefinition->getModuleId() );

        // Is the module loaded.
        if ( pLoadedEntry == NULL )
        {
            // No, so warn.
            if ( mEchoInfo )
            {
                Con::printf( "Module Manager: Unloading explicit module Id '%s' at version Id '%d' but ignoring as it is not loaded.",
                    pLoadReadyModuleDefinition->getModuleId(), pLoadReadyModuleDefinition->getVersionId() );
            }

            // Skip.
            continue;
        }

        // Reduce load count.
        pLoadedEntry->mpModuleDefinition->reduceLoadCount();

        // Sanity!
        AssertFatal( pLoadedEntry->mpModuleDefinition->getLoadCount() >= 0, "ModuleManager::unloadModuleGroup() - Encountered an invalid load count." );

        // Do we need to unload?
        if ( pLoadedEntry->mpModuleDefinition->getLoadCount() == 0 )
        {
            // Yes, so info.
            if ( mEchoInfo )
            {
                Con::printSeparator();
                Con::printf( "Module Manager: Unload explicit module Id '%s' at version Id '%d'.",
                    pLoadReadyModuleDefinition->getModuleId(), pLoadReadyModuleDefinition->getVersionId() );
            }

            // Raise notifications.
            raiseModulePreUnloadNotifications( pLoadReadyModuleDefinition );

            // Fetch the module Id loaded entry.
            typeModuleLoadEntryVector::iterator moduleLoadedItr = findModuleLoaded( pLoadReadyModuleDefinition->getModuleId() );

            // Sanity!
            AssertFatal( moduleLoadedItr != NULL, "ModuleManager::unloadModuleExplicit() - Cannot find module to unload it." );

            // Dequeue module loaded.
            mModulesLoaded.erase_fast( moduleLoadedItr );

            // Fetch scope set.
            SimSet* pScopeSet = dynamic_cast<SimSet*>(Sim::findObject(pLoadReadyModuleDefinition->mScopeSet));

            // Is the destroy method available?
            if ( pScopeSet->isMethod( pLoadReadyModuleDefinition->getDestroyFunction() ) )
            {
                // Yes, so call the destroy method.
                Con::executef( pScopeSet, pLoadReadyModuleDefinition->getDestroyFunction() );
            }

            // Remove scope set.
            pScopeSet->deleteAllObjects();
            pScopeSet->unregisterObject();
            pLoadReadyModuleDefinition->mScopeSet = 0;

            // Remove path expando for module.
            Con::removePathExpando( pLoadReadyModuleDefinition->getModuleId() );

            // Bump modules unloaded count.
            modulesUnloadedCount++;

            // Raise notifications.
            raiseModulePostUnloadNotifications( pLoadReadyModuleDefinition );
        }
    }

    // Info.
    if ( mEchoInfo )
    {
        Con::printSeparator();
        Con::printf( "Module Manager: Finish unloading '%d' explicit module(s).", modulesUnloadedCount );
        Con::printSeparator();
    }

    return true;
}

//-----------------------------------------------------------------------------

ModuleDefinition* ModuleManager::findModule( const char* pModuleId, const U32 versionId )
{
    // Sanity!
    AssertFatal( pModuleId != NULL, "Cannot find module with NULL module Id." );

    // Find module definition.
    ModuleDefinitionEntry::iterator moduleItr = findModuleDefinition( StringTable->insert( pModuleId ), versionId );

     // Finish if module was not found.
    if ( moduleItr == NULL )
        return NULL;

    return *moduleItr;
}

//-----------------------------------------------------------------------------

ModuleDefinition* ModuleManager::findLoadedModule( const char* pModuleId )
{
    // Sanity!
    AssertFatal( pModuleId != NULL, "Cannot find module with NULL module Id." );

    // Fetch module Id.
    StringTableEntry moduleId = StringTable->insert( pModuleId );

    // Iterate loaded modules.
    for ( typeModuleLoadEntryVector::iterator loadedModuleItr = mModulesLoaded.begin(); loadedModuleItr != mModulesLoaded.end(); ++loadedModuleItr )
    {
        // Skip if not the module.
        if ( loadedModuleItr->mpModuleDefinition->getModuleId() != moduleId )
            continue;

        return loadedModuleItr->mpModuleDefinition;
    }

    return NULL;
}

//-----------------------------------------------------------------------------

void ModuleManager::findModules( const bool loadedOnly, typeConstModuleDefinitionVector& moduleDefinitions )
{
    // Iterate module Ids.
    for( typeModuleIdDatabaseHash::iterator moduleIdItr = mModuleIdDatabase.begin(); moduleIdItr != mModuleIdDatabase.end(); ++moduleIdItr )
    {
        // Fetch module definition entry.
        ModuleDefinitionEntry* pModuleDefinitionEntry = moduleIdItr->value;

        // Iterate module definitions.
        for ( typeModuleDefinitionVector::iterator moduleDefinitionItr = pModuleDefinitionEntry->begin(); moduleDefinitionItr != pModuleDefinitionEntry->end(); ++moduleDefinitionItr )
        {
            // Fetch module definition.
            ModuleDefinition* pModuleDefinition = *moduleDefinitionItr;

            // Are we searching for loaded modules only?
            if ( loadedOnly )
            {
                // Yes, so skip if the module is not loaded.
                if ( pModuleDefinition->getLoadCount() == 0 )
                    continue;

                // Use module definition.
                moduleDefinitions.push_back( pModuleDefinition );

                // Finish iterating module definitions as only a single module in this entry can be loaded concurrently.
                break;
            }

            // use module definition.
            moduleDefinitions.push_back( pModuleDefinition );
        }
    }
}

//-----------------------------------------------------------------------------

void ModuleManager::findModuleTypes( const char* pModuleType, const bool loadedOnly, typeConstModuleDefinitionVector& moduleDefinitions )
{
    // Fetch module type.
    StringTableEntry moduleType = StringTable->insert( pModuleType );

    // Iterate module Ids.
    for( typeModuleIdDatabaseHash::iterator moduleIdItr = mModuleIdDatabase.begin(); moduleIdItr != mModuleIdDatabase.end(); ++moduleIdItr )
    {
        // Fetch module definition entry.
        ModuleDefinitionEntry* pModuleDefinitionEntry = moduleIdItr->value;

        // Skip if note the module type we're searching for.
        if ( pModuleDefinitionEntry->mModuleType != moduleType )
            continue;

        // Iterate module definitions.
        for ( typeModuleDefinitionVector::iterator moduleDefinitionItr = pModuleDefinitionEntry->begin(); moduleDefinitionItr != pModuleDefinitionEntry->end(); ++moduleDefinitionItr )
        {
            // Fetch module definition.
            ModuleDefinition* pModuleDefinition = *moduleDefinitionItr;

            // Are we searching for loaded modules only?
            if ( loadedOnly )
            {
                // Yes, so skip if the module is not loaded.
                if ( pModuleDefinition->getLoadCount() == 0 )
                    continue;

                // Use module definition.
                moduleDefinitions.push_back( pModuleDefinition );

                // Finish iterating module definitions as only a single module in this entry can be loaded concurrently.
                break;
            }

            // use module definition.
            moduleDefinitions.push_back( pModuleDefinition );
        }
    }
}

//-----------------------------------------------------------------------------

StringTableEntry ModuleManager::copyModule( ModuleDefinition* pSourceModuleDefinition, const char* pTargetModuleId, const char* pTargetPath, const bool useVersionPathing )
{
    // Sanity!
    AssertFatal( pSourceModuleDefinition != NULL, "Cannot copy module using a NULL source module definition." );
    AssertFatal( pTargetModuleId != NULL, "Cannot copy module using a NULL target module Id." );
    AssertFatal( pTargetPath != NULL, "Cannot copy module using a NULL target path." );

    // Fetch the source module Id.
    StringTableEntry sourceModuleId = pSourceModuleDefinition->getModuleId();

    // Is the source module definition registered with this module manager?
    if ( pSourceModuleDefinition->getModuleManager() != this )
    {
        // No, so warn.
        Con::warnf("Module Manager: Cannot copy module Id '%s' as it is not registered with this module manager.", sourceModuleId );
        return StringTable->EmptyString();
    }

    // Fetch the target module Id.
    StringTableEntry targetModuleId = StringTable->insert( pTargetModuleId );

    // Extend moduleId/VersionId pathing.
    char versionPathBuffer[1024];

    // Are we using version pathing?
    if ( useVersionPathing )
    {
        // Yes, so format it.
        dSprintf( versionPathBuffer, sizeof(versionPathBuffer), "%s/%s/%d",
            pTargetPath, targetModuleId, pSourceModuleDefinition->getVersionId() );
    }
    else
    {
        // No, so a straight copy.
        dSprintf( versionPathBuffer, sizeof(versionPathBuffer), "%s", pTargetPath );
    }

    // Expand the path.
    char targetPathBuffer[1024];
    Con::expandPath( targetPathBuffer, sizeof(targetPathBuffer), versionPathBuffer );
    pTargetPath = targetPathBuffer;

    // Info.
    if ( mEchoInfo )
    {
        Con::printf( "Module Manager: Started copying module Id '%s' to target directory '%s'.", sourceModuleId, pTargetPath );
    }

    // Is the target folder a directory?
    if ( !Platform::isDirectory( pTargetPath ) )
    {
        // No, so we have to ensure that there is a trailing slash as that indicates a folder (not a file) when creating a path in the platform code.
        char createDirectoryBuffer[1024];
        Con::expandPath( createDirectoryBuffer, sizeof(createDirectoryBuffer), pTargetPath, NULL, true );

        // No, so can we create it?
        if ( !Platform::createPath( createDirectoryBuffer ) )
        {
            // No, so warn.
            Con::warnf("Module Manager: Cannot copy module Id '%s' using target directory '%s' as directory was not found and could not be created.",
                sourceModuleId, pTargetPath );
            return StringTable->EmptyString();
        }
    }

    // Copy the source module to the target folder.
    if ( !dPathCopy( pSourceModuleDefinition->getModulePath(), pTargetPath, false ) )
    {
        // Warn.
        Con::warnf("Module Manager: Cannot copy module Id '%s' using target directory '%s' as directory copy failed.",
            sourceModuleId, pTargetPath );
        return StringTable->EmptyString();
    }

    // Format the new source module definition file-path.
    char newModuleDefinitionSourceFileBuffer[1024];
    dSprintf( newModuleDefinitionSourceFileBuffer, sizeof(newModuleDefinitionSourceFileBuffer), "%s/%s", pTargetPath, pSourceModuleDefinition->getModuleFile() );

    // Finish if source/target module Ids are identical.
    if ( sourceModuleId == targetModuleId )
        return StringTable->insert( newModuleDefinitionSourceFileBuffer );

    // Format the new target module definition file-path.
    char newModuleDefinitionTargetFileBuffer[1024];
    dSprintf( newModuleDefinitionTargetFileBuffer, sizeof(newModuleDefinitionTargetFileBuffer), "%s/%s.%s", pTargetPath, targetModuleId, MODULE_MANAGER_MODULE_DEFINITION_EXTENSION );

    // Rename the module definition.
    if ( !dFileRename( newModuleDefinitionSourceFileBuffer, newModuleDefinitionTargetFileBuffer ) )
    {
        // Warn.
        Con::warnf("Module Manager: Cannot copy module Id '%s' using target directory '%s' as renaming the module from '%s' to '%s' failed.",
            sourceModuleId, pTargetPath, newModuleDefinitionSourceFileBuffer, newModuleDefinitionTargetFileBuffer );
        return StringTable->EmptyString();
    }

    Vector<StringTableEntry> directories;

    // Find directories.
    if ( !Platform::dumpDirectories( pTargetPath, directories, -1 ) )
    {
        // Warn.
        Con::warnf("Module Manager: Cannot copy module Id '%s' using target directory '%s' as sub-folder scanning/renaming failed.",
            sourceModuleId, pTargetPath );
        return StringTable->EmptyString();
    }

    TamlModuleIdUpdateVisitor moduleIdUpdateVisitor;
    moduleIdUpdateVisitor.setModuleIdFrom( sourceModuleId );
    moduleIdUpdateVisitor.setModuleIdTo( targetModuleId );

    Vector<Platform::FileInfo> files;

    const char* pExtension = (const char*)"Taml";
    const U32 extensionLength = dStrlen(pExtension);

    // Iterate directories.
    for( Vector<StringTableEntry>::iterator basePathItr = directories.begin(); basePathItr != directories.end(); ++basePathItr )
    {
        // Fetch base path.
        StringTableEntry basePath = *basePathItr;

        // Find files.
        files.clear();
        if ( !Platform::dumpPath( basePath, files, 0 ) )
        {
            // Warn.
            Con::warnf("Module Manager: Cannot copy module Id '%s' using target directory '%s' as sub-folder scanning/renaming failed.",
                sourceModuleId, pTargetPath );
            return StringTable->EmptyString();
        }

        // Iterate files.
        for ( Vector<Platform::FileInfo>::iterator fileItr = files.begin(); fileItr != files.end(); ++fileItr )
        {
            // Fetch file info.
            Platform::FileInfo* pFileInfo = fileItr;

            // Fetch filename.
            const char* pFilename = pFileInfo->pFileName;

            // Find filename length.
            const U32 filenameLength = dStrlen( pFilename );

            // Skip if extension is longer than filename.
            if ( extensionLength >= filenameLength )
                continue;

            // Skip if extension not found.
            if ( dStricmp( pFilename + filenameLength - extensionLength, pExtension ) != 0 )
                continue;

            char parseFileBuffer[1024];
            dSprintf( parseFileBuffer, sizeof(parseFileBuffer), "%s/%s", pFileInfo->pFullPath, pFilename );

            // Parse file.            
            if ( !mTaml.parse( parseFileBuffer, moduleIdUpdateVisitor ) )
            {
                // Warn.
                Con::warnf("Module Manager: Failed to parse file '%s' whilst copying module Id '%s' using target directory '%s'.",
                    parseFileBuffer, sourceModuleId, pTargetPath );
                return StringTable->EmptyString();
            }
        }
    }

    // Info.
    if ( mEchoInfo )
    {
        Con::printf( "Module Manager: Finished copying module Id '%s' to target directory '%s'.", sourceModuleId, pTargetPath );
    }

    return StringTable->insert( newModuleDefinitionTargetFileBuffer );
}

//-----------------------------------------------------------------------------

bool ModuleManager::synchronizeDependencies( ModuleDefinition* pRootModuleDefinition, const char* pTargetDependencyPath )
{
    // Sanity!
    AssertFatal( pRootModuleDefinition != NULL, "Cannot synchronize dependencies with NULL root module definition." );
    AssertFatal( pTargetDependencyPath != NULL, "Cannot synchronize dependencies with NULL target dependency path." );

    // Fetch the root module Id.
    StringTableEntry rootModuleId = pRootModuleDefinition->getModuleId();

    // Is the root module definition registered with this module manager?
    if ( pRootModuleDefinition->getModuleManager() != this )
    {
        // No, so warn.
        Con::warnf("Cannot synchronize dependencies for module Id '%s' as it is not registered with this module manager.", rootModuleId );
        return false;
    }

    // Expand the path.
    char targetPathBuffer[1024];
    Con::expandPath( targetPathBuffer, sizeof(targetPathBuffer), pTargetDependencyPath );
    pTargetDependencyPath = targetPathBuffer;

    // Is the target dependency folder a directory?
    if ( !Platform::isDirectory( pTargetDependencyPath ) )
    {
        // No, so we have to ensure that there is a trailing slash as that indicates a folder (not a file) when creating a path in the platform code.
        char createDirectoryBuffer[1024];
        Con::expandPath( createDirectoryBuffer, sizeof(createDirectoryBuffer), pTargetDependencyPath, NULL, true );

        // No, so can we create it?
        if ( !Platform::createPath( createDirectoryBuffer ) )
        {
            // No, so warn.
            Con::warnf("Cannot synchronize dependencies for module Id '%s' using target directory '%s' as directory was not found and could not be created.",
                rootModuleId, pTargetDependencyPath );
            return false;
        }
    }

    typeModuleLoadEntryVector       resolvingQueue;
    typeModuleLoadEntryVector       sourceModulesNeeded;

    // Could we resolve source dependencies?
    if ( !resolveModuleDependencies( rootModuleId, pRootModuleDefinition->getVersionId(), pRootModuleDefinition->getModuleGroup(), true, resolvingQueue, sourceModulesNeeded ) )
    {
        // No, so warn.
        Con::warnf("Cannot synchronize dependencies for root module Id '%s' as its dependencies could not be resolved.", rootModuleId );
        return false;
    }

    // Sanity!
    AssertFatal( sourceModulesNeeded.size() > 0, "Cannot synchronize dependencies as no modules were returned." );

    // Remove the root module definition.
    sourceModulesNeeded.pop_back();

    // Initialize the target module manager and scan the target folder for modules.
    ModuleManager targetModuleManager;
    targetModuleManager.mEnforceDependencies = true;
    targetModuleManager.mEchoInfo = false;
    targetModuleManager.scanModules( pTargetDependencyPath );

    char targetFolderGenerateBuffer[1024];

    // Iterate module definitions.
    for ( typeModuleLoadEntryVector::iterator sourceModuleItr = sourceModulesNeeded.begin(); sourceModuleItr != sourceModulesNeeded.end(); ++sourceModuleItr )
    {
        // Fetch module definition.
        ModuleDefinition* pSourceModuleDefinition = sourceModuleItr->mpModuleDefinition;
        
        // Fetch the source module Id,
        StringTableEntry sourceModuleId = pSourceModuleDefinition->getModuleId();

        // Fetch the source module version Id.
        const U32 sourceVersionId = pSourceModuleDefinition->getVersionId();

        // Fetch the source module build Id.
        const U32 sourceBuildId = pSourceModuleDefinition->getBuildId();

        // Fetch module definition entry for this module Id in the target.
        ModuleDefinitionEntry* pDefinitions = targetModuleManager.findModuleId( sourceModuleId );

        // Is the module Id present in the target?
        if ( pDefinitions == NULL )
        {
            // No, so format module Id folder path.
            dSprintf( targetFolderGenerateBuffer, sizeof(targetFolderGenerateBuffer), "%s/%s/", pTargetDependencyPath, sourceModuleId );

            // Create module Id folder.
            if ( !Platform::createPath( targetFolderGenerateBuffer ) )
            {
                // Warn.
                Con::warnf("Cannot synchronize dependencies for module Id '%s' as the target directory '%s' could not be created.", sourceModuleId, targetFolderGenerateBuffer );
                return false;
            }
        }
        else
        {
            // Yes, so fetch the module definition for this module Id and version Id in the target.
            ModuleDefinitionEntry::iterator definitionItr = targetModuleManager.findModuleDefinition( sourceModuleId, sourceVersionId );

            // Is the specific module definition present in the target?
            if ( definitionItr != NULL )
            {
                // Yes, so fetch the module definition.
                ModuleDefinition* pTargetModuleDefinition = *definitionItr;

                // Fetch the target module build Id.
                const U32 targetBuildId = pTargetModuleDefinition->getBuildId();

                // Fetch the target module path.
                StringTableEntry targetModulePath = pTargetModuleDefinition->getModulePath();

                // Remove the target module definition from the database.
                targetModuleManager.removeModuleDefinition( pTargetModuleDefinition );

                // Skip if the target definition is the same build Id.
                if ( targetBuildId == sourceBuildId )
                    continue;

                // Delete the target module definition folder.
                if ( !Platform::deleteDirectory( targetModulePath ) )
                {
                    // Warn.
                    Con::warnf("Cannot synchronize dependencies for module Id '%s' at version Id '%d' as the old module at '%s' could not be deleted.",
                        sourceModuleId, sourceVersionId, targetModulePath );
                    return false;
                }
            }
        }

        // Format source module path.
        char sourceFolderPath[1024];
        Con::expandPath( sourceFolderPath, sizeof(sourceFolderPath), pSourceModuleDefinition->getModulePath() );

        // Format target module path.
        dSprintf( targetFolderGenerateBuffer, sizeof(targetFolderGenerateBuffer), "%s/%s/%d", pTargetDependencyPath, sourceModuleId, sourceVersionId );

        // Copy the source module to the target folder.
        if (!dPathCopy(sourceFolderPath, targetFolderGenerateBuffer, false))
        {
            // Warn.
            Con::warnf("Cannot synchronize dependencies for module Id '%s' at version Id '%d' as the module could not be copied to '%s'.",
                sourceModuleId, sourceVersionId, targetFolderGenerateBuffer );
            return false;
        }
    }

    // Find any target modules left, These are orphaned modules not depended upon by any other module.
    typeConstModuleDefinitionVector orphanedTargetModules;
    targetModuleManager.findModules( false, orphanedTargetModules );

    // Iterate module definitions.
    for ( typeConstModuleDefinitionVector::const_iterator moduleDefinitionItr = orphanedTargetModules.begin(); moduleDefinitionItr != orphanedTargetModules.end(); ++moduleDefinitionItr )
    {
        // Fetch orphaned module definition.
        const ModuleDefinition* pOrphanedModuleDefinition = *moduleDefinitionItr;
       
        // Delete the target module definition folder.
        if ( !Platform::deleteDirectory( pOrphanedModuleDefinition->getModulePath() ) )
        {
            // Warn.
            Con::warnf("Cannot delete orphaned module Id '%s' at version Id '%d' from '%s'.",
                pOrphanedModuleDefinition->getModuleId(), pOrphanedModuleDefinition->getVersionId(), pOrphanedModuleDefinition->getModulePath() );
        }
    }

    return true;
}

//-----------------------------------------------------------------------------

bool ModuleManager::canMergeModules( const char* pMergeSourcePath )
{
    // Sanity!
    AssertFatal( pMergeSourcePath != NULL, "Cannot check merge modules with NULL source path." );

    // Expand the source path.
    char sourcePathBuffer[1024];
    Con::expandPath( sourcePathBuffer, sizeof(sourcePathBuffer), pMergeSourcePath );
    pMergeSourcePath = sourcePathBuffer;

    // Is the path a valid directory?
    if ( !Platform::isDirectory( sourcePathBuffer ) )
    {
        // No, so warn.
        Con::warnf( "Cannot check merge modules as path is invalid '%s'.", sourcePathBuffer );
        return false;
    }

    // Initialize the source module manager and scan the source folder for modules.
    ModuleManager mergeModuleManager;
    mergeModuleManager.mEnforceDependencies = false;
    mergeModuleManager.mEchoInfo = false;
    mergeModuleManager.scanModules( pMergeSourcePath );

    // Find all the merge modules.
    typeConstModuleDefinitionVector mergeModules;
    mergeModuleManager.findModules( false, mergeModules );

    // Iterate found merge module definitions.
    for ( typeConstModuleDefinitionVector::const_iterator mergeModuleItr = mergeModules.begin(); mergeModuleItr != mergeModules.end(); ++mergeModuleItr )
    {
        // Fetch module definition.
        const ModuleDefinition* pMergeModuleDefinition = *mergeModuleItr;

        // Fetch module Id.
        StringTableEntry moduleId = pMergeModuleDefinition->getModuleId();

        // Fetch version Id.
        const U32 versionId = pMergeModuleDefinition->getVersionId();

        // Fetch module group.
        StringTableEntry moduleGroup = pMergeModuleDefinition->getModuleGroup();

        // Cannot merge if module already exists.
        if ( findModuleDefinition( moduleId, versionId ) != NULL )
            return false;

        // Cannot merge if module is part of a loaded group.
        if ( findGroupLoaded( moduleGroup ) != NULL )
            return false;
    }

    // Can merge modules.
    return true;
}

//-----------------------------------------------------------------------------

bool ModuleManager::mergeModules( const char* pMergeTargetPath, const bool removeMergeDefinition, const bool registerNewModules )
{
    // Sanity!
    AssertFatal( pMergeTargetPath != NULL, "Cannot merge modules with a target path of NULL." );

    // Is a module merge available?
    if ( !isModuleMergeAvailable() )
    {
        // No, so warn.
        Con::warnf( "Cannot merge modules as a module merge is not available." );
        return false;
    }

    // Expand the target path.
    char targetPathBuffer[1024];
    Con::expandPath( targetPathBuffer, sizeof(targetPathBuffer), pMergeTargetPath );
    pMergeTargetPath = targetPathBuffer;

    // Fetch module merge file path.
    StringTableEntry moduleMergeFilePath = getModuleMergeFilePath();

    // Read module merge definition.
    Taml taml;
    ModuleMergeDefinition* pModuleMergeDefinition = taml.read<ModuleMergeDefinition>( moduleMergeFilePath );
    
    // Do we have a module merge definition.
    if ( pModuleMergeDefinition == NULL )
    {
        // No, so warn.
        Con::warnf( "Cannot merge modules as the module merge definition file failed to load '%s'.", moduleMergeFilePath );
        return false;
    }

    // Fetch the merge source path.
    StringTableEntry mergeSourcePath = pModuleMergeDefinition->getModuleMergePath();

    // Remove the module merge definition.
    pModuleMergeDefinition->deleteObject();
    pModuleMergeDefinition = NULL;

    // If we cannot merge the modules then we only process modules flagged as critical merge.
    const bool criticalMergeOnly = !canMergeModules( mergeSourcePath );

    // Initialize the target module manager and scan the target folder for modules.
    ModuleManager targetModuleManager;
    targetModuleManager.mEnforceDependencies = false;
    targetModuleManager.mEchoInfo = false;
    targetModuleManager.scanModules( pMergeTargetPath );

    // Initialize the source module manager and scan the source folder for modules.
    ModuleManager sourceModuleManager;
    sourceModuleManager.mEnforceDependencies = false;
    sourceModuleManager.mEchoInfo = false;
    sourceModuleManager.scanModules( mergeSourcePath );

    // Find all the source modules.
    typeConstModuleDefinitionVector sourceModules;
    sourceModuleManager.findModules( false, sourceModules );

    // Iterate found merge module definitions.
    for ( typeConstModuleDefinitionVector::const_iterator sourceModuleItr = sourceModules.begin(); sourceModuleItr != sourceModules.end(); ++sourceModuleItr )
    {
        // Fetch the source module definition.
        const ModuleDefinition* pSourceModuleDefinition = *sourceModuleItr;

        // Skip if we're performing a critical merge only and the module is not flagged as critical merge.
        if ( criticalMergeOnly && pSourceModuleDefinition->getCriticalMerge() )
            continue;

        // Fetch source module Id.
        const StringTableEntry sourceModuleId = pSourceModuleDefinition->getModuleId();

        // Fetch source version Id.
        const U32 sourceVersionId = pSourceModuleDefinition->getVersionId();

        // Fetch source build Id.
        const U32 sourceBuildId = pSourceModuleDefinition->getBuildId();

        // Format module Id folder path.
        char targetModuleIdBuffer[1024];
        dSprintf( targetModuleIdBuffer, sizeof(targetModuleIdBuffer), "%s/%s/", pMergeTargetPath, sourceModuleId );

        // Flag to indicate if the merged module needs registering.
        bool shouldRegisterModule;

        // Does the module Id exist?
        if ( targetModuleManager.findModuleId( sourceModuleId ) == NULL )
        {
            // No, so create module Id folder.
            if ( !Platform::createPath( targetModuleIdBuffer ) )
            {
                // Warn.
                Con::warnf("Cannot merge modules for module '%s' as the path '%s' could not be created.", sourceModuleId, targetModuleIdBuffer );
                return false;
            }

            // Module Should be registered.
            shouldRegisterModule = true;
        }
        else
        {
            // Yes, so find the target module definition that matches the source module definition.
            ModuleDefinitionEntry::iterator targetModuleDefinitionItr = targetModuleManager.findModuleDefinition( sourceModuleId, sourceVersionId );

            // Is there an existing target module definition entry?
            if ( targetModuleDefinitionItr != NULL )
            {
                // Yes, so fetch the target module definition.
                const ModuleDefinition* pTargetModuleDefinition = *targetModuleDefinitionItr;

                // Fetch target module path.
                StringTableEntry targetModulePath = pTargetModuleDefinition->getModulePath();

                // Yes, so we have to remove it first.
                if ( !Platform::deleteDirectory( targetModulePath ) )
                {
                    // Module was not deleted so warn.
                    Con::warnf( "Failed to remove module folder located at '%s'.  Module will be copied over.", targetModulePath );
                }

                // Is the build Id being downgraded?
                if ( sourceBuildId < pTargetModuleDefinition->getBuildId() )
                {
                    // Yes, so warn.
                    Con::warnf( "Encountered a downgraded build Id for module Id '%s' at version Id '%d'.", sourceModuleId, sourceBuildId );
                }

                // Module should not be registered.
                shouldRegisterModule = false;
            }
            else
            {
                // Module Should be registered.
                shouldRegisterModule = true;
            }
        }

        // Fetch source module path.
        StringTableEntry sourceModulePath = pSourceModuleDefinition->getModulePath();

        // Format target version Id folder path.
        char targetVersionIdBuffer[1024];
        dSprintf( targetVersionIdBuffer, sizeof(targetVersionIdBuffer), "%s%d", targetModuleIdBuffer, sourceVersionId );

        // Copy module (allow overwrites as we may have failed to remove the old folder in which case this is likely to fail as well).
        if (!dPathCopy(sourceModulePath, targetVersionIdBuffer, false))
        {
            // Failed to copy module.
            Con::warnf( "Failed to copy module folder located at '%s' to location '%s'.  The modules may now be corrupted.", sourceModulePath, targetVersionIdBuffer );
        }

        // Are we registering new modules and the module needs registering?
        if ( registerNewModules && shouldRegisterModule )
        {
            // Yes, so scan module.
            scanModules( targetVersionIdBuffer, true );
        }

        // Is the module part of a critical merge?
        if ( criticalMergeOnly )
        {
            // Yes, so we need to remove the source module definition.
            if ( !Platform::deleteDirectory( sourceModulePath ) )
            {
                // Module was not deleted so warn.
                Con::warnf( "Failed to remove CRITICAL merge module folder located at '%s'.  Module will be copied over.", sourceModulePath );
            }
        }
    }

    // Do we need to remove the module merge definition file?
    if ( removeMergeDefinition )
    {
        // Yes, so remove it.
        dFileDelete( moduleMergeFilePath );
    }

    return true;
}

//-----------------------------------------------------------------------------

void ModuleManager::addListener( SimObject* pListener )
{
    // Sanity!
    AssertFatal( pListener != NULL, "Cannot add notifications to a NULL object." );

    // Ignore if already added.
    if (mNotificationListeners.find( pListener) != mNotificationListeners.end())
        return;
        
    // Add as a listener.
    mNotificationListeners.addObject( pListener );
}

//-----------------------------------------------------------------------------

void ModuleManager::removeListener( SimObject* pListener )
{
    // Sanity!
    AssertFatal( pListener != NULL, "Cannot remove notifications from a NULL object." );

    // Remove as a listener.
    mNotificationListeners.removeObject( pListener );
}

//-----------------------------------------------------------------------------

void ModuleManager::clearDatabase( void )
{
    // Lock database.
    AssertFatal( mDatabaseLocks == 0, "Cannot clear database if database is locked." );

    // Iterate groups loaded.
    while ( mGroupsLoaded.size() > 0 )
    {
        // Unload module group.
        unloadModuleGroup( *mGroupsLoaded.begin() );
    }

    // Iterate any other explicit modules that are loaded.
    while ( mModulesLoaded.size() > 0 )
    {
        // Fetch module definition.
        ModuleDefinition* pModuleDefinition = mModulesLoaded.begin()->mpModuleDefinition;

        // Unload explicit module.
        unloadModuleExplicit( pModuleDefinition->getModuleId() );
    }

    // Iterate modules to delete module definitions.
    for ( typeModuleIdDatabaseHash::iterator moduleItr = mModuleIdDatabase.begin(); moduleItr != mModuleIdDatabase.end(); ++moduleItr )
    {
        // Fetch modules definitions.
        ModuleDefinitionEntry* pDefinitions = moduleItr->value;

        // Iterate module definitions.
        for ( ModuleDefinitionEntry::iterator definitionItr = pDefinitions->begin(); definitionItr != pDefinitions->end(); ++definitionItr )
        {
            // Fetch module definition.
            ModuleDefinition* pModuleDefinition = *definitionItr;

            // Remove notification before we delete it.
            clearNotify( pModuleDefinition );

            // Delete module definition.
            pModuleDefinition->deleteObject();
        }

        // Clear definitions.
        delete pDefinitions;        
    }

    // Clear database.
    mModuleIdDatabase.clear();

    // Iterate module groups.
    for ( typeGroupModuleHash::iterator moduleGroupItr = mGroupModules.begin(); moduleGroupItr != mGroupModules.end(); ++moduleGroupItr )
    {
        // Delete module group vector.
        delete moduleGroupItr->value;
    }

    // Clear module groups.
    mGroupModules.clear();
}

//-----------------------------------------------------------------------------

bool ModuleManager::removeModuleDefinition( ModuleDefinition* pModuleDefinition )
{
    // Sanity!
    AssertFatal( pModuleDefinition != NULL, "Cannot remove module definition if it is NULL." );
    
    // Fetch module Id.
    StringTableEntry moduleId = pModuleDefinition->getModuleId();

    // Is the module definition registered with this module manager?
    if ( pModuleDefinition->getModuleManager() != this )
    {
        // No, so warn.
        Con::warnf("Cannot remove module definition '%s' as it is not registered with this module manager.", moduleId );
        return false;
    }

    // Is the module definition loaded?
    if ( pModuleDefinition->getLoadCount() > 0 )
    {
        // No, so warn.
        Con::warnf("Cannot remove module definition '%s' as it is loaded.", moduleId );
        return false;
    }

    // Find module Id.
    typeModuleIdDatabaseHash::iterator moduleItr = mModuleIdDatabase.find( moduleId );

    // Sanity!
    AssertFatal( moduleItr != mModuleIdDatabase.end(), "Failed to find module definition." );

    // Fetch modules definitions.
    ModuleDefinitionEntry* pDefinitions = moduleItr->value;

    // Fetch version Id.
    const U32 versionId = pModuleDefinition->getVersionId();

    // Iterate module definitions.
    for ( ModuleDefinitionEntry::iterator definitionItr = pDefinitions->begin(); definitionItr != pDefinitions->end(); ++definitionItr )
    {
        // Skip if this isn't the version Id we're searching for.
        if ( versionId != (*definitionItr)->getVersionId() )
            continue;

        // Remove definition entry.
        pDefinitions->erase( definitionItr ); 

        // Remove notification before we delete it.
        clearNotify( pModuleDefinition );

        // Delete module definition.
        pModuleDefinition->deleteObject();

        // Are there any modules left for this module Id?
        if ( findModuleId( moduleId ) == NULL )
        {
            bool moduleIdFound = false;

            // No, so remove from groups.
            for( typeGroupModuleHash::iterator moduleGroupItr = mGroupModules.begin(); moduleGroupItr != mGroupModules.end(); ++moduleGroupItr )
            {
                // Fetch module Ids.
                typeModuleIdVector* pModuleIds = moduleGroupItr->value;

                // Iterate module Id.
                for( typeModuleIdVector::iterator moduleIdItr = pModuleIds->begin(); moduleIdItr != pModuleIds->end(); ++moduleIdItr )
                {
                    // Skip if this isn't the Id.
                    if ( *moduleIdItr != moduleId )
                        continue;

                    // Remove the module Id.
                    pModuleIds->erase( moduleIdItr );

                    // Flag as found.
                    moduleIdFound = true;

                    break;
                }

                // Finish if found.
                if ( moduleIdFound )
                    break;
            }
        }

        return true;
    }

    // Sanity!
    AssertFatal( false, "Failed to find module definition." );

    return false;
}

//-----------------------------------------------------------------------------

bool ModuleManager::registerModule( const char* pModulePath, const char* pModuleFile )
{
    // Sanity!
    AssertFatal( pModulePath != NULL, "Cannot scan module with NULL module path." );
    AssertFatal( pModuleFile != NULL, "Cannot scan module with NULL module file." );

    // Make the module path a full-path.
    char fullPathBuffer[1024];
    Platform::makeFullPathName( pModulePath, fullPathBuffer, sizeof(fullPathBuffer) );
    pModulePath = fullPathBuffer;


    char formatBuffer[1024];

    // Fetch module path trail character.
    char modulePathTrail = pModulePath[dStrlen(pModulePath) - 1];

    // Format module file-path.
    dSprintf( formatBuffer, sizeof(formatBuffer), modulePathTrail == '/' ? "%s%s" : "%s/%s", pModulePath, pModuleFile );

    // Read the module file.
    ModuleDefinition* pModuleDefinition = mTaml.read<ModuleDefinition>( formatBuffer );

    // Did we read a module definition?
    if ( pModuleDefinition == NULL )
    {
        // No, so warn.
        Con::warnf( "Module Manager: Failed to read module definition in file '%s'.", formatBuffer );
        return false;
    }

    // Set the module manager.
    pModuleDefinition->setModuleManager( this );

    // Set module definition path.
    pModuleDefinition->setModulePath( pModulePath );

    // Set module file.
    pModuleDefinition->setModuleFile( pModuleFile );

    // Set module file-path.
    pModuleDefinition->setModuleFilePath( formatBuffer );

    // Fetch module Id.
    StringTableEntry moduleId = pModuleDefinition->getModuleId();

    // Fetch module version Id.
    const U32 versionId = pModuleDefinition->getVersionId();

    // Fetch module group.
    StringTableEntry moduleGroup = pModuleDefinition->getModuleGroup();

    // Fetch module type.
    StringTableEntry moduleType = pModuleDefinition->getModuleType();

    // Is the module enabled?
    if ( !pModuleDefinition->getEnabled() )
    {
        // No, so warn.
        Con::warnf( "Module Manager: Found module: '%s' but it is disabled.", pModuleDefinition->getModuleFilePath() );

        // Destroy module definition and finish.
        pModuleDefinition->deleteObject();
        return false;
    }

    // Is the module Id valid?
    if (moduleId == StringTable->EmptyString())
    {
        // No, so warn.
        Con::warnf( "Module Manager: Found module: '%s' but it has an unspecified module Id.",
            pModuleDefinition->getModuleFilePath() );

        // Destroy module definition and finish.
        pModuleDefinition->deleteObject();
        return false;
    }

    // Is the module version Id valid?
    if ( versionId == 0 )
    {
        // No, so warn.
        Con::warnf( "Module Manager: Found Manager: Registering module: '%s' but it has an invalid Version Id of '0'.",
            pModuleDefinition->getModuleFilePath() );

        // Destroy module definition and finish.
        pModuleDefinition->deleteObject();
        return false;
    }

    // Is the module group already loaded?
    if ( findGroupLoaded( moduleGroup ) != NULL )
    {
        // Yes, so warn.
        Con::warnf( "Module Manager: Found module: '%s' but it is in a module group '%s' which has already been loaded.",
            pModuleDefinition->getModuleFilePath(),
            moduleGroup );

        // Destroy module definition and finish.
        pModuleDefinition->deleteObject();
        return false;
    }

    // Was a script-file specified?
    if ( pModuleDefinition->getScriptFile() != StringTable->EmptyString() )
    {
        // Yes, so format module script file-path.
        dSprintf( formatBuffer, sizeof(formatBuffer), modulePathTrail == '/' ? "%s%s" : "%s/%s", pModulePath, pModuleDefinition->getScriptFile() );
        pModuleDefinition->setModuleScriptFilePath( formatBuffer );
    }

    // Format module signature,
    dSprintf( formatBuffer, sizeof(formatBuffer), "%s_%d_%d", moduleId, versionId, pModuleDefinition->getBuildId() );
    pModuleDefinition->setSignature( formatBuffer );

    // Locked the module definition.
    pModuleDefinition->setLocked( true );

    // Fetch modules definitions.
    ModuleDefinitionEntry* pDefinitions = findModuleId( moduleId );

    // Did we find the module Id?
    if ( pDefinitions != NULL )
    {
        // Yes, so find the module definition.
        ModuleDefinitionEntry::iterator definitionItr = findModuleDefinition( moduleId, versionId );

        // Does this version Id already exist?
        if ( definitionItr != NULL )
        {
            // Yes, so warn.
            Con::warnf( "Module Manager: Found module: '%s' but it is already registered as module Id '%s' at version Id '%d'.",
                pModuleDefinition->getModuleFilePath(), moduleId, versionId );

            // Destroy module definition and finish.
            pModuleDefinition->deleteObject();
            return false;
        }

        // Is the module group the same as the module definitions we already have?
        if ( moduleGroup != pDefinitions->mModuleGroup )
        {
            // No, so warn.
            Con::warnf( "Module Manager: Found module: '%s' but its module group '%s' is not the same as other module definitions of the same module Id.",
                pModuleDefinition->getModuleFilePath(), moduleGroup );

            // Destroy module definition and finish.
            pModuleDefinition->deleteObject();
            return false;
        }

        // Is the module type the same as the module definitions we already have?
        if ( moduleType != pDefinitions->mModuleType )
        {
            // No, so warn.
            Con::warnf( "Module Manager: Found module: '%s' but its module type '%s' is not the same as other module definitions of the same module Id.",
                pModuleDefinition->getModuleFilePath(), moduleGroup );

            // Destroy module definition and finish.
            pModuleDefinition->deleteObject();
            return false;
        }
    }
    else
    {
        // No, so create a vector of definitions.
        pDefinitions = new ModuleDefinitionEntry( moduleId, moduleGroup, moduleType );

        // Insert module Id definitions.
        mModuleIdDatabase.insert( moduleId, pDefinitions );
    }
    
    // Add module definition.
    pDefinitions->push_back( pModuleDefinition );

    // Sort module definitions by version Id so that higher versions appear first.
    dQsort( pDefinitions->address(), pDefinitions->size(), sizeof(ModuleDefinition*), moduleDefinitionVersionIdSort );

    // Find module group.
    typeGroupModuleHash::iterator moduleGroupItr = mGroupModules.find( moduleGroup );

    // Did we find the module group?
    if ( moduleGroupItr != mGroupModules.end() )
    {
        // Yes, so fetch module Ids.
        typeModuleIdVector* pModuleIds = moduleGroupItr->value;

        // Is the module Id already present?
        bool moduleIdFound = false;
        for( typeModuleIdVector::iterator moduleIdItr = pModuleIds->begin(); moduleIdItr != pModuleIds->end(); ++moduleIdItr )
        {
            // Skip if this isn't the Id.
            if ( *moduleIdItr != moduleId )
                continue;

            // Flag as found.
            moduleIdFound = true;
            break;
        }

        // Add if module Id was not found.
        if ( !moduleIdFound )
            pModuleIds->push_back( moduleId );
    }
    else
    {
        // No, so insert a module Id vector.
        moduleGroupItr = mGroupModules.insert( pModuleDefinition->getModuleGroup(), new typeModuleIdVector() );

        // Add module Id.
        moduleGroupItr->value->push_back( moduleId );
    }

    // Notify if the module definition is destroyed.
    deleteNotify( pModuleDefinition );

    // Info.
    if ( mEchoInfo )
    {
#if 1
        Con::printf( "Module Manager: Registering: '%s' [ ID='%s', VersionId='%d', BuildId='%d', Description='%s' ].",
            pModuleDefinition->getModuleFilePath(),
            pModuleDefinition->getModuleId(),
            pModuleDefinition->getVersionId(),
            pModuleDefinition->getBuildId(),
            pModuleDefinition->getModuleDescription()
            );
#else
        Con::printf( "Module Manager: Registering: '%s' [ ID='%s', VersionId='%d', BuildId='%d', Description='%s', Group='%s', Dependencies='%s', ScriptFile='%s', CreateFunction='%s', DestroyFunction='%s' ].",
            pModuleDefinition->getModuleFilePath(),
            pModuleDefinition->getModuleId(),
            pModuleDefinition->getVersionId(),
            pModuleDefinition->getBuildId(),
            pModuleDefinition->getModuleDescription(),
            pModuleDefinition->getModuleGroup(),
            pModuleDefinition->getDataField( StringTable->insert("Dependencies"), NULL ),
            pModuleDefinition->getScriptFile(),
            pModuleDefinition->getCreateFunction(),
            pModuleDefinition->getDestroyFunction()
            );
#endif
    }

    // Emit notifications.
    for( SimSet::iterator notifyItr = mNotificationListeners.begin(); notifyItr != mNotificationListeners.end(); ++notifyItr )
    {
        Con::executef( *notifyItr, "onModuleRegister", pModuleDefinition->getIdString() );
    }

    return true;
}

//-----------------------------------------------------------------------------

bool ModuleManager::unregisterModule( const char* pModuleId, const U32 versionId )
{
    // Sanity!
    AssertFatal( pModuleId != NULL, "A module Id cannot be NULL." );

    // Fetch module Id.
    StringTableEntry moduleId = StringTable->insert( pModuleId );

    // Find the module definition.
    ModuleDefinition* pModuleDefinition = findModule( pModuleId, versionId );

    // Did we find the module definition?
    if ( pModuleDefinition == NULL )
    {
        // No, so warn.
        Con::warnf( "Module Manager: Cannot unregister module Id '%s' as it is not registered.", moduleId );
        return false;
    }

    // Remove the module definition.
    return removeModuleDefinition( pModuleDefinition );
}

//-----------------------------------------------------------------------------

void ModuleManager::raiseModulePreLoadNotifications( ModuleDefinition* pModuleDefinition )
{
    // Raise notifications.
    for( SimSet::iterator notifyItr = mNotificationListeners.begin(); notifyItr != mNotificationListeners.end(); ++notifyItr )
    {
        // Fetch listener object.
        SimObject* pListener = *notifyItr;

        // Perform object callback.
        ModuleCallbacks* pCallbacks = dynamic_cast<ModuleCallbacks*>( pListener );
        if ( pCallbacks != NULL )
            pCallbacks->onModulePreLoad( pModuleDefinition );             
            
        // Perform script callback.
        if ( pListener->isMethod( "onModulePreLoad" ) )
            Con::executef( pListener, "onModulePreLoad", pModuleDefinition->getIdString() );
    }
}

//-----------------------------------------------------------------------------

void ModuleManager::raiseModulePostLoadNotifications( ModuleDefinition* pModuleDefinition )
{
    // Raise notifications.
    for( SimSet::iterator notifyItr = mNotificationListeners.begin(); notifyItr != mNotificationListeners.end(); ++notifyItr )
    {
        // Fetch listener object.
        SimObject* pListener = *notifyItr;

        // Perform object callback.
        ModuleCallbacks* pCallbacks = dynamic_cast<ModuleCallbacks*>( pListener );
        if ( pCallbacks != NULL )
            pCallbacks->onModulePostLoad( pModuleDefinition );             
            
        // Perform script callback.
        if ( pListener->isMethod( "onModulePostLoad" ) )
            Con::executef( pListener, "onModulePostLoad", pModuleDefinition->getIdString() );
    }
}

//-----------------------------------------------------------------------------

void ModuleManager::raiseModulePreUnloadNotifications( ModuleDefinition* pModuleDefinition )
{
    // Raise notifications.
    for( SimSet::iterator notifyItr = mNotificationListeners.begin(); notifyItr != mNotificationListeners.end(); ++notifyItr )
    {
        // Fetch listener object.
        SimObject* pListener = *notifyItr;

        // Perform object callback.
        ModuleCallbacks* pCallbacks = dynamic_cast<ModuleCallbacks*>( pListener );
        if ( pCallbacks != NULL )
            pCallbacks->onModulePreUnload( pModuleDefinition );             
            
        // Perform script callback.
        if ( pListener->isMethod( "onModulePreUnload" ) )
            Con::executef( pListener, "onModulePreUnload", pModuleDefinition->getIdString() );
    }
}

//-----------------------------------------------------------------------------

void ModuleManager::raiseModulePostUnloadNotifications( ModuleDefinition* pModuleDefinition )
{
    // Raise notifications.
    for( SimSet::iterator notifyItr = mNotificationListeners.begin(); notifyItr != mNotificationListeners.end(); ++notifyItr )
    {
        // Fetch listener object.
        SimObject* pListener = *notifyItr;

        // Perform object callback.
        ModuleCallbacks* pCallbacks = dynamic_cast<ModuleCallbacks*>( pListener );
        if ( pCallbacks != NULL )
            pCallbacks->onModulePostUnload( pModuleDefinition );             
            
        // Perform script callback.
        if ( pListener->isMethod( "onModulePostUnload" ) )
            Con::executef( pListener, "onModulePostUnload", pModuleDefinition->getIdString() );
    }
}

//-----------------------------------------------------------------------------

ModuleManager::ModuleDefinitionEntry* ModuleManager::findModuleId( StringTableEntry moduleId )
{
    // Sanity!
    AssertFatal( moduleId != NULL, "A module Id cannot be NULL." );

    // Is the module Id valid?
    if ( moduleId == StringTable->EmptyString() )
    {
        // No, so warn.
        Con::warnf( "Module Manager: Invalid Module Id." );
        return NULL;
    }

    // Find module Id.
    typeModuleIdDatabaseHash::iterator moduleItr = mModuleIdDatabase.find( moduleId );

    // Return appropriately.
    return moduleItr != mModuleIdDatabase.end() ? moduleItr->value : NULL;
}

//-----------------------------------------------------------------------------

ModuleManager::ModuleDefinitionEntry::iterator ModuleManager::findModuleDefinition( StringTableEntry moduleId, const U32 versionId )
{
    // Fetch modules definitions.
    ModuleDefinitionEntry* pDefinitions = findModuleId( moduleId );

    // Finish if no module definitions for the module Id.
    if ( pDefinitions == NULL )
        return NULL;

    // Iterate module definitions.
    for ( ModuleDefinitionEntry::iterator definitionItr = pDefinitions->begin(); definitionItr != pDefinitions->end(); ++definitionItr )
    {
        // Skip if this isn't the version Id we're searching for.
        if ( versionId != (*definitionItr)->getVersionId() )
            continue;

        // Return module definition iterator.
        return definitionItr;
    }

    // Not found.
    return NULL;
}

//-----------------------------------------------------------------------------

bool ModuleManager::resolveModuleDependencies( StringTableEntry moduleId, const U32 versionId, StringTableEntry moduleGroup, bool synchronizedOnly, typeModuleLoadEntryVector& moduleResolvingQueue, typeModuleLoadEntryVector& moduleReadyQueue )
{
    // Fetch the module Id ready entry.
    ModuleLoadEntry* pLoadReadyEntry = findModuleReady( moduleId, moduleReadyQueue );

    // Is there a load entry?
    if ( pLoadReadyEntry )
    {
        // Yes, so finish if the version Id is not important,
        if ( versionId == 0 )
            return true;

        // Finish if the version Id are compatible.
        if ( versionId == pLoadReadyEntry->mpModuleDefinition->getVersionId() )
            return true;

        // Is it a strict version Id?
        if ( pLoadReadyEntry->mStrictVersionId )
        {
            // Yes, so warn.
            Con::warnf( "Module Manager: A module dependency was detected loading module Id '%s' at version Id '%d' in group '%s' but an version Id '%d' is also required.",
                moduleId, versionId, pLoadReadyEntry->mpModuleDefinition->getVersionId(), moduleGroup );
            return false;
        }

        // No, so find the required module version Id.
        ModuleDefinitionEntry::iterator definitionItr = findModuleDefinition( moduleId, versionId );

        // Did we find the requested module definition.
        if ( definitionItr == NULL )
        {
            // No, so we can safely ignore the missing dependency if we're not enforcing dependencies.
            if ( !mEnforceDependencies )
                return true;

            // Warn!
            Con::warnf( "Module Manager: A missing module dependency was detected loading module Id '%s' at version Id '%d' in group '%s'.",
                moduleId, versionId, moduleGroup );
            return false;
        }

        // Set the new module definition.
        pLoadReadyEntry->mpModuleDefinition = *definitionItr;

        // Set strict version Id.
        pLoadReadyEntry->mStrictVersionId = true;
                
        return true;
    }

    // Is the module Id load resolving?
    if ( findModuleResolving( moduleId, moduleResolvingQueue ) != NULL )
    {
        // Yes, so a cycle has been detected so warn.
        Con::warnf( "Module Manager: A cyclic dependency was detected resolving module Id '%s' at version Id '%d' in group '%s'.",
            moduleId, versionId, moduleGroup );
        return false;
    }

    // Reset selected module definition.
    ModuleDefinition* pSelectedModuleDefinition = NULL;

    // Do we want the latest version Id?
    if ( versionId == 0 )
    {
        // Yes, so find the module Id.
        typeModuleIdDatabaseHash::iterator moduleIdItr = mModuleIdDatabase.find( moduleId );

        // Did we find the module Id?
        if ( moduleIdItr == mModuleIdDatabase.end() )
        {
            // No, so we can safely ignore the missing dependency if we're not enforcing dependencies.
            if ( !mEnforceDependencies )
                return true;

            // Warn!
            Con::warnf( "Module Manager: A missing module dependency was detected loading module Id '%s' at version Id '%d' in group '%s'.",
                moduleId, versionId, moduleGroup );
            return false;
        }

        // Fetch first module definition which should be the highest version Id.
        pSelectedModuleDefinition = (*moduleIdItr->value)[0];
    }
    else
    {
        // No, so find the module Id at the specific version Id.
        ModuleDefinitionEntry::iterator definitionItr = findModuleDefinition( moduleId, versionId );

        // Did we find the module definition?
        if ( definitionItr == NULL )
        {
            // No, so we can safely ignore the missing dependency if we're not enforcing dependencies.
            if ( !mEnforceDependencies )
                return true;

            // Warn!
            Con::warnf( "Module Manager: A missing module dependency was detected loading module Id '%s' at version Id '%d' in group '%s'.",
                moduleId, versionId, moduleGroup );
            return false;
        }

        // Select the module definition.
        pSelectedModuleDefinition = *definitionItr;
    }

    // If we're only resolving synchronized modules and the module is not synchronized then finish.
    if ( synchronizedOnly && !pSelectedModuleDefinition->getSynchronized() )
        return true;

    // Create a load entry.
    ModuleLoadEntry loadEntry( pSelectedModuleDefinition, false );

    // Fetch module dependencies.
    const ModuleDefinition::typeModuleDependencyVector& moduleDependencies = pSelectedModuleDefinition->getDependencies();

    // Do we have any module dependencies?
    if ( moduleDependencies.size() > 0 )
    {
        // Yes, so queue this module as resolving.
        moduleResolvingQueue.push_back( loadEntry );

        // Iterate module dependencies.
        for( ModuleDefinition::typeModuleDependencyVector::const_iterator dependencyItr = moduleDependencies.begin(); dependencyItr != moduleDependencies.end(); ++dependencyItr )
        {            
            // Finish if we could not the dependent module Id at the version Id.
            if ( !resolveModuleDependencies( dependencyItr->mModuleId, dependencyItr->mVersionId, moduleGroup, synchronizedOnly, moduleResolvingQueue, moduleReadyQueue ) )
                return false;
        }

        // Remove module as resolving.
        moduleResolvingQueue.pop_back();
    }

    // Queue module as ready.
    moduleReadyQueue.push_back( loadEntry );        

    return true;
}

//-----------------------------------------------------------------------------

ModuleManager::ModuleLoadEntry* ModuleManager::findModuleResolving( StringTableEntry moduleId, typeModuleLoadEntryVector& moduleResolvingQueue )
{
    // Iterate module load resolving queue.
    for( typeModuleLoadEntryVector::iterator loadEntryItr = moduleResolvingQueue.begin(); loadEntryItr != moduleResolvingQueue.end(); ++loadEntryItr )
    {
        // Finish if found.
        if ( moduleId == loadEntryItr->mpModuleDefinition->getModuleId() )
            return loadEntryItr;
    }

    // Not found.
    return NULL;
}

//-----------------------------------------------------------------------------

ModuleManager::ModuleLoadEntry* ModuleManager::findModuleReady( StringTableEntry moduleId, typeModuleLoadEntryVector& moduleReadyQueue )
{
    // Iterate module load ready queue.
    for( typeModuleLoadEntryVector::iterator loadEntryItr = moduleReadyQueue.begin(); loadEntryItr != moduleReadyQueue.end(); ++loadEntryItr )
    {
        // Finish if found.
        if ( moduleId == loadEntryItr->mpModuleDefinition->getModuleId() )
            return loadEntryItr;
    }

    // Not found.
    return NULL;
}

//-----------------------------------------------------------------------------

ModuleManager::typeModuleLoadEntryVector::iterator ModuleManager::findModuleLoaded( StringTableEntry moduleId, const U32 versionId )
{
    // Iterate module loaded queue.
    for( typeModuleLoadEntryVector::iterator loadEntryItr = mModulesLoaded.begin(); loadEntryItr != mModulesLoaded.end(); ++loadEntryItr )
    {
        // Skip if not the module Id we're looking for.
        if ( moduleId != loadEntryItr->mpModuleDefinition->getModuleId() )
            continue;

        // Skip if we are searching for a specific version and it does not match.
        if ( versionId != 0 && versionId != loadEntryItr->mpModuleDefinition->getVersionId() )
            continue;

        return loadEntryItr;
    }

    // Not found.
    return NULL;
}

//-----------------------------------------------------------------------------

ModuleManager::typeGroupVector::iterator ModuleManager::findGroupLoaded( StringTableEntry moduleGroup )
{
    // Iterate groups loaded queue.
    for( typeGroupVector::iterator groupsLoadedItr = mGroupsLoaded.begin(); groupsLoadedItr != mGroupsLoaded.end(); ++groupsLoadedItr )
    {
        // Finish if found.
        if ( moduleGroup == *groupsLoadedItr )
            return groupsLoadedItr;
    }

    // Not found.
    return NULL;
}

//-----------------------------------------------------------------------------

StringTableEntry ModuleManager::getModuleMergeFilePath( void ) const
{
    // Format merge file path.
    char filePathBuffer[1024];
    dSprintf( filePathBuffer, sizeof(filePathBuffer), "%s/%s", Platform::getExecutablePath(), MODULE_MANAGER_MERGE_FILE );

    return StringTable->insert( filePathBuffer );
}
