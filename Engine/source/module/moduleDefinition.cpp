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

#include "moduleDefinition.h"

#ifndef _MODULE_MANAGER_H
#include "moduleManager.h"
#endif

// Script bindings.
#include "moduleDefinition_ScriptBinding.h"

#ifndef _CONSOLETYPES_H_
#include "console/consoleTypes.h"
#endif

#ifndef _TAML_H_
#include "persistence/taml/taml.h"
#endif

//-----------------------------------------------------------------------------

IMPLEMENT_CONOBJECT( ModuleDefinition );

//-----------------------------------------------------------------------------

ModuleDefinition::ModuleDefinition() :
mModuleId(StringTable->EmptyString()),
    mVersionId( 0 ),
    mBuildId( 0 ),
    mEnabled( true ),
    mSynchronized( false ),
    mDeprecated( false ),
    mCriticalMerge( false ),
    mModuleDescription( StringTable->EmptyString() ),
    mAuthor(StringTable->EmptyString()),
    mModuleGroup(StringTable->EmptyString()),
    mModuleType(StringTable->EmptyString()),
    mScriptFile(StringTable->EmptyString()),
    mCreateFunction(StringTable->EmptyString()),
    mDestroyFunction(StringTable->EmptyString()),
    mAssetTagsManifest(StringTable->EmptyString()),
    mModulePath(StringTable->EmptyString()),
    mModuleFile(StringTable->EmptyString()),
    mModuleFilePath(StringTable->EmptyString()),
    mModuleScriptFilePath(StringTable->EmptyString()),
    mSignature(StringTable->EmptyString()),
    mLoadCount( 0 ),
    mLocked( false ),
    mScopeSet( 0 ),
    mpModuleManager( NULL )
{
    // Set Vector Associations.
    VECTOR_SET_ASSOCIATION( mDependencies );
    VECTOR_SET_ASSOCIATION( mModuleAssets );
}

//-----------------------------------------------------------------------------

void ModuleDefinition::initPersistFields()
{
    // Call parent.
    Parent::initPersistFields();

    addProtectedField("ModuleId", TypeString, Offset(mModuleId, ModuleDefinition), &defaultProtectedSetFn, &defaultProtectedGetFn, "");

    /// Module configuration.
    addProtectedField( "ModuleId", TypeString, Offset(mModuleId, ModuleDefinition), &setModuleId, &defaultProtectedGetFn, "A unique string Id for the module.  It can contain any characters except a comma or semi-colon (the asset scope character)." );
    addProtectedField( "VersionId", TypeS32, Offset(mVersionId, ModuleDefinition), &setVersionId, &defaultProtectedGetFn, "The version Id.  Breaking changes to a module should use a higher version Id." );
    addProtectedField( "BuildId", TypeS32, Offset(mBuildId, ModuleDefinition), &setBuildId, &defaultProtectedGetFn, &writeBuildId, "The build Id.  Non-breaking changes to a module should use a higher build Id.  Optional: If not specified then the build Id will be zero." );
    addProtectedField( "Enabled", TypeBool, Offset(mEnabled, ModuleDefinition), &setEnabled, &defaultProtectedGetFn, &writeEnabled, "Whether the module is enabled or not.  When disabled, it is effectively ignored.  Optional: If not specified then the module is enabled." );
    addProtectedField( "Synchronized", TypeBool, Offset(mSynchronized, ModuleDefinition), &setSynchronized, &defaultProtectedGetFn, &writeSynchronized, "Whether the module should be synchronized or not.  Optional: If not specified then the module is not synchronized." );
    addProtectedField( "Deprecated", TypeBool, Offset(mDeprecated, ModuleDefinition), &setDeprecated, &defaultProtectedGetFn, &writeDeprecated, "Whether the module is deprecated or not.  Optional: If not specified then the module is not deprecated." );
    addProtectedField( "CriticalMerge", TypeBool, Offset(mCriticalMerge, ModuleDefinition), &setDeprecated, &defaultProtectedGetFn, &writeCriticalMerge, "Whether the merging of a module prior to a restart is critical or not.  Optional: If not specified then the module is not merge critical." );
    addProtectedField( "Description", TypeString, Offset(mModuleDescription, ModuleDefinition), &setModuleDescription, &defaultProtectedGetFn, &writeModuleDescription, "The description typically used for debugging purposes but can be used for anything." );
    addProtectedField( "Author", TypeString, Offset(mAuthor, ModuleDefinition), &setAuthor, &defaultProtectedGetFn, &writeAuthor, "The author of the module." );
    addProtectedField( "Group", TypeString, Offset(mModuleGroup, ModuleDefinition), &setModuleGroup, &defaultProtectedGetFn, "The module group used typically when loading modules as a group." );
    addProtectedField( "Type", TypeString, Offset(mModuleType, ModuleDefinition), &setModuleType, &defaultProtectedGetFn, &writeModuleType, "The module type typically used to distinguish modules during module enumeration.  Optional: If not specified then the type is empty although this can still be used as a pseudo 'global' type for instance." );
    addProtectedField( "Dependencies", TypeString, Offset(mDependencies, ModuleDefinition), &setDependencies, &getDependencies, &writeDependencies, "A comma-separated list of module Ids/VersionIds (<ModuleId>=<VersionId>,<ModuleId>=<VersionId>,etc) which this module depends upon. Optional: If not specified then no dependencies are assumed." );
    addProtectedField( "ScriptFile", TypeString, Offset(mScriptFile, ModuleDefinition), &setScriptFile, &defaultProtectedGetFn, &writeScriptFile, "The name of the script file to compile when loading the module.  Optional." );
    addProtectedField( "CreateFunction", TypeString, Offset(mCreateFunction, ModuleDefinition), &setCreateFunction, &defaultProtectedGetFn, &writeCreateFunction, "The name of the function used to create the module.  Optional: If not specified then no create function is called." );
    addProtectedField( "DestroyFunction", TypeString, Offset(mDestroyFunction, ModuleDefinition), &setDestroyFunction, &defaultProtectedGetFn, &writeDestroyFunction, "The name of the function used to destroy the module.  Optional: If not specified then no destroy function is called." );
    addProtectedField( "AssetTagsManifest", TypeString, Offset(mAssetTagsManifest, ModuleDefinition), &setAssetTagsManifest, &defaultProtectedGetFn, &writeAssetTagsManifest, "The name of tags asset manifest file if this module contains asset tags.  Optional: If not specified then no asset tags will be found for this module.  Currently, only a single asset tag manifest should exist." );
    addProtectedField( "ScopeSet", TypeS32, Offset( mScopeSet, ModuleDefinition ), &defaultProtectedNotSetFn, &getScopeSet, &defaultProtectedNotWriteFn, "The scope set used to control the lifetime scope of objects that the module uses.  Objects added to this set are destroyed automatically when the module is unloaded." );

    /// Module location (Read-only).
    addProtectedField( "ModulePath", TypeString, Offset(mModulePath, ModuleDefinition), &defaultProtectedNotSetFn, &defaultProtectedGetFn, &defaultProtectedNotWriteFn, "The path of the module.  This is read-only and is available only after the module has been registered by a module manager." );
    addProtectedField( "ModuleFile", TypeString, Offset(mModuleFile, ModuleDefinition), &defaultProtectedNotSetFn, &defaultProtectedGetFn, &defaultProtectedNotWriteFn, "The file of the module.  This is read-only and is available only after the module has been registered by a module manager." );
    addProtectedField( "ModuleFilePath", TypeString, Offset(mModuleFilePath, ModuleDefinition), &defaultProtectedNotSetFn, &defaultProtectedGetFn, &defaultProtectedNotWriteFn, "The file-path of the module definition.  This is read-only and is available only after the module has been registered by a module manager." );
    addProtectedField( "ModuleScriptFilePath", TypeString, Offset(mModuleScriptFilePath, ModuleDefinition), &defaultProtectedNotSetFn, &defaultProtectedGetFn, &defaultProtectedNotWriteFn, "The file-path of the script-file referenced in the module definition.  This is read-only and is available only after the module has been registered by a module manager." );

    /// Misc.
    addProtectedField( "Signature", TypeString, 0, &defaultProtectedNotSetFn, &getSignature, &defaultProtectedNotWriteFn, "A unique signature of the module definition based upon its Id, version and build.  This is read-only and is available only after the module has been registered by a module manager." );
}

