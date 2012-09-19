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

#ifndef _INTERIORLMMANAGER_H_
#define _INTERIORLMMANAGER_H_

#ifndef _PLATFORM_H_
#include "platform/platform.h"
#endif
#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif

#include "gfx/gfxTextureHandle.h"

class GBitmap;
class Interior;
class InteriorInstance;

typedef U32 LM_HANDLE;

class InteriorLMManager
{
   private:

      struct InstanceLMInfo {
         InteriorInstance *         mInstance;
         LM_HANDLE *                mHandlePtr;
         Vector<GFXTexHandle>       mLightmapHandles;
      };

      struct InteriorLMInfo {
         Interior *                 mInterior;
         LM_HANDLE *                mHandlePtr;
         U32                        mNumLightmaps;
         LM_HANDLE                  mBaseInstanceHandle;
         Vector<InstanceLMInfo*>    mInstances;
      };

      Vector<InteriorLMInfo*>       mInteriors;

   public:

      InteriorLMManager();
      ~InteriorLMManager();

      void destroyBitmaps();
      void destroyTextures();

      void downloadGLTextures();
      void downloadGLTextures(LM_HANDLE interiorHandle);
      bool loadBaseLightmaps(LM_HANDLE interiorHandle, LM_HANDLE instanceHandle);

      void addInterior(LM_HANDLE & interiorHandle, U32 numLightmaps, Interior * interior);
      void removeInterior(LM_HANDLE interiorHandle);

      void addInstance(LM_HANDLE interiorHandle, LM_HANDLE & instanceHandle, InteriorInstance * instance);
      void removeInstance(LM_HANDLE interiorHandle, LM_HANDLE instanceHandle);
      void useBaseTextures(LM_HANDLE interiorHandle, LM_HANDLE instanceHandle);

      void clearLightmaps(LM_HANDLE interiorHandle, LM_HANDLE instanceHandle);

      GFXTexHandle     &getHandle(LM_HANDLE interiorHandle, LM_HANDLE instanceHandle, U32 index);
      Vector<GFXTexHandle> & getHandles(LM_HANDLE interiorHandle, LM_HANDLE instanceHandle);

      // helper's
      GFXTexHandle &duplicateBaseLightmap(LM_HANDLE interiorHandle, LM_HANDLE instanceHandle, U32 index);
      GBitmap * getBitmap(LM_HANDLE interiorHandle, LM_HANDLE instanceHandle, U32 index);
};

extern InteriorLMManager         gInteriorLMManager;

#endif
