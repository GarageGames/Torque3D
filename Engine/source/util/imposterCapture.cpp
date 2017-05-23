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
#include "util/imposterCapture.h"

#include "gfx/bitmap/gBitmap.h"
#include "core/color.h"
#include "renderInstance/renderPassManager.h"
#include "renderInstance/renderMeshMgr.h"
#include "materials/materialManager.h"
#include "materials/materialFeatureTypes.h"
#include "materials/customMaterialDefinition.h"
#include "ts/tsShapeInstance.h"
#include "scene/sceneManager.h"
#include "scene/sceneRenderState.h"
#include "lighting/lightInfo.h"
#include "gfx/gfxTransformSaver.h"
#include "gfx/gfxDebugEvent.h"
#include "core/stream/fileStream.h"


/// A material hook used to hold imposter generation 
/// rendering materials for an object.
class ImposterCaptureMaterialHook : public MatInstanceHook
{
public:

   ImposterCaptureMaterialHook();

   // MatInstanceHook
   virtual ~ImposterCaptureMaterialHook();
   virtual const MatInstanceHookType& getType() const { return Type; }

   /// The material hook type.
   static const MatInstanceHookType Type;

   void init( BaseMatInstance *mat );

   static BaseMatInstance* getDiffuseInst( BaseMatInstance *inMat ) 
      { return _getOrCreateHook( inMat )->mDiffuseMatInst; }

   static BaseMatInstance* getNormalsInst( BaseMatInstance *inMat ) 
      { return _getOrCreateHook( inMat )->mNormalsMatInst; }

protected:

   static void _overrideFeatures(   ProcessedMaterial *mat,
                                    U32 stageNum,
                                    MaterialFeatureData &fd, 
                                    const FeatureSet &features );

   static ImposterCaptureMaterialHook* _getOrCreateHook( BaseMatInstance *inMat );

   /// 
   BaseMatInstance *mDiffuseMatInst;

   ///
   BaseMatInstance *mNormalsMatInst;
};


const MatInstanceHookType ImposterCaptureMaterialHook::Type( "ImposterCapture" );


ImposterCaptureMaterialHook::ImposterCaptureMaterialHook()
   :  mDiffuseMatInst( NULL ),
      mNormalsMatInst( NULL )
{
}

ImposterCaptureMaterialHook::~ImposterCaptureMaterialHook()
{
   SAFE_DELETE( mDiffuseMatInst );
   SAFE_DELETE( mNormalsMatInst );
}

void ImposterCaptureMaterialHook::init( BaseMatInstance *inMat )
{
   // We cannot capture impostors on custom materials
   // as we don't know how to get just diffuse and just
   // normals rendering.
   if ( dynamic_cast<CustomMaterial*>( inMat->getMaterial() ) )
      return;

   // Tweak the feature data to include just what we need.
   FeatureSet features;
   features.addFeature( MFT_VertTransform );
   features.addFeature( MFT_DiffuseMap );
   features.addFeature( MFT_OverlayMap );
   features.addFeature( MFT_DetailMap );
   features.addFeature( MFT_DiffuseColor );
   features.addFeature( MFT_AlphaTest );
   features.addFeature( MFT_IsTranslucent );

   const String &matName = inMat->getMaterial()->getName();

   mDiffuseMatInst = MATMGR->createMatInstance( matName );
   mDiffuseMatInst->getFeaturesDelegate().bind( &ImposterCaptureMaterialHook::_overrideFeatures );
   mDiffuseMatInst->init( features, inMat->getVertexFormat() );
   
   features.addFeature( MFT_IsDXTnm );
   features.addFeature( MFT_NormalMap );
   features.addFeature( MFT_NormalsOut );
   features.addFeature( MFT_AccuMap );
   mNormalsMatInst = MATMGR->createMatInstance( matName );
   mNormalsMatInst->getFeaturesDelegate().bind( &ImposterCaptureMaterialHook::_overrideFeatures );
   mNormalsMatInst->init( features, inMat->getVertexFormat() );
}

