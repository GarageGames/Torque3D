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
#include "forest/windDeformation.h"

#include "forest/forestItem.h"
#include "forest/forestWindMgr.h"
#include "forest/forestWindAccumulator.h"
#include "materials/sceneData.h"
#include "scene/sceneRenderState.h"
#include "gfx/gfxShader.h"
#include "gfx/gfxStructs.h"


ImplementFeatureType( MFT_WindEffect, MFG_PreTransform, 0, false );


void WindDeformationConstHandles::init( GFXShader *shader )
{
   mWindDirAndSpeed = shader->getShaderConstHandle( "$windDirAndSpeed" );
   mWindParams = shader->getShaderConstHandle( "$windParams" );
}

void WindDeformationConstHandles::setConsts( SceneRenderState *state,
                                             const SceneData &sgData,
                                             GFXShaderConstBuffer *buffer )
{
   PROFILE_SCOPE( WindDeformationConstHandles_setConsts );

   Point3F windDir( 0, 0, 0 );
   F32 windSpeed = 0;
   F32 bendScale = 0;
   F32 branchAmp = 0;
   F32 detailAmp = 0;
   F32 detailFreq = 0;

   const ForestItem *item = (const ForestItem*)sgData.materialHint;

   ForestWindAccumulator *wind = NULL;
   if ( item ) 
   {
      // First setup the per-datablock shader constants.
      //
      // These cannot be skipped as they modifiy the rest
      // state of a tree even without wind.
      //
      const ForestItemData *itemData = item->getData();
      const F32 windScale = itemData->mWindScale;
      bendScale = itemData->mTrunkBendScale * windScale;
      branchAmp = itemData->mWindBranchAmp * windScale;
      detailAmp = itemData->mWindDetailAmp * windScale;
      detailFreq = itemData->mWindDetailFreq * windScale;

      // Look for wind state.
      wind = WINDMGR->getLocalWind( item->getKey() );
   }

   if ( wind )
   {
      // Calculate distance to camera fade.   
      F32 toCamLen = ( state->getDiffuseCameraPosition() - wind->getPosition()).len();
      F32 toCamInterp = 1.0f - (toCamLen / ForestWindMgr::smWindEffectRadius);
      toCamInterp = mClampF( toCamInterp, 0.0f, 1.0f );

      windDir.set( wind->getDirection().x, wind->getDirection().y, 0.0f );
      windSpeed = wind->getStrength();

      // Since the effect is just a displacement
      // of geometry i need to scale it up for smaller
      // trees so it looks like its affecting it more.
      //
      // TODO: This is possibly a side effect of not 
      // properly doing the displacement physics in the
      // first place.
      //
      const F32 strengthScale = 1.0f / item->getScale();
      windDir *= strengthScale;
      windSpeed *= strengthScale;

      // Add in fade.
      windDir *= toCamInterp;
      windSpeed *= toCamInterp;

      MatrixF invXfm( item->getTransform() );
      invXfm.inverse();      
      invXfm.mulV( windDir );
   }
   else
   {
      /*
      // HACK:  This should go away... we need to work out a strategy
      // for allocating accumulators that accounts for random objects
      // and not just forest elements.
      //
      // Maybe add a generic 'id' value to SceneData that is set
      // by the RenderInst that can be any unique identifier for that
      // object that we can used down here to fetch an item id.
            
      // TODO: This is horrible... we should avoid access to the script
      // system while rendering as its really slow!            
      if ( Con::getBoolVariable( "$tsStaticWindHack", false ) )
      {
         VectorF windVec( mSin( Sim::getCurrentTime() / 1000.0f ) * 0.25f, 0.0f, 0.0f );
         //sgData.objTrans.mulV( windVec );
         windDir.x = windVec.x;
         windDir.y = windVec.y;
         windSpeed = 0.75f;
      }
      */
   }

   buffer->setSafe( mWindDirAndSpeed, Point3F( windDir.x, windDir.y, windSpeed ) );
   buffer->setSafe( mWindParams, Point4F( bendScale, branchAmp, detailAmp, detailFreq ) );
}
