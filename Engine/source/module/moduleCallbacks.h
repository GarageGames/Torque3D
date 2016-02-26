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

#ifndef _MODULE_CALLBACKS_H_
#define _MODULE_CALLBACKS_H_

#ifndef _MODULE_DEFINITION_H
#include "moduleDefinition.h"
#endif

//-----------------------------------------------------------------------------

/// @ingroup moduleGroup
/// @see moduleGroup
class ModuleCallbacks
{
    friend class ModuleManager;

private:
    // Called when a module is about to be loaded.
    virtual void onModulePreLoad( ModuleDefinition* pModuleDefinition ) {}

    // Called when a module has been loaded.
    virtual void onModulePostLoad( ModuleDefinition* pModuleDefinition ) {}

    // Called when a module is about to be unloaded.
    virtual void onModulePreUnload( ModuleDefinition* pModuleDefinition ) {}

    // Called when a module has been unloaded.
    virtual void onModulePostUnload( ModuleDefinition* pModuleDefinition ) {}
};

#endif // _MODULE_CALLBACKS_H_
