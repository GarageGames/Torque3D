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
#ifndef GAME_OBJECT_ASSET_H
#define GAME_OBJECT_ASSET_H

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
#ifndef _GUI_INSPECTOR_TYPES_H_
#include "gui/editor/guiInspectorTypes.h"
#endif

//-----------------------------------------------------------------------------
class GameObjectAsset : public AssetBase
{
   typedef AssetBase Parent;

   StringTableEntry mGameObjectName;
   StringTableEntry mScriptFile;
   StringTableEntry mTAMLFile;

public:
   GameObjectAsset();
   virtual ~GameObjectAsset();

   /// Engine.
   static void initPersistFields();
   virtual void copyTo(SimObject* object);

   const char* create();

   /// Declare Console Object.
   DECLARE_CONOBJECT(GameObjectAsset);

   void                    setScriptFile(const char* pScriptFile);
   inline StringTableEntry getScriptFile(void) const { return mScriptFile; };
   void                    setTAMLFile(const char* pScriptFile);
   inline StringTableEntry getTAMLFile(void) const { return mTAMLFile; };

protected:
   virtual void            initializeAsset(void);
   virtual void            onAssetRefresh(void);

   static bool setScriptFile(void *obj, const char *index, const char *data) { static_cast<GameObjectAsset*>(obj)->setScriptFile(data); return false; }
   static const char* getScriptFile(void* obj, const char* data) { return static_cast<GameObjectAsset*>(obj)->getScriptFile(); }
   static bool setTAMLFile(void *obj, const char *index, const char *data) { static_cast<GameObjectAsset*>(obj)->setTAMLFile(data); return false; }
   static const char* getTAMLFile(void* obj, const char* data) { return static_cast<GameObjectAsset*>(obj)->getTAMLFile(); }
};

DefineConsoleType(TypeGameObjectAssetPtr, GameObjectAsset)


//-----------------------------------------------------------------------------
// TypeAssetId GuiInspectorField Class
//-----------------------------------------------------------------------------
class GuiInspectorTypeGameObjectAssetPtr : public GuiInspectorField
{
   typedef GuiInspectorField Parent;
public:

   GuiButtonCtrl  *mGameObjectEditButton;

   DECLARE_CONOBJECT(GuiInspectorTypeGameObjectAssetPtr);
   static void consoleInit();

   virtual GuiControl* constructEditControl();
   virtual bool updateRects();
};

#endif // _ASSET_BASE_H_

