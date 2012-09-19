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
#include "ts/tsLastDetail.h"

#include "renderInstance/renderPassManager.h"
#include "ts/tsShapeInstance.h"
#include "scene/sceneManager.h"
#include "scene/sceneRenderState.h"
#include "lighting/lightInfo.h"
#include "renderInstance/renderImposterMgr.h"
#include "gfx/gfxTransformSaver.h"
#include "gfx/bitmap/ddsFile.h"
#include "gfx/bitmap/ddsUtils.h"
#include "gfx/gfxTextureManager.h"
#include "math/mRandom.h"
#include "core/stream/fileStream.h"
#include "util/imposterCapture.h"
#include "materials/materialManager.h"
#include "materials/materialFeatureTypes.h"
#include "console/consoleTypes.h"


GFXImplementVertexFormat( ImposterState )
{
   addElement( "POSITION", GFXDeclType_Float4 );

   addElement( "ImposterParams", GFXDeclType_Float2, 0 );

   addElement( "ImposterUpVec", GFXDeclType_Float3, 1 );
   addElement( "ImposterRightVec", GFXDeclType_Float3, 2 );
};


Vector<TSLastDetail*> TSLastDetail::smLastDetails;

bool TSLastDetail::smCanShadow = true;


AFTER_MODULE_INIT( Sim )
{
   Con::addVariable( "$pref::imposter::canShadow", TypeBool, &TSLastDetail::smCanShadow,
      "User preference which toggles shadows from imposters.  Defaults to true.\n"
      "@ingroup Rendering\n" );
}


TSLastDetail::TSLastDetail(   TSShape *shape,
                              const String &cachePath,
                              U32 numEquatorSteps,
                              U32 numPolarSteps,
                              F32 polarAngle,
                              bool includePoles,
                              S32 dl, S32 dim )
{
   mNumEquatorSteps = getMax( numEquatorSteps, (U32)1 );
   mNumPolarSteps = numPolarSteps;
   mPolarAngle = polarAngle;
   mIncludePoles = includePoles;
   mShape = shape;
   mDl = dl;
   mDim = getMax( dim, (S32)32 );

   mRadius = mShape->radius;
   mCenter = mShape->center;

   mCachePath = cachePath;

   mMaterial = NULL;
   mMatInstance = NULL;

   // Store this in the static list.
   smLastDetails.push_back( this );  
}

TSLastDetail::~TSLastDetail()
{
   SAFE_DELETE( mMatInstance );
   if ( mMaterial )
      mMaterial->deleteObject();

   // Remove ourselves from the list.
   Vector<TSLastDetail*>::iterator iter = find( smLastDetails.begin(), smLastDetails.end(), this );
   smLastDetails.erase( iter );
}

void TSLastDetail::render( const TSRenderState &rdata, F32 alpha )
{
   // Early out if we have nothing to render.
   if (  alpha < 0.01f || 
         !mMatInstance ||
         mMaterial->mImposterUVs.size() == 0 )
      return;

   const MatrixF &mat = GFX->getWorldMatrix();

   // Post a render instance for this imposter... the special
   // imposter render manager will do the magic!
   RenderPassManager *renderPass = rdata.getSceneState()->getRenderPass();

   ImposterRenderInst *ri = renderPass->allocInst<ImposterRenderInst>();
   ri->mat = rdata.getSceneState()->getOverrideMaterial( mMatInstance );

   ri->state.alpha = alpha;

   // Store the up and right vectors of the rotation
   // and we'll generate the up vector in the shader.
   //
   // This is faster than building a quat on the
   // CPU and then rebuilding the matrix on the GPU.
   //
   // NOTE: These vector include scale.
   //
   mat.getColumn( 2, &ri->state.upVec );
   mat.getColumn( 0, &ri->state.rightVec );

   // We send the unscaled size and the vertex shader
   // will use the orientation vectors above to scale it.
   ri->state.halfSize = mRadius;

   // We use the center of the object bounds for
   // the center of the billboard quad.
   mat.mulP( mCenter, &ri->state.center );

   // We sort by the imposter type first so that RIT_Imposter and s
   // RIT_ImposterBatches do not get mixed together.
   //
   // We then sort by material.
   //
   ri->defaultKey = 1;
   ri->defaultKey2 = ri->mat->getStateHint();

   renderPass->addInst( ri );   
}

