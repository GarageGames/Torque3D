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
#include "environment/sun.h"

#include "gfx/bitmap/gBitmap.h"
#include "math/mathIO.h"
#include "core/stream/bitStream.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "scene/sceneManager.h"
#include "math/mathUtils.h"
#include "lighting/lightInfo.h"
#include "lighting/lightManager.h"
#include "scene/sceneRenderState.h"
#include "renderInstance/renderPassManager.h"
#include "sim/netConnection.h"
#include "environment/timeOfDay.h"
#include "gfx/gfxTransformSaver.h"
#include "materials/materialManager.h"
#include "materials/baseMatInstance.h"
#include "materials/sceneData.h"
#include "math/util/matrixSet.h"


IMPLEMENT_CO_NETOBJECT_V1(Sun);

ConsoleDocClass( Sun,
   "@brief A global light affecting your entire scene and optionally renders a corona effect.\n\n"

   "Sun is both the directional and ambient light for your entire scene.\n\n"   

   "@ingroup Atmosphere"
);

//-----------------------------------------------------------------------------

Sun::Sun()
{
   mNetFlags.set(Ghostable | ScopeAlways);
   mTypeMask = EnvironmentObjectType | LightObjectType | StaticObjectType;

   mLightColor.set(0.7f, 0.7f, 0.7f);
   mLightAmbient.set(0.3f, 0.3f, 0.3f);
   mBrightness = 1.0f;
   mSunAzimuth = 0.0f;
   mSunElevation = 35.0f;
   mCastShadows = true;
   mStaticRefreshFreq = 250;
   mDynamicRefreshFreq = 8;

   mAnimateSun = false;
   mTotalTime = 0.0f;
   mCurrTime = 0.0f;
   mStartAzimuth = 0.0f;
   mEndAzimuth = 0.0f;
   mStartElevation = 0.0f;
   mEndElevation = 0.0f;

   mLight = LightManager::createLightInfo();
   mLight->setType( LightInfo::Vector );

   mFlareData = NULL;
   mFlareState.clear();
   mFlareScale = 1.0f;

   mCoronaEnabled = true;
   mCoronaScale = 0.5f;
   mCoronaTint.set( 1.0f, 1.0f, 1.0f, 1.0f );
   mCoronaUseLightColor = true;
   mCoronaMatInst = NULL;

   mMatrixSet = reinterpret_cast<MatrixSet *>(dMalloc_aligned(sizeof(MatrixSet), 16));
   constructInPlace(mMatrixSet);

   mCoronaWorldRadius = 0.0f;
   mLightWorldPos = Point3F::Zero;
}

Sun::~Sun()
{
   SAFE_DELETE( mLight );
   SAFE_DELETE( mCoronaMatInst );
   dFree_aligned(mMatrixSet);
}

bool Sun::onAdd()
{
   if ( !Parent::onAdd() )
      return false;

   // Register as listener to TimeOfDay update events
   TimeOfDay::getTimeOfDayUpdateSignal().notify( this, &Sun::_updateTimeOfDay );

	// Make this thing have a global bounds so that its 
   // always returned from spatial light queries.
	setGlobalBounds();
	resetWorldBox();
	setRenderTransform( mObjToWorld );
	addToScene();

   _initCorona();

   // Update the light parameters.
   _conformLights();

   setProcessTick( true );

   return true;
}

void Sun::onRemove()
{   
   TimeOfDay::getTimeOfDayUpdateSignal().remove( this, &Sun::_updateTimeOfDay );

   removeFromScene();
   Parent::onRemove();
}