void ImposterCaptureMaterialHook::_overrideFeatures(  ProcessedMaterial *mat,
                                                      U32 stageNum,
                                                      MaterialFeatureData &fd, 
                                                      const FeatureSet &features )
{
   if ( features.hasFeature( MFT_NormalsOut) )
      fd.features.addFeature( MFT_NormalsOut );

   fd.features.addFeature( MFT_ForwardShading );
   fd.features.addFeature( MFT_Imposter );
}

ImposterCaptureMaterialHook* ImposterCaptureMaterialHook::_getOrCreateHook( BaseMatInstance *inMat )
{
   ImposterCaptureMaterialHook *hook = inMat->getHook<ImposterCaptureMaterialHook>();
   if ( !hook )
   {
      // Create a hook and initialize it using the incoming material.
      hook = new ImposterCaptureMaterialHook;
      hook->init( inMat );
      inMat->addHook( hook );
   }

   return hook;
}


ImposterCapture::ImposterCapture()
:  mDl( 0 ),
   mDim( 0 ),
   mRadius( 0.0f ),
   mCenter( Point3F( 0, 0, 0 ) ),
   mBlackBmp( NULL ),
   mWhiteBmp( NULL ),
   mState( NULL ),
   mShapeInstance( NULL ),
   mRenderTarget( NULL ),
   mRenderPass( NULL ),
   mMeshRenderBin( NULL )
{     
}                                   

ImposterCapture::~ImposterCapture()
{
   AssertFatal( !mShapeInstance, "ImposterCapture destructor - TSShapeInstance hasn't been cleared!" );
}

void ImposterCapture::_colorAverageFilter( U32 dimensions, const U8 *inBmpBits, U8 *outBmpBits )
{
   ColorF color;
   U32 count = 0;
   U32 index, index2;

   for ( S32 y = 0; y < dimensions; y++ )
   {
      for( S32 x = 0; x < dimensions; x++ )
      {
         // We only blend on transparent pixels.
         index = ( ( y * dimensions ) + x ) * 4;
         if ( inBmpBits[index+3] > 84 )
         {
            outBmpBits[index+0] = inBmpBits[index+0];
            outBmpBits[index+1] = inBmpBits[index+1];
            outBmpBits[index+2] = inBmpBits[index+2];
            outBmpBits[index+3] = inBmpBits[index+3]; //hack
            continue;
         }

         color.set(0,0,0);
         count = 0;

         for ( S32 fy = y-6; fy <= y+6; fy++ )
         {
            for ( S32 fx = x-6; fx <= x+6; fx++ )
            {
               if (  fy >= 0 && fy < (dimensions-1) &&
                     fx >= 0 && fx < (dimensions-1) )
               {
                  index2 = ( ( fy * dimensions ) + fx ) * 4;
                  if ( inBmpBits[index2+3] > 84 )
                  {
                     color.red += inBmpBits[index2+0];
                     color.green += inBmpBits[index2+1];
                     color.blue += inBmpBits[index2+2];
                     ++count;
                  }
               }
            }
         }

         outBmpBits[index+0] = (U8)( (F32)color.red / (F32)count );
         outBmpBits[index+1] = (U8)( (F32)color.green / (F32)count );
         outBmpBits[index+2] = (U8)( (F32)color.blue / (F32)count );
         outBmpBits[index+3] = 0;
      }
   }
}

void ImposterCapture::_renderToTexture( GFXTexHandle texHandle, GBitmap *outBitmap, const ColorI &color )
{
   GFXDEBUGEVENT_SCOPE( ImposterCapture_RenderToTexture, ColorI::RED );
   PROFILE_SCOPE( ImposterCapture_RenderToTexture );

   mRenderTarget->attachTexture( GFXTextureTarget::Color0, texHandle );
   mRenderTarget->attachTexture( GFXTextureTarget::DepthStencil, mDepthBuffer );
   GFX->setActiveRenderTarget( mRenderTarget );

   GFX->clear( GFXClearZBuffer | GFXClearStencil | GFXClearTarget, color, 1.0f, 0 );

   mShapeInstance->render( mRData, mDl, 1.0f );

   mState->getRenderPass()->renderPass( mState );

   mRenderTarget->resolve();

   texHandle->copyToBmp( outBitmap );
}

