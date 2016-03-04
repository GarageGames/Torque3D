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
#include "moduleDefinition.h"
#include "moduleManager.h"

DefineEngineMethod(ModuleManager, setModuleExtension, bool, (const char* moduleExtension), (""),
   "Set the module extension used to scan for modules.  The default is 'module'.\n"
   "@param moduleExtension The module extension used to scan for modules.Do not use a period character.\n"
   "@return Whether setting the module extension was successful or not.\n")
{
    // Set module extension.
   return object->setModuleExtension(moduleExtension);
}

//-----------------------------------------------------------------------------

DefineEngineMethod(ModuleManager, scanModules, bool, (const char* pRootPath, bool rootOnly), ("", false),
   "Scans for modules which are sub-directories of the specified path.\n"
   "@param moduleRootPath The root directory to scan for sub - directories containing modules.\n"
   "@param rootOnly[Optional] - Specifies whether to only scan the root path or not when searching for modules.\n"
   "@return Whether the scan was successful or not.A successful scan can still find zero modules.\n")
{
    // Scan modules.
    return object->scanModules( pRootPath, rootOnly );
}

//-----------------------------------------------------------------------------

DefineEngineMethod(ModuleManager, unregisterModule, bool, (const char* pModuleId, bool versionId), ("", false),
   "Unregister the specified module.\n"
   "@param moduleId The module Id to unregister.\n"
   "@param versionId The version Id to unregister.\n"
   "@return Whether the module was unregister or not.\n")
{
    // Unregister the module.
    return object->unregisterModule( pModuleId, versionId );
}

//-----------------------------------------------------------------------------

DefineEngineMethod(ModuleManager, loadGroup, bool, (const char* pModuleGroup), (""),
   "Load the specified module group.\n"
   "@param moduleGroup The module group to load.\n"
   "@return Whether the module group was loaded or not.\n")
{
    // Load module group.
   return object->loadModuleGroup(pModuleGroup);
}

//-----------------------------------------------------------------------------

DefineEngineMethod(ModuleManager, unloadGroup, bool, (const char* pModuleGroup), (""),
   "Unload the specified module group.\n"
   "@param moduleGroup The module group to unload.\n"
   "@return Whether the module group was unloaded or not.\n")
{
    // Unload module group.
   return object->unloadModuleGroup(pModuleGroup);
}

//-----------------------------------------------------------------------------

DefineEngineMethod(ModuleManager, loadExplicit, bool, (const char* pModuleId, S32 pVersionId), ("", -1),
   "Load the specified module explicitly.\n"
   "@param moduleId The module Id to load.\n"
   "@param versionId The version Id to load.Optional:  Will load the latest version.\n"
   "@return Whether the module Id was loaded or not.\n")
{
    if (pVersionId == -1)
       return object->loadModuleExplicit(pModuleId);
    else
       return object->loadModuleExplicit(pModuleId, pVersionId);
}

//-----------------------------------------------------------------------------

DefineEngineMethod(ModuleManager, unloadExplicit, bool, (const char* pModuleId), (""),
   "Unload the specified module explicitly.\n"
   "@param moduleId The module Id to unload.\n"
   "@return Whether the module Id was unloaded or not.\n")
{
    // Unload module Id explicitly.
   return object->unloadModuleExplicit(pModuleId);
}

//-----------------------------------------------------------------------------

DefineEngineMethod(ModuleManager, findModule, String, (const char* pModuleId, U32 pVersionId), ("", 0),
   "Find the specific module Id optionally at the specified version Id.\n"
   "@param moduleId The module Id to find.\n"
   "@param versionId The version Id to find.\n"
   "@return The module definition object or NULL if not found.\n")
{
    // Find module definition.
   ModuleDefinition* pModuleDefinition = object->findModule(pModuleId, pVersionId);

    // Return nothing if not found.
    if ( pModuleDefinition == NULL )
        return StringTable->EmptyString();

    return pModuleDefinition->getIdString();
}

//-----------------------------------------------------------------------------

