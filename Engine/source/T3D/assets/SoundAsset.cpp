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

#ifndef SOUND_ASSET_H
#include "SoundAsset.h"
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

IMPLEMENT_CONOBJECT(SoundAsset);

ConsoleType(SoundAssetPtr, TypeSoundAssetPtr, SoundAsset, ASSET_ID_FIELD_PREFIX)

//-----------------------------------------------------------------------------

ConsoleGetType(TypeSoundAssetPtr)
{
   // Fetch asset Id.
   return (*((AssetPtr<SoundAsset>*)dptr)).getAssetId();
}

//-----------------------------------------------------------------------------

ConsoleSetType(TypeSoundAssetPtr)
{
   // Was a single argument specified?
   if (argc == 1)
   {
      // Yes, so fetch field value.
      const char* pFieldValue = argv[0];

      // Fetch asset pointer.
      AssetPtr<SoundAsset>* pAssetPtr = dynamic_cast<AssetPtr<SoundAsset>*>((AssetPtrBase*)(dptr));

      // Is the asset pointer the correct type?
      if (pAssetPtr == NULL)
      {
         // No, so fail.
         //Con::warnf("(TypeSoundAssetPtr) - Failed to set asset Id '%d'.", pFieldValue);
         return;
      }

      // Set asset.
      pAssetPtr->setAssetId(pFieldValue);

      return;
   }

   // Warn.
   Con::warnf("(TypeSoundAssetPtr) - Cannot set multiple args to a single asset.");
}

//-----------------------------------------------------------------------------

SoundAsset::SoundAsset()
{
   mSoundFilePath = StringTable->EmptyString();

   mPitchAdjust = 0;
   mVolumeAdjust = 0;

   //mSound = nullptr;
}

//-----------------------------------------------------------------------------

SoundAsset::~SoundAsset()
{
   // If the asset manager does not own the asset then we own the
   // asset definition so delete it.
   if (!getOwned())
      delete mpAssetDefinition;
}

//-----------------------------------------------------------------------------

void SoundAsset::initPersistFields()
{
   // Call parent.
   Parent::initPersistFields();

   addField("soundFilePath", TypeFilename, Offset(mSoundFilePath, SoundAsset), "Path to the sound file.");

   addField("pitchAdjust", TypeF32, Offset(mPitchAdjust, SoundAsset), "Adjustment of the pitch value");
   addField("volumeAdjust", TypeF32, Offset(mVolumeAdjust, SoundAsset), "Adjustment to the volume.");
}

//------------------------------------------------------------------------------

void SoundAsset::copyTo(SimObject* object)
{
   // Call to parent.
   Parent::copyTo(object);
}

void SoundAsset::initializeAsset(void)
{
}

void SoundAsset::onAssetRefresh(void)
{

}