void ImposterCapture::_separateAlpha( GBitmap *imposterOut )
{
   PROFILE_START(TSShapeInstance_snapshot_sb_separate);

   // TODO: Remove all this when we get rid of the 'render on black/white'.

      // now separate the color and alpha channels
      GBitmap *bmp = new GBitmap;
      bmp->allocateBitmap(mDim, mDim, false, GFXFormatR8G8B8A8);
      U8 * wbmp = (U8*)mWhiteBmp->getBits(0);
      U8 * bbmp = (U8*)mBlackBmp->getBits(0);
      U8 * dst  = (U8*)bmp->getBits(0);

      const U32 pixCount = mDim * mDim;

      // simpler, probably faster...
      for ( U32 i=0; i < pixCount; i++ )
      {
         // Shape on black background is alpha * color, shape on white 
         // background is alpha * color + (1-alpha) * 255 we want 255 *
         // alpha, or 255 - (white - black).
         //
         // JMQ: or more verbosely:
         //  cB = alpha * color + (0 * (1 - alpha))
         //  cB = alpha * color
         //  cW = alpha * color + (255 * (1 - alpha))
         //  cW = cB + (255 * (1 - alpha))
         // solving for alpha
         //  cW - cB = 255 * (1 - alpha)
         //  (cW - cB)/255 = (1 - alpha)
         //  alpha = 1 - (cW - cB)/255
         // since we want alpha*255, multiply through by 255
         //  alpha * 255 = 255 - cW - cB
         U32 alpha = 255 - (wbmp[i*3+0] - bbmp[i*3+0]);
         alpha    += 255 - (wbmp[i*3+1] - bbmp[i*3+1]);
         alpha    += 255 - (wbmp[i*3+2] - bbmp[i*3+2]);

         if ( alpha != 0 )
         {
            F32 floatAlpha = ((F32)alpha)/(3.0f*255.0f); 
            dst[i*4+0] = (U8)(bbmp[i*3+0] / floatAlpha);
            dst[i*4+1] = (U8)(bbmp[i*3+1] / floatAlpha);
            dst[i*4+2] = (U8)(bbmp[i*3+2] / floatAlpha);

            // Before we assign the alpha we "fizzle" the value
            // if its greater than 84.  This causes the imposter
            // to dissolve instead of popping into view.
            alpha /= 3;
            dst[i*4+3] = (U8)alpha;
         }
         else
         {
            dst[i*4+0] = dst[i*4+1] = dst[i*4+2] = dst[i*4+3] = 0;
         }
      }

   PROFILE_END(); // TSShapeInstance_snapshot_sb_separate
  
   PROFILE_START(TSShapeInstance_snapshot_sb_filter);

      // We now run a special kernel filter over the image that
      // averages color into the transparent areas.  This should
      // in essence give us a border around the edges of the 
      // imposter silhouette which fixes the artifacts around the
      // alpha test billboards.
      U8* dst2 = (U8*)imposterOut->getBits(0);

      _colorAverageFilter( mDim, dst, dst2 );
      
      if ( 0 )
      {
         FileStream fs;
         if ( fs.open( "./imposterout.png", Torque::FS::File::Write ) )
            imposterOut->writeBitmap( "png", fs );

         fs.close();

         if ( fs.open( "./temp.png", Torque::FS::File::Write ) )
            bmp->writeBitmap( "png", fs );

         fs.close();
      }
   

   PROFILE_END(); // TSShapeInstance_snapshot_sb_filter

   delete bmp;
}


