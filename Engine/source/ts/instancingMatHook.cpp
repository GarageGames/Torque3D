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
#include "ts/instancingMatHook.h"

#include "platform/profiler.h"
#include "materials/materialFeatureTypes.h"
#include "materials/matInstance.h"


const MatInstanceHookType InstancingMaterialHook::Type( "Instancing" );

InstancingMaterialHook::InstancingMaterialHook() :
   mMatInst( NULL )
{
}

InstancingMaterialHook::~InstancingMaterialHook()
{
   SAFE_DELETE( mMatInst );
}

BaseMatInstance* InstancingMaterialHook::getInstancingMat( BaseMatInstance *matInst )
{
   PROFILE_SCOPE( InstancingMaterialHook_GetInstancingMat );

   if ( matInst == NULL )
      return NULL;

   InstancingMaterialHook *hook = matInst->getHook<InstancingMaterialHook>();
   if ( hook == NULL )
   {
      hook = new InstancingMaterialHook();
      matInst->addHook( hook );

      BaseMatInstance *instMat = matInst->getMaterial()->createMatInstance();
      FeatureSet features( matInst->getRequestedFeatures() );
      features.addFeature( MFT_UseInstancing ); 

      // Allow for named texture assignments
      Material::sAllowTextureTargetAssignment = true;

      if ( !instMat->init( features, matInst->getVertexFormat() ) )
         SAFE_DELETE( instMat );

      // Turn back off named texture assignments
      Material::sAllowTextureTargetAssignment = false;

      hook->mMatInst = instMat;
   }

   return hook->mMatInst;
}