void Sun::initPersistFields()
{
   addGroup( "Orbit" );

      addField( "azimuth", TypeF32, Offset( mSunAzimuth, Sun ), 
         "The horizontal angle of the sun measured clockwise from the positive Y world axis." );

      addField( "elevation", TypeF32, Offset( mSunElevation, Sun ),
         "The elevation angle of the sun above or below the horizon." );

   endGroup( "Orbit" );	

   // We only add the basic lighting options that all lighting
   // systems would use... the specific lighting system options
   // are injected at runtime by the lighting system itself.

   addGroup( "Lighting" );

      addField( "color", TypeColorF, Offset( mLightColor, Sun ), 
         "Color shading applied to surfaces in direct contact with light source.");

      addField( "ambient", TypeColorF, Offset( mLightAmbient, Sun ), "Color shading applied to surfaces not "
         "in direct contact with light source, such as in the shadows or interiors.");       

      addField( "brightness", TypeF32, Offset( mBrightness, Sun ), 
         "Adjust the Sun's global contrast/intensity");      

      addField( "castShadows", TypeBool, Offset( mCastShadows, Sun ), 
         "Enables/disables shadows cast by objects due to Sun light");    

      addField("staticRefreshFreq", TypeS32, Offset(mStaticRefreshFreq, Sun), "static shadow refresh rate (milliseconds)");
      addField("dynamicRefreshFreq", TypeS32, Offset(mDynamicRefreshFreq, Sun), "dynamic shadow refresh rate (milliseconds)");

   endGroup( "Lighting" );

   addGroup( "Corona" );

      addField( "coronaEnabled", TypeBool, Offset( mCoronaEnabled, Sun ), 
         "Enable or disable rendering of the corona sprite." );

      addField( "coronaMaterial", TypeMaterialName, Offset( mCoronaMatName, Sun ),
         "Texture for the corona sprite." );

      addField( "coronaScale", TypeF32, Offset( mCoronaScale, Sun ),
         "Controls size the corona sprite renders, specified as a fractional amount of the screen height." );

      addField( "coronaTint", TypeColorF, Offset( mCoronaTint, Sun ),
         "Modulates the corona sprite color ( if coronaUseLightColor is false )." );

      addField( "coronaUseLightColor", TypeBool, Offset( mCoronaUseLightColor, Sun ),
         "Modulate the corona sprite color by the color of the light ( overrides coronaTint )." );

   endGroup( "Corona" );


   addGroup( "Misc" );

      addField( "flareType", TYPEID< LightFlareData >(), Offset( mFlareData, Sun ), 
         "Datablock for the flare produced by the Sun" );

      addField( "flareScale", TypeF32, Offset( mFlareScale, Sun ), 
         "Changes the size and intensity of the flare." );

   endGroup( "Misc" );

   // Now inject any light manager specific fields.
   LightManager::initLightFields();

   Parent::initPersistFields();
}

void Sun::inspectPostApply()
{
   _conformLights();
   setMaskBits(UpdateMask);
}

U32 Sun::packUpdate(NetConnection *conn, U32 mask, BitStream *stream )
{
   U32 retMask = Parent::packUpdate( conn, mask, stream );

   if ( stream->writeFlag( mask & UpdateMask ) )
   {
      stream->write( mSunAzimuth );
      stream->write( mSunElevation );
      stream->write( mLightColor );
      stream->write( mLightAmbient );
      stream->write( mBrightness );      
      stream->writeFlag( mCastShadows ); 
      stream->write(mStaticRefreshFreq);
      stream->write(mDynamicRefreshFreq);
      stream->write( mFlareScale );

      if ( stream->writeFlag( mFlareData ) )
      {
         stream->writeRangedU32( mFlareData->getId(),
            DataBlockObjectIdFirst, 
            DataBlockObjectIdLast );
      }

      stream->writeFlag( mCoronaEnabled );
      stream->write( mCoronaMatName );
      stream->write( mCoronaScale );
      stream->write( mCoronaTint );
      stream->writeFlag( mCoronaUseLightColor );

      mLight->packExtended( stream ); 
   }

   return retMask;
}

void Sun::unpackUpdate( NetConnection *conn, BitStream *stream )
{
   Parent::unpackUpdate( conn, stream );

   if ( stream->readFlag() ) // UpdateMask
   {
      stream->read( &mSunAzimuth );
      stream->read( &mSunElevation );
      stream->read( &mLightColor );
      stream->read( &mLightAmbient );
      stream->read( &mBrightness );      
      mCastShadows = stream->readFlag();
      stream->read(&mStaticRefreshFreq);
      stream->read(&mDynamicRefreshFreq);
      stream->read( &mFlareScale );

      if ( stream->readFlag() )
      {
         SimObjectId id = stream->readRangedU32( DataBlockObjectIdFirst, DataBlockObjectIdLast );  
         LightFlareData *datablock = NULL;

         if ( Sim::findObject( id, datablock ) )
            mFlareData = datablock;
         else
         {
            conn->setLastError( "Sun::unpackUpdate() - invalid LightFlareData!" );
            mFlareData = NULL;
         }
      }
      else
         mFlareData = NULL;

      mCoronaEnabled = stream->readFlag();
      stream->read( &mCoronaMatName );
      stream->read( &mCoronaScale );
      stream->read( &mCoronaTint );
      mCoronaUseLightColor = stream->readFlag();

      mLight->unpackExtended( stream ); 
   }

   if ( isProperlyAdded() )
   {
      _initCorona();
      _conformLights();
   }
}

