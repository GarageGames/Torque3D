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
#include "platform/profiler.h"
#include "console/consoleTypes.h"
#include "cloudLayer.h"

#include "gfx/gfxTransformSaver.h"
#include "gfx/gfxTextureManager.h"
#include "core/stream/fileStream.h"
#include "core/stream/bitStream.h"
#include "scene/sceneRenderState.h"
#include "renderInstance/renderPassManager.h"
#include "gfx/primBuilder.h"
#include "materials/materialManager.h"
#include "materials/customMaterialDefinition.h"
#include "materials/shaderData.h"
#include "lighting/lightInfo.h"
#include "math/mathIO.h"

ConsoleDocClass( CloudLayer,
   "@brief A layer of clouds which change shape over time and are affected by scene lighting.\n\n"

   "%CloudLayer always renders overhead, following the camera. It is intended "
   "as part of the background of your level, rendering in front of Sky/Sun "
   "type objects and behind everything else.\n\n"

   "The illusion of clouds forming and changing over time is controlled by the "
   "normal/opacity texture and the three sets of texture animation parameters. "
   "The texture is sampled three times.  The first sample defines overall cloud "
   "density, where clouds are likely to form and their general size and shape. "
   "The second two samples control how it changes over time; they are "
   "combined and used as modifiers to the first sample.\n\n"

   "%CloudLayer is affected by scene lighting and is designed to be used in "
   "scenes with dynamic lighting or time of day changes.\n\n"

   "@ingroup Atmosphere"
);

GFXImplementVertexFormat( GFXCloudVertex )
{
   addElement( "POSITION", GFXDeclType_Float3 );
   addElement( "NORMAL", GFXDeclType_Float3 );
   addElement( "BINORMAL", GFXDeclType_Float3 );
   addElement( "TANGENT", GFXDeclType_Float3 );   
   addElement( "TEXCOORD", GFXDeclType_Float2, 0 );   
}

U32 CloudLayer::smVertStride = 50;
U32 CloudLayer::smStrideMinusOne = smVertStride - 1;
U32 CloudLayer::smVertCount = smVertStride * smVertStride;
U32 CloudLayer::smTriangleCount = smStrideMinusOne * smStrideMinusOne * 2;

CloudLayer::CloudLayer()
: mLastTime( 0 ),
  mBaseColor( 0.9f, 0.9f, 0.9f, 1.0f ),
  mExposure( 1.0f ),
  mCoverage( 0.5f ),
  mWindSpeed( 1.0f )
{
   mTypeMask |= EnvironmentObjectType | StaticObjectType;
   mNetFlags.set(Ghostable | ScopeAlways);

   mModelViewProjSC =
   mAmbientColorSC =
   mSunColorSC =
   mSunVecSC =
   mTexScaleSC =
   mBaseColorSC =
   mCoverageSC =
   mExposureSC =
   mEyePosWorldSC = 0;

   mTexOffsetSC[0] = mTexOffsetSC[1] = mTexOffsetSC[2] = 0;

   mTexScale[0] = 1.0;
   mTexScale[1] = 1.0;
   mTexScale[2] = 1.0;

   mTexDirection[0].set( 1.0f, 0.0f );
   mTexDirection[1].set( 0.0f, 1.0f );
   mTexDirection[2].set( 0.5f, 0.0f );

   mTexSpeed[0] = 0.005f;
   mTexSpeed[1] = 0.005f;
   mTexSpeed[2] = 0.005f;

   mTexOffset[0] = mTexOffset[1] = mTexOffset[2] = Point2F::Zero;

   mHeight = 4.0f;
}

IMPLEMENT_CO_NETOBJECT_V1( CloudLayer );

// ConsoleObject...


