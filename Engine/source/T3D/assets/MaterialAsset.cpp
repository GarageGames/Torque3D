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

#ifndef MATERIALASSET_H
#include "MaterialAsset.h"
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

//-----------------------------------------------------------------------------

IMPLEMENT_CONOBJECT(MaterialAsset);

ConsoleType(MaterialAssetPtr, TypeMaterialAssetPtr, MaterialAsset, ASSET_ID_FIELD_PREFIX)

//-----------------------------------------------------------------------------

ConsoleGetType(TypeMaterialAssetPtr)
{
   // Fetch asset Id.
   return (*((AssetPtr<MaterialAsset>*)dptr)).getAssetId();
}

//-----------------------------------------------------------------------------

ConsoleSetType(TypeMaterialAssetPtr)
{
   // Was a single argument specified?
   if (argc == 1)
   {
      // Yes, so fetch field value.
      const char* pFieldValue = argv[0];

      // Fetch asset pointer.
      AssetPtr<MaterialAsset>* pAssetPtr = dynamic_cast<AssetPtr<MaterialAsset>*>((AssetPtrBase*)(dptr));

      // Is the asset pointer the correct type?
      if (pAssetPtr == NULL)
      {
         // No, so fail.
         //Con::warnf("(TypeMaterialAssetPtr) - Failed to set asset Id '%d'.", pFieldValue);
         return;
      }

      // Set asset.
      pAssetPtr->setAssetId(pFieldValue);

      return;
   }

   // Warn.
   Con::warnf("(TypeMaterialAssetPtr) - Cannot set multiple args to a single asset.");
}

//-----------------------------------------------------------------------------

MaterialAsset::MaterialAsset()
{
   mShaderGraphFile = "";
   mScriptFile = "";
   mMatDefinitionName = "";
}

//-----------------------------------------------------------------------------

MaterialAsset::~MaterialAsset()
{
}

//-----------------------------------------------------------------------------

void MaterialAsset::initPersistFields()
{
   // Call parent.
   Parent::initPersistFields();

   //addField("shaderGraph", TypeRealString, Offset(mShaderGraphFile, MaterialAsset), "");
   addField("scriptFile", TypeRealString, Offset(mScriptFile, MaterialAsset), "Path to the file containing the material definition.");
   addField("materialDefinitionName", TypeRealString, Offset(mMatDefinitionName, MaterialAsset), "Name of the material definition this asset is for.");
}

void MaterialAsset::initializeAsset()
{
   // Call parent.
   Parent::initializeAsset();

   compileShader();

   if (Platform::isFile(mScriptFile))
      Con::executeFile(mScriptFile, false, false);
}

void MaterialAsset::onAssetRefresh()
{
   if (Platform::isFile(mScriptFile))
      Con::executeFile(mScriptFile, false, false);

   if (mMatDefinitionName != StringTable->EmptyString())
   {
      Material* matDef;
      if (!Sim::findObject(mMatDefinitionName, matDef))
      {
         Con::errorf("MaterialAsset: Unable to find the Material %s", mMatDefinitionName);
         return;
      }

      matDef->reload();
   }
}

//------------------------------------------------------------------------------

void MaterialAsset::compileShader()
{
}

void MaterialAsset::copyTo(SimObject* object)
{
   // Call to parent.
   Parent::copyTo(object);
}

DefineEngineMethod(MaterialAsset, compileShader, void, (), , "Compiles the material's generated shader, if any. Not yet implemented\n")
{
   object->compileShader();
}

//-----------------------------------------------------------------------------
// GuiInspectorTypeAssetId
//-----------------------------------------------------------------------------

IMPLEMENT_CONOBJECT(GuiInspectorTypeMaterialAssetPtr);

ConsoleDocClass(GuiInspectorTypeMaterialAssetPtr,
   "@brief Inspector field type for Material Asset Objects\n\n"
   "Editor use only.\n\n"
   "@internal"
);

void GuiInspectorTypeMaterialAssetPtr::consoleInit()
{
   Parent::consoleInit();

   ConsoleBaseType::getType(TypeMaterialAssetPtr)->setInspectorFieldType("GuiInspectorTypeMaterialAssetPtr");
}