void ImposterCapture::_convertDXT5nm( GBitmap *normalsOut )
{
   PROFILE_SCOPE(ImposterCapture_ConvertDXT5nm);

   U8 *bits  = (U8*)normalsOut->getBits(0);
   const U32 pixCount = mDim * mDim;
   U8 x, y, z;

   // Encoding in object space DXT5 which moves
   // one of the coords to the alpha channel for
   // improved precision.... in our case z.

   for ( U32 i=0; i < pixCount; i++ )
   {
      x = bits[i*4+0];
      y = bits[i*4+1];
      z = bits[i*4+2];

      bits[i*4+0] = x;
      bits[i*4+1] = y;
      bits[i*4+2] = 0;
      bits[i*4+3] = z;
   }
}

void ImposterCapture::begin(  TSShapeInstance *shapeInst,
                              S32 dl, 
                              S32 dim,
                              F32 radius,
                              const Point3F &center )
{
   mShapeInstance = shapeInst;
   mDl = dl;
   mDim = dim;
   mRadius = radius;
   mCenter = center;

   mBlackTex.set( mDim, mDim, GFXFormatR8G8B8A8, &GFXDefaultRenderTargetProfile, avar( "%s() - (line %d)", __FUNCTION__, __LINE__ ) ); 
   mWhiteTex.set( mDim, mDim, GFXFormatR8G8B8A8, &GFXDefaultRenderTargetProfile, avar( "%s() - (line %d)", __FUNCTION__, __LINE__ ) ); 
   mNormalTex.set( mDim, mDim, GFXFormatR8G8B8A8, &GFXDefaultRenderTargetProfile, avar( "%s() - (line %d)", __FUNCTION__, __LINE__ ) ); 
   mDepthBuffer.set( mDim, mDim, GFXFormatD24S8, &GFXDefaultZTargetProfile, avar( "%s() - (line %d)", __FUNCTION__, __LINE__ ) ); 

   // copy the black render target data into a bitmap
   mBlackBmp = new GBitmap;
   mBlackBmp->allocateBitmap(mDim, mDim, false, GFXFormatR8G8B8);

   // copy the white target data into a bitmap
   mWhiteBmp = new GBitmap;
   mWhiteBmp->allocateBitmap(mDim, mDim, false, GFXFormatR8G8B8);

   // Setup viewport and frustrum to do orthographic projection.
   RectI viewport( 0, 0, mDim, mDim );
   GFX->setViewport( viewport );
   GFX->setOrtho( -mRadius, mRadius, -mRadius, mRadius, 1, 20.0f * mRadius );

   // Position camera looking out the X axis.
   MatrixF cameraMatrix( true );
   cameraMatrix.setColumn( 0, Point3F( 0, 0, 1 ) );
   cameraMatrix.setColumn( 1, Point3F( 1, 0, 0 ) );
   cameraMatrix.setColumn( 2, Point3F( 0, 1, 0 ) );

   // setup scene state required for TS mesh render...this is messy and inefficient; 
   // should have a mode where most of this is done just once (and then 
   // only the camera matrix changes between snapshots).
   // note that we use getFrustum here, but we set up an ortho projection above.  
   // it doesn't seem like the scene state object pays attention to whether the projection is 
   // ortho or not.  this could become a problem if some code downstream tries to 
   // reconstruct the projection matrix using the dimensions and doesn't 
   // realize it should be ortho.  at the moment no code is doing that.
   F32 left, right, top, bottom, nearPlane, farPlane;
   bool isOrtho;
   GFX->getFrustum( &left, &right, &bottom, &top, &nearPlane, &farPlane, &isOrtho );
   Frustum frust( isOrtho, left, right, top, bottom, nearPlane, farPlane, cameraMatrix );

   // Set up render pass.

   mRenderPass = new RenderPassManager();
   mRenderPass->assignName( "DiffuseRenderPass" );
   mMeshRenderBin = new RenderMeshMgr();
   mRenderPass->addManager( mMeshRenderBin );

   // Set up scene state.

   mState = new SceneRenderState(
      gClientSceneGraph,
      SPT_Diffuse,
      SceneCameraState( viewport, frust, GFX->getWorldMatrix(),GFX->getProjectionMatrix() ),
      mRenderPass,
      false
   );

   // Set up our TS render state.
   mRData.setSceneState( mState );
   mRData.setCubemap( NULL );
   mRData.setFadeOverride( 1.0f );

   // set gfx up for render to texture
   GFX->pushActiveRenderTarget();
   mRenderTarget = GFX->allocRenderToTextureTarget();

}