void Sun::submitLights( LightManager *lm, bool staticLighting )
{
   // The sun is a special light and needs special registration.
   lm->setSpecialLight( LightManager::slSunLightType, mLight );
}


void Sun::advanceTime( F32 timeDelta )
{
   if (mAnimateSun)
   {
      if (mCurrTime >= mTotalTime)
      {
         mAnimateSun = false;
         mCurrTime = 0.0f;
      }
      else
      {
         mCurrTime += timeDelta;

         F32 fract   = mCurrTime / mTotalTime;
         F32 inverse = 1.0f - fract;

         F32 newAzimuth   = mStartAzimuth * inverse + mEndAzimuth * fract;
         F32 newElevation = mStartElevation * inverse + mEndElevation * fract;

         if (newAzimuth > 360.0f)
            newAzimuth -= 360.0f;
         if (newElevation > 360.0f)
            newElevation -= 360.0f;

         setAzimuth(newAzimuth);
         setElevation(newElevation);
      }
   }
}


void Sun::prepRenderImage( SceneRenderState *state )
{
   // Only render into diffuse and reflect passes.

   if( !state->isDiffusePass() &&
       !state->isReflectPass() )
      return;
   
   mLightWorldPos = state->getCameraPosition() - state->getFarPlane() * mLight->getDirection() * 0.9f;
   F32 dist = ( mLightWorldPos - state->getCameraPosition() ).len();

   F32 screenRadius = GFX->getViewport().extent.y * mCoronaScale * 0.5f;
   mCoronaWorldRadius = screenRadius * dist / state->getWorldToScreenScale().y;   

   // Render instance for Corona effect.   
   if ( mCoronaEnabled && mCoronaMatInst )
   {
      mMatrixSet->setSceneProjection( GFX->getProjectionMatrix() );
      mMatrixSet->setSceneView( GFX->getViewMatrix() );
      mMatrixSet->setWorld( GFX->getWorldMatrix() );

      ObjectRenderInst *ri = state->getRenderPass()->allocInst<ObjectRenderInst>();
      ri->renderDelegate.bind( this, &Sun::_renderCorona );
      ri->type = RenderPassManager::RIT_Sky;      
      // Render after sky objects and before CloudLayer!
      ri->defaultKey = 5;
      ri->defaultKey2 = 0;
      state->getRenderPass()->addInst( ri );
   }

   // LightFlareData handles rendering flare effects.
   if ( mFlareData )
   {
      mFlareState.fullBrightness = mBrightness;
      mFlareState.scale = mFlareScale;
      mFlareState.lightInfo = mLight;
      mFlareState.worldRadius = mCoronaWorldRadius;

      mFlareState.lightMat.identity();
      mFlareState.lightMat.setPosition( mLightWorldPos );

      mFlareData->prepRender( state, &mFlareState );
   }
}

void Sun::setAzimuth( F32 azimuth )
{
   mSunAzimuth = azimuth;
   _conformLights();
   setMaskBits( UpdateMask ); // TODO: Break out the masks to save bandwidth!
}

void Sun::setElevation( F32 elevation )
{
   mSunElevation = elevation;
   _conformLights();
   setMaskBits( UpdateMask ); // TODO: Break out the masks to save some space!
}

void Sun::setColor( const ColorF &color )
{
   mLightColor = color;
   _conformLights();
   setMaskBits( UpdateMask ); // TODO: Break out the masks to save some space!
}

void Sun::animate( F32 duration, F32 startAzimuth, F32 endAzimuth, F32 startElevation, F32 endElevation )
{
   mAnimateSun = true;
   mCurrTime = 0.0f;

   mTotalTime = duration;

   mStartAzimuth = startAzimuth;
   mEndAzimuth = endAzimuth;
   mStartElevation = startElevation;
   mEndElevation = endElevation;
}

void Sun::_conformLights()
{
   // Build the light direction from the azimuth and elevation.
   F32 yaw = mDegToRad(mClampF(mSunAzimuth,0,359));
   F32 pitch = mDegToRad(mClampF(mSunElevation,-360,+360));
   VectorF lightDirection;
   MathUtils::getVectorFromAngles(lightDirection, yaw, pitch);
   lightDirection.normalize();
   mLight->setDirection( -lightDirection );
   mLight->setBrightness( mBrightness );

   // Now make sure the colors are within range.
   mLightColor.clamp();
   mLight->setColor( mLightColor );
   mLightAmbient.clamp();
   mLight->setAmbient( mLightAmbient );

   // Optimization... disable shadows if the ambient and 
   // directional color are the same.
   bool castShadows = mLightColor != mLightAmbient && mCastShadows; 
   mLight->setCastShadows( castShadows );
   mLight->setStaticRefreshFreq(mStaticRefreshFreq);
   mLight->setDynamicRefreshFreq(mDynamicRefreshFreq);
}

