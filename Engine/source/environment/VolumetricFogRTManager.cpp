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
   
//-----------------------------------------------------------------------------
// Volumetric Fog Rendertarget Manager
//
// Creates and maintains one set of rendertargets to be used by every
// VolumetricFog object in the scene.
//
// Will be loaded at startup end removed when ending game.
//
//-----------------------------------------------------------------------------
   
#include "VolumetricFogRTManager.h"
#include "core/module.h"
#include "scene/sceneManager.h"
#include "windowManager/platformWindowMgr.h"
#include "console/engineAPI.h"
#include "gui/core/guiCanvas.h"
#include "gfx/gfxDevice.h"
   
MODULE_BEGIN(VolumetricFogRTManager)
   
MODULE_INIT_AFTER(Scene)
MODULE_SHUTDOWN_BEFORE(Scene)
   
MODULE_INIT
{
   gVolumetricFogRTManager = new VolumetricFogRTManager;
   gClientSceneGraph->addObjectToScene(gVolumetricFogRTManager);
}
   
MODULE_SHUTDOWN
{
   gClientSceneGraph->removeObjectFromScene(gVolumetricFogRTManager);
   SAFE_DELETE(gVolumetricFogRTManager);
}
   
MODULE_END;
   
ConsoleDocClass( VolumetricFogRTManager,
"@brief Creates and maintains one set of rendertargets to be used by every\n"
"VolumetricFog object in the scene.\n\n"
"Will be loaded at startup end removed when ending game.\n\n"
"Methods:\n"
" get() returns the currently loaded VolumetricFogRTManager, also accessible\n"
" through VFRTM define.\n"
" Init() Initializes the rendertargets, called when a VolumetricFog object is\n"
" added to the scene.\n"
" isInitialed() returns true if Rendertargets are present, false if not, then\n"
" Init() should be called to create the rendertargets.\n"
" setQuality(U32 Quality) Normally a rendertarget has the same size as the view,\n"
" with this method you can scale down the size of it.\n"
" Be aware that scaling down will introduce renderartefacts.\n"
"@ingroup Atmosphere"
);
   
VolumetricFogRTMResizeSignal VolumetricFogRTManager::smVolumetricFogRTMResizeSignal;
   
VolumetricFogRTManager *gVolumetricFogRTManager = NULL;

S32 VolumetricFogRTManager::mTargetScale = 1;
   
IMPLEMENT_CONOBJECT(VolumetricFogRTManager);
   
VolumetricFogRTManager::VolumetricFogRTManager()
{
   setGlobalBounds();
   mTypeMask |= EnvironmentObjectType;
   mNetFlags.set(IsGhost);
   mIsInitialized = false;
   mNumFogObjects = 0;
}
   
VolumetricFogRTManager::~VolumetricFogRTManager()
{
   if (mFrontTarget.isRegistered())
      mFrontTarget.unregister();
   
   if (mDepthTarget.isRegistered())
      mDepthTarget.unregister();
   
   if (mDepthBuffer.isValid())
      mDepthBuffer->kill();
   
   if (mFrontBuffer.isValid())
      mFrontBuffer->kill();
}
   
void VolumetricFogRTManager::onSceneRemove()
{
   if (mIsInitialized)
      mPlatformWindow->getScreenResChangeSignal().remove(this, &VolumetricFogRTManager::ResizeRT);
}
   
void VolumetricFogRTManager::onRemove()
{
   removeFromScene();
   Parent::onRemove();
}
   
void VolumetricFogRTManager::consoleInit()
{
   Con::addVariable("$pref::VolumetricFog::Quality", TypeS32, &mTargetScale,
   "The scale of the rendertargets.\n"
   "@ingroup Rendering\n");
}
   