void TSLastDetail::update( bool forceUpdate )
{
   // This should never be called on a dedicated server or
   // anywhere else where we don't have a GFX device!
   AssertFatal( GFXDevice::devicePresent(), "TSLastDetail::update() - Cannot update without a GFX device!" );

   // Clear the materialfirst.
   SAFE_DELETE( mMatInstance );
   if ( mMaterial )
   {
      mMaterial->deleteObject();
      mMaterial = NULL;
   }

   // Make sure imposter textures have been flushed (and not just queued for deletion)
   TEXMGR->cleanupCache();

   // Get the real path to the source shape for doing modified time
   // comparisons... this might be different if the DAEs have been 
   // deleted from the install.
   String shapeFile( mCachePath );
   if ( !Platform::isFile( shapeFile ) )
   {
      Torque::Path path(shapeFile);
      path.setExtension("cached.dts");
      shapeFile = path.getFullPath();
      if ( !Platform::isFile( shapeFile ) )  
      {
         Con::errorf( "TSLastDetail::update - '%s' could not be found!", mCachePath.c_str() );
         return;
      }
   }

   // Do we need to update the imposter?
   const String diffuseMapPath = _getDiffuseMapPath();
   if (  forceUpdate || 
         Platform::compareModifiedTimes( diffuseMapPath, shapeFile ) <= 0 )
      _update();

   // If the time check fails now then the update must have not worked.
   if ( Platform::compareModifiedTimes( diffuseMapPath, shapeFile ) < 0 )
   {
      Con::errorf( "TSLastDetail::update - Failed to create imposters for '%s'!", mCachePath.c_str() );
      return;
   }

   // Figure out what our vertex format will be.
   //
   // If we're on SM 3.0 we can do multiple vertex streams
   // and the performance win is big as we send 3x less data
   // on each imposter instance.
   //
   // The problem is SM 2.0 won't do this, so we need to
   // support fallback to regular single stream imposters.
   //
   //mImposterVertDecl.copy( *getGFXVertexFormat<ImposterCorner>() );
   //mImposterVertDecl.append( *getGFXVertexFormat<ImposterState>(), 1 );
   //mImposterVertDecl.getDecl();   
   mImposterVertDecl.clear();
   mImposterVertDecl.copy( *getGFXVertexFormat<ImposterState>() );

   // Setup the material for this imposter.
   mMaterial = MATMGR->allocateAndRegister( String::EmptyString );
   mMaterial->mAutoGenerated = true;
   mMaterial->mDiffuseMapFilename[0] = diffuseMapPath;
   mMaterial->mNormalMapFilename[0] = _getNormalMapPath();
   mMaterial->mImposterLimits.set( (mNumPolarSteps * 2) + 1, mNumEquatorSteps, mPolarAngle, mIncludePoles );
   mMaterial->mTranslucent = true;
   mMaterial->mTranslucentBlendOp = Material::None;
   mMaterial->mTranslucentZWrite = true;
   mMaterial->mDoubleSided = true;
   mMaterial->mAlphaTest = true;
   mMaterial->mAlphaRef = 84;

   // Create the material instance.
   FeatureSet features = MATMGR->getDefaultFeatures();
   features.addFeature( MFT_ImposterVert );
   mMatInstance = mMaterial->createMatInstance();
   if ( !mMatInstance->init( features, &mImposterVertDecl ) )
   {
      delete mMatInstance;
      mMatInstance = NULL;
   }

   // Get the diffuse texture and from its size and
   // the imposter dimensions we can generate the UVs.
   GFXTexHandle diffuseTex( diffuseMapPath, &GFXDefaultStaticDiffuseProfile, String::EmptyString );
   Point2I texSize( diffuseTex->getWidth(), diffuseTex->getHeight() );

   _validateDim();

   S32 downscaledDim = mDim >> GFXTextureManager::getTextureDownscalePower(&GFXDefaultStaticDiffuseProfile);

   // Ok... pack in bitmaps till we run out.
   Vector<RectF> imposterUVs;
   for ( S32 y=0; y+downscaledDim <= texSize.y; )
   {
      for ( S32 x=0; x+downscaledDim <= texSize.x; )
      {
         // Store the uv for later lookup.
         RectF info;
         info.point.set( (F32)x / (F32)texSize.x, (F32)y / (F32)texSize.y );
         info.extent.set( (F32)downscaledDim / (F32)texSize.x, (F32)downscaledDim / (F32)texSize.y );
         imposterUVs.push_back( info );
         
         x += downscaledDim;
      }

      y += downscaledDim;
   }

   AssertFatal( imposterUVs.size() != 0, "hey" );

   mMaterial->mImposterUVs = imposterUVs;
}