bool CloudLayer::onAdd()
{
   if ( !Parent::onAdd() )
      return false;

   setGlobalBounds();
   resetWorldBox();

   addToScene();

   if ( isClientObject() )
   {
      _initTexture();
      _initBuffers();

      // Find ShaderData
      ShaderData *shaderData;
      mShader = Sim::findObject( "CloudLayerShader", shaderData ) ? 
                  shaderData->getShader() : NULL;
      if ( !mShader )
      {
         Con::errorf( "CloudLayer::onAdd - could not find CloudLayerShader" );
         return false;
      }

      // Create ShaderConstBuffer and Handles
      mShaderConsts = mShader->allocConstBuffer();
      mModelViewProjSC = mShader->getShaderConstHandle( "$modelView" );
      mEyePosWorldSC = mShader->getShaderConstHandle( "$eyePosWorld" );
      mSunVecSC = mShader->getShaderConstHandle( "$sunVec" );
      mTexOffsetSC[0] = mShader->getShaderConstHandle( "$texOffset0" );
      mTexOffsetSC[1] = mShader->getShaderConstHandle( "$texOffset1" );
      mTexOffsetSC[2] = mShader->getShaderConstHandle( "$texOffset2" );
      mTexScaleSC = mShader->getShaderConstHandle( "$texScale" );
      mAmbientColorSC = mShader->getShaderConstHandle( "$ambientColor" );
      mSunColorSC = mShader->getShaderConstHandle( "$sunColor" );
      mCoverageSC = mShader->getShaderConstHandle( "$cloudCoverage" );
      mExposureSC = mShader->getShaderConstHandle( "$cloudExposure" );
      mBaseColorSC = mShader->getShaderConstHandle( "$cloudBaseColor" );
      mNormalHeightMapSC = mShader->getShaderConstHandle( "$normalHeightMap" );

      // Create StateBlocks
      GFXStateBlockDesc desc;
      desc.setCullMode( GFXCullNone );
      desc.setBlend( true );
      desc.setZReadWrite( true, false );
      desc.samplersDefined = true;
      desc.samplers[0].addressModeU = GFXAddressWrap;
      desc.samplers[0].addressModeV = GFXAddressWrap;
      desc.samplers[0].addressModeW = GFXAddressWrap;
      desc.samplers[0].magFilter = GFXTextureFilterLinear;
      desc.samplers[0].minFilter = GFXTextureFilterLinear;
      desc.samplers[0].mipFilter = GFXTextureFilterLinear;
      desc.samplers[0].textureColorOp = GFXTOPModulate;

      mStateblock = GFX->createStateBlock( desc );   
   }

   return true;
}

void CloudLayer::onRemove()
{
   removeFromScene();

   Parent::onRemove();
}

void CloudLayer::initPersistFields()
{
   addGroup( "CloudLayer" );	   
      
      addField( "texture", TypeImageFilename, Offset( mTextureName, CloudLayer ),
         "An RGBA texture which should contain normals and opacity (density)." );

      addArray( "Textures", TEX_COUNT );

         addField( "texScale", TypeF32, Offset( mTexScale, CloudLayer ), TEX_COUNT,
            "Controls the texture repeat of this slot." );

         addField( "texDirection", TypePoint2F, Offset( mTexDirection, CloudLayer ), TEX_COUNT,
            "Controls the direction this slot scrolls." );

         addField( "texSpeed", TypeF32, Offset( mTexSpeed, CloudLayer ), TEX_COUNT,
            "Controls the speed this slot scrolls." );

      endArray( "Textures" );

      addField( "baseColor", TypeColorF, Offset( mBaseColor, CloudLayer ),
         "Base cloud color before lighting." );

      addField( "exposure", TypeF32, Offset( mExposure, CloudLayer ),
         "Brightness scale so CloudLayer can be overblown if desired." );
      
      addField( "coverage", TypeF32, Offset( mCoverage, CloudLayer ),
         "Fraction of sky covered by clouds 0-1." );

      addField( "windSpeed", TypeF32, Offset( mWindSpeed, CloudLayer ),
         "Overall scalar to texture scroll speed." );

      addField( "height", TypeF32, Offset( mHeight, CloudLayer ),
         "Abstract number which controls the curvature and height of the dome mesh." );

   endGroup( "CloudLayer" );

   Parent::initPersistFields();
}

void CloudLayer::inspectPostApply()
{
   Parent::inspectPostApply();
   setMaskBits( CloudLayerMask );
}


// NetObject...


U32 CloudLayer::packUpdate( NetConnection *conn, U32 mask, BitStream *stream )
{
   U32 retMask = Parent::packUpdate( conn, mask, stream );

   stream->write( mTextureName );
   
   for ( U32 i = 0; i < TEX_COUNT; i++ )
   {
      stream->write( mTexScale[i] );      
      stream->write( mTexSpeed[i] );
      mathWrite( *stream, mTexDirection[i] );
   }

   stream->write( mBaseColor );
   stream->write( mCoverage );
   stream->write( mExposure );
   stream->write( mWindSpeed );
   stream->write( mHeight );

   return retMask;
}