DefineEngineMethod(ModuleManager, findModules, String, (bool loadedOnly), (false),
   "Find all the modules registered with the specified loaded state.\n"
   "@param loadedOnly Whether to return only modules that are loaded or not.\n"
   "@return A list of space - separated module definition object Ids.\n")
{
    // Find module type definitions.
    Vector<const ModuleDefinition*> moduleDefinitions;

    // Find modules.
    object->findModules( loadedOnly, moduleDefinitions );

    // Fetch module definition count.
    const U32 moduleDefinitionCount = (U32)moduleDefinitions.size();

    // Finish if no module definition were found.
    if ( moduleDefinitionCount == 0 )
       return StringTable->EmptyString();

    // Create a return buffer.
    S32 bufferSize = 4096;
    char* pReturnBuffer = Con::getReturnBuffer( bufferSize );
    char* pBufferWrite = pReturnBuffer;

    // Iterate module definitions.
    for ( ModuleManager::typeConstModuleDefinitionVector::const_iterator moduleDefinitionItr = moduleDefinitions.begin(); moduleDefinitionItr != moduleDefinitions.end(); ++moduleDefinitionItr )
    {
        // Fetch module definition.
        const ModuleDefinition* pModuleDefinition = *moduleDefinitionItr;

        // Format module definition.
        const U32 offset = dSprintf( pBufferWrite, bufferSize, "%d ", pModuleDefinition->getId() );
        pBufferWrite += offset;
        bufferSize -= offset;

        // Are we out of buffer space?
        if ( bufferSize <= 0 )
        {
            // Yes, so warn.
            Con::warnf( "ModuleManager::findModules() - Ran out of buffer space." );
            break;
        }
    }

    return pReturnBuffer;
}

//-----------------------------------------------------------------------------

DefineEngineMethod(ModuleManager, findModuleTypes, String, (const char* pModuleType, bool loadedOnly), ("", false),
   "Find the modules registered with the specified module type.\n"
   "@param moduleType The module type to search for.\n"
   "@param loadedOnly Whether to return only modules that are loaded or not.\n"
   "@return A list of space - separated module definition object Ids.\n")
{
    // Find module type definitions.
    Vector<const ModuleDefinition*> moduleDefinitions;

    // Find module types.
    object->findModuleTypes( pModuleType, loadedOnly, moduleDefinitions );

    // Fetch module definition count.
    const U32 moduleDefinitionCount = (U32)moduleDefinitions.size();

    // Finish if no module definition were found.
    if ( moduleDefinitionCount == 0 )
       return StringTable->EmptyString();

    // Create a return buffer.
    S32 bufferSize = 4096;
    char* pReturnBuffer = Con::getReturnBuffer( bufferSize );
    char* pBufferWrite = pReturnBuffer;

    // Iterate module definitions.
    for ( ModuleManager::typeConstModuleDefinitionVector::const_iterator moduleDefinitionItr = moduleDefinitions.begin(); moduleDefinitionItr != moduleDefinitions.end(); ++moduleDefinitionItr )
    {
        // Fetch module definition.
        const ModuleDefinition* pModuleDefinition = *moduleDefinitionItr;

        // Format module definition.
        const U32 offset = dSprintf( pBufferWrite, bufferSize, "%d ", pModuleDefinition->getId() );
        pBufferWrite += offset;
        bufferSize -= offset;

        // Are we out of buffer space?
        if ( bufferSize <= 0 )
        {
            // Yes, so warn.
            Con::warnf( "ModuleManager::findTypes() - Ran out of buffer space." );
            break;
        }
    }

    return pReturnBuffer;
}

//-----------------------------------------------------------------------------

DefineEngineMethod(ModuleManager, copyModule, String, (const char* sourceModuleDefinition, const char* pTargetModuleId, const char* pTargetPath, const bool useVersionPathing), 
   ("", "", "", false),
   "Copy the module to a new location with a new module Id.\n"
   "@param sourceModuleDefinition The module definition to copy.\n"
   "@param targetModuleId The module Id to rename the copied module to including all references to the source module Id.It is valid to specifiy the source module Id to produce an identical copy.\n"
   "@param targetPath The target path to copy the module to.Addition folders will be created depending on whether 'useVersionPathing' is used or not.\n"
   "@param useVersionPathing Whether to add a '/targetModuleId/versionId' folder to the target path or not.This allows copying multiple versions of the same module Id.\n"
   "@return The new module definition file if copy was successful or NULL if not.\n")
{
   // Find the source module definition.
   ModuleDefinition* pSourceModuleDefinition = dynamic_cast<ModuleDefinition*>(Sim::findObject(sourceModuleDefinition));

   // Was the module definition found?
   if ( pSourceModuleDefinition == NULL )
   {
      // No, so warn.
      Con::warnf("ModuleManager::copyModule() - Could not find source module definition '%s'.", sourceModuleDefinition);
      return "";
   }

    // Copy module.
    return object->copyModule( pSourceModuleDefinition, pTargetModuleId, pTargetPath, useVersionPathing );
}

