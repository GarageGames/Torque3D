//-----------------------------------------------------------------------------
// Copyright (c) 2012 GarageGames, LLC
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

#include "interior/interiorLMManager.h"
#include "gfx/gfxTextureManager.h"
#include "gfx/bitmap/gBitmap.h"
#include "interior/interiorRes.h"
#include "interior/interiorInstance.h"
#include "interior/interior.h"

//------------------------------------------------------------------------------
// Globals
InteriorLMManager   gInteriorLMManager;

//------------------------------------------------------------------------------

InteriorLMManager::InteriorLMManager()
{
   VECTOR_SET_ASSOCIATION( mInteriors );
}

InteriorLMManager::~InteriorLMManager()
{
   for(U32 i = 0; i < mInteriors.size(); i++)
      removeInterior(LM_HANDLE(i));
}

//------------------------------------------------------------------------------
void InteriorLMManager::addInterior(LM_HANDLE & interiorHandle, U32 numLightmaps, Interior * interior)
{
   interiorHandle = mInteriors.size();
   mInteriors.increment();
   mInteriors.last() = new InteriorLMInfo;

   mInteriors.last()->mInterior = interior;
   mInteriors.last()->mHandlePtr = &interiorHandle;
   mInteriors.last()->mNumLightmaps = numLightmaps;

   // create base instance
   addInstance(interiorHandle, mInteriors.last()->mBaseInstanceHandle, 0);
   AssertFatal(mInteriors.last()->mBaseInstanceHandle == LM_HANDLE(0), "InteriorLMManager::addInterior: invalid base instance handle");

   // steal the lightmaps from the interior
   Vector<GFXTexHandle>& texHandles = getHandles(interiorHandle, 0);
   for(U32 i = 0; i < interior->mLightmaps.size(); i++)
   {
      AssertFatal(interior->mLightmaps[i], "InteriorLMManager::addInterior: interior missing lightmap");
      texHandles[i].set(interior->mLightmaps[i], &GFXDefaultPersistentProfile, true, String("Interior Lightmap"));
   }

   interior->mLightmaps.clear();
}

void InteriorLMManager::removeInterior(LM_HANDLE interiorHandle)
{
   AssertFatal(interiorHandle < mInteriors.size(), "InteriorLMManager::removeInterior: invalid interior handle");
   AssertFatal(mInteriors[interiorHandle]->mInstances.size() == 1, "InteriorLMManager::removeInterior: cannot remove base interior");

   // remove base instance
   removeInstance(interiorHandle, 0);

   *mInteriors[interiorHandle]->mHandlePtr = LM_HANDLE(-1);

   delete mInteriors[interiorHandle];

   // last one? otherwise move it
   if((mInteriors.size()-1) != interiorHandle)
   {
      mInteriors[interiorHandle] = mInteriors.last();
      *(mInteriors[interiorHandle]->mHandlePtr) = interiorHandle;
   }

   mInteriors.decrement();
}

//------------------------------------------------------------------------------
void InteriorLMManager::addInstance(LM_HANDLE interiorHandle, LM_HANDLE & instanceHandle, InteriorInstance * instance)
{
   AssertFatal(interiorHandle < mInteriors.size(), "InteriorLMManager::addInstance: invalid interior handle");
   AssertFatal(interiorHandle == *(mInteriors[interiorHandle]->mHandlePtr), "InteriorLMManager::addInstance: invalid handle value");

   InteriorLMInfo * interiorInfo = mInteriors[interiorHandle];

   // create the instance info and fill
   InstanceLMInfo * instanceInfo = new InstanceLMInfo;

   instanceInfo->mInstance = instance;
   instanceInfo->mHandlePtr = &instanceHandle;
   instanceHandle = interiorInfo->mInstances.size();

   interiorInfo->mInstances.push_back(instanceInfo);

   // create/clear list
   instanceInfo->mLightmapHandles.setSize(interiorInfo->mNumLightmaps);

   for(U32 i = 0; i < instanceInfo->mLightmapHandles.size(); i++)
      instanceInfo->mLightmapHandles[i] = NULL;
}