bool VolumetricFogRTManager::Init()
{
   if (mIsInitialized)
   {
      Con::errorf("VolumetricFogRTManager allready initialized!!");
      return true;
   }
   
   GuiCanvas* cv = dynamic_cast<GuiCanvas*>(Sim::findObject("Canvas"));
   if (cv == NULL)
   {
      Con::errorf("VolumetricFogRTManager::Init() - Canvas not found!!");
      return false;
   }
   
   mPlatformWindow = cv->getPlatformWindow();
   mPlatformWindow->getScreenResChangeSignal().notify(this,&VolumetricFogRTManager::ResizeRT);
   
   if (mTargetScale < 1 || GFX->getAdapterType() == Direct3D11)
      mTargetScale = 1;
   
   mWidth = mFloor(mPlatformWindow->getClientExtent().x / mTargetScale);
   mHeight = mFloor(mPlatformWindow->getClientExtent().y / mTargetScale);
   
   mDepthBuffer = GFXTexHandle(mWidth, mHeight, GFXFormatR32F,
   &GFXDefaultRenderTargetProfile, avar("%s() - mDepthBuffer (line %d)", __FUNCTION__, __LINE__));
   if (!mDepthBuffer.isValid())
   {
      Con::errorf("VolumetricFogRTManager Fatal Error: Unable to create Depthbuffer");
      return false;
   }
   if (!mDepthTarget.registerWithName("volfogdepth"))
   {
      Con::errorf("VolumetricFogRTManager Fatal Error : Unable to register Depthbuffer");
      return false;
   }
   mDepthTarget.setTexture(mDepthBuffer);
   
   mFrontBuffer = GFXTexHandle(mWidth, mHeight, GFXFormatR32F,
   &GFXDefaultRenderTargetProfile, avar("%s() - mFrontBuffer (line %d)", __FUNCTION__, __LINE__));
   if (!mFrontBuffer.isValid())
   {
      Con::errorf("VolumetricFogRTManager Fatal Error: Unable to create front buffer");
      return false;
   }
   if (!mFrontTarget.registerWithName("volfogfront"))
   {
      Con::errorf("VolumetricFogRTManager Fatal Error : Unable to register Frontbuffer");
      return false;
   }
   
   mFrontTarget.setTexture(mFrontBuffer);
   
   Con::setVariable("$VolumetricFog::density", "0.0");
   
   mIsInitialized = true;
   
   return true;
}
   
U32 VolumetricFogRTManager::IncFogObjects()
{
   mNumFogObjects++;
   return mNumFogObjects;
}
   
U32 VolumetricFogRTManager::DecFogObjects()
{
   if (mNumFogObjects > 0)
      mNumFogObjects--;
   return mNumFogObjects;
}
   
void VolumetricFogRTManager::ResizeRT(PlatformWindow* win,bool resize)
{
   mFogHasAnswered = 0;
   smVolumetricFogRTMResizeSignal.trigger(this, true);
}
   
void VolumetricFogRTManager::FogAnswered()
{
   mFogHasAnswered++;
   if (mFogHasAnswered == mNumFogObjects)
   {
      if (Resize())
         smVolumetricFogRTMResizeSignal.trigger(this, false);
      else
         Con::errorf("VolumetricFogRTManager::FogAnswered - Error resizing rendertargets!");
   }
}
   
bool VolumetricFogRTManager::Resize()
{
   if (mTargetScale < 1 || GFX->getAdapterType() == Direct3D11)
      mTargetScale = 1;

   mWidth = mFloor(mPlatformWindow->getClientExtent().x / mTargetScale);
   mHeight = mFloor(mPlatformWindow->getClientExtent().y / mTargetScale);
   
   if (mWidth < 16 || mHeight < 16)
      return false;
   
   if (mFrontTarget.isRegistered())
      mFrontTarget.setTexture(NULL);
   
   if (mDepthTarget.isRegistered())
      mDepthTarget.setTexture(NULL);
   
   if (mDepthBuffer.isValid())
      mDepthBuffer->kill();
   
   if (mFrontBuffer.isValid())
      mFrontBuffer->kill();
   
   mFrontBuffer = GFXTexHandle(mWidth, mHeight, GFXFormatR32F,
   &GFXDefaultRenderTargetProfile, avar("%s() - mFrontBuffer (line %d)", __FUNCTION__, __LINE__));
   if (!mFrontBuffer.isValid())
   {
      Con::errorf("VolumetricFogRTManager::Resize() Fatal Error: Unable to create front buffer");
      return false;
   }
   mFrontTarget.setTexture(mFrontBuffer);
   
   mDepthBuffer = GFXTexHandle(mWidth, mHeight, GFXFormatR32F,
   &GFXDefaultRenderTargetProfile, avar("%s() - mDepthBuffer (line %d)", __FUNCTION__, __LINE__));
   if (!mDepthBuffer.isValid())
   {
      Con::errorf("VolumetricFogRTManager::Resize() Fatal Error: Unable to create Depthbuffer");
      return false;
   }
   mDepthTarget.setTexture(mDepthBuffer);
   return true;
}
   
S32 VolumetricFogRTManager::setQuality(U32 Quality)
{
   if (!mIsInitialized)
      return (mTargetScale = Quality);
      
   if (Quality < 1)
      Quality = 1;
   
   if (Quality == mTargetScale)
      return mTargetScale;
   
   mTargetScale = Quality;
   
   mFogHasAnswered = 0;
   smVolumetricFogRTMResizeSignal.trigger(this, true);
   
   return mTargetScale;
}
   
VolumetricFogRTManager* VolumetricFogRTManager::get()
{
   return gVolumetricFogRTManager;
}
   
DefineConsoleFunction(SetFogVolumeQuality, S32, (U32 new_quality), ,
"@brief Resizes the rendertargets of the Volumetric Fog object.\n"
"@params new_quality new quality for the rendertargets 1 = full size, 2 = halfsize, 3 = 1/3, 4 = 1/4 ...")
{
   if (VFRTM == NULL)
      return -1;
   return VFRTM->setQuality(new_quality);
}