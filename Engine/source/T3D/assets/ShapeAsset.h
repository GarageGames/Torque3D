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
#ifndef SHAPE_ASSET_H
#define SHAPE_ASSET_H

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

#ifndef _TSSHAPE_H_
#include "ts/tsShape.h"
#endif
#ifndef __RESOURCE_H__
#include "core/resource.h"
#endif
#ifndef _ASSET_PTR_H_
#include "assets/assetPtr.h"
#endif 
#ifndef MATERIALASSET_H
#include "MaterialAsset.h"
#endif
#ifndef SHAPE_ANIMATION_ASSET_H
#include "ShapeAnimationAsset.h"
#endif

#include "gui/editor/guiInspectorTypes.h"

//-----------------------------------------------------------------------------
class ShapeAsset : public AssetBase
{
   typedef AssetBase Parent;

protected:
   StringTableEntry   mFileName;
   Resource<TSShape>	 mShape;

   //Material assets we're dependent on and use
   Vector<StringTableEntry> mMaterialAssetIds;
   Vector<AssetPtr<MaterialAsset>> mMaterialAssets;

   //Animation assets we're dependent on and use
   Vector<StringTableEntry> mAnimationAssetIds;
   Vector<AssetPtr<ShapeAnimationAsset>> mAnimationAssets;

public:
   ShapeAsset();
   virtual ~ShapeAsset();

   /// Engine.
   static void initPersistFields();
   virtual void copyTo(SimObject* object);

   virtual void setDataField(StringTableEntry slotName, const char *array, const char *value);

   virtual void initializeAsset();

   /// Declare Console Object.
   DECLARE_CONOBJECT(ShapeAsset);

   bool loadShape();

   TSShape* getShape() { return mShape; }

   Resource<TSShape> getShapeResource() { return mShape; }

   void SplitSequencePathAndName(String& srcPath, String& srcName);
   String getShapeFilename() { return mFileName; }
   
   U32 getShapeFilenameHash() { return _StringTable::hashString(mFileName); }

   S32 getMaterialCount() { return mMaterialAssets.size(); }
   S32 getAnimationCount() { return mAnimationAssets.size(); }
   ShapeAnimationAsset* getAnimation(S32 index);

protected:
   virtual void            onAssetRefresh(void);
};

DefineConsoleType(TypeShapeAssetPtr, S32)

//-----------------------------------------------------------------------------
// TypeAssetId GuiInspectorField Class
//-----------------------------------------------------------------------------
class GuiInspectorTypeShapeAssetPtr : public GuiInspectorTypeFileName
{
   typedef GuiInspectorTypeFileName Parent;
public:

   GuiBitmapButtonCtrl  *mShapeEdButton;

   DECLARE_CONOBJECT(GuiInspectorTypeShapeAssetPtr);
   static void consoleInit();

   virtual GuiControl* constructEditControl();
   virtual bool updateRects();
};

#endif

