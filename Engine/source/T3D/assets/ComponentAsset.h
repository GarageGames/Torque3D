#pragma once
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
#ifndef COMPONENT_ASSET_H
#define COMPONENT_ASSET_H

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
class ComponentAsset : public AssetBase
{
   typedef AssetBase Parent;

   StringTableEntry mComponentName;
   StringTableEntry mComponentClass;
   StringTableEntry mFriendlyName;
   StringTableEntry mComponentType;
   StringTableEntry mDescription;

   StringTableEntry mScriptFile;

public:
   ComponentAsset();
   virtual ~ComponentAsset();

   /// Engine.
   static void initPersistFields();
   virtual void copyTo(SimObject* object);

   /// Declare Console Object.
   DECLARE_CONOBJECT(ComponentAsset);

   StringTableEntry getComponentName() { return mComponentName; }
   StringTableEntry getComponentClass() { return mComponentClass; }
   StringTableEntry getFriendlyName() { return mFriendlyName; }
   StringTableEntry getComponentType() { return mComponentType; }
   StringTableEntry getDescription() { return mDescription; }

   void setComponentName(StringTableEntry name) { mComponentName = name; }
   void setComponentClass(StringTableEntry name) { mComponentClass = name; }
   void setFriendlyName(StringTableEntry name) { mFriendlyName = name; }
   void setComponentType(StringTableEntry typeName) { mComponentType = typeName; }
   void setDescription(StringTableEntry description) { mDescription = description; }

   AssetDefinition* getAssetDefinition() { return mpAssetDefinition; }

   void                    setScriptFile(const char* pScriptFile);
   inline StringTableEntry getScriptFile(void) const { return mScriptFile; };

protected:
   virtual void            initializeAsset(void);
   virtual void            onAssetRefresh(void);

   static bool setScriptFile(void *obj, const char *index, const char *data) { static_cast<ComponentAsset*>(obj)->setScriptFile(data); return false; }
   static const char* getScriptFile(void* obj, const char* data) { return static_cast<ComponentAsset*>(obj)->getScriptFile(); }
};

DefineConsoleType(TypeComponentAssetPtr, ComponentAsset)

#endif // _ASSET_BASE_H_

