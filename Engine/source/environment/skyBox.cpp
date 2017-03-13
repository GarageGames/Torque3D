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
#include "environment/skyBox.h"

#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "scene/sceneRenderState.h"
#include "renderInstance/renderPassManager.h"
#include "gfx/primBuilder.h"
#include "gfx/gfxTransformSaver.h"
#include "core/stream/fileStream.h"
#include "core/stream/bitStream.h"
#include "materials/materialManager.h"
#include "materials/materialFeatureTypes.h"
#include "materials/sceneData.h"
#include "T3D/gameFunctions.h"
#include "renderInstance/renderBinManager.h"
#include "materials/processedMaterial.h"
#include "gfx/gfxDebugEvent.h"
#include "math/util/matrixSet.h"


IMPLEMENT_CO_NETOBJECT_V1( SkyBox );

ConsoleDocClass( SkyBox,
   "@brief Represents the sky with an artist-created cubemap.\n\n"

   "SkyBox is not a directional light and should be used in conjunction with a Sun object.\n\n"

   "@ingroup Atmosphere"
);

SkyBox::SkyBox()
{
   mTypeMask |= EnvironmentObjectType | StaticObjectType;
   mNetFlags.set(Ghostable | ScopeAlways);

   mMatName = "";
   mMatInstance = NULL;

   mIsVBDirty = false;
   mDrawBottom = true;
   mDrawTop = true;
   mExposure = 1.0;
   mPrimCount = 0;
   mFogBandHeight = 0;
   mFogBandWidth = 0.1f;

   mMatrixSet = reinterpret_cast<MatrixSet *>(dMalloc_aligned(sizeof(MatrixSet), 16));
   constructInPlace(mMatrixSet);

   mFogBandMat = NULL;
   mFogBandMatInst = NULL;
}

SkyBox::~SkyBox()
{
   dFree_aligned(mMatrixSet);

   if( mMatInstance )
      SAFE_DELETE( mMatInstance );

   SAFE_DELETE( mFogBandMatInst );

   if ( mFogBandMat )
   {
      mFogBandMat->deleteObject();
      mFogBandMat = NULL;
   }   
}

bool SkyBox::onAdd()
{
   if ( !Parent::onAdd() )
      return false;

   setGlobalBounds();
   resetWorldBox();

   addToScene();

   if ( isClientObject() )
   {
      _initRender();
      _updateMaterial();
   }

   return true;
}

void SkyBox::onRemove()
{
   removeFromScene();
   Parent::onRemove();
}

void SkyBox::initPersistFields()
{
   addGroup( "Sky Box" );	

   addField( "material", TypeMaterialName, Offset( mMatName, SkyBox ), 
      "The name of a cubemap material for the sky box." );

   addField( "drawBottom", TypeBool, Offset( mDrawBottom, SkyBox ),
      "If false the bottom of the skybox is not rendered." );

   addField( "drawTop", TypeBool, Offset( mDrawTop, SkyBox ),
      "If false the top of the skybox is not rendered." );

   addField( "Exposure", TypeF32, Offset( mExposure, SkyBox ),
      "The light exposure for lighting the skybox." );

   addField( "SkyboxColor", TypeColorF, Offset( mSkyboxColor, SkyBox ),
      "Modulates the skybox color. This will blend with the ambient light color ");

   addField( "fogBandHeight", TypeF32, Offset( mFogBandHeight, SkyBox ),
      "The height (0-1) of the fog band from the horizon to the top of the SkyBox." );

   addField( "fogBandWidth", TypeF32, Offset( mFogBandWidth, SkyBox ),
      "The width of the fog band (0-1), added on top of the fog height." );

   endGroup( "Sky Box" );

   Parent::initPersistFields();
}

void SkyBox::inspectPostApply()
{
   Parent::inspectPostApply();
   _updateMaterial();
}

