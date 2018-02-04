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

#ifndef _SHAPE_ASSET_H_
#include "ShapeAsset.h"
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

#include "core/resourceManager.h"

// Debug Profiling.
#include "platform/profiler.h"

//-----------------------------------------------------------------------------

IMPLEMENT_CONOBJECT(ShapeAsset);

ConsoleType(assetIdString, TypeShapeAssetPtr, String, ASSET_ID_FIELD_PREFIX)

//-----------------------------------------------------------------------------

ConsoleGetType(TypeShapeAssetPtr)
{
   // Fetch asset Id.
   return *((StringTableEntry*)dptr);
}

//-----------------------------------------------------------------------------

ConsoleSetType(TypeShapeAssetPtr)
{
   // Was a single argument specified?
   if (argc == 1)
   {
      // Yes, so fetch field value.
      const char* pFieldValue = argv[0];

      // Fetch asset Id.
      StringTableEntry* assetId = (StringTableEntry*)(dptr);

      // Update asset value.
      *assetId = StringTable->insert(pFieldValue);

      return;
   }

   // Warn.
   Con::warnf("(TypeAssetId) - Cannot set multiple args to a single asset.");
}

//-----------------------------------------------------------------------------

ShapeAsset::ShapeAsset()
{
}

//-----------------------------------------------------------------------------

ShapeAsset::~ShapeAsset()
{
   // If the asset manager does not own the asset then we own the
   // asset definition so delete it.
   if (!getOwned())
      delete mpAssetDefinition;
}

//-----------------------------------------------------------------------------

void ShapeAsset::initPersistFields()
{
   // Call parent.
   Parent::initPersistFields();

   addField("fileName", TypeFilename, Offset(mFileName, ShapeAsset), "Path to the shape file we want to render");
}

void ShapeAsset::setDataField(StringTableEntry slotName, const char *array, const char *value)
{
   Parent::setDataField(slotName, array, value);

   //Now, if it's a material slot of some fashion, set it up
   StringTableEntry matSlotName = StringTable->insert("materialAsset");
   if (String(slotName).startsWith(matSlotName))
   {
      StringTableEntry matId = StringTable->insert(value);

      mMaterialAssetIds.push_back(matId);
   }
}

void ShapeAsset::initializeAsset()
{
   // Call parent.
   Parent::initializeAsset();

   if (dStrcmp(mFileName, "") == 0)
      return;

   loadShape();
}

