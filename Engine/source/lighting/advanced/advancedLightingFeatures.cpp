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

#include "platform/platform.h"
#include "lighting/advanced/advancedLightingFeatures.h"

#include "shaderGen/featureMgr.h"
#include "gfx/gfxStringEnumTranslate.h"
#include "materials/materialParameters.h"
#include "materials/materialFeatureTypes.h"
#include "materials/matTextureTarget.h"
#include "gfx/gfxDevice.h"
#include "core/util/safeDelete.h"

#if defined( TORQUE_OS_WIN )
#  include "lighting/advanced/hlsl/gBufferConditionerHLSL.h"
#  include "lighting/advanced/hlsl/advancedLightingFeaturesHLSL.h"
#endif
#if defined( TORQUE_OPENGL )
#  include "lighting/advanced/glsl/gBufferConditionerGLSL.h"
#  include "lighting/advanced/glsl/advancedLightingFeaturesGLSL.h"
#endif



bool AdvancedLightingFeatures::smFeaturesRegistered = false;

void AdvancedLightingFeatures::registerFeatures( const GFXFormat &deferredTargetFormat, const GFXFormat &lightInfoTargetFormat )
{
   AssertFatal( !smFeaturesRegistered, "AdvancedLightingFeatures::registerFeatures() - Features already registered. Bad!" );

   // If we ever need this...
   TORQUE_UNUSED(lightInfoTargetFormat);

   ConditionerFeature *cond = NULL;

   if(GFX->getAdapterType() == OpenGL)
   {
#if defined( TORQUE_OPENGL ) 
      cond = new GBufferConditionerGLSL( deferredTargetFormat, GBufferConditionerGLSL::ViewSpace );
      FEATUREMGR->registerFeature(MFT_DeferredConditioner, cond);
      FEATUREMGR->registerFeature(MFT_RTLighting, new DeferredRTLightingFeatGLSL());
      FEATUREMGR->registerFeature(MFT_NormalMap, new DeferredBumpFeatGLSL());
      FEATUREMGR->registerFeature(MFT_PixSpecular, new DeferredPixelSpecularGLSL());
      FEATUREMGR->registerFeature(MFT_MinnaertShading, new DeferredMinnaertGLSL());
      FEATUREMGR->registerFeature(MFT_SubSurface, new DeferredSubSurfaceGLSL());
#endif
   }
   else
   {
#if defined( TORQUE_OS_WIN )
      cond = new GBufferConditionerHLSL( deferredTargetFormat, GBufferConditionerHLSL::ViewSpace );
      FEATUREMGR->registerFeature(MFT_DeferredConditioner, cond);
      FEATUREMGR->registerFeature(MFT_RTLighting, new DeferredRTLightingFeatHLSL());
      FEATUREMGR->registerFeature(MFT_NormalMap, new DeferredBumpFeatHLSL());
      FEATUREMGR->registerFeature(MFT_PixSpecular, new DeferredPixelSpecularHLSL());
      FEATUREMGR->registerFeature(MFT_MinnaertShading, new DeferredMinnaertHLSL());
      FEATUREMGR->registerFeature(MFT_SubSurface, new DeferredSubSurfaceHLSL());
#endif
   }

   NamedTexTarget *target = NamedTexTarget::find( "deferred" );
   if ( target )
      target->setConditioner( cond );

   smFeaturesRegistered = true;
}

void AdvancedLightingFeatures::unregisterFeatures()
{
   NamedTexTarget *target = NamedTexTarget::find( "deferred" );
   if ( target )
      target->setConditioner( NULL );

   FEATUREMGR->unregisterFeature(MFT_DeferredConditioner);
   FEATUREMGR->unregisterFeature(MFT_RTLighting);
   FEATUREMGR->unregisterFeature(MFT_NormalMap);
   FEATUREMGR->unregisterFeature(MFT_PixSpecular);
   FEATUREMGR->unregisterFeature(MFT_MinnaertShading);
   FEATUREMGR->unregisterFeature(MFT_SubSurface);

   smFeaturesRegistered = false;
}
