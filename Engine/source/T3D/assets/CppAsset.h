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
#define CPP_ASSET_H
#pragma once

#ifndef _ASSET_BASE_H_
#include "assets/assetBase.h"
#endif

#ifndef _ASSET_DEFINITION_H_
#include "assets/assetDefinition.h"
#endif

#ifndef _STRINGUNIT_H_
#include "string/stringUnit.h"
#endif

#ifndef _ASSET_FIELD_TYPES_H_
#include "assets/assetFieldTypes.h"
#endif

//-----------------------------------------------------------------------------
class CppAsset : public AssetBase
{
   typedef AssetBase Parent;

   StringTableEntry        mCodeFile;
   StringTableEntry        mHeaderFile;

public:
   CppAsset();
   virtual ~CppAsset();

   /// Engine.
   static void initPersistFields();
   virtual void copyTo(SimObject* object);

   /// Declare Console Object.
   DECLARE_CONOBJECT(CppAsset);

   void                    setCppFile(const char* pCppFile);
   inline StringTableEntry getCppFile(void) const { return mCodeFile; };

   void                    setHeaderFile(const char* pHeaderFile);
   inline StringTableEntry getHeaderFile(void) const { return mHeaderFile; };

protected:
	virtual void            initializeAsset(void) {};
	virtual void            onAssetRefresh(void) {};

   static bool setCppFile(void *obj, const char *index, const char *data) { static_cast<CppAsset*>(obj)->setCppFile(data); return false; }
   static const char* getCppFile(void* obj, const char* data) { return static_cast<CppAsset*>(obj)->getCppFile(); }

   static bool setHeaderFile(void *obj, const char *index, const char *data) { static_cast<CppAsset*>(obj)->setHeaderFile(data); return false; }
   static const char* getHeaderFile(void* obj, const char* data) { return static_cast<CppAsset*>(obj)->getHeaderFile(); }
};

DefineConsoleType(TypeCppAssetPtr, CppAsset)

#endif // _ASSET_BASE_H_

