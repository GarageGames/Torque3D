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

#include "T3D/entity.h"

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
   mGameObjectName = StringTable->EmptyString();
   mScriptFile = StringTable->EmptyString();
   mTAMLFile = StringTable->EmptyString();
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

   addProtectedField("scriptFile", TypeAssetLooseFilePath, Offset(mScriptFile, GameObjectAsset),
      &setScriptFile, &getScriptFile, "Path to the script file for the GameObject's script code.");
   addProtectedField("TAMLFile", TypeAssetLooseFilePath, Offset(mTAMLFile, GameObjectAsset),
      &setTAMLFile, &getTAMLFile, "Path to the taml file for the GameObject's heirarchy.");
}

//------------------------------------------------------------------------------

void GameObjectAsset::copyTo(SimObject* object)
{
   // Call to parent.
   Parent::copyTo(object);
}

void GameObjectAsset::initializeAsset()
{
   //Ensure we have an expanded filepath
   if (!Platform::isFullPath(mScriptFile))
      mScriptFile = getOwned() ? expandAssetFilePath(mScriptFile) : mScriptFile;

   if (Platform::isFile(mScriptFile))
      Con::executeFile(mScriptFile, false, false);

   if (!Platform::isFullPath(mTAMLFile))
      mTAMLFile = getOwned() ? expandAssetFilePath(mTAMLFile) : mTAMLFile;
}

void GameObjectAsset::onAssetRefresh()
{
   //Ensure we have an expanded filepath
   mScriptFile = expandAssetFilePath(mScriptFile);

   if (Platform::isFile(mScriptFile))
      Con::executeFile(mScriptFile, false, false);

   mTAMLFile = expandAssetFilePath(mTAMLFile);
}

void GameObjectAsset::setScriptFile(const char* pScriptFile)
{
   // Sanity!
   AssertFatal(pScriptFile != NULL, "Cannot use a NULL script file.");

   // Fetch image file.
   pScriptFile = StringTable->insert(pScriptFile);

   // Ignore no change,
   if (pScriptFile == mScriptFile)
      return;

   // Update.
   mScriptFile = getOwned() ? expandAssetFilePath(pScriptFile) : pScriptFile;

   // Refresh the asset.
   refreshAsset();
}


void GameObjectAsset::setTAMLFile(const char* pTAMLFile)
{
   // Sanity!
   AssertFatal(pTAMLFile != NULL, "Cannot use a NULL TAML file.");

   // Fetch image file.
   pTAMLFile = StringTable->insert(pTAMLFile);

   // Ignore no change,
   if (pTAMLFile == mTAMLFile)
      return;

   // Update.
   mTAMLFile = getOwned() ? expandAssetFilePath(pTAMLFile) : pTAMLFile;

   // Refresh the asset.
   refreshAsset();
}


const char* GameObjectAsset::create()
{
   if (!Platform::isFile(mTAMLFile))
      return "";

   // Set the format mode.
   Taml taml;

   // Yes, so set it.
   taml.setFormatMode(Taml::getFormatModeEnum("xml"));

   // Turn-off auto-formatting.
   taml.setAutoFormat(false);

   // Read object.
   SimObject* pSimObject = taml.read(mTAMLFile);

   // Did we find the object?
   if (pSimObject == NULL)
   {
      // No, so warn.
      Con::warnf("GameObjectAsset::create() - Could not read object from file '%s'.", mTAMLFile);
      return "";
   }

   //Flag it so we know where it came from
   pSimObject->setDataField(StringTable->insert("GameObject"), nullptr, getAssetId());

   return pSimObject->getIdString();
}

DefineEngineMethod(GameObjectAsset, createObject, const char*, (),,
   "Creates an instance of the given GameObject given the asset definition.\n"
   "@return The GameObject entity created from the asset.")
{
   return object->create();
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
   // Create "Open in ShapeEditor" button
   mGameObjectEditButton = new GuiButtonCtrl();

   // Change filespec
   char szBuffer[512];
   dSprintf(szBuffer, sizeof(szBuffer), "%d.onClick(%s);", this->getId(), mCaption);
   mGameObjectEditButton->setField("Command", szBuffer);

   mGameObjectEditButton->setDataField(StringTable->insert("Profile"), NULL, "GuiButtonProfile");
   mGameObjectEditButton->setDataField(StringTable->insert("tooltipprofile"), NULL, "GuiToolTipProfile");
   mGameObjectEditButton->setDataField(StringTable->insert("hovertime"), NULL, "1000");

   const char* assetId = getData();

   if (assetId == "")
   {
      mGameObjectEditButton->setText("Create Game Object");

      mGameObjectEditButton->setDataField(StringTable->insert("tooltip"), NULL, "Convert this object into a reusable Game Object asset.");
   }
   else
   {
      GameObjectAsset* goAsset = AssetDatabase.acquireAsset< GameObjectAsset>(assetId);

      if (goAsset)
      {
         mGameObjectEditButton->setText("Edit Game Object");

         mGameObjectEditButton->setDataField(StringTable->insert("tooltip"), NULL, "Edit this object instance or Game Object asset.");
      }
      else
      {
         mGameObjectEditButton->setText("Create Game Object");

         mGameObjectEditButton->setDataField(StringTable->insert("tooltip"), NULL, "Convert this object into a reusable Game Object asset.");
      }
   }

   //mGameObjectEditButton->registerObject();
   _registerEditControl(mGameObjectEditButton);

   addObject(mGameObjectEditButton);

   return mGameObjectEditButton;
}

bool GuiInspectorTypeGameObjectAssetPtr::updateRects()
{
   S32 dividerPos, dividerMargin;
   mInspector->getDivider(dividerPos, dividerMargin);
   Point2I fieldExtent = getExtent();
   Point2I fieldPos = getPosition();

   mCaptionRect.set(0, 0, fieldExtent.x - dividerPos - dividerMargin, fieldExtent.y);
   mEditCtrlRect.set(fieldExtent.x - dividerPos + dividerMargin, 1, dividerPos - dividerMargin, fieldExtent.y);

   bool resized = mEdit->resize(mEditCtrlRect.point, mEditCtrlRect.extent);
   if (mGameObjectEditButton != NULL)
   {
      resized |= mGameObjectEditButton->resize(mEditCtrlRect.point, mEditCtrlRect.extent);
   }

   return resized;
}
