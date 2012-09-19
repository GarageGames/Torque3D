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

#ifndef _SHADOWMATHOOK_H_
#define _SHADOWMATHOOK_H_

#ifndef _MATINSTANCEHOOK_H_
#include "materials/matInstanceHook.h"
#endif
#ifndef _MATINSTANCE_H_
#include "materials/matInstance.h"
#endif

// TODO: Move ShadowType enum to somewhere 
// with less dependancies.
#ifndef _SHADOWMAPPASS_H_
#include "lighting/shadowMap/shadowMapPass.h"
#endif

class ShadowMatInstance : public MatInstance
{
   typedef MatInstance Parent;

   bool mLightmappedMaterial;
public:
   ShadowMatInstance( Material *mat );
   virtual ~ShadowMatInstance() {}

   virtual bool setupPass( SceneRenderState *state, const SceneData &sgData );
};

class ShadowMaterialHook : public MatInstanceHook
{
public:

   ShadowMaterialHook();

   // MatInstanceHook
   virtual ~ShadowMaterialHook();
   virtual const MatInstanceHookType& getType() const { return Type; }

   /// The material hook type.
   static const MatInstanceHookType Type;

   BaseMatInstance* getShadowMat( ShadowType type ) const;

   void init( BaseMatInstance *mat );

protected:

   static void _overrideFeatures(   ProcessedMaterial *mat,
                                    U32 stageNum,
                                    MaterialFeatureData &fd, 
                                    const FeatureSet &features );

   /// 
   BaseMatInstance* mShadowMat[ShadowType_Count];


};

#endif // _SHADOWMATHOOK_H_