void InteriorLMManager::removeInstance(LM_HANDLE interiorHandle, LM_HANDLE instanceHandle)
{
   AssertFatal(interiorHandle < mInteriors.size(), "InteriorLMManager::removeInstance: invalid interior handle");
   AssertFatal(instanceHandle < mInteriors[interiorHandle]->mInstances.size(), "InteriorLMManager::removeInstance: invalid instance handle");
   AssertFatal(!(instanceHandle == mInteriors[interiorHandle]->mBaseInstanceHandle &&
      mInteriors[interiorHandle]->mInstances.size() > 1), "InteriorLMManager::removeInstance: invalid base instance");

   InteriorLMInfo * itrInfo = mInteriors[interiorHandle];

   // kill it
   InstanceLMInfo * instInfo = itrInfo->mInstances[instanceHandle];
   for(U32 i = 0; i < instInfo->mLightmapHandles.size(); i++)
      instInfo->mLightmapHandles[i] = NULL;

   // reset on last instance removal only (multi detailed shapes share the same instance handle)
   if(itrInfo->mInstances.size() == 1)
      *instInfo->mHandlePtr = LM_HANDLE(-1);

   delete instInfo;

   // last one? otherwise move it
   if((itrInfo->mInstances.size()-1) != instanceHandle)
   {
      itrInfo->mInstances[instanceHandle] = itrInfo->mInstances.last();
      *(itrInfo->mInstances[instanceHandle]->mHandlePtr) = instanceHandle;
   }

   itrInfo->mInstances.decrement();
}

void InteriorLMManager::useBaseTextures(LM_HANDLE interiorHandle, LM_HANDLE instanceHandle)
{
   AssertFatal(interiorHandle < mInteriors.size(), "InteriorLMManager::useBaseTextures: invalid interior handle");
   AssertFatal(interiorHandle == *(mInteriors[interiorHandle]->mHandlePtr), "InteriorLMManager::useBaseTextures: invalid handle value");

   // Make sure the base light maps are loaded
   loadBaseLightmaps(interiorHandle,instanceHandle);

   // Install base lightmaps for this instance...
   Vector<GFXTexHandle>& baseHandles = getHandles(interiorHandle, 0);
   Vector<GFXTexHandle>& texHandles = getHandles(interiorHandle, instanceHandle);
   for(U32 i = 0; i < baseHandles.size(); i++)
      texHandles[i] = baseHandles[i];
}

//------------------------------------------------------------------------------
void InteriorLMManager::destroyBitmaps()
{
   for(S32 i = mInteriors.size() - 1; i >= 0; i--)
   {
      InteriorLMInfo * interiorInfo = mInteriors[i];
      for(S32 j = interiorInfo->mInstances.size() - 1; j >= 0; j--)
      {
         InstanceLMInfo * instanceInfo = interiorInfo->mInstances[j];
         for(S32 k = instanceInfo->mLightmapHandles.size() - 1; k >= 0; k--)
         {
            if(!instanceInfo->mLightmapHandles[k])
               continue;

            GFXTextureObject * texObj = instanceInfo->mLightmapHandles[k];
            if(!texObj || !texObj->mBitmap)
               continue;

            // don't remove 'keep' bitmaps
            if(!interiorInfo->mInterior->mLightmapKeep[k])
            {
//               SAFE_DELETE(texObj->mBitmap);
//               texObj->bitmap = 0;
            }
         }
      }
   }
}

void InteriorLMManager::destroyTextures()
{
   for(S32 i = mInteriors.size() - 1; i >= 0; i--)
   {
      InteriorLMInfo * interiorInfo = mInteriors[i];
      for(S32 j = interiorInfo->mInstances.size() - 1; j >= 0; j--)
      {
         InstanceLMInfo * instanceInfo = interiorInfo->mInstances[j];
         for(S32 k = interiorInfo->mNumLightmaps - 1; k >= 0; k--)
         {
				// will want to remove the vector here eventually... so dont clear
            instanceInfo->mLightmapHandles[k] = NULL;
         }
      }
   }
}

void InteriorLMManager::downloadGLTextures()
{
   for(S32 i = mInteriors.size() - 1; i >= 0; i--)
      downloadGLTextures(i);
}