U32 SkyBox::packUpdate( NetConnection *conn, U32 mask, BitStream *stream )
{
   U32 retMask = Parent::packUpdate( conn, mask, stream );
   
   stream->write( mMatName );
   stream->writeFlag( mDrawBottom );
   stream->writeFlag( mDrawTop );
   stream->write( mExposure );
   stream->write( mSkyboxColor );
   stream->write( mFogBandHeight );
   stream->write( mFogBandWidth );

   return retMask;
}

void SkyBox::unpackUpdate( NetConnection *conn, BitStream *stream )
{
   Parent::unpackUpdate( conn, stream );

   String tmpString( "" );
   stream->read( &tmpString );
   if ( !tmpString.equal( mMatName, String::NoCase ) )
   {
      mMatName = tmpString;
      _updateMaterial();
   }

   bool drawBottom = stream->readFlag();
   bool drawTop = stream->readFlag();

   F32 exposure = 0;
   stream->read( &exposure );

   ColorF skyboxColor;
   stream->read( &skyboxColor );

   F32 bandHeight = 0;
   stream->read( &bandHeight );
   F32 bandWidth = 0;
   stream->read( &bandWidth );

   // If this flag has changed
   // we need to update the vertex buffer.
   if (  drawBottom != mDrawBottom || 
         bandHeight != mFogBandHeight ||
         bandWidth != mFogBandWidth ||
		 drawTop != mDrawTop ||
		 exposure != mExposure ||
		 skyboxColor != mSkyboxColor )
   {
      mDrawBottom = drawBottom;
	  mDrawTop = drawTop;
      mFogBandHeight = bandHeight;
      mFogBandWidth = bandWidth;
	  mExposure = exposure;
	  mSkyboxColor = skyboxColor;
      mIsVBDirty = true;
      _initRender();
   }
}

void SkyBox::prepRenderImage( SceneRenderState *state )
{
   PROFILE_SCOPE( SkyBox_prepRenderImage );

   if (  state->isShadowPass() || 
         mVB.isNull() || 
         mFogBandVB.isNull() || 
         !mMatInstance )
      return;

   mMatrixSet->setSceneView(GFX->getWorldMatrix());
   mMatrixSet->setSceneProjection(GFX->getProjectionMatrix());

   ObjectRenderInst *ri = state->getRenderPass()->allocInst<ObjectRenderInst>();
   ri->renderDelegate.bind( this, &SkyBox::_renderObject );
   ri->type = RenderPassManager::RIT_Sky;
   ri->defaultKey = 10;
   ri->defaultKey2 = 2;
   state->getRenderPass()->addInst( ri );
}

void SkyBox::_renderObject( ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance *mi )
{
   GFXDEBUGEVENT_SCOPE( SkyBox_RenderObject, ColorF::WHITE );

   GFXTransformSaver saver;  
   GFX->setVertexBuffer( mVB );         

   MatrixF worldMat = MatrixF::Identity;
   worldMat.setPosition( state->getCameraPosition() );

   SceneData sgData;
   sgData.init( state );
   sgData.objTrans = &worldMat;

   // fixes ambient 
   sgData.ambientLightColor *= mSkyboxColor;
   sgData.ambientLightColor *= mExposure;

   mMatrixSet->restoreSceneViewProjection();
   mMatrixSet->setWorld( worldMat );
   if ( state->isReflectPass() )
      mMatrixSet->setProjection( state->getSceneManager()->getNonClipProjection() );

   while ( mMatInstance->setupPass( state, sgData ) )
   {         
      mMatInstance->setTransforms( *mMatrixSet, state );
      mMatInstance->setSceneInfo( state, sgData );

      GFX->drawPrimitive( GFXTriangleList, 0, mPrimCount );     
   }

   // Draw render band.
   if ( mFogBandHeight > 0 && mFogBandMatInst )
   {
      const FogData &fog = state->getSceneManager()->getFogData();
      if ( mLastFogColor != fog.color )
      {
         mLastFogColor = fog.color;
         _initRender();
      }

      // Just need it to follow the camera... no rotation.
      MatrixF camPosMat( MatrixF::Identity );
      camPosMat.setPosition( worldMat.getPosition() );
      sgData.objTrans = &camPosMat;
      mMatrixSet->setWorld( *sgData.objTrans );

      while ( mFogBandMatInst->setupPass( state, sgData ) )
      {
         mFogBandMatInst->setTransforms( *mMatrixSet, state );
         mFogBandMatInst->setSceneInfo( state, sgData );

         GFX->setVertexBuffer( mFogBandVB );      
         //GFX->drawPrimitive( GFXTriangleList, 0, 16 );
         GFX->drawPrimitive( GFXTriangleList, 0, 96 );
      }
   }
}

