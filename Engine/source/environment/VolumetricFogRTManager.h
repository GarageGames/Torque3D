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
   
#ifndef _VolumetricFogRTManager_H_
#define _VolumetricFogRTManager_H_
   
#ifndef _SCENEOBJECT_H_
#include "scene/sceneObject.h"
#endif
#ifndef _MATTEXTURETARGET_H_
#include "materials/matTextureTarget.h"
#endif
#ifndef _GFXTARGET_H_
#include "gfx/gfxTarget.h"
#endif
#ifndef _SIGNAL_H_
#include "core/util/tSignal.h"
#endif
   
class VolumetricFogRTManager;
   
typedef Signal<void(VolumetricFogRTManager *VolumetricFogRTManager, bool resize)> VolumetricFogRTMResizeSignal;
   
#define VFRTM VolumetricFogRTManager::get()
   
class VolumetricFogRTManager : public SceneObject
{
   public:
      typedef SceneObject Parent;
   
   protected:
      GFXTexHandle mDepthBuffer;
      GFXTexHandle mFrontBuffer;
   
      NamedTexTarget mDepthTarget;
      NamedTexTarget mFrontTarget;
   
      PlatformWindow* mPlatformWindow;
   
      static S32 mTargetScale;
      bool mIsInitialized;
      U32 mNumFogObjects;
      U32 mFogHasAnswered;
      U32 mWidth;
      U32 mHeight;
   
      void onRemove();
      void onSceneRemove();
      void ResizeRT(PlatformWindow *win, bool resize);
   
      static VolumetricFogRTMResizeSignal smVolumetricFogRTMResizeSignal;

   public:
      VolumetricFogRTManager();
      ~VolumetricFogRTManager();
      static VolumetricFogRTManager *get();
      bool Init();
      bool IsInitialized() { return mIsInitialized; }
      static void consoleInit();
      static VolumetricFogRTMResizeSignal& getVolumetricFogRTMResizeSignal() { return smVolumetricFogRTMResizeSignal; }
      void FogAnswered();
      S32 setQuality(U32 Quality);
      bool Resize();
      U32 IncFogObjects();
      U32 DecFogObjects();
   
   DECLARE_CONOBJECT(VolumetricFogRTManager);
};
   
extern VolumetricFogRTManager* gVolumetricFogRTManager;
   
#endif