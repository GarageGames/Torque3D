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

#ifndef _BARRELDISTORTIONPOSTEFFECT_H_
#define _BARRELDISTORTIONPOSTEFFECT_H_

#include "postFx/postEffect.h"

class BarrelDistortionPostEffect : public PostEffect
{
   typedef PostEffect Parent;

protected:
   GFXShaderConstHandle *mHmdWarpParamSC;
   GFXShaderConstHandle *mScaleSC;
   GFXShaderConstHandle *mScaleInSC;
   GFXShaderConstHandle *mLensCenterSC;
   GFXShaderConstHandle *mScreenCenterSC;

   // Oculus VR HMD index to reference
   S32 mHMDIndex;

   // Oculus VR sensor index to reference
   S32 mSensorIndex;

   // Used to increase the size of the window into the world at the
   // expense of apparent resolution.
   F32 mScaleOutput;

protected:
   virtual void _setupConstants( const SceneRenderState *state );

public:
   BarrelDistortionPostEffect();
   virtual ~BarrelDistortionPostEffect();

   DECLARE_CONOBJECT(BarrelDistortionPostEffect);

   // SimObject
   virtual bool onAdd();
   virtual void onRemove();
   static void initPersistFields();

   virtual void process(   const SceneRenderState *state, 
                           GFXTexHandle &inOutTex,
                           const RectI *inTexViewport = NULL );
};

#endif   // _BARRELDISTORTIONPOSTEFFECT_H_