void SkyBox::_initRender()
{
   GFXVertexPNT *tmpVerts = NULL;

   U32 vertCount = 36;

   if ( !mDrawBottom )
      vertCount = vertCount - 6;

   if (!mDrawTop)
      vertCount = vertCount - 6;

   mPrimCount = vertCount / 3;

   // Create temp vertex pointer
   // so we can read from it
   // for generating the normals below.
   tmpVerts = new GFXVertexPNT[vertCount];

   // We don't bother sharing
   // vertices here, in order to
   // avoid using a primitive buffer.
   tmpVerts[0].point.set( -1, -1, 1 );
   tmpVerts[1].point.set( 1, -1, 1 );
   tmpVerts[2].point.set( 1, -1, -1 );

   tmpVerts[0].texCoord.set( 0, 0 );
   tmpVerts[1].texCoord.set( 1.0f, 0 );
   tmpVerts[2].texCoord.set( 1.0f, 1.0f );

   tmpVerts[3].point.set( -1, -1, 1 );
   tmpVerts[4].point.set( 1, -1, -1 );
   tmpVerts[5].point.set( -1, -1, -1 );

   tmpVerts[3].texCoord.set( 0, 0 );
   tmpVerts[4].texCoord.set( 1.0f, 1.0f );
   tmpVerts[5].texCoord.set( 0, 1.0f );

   tmpVerts[6].point.set( 1, -1, 1 );
   tmpVerts[7].point.set( 1, 1, 1 );
   tmpVerts[8].point.set( 1, 1, -1 );

   tmpVerts[6].texCoord.set( 0, 0 );
   tmpVerts[7].texCoord.set( 1.0f, 0 );
   tmpVerts[8].texCoord.set( 1.0f, 1.0f );

   tmpVerts[9].point.set( 1, -1, 1 );
   tmpVerts[10].point.set( 1, 1, -1 );
   tmpVerts[11].point.set( 1, -1, -1 );

   tmpVerts[9].texCoord.set( 0, 0 );
   tmpVerts[10].texCoord.set( 1.0f, 1.0f );
   tmpVerts[11].texCoord.set( 0, 1.0f );

   tmpVerts[12].point.set( -1, 1, 1 );
   tmpVerts[13].point.set( -1, -1, 1 );
   tmpVerts[14].point.set( -1, -1, -1 );

   tmpVerts[12].texCoord.set( 0, 0 );
   tmpVerts[13].texCoord.set( 1.0f, 0 );
   tmpVerts[14].texCoord.set( 1.0f, 1.0f );

   tmpVerts[15].point.set( -1, 1, 1 );
   tmpVerts[16].point.set( -1, -1, -1 ); 
   tmpVerts[17].point.set( -1, 1, -1 );

   tmpVerts[15].texCoord.set( 0, 0 );
   tmpVerts[16].texCoord.set( 1.0f, 1.0f ); 
   tmpVerts[17].texCoord.set( 1.0f, 0 );

   tmpVerts[18].point.set( 1, 1, 1 );
   tmpVerts[19].point.set( -1, 1, 1 );
   tmpVerts[20].point.set( -1, 1, -1 );

   tmpVerts[18].texCoord.set( 0, 0 );
   tmpVerts[19].texCoord.set( 1.0f, 0 );
   tmpVerts[20].texCoord.set( 1.0f, 1.0f );

   tmpVerts[21].point.set( 1, 1, 1 );
   tmpVerts[22].point.set( -1, 1, -1 );
   tmpVerts[23].point.set( 1, 1, -1 );

   tmpVerts[21].texCoord.set( 0, 0 );
   tmpVerts[22].texCoord.set( 1.0f, 1.0f );
   tmpVerts[23].texCoord.set( 0, 1.0f );

   if ( mDrawTop )
   {
		// rendering the top face	
	   tmpVerts[24].point.set( -1, -1, 1 );
	   tmpVerts[25].point.set( -1, 1, 1 );
	   tmpVerts[26].point.set( 1, 1, 1 );

	   tmpVerts[24].texCoord.set( 0, 0 );
	   tmpVerts[25].texCoord.set( 1.0f, 0 );
	   tmpVerts[26].texCoord.set( 1.0f, 1.0f );

	   tmpVerts[27].point.set( -1, -1, 1 );
	   tmpVerts[28].point.set( 1, 1, 1 );
	   tmpVerts[29].point.set( 1, -1, 1 );

	   tmpVerts[27].texCoord.set( 0, 0 );
	   tmpVerts[28].texCoord.set( 1.0f, 1.0f );
	   tmpVerts[29].texCoord.set( 0, 1.0f );

	   if ( mDrawBottom )  {
			// we are rendering top and bottom faces
		  tmpVerts[30].point.set( 1, 1, -1 ); 
		  tmpVerts[31].point.set( -1, 1, -1 );
		  tmpVerts[32].point.set( -1, -1, -1 );

		  tmpVerts[30].texCoord.set( 1.0f, 1.0f ); 
		  tmpVerts[31].texCoord.set( 1.0f, 0 );
		  tmpVerts[32].texCoord.set( 0, 0 );

		  tmpVerts[33].point.set( 1, -1, -1 ); 
		  tmpVerts[34].point.set( 1, 1, -1 );
		  tmpVerts[35].point.set( -1, -1, -1 );

		  tmpVerts[33].texCoord.set( 0, 1.0f ); 
		  tmpVerts[34].texCoord.set( 1.0f, 1.0f );
		  tmpVerts[35].texCoord.set( 0, 0 );
	   } 
   }

   // Only set up these vertices if the SkyBox
   // is set to render the bottom face and not the top
   if ( mDrawBottom && !mDrawTop)  {
      tmpVerts[24].point.set( 1, 1, -1 ); 
      tmpVerts[25].point.set( -1, 1, -1 );
      tmpVerts[26].point.set( -1, -1, -1 );

      tmpVerts[24].texCoord.set( 1.0f, 1.0f ); 
      tmpVerts[25].texCoord.set( 1.0f, 0 );
      tmpVerts[26].texCoord.set( 0, 0 );

      tmpVerts[27].point.set( 1, -1, -1 ); 
      tmpVerts[28].point.set( 1, 1, -1 );
      tmpVerts[29].point.set( -1, -1, -1 );

      tmpVerts[27].texCoord.set( 0, 1.0f ); 
      tmpVerts[28].texCoord.set( 1.0f, 1.0f );
      tmpVerts[29].texCoord.set( 0, 0 );
   }

   VectorF tmp( 0, 0, 0 );

   for ( U32 i = 0; i < vertCount; i++ )
   {
      //tmp = tmpVerts[i].point;
      //tmp.normalize();
      //tmpVerts[i].normal.set( tmp );

      // Note: SkyBox renders with a regular material, which uses the "Reflect Cube"
      // feature. 
      //
      // This feature is really designed a cubemap representing a reflection
      // on an objects surface and therefore looks up into the cubemap with the 
      // cubemap-space view vector reflected by the vert normal. 
      //
      // Since we are actually viewing the skybox from "inside" not from 
      // "outside" this reflection ends up making the cubemap appear upsidown. 
      // Therefore we set the vert-normals to "zero" so that the reflection 
      // operation returns the input, unreflected, vector.

      tmpVerts[i].normal.set( Point3F::Zero );
   }

   if ( mVB.isNull() || mIsVBDirty )
   {
      mVB.set( GFX, vertCount, GFXBufferTypeStatic );
      mIsVBDirty = false;
   }

   GFXVertexPNT *vertPtr = mVB.lock();
   if (!vertPtr)
   {
      delete[] tmpVerts;
      return;
   }

   dMemcpy(vertPtr, tmpVerts, sizeof( GFXVertexPNT) * vertCount);

   mVB.unlock();

   // Clean up temp verts.
   delete [] tmpVerts;

   if ( mFogBandVB.isNull() )
      mFogBandVB.set( GFX, 288, GFXBufferTypeStatic );
      //mFogBandVB.set( GFX, 192, GFXBufferTypeStatic );
      //mFogBandVB.set( GFX, 48, GFXBufferTypeStatic );

   GFXVertexPC *bandVertPtr = mFogBandVB.lock();
   if(!bandVertPtr) return;

   // Grab the fog color.
   ColorI fogColor( mLastFogColor.red * 255, mLastFogColor.green * 255, mLastFogColor.blue * 255 );
   ColorI fogColorAlpha( mLastFogColor.red * 255, mLastFogColor.green * 255, mLastFogColor.blue * 255, 20 );
   ColorI fogColorAlphaFaint( mLastFogColor.red * 255, mLastFogColor.green * 255, mLastFogColor.blue * 255, 0 );
   ColorI fogColorPink( 255, 128, 128 );

   F32 fogBase = -1.0f + mFogBandHeight;
   F32 totalFogHeight = mFogBandHeight + mFogBandWidth - 1.0f;
	if (totalFogHeight > 1.0)
		totalFogHeight = 1.0;
	F32 degree = M_PI_F / 8.0f;
	int pointNum = 0;

	for (int i=0;i<16;i++) {
		F32 x1 = sin(degree * F32(i));
		F32 y1 = cos(degree * F32(i));
		F32 x2 = sin(degree * (F32(i) + 1.0f));
		F32 y2 = cos(degree * (F32(i) + 1.0f));
		pointNum = i * 18;

	   // very faint uppder portion
      bandVertPtr[pointNum].point.set( x1, y1, totalFogHeight + mFogBandWidth );
      bandVertPtr[pointNum+1].point.set( x2, y2, totalFogHeight + mFogBandWidth);
      bandVertPtr[pointNum+2].point.set( x1, y1, totalFogHeight);

      bandVertPtr[pointNum].color.set( fogColorAlphaFaint );
      bandVertPtr[pointNum+1].color.set( fogColorAlphaFaint );
      bandVertPtr[pointNum+2].color.set( fogColorAlpha );

      bandVertPtr[pointNum+3].point.set( x2, y2, totalFogHeight + mFogBandWidth );
      bandVertPtr[pointNum+4].point.set( x2, y2, totalFogHeight );
      bandVertPtr[pointNum+5].point.set( x1, y1, totalFogHeight );

      bandVertPtr[pointNum+3].color.set( fogColorAlphaFaint );
      bandVertPtr[pointNum+4].color.set( fogColorAlpha );
      bandVertPtr[pointNum+5].color.set( fogColorAlpha );

	   // middle portion
      bandVertPtr[pointNum+6].point.set( x1, y1, totalFogHeight );
      bandVertPtr[pointNum+7].point.set( x2, y2, totalFogHeight );
      bandVertPtr[pointNum+8].point.set( x1, y1, fogBase );

      bandVertPtr[pointNum+6].color.set( fogColorAlpha );
      bandVertPtr[pointNum+7].color.set( fogColorAlpha );
      bandVertPtr[pointNum+8].color.set( fogColor );

      bandVertPtr[pointNum+9].point.set( x2, y2, totalFogHeight );
      bandVertPtr[pointNum+10].point.set( x2, y2, fogBase );
      bandVertPtr[pointNum+11].point.set( x1, y1, fogBase );

      bandVertPtr[pointNum+9].color.set( fogColorAlpha );
      bandVertPtr[pointNum+10].color.set( fogColor );
      bandVertPtr[pointNum+11].color.set( fogColor );


	  // lower portion
      bandVertPtr[pointNum+12].point.set( x1, y1, -1 );
      bandVertPtr[pointNum+13].point.set( x1, y1, fogBase );
      bandVertPtr[pointNum+14].point.set( x2, y2, -1 );

      bandVertPtr[pointNum+12].color.set( fogColor );
      bandVertPtr[pointNum+13].color.set( fogColor );
      bandVertPtr[pointNum+14].color.set( fogColor );

      bandVertPtr[pointNum+15].point.set( x1, y1, fogBase );
      bandVertPtr[pointNum+16].point.set( x2, y2, fogBase );
      bandVertPtr[pointNum+17].point.set( x2, y2, -1 );

      bandVertPtr[pointNum+15].color.set( fogColor );
      bandVertPtr[pointNum+16].color.set( fogColor );
      bandVertPtr[pointNum+17].color.set( fogColor );
   }

   mFogBandVB.unlock();

   SAFE_DELETE( mFogBandMatInst );
   if ( mFogBandMat )
   {
      mFogBandMat->deleteObject();
      mFogBandMat = NULL;
   }

   // Setup the material for this imposter.
   mFogBandMat = MATMGR->allocateAndRegister( String::EmptyString );
   mFogBandMat->mAutoGenerated = true;   
   mFogBandMat->mTranslucent = true;   
   mFogBandMat->mVertColor[0] = true;
   mFogBandMat->mDoubleSided = true;
   mFogBandMat->mEmissive[0] = true;

   mFogBandMatInst = mFogBandMat->createMatInstance();
   mFogBandMatInst->init( MATMGR->getDefaultFeatures(), getGFXVertexFormat<GFXVertexPC>() );
}

