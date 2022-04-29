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
#ifndef CPP_ASSET_H
#include "CppAsset.h"
#endif

#ifndef _ASSET_MANAGER_H_
#include "assets/assetManager.h"
#endif

#ifndef _CONSOLETYPES_H_
#include "console/consoleTypes.h"
#endif

#ifndef _TAML_
#include "persistence/taml/taml.h"
#endif

#ifndef _ASSET_PTR_H_
#include "assets/assetPtr.h"
#endif

// Debug Profiling.
#include "platform/profiler.h"

//-----------------------------------------------------------------------------

IMPLEMENT_CONOBJECT(CppAsset);

ConsoleType(CppAssetPtr, TypeCppAssetPtr, CppAsset, ASSET_ID_FIELD_PREFIX)

//-----------------------------------------------------------------------------

ConsoleGetType(TypeCppAssetPtr)
{
   // Fetch asset Id.
   return (*((AssetPtr<CppAsset>*)dptr)).getAssetId();
}

//-----------------------------------------------------------------------------

ConsoleSetType(TypeCppAssetPtr)
{
   // Was a single argument specified?
   if (argc == 1)
   {
      // Yes, so fetch field value.
      const char* pFieldValue = argv[0];

      // Fetch asset pointer.
      AssetPtr<CppAsset>* pAssetPtr = dynamic_cast<AssetPtr<CppAsset>*>((AssetPtrBase*)(dptr));

      // Is the asset pointer the correct type?
      if (pAssetPtr == NULL)
      {
         // No, so fail.
         //Con::warnf("(TypeCppAssetPtr) - Failed to set asset Id '%d'.", pFieldValue);
         return;
      }

      // Set asset.
      pAssetPtr->setAssetId(pFieldValue);

      return;
   }

   // Warn.
   Con::warnf("(TypeCppAssetPtr) - Cannot set multiple args to a single asset.");
}

//-----------------------------------------------------------------------------

CppAsset::CppAsset() : AssetBase()
{
   mCodeFile = StringTable->EmptyString();
   mHeaderFile = StringTable->EmptyString();
}

//-----------------------------------------------------------------------------

CppAsset::~CppAsset()
{
   // If the asset manager does not own the asset then we own the
   // asset definition so delete it.
   if (!getOwned())
      delete mpAssetDefinition;
}

//-----------------------------------------------------------------------------

void CppAsset::initPersistFields()
{
   // Call parent.
   Parent::initPersistFields();

   addProtectedField("codeFile", TypeAssetLooseFilePath, Offset(mCodeFile, CppAsset),
      &setCppFile, &getCppFile, "Path to the cpp file.");

   addProtectedField("headerFile", TypeAssetLooseFilePath, Offset(mHeaderFile, CppAsset),
      &setHeaderFile, &getHeaderFile, "Path to the h file.");

}

//------------------------------------------------------------------------------

void CppAsset::copyTo(SimObject* object)
{
   // Call to parent.
   Parent::copyTo(object);
}

void CppAsset::setCppFile(const char* pCppFile)
{
   // Sanity!
   AssertFatal(pCppFile != NULL, "Cannot use a NULL code file.");

   // Fetch image file.
   pCppFile = StringTable->insert(pCppFile);

   // Ignore no change,
   if (pCppFile == mCodeFile)
      return;

   // Update.
   mCodeFile = getOwned() ? expandAssetFilePath(pCppFile) : StringTable->insert(pCppFile);

   // Refresh the asset.
   refreshAsset();
}

void CppAsset::setHeaderFile(const char* pHeaderFile)
{
   // Sanity!
   AssertFatal(pHeaderFile != NULL, "Cannot use a NULL header file.");

   // Fetch image file.
   pHeaderFile = StringTable->insert(pHeaderFile);

   // Ignore no change,
   if (pHeaderFile == mHeaderFile)
      return;

   // Update.
   mHeaderFile = getOwned() ? expandAssetFilePath(pHeaderFile) : StringTable->insert(pHeaderFile);

   // Refresh the asset.
   refreshAsset();
}