bool ShapeAsset::loadShape()
{
   mMaterialAssets.clear();
   mMaterialAssetIds.clear();

   //First, load any material, animation, etc assets we may be referencing in our asset
   // Find any asset dependencies.
   AssetManager::typeAssetDependsOnHash::Iterator assetDependenciesItr = mpOwningAssetManager->getDependedOnAssets()->find(mpAssetDefinition->mAssetId);

   // Does the asset have any dependencies?
   if (assetDependenciesItr != mpOwningAssetManager->getDependedOnAssets()->end())
   {
      // Iterate all dependencies.
      while (assetDependenciesItr != mpOwningAssetManager->getDependedOnAssets()->end() && assetDependenciesItr->key == mpAssetDefinition->mAssetId)
      {
         StringTableEntry assetType = mpOwningAssetManager->getAssetType(assetDependenciesItr->value);

         if (assetType == StringTable->insert("MaterialAsset"))
         {
            mMaterialAssetIds.push_back(assetDependenciesItr->value);

            //Force the asset to become initialized if it hasn't been already
            AssetPtr<MaterialAsset> matAsset = assetDependenciesItr->value;

            mMaterialAssets.push_back(matAsset);
         }
         else if (assetType == StringTable->insert("ShapeAnimationAsset"))
         {
            mAnimationAssetIds.push_back(assetDependenciesItr->value);

            //Force the asset to become initialized if it hasn't been already
            AssetPtr<ShapeAnimationAsset> animAsset = assetDependenciesItr->value;

            mAnimationAssets.push_back(animAsset);
         }

         // Next dependency.
         assetDependenciesItr++;
      }
   }

   mShape = ResourceManager::get().load(mFileName);

   if (!mShape)
   {
      Con::errorf("StaticMesh::updateShape : failed to load shape file!");
      return false; //if it failed to load, bail out
   }

   bool hasBlends = false;

   //Now that we've successfully loaded our shape and have any materials and animations loaded
   //we need to set up the animations we're using on our shape
   for (S32 i = mAnimationAssets.size()-1; i >= 0; --i)
   {
      String srcName = mAnimationAssets[i]->getAnimationName();
      String srcPath(mAnimationAssets[i]->getAnimationFilename());
      //SplitSequencePathAndName(srcPath, srcName);

      if (!mShape->addSequence(srcPath, srcName, srcName,
         mAnimationAssets[i]->getStartFrame(), mAnimationAssets[i]->getEndFrame(), mAnimationAssets[i]->getPadRotation(), mAnimationAssets[i]->getPadTransforms()))
         return false;

      if (mAnimationAssets[i]->isBlend())
         hasBlends = true;
   }

   //if any of our animations are blends, set those up now
   if (hasBlends)
   {
      for (U32 i=0; i < mAnimationAssets.size(); ++i)
      {
         if (mAnimationAssets[i]->isBlend() && mAnimationAssets[i]->getBlendAnimationName() != StringTable->EmptyString())
         {
            //gotta do a bit of logic here.
            //First, we need to make sure the anim asset we depend on for our blend is loaded
            AssetPtr<ShapeAnimationAsset> blendAnimAsset = mAnimationAssets[i]->getBlendAnimationName();

            if (blendAnimAsset.isNull())
            {
               Con::errorf("ShapeAsset::initializeAsset - Unable to acquire reference animation asset %s for asset %s to blend!", mAnimationAssets[i]->getBlendAnimationName(), mAnimationAssets[i]->getAssetName());
               return false;
            }

            String refAnimName = blendAnimAsset->getAnimationName();
            if (!mShape->setSequenceBlend(mAnimationAssets[i]->getAnimationName(), true, blendAnimAsset->getAnimationName(), mAnimationAssets[i]->getBlendFrame()))
            {
               Con::errorf("ShapeAnimationAsset::initializeAsset - Unable to set animation clip %s for asset %s to blend!", mAnimationAssets[i]->getAnimationName(), mAnimationAssets[i]->getAssetName());
               return false;
            }
         }
      }
   }

   return true;
}

//------------------------------------------------------------------------------

void ShapeAsset::copyTo(SimObject* object)
{
   // Call to parent.
   Parent::copyTo(object);
}

void ShapeAsset::onAssetRefresh(void)
{
   if (dStrcmp(mFileName, "") == 0)
      return;

   loadShape();
}

void ShapeAsset::SplitSequencePathAndName(String& srcPath, String& srcName)
{
   srcName = "";

   // Determine if there is a sequence name at the end of the source string, and
   // if so, split the filename from the sequence name
   S32 split = srcPath.find(' ', 0, String::Right);
   S32 split2 = srcPath.find('\t', 0, String::Right);
   if ((split == String::NPos) || (split2 > split))
      split = split2;
   if (split != String::NPos)
   {
      split2 = split + 1;
      while ((srcPath[split2] != '\0') && dIsspace(srcPath[split2]))
         split2++;

      // now 'split' is at the end of the path, and 'split2' is at the start of the sequence name
      srcName = srcPath.substr(split2);
      srcPath = srcPath.erase(split, srcPath.length() - split);
   }
}

ShapeAnimationAsset* ShapeAsset::getAnimation(S32 index)
{
   if (index < mAnimationAssets.size())
   {
      return mAnimationAssets[index];
   }

   return nullptr;
}

DefineEngineMethod(ShapeAsset, getMaterialCount, S32, (), ,
   "Gets the number of materials for this shape asset.\n"
   "@return Material count.\n")
{
   return object->getMaterialCount();
}

DefineEngineMethod(ShapeAsset, getAnimationCount, S32, (), ,
   "Gets the number of animations for this shape asset.\n"
   "@return Animation count.\n")
{
   return object->getAnimationCount();
}

