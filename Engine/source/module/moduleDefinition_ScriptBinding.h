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

DefineEngineMethod(ModuleDefinition, save, bool, (),,
   "Saves the module definition to the file it was loaded from (if any).\n"
   "@return (bool success) Whether the module definition was saved or not.\n")
{
    // Save.
    return object->save();
}

//-----------------------------------------------------------------------------

DefineEngineMethod(ModuleDefinition, getModuleManager, S32, (),,
   "Gets the module manager which this module definition is registered with (if any).\n"
   "@return (moduleManager) The module manager which this module definition is registered with (zero if not registered).\n")
{
    // Fetch module manager.
    ModuleManager* pModuleManager = object->getModuleManager();

    return pModuleManager != NULL ? pModuleManager->getId() : 0;
}

//-----------------------------------------------------------------------------

DefineEngineMethod(ModuleDefinition, getDependencyCount, S32, (), ,
   "Gets the number of module dependencies this module definition has.\n"
   "@return (int count) The number of module dependencies this module definition has.\n")
{
    // Get module dependency count.
    return object->getDependencyCount();
}

//-----------------------------------------------------------------------------

DefineEngineMethod(ModuleDefinition, getDependency, String, (U32 dependencyIndex), (0), 
   "Gets the module dependency at the specified index.\n"
   "@param dependencyIndex The module dependency index.\n"
   "@return (module - dependency) The module dependency at the specified index.")
{
    // Get module dependency.
    ModuleDefinition::ModuleDependency dependency;
    if ( object->getDependency( dependencyIndex, dependency ) == false )
        return StringTable->EmptyString();

    // Format module dependency.
    char* pReturnBuffer = Con::getReturnBuffer( 256 );
    dSprintf( pReturnBuffer, 256, "%s %d", dependency.mModuleId, dependency.mVersionId );

    return pReturnBuffer;
}

//-----------------------------------------------------------------------------

DefineEngineMethod(ModuleDefinition, addDependency, bool, (const char* pModuleId, U32 versionId), ("", 0),
   "Adds the specified moduleId and vesionId as a dependency.\n"
   "@param moduleId The module Id to add as a dependency.\n"
   "@param versionId The version Id to add as a dependency.  Using zero indicates any version."
   "@return (bool success) Whether the module dependency was added or not.")
{
    // Add dependency.
    return object->addDependency( pModuleId, versionId );
}

//-----------------------------------------------------------------------------

DefineEngineMethod(ModuleDefinition, removeDependency, bool, (const char* pModuleId), (""),
   "Removes the specified moduleId as a dependency.\n"
   "@param moduleId The module Id to remove as a dependency.\n"
   "@return (bool success) Whether the module dependency was removed or not.")
{
    // Remove dependency.
    return object->removeDependency( pModuleId );
}