void ImposterCapture::capture(   const MatrixF &rotMatrix, 
                                 GBitmap **imposterOut,
                                 GBitmap **normalMapOut )
{
   GFXTransformSaver saver;

   // this version of the snapshot function renders the shape to a black texture, then to white, then reads bitmaps 
   // back for both renders and combines them, restoring the alpha and color values.  this is based on the
   // TGE implementation.  it is not fast due to the copy and software combination operations.  the generated bitmaps
   // are upside-down (which is how TGE generated them...)

   (*imposterOut) = new GBitmap( mDim, mDim, false, GFXFormatR8G8B8A8 );
   (*normalMapOut) = new GBitmap( mDim, mDim, false, GFXFormatR8G8B8A8 );

   // The object to world transform.
   MatrixF centerMat( true );
   centerMat.setPosition( -mCenter );
   MatrixF objMatrix( rotMatrix );
   objMatrix.mul( centerMat );
   GFX->setWorldMatrix( objMatrix );

   // The view transform.
   MatrixF view( EulerF( M_PI_F / 2.0f, 0, M_PI_F ), Point3F( 0, 0, -10.0f * mRadius ) );
   mRenderPass->assignSharedXform( RenderPassManager::View, view );

   mRenderPass->assignSharedXform( RenderPassManager::Projection, GFX->getProjectionMatrix() );

   // Render the diffuse pass.
   mRenderPass->clear();
   mMeshRenderBin->getMatOverrideDelegate().bind( ImposterCaptureMaterialHook::getDiffuseInst );
   _renderToTexture( mBlackTex, mBlackBmp, ColorI(0, 0, 0, 0) );
   _renderToTexture( mWhiteTex, mWhiteBmp, ColorI(255, 255, 255, 255) );

   // Now render the normals.
   mRenderPass->clear();
   mMeshRenderBin->getMatOverrideDelegate().bind( ImposterCaptureMaterialHook::getNormalsInst );
   _renderToTexture( mNormalTex, *normalMapOut, ColorI(0, 0, 0, 0) );


   _separateAlpha( *imposterOut );
   _convertDXT5nm( *normalMapOut );

   if ( 0 )
   {
      // Render out the bitmaps for debug purposes.
      FileStream fs;
      if ( fs.open( "./blackbmp.png", Torque::FS::File::Write ) )
         mBlackBmp->writeBitmap( "png", fs );

      fs.close();

      if ( fs.open( "./whitebmp.png", Torque::FS::File::Write ) )
         mWhiteBmp->writeBitmap( "png", fs );

      fs.close();

      if ( fs.open( "./normalbmp.png", Torque::FS::File::Write ) )
         (*normalMapOut)->writeBitmap( "png", fs );

      fs.close();

      if ( fs.open( "./finalimposter.png", Torque::FS::File::Write ) )
         (*imposterOut)->writeBitmap( "png", fs );

      fs.close();
   }
}

void ImposterCapture::end()
{
   GFX->popActiveRenderTarget();

   mBlackTex.free();
   mWhiteTex.free(); 
   mNormalTex.free();

   mShapeInstance = NULL;
   
   mRenderTarget = NULL;
   mMeshRenderBin = NULL; // Deleted by mRenderPass
   SAFE_DELETE( mState );
   SAFE_DELETE( mRenderPass );
   SAFE_DELETE( mBlackBmp );
   SAFE_DELETE( mWhiteBmp );
}

