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
#include "forest/ts/tsForestItemData.h"

#include "forest/ts/tsForestCellBatch.h"
#include "core/resourceManager.h"
#include "ts/tsShapeInstance.h"
#include "ts/tsLastDetail.h"
#include "sim/netConnection.h"
#include "materials/materialManager.h"
#include "forest/windDeformation.h"


IMPLEMENT_CO_DATABLOCK_V1(TSForestItemData);

ConsoleDocClass( TSForestItemData,
   "@brief Concrete implementation of ForestItemData which loads and renders "
   "dts format shapeFiles.\n\n"
   "@ingroup Forest"
);

TSForestItemData::TSForestItemData()
   :  mShapeInstance( NULL ),
      mIsClientObject( false )
{
}

TSForestItemData::~TSForestItemData()
{
}

bool TSForestItemData::preload( bool server, String &errorBuffer )   
{
   mIsClientObject = !server;

   if ( !SimDataBlock::preload( server, errorBuffer ) )
      return false;

   return true;
}

void TSForestItemData::_updateCollisionDetails()
{
   mCollisionDetails.clear();
   mLOSDetails.clear();
   mShape->findColDetails( false, &mCollisionDetails, &mLOSDetails );
}

bool TSForestItemData::onAdd()
{
   if ( !Parent::onAdd() )
      return false;

   // Register for the resource change signal.
   ResourceManager::get().getChangedSignal().notify( this, &TSForestItemData::_onResourceChanged );

   return true;
}

void TSForestItemData::onRemove()
{
   // Remove the resource change signal.
   ResourceManager::get().getChangedSignal().remove( this, &TSForestItemData::_onResourceChanged );

   SAFE_DELETE( mShapeInstance );

   Parent::onRemove();
}

void TSForestItemData::inspectPostApply()
{
   Parent::inspectPostApply();

   SAFE_DELETE( mShapeInstance );
   _loadShape();
}

void TSForestItemData::_onResourceChanged( const Torque::Path &path )
{
   if ( path != Path( mShapeFile ) )
      return;
   
   SAFE_DELETE( mShapeInstance );
   _loadShape();   

   getReloadSignal().trigger();
}

void TSForestItemData::_loadShape()
{
   mShape = ResourceManager::get().load(mShapeFile);
   if ( !(bool)mShape )
      return;

   if ( mIsClientObject && 
       !mShape->preloadMaterialList( mShapeFile ) )   
      return;
   
   // Lets add an autobillboard detail if don't have one.
   //_checkLastDetail();

   _updateCollisionDetails();
}

TSShapeInstance* TSForestItemData::_getShapeInstance() const
{
   // Create the shape instance if we haven't already.
   if ( !mShapeInstance && mShape )
   {
      // Create the instance.
      mShapeInstance = new TSShapeInstance( mShape, true );
        
      // So we can make OpCode collision calls.
      mShapeInstance->prepCollision();

      // Get the material features adding the wind effect if
      // we have a positive wind scale and have vertex color 
      // data which is used for the weighting.
      FeatureSet features = MATMGR->getDefaultFeatures();
      if ( mWindScale > 0.0f && mShape->getVertexFormat()->hasColor() )
      {
         // We create our own cloned material list to
         // enable the wind effects.
         features.addFeature( MFT_WindEffect );
         mShapeInstance->cloneMaterialList( &features );
      }
   }

   return mShapeInstance;
}

void TSForestItemData::_checkLastDetail()
{
   const S32 dl = mShape->mSmallestVisibleDL;
   const TSDetail *detail = &mShape->details[dl];

   // TODO: Expose some real parameters to the datablock maybe?
   if ( detail->subShapeNum != -1 )
   {
      mShape->addImposter( mShapeFile, 10, 4, 0, 0, 256, 0, 0 );

      // HACK: If i don't do this it crashes!
      while ( mShape->detailCollisionAccelerators.size() < mShape->details.size() )
         mShape->detailCollisionAccelerators.push_back( NULL );
   }
}

TSLastDetail* TSForestItemData::getLastDetail() const
{
   // Gotta call this first of the last detail isn't created!
   if (!_getShapeInstance())
      return NULL;

   const S32 dl = mShape->mSmallestVisibleDL;
   const TSDetail* detail = &mShape->details[dl];
   if (  detail->subShapeNum >= 0 ||
         mShape->billboardDetails.size() <= dl )
      return NULL;

   return mShape->billboardDetails[dl];
}

ForestCellBatch* TSForestItemData::allocateBatch() const
{
   TSLastDetail* lastDetail = getLastDetail();
   if ( !lastDetail )
      return NULL;

   return new TSForestCellBatch( lastDetail );
}

bool TSForestItemData::canBillboard( const SceneRenderState *state, const ForestItem &item, F32 distToCamera ) const
{
   PROFILE_SCOPE( TSForestItemData_canBillboard );

   if ( !mShape )
      return false;

   // Use the shape instance to do the work it normally does.
   TSShapeInstance *shapeInstance = _getShapeInstance();
   const S32 dl = shapeInstance->setDetailFromDistance( state, distToCamera / item.getScale() );

   // This item has a null LOD... lets consider 
   // that as being billboarded.
   if ( dl < 0 )
      return true;

   const TSDetail *detail = &mShape->details[dl];
   if ( detail->subShapeNum < 0 && dl < mShape->billboardDetails.size() )
      return true;

   return false;
}

bool TSForestItemData::render( TSRenderState *rdata, const ForestItem &item ) const
{
   PROFILE_SCOPE( TSForestItemData_render );

   // This shouldn't happen normally at runtime, but during
   // development a file change notification on a bad file
   // can cause us to get here without a shape.
   TSShapeInstance *shapeInst = _getShapeInstance();
   if ( !shapeInst )
      return false;

   const F32 scale = item.getScale();

   // Figure out the distance of this item to the camera.
   const SceneRenderState *state = rdata->getSceneState();
   F32 dist = ( item.getPosition() - state->getDiffuseCameraPosition() ).len();

   // TODO: Selecting the lod seems more expensive than
   // it should be... we should look to optimize this.
   if ( shapeInst->setDetailFromDistance( state, dist / scale ) < 0 )
      return false;

   // TSShapeInstance::render() uses the 
   // world matrix for the RenderInst.
   MatrixF worldMat = item.getTransform();
   worldMat.scale( scale );
   GFX->setWorldMatrix( worldMat );
   rdata->setMaterialHint( (void*)&item );

   // This isn't documented well, but these calls
   // don't really render... rather they batch the
   // shape to be rendered by the render instance
   // manager later on.
   shapeInst->animate();
   shapeInst->render( *rdata );
   return true;
}