void TSLastDetail::_validateDim()
{
   // Loop till they fit.
   S32 newDim = mDim;
   while ( true )
   {
      S32 maxImposters = ( smMaxTexSize / newDim ) * ( smMaxTexSize / newDim );
      S32 imposterCount = ( ((2*mNumPolarSteps) + 1 ) * mNumEquatorSteps ) + ( mIncludePoles ? 2 : 0 );
      if ( imposterCount <= maxImposters )
         break;

      // There are too many imposters to fit a single 
      // texture, so we fail.  These imposters are for
      // rendering small distant objects.  If you need
      // a really high resolution imposter or many images
      // around the equator and poles, maybe you need a
      // custom solution.

      newDim /= 2;
   }

   if ( newDim != mDim )
   {
      Con::printf( "TSLastDetail::_validateDim - '%s' detail dimensions too big! Reduced from %d to %d.", 
         mCachePath.c_str(),
         mDim, newDim );

      mDim = newDim;
   }
}

void TSLastDetail::_update()
{
   // We're gonna render... make sure we can.
   bool sceneBegun = GFX->canCurrentlyRender();
   if ( !sceneBegun )
      GFX->beginScene();

   _validateDim();

   Vector<GBitmap*> bitmaps;
   Vector<GBitmap*> normalmaps;

   // We need to create our own instance to render with.
   TSShapeInstance *shape = new TSShapeInstance( mShape, true );

   // Animate the shape once.
   shape->animate( mDl );

   // So we don't have to change it everywhere.
   const GFXFormat format = GFXFormatR8G8B8A8;  

   S32 imposterCount = ( ((2*mNumPolarSteps) + 1 ) * mNumEquatorSteps ) + ( mIncludePoles ? 2 : 0 );

   // Figure out the optimal texture size.
   Point2I texSize( smMaxTexSize, smMaxTexSize );
   while ( true )
   {
      Point2I halfSize( texSize.x / 2, texSize.y / 2 );
      U32 count = ( halfSize.x / mDim ) * ( halfSize.y / mDim );
      if ( count < imposterCount )
      {
         // Try half of the height.
         count = ( texSize.x / mDim ) * ( halfSize.y / mDim );
         if ( count >= imposterCount )
            texSize.y = halfSize.y;
         break;
      }

      texSize = halfSize;
   }

   GBitmap *imposter = NULL;
   GBitmap *normalmap = NULL;
   GBitmap destBmp( texSize.x, texSize.y, true, format );
   GBitmap destNormal( texSize.x, texSize.y, true, format );

   U32 mipLevels = destBmp.getNumMipLevels();

   ImposterCapture *imposterCap = new ImposterCapture();

   F32 equatorStepSize = M_2PI_F / (F32)mNumEquatorSteps;

   static const MatrixF topXfm( EulerF( -M_PI_F / 2.0f, 0, 0 ) );
   static const MatrixF bottomXfm( EulerF( M_PI_F / 2.0f, 0, 0 ) );

   MatrixF angMat;

   F32 polarStepSize = 0.0f;
   if ( mNumPolarSteps > 0 )
      polarStepSize = -( 0.5f * M_PI_F - mDegToRad( mPolarAngle ) ) / (F32)mNumPolarSteps;

   PROFILE_START(TSLastDetail_snapshots);

   S32 currDim = mDim;
   for ( S32 mip = 0; mip < mipLevels; mip++ )
   {
      if ( currDim < 1 )
         currDim = 1;
      
      dMemset( destBmp.getWritableBits(mip), 0, destBmp.getWidth(mip) * destBmp.getHeight(mip) * GFXFormat_getByteSize( format ) );
      dMemset( destNormal.getWritableBits(mip), 0, destNormal.getWidth(mip) * destNormal.getHeight(mip) * GFXFormat_getByteSize( format ) );

      bitmaps.clear();
      normalmaps.clear();

      F32 rotX = 0.0f;
      if ( mNumPolarSteps > 0 )
         rotX = -( mDegToRad( mPolarAngle ) - 0.5f * M_PI_F );

      // We capture the images in a particular order which must
      // match the order expected by the imposter renderer.

      imposterCap->begin( shape, mDl, currDim, mRadius, mCenter );

      for ( U32 j=0; j < (2 * mNumPolarSteps + 1); j++ )
      {
         F32 rotZ = -M_PI_F / 2.0f;

         for ( U32 k=0; k < mNumEquatorSteps; k++ )
         {            
            angMat.mul( MatrixF( EulerF( rotX, 0, 0 ) ),
                        MatrixF( EulerF( 0, 0, rotZ ) ) );

            imposterCap->capture( angMat, &imposter, &normalmap );

            bitmaps.push_back( imposter );
            normalmaps.push_back( normalmap );

            rotZ += equatorStepSize;
         }

         rotX += polarStepSize;

         if ( mIncludePoles )
         {
            imposterCap->capture( topXfm, &imposter, &normalmap );

            bitmaps.push_back(imposter);
            normalmaps.push_back( normalmap );

            imposterCap->capture( bottomXfm, &imposter, &normalmap );

            bitmaps.push_back( imposter );
            normalmaps.push_back( normalmap );
         }         
      }

      imposterCap->end();

      Point2I texSize( destBmp.getWidth(mip), destBmp.getHeight(mip) );

      // Ok... pack in bitmaps till we run out.
      for ( S32 y=0; y+currDim <= texSize.y; )
      {
         for ( S32 x=0; x+currDim <= texSize.x; )
         {
            // Copy the next bitmap to the dest texture.
            GBitmap* bmp = bitmaps.first();
            bitmaps.pop_front();
            destBmp.copyRect( bmp, RectI( 0, 0, currDim, currDim ), Point2I( x, y ), 0, mip );
            delete bmp;

            // Copy the next normal to the dest texture.
            GBitmap* normalmap = normalmaps.first();
            normalmaps.pop_front();
            destNormal.copyRect( normalmap, RectI( 0, 0, currDim, currDim ), Point2I( x, y ), 0, mip );
            delete normalmap;

            // Did we finish?
            if ( bitmaps.empty() )
               break;

            x += currDim;
         }

         // Did we finish?
         if ( bitmaps.empty() )
            break;

         y += currDim;
      }

      // Next mip...
      currDim /= 2;
   }

   PROFILE_END(); // TSLastDetail_snapshots

   delete imposterCap;
   delete shape;   
   
   
   // Should we dump the images?
   if ( Con::getBoolVariable( "$TSLastDetail::dumpImposters", false ) )
   {
      String imposterPath = mCachePath + ".imposter.png";
      String normalsPath = mCachePath + ".imposter_normals.png";

      FileStream stream;
      if ( stream.open( imposterPath, Torque::FS::File::Write  ) )
         destBmp.writeBitmap( "png", stream );
      stream.close();

      if ( stream.open( normalsPath, Torque::FS::File::Write ) )
         destNormal.writeBitmap( "png", stream );
      stream.close();
   }

   // DEBUG: Some code to force usage of a test image.
   //GBitmap* tempMap = GBitmap::load( "./forest/data/test1234.png" );
   //tempMap->extrudeMipLevels();
   //mTexture.set( tempMap, &GFXDefaultStaticDiffuseProfile, false );
   //delete tempMap;

   DDSFile *ddsDest = DDSFile::createDDSFileFromGBitmap( &destBmp );
   DDSUtil::squishDDS( ddsDest, GFXFormatDXT3 );

   DDSFile *ddsNormals = DDSFile::createDDSFileFromGBitmap( &destNormal );
   DDSUtil::squishDDS( ddsNormals, GFXFormatDXT5 );

   // Finally save the imposters to disk.
   FileStream fs;
   if ( fs.open( _getDiffuseMapPath(), Torque::FS::File::Write ) )
   {
      ddsDest->write( fs );
      fs.close();
   }
   if ( fs.open( _getNormalMapPath(), Torque::FS::File::Write ) )
   {
      ddsNormals->write( fs );
      fs.close();
   }

   delete ddsDest;
   delete ddsNormals;

   // If we did a begin then end it now.
   if ( !sceneBegun )
      GFX->endScene();
}