void Sun::_initCorona()
{
   if ( isServerObject() )
      return;
      
   SAFE_DELETE( mCoronaMatInst );

   if ( mCoronaMatName.isNotEmpty() )      
      mCoronaMatInst = MATMGR->createMatInstance( mCoronaMatName, MATMGR->getDefaultFeatures(), getGFXVertexFormat<GFXVertexPCT>() );         
}

void Sun::_renderCorona( ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance *overrideMat )
{   
   // Calculate Billboard Radius (in world units) to be constant, independent of distance.
   // Takes into account distance, viewport size, and specified size in editor
   F32 BBRadius = mCoronaWorldRadius;

   mMatrixSet->restoreSceneViewProjection();   
   
   if ( state->isReflectPass() )
      mMatrixSet->setProjection( state->getSceneManager()->getNonClipProjection() );

   //mMatrixSet->setWorld( MatrixF::Identity );

   // Initialize points with basic info
   Point3F points[4];
   points[0] = Point3F(-BBRadius, 0.0, -BBRadius);
   points[1] = Point3F( -BBRadius, 0.0, BBRadius);
   points[2] = Point3F( BBRadius, 0.0,  -BBRadius);
   points[3] = Point3F(BBRadius, 0.0, BBRadius);

   static const Point2F sCoords[4] = 
   {
      Point2F( 0.0f, 0.0f ),
      Point2F( 0.0f, 1.0f ),      
      Point2F( 1.0f, 0.0f ),
      Point2F(1.0f, 1.0f)
   };

   // Get info we need to adjust points
   const MatrixF &camView = state->getCameraTransform();

   // Finalize points
   for(S32 i = 0; i < 4; i++)
   {
      // align with camera
      camView.mulV(points[i]);
      // offset
      points[i] += mLightWorldPos;
   }

   ColorF vertColor;
   if ( mCoronaUseLightColor )
      vertColor = mLightColor;
   else
      vertColor = mCoronaTint;

   GFXVertexBufferHandle< GFXVertexPCT > vb;
   vb.set( GFX, 4, GFXBufferTypeVolatile );
   GFXVertexPCT *pVert = vb.lock();
   if(!pVert) return;

   for ( S32 i = 0; i < 4; i++ )
   {
      pVert->color.set( vertColor );
      pVert->point.set( points[i] );
      pVert->texCoord.set( sCoords[i].x, sCoords[i].y );
      pVert++;
   }

   vb.unlock();

   // Setup SceneData struct.

   SceneData sgData;
   sgData.wireframe = GFXDevice::getWireframe();
   sgData.visibility = 1.0f;

   // Draw it

   while ( mCoronaMatInst->setupPass( state, sgData ) )
   {
      mCoronaMatInst->setTransforms( *mMatrixSet, state );
      mCoronaMatInst->setSceneInfo( state, sgData );

      GFX->setVertexBuffer( vb );      
      GFX->drawPrimitive( GFXTriangleStrip, 0, 2 );
   }
}

void Sun::_updateTimeOfDay( TimeOfDay *timeOfDay, F32 time )
{
   setElevation( timeOfDay->getElevationDegrees() );
   setAzimuth( timeOfDay->getAzimuthDegrees() );
}

void Sun::_onSelected()
{
#ifdef TORQUE_DEBUG
   // Enable debug rendering on the light.
   if( isClientObject() )
      mLight->enableDebugRendering( true );
#endif


   Parent::_onSelected();
}

void Sun::_onUnselected()
{
#ifdef TORQUE_DEBUG
   // Disable debug rendering on the light.
   if( isClientObject() )
      mLight->enableDebugRendering( false );
#endif

   Parent::_onUnselected();
}

DefineConsoleMethod(Sun, apply, void, (), , "")
{
   object->inspectPostApply();
}

DefineConsoleMethod(Sun, animate, void, ( F32 duration, F32 startAzimuth, F32 endAzimuth, F32 startElevation, F32 endElevation ), , "animate( F32 duration, F32 startAzimuth, F32 endAzimuth, F32 startElevation, F32 endElevation )")
{

   object->animate(duration, startAzimuth, endAzimuth, startElevation, endElevation);
}