void CloudLayer::unpackUpdate( NetConnection *conn, BitStream *stream )
{
   Parent::unpackUpdate( conn, stream );

   String oldTextureName = mTextureName;
   stream->read( &mTextureName );

   for ( U32 i = 0; i < TEX_COUNT; i++ )
   {
      stream->read( &mTexScale[i] );      
      stream->read( &mTexSpeed[i] );
      mathRead( *stream, &mTexDirection[i] );
   }

   stream->read( &mBaseColor );

   F32 oldCoverage = mCoverage;
   stream->read( &mCoverage );
   stream->read( &mExposure );

   stream->read( &mWindSpeed );

   F32 oldHeight = mHeight;
   stream->read( &mHeight );

   if ( isProperlyAdded() )
   {
      if ( ( oldTextureName != mTextureName ) || ( ( oldCoverage == 0.0f ) != ( mCoverage == 0.0f ) ) )
         _initTexture();
      if ( oldHeight != mHeight )
         _initBuffers();
   }
}


// SceneObject...


void CloudLayer::prepRenderImage( SceneRenderState *state )
{
   PROFILE_SCOPE( CloudLayer_prepRenderImage );

   if ( mCoverage <= 0.0f )
      return;

   if ( state->isDiffusePass() )
   {
      // Scroll textures...

      U32 time = Sim::getCurrentTime();
      F32 delta = (F32)( time - mLastTime ) / 1000.0f;
      mLastTime = time;

      for ( U32 i = 0; i < 3; i++ )
      {
         mTexOffset[i] += mTexDirection[i] * mTexSpeed[i] * delta * mWindSpeed;
      }
   }

   // This should be sufficient for most objects that don't manage zones, and
   // don't need to return a specialized RenderImage...

   ObjectRenderInst *ri = state->getRenderPass()->allocInst<ObjectRenderInst>();
   ri->renderDelegate.bind( this, &CloudLayer::renderObject );
   ri->type = RenderPassManager::RIT_Sky;
   ri->defaultKey = 0;
   ri->defaultKey2 = 0;
   state->getRenderPass()->addInst( ri );
}

void CloudLayer::renderObject( ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance *mi )
{
   GFXTransformSaver saver;

   const Point3F &camPos = state->getCameraPosition();
   MatrixF xfm(true);
   xfm.setPosition(camPos);
   GFX->multWorld(xfm);   

   if ( state->isReflectPass() )
      GFX->setProjectionMatrix( state->getSceneManager()->getNonClipProjection() );

   GFX->setShader( mShader );
   GFX->setShaderConstBuffer( mShaderConsts );
   GFX->setStateBlock( mStateblock );

   // Set all the shader consts...

   MatrixF xform(GFX->getProjectionMatrix());
   xform *= GFX->getViewMatrix();
   xform *= GFX->getWorldMatrix();

   mShaderConsts->setSafe( mModelViewProjSC, xform );

   mShaderConsts->setSafe( mEyePosWorldSC, camPos );

   LightInfo *lightinfo = LIGHTMGR->getSpecialLight(LightManager::slSunLightType);
   const ColorF &sunlight = state->getAmbientLightColor();

   Point3F ambientColor( sunlight.red, sunlight.green, sunlight.blue );
   mShaderConsts->setSafe( mAmbientColorSC, ambientColor );   

   const ColorF &sunColor = lightinfo->getColor();
   Point3F data( sunColor.red, sunColor.green, sunColor.blue );
   mShaderConsts->setSafe( mSunColorSC, data );

   mShaderConsts->setSafe( mSunVecSC, lightinfo->getDirection() );

   for ( U32 i = 0; i < TEX_COUNT; i++ )         
      mShaderConsts->setSafe( mTexOffsetSC[i], mTexOffset[i] );

   Point3F scale( mTexScale[0], mTexScale[1], mTexScale[2] );
   mShaderConsts->setSafe( mTexScaleSC, scale );

   Point3F color;
   color.set( mBaseColor.red, mBaseColor.green, mBaseColor.blue );
   mShaderConsts->setSafe( mBaseColorSC, color );

   mShaderConsts->setSafe( mCoverageSC, mCoverage );

   mShaderConsts->setSafe( mExposureSC, mExposure );

   GFX->setTexture( mNormalHeightMapSC->getSamplerRegister(), mTexture );                            
   GFX->setVertexBuffer( mVB );            
   GFX->setPrimitiveBuffer( mPB );

   GFX->drawIndexedPrimitive( GFXTriangleList, 0, 0, smVertCount, 0, smTriangleCount );
}