void InteriorLMManager::downloadGLTextures(LM_HANDLE interiorHandle)
{

   AssertFatal(interiorHandle < mInteriors.size(), "InteriorLMManager::downloadGLTextures: invalid interior handle");
   InteriorLMInfo * interiorInfo = mInteriors[interiorHandle];

   // The bit vector is used to keep track of which lightmap sets need
   // to be loaded from the shared "base" instance.  Every instance
   // can have it's own lightmap set due to mission lighting.
   BitVector needTexture;
   needTexture.setSize(interiorInfo->mNumLightmaps);
   needTexture.clear();

   for(S32 j = interiorInfo->mInstances.size() - 1; j >= 0; j--)
   {
      InstanceLMInfo * instanceInfo = interiorInfo->mInstances[j];
      for(S32 k = instanceInfo->mLightmapHandles.size() - 1; k >= 0; k--)
      {
         // All instances can share the base instances static lightmaps.
         // Test here to see if we need to load those lightmaps.
         if ((j == 0) && !needTexture.test(k))
            continue;
         if (!instanceInfo->mLightmapHandles[k])  
         {
            needTexture.set(k);
            continue;
         }
         GFXTexHandle texObj = instanceInfo->mLightmapHandles[k];
         if (!texObj || !texObj->mBitmap) 
         {
            needTexture.set(k);
            continue;
         }

         instanceInfo->mLightmapHandles[k].set( texObj->mBitmap, &GFXDefaultPersistentProfile, false, String("Interior Lightmap Handle") );
      }
   }
}

bool InteriorLMManager::loadBaseLightmaps(LM_HANDLE interiorHandle, LM_HANDLE instanceHandle)
{
   AssertFatal(interiorHandle < mInteriors.size(), "InteriorLMManager::loadBaseLightmaps: invalid interior handle");
   AssertFatal(instanceHandle < mInteriors[interiorHandle]->mInstances.size(), "InteriorLMManager::loadBaseLightmaps: invalid instance handle");

   // must use a valid instance handle
   if(!instanceHandle)
      return(false);

   InteriorLMInfo * interiorInfo = mInteriors[interiorHandle];
   if(!interiorInfo->mNumLightmaps)
      return(false);

   InstanceLMInfo * baseInstanceInfo = interiorInfo->mInstances[0];

   // already loaded? (if any bitmap is present, then assumed that all will be)
   GFXTexHandle texture (baseInstanceInfo->mLightmapHandles[0]);
   if(texture.isValid() && texture.getBitmap())
      return(true);

   InstanceLMInfo * instanceInfo = interiorInfo->mInstances[instanceHandle];

   Resource<InteriorResource> & interiorRes = instanceInfo->mInstance->getResource();
   if(!bool(interiorRes))
      return(false);

   GBitmap *** pBitmaps = 0;
   if(!instanceInfo->mInstance->readLightmaps(&pBitmaps))
      return(false);

   for(U32 i = 0; i < interiorRes->getNumDetailLevels(); i++)
   {
      Interior * interior = interiorRes->getDetailLevel(i);
      AssertFatal(interior, "InteriorLMManager::loadBaseLightmaps: invalid detail level in resource");
      AssertFatal(interior->getLMHandle() != LM_HANDLE(-1), "InteriorLMManager::loadBaseLightmaps: interior not added to manager");
      AssertFatal(interior->getLMHandle() < mInteriors.size(), "InteriorLMManager::loadBaseLightmaps: invalid interior");

      InteriorLMInfo * interiorInfo = mInteriors[interior->getLMHandle()];
      InstanceLMInfo * baseInstanceInfo = interiorInfo->mInstances[0];

      for(U32 j = 0; j < interiorInfo->mNumLightmaps; j++)
      {
         AssertFatal(pBitmaps[i][j], "InteriorLMManager::loadBaseLightmaps: invalid bitmap");

         if (baseInstanceInfo->mLightmapHandles[j])
			{
				GFXTextureObject * texObj = baseInstanceInfo->mLightmapHandles[j];
				texObj->mBitmap = pBitmaps[i][j];
			}
			else
         	baseInstanceInfo->mLightmapHandles[j].set( pBitmaps[i][j], &GFXDefaultPersistentProfile, false, String("Interior Lightmap Handle") );
      }
   }

   delete [] pBitmaps;
   return(true);
}

//------------------------------------------------------------------------------
GFXTexHandle &InteriorLMManager::getHandle(LM_HANDLE interiorHandle, LM_HANDLE instanceHandle, U32 index)
{
   AssertFatal(interiorHandle < mInteriors.size(), "InteriorLMManager::getHandle: invalid interior handle");
   AssertFatal(instanceHandle < mInteriors[interiorHandle]->mInstances.size(), "InteriorLMManager::getHandle: invalid instance handle");
   AssertFatal(index < mInteriors[interiorHandle]->mNumLightmaps, "InteriorLMManager::getHandle: invalid texture index");

   // valid? if not, then get base lightmap handle
   if(!mInteriors[interiorHandle]->mInstances[instanceHandle]->mLightmapHandles[index])
   {
      AssertFatal(mInteriors[interiorHandle]->mInstances[0]->mLightmapHandles[index], "InteriorLMManager::getHandle: invalid base texture handle");
      return(mInteriors[interiorHandle]->mInstances[0]->mLightmapHandles[index]);
   }
   return(mInteriors[interiorHandle]->mInstances[instanceHandle]->mLightmapHandles[index]);
}

