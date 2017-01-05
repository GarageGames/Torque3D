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
#include "scene/reflectionMatHook.h"

#include "materials/materialManager.h"
#include "materials/customMaterialDefinition.h"
#include "materials/materialFeatureTypes.h"
#include "materials/materialFeatureData.h"
#include "shaderGen/featureType.h"
#include "shaderGen/featureMgr.h"
#include "scene/sceneRenderState.h"


const MatInstanceHookType ReflectionMaterialHook::Type( "Reflection" );

ReflectionMaterialHook::ReflectionMaterialHook() : 
   mReflectMat(NULL)
{

}

ReflectionMaterialHook::~ReflectionMaterialHook()
{
   SAFE_DELETE(mReflectMat);
}

void ReflectionMaterialHook::init( BaseMatInstance *inMat )
{
   if( !inMat->isValid() )
      return;

   Material *reflectMat = (Material*)inMat->getMaterial();
   if ( inMat->isCustomMaterial() )
   {
      // This is a custom material... who knows what it really does...do something
      // smart here later.
   }

   // We may want to disable some states that the material might enable for us.
   GFXStateBlockDesc refractState = inMat->getUserStateBlock();

   // Always z-read, and z-write if the material isn't translucent
   refractState.setZReadWrite( true, reflectMat->isTranslucent() ? false : true );
   
   // Create reflection material instance.
   BaseMatInstance *newMat = new ReflectionMatInstance( reflectMat );
   newMat->setUserObject( inMat->getUserObject() );
   newMat->getFeaturesDelegate().bind( &ReflectionMaterialHook::_overrideFeatures );
   newMat->addStateBlockDesc( refractState );
   if( !newMat->init( inMat->getFeatures(), inMat->getVertexFormat() ) )
   {
      SAFE_DELETE( newMat );
      newMat = MATMGR->createWarningMatInstance();
   }
   
   mReflectMat = newMat;
}

void ReflectionMaterialHook::_overrideFeatures(  ProcessedMaterial *mat,
                                             U32 stageNum,
                                             MaterialFeatureData &fd, 
                                             const FeatureSet &features )
{
   // First stage only in reflections
   if( stageNum != 0 )
   {
      fd.features.clear();
      return;
   }

   fd.features.addFeature( MFT_Fog );
}

//------------------------------------------------------------------------------

ReflectionMatInstance::ReflectionMatInstance( Material *mat ) 
 : MatInstance( *mat )
{

}

bool ReflectionMatInstance::setupPass( SceneRenderState *state, const SceneData &sgData )
{
   return Parent::setupPass(state, sgData);
}