// CloudLayer Internal Methods....


void CloudLayer::_initTexture()
{
   if ( mCoverage <= 0.0f )
   {
      mTexture = NULL;
      return;
   }

   if ( mTextureName.isNotEmpty() )
      mTexture.set( mTextureName, &GFXDefaultStaticDiffuseProfile, "CloudLayer" );

   if ( mTexture.isNull() )
      mTexture.set( GFXTextureManager::getWarningTexturePath(), &GFXDefaultStaticDiffuseProfile, "CloudLayer" );
}

void CloudLayer::_initBuffers()
{      
   // Vertex Buffer...

   Point3F vertScale( 16.0f, 16.0f, mHeight );
   F32 zOffset = -( mCos( mSqrt( 1.0f ) ) + 0.01f );
   
   mVB.set( GFX, smVertCount, GFXBufferTypeStatic );   
   GFXCloudVertex *pVert = mVB.lock(); 
   if(!pVert) return;

   for ( U32 y = 0; y < smVertStride; y++ )
   {
      F32 v = ( (F32)y / (F32)smStrideMinusOne - 0.5f ) * 2.0f;

      for ( U32 x = 0; x < smVertStride; x++ )
      {
         F32 u = ( (F32)x / (F32)smStrideMinusOne - 0.5f ) * 2.0f;

         F32 sx = u;
         F32 sy = v;
         F32 sz = mCos( mSqrt( sx*sx + sy*sy ) ) + zOffset;
         //F32 sz = 1.0f;
         pVert->point.set( sx, sy, sz );
         pVert->point *= vertScale;

         // The vert to our right.
         Point3F rpnt;

         F32 ru = ( (F32)( x + 1 ) / (F32)smStrideMinusOne - 0.5f ) * 2.0f;
         F32 rv = v;

         rpnt.x = ru;
         rpnt.y = rv;
         rpnt.z = mCos( mSqrt( rpnt.x*rpnt.x + rpnt.y*rpnt.y ) ) + zOffset;
         rpnt *= vertScale;

         // The vert to our front.
         Point3F fpnt;

         F32 fu = u;
         F32 fv = ( (F32)( y + 1 ) / (F32)smStrideMinusOne - 0.5f ) * 2.0f;

         fpnt.x = fu;
         fpnt.y = fv;
         fpnt.z = mCos( mSqrt( fpnt.x*fpnt.x + fpnt.y*fpnt.y ) ) + zOffset;
         fpnt *= vertScale;

         Point3F fvec = fpnt - pVert->point;
         fvec.normalize();

         Point3F rvec = rpnt - pVert->point;
         rvec.normalize();

         pVert->normal = mCross( fvec, rvec );
         pVert->normal.normalize();
         pVert->binormal = fvec;
         pVert->tangent = rvec;
         pVert->texCoord.set( u, v );   
         pVert++;
      }
   }

   mVB.unlock();


   // Primitive Buffer...   

   mPB.set( GFX, smTriangleCount * 3, smTriangleCount, GFXBufferTypeStatic );

   U16 *pIdx = NULL;   
   mPB.lock(&pIdx);     
   U32 curIdx = 0; 

   for ( U32 y = 0; y < smStrideMinusOne; y++ )
   {
      for ( U32 x = 0; x < smStrideMinusOne; x++ )
      {
         U32 offset = x + y * smVertStride;

         pIdx[curIdx] = offset;
         curIdx++;
         pIdx[curIdx] = offset + 1;
         curIdx++;
         pIdx[curIdx] = offset + smVertStride + 1;
         curIdx++;

         pIdx[curIdx] = offset;
         curIdx++;
         pIdx[curIdx] = offset + smVertStride + 1;
         curIdx++;
         pIdx[curIdx] = offset + smVertStride;
         curIdx++;
      }
   }

   mPB.unlock();   
}