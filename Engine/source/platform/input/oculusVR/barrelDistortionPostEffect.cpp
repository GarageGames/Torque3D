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

#include "platform/input/oculusVR/barrelDistortionPostEffect.h"

#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "gfx/gfxDevice.h"
#include "platform/input/oculusVR/oculusVRDevice.h"

extern bool gEditingMission;

ConsoleDocClass( BarrelDistortionPostEffect, 
   "@brief A fullscreen shader effect used with the Oculus Rift.\n\n"

   "@section PFXTextureIdentifiers\n\n"   

   "@ingroup Rendering\n"
);

IMPLEMENT_CONOBJECT(BarrelDistortionPostEffect);

BarrelDistortionPostEffect::BarrelDistortionPostEffect() 
   :  PostEffect(),
      mHmdWarpParamSC(NULL),
      mScaleSC(NULL),
      mScaleInSC(NULL),
      mLensCenterSC(NULL),
      mScreenCenterSC(NULL)
{
   mHMDIndex = 0;
   mSensorIndex = 0;
   mScaleOutput = 1.0f;
}

BarrelDistortionPostEffect::~BarrelDistortionPostEffect()
{
}

void BarrelDistortionPostEffect::initPersistFields()
{
   addField( "hmdIndex", TypeS32, Offset( mHMDIndex, BarrelDistortionPostEffect ), 
      "Oculus VR HMD index to reference." );

   addField( "sensorIndex", TypeS32, Offset( mSensorIndex, BarrelDistortionPostEffect ), 
      "Oculus VR sensor index to reference." );

   addField( "scaleOutput", TypeF32, Offset( mScaleOutput, BarrelDistortionPostEffect ), 
      "Used to increase the size of the window into the world at the expense of apparent resolution." );

   Parent::initPersistFields();
}

bool BarrelDistortionPostEffect::onAdd()
{
   if( !Parent::onAdd() )
      return false;

   return true;
}

void BarrelDistortionPostEffect::onRemove()
{
   Parent::onRemove();
}

void BarrelDistortionPostEffect::_setupConstants( const SceneRenderState *state )
{
   Parent::_setupConstants(state);

   // Define the shader constants
   if(!mHmdWarpParamSC)
      mHmdWarpParamSC = mShader->getShaderConstHandle( "$HmdWarpParam" );

   if(!mScaleSC)
      mScaleSC = mShader->getShaderConstHandle( "$Scale" );

   if(!mScaleInSC)
      mScaleInSC = mShader->getShaderConstHandle( "$ScaleIn" );

   if(!mLensCenterSC)
      mLensCenterSC = mShader->getShaderConstHandle( "$LensCenter" );

   if(!mScreenCenterSC)
      mScreenCenterSC = mShader->getShaderConstHandle( "$ScreenCenter" );

   const Point2I &resolution = GFX->getActiveRenderTarget()->getSize();
   F32 widthScale = 0.5f;
   F32 heightScale = 1.0f;
   F32 aspectRatio = (resolution.x * 0.5f) / resolution.y;

   // Set up the HMD dependant shader constants
   if(ManagedSingleton<OculusVRDevice>::instanceOrNull() && OCULUSVRDEV->getHMDDevice(mHMDIndex))
   {
      const OculusVRHMDDevice* hmd = OCULUSVRDEV->getHMDDevice(mHMDIndex);

      if(mHmdWarpParamSC->isValid())
      {
         const Point4F& distortion = hmd->getKDistortion();
         mShaderConsts->set( mHmdWarpParamSC, distortion );
      }

      if(mScaleSC->isValid())
      {
         F32 scaleFactor = hmd->getDistortionScale();
         if(!mIsZero(mScaleOutput))
         {
            scaleFactor /= mScaleOutput;
         }
         Point2F scale;
         scale.x = widthScale * 0.5f * scaleFactor;
         scale.y = heightScale * 0.5f * scaleFactor * aspectRatio;
         mShaderConsts->set( mScaleSC, scale );
      }

      if(mLensCenterSC->isValid())
      {
         F32 xCenterOffset = hmd->getCenterOffset();
         Point3F lensCenter;
         lensCenter.x = (widthScale + xCenterOffset * 0.5f) * 0.5f;
         lensCenter.y = (widthScale - xCenterOffset * 0.5f) * 0.5f;
         lensCenter.z = heightScale * 0.5f;
         mShaderConsts->set( mLensCenterSC, lensCenter );
      }
   }
   else
   {
      if(mHmdWarpParamSC->isValid())
      {
         mShaderConsts->set( mHmdWarpParamSC, Point4F(0.0f, 0.0f, 0.0f, 0.0f) );
      }

      if(mScaleSC->isValid())
      {
         mShaderConsts->set( mScaleSC, Point2F(1.0f, 1.0f) );
      }

      if(mLensCenterSC->isValid())
      {
         Point3F lensCenter;
         lensCenter.x = widthScale * 0.5f;
         lensCenter.y = widthScale * 0.5f;
         lensCenter.z = heightScale * 0.5f;
         mShaderConsts->set( mLensCenterSC, lensCenter );
      }
   }

   if(mScaleInSC->isValid())
   {
      Point2F scaleIn;
      scaleIn.x = 2.0f / widthScale;
      scaleIn.y = 2.0f / heightScale / aspectRatio;
      mShaderConsts->set( mScaleInSC, scaleIn );
   }

   if(mScreenCenterSC->isValid())
   {
      mShaderConsts->set( mScreenCenterSC, Point2F(widthScale * 0.5f, heightScale * 0.5f) );
   }
}

void BarrelDistortionPostEffect::process(const SceneRenderState *state, GFXTexHandle &inOutTex, const RectI *inTexViewport)
{
   // Don't draw the post effect if the editor is active
   if(gEditingMission)
      return;

   Parent::process(state, inOutTex, inTexViewport);
}