//-----------------------------------------------------------------------------

bool ModuleDefinition::getDependency( const U32 dependencyIndex, ModuleDependency& dependency ) const
{
    // Is dependency index out of bounds?
    if ( dependencyIndex >= (U32)mDependencies.size() )
    {
        // Yes, so warn.
        Con::warnf("Could not get module dependency '%d' as it is out of range.", dependencyIndex);
        return false;
    }

    // Fetch module dependency.
    dependency = mDependencies[dependencyIndex];

    return true;
}

//-----------------------------------------------------------------------------

bool ModuleDefinition::addDependency( const char* pModuleId, const U32 versionId )
{
    // Fetch module Id.
    StringTableEntry moduleId = StringTable->insert( pModuleId );

    // Do we have any existing dependencies?
    if ( mDependencies.size() > 0 )
    {
        // Yes, so is the module Id already a dependency?
        for( typeModuleDependencyVector::iterator dependencyItr = mDependencies.begin(); dependencyItr != mDependencies.end(); ++dependencyItr )
        {
            // Skip if not the same module Id.
            if ( dependencyItr->mModuleId != moduleId )
                continue;

            // Dependency already exists so warn.
            Con::warnf("Could not add dependency of module Id '%s' at version Id '%d' as the module Id is already a dependency.", pModuleId, versionId );
            return false;
        }
    }

    // Populate module dependency.
    ModuleDefinition::ModuleDependency dependency( moduleId, versionId );

    // Store dependency.
    mDependencies.push_back( dependency );

    return true;
}

//-----------------------------------------------------------------------------

bool ModuleDefinition::removeDependency( const char* pModuleId )
{
    // Fetch module Id.
    StringTableEntry moduleId = StringTable->insert( pModuleId );

    // Do we have any existing dependencies?
    if ( mDependencies.size() > 0 )
    {
        // Yes, so is the module Id a dependency?
        for( typeModuleDependencyVector::iterator dependencyItr = mDependencies.begin(); dependencyItr != mDependencies.end(); ++dependencyItr )
        {
            // Skip if not the same module Id.
            if ( dependencyItr->mModuleId != moduleId )
                continue;

            // Remove dependency.
            mDependencies.erase( dependencyItr );

            return true;
        }
    }

    // No, so warn.
    Con::warnf("Could not remove dependency of module Id '%s' as the module Id is not a dependency.", pModuleId );
    return false;
}

//-----------------------------------------------------------------------------

bool ModuleDefinition::save( void )
{
    // Does the module have a file-path yet?
   if (mModuleFilePath == StringTable->EmptyString())
    {
        // No, so warn.
        Con::warnf("Save() - Cannot save module definition '%s' as it does not have a file-path.", mModuleId );
        return false;
    }

    // Save the module file.
    Taml taml;
    return taml.write( this, mModuleFilePath );
}