GuiControl* GuiInspectorTypeMaterialAssetPtr::constructEditControl()
{
   // Create base filename edit controls
   mUseHeightOverride = true;
   mHeightOverride = 100;

   mMatEdContainer = new GuiControl();
   mMatEdContainer->registerObject();

   addObject(mMatEdContainer);

   // Create "Open in ShapeEditor" button
   mMatPreviewButton = new GuiBitmapButtonCtrl();

   const char* matAssetId = getData();

   MaterialAsset* matAsset = AssetDatabase.acquireAsset< MaterialAsset>(matAssetId);

   Material* materialDef = nullptr;

   char bitmapName[512] = "tools/worldEditor/images/toolbar/shape-editor";

   if (!Sim::findObject(matAsset->getMaterialDefinitionName(), materialDef))
   {
      Con::errorf("GuiInspectorTypeMaterialAssetPtr::constructEditControl() - unable to find material in asset");
   }
   else
   {
      mMatPreviewButton->setBitmap(materialDef->mDiffuseMapFilename[0]);
   }

   mMatPreviewButton->setPosition(0, 0);
   mMatPreviewButton->setExtent(100,100);

   // Change filespec
   char szBuffer[512];
   dSprintf(szBuffer, sizeof(szBuffer), "AssetBrowser.showDialog(\"MaterialAsset\", \"AssetBrowser.changeAsset\", %d, %s);",
      mInspector->getComponentGroupTargetId(), mCaption);
   mMatPreviewButton->setField("Command", szBuffer);

   mMatPreviewButton->setDataField(StringTable->insert("Profile"), NULL, "GuiButtonProfile");
   mMatPreviewButton->setDataField(StringTable->insert("tooltipprofile"), NULL, "GuiToolTipProfile");
   mMatPreviewButton->setDataField(StringTable->insert("hovertime"), NULL, "1000");

   StringBuilder strbld;
   strbld.append(matAsset->getMaterialDefinitionName());
   strbld.append("\n");
   strbld.append("Open this file in the Material Editor");

   mMatPreviewButton->setDataField(StringTable->insert("tooltip"), NULL, strbld.data());

   _registerEditControl(mMatPreviewButton);
   //mMatPreviewButton->registerObject();
   mMatEdContainer->addObject(mMatPreviewButton);

   mMatAssetIdTxt = new GuiTextEditCtrl();
   mMatAssetIdTxt->registerObject();
   mMatAssetIdTxt->setActive(false);

   mMatAssetIdTxt->setText(matAssetId);

   mMatAssetIdTxt->setBounds(100, 0, 150, 18);
   mMatEdContainer->addObject(mMatAssetIdTxt);

   return mMatEdContainer;
}

bool GuiInspectorTypeMaterialAssetPtr::updateRects()
{
   S32 dividerPos, dividerMargin;
   mInspector->getDivider(dividerPos, dividerMargin);
   Point2I fieldExtent = getExtent();
   Point2I fieldPos = getPosition();

   mCaptionRect.set(0, 0, fieldExtent.x - dividerPos - dividerMargin, fieldExtent.y);
   mEditCtrlRect.set(fieldExtent.x - dividerPos + dividerMargin, 1, dividerPos - dividerMargin - 34, fieldExtent.y);

   bool resized = mEdit->resize(mEditCtrlRect.point, mEditCtrlRect.extent);

   if (mMatEdContainer != nullptr)
   {
      mMatPreviewButton->resize(mEditCtrlRect.point, mEditCtrlRect.extent);
   }

   if (mMatPreviewButton != nullptr)
   {
      mMatPreviewButton->resize(Point2I::Zero, Point2I(100, 100));
   }

   if (mMatAssetIdTxt != nullptr)
   {
      mMatAssetIdTxt->resize(Point2I(100, 0), Point2I(mEditCtrlRect.extent.x - 100, 18));
   }
   /*if (mMatPreviewButton != NULL)
   {
      RectI shapeEdRect(fieldExtent.x - 16, 2, 100, fieldExtent.y - 4);
      resized |= mMatPreviewButton->resize(shapeEdRect.point, shapeEdRect.extent);
   }*/

   return resized;
}

void GuiInspectorTypeMaterialAssetPtr::setMaterialAsset(String assetId)
{
   mTargetObject->setDataField(mCaption, "", assetId);

   //force a refresh
   SimObject* obj = mInspector->getInspectObject();
   mInspector->inspectObject(obj);
}

DefineEngineMethod(GuiInspectorTypeMaterialAssetPtr, setMaterialAsset, void, (String assetId), (""),
   "Gets a particular shape animation asset for this shape.\n"
   "@param animation asset index.\n"
   "@return Shape Animation Asset.\n")
{
   if (assetId == String::EmptyString)
      return;

   return object->setMaterialAsset(assetId);
}