void SkyBox::onStaticModified( const char *slotName, const char *newValue )
{
   Parent::onStaticModified( slotName, newValue );

   if ( dStricmp( slotName, "material" ) == 0 )
      setMaskBits( 0xFFFFFFFF );
}

void SkyBox::_initMaterial()
{
   if ( mMatInstance )
      SAFE_DELETE( mMatInstance );

   if ( mMaterial )
      mMatInstance = mMaterial->createMatInstance();
   else
      mMatInstance = MATMGR->createMatInstance( "WarningMaterial" );

   // We want to disable culling and z write.
   GFXStateBlockDesc desc;
   desc.setCullMode( GFXCullNone );
   desc.setBlend( true );
   desc.setZReadWrite( true, false );
   mMatInstance->addStateBlockDesc( desc );

   // Allow lighting on the skybox material by default.
   FeatureSet features = MATMGR->getDefaultFeatures();
   features.removeFeature( MFT_Visibility );
   features.addFeature(MFT_SkyBox);

   // Now initialize the material.
   mMatInstance->init(features, getGFXVertexFormat<GFXVertexPNT>());
}

void SkyBox::_updateMaterial()
{
   if ( mMatName.isEmpty() )
      return;

   Material *pMat = NULL;
   if ( !Sim::findObject( mMatName, pMat ) )
      Con::printf( "SkyBox::_updateMaterial, failed to find Material of name %s!", mMatName.c_str() );
   else if ( isProperlyAdded() )
   {
      mMaterial = pMat;
      _initMaterial(); 
   }
}

BaseMatInstance* SkyBox::_getMaterialInstance()
{
   if ( !mMaterial || !mMatInstance || mMatInstance->getMaterial() != mMaterial )
      _initMaterial();

   if ( !mMatInstance )
      return NULL;

   return mMatInstance;
}

DefineConsoleMethod( SkyBox, postApply, void, (), , "")
{
	object->inspectPostApply();
}