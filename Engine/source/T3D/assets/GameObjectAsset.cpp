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

#ifndef GAME_OBJECT_ASSET_H
#include "GameObjectAsset.h"
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

IMPLEMENT_CONOBJECT(GameObjectAsset);

ConsoleType(GameObjectAssetPtr, TypeGameObjectAssetPtr, GameObjectAsset, ASSET_ID_FIELD_PREFIX)

//-----------------------------------------------------------------------------

ConsoleGetType(TypeGameObjectAssetPtr)
{
   // Fetch asset Id.
   return (*((AssetPtr<GameObjectAsset>*)dptr)).getAssetId();
}

//-----------------------------------------------------------------------------

ConsoleSetType(TypeGameObjectAssetPtr)
{
   // Was a single argument specified?
   if (argc == 1)
   {
      // Yes, so fetch field value.
      const char* pFieldValue = argv[0];

      // Fetch asset pointer.
      AssetPtr<GameObjectAsset>* pAssetPtr = dynamic_cast<AssetPtr<GameObjectAsset>*>((AssetPtrBase*)(dptr));

      // Is the asset pointer the correct type?
      if (pAssetPtr == NULL)
      {
         // No, so fail.
         //Con::warnf("(TypeGameObjectAssetPtr) - Failed to set asset Id '%d'.", pFieldValue);
         return;
      }

      // Set asset.
      pAssetPtr->setAssetId(pFieldValue);

      return;
   }

   // Warn.
   Con::warnf("(TypeGameObjectAssetPtr) - Cannot set multiple args to a single asset.");
}

//-----------------------------------------------------------------------------

GameObjectAsset::GameObjectAsset()
{
   mGameObjectName = StringTable->lookup("");
   mScriptFilePath = StringTable->lookup("");
   mTAMLFilePath = StringTable->lookup("");
}

//-----------------------------------------------------------------------------

GameObjectAsset::~GameObjectAsset()
{
   // If the asset manager does not own the asset then we own the
   // asset definition so delete it.
   if (!getOwned())
      delete mpAssetDefinition;
}

//-----------------------------------------------------------------------------

void GameObjectAsset::initPersistFields()
{
   // Call parent.
   Parent::initPersistFields();

   addField("gameObjectName", TypeString, Offset(mGameObjectName, GameObjectAsset), "Name of the game object. Defines the created object's class.");
   addField("scriptFilePath", TypeString, Offset(mScriptFilePath, GameObjectAsset), "Path to the script file for the GameObject's script code.");
   addField("TAMLFilePath", TypeString, Offset(mTAMLFilePath, GameObjectAsset), "Path to the taml file for the GameObject's heirarchy.");
}

//------------------------------------------------------------------------------

void GameObjectAsset::copyTo(SimObject* object)
{
   // Call to parent.
   Parent::copyTo(object);
}

void GameObjectAsset::initializeAsset()
{
   if (Platform::isFile(mScriptFilePath))
      Con::executeFile(mScriptFilePath, false, false);
}

void GameObjectAsset::onAssetRefresh()
{
   if (Platform::isFile(mScriptFilePath))
      Con::executeFile(mScriptFilePath, false, false);
}

//-----------------------------------------------------------------------------
// GuiInspectorTypeAssetId
//-----------------------------------------------------------------------------

IMPLEMENT_CONOBJECT(GuiInspectorTypeGameObjectAssetPtr);

ConsoleDocClass(GuiInspectorTypeGameObjectAssetPtr,
   "@brief Inspector field type for Game Objects\n\n"
   "Editor use only.\n\n"
   "@internal"
);

void GuiInspectorTypeGameObjectAssetPtr::consoleInit()
{
   Parent::consoleInit();

   ConsoleBaseType::getType(TypeGameObjectAssetPtr)->setInspectorFieldType("GuiInspectorTypeGameObjectAssetPtr");
}

GuiControl* GuiInspectorTypeGameObjectAssetPtr::constructEditControl()
{
   // Create base filename edit controls
   GuiControl *retCtrl = Parent::constructEditControl();
   if (retCtrl == NULL)
      return retCtrl;

   // Change filespec
   char szBuffer[512];
   dSprintf(szBuffer, sizeof(szBuffer), "AssetBrowser.showDialog(\"GameObjectAsset\", \"AssetBrowser.changeAsset\", %d, %s);",
      mInspector->getComponentGroupTargetId(), mCaption);
   mBrowseButton->setField("Command", szBuffer);

   // Create "Open in ShapeEditor" button
   mSMEdButton = new GuiBitmapButtonCtrl();

   dSprintf(szBuffer, sizeof(szBuffer), "echo(\"Game Object Editor not implemented yet!\");", retCtrl->getId());
   mSMEdButton->setField("Command", szBuffer);

   char bitmapName[512] = "tools/worldEditor/images/toolbar/shape-editor";
   mSMEdButton->setBitmap(bitmapName);

   mSMEdButton->setDataField(StringTable->insert("Profile"), NULL, "GuiButtonProfile");
   mSMEdButton->setDataField(StringTable->insert("tooltipprofile"), NULL, "GuiToolTipProfile");
   mSMEdButton->setDataField(StringTable->insert("hovertime"), NULL, "1000");
   mSMEdButton->setDataField(StringTable->insert("tooltip"), NULL, "Open this file in the State Machine Editor");

   mSMEdButton->registerObject();
   addObject(mSMEdButton);

   return retCtrl;
}

bool GuiInspectorTypeGameObjectAssetPtr::updateRects()
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

   if (mSMEdButton != NULL)
   {
      RectI shapeEdRect(fieldExtent.x - 16, 2, 14, fieldExtent.y - 4);
      resized |= mSMEdButton->resize(shapeEdRect.point, shapeEdRect.extent);
   }

   return resized;
}