GBitmap * InteriorLMManager::getBitmap(LM_HANDLE interiorHandle, LM_HANDLE instanceHandle, U32 index)
{
   AssertFatal(interiorHandle < mInteriors.size(), "InteriorLMManager::getBitmap: invalid interior handle");
   AssertFatal(instanceHandle < mInteriors[interiorHandle]->mInstances.size(), "InteriorLMManager::getBitmap: invalid instance handle");
   AssertFatal(index < mInteriors[interiorHandle]->mNumLightmaps, "InteriorLMManager::getBitmap: invalid texture index");

   if(!mInteriors[interiorHandle]->mInstances[instanceHandle]->mLightmapHandles[index])
   {
      AssertFatal(mInteriors[interiorHandle]->mInstances[0]->mLightmapHandles[index], "InteriorLMManager::getBitmap: invalid base texture handle");
      return(mInteriors[interiorHandle]->mInstances[0]->mLightmapHandles[index]->getBitmap());
   }

   return(mInteriors[interiorHandle]->mInstances[instanceHandle]->mLightmapHandles[index]->getBitmap());
}

Vector<GFXTexHandle> & InteriorLMManager::getHandles(LM_HANDLE interiorHandle, LM_HANDLE instanceHandle)
{
   AssertFatal(interiorHandle < mInteriors.size(), "InteriorLMManager::getHandles: invalid interior handle");
   AssertFatal(instanceHandle < mInteriors[interiorHandle]->mInstances.size(), "InteriorLMManager::getHandles: invalid instance handle");
   return(mInteriors[interiorHandle]->mInstances[instanceHandle]->mLightmapHandles);
}

//------------------------------------------------------------------------------

void InteriorLMManager::clearLightmaps(LM_HANDLE interiorHandle, LM_HANDLE instanceHandle)
{
   AssertFatal(interiorHandle < mInteriors.size(), "InteriorLMManager::clearLightmaps: invalid interior handle");
   AssertFatal(instanceHandle < mInteriors[interiorHandle]->mInstances.size(), "InteriorLMManager::clearLightmaps: invalid instance handle");

   for(U32 i = 0; i < mInteriors[interiorHandle]->mNumLightmaps; i++)
      mInteriors[interiorHandle]->mInstances[instanceHandle]->mLightmapHandles[i] = 0;
}

//------------------------------------------------------------------------------
GFXTexHandle &InteriorLMManager::duplicateBaseLightmap(LM_HANDLE interiorHandle, LM_HANDLE instanceHandle, U32 index)
{
   AssertFatal(interiorHandle < mInteriors.size(), "InteriorLMManager::duplicateBaseLightmap: invalid interior handle");
   AssertFatal(instanceHandle < mInteriors[interiorHandle]->mInstances.size(), "InteriorLMManager::duplicateBaseLightmap: invalid instance handle");
   AssertFatal(index < mInteriors[interiorHandle]->mNumLightmaps, "InteriorLMManager::duplicateBaseLightmap: invalid texture index");

   // already exists?
   GFXTexHandle texHandle = mInteriors[interiorHandle]->mInstances[instanceHandle]->mLightmapHandles[index];
   if(texHandle && texHandle->getBitmap() )
      return mInteriors[interiorHandle]->mInstances[instanceHandle]->mLightmapHandles[index];

   AssertFatal(mInteriors[interiorHandle]->mInstances[0]->mLightmapHandles[index], "InteriorLMManager::duplicateBaseLightmap: invalid base handle");

   // copy it
   GBitmap * src = mInteriors[interiorHandle]->mInstances[0]->mLightmapHandles[index]->getBitmap();
   GBitmap * dest = new GBitmap(*src);

   // don't want this texture to be downloaded yet (SceneLighting will take care of that)
   GFXTexHandle tHandle( dest, &GFXDefaultPersistentProfile, true, String("Interior Lightmap Handle 2") );
   mInteriors[interiorHandle]->mInstances[instanceHandle]->mLightmapHandles[index] = tHandle;
   return mInteriors[interiorHandle]->mInstances[instanceHandle]->mLightmapHandles[index];
}
