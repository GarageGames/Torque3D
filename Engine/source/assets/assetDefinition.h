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

#ifndef _ASSET_DEFINITION_H_
#define _ASSET_DEFINITION_H_

#ifndef _STRINGTABLE_H_
#include "core/stringTable.h"
#endif

#ifndef _STRINGUNIT_H_
#include "core/strings/stringUnit.h"
#endif

#ifndef _SIM_H_
#include "console/sim.h"
#endif

#ifndef _SIMOBJECT_H_
#include "console/simObject.h"
#endif

#ifndef _CONSOLEOBJECT_H_
#include "console/consoleObject.h"
#endif

//-----------------------------------------------------------------------------

class AssetBase;
class ModuleDefinition;

//-----------------------------------------------------------------------------

struct AssetDefinition
{
public:
    AssetDefinition() { reset(); }
    virtual ~AssetDefinition() {}

    virtual void reset( void )
    {
        mAssetLoading = false;
        mpModuleDefinition = NULL;
        mpAssetBase = NULL;
        mAssetBaseFilePath = StringTable->EmptyString();
        mAssetId = StringTable->EmptyString();
        mAssetLoadedCount = 0;
        mAssetUnloadedCount = 0;
        mAssetRefreshEnable = true;
        mAssetLooseFiles.clear();

        // Reset persisted state.
        mAssetName = StringTable->EmptyString();
        mAssetDescription = StringTable->EmptyString();
        mAssetAutoUnload = true;
        mAssetInternal = false;
        mAssetPrivate = false;
        mAssetType = StringTable->EmptyString();
        mAssetCategory = StringTable->EmptyString();
    }

    ModuleDefinition*           mpModuleDefinition;
    AssetBase*                  mpAssetBase;
    StringTableEntry            mAssetBaseFilePath;
    StringTableEntry            mAssetId;
    U32                         mAssetLoadedCount;
    U32                         mAssetUnloadedCount;
    bool                        mAssetRefreshEnable;
    Vector<StringTableEntry>    mAssetLooseFiles;

    /// Persisted state.
    StringTableEntry            mAssetName;
    StringTableEntry            mAssetDescription;
    bool                        mAssetAutoUnload;
    bool                        mAssetInternal; 
    bool                        mAssetPrivate;
    bool                        mAssetLoading;
    StringTableEntry            mAssetType;
    StringTableEntry            mAssetCategory;
};

#endif // _ASSET_DEFINITION_H_

