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
         //Con::warnf("(TypeTextureAssetPtr) - Failed to set asset Id '%d'.", pFieldValue);
         return;
      }

      // Set asset.
      pAssetPtr->setAssetId(pFieldValue);

      return;
   }

   // Warn.
   Con::warnf("(TypeTextureAssetPtr) - Cannot set multiple args to a single asset.");
}

//-----------------------------------------------------------------------------

GameObjectAsset::GameObjectAsset() :
   mpOwningAssetManager(NULL),
   mAssetInitialized(false),
   mAcquireReferenceCount(0)
{
   // Generate an asset definition.
   mpAssetDefinition = new AssetDefinition();

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