//-----------------------------------------------------------------------------

DefineEngineMethod(ModuleManager, synchronizeDependencies, bool, (const char* rootModuleDefinition, const char* pTargetDependencyFolder), ("", ""),
   "Synchronize the module dependencies of a module definition to a target dependency folder.\n"
   "@param rootModuleDefinition The module definition used to determine dependencies.\n"
   "@param targetDependencyPath The target dependency folder to copy dependencies to.\n"
   "@return Whether the module dependencies were synchronized correctly or not.\n")
{
    // Find the root module definition.
   ModuleDefinition* pRootModuleDefinition = dynamic_cast<ModuleDefinition*>(Sim::findObject(rootModuleDefinition));

    // Was the module definition found?
    if ( pRootModuleDefinition == NULL )
    {
        // No, so warn.
       Con::warnf("ModuleManager::synchronizeModules() - Could not find root module definition '%s'.", rootModuleDefinition);
        return false;
    }

    // Synchronize dependencies.
    return object->synchronizeDependencies( pRootModuleDefinition, pTargetDependencyFolder );
}

//-----------------------------------------------------------------------------

DefineEngineMethod(ModuleManager, isModuleMergeAvailable, bool, (),,
   "Checks whether a module merge definition file is available or not.\n"
   "@return Whether a module merge definition file is available or not.\n")
{
    // Check if module merge is available or not.
    return object->isModuleMergeAvailable();
}

//-----------------------------------------------------------------------------

DefineEngineMethod(ModuleManager, canMergeModules, bool, (const char* mergeSourcePath), (""),
   "Checks whether a module merge using the modules in the source path can current happen or not.\n"
   "@param mergeSourcePath The path where modules to be merged are located.\n"
   "@return Whether a module merge using the modules in the source path can current happen or not.\n")
{
    // Check whether the merge modules can current happen or not.
   return object->canMergeModules(mergeSourcePath);
}

//-----------------------------------------------------------------------------

DefineEngineMethod(ModuleManager, mergeModules, bool, (const char* pMergeTargetPath, bool removeMergeDefinition, bool registerNewModules), ("", false, false),
   "Performs a module merge into the selected target path.\n"
   "@param mergeTargetPath The path where modules will be merged into.\n"
   "@param removeMergeDefinition Whether to remove any merge definition found or not if merge is successful.\n"
   "@param registerNewModules Whether new (not replaced or updated) modules should be registered or not.\n"
   "@return Whether the module merge was successful or not.Failure here could result in a corrupt module state.Reinstall is recommended or at least advised to the user is recommended.\n")
{
    // Merge modules.
    return object->mergeModules( pMergeTargetPath, removeMergeDefinition, registerNewModules );
}

//-----------------------------------------------------------------------------

DefineEngineMethod(ModuleManager, addListener, void, (const char* listenerObject), (""),
   "Registers the specified object as a listener for module notifications.\n"
   "@param listenerObject The object to start receiving module notifications.\n"
   "@return No return value.\n")
{
    // Find object.
   SimObject* pListener = Sim::findObject(listenerObject);

    // Did we find the listener object?
    if ( pListener == NULL )
    {
        // No, so warn.
       Con::warnf("ModuleManager::addNotifications() - Could not find the listener object '%s'.", listenerObject);
        return;
    }

    object->addListener( pListener );
}

//-----------------------------------------------------------------------------

DefineEngineMethod(ModuleManager, removeListener, void, (const char* listenerObject), (""),
   "Unregisters the specified object as a listener for module notifications.\n"
   "@param listenerObject The object to stop receiving module notifications.\n"
   "@return No return value.\n")
{
    // Find object.
   SimObject* pListener = Sim::findObject(listenerObject);

    // Did we find the listener object?
    if ( pListener == NULL )
    {
        // No, so warn.
       Con::warnf("ModuleManager::removeNotifications() - Could not find the listener object '%s'.", listenerObject);
        return;
    }

    object->removeListener( pListener );
}
