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

#ifndef _ASSET_FIELD_TYPES_H_
#define _ASSET_FIELD_TYPES_H_

#ifndef _CONSOLE_BASE_TYPE_H_
#include "console/consoleTypes.h"
#endif

//-----------------------------------------------------------------------------

DefineConsoleType( TypeAssetId, S32 )
DefineConsoleType( TypeAssetLooseFilePath, String )

//-----------------------------------------------------------------------------

/// Asset scope.
#define ASSET_SCOPE_TOKEN				":"

/// Asset assignment.
#define ASSET_ASSIGNMENT_TOKEN			"="

/// Asset Id.
#define ASSET_ID_SIGNATURE              "@asset"
#define ASSET_ID_FIELD_PREFIX           "@asset="

/// Asset loose file.
#define ASSET_LOOSEFILE_SIGNATURE       "@assetFile"
#define ASSET_LOOSE_FILE_FIELD_PREFIX   "@assetFile="

//-----------------------------------------------------------------------------

extern StringTableEntry assetLooseIdSignature;
extern StringTableEntry assetLooseFileSignature;

#endif // _ASSET_FIELD_TYPES_H_