DefineEngineMethod(ShapeAsset, getAnimation, ShapeAnimationAsset*, (S32 index), (0),
   "Gets a particular shape animation asset for this shape.\n"
   "@param animation asset index.\n"
   "@return Shape Animation Asset.\n")
{
   return object->getAnimation(index);
}
//-----------------------------------------------------------------------------
// GuiInspectorTypeAssetId
//-----------------------------------------------------------------------------

IMPLEMENT_CONOBJECT(GuiInspectorTypeShapeAssetPtr);

ConsoleDocClass(GuiInspectorTypeShapeAssetPtr,
   "@brief Inspector field type for Shapes\n\n"
   "Editor use only.\n\n"
   "@internal"
   );

void GuiInspectorTypeShapeAssetPtr::consoleInit()
{
   Parent::consoleInit();

   ConsoleBaseType::getType(TypeShapeAssetPtr)->setInspectorFieldType("GuiInspectorTypeShapeAssetPtr");
}

GuiControl* GuiInspectorTypeShapeAssetPtr::constructEditControl()
{
   // Create base filename edit controls
   GuiControl *retCtrl = Parent::constructEditControl();
   if (retCtrl == NULL)
      return retCtrl;

   // Change filespec
   char szBuffer[512];
   dSprintf(szBuffer, sizeof(szBuffer), "AssetBrowser.showDialog(\"ShapeAsset\", \"AssetBrowser.changeAsset\", %d, %s);", 
      mInspector->getComponentGroupTargetId(), mCaption);
   mBrowseButton->setField("Command", szBuffer);

   setDataField(StringTable->insert("ComponentOwner"), NULL, String::ToString(mInspector->getComponentGroupTargetId()).c_str());

   // Create "Open in ShapeEditor" button
   mShapeEdButton = new GuiBitmapButtonCtrl();

   dSprintf(szBuffer, sizeof(szBuffer), "ShapeEditorPlugin.openShapeAsset(%d.getText());", retCtrl->getId());
   mShapeEdButton->setField("Command", szBuffer);

   char bitmapName[512] = "tools/worldEditor/images/toolbar/shape-editor";
   mShapeEdButton->setBitmap(bitmapName);

   mShapeEdButton->setDataField(StringTable->insert("Profile"), NULL, "GuiButtonProfile");
   mShapeEdButton->setDataField(StringTable->insert("tooltipprofile"), NULL, "GuiToolTipProfile");
   mShapeEdButton->setDataField(StringTable->insert("hovertime"), NULL, "1000");
   mShapeEdButton->setDataField(StringTable->insert("tooltip"), NULL, "Open this file in the Shape Editor");

   mShapeEdButton->registerObject();
   addObject(mShapeEdButton);

   return retCtrl;
}

bool GuiInspectorTypeShapeAssetPtr::updateRects()
{
   S32 dividerPos, dividerMargin;
   mInspector->getDivider(dividerPos, dividerMargin);
   Point2I fieldExtent = getExtent();
   Point2I fieldPos = getPosition();

   mCaptionRect.set(0, 0, fieldExtent.x - dividerPos - dividerMargin, fieldExtent.y);
   mEditCtrlRect.set(fieldExtent.x - dividerPos + dividerMargin, 1, dividerPos - dividerMargin - 34, fieldExtent.y);

   bool resized = mEdit->resize(mEditCtrlRect.point, mEditCtrlRect.extent);
   if (mBrowseButton != NULL)
   {
      mBrowseRect.set(fieldExtent.x - 32, 2, 14, fieldExtent.y - 4);
      resized |= mBrowseButton->resize(mBrowseRect.point, mBrowseRect.extent);
   }

   if (mShapeEdButton != NULL)
   {
      RectI shapeEdRect(fieldExtent.x - 16, 2, 14, fieldExtent.y - 4);
      resized |= mShapeEdButton->resize(shapeEdRect.point, shapeEdRect.extent);
   }

   return resized;
}