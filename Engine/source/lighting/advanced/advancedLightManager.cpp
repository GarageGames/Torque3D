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
#include "lighting/advanced/advancedLightManager.h"

#include "lighting/advanced/advancedLightBinManager.h"
#include "lighting/advanced/advancedLightingFeatures.h"
#include "lighting/shadowMap/shadowMapManager.h"
#include "lighting/shadowMap/lightShadowMap.h"
#include "lighting/common/sceneLighting.h"
#include "lighting/common/lightMapParams.h"
#include "core/util/safeDelete.h"
#include "renderInstance/renderPrePassMgr.h"
#include "materials/materialManager.h"
#include "math/util/sphereMesh.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "scene/sceneRenderState.h"
#include "gfx/gfxCardProfile.h"
#include "gfx/gfxTextureProfile.h"


ImplementEnumType( ShadowType,
   "\n\n"
   "@ingroup AdvancedLighting" )
   { ShadowType_Spot,                     "Spot" },
   { ShadowType_PSSM,                     "PSSM" },
   { ShadowType_Paraboloid,               "Paraboloid" },
   { ShadowType_DualParaboloidSinglePass, "DualParaboloidSinglePass" },
   { ShadowType_DualParaboloid,           "DualParaboloid" },
   { ShadowType_CubeMap,                  "CubeMap" },
EndImplementEnumType;


AdvancedLightManager AdvancedLightManager::smSingleton;


AdvancedLightManager::AdvancedLightManager()
   :  LightManager( "Advanced Lighting", "ADVLM" )
{
   mLightBinManager = NULL;
   mLastShader = NULL;
   mAvailableSLInterfaces = NULL;
}

AdvancedLightManager::~AdvancedLightManager()
{
   mLastShader = NULL;
   mLastConstants = NULL;

   for (LightConstantMap::Iterator i = mConstantLookup.begin(); i != mConstantLookup.end(); i++)
   {
      if (i->value)
         SAFE_DELETE(i->value);
   }
   mConstantLookup.clear();
}

bool AdvancedLightManager::isCompatible() const
{
   // TODO: We need at least 3.0 shaders at the moment
   // but this should be relaxed to 2.0 soon.
   if ( GFX->getPixelShaderVersion() < 3.0 )
      return false;

   // TODO: Test for the necessary texture formats!
   bool autoMips;
   if(!GFX->getCardProfiler()->checkFormat(GFXFormatR16F, &GFXDefaultRenderTargetProfile, autoMips))
      return false;

   return true;
}

void AdvancedLightManager::activate( SceneManager *sceneManager )
{
   Parent::activate( sceneManager );

   GFXShader::addGlobalMacro( "TORQUE_ADVANCED_LIGHTING" );

   sceneManager->setPostEffectFog( true );

   SHADOWMGR->activate();

   // Find a target format that supports blending... 
   // we prefer the floating point format if it works.
   Vector<GFXFormat> formats;
   formats.push_back( GFXFormatR16G16B16A16F );
   //formats.push_back( GFXFormatR16G16B16A16 );
   GFXFormat blendTargetFormat = GFX->selectSupportedFormat( &GFXDefaultRenderTargetProfile,
                                                         formats,
                                                         true,
                                                         true,
                                                         false );

   mLightBinManager = new AdvancedLightBinManager( this, SHADOWMGR, blendTargetFormat );
   mLightBinManager->assignName( "AL_LightBinMgr" );

   // First look for the prepass bin...
   RenderPrePassMgr *prePassBin = _findPrePassRenderBin();

   // If we didn't find the prepass bin then add one.
   if ( !prePassBin )
   {
      prePassBin = new RenderPrePassMgr( true, blendTargetFormat );
      prePassBin->assignName( "AL_PrePassBin" );
      prePassBin->registerObject();
      getSceneManager()->getDefaultRenderPass()->addManager( prePassBin );
      mPrePassRenderBin = prePassBin;
   }

   // Tell the material manager that prepass is enabled.
   MATMGR->setPrePassEnabled( true );

   // Insert our light bin manager.
   mLightBinManager->setRenderOrder( prePassBin->getRenderOrder() + 0.01f );
   getSceneManager()->getDefaultRenderPass()->addManager( mLightBinManager );

   AdvancedLightingFeatures::registerFeatures(mPrePassRenderBin->getTargetFormat(), mLightBinManager->getTargetFormat());

   // Last thing... let everyone know we're active.
   smActivateSignal.trigger( getId(), true );
}

void AdvancedLightManager::deactivate()
{
   Parent::deactivate();

   GFXShader::removeGlobalMacro( "TORQUE_ADVANCED_LIGHTING" );

   // Release our bin manager... it will take care of
   // removing itself from the render passes.
   if( mLightBinManager )
   {
      mLightBinManager->MRTLightmapsDuringPrePass(false);
      mLightBinManager->deleteObject();
   }
   mLightBinManager = NULL;

   if ( mPrePassRenderBin )
      mPrePassRenderBin->deleteObject();
   mPrePassRenderBin = NULL;

   SHADOWMGR->deactivate();

   mLastShader = NULL;
   mLastConstants = NULL;

   for (LightConstantMap::Iterator i = mConstantLookup.begin(); i != mConstantLookup.end(); i++)
   {
      if (i->value)
         SAFE_DELETE(i->value);
   }
   mConstantLookup.clear();

   mSphereGeometry = NULL;
   mSphereIndices = NULL;
   mConeGeometry = NULL;
   mConeIndices = NULL;

   AdvancedLightingFeatures::unregisterFeatures();

   // Now let everyone know we've deactivated.
   smActivateSignal.trigger( getId(), false );
}

void AdvancedLightManager::_addLightInfoEx( LightInfo *lightInfo )
{
   lightInfo->addExtended( new ShadowMapParams( lightInfo ) );
   lightInfo->addExtended( new LightMapParams( lightInfo ) );
}


void AdvancedLightManager::_initLightFields()
{
   #define DEFINE_LIGHT_FIELD( var, type, enum_ )                             \
   static inline const char* _get##var##Field( void *obj, const char *data )  \
   {                                                                          \
      ShadowMapParams *p = _getShadowMapParams( obj );                        \
      if ( p )                                                                \
         return Con::getData( type, &p->var, 0, enum_ );                      \
      else                                                                    \
         return "";                                                           \
   }                                                                          \
                                                                              \
   static inline bool _set##var##Field( void *object, const char *index, const char *data )  \
   {                                                                                         \
      ShadowMapParams *p = _getShadowMapParams( object );                     \
      if ( p )                                                                \
      {                                                                       \
         Con::setData( type, &p->var, 0, 1, &data, enum_ );                   \
         p->_validate();                                                      \
      }                                                                       \
      return false;                                                           \
   }

   #define DEFINE_LIGHTMAP_FIELD( var, type, enum_ )                          \
   static inline const char* _get##var##Field( void *obj, const char *data )  \
   {                                                                          \
      LightMapParams *p = _getLightMapParams( obj );                          \
      if ( p )                                                                \
         return Con::getData( type, &p->var, 0, enum_ );                      \
      else                                                                    \
         return "";                                                           \
   }                                                                          \
   \
   static inline bool _set##var##Field( void *object, const char *index, const char *data )  \
   {                                                                                         \
      LightMapParams *p = _getLightMapParams( object );                       \
      if ( p )                                                                \
      {                                                                       \
         Con::setData( type, &p->var, 0, 1, &data, enum_ );                   \
      }                                                                       \
      return false;                                                           \
   }

   #define ADD_LIGHT_FIELD( field, type, var, desc )                       \
   ConsoleObject::addProtectedField( field, type, 0,                       \
      &Dummy::_set##var##Field, &Dummy::_get##var##Field, desc )

   // Our dummy adaptor class which we hide in here
   // to keep from poluting the global namespace.
   class Dummy
   {
   protected:

      static inline ShadowMapParams* _getShadowMapParams( void *obj )
      {
         ISceneLight *sceneLight = dynamic_cast<ISceneLight*>( (SimObject*)obj );
         if ( sceneLight )
         {
            LightInfo *lightInfo = sceneLight->getLight();                           
            if ( lightInfo )
               return lightInfo->getExtended<ShadowMapParams>();
         }
         return NULL;
      }

      static inline LightMapParams* _getLightMapParams( void *obj )
      {
         ISceneLight *sceneLight = dynamic_cast<ISceneLight*>( (SimObject*)obj );
         if ( sceneLight )
         {
            LightInfo *lightInfo = sceneLight->getLight();                           
            if ( lightInfo )
               return lightInfo->getExtended<LightMapParams>();
         }
         return NULL;
      }

   public:

      DEFINE_LIGHT_FIELD( attenuationRatio, TypePoint3F, NULL );
      DEFINE_LIGHT_FIELD( shadowType, TYPEID< ShadowType >(), ConsoleBaseType::getType( TYPEID< ShadowType >() )->getEnumTable() );
      DEFINE_LIGHT_FIELD( texSize, TypeS32, NULL );
      DEFINE_LIGHT_FIELD( cookie, TypeStringFilename, NULL );      
      DEFINE_LIGHT_FIELD( numSplits, TypeS32, NULL );
      DEFINE_LIGHT_FIELD( logWeight, TypeF32, NULL );
      DEFINE_LIGHT_FIELD( overDarkFactor, TypePoint4F, NULL);
      DEFINE_LIGHT_FIELD( shadowDistance, TypeF32, NULL );
      DEFINE_LIGHT_FIELD( shadowSoftness, TypeF32, NULL );
      DEFINE_LIGHT_FIELD( fadeStartDist, TypeF32, NULL );
      DEFINE_LIGHT_FIELD( lastSplitTerrainOnly, TypeBool, NULL );     

      DEFINE_LIGHTMAP_FIELD( representedInLightmap, TypeBool, NULL );
      DEFINE_LIGHTMAP_FIELD( shadowDarkenColor, TypeColorF, NULL );
      DEFINE_LIGHTMAP_FIELD( includeLightmappedGeometryInShadow, TypeBool, NULL );
   };

   ConsoleObject::addGroup( "Advanced Lighting" );

      ADD_LIGHT_FIELD( "attenuationRatio", TypePoint3F, attenuationRatio,
         "The proportions of constant, linear, and quadratic attenuation to use for "
         "the falloff for point and spot lights." );

      ADD_LIGHT_FIELD( "shadowType", TYPEID< ShadowType >(), shadowType,
         "The type of shadow to use on this light." );

      ADD_LIGHT_FIELD( "cookie", TypeStringFilename, cookie,
         "A custom pattern texture which is projected from the light." );

      ADD_LIGHT_FIELD( "texSize", TypeS32, texSize,
         "The texture size of the shadow map." );

      ADD_LIGHT_FIELD( "overDarkFactor", TypePoint4F, overDarkFactor,
         "The ESM shadow darkening factor");

      ADD_LIGHT_FIELD( "shadowDistance", TypeF32, shadowDistance,
         "The distance from the camera to extend the PSSM shadow." );

      ADD_LIGHT_FIELD( "shadowSoftness", TypeF32, shadowSoftness,
         "" );

      ADD_LIGHT_FIELD( "numSplits", TypeS32, numSplits,
         "The logrithmic PSSM split distance factor." );

      ADD_LIGHT_FIELD( "logWeight", TypeF32, logWeight,
         "The logrithmic PSSM split distance factor." );      

      ADD_LIGHT_FIELD( "fadeStartDistance", TypeF32, fadeStartDist,
         "Start fading shadows out at this distance.  0 = auto calculate this distance.");

      ADD_LIGHT_FIELD( "lastSplitTerrainOnly", TypeBool, lastSplitTerrainOnly,
         "This toggles only terrain being rendered to the last split of a PSSM shadow map.");

   ConsoleObject::endGroup( "Advanced Lighting" );

   ConsoleObject::addGroup( "Advanced Lighting Lightmap" );

      ADD_LIGHT_FIELD( "representedInLightmap", TypeBool, representedInLightmap,
         "This light is represented in lightmaps (static light, default: false)");

      ADD_LIGHT_FIELD( "shadowDarkenColor", TypeColorF, shadowDarkenColor,
         "The color that should be used to multiply-blend dynamic shadows onto lightmapped geometry (ignored if 'representedInLightmap' is false)");

      ADD_LIGHT_FIELD( "includeLightmappedGeometryInShadow", TypeBool, includeLightmappedGeometryInShadow,
         "This light should render lightmapped geometry during its shadow-map update (ignored if 'representedInLightmap' is false)");

   ConsoleObject::endGroup( "Advanced Lighting Lightmap" );

   #undef DEFINE_LIGHT_FIELD
   #undef ADD_LIGHT_FIELD
}

void AdvancedLightManager::setLightInfo(  ProcessedMaterial *pmat, 
                                          const Material *mat, 
                                          const SceneData &sgData,
                                          const SceneRenderState *state,
                                          U32 pass, 
                                          GFXShaderConstBuffer *shaderConsts)
{
   // Skip this if we're rendering from the prepass bin.
   if ( sgData.binType == SceneData::PrePassBin )
      return;

   PROFILE_SCOPE(AdvancedLightManager_setLightInfo);

   LightingShaderConstants *lsc = getLightingShaderConstants(shaderConsts);

   LightShadowMap *lsm = SHADOWMGR->getCurrentShadowMap();
   LightShadowMap *dynamicShadowMap = SHADOWMGR->getCurrentDynamicShadowMap();

   LightInfo *light;
   if (lsm)
      light = lsm->getLightInfo();
   else if ( dynamicShadowMap )
      light = dynamicShadowMap->getLightInfo();
   else
   {
      light = sgData.lights[0];
      if ( !light )
         light = getDefaultLight();
   }

   // NOTE: If you encounter a crash from this point forward
   // while setting a shader constant its probably because the
   // mConstantLookup has bad shaders/constants in it.
   //
   // This is a known crash bug that can occur if materials/shaders
   // are reloaded and the light manager is not reset.
   //
   // We should look to fix this by clearing the table.

   // Update the forward shading light constants.
   _update4LightConsts( sgData,
                        lsc->mLightPositionSC,
                        lsc->mLightDiffuseSC,
                        lsc->mLightAmbientSC,
                        lsc->mLightInvRadiusSqSC,
                        lsc->mLightSpotDirSC,
                        lsc->mLightSpotAngleSC,
                        lsc->mLightSpotFalloffSC,
                        shaderConsts );

   // Static
   if (lsm && light->getCastShadows())
   {
      if (  lsc->mWorldToLightProjSC->isValid() )
         shaderConsts->set(   lsc->mWorldToLightProjSC, 
                              lsm->getWorldToLightProj(), 
                              lsc->mWorldToLightProjSC->getType() );

      if (  lsc->mViewToLightProjSC->isValid() )
      {
         // TODO: Should probably cache these results and 
         // not do this mul here on every material that needs
         // this transform.

         shaderConsts->set(   lsc->mViewToLightProjSC, 
                              lsm->getWorldToLightProj() * state->getCameraTransform(), 
                              lsc->mViewToLightProjSC->getType() );
      }

      shaderConsts->setSafe( lsc->mShadowMapSizeSC, 1.0f / (F32)lsm->getTexSize() );

      // Do this last so that overrides can properly override parameters previously set
      lsm->setShaderParameters(shaderConsts, lsc);
   }
   else
   {
      if ( lsc->mViewToLightProjSC->isValid() )
      {
         // TODO: Should probably cache these results and 
         // not do this mul here on every material that needs
         // this transform.
         MatrixF proj;
         light->getWorldToLightProj( &proj );

         shaderConsts->set(   lsc->mViewToLightProjSC, 
                              proj * state->getCameraTransform(), 
                              lsc->mViewToLightProjSC->getType() );
      }
   }

   // Dynamic
   if ( dynamicShadowMap )
   {
      if (  lsc->mDynamicWorldToLightProjSC->isValid() )
         shaderConsts->set(   lsc->mDynamicWorldToLightProjSC, 
                              dynamicShadowMap->getWorldToLightProj(), 
                              lsc->mDynamicWorldToLightProjSC->getType() );

      if (  lsc->mDynamicViewToLightProjSC->isValid() )
      {
         // TODO: Should probably cache these results and 
         // not do this mul here on every material that needs
         // this transform.

         shaderConsts->set(   lsc->mDynamicViewToLightProjSC, 
                              dynamicShadowMap->getWorldToLightProj() * state->getCameraTransform(), 
                              lsc->mDynamicViewToLightProjSC->getType() );
      }

      shaderConsts->setSafe( lsc->mShadowMapSizeSC, 1.0f / (F32)dynamicShadowMap->getTexSize() );

      // Do this last so that overrides can properly override parameters previously set
      dynamicShadowMap->setShaderParameters(shaderConsts, lsc);
   }   
   else
   {
      if ( lsc->mDynamicViewToLightProjSC->isValid() )
      {
         // TODO: Should probably cache these results and 
         // not do this mul here on every material that needs
         // this transform.
         MatrixF proj;
         light->getWorldToLightProj( &proj );

         shaderConsts->set(   lsc->mDynamicViewToLightProjSC, 
                              proj * state->getCameraTransform(), 
                              lsc->mDynamicViewToLightProjSC->getType() );
      }
   }
}

void AdvancedLightManager::registerGlobalLight(LightInfo *light, SimObject *obj)
{
   Parent::registerGlobalLight( light, obj );

   // Pass the volume lights to the bin manager.
   if (  mLightBinManager &&
         (  light->getType() == LightInfo::Point ||
            light->getType() == LightInfo::Spot ) )
      mLightBinManager->addLight( light );
}

void AdvancedLightManager::unregisterAllLights()
{
   Parent::unregisterAllLights();

   if ( mLightBinManager )
      mLightBinManager->clearAllLights();
}

bool AdvancedLightManager::setTextureStage(  const SceneData &sgData,
                                             const U32 currTexFlag,
                                             const U32 textureSlot, 
                                             GFXShaderConstBuffer *shaderConsts, 
                                             ShaderConstHandles *handles )
{
   LightShadowMap* lsm = SHADOWMGR->getCurrentShadowMap();
   LightShadowMap* dynamicShadowMap = SHADOWMGR->getCurrentDynamicShadowMap();

   // Assign Shadowmap, if it exists
   LightingShaderConstants* lsc = getLightingShaderConstants(shaderConsts);
   if ( !lsc )
      return false;

   if ( currTexFlag == Material::DynamicLight )
   {
      // Static
      if ( lsm && lsm->getLightInfo()->getCastShadows() )
         return lsm->setTextureStage( currTexFlag, lsc );

      S32 reg = lsc->mShadowMapSC->getSamplerRegister();
   	if ( reg != -1 )
      	GFX->setTexture( reg, GFXTexHandle::ONE );

      return true;
   } else if ( currTexFlag == Material::DynamicShadowMap )
   {
      // Dynamic
      if ( dynamicShadowMap )
         return dynamicShadowMap->setTextureStage( currTexFlag, lsc );

      S32 reg = lsc->mDynamicShadowMapSC->getSamplerRegister();
   	if ( reg != -1 )
      	GFX->setTexture( reg, GFXTexHandle::ONE );

      return true;
   }
   else if ( currTexFlag == Material::DynamicLightMask )
   {
      S32 reg = lsc->mCookieMapSC->getSamplerRegister();
   	if ( reg != -1 && sgData.lights[0] )
      {
         ShadowMapParams *p = sgData.lights[0]->getExtended<ShadowMapParams>();

         if ( lsc->mCookieMapSC->getType() == GFXSCT_SamplerCube )
            GFX->setCubeTexture( reg, p->getCookieCubeTex() );
         else
      	   GFX->setTexture( reg, p->getCookieTex() );
      }

      return true;
   }

   return false;
}

LightingShaderConstants* AdvancedLightManager::getLightingShaderConstants(GFXShaderConstBuffer* buffer)
{
   if ( !buffer )
      return NULL;

   PROFILE_SCOPE( AdvancedLightManager_GetLightingShaderConstants );

   GFXShader* shader = buffer->getShader();

   // Check to see if this is the same shader, we'll get hit repeatedly by
   // the same one due to the render bin loops.
   if ( mLastShader.getPointer() != shader )
   {
      LightConstantMap::Iterator iter = mConstantLookup.find(shader);   
      if ( iter != mConstantLookup.end() )
      {
         mLastConstants = iter->value;
      } 
      else 
      {     
         LightingShaderConstants* lsc = new LightingShaderConstants();
         mConstantLookup[shader] = lsc;

         mLastConstants = lsc;      
      }

      // Set our new shader
      mLastShader = shader;
   }

   // Make sure that our current lighting constants are initialized
   if (!mLastConstants->mInit)
      mLastConstants->init(shader);

   return mLastConstants;
}

GFXVertexBufferHandle<AdvancedLightManager::LightVertex> AdvancedLightManager::getSphereMesh(U32 &outNumPrimitives, GFXPrimitiveBuffer *&outPrimitives)
{
   static SphereMesh sSphereMesh;

   if( mSphereGeometry.isNull() )
   {
      const SphereMesh::TriangleMesh * sphereMesh = sSphereMesh.getMesh(3);
      S32 numPoly = sphereMesh->numPoly;
      mSpherePrimitiveCount = 0;
      mSphereGeometry.set(GFX, numPoly*3, GFXBufferTypeStatic);
      mSphereGeometry.lock();
      S32 vertexIndex = 0;
      
      for (S32 i=0; i<numPoly; i++)
      {
         mSpherePrimitiveCount++;

         mSphereGeometry[vertexIndex].point = sphereMesh->poly[i].pnt[0];
         mSphereGeometry[vertexIndex].color = ColorI::WHITE;
         vertexIndex++;

         mSphereGeometry[vertexIndex].point = sphereMesh->poly[i].pnt[1];
         mSphereGeometry[vertexIndex].color = ColorI::WHITE;
         vertexIndex++;

         mSphereGeometry[vertexIndex].point = sphereMesh->poly[i].pnt[2];
         mSphereGeometry[vertexIndex].color = ColorI::WHITE;
         vertexIndex++;
      }
      mSphereGeometry.unlock();
   }

   outNumPrimitives = mSpherePrimitiveCount;
   outPrimitives = NULL; // For now
   return mSphereGeometry;
}

GFXVertexBufferHandle<AdvancedLightManager::LightVertex> AdvancedLightManager::getConeMesh(U32 &outNumPrimitives, GFXPrimitiveBuffer *&outPrimitives )
{
   static const Point2F circlePoints[] = 
   {
      Point2F(0.707107f, 0.707107f),
      Point2F(0.923880f, 0.382683f),
      Point2F(1.000000f, 0.000000f),
      Point2F(0.923880f, -0.382684f),
      Point2F(0.707107f, -0.707107f),
      Point2F(0.382683f, -0.923880f),
      Point2F(0.000000f, -1.000000f),
      Point2F(-0.382683f, -0.923880f),
      Point2F(-0.707107f, -0.707107f),
      Point2F(-0.923880f, -0.382684f),
      Point2F(-1.000000f, 0.000000f),
      Point2F(-0.923879f, 0.382684f),
      Point2F(-0.707107f, 0.707107f),
      Point2F(-0.382683f, 0.923880f),
      Point2F(0.000000f, 1.000000f),
      Point2F(0.382684f, 0.923879f)
   };
   const S32 numPoints = sizeof(circlePoints)/sizeof(Point2F);

   if ( mConeGeometry.isNull() )
   {
      mConeGeometry.set(GFX, numPoints + 1, GFXBufferTypeStatic);
      mConeGeometry.lock();

      mConeGeometry[0].point = Point3F(0.0f,0.0f,0.0f);

      for (S32 i=1; i<numPoints + 1; i++)
      {
         S32 imod = (i - 1) % numPoints;
         mConeGeometry[i].point = Point3F(circlePoints[imod].x,circlePoints[imod].y, -1.0f);
         mConeGeometry[i].color = ColorI::WHITE;
      }
      mConeGeometry.unlock();

      mConePrimitiveCount = numPoints * 2 - 1;

      // Now build the index buffer
      mConeIndices.set(GFX, mConePrimitiveCount * 3, mConePrimitiveCount, GFXBufferTypeStatic);

      U16 *idx = NULL;
      mConeIndices.lock( &idx );
      // Build the cone
      U32 idxIdx = 0;
      for( U32 i = 1; i < numPoints + 1; i++ )
      {
         idx[idxIdx++] = 0; // Triangles on cone start at top point
         idx[idxIdx++] = i;
         idx[idxIdx++] = ( i + 1 > numPoints ) ? 1 : i + 1;
      }

      // Build the bottom of the cone (reverse winding order)
      for( U32 i = 1; i < numPoints - 1; i++ )
      {
         idx[idxIdx++] = 1;
         idx[idxIdx++] = i + 2;
         idx[idxIdx++] = i + 1;
      }
      mConeIndices.unlock();
   }

   outNumPrimitives = mConePrimitiveCount;
   outPrimitives = mConeIndices.getPointer();
   return mConeGeometry;
}

LightShadowMap* AdvancedLightManager::findShadowMapForObject( SimObject *object )
{
   if ( !object )
      return NULL;

   ISceneLight *sceneLight = dynamic_cast<ISceneLight*>( object );
   if ( !sceneLight || !sceneLight->getLight() )
      return NULL;

   return sceneLight->getLight()->getExtended<ShadowMapParams>()->getShadowMap();
}

DefineConsoleFunction( setShadowVizLight, const char*, (const char* name), (""), "")
{
   static const String DebugTargetName( "AL_ShadowVizTexture" );

   NamedTexTarget *target = NamedTexTarget::find( DebugTargetName );
   if ( target )
      target->unregister();

   AdvancedLightManager *lm = dynamic_cast<AdvancedLightManager*>( LIGHTMGR );
   if ( !lm )
      return 0;

   SimObject *object;
   Sim::findObject( name, object );
   LightShadowMap *lightShadowMap = lm->findShadowMapForObject( object );
   if ( !lightShadowMap || !lightShadowMap->getTexture() )
      return 0;

   lightShadowMap->setDebugTarget( DebugTargetName );

   GFXTextureObject *texObject = lightShadowMap->getTexture();
   const Point3I &size = texObject->getSize();
   F32 aspect = (F32)size.x / (F32)size.y;

   static const U32 bufSize = 64;
   char *result = Con::getReturnBuffer( bufSize );
   dSprintf( result, bufSize, "%d %d %g", size.x, size.y, aspect ); 
   return result;
}