void TSLastDetail::deleteImposterCacheTextures()
{
   const String diffuseMap = _getDiffuseMapPath();
   if ( diffuseMap.length() )
      dFileDelete( diffuseMap );

   const String normalMap = _getNormalMapPath();
   if ( normalMap.length() )
      dFileDelete( normalMap );
}

void TSLastDetail::updateImposterImages( bool forceUpdate )
{
   // Can't do it without GFX!
   if ( !GFXDevice::devicePresent() )
      return;

   //D3DPERF_SetMarker( D3DCOLOR_RGBA( 0, 255, 0, 255 ), L"TSLastDetail::makeImposter" );

   bool sceneBegun = GFX->canCurrentlyRender();
   if ( !sceneBegun )
      GFX->beginScene();

   Vector<TSLastDetail*>::iterator iter = smLastDetails.begin();
   for ( ; iter != smLastDetails.end(); iter++ )
      (*iter)->update( forceUpdate );

   if ( !sceneBegun )
      GFX->endScene();
}

ConsoleFunction(tsUpdateImposterImages, void, 1, 2, "tsUpdateImposterImages( bool forceupdate )")
{
   if ( argc > 1 )
      TSLastDetail::updateImposterImages( dAtob( argv[1] ) );
   else
      TSLastDetail::updateImposterImages();
}


