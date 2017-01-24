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
#include "lighting/lightManager.h"

#include "console/console.h"
#include "console/consoleTypes.h"
#include "core/util/safeDelete.h"
#include "console/sim.h"
#include "console/simSet.h"
#include "scene/sceneManager.h"
#include "materials/materialManager.h"
#include "materials/sceneData.h"
#include "lighting/lightInfo.h"
#include "lighting/lightingInterfaces.h"
#include "T3D/gameBase/gameConnection.h"
#include "gfx/gfxStringEnumTranslate.h"
#include "console/engineAPI.h"
#include "renderInstance/renderPrePassMgr.h"


Signal<void(const char*,bool)> LightManager::smActivateSignal;
LightManager *LightManager::smActiveLM = NULL;


LightManager::LightManager( const char *name, const char *id )
   :  mName( name ),
      mId( id ),
      mIsActive( false ),      
      mSceneManager( NULL ),
      mDefaultLight( NULL ),
      mAvailableSLInterfaces( NULL ),
      mCullPos( Point3F::Zero )
{ 
   _getLightManagers().insert( mName, this );

   dMemset( &mSpecialLights, 0, sizeof( mSpecialLights ) );
}

LightManager::~LightManager() 
{
   _getLightManagers().erase( mName );
   SAFE_DELETE( mAvailableSLInterfaces );
   SAFE_DELETE( mDefaultLight );
}

LightManagerMap& LightManager::_getLightManagers()
{
   static LightManagerMap lightManagerMap;
   return lightManagerMap;
}

LightManager* LightManager::findByName( const char *name )
{
   LightManagerMap &lightManagers = _getLightManagers();

   LightManagerMap::Iterator iter = lightManagers.find( name );
   if ( iter != lightManagers.end() )
      return iter->value;

   return NULL;
}

void LightManager::getLightManagerNames( String *outString )
{
   LightManagerMap &lightManagers = _getLightManagers();
   LightManagerMap::Iterator iter = lightManagers.begin();
   for ( ; iter != lightManagers.end(); iter++ )
      *outString += iter->key + "\t";

   // TODO!
   //outString->rtrim();
}

LightInfo* LightManager::createLightInfo(LightInfo* light /* = NULL */)
{
   LightInfo *outLight = (light != NULL) ? light : new LightInfo;

   LightManagerMap &lightManagers = _getLightManagers();
   LightManagerMap::Iterator iter = lightManagers.begin();
   for ( ; iter != lightManagers.end(); iter++ )
   {
      LightManager *lm = iter->value;
      lm->_addLightInfoEx( outLight );
   }

   return outLight;
}

void LightManager::initLightFields()
{
   LightManagerMap &lightManagers = _getLightManagers();

   LightManagerMap::Iterator iter = lightManagers.begin();
   for ( ; iter != lightManagers.end(); iter++ )
   {
      LightManager *lm = iter->value;
      lm->_initLightFields();
   }
}

IMPLEMENT_GLOBAL_CALLBACK( onLightManagerActivate, void, ( const char *name ), ( name ),
   "A callback called by the engine when a light manager is activated.\n"
   "@param name The name of the light manager being activated.\n"
   "@ingroup Lighting\n" );

void LightManager::activate( SceneManager *sceneManager )
{
   AssertFatal( sceneManager, "LightManager::activate() - Got null scene manager!" );
   AssertFatal( mIsActive == false, "LightManager::activate() - Already activated!" );
   AssertFatal( smActiveLM == NULL, "LightManager::activate() - A previous LM is still active!" );

   mIsActive = true;
   mSceneManager = sceneManager;
   smActiveLM = this;

   onLightManagerActivate_callback( getName() );
}

IMPLEMENT_GLOBAL_CALLBACK( onLightManagerDeactivate, void, ( const char *name ), ( name ),
   "A callback called by the engine when a light manager is deactivated.\n"
   "@param name The name of the light manager being deactivated.\n"
   "@ingroup Lighting\n" );

void LightManager::deactivate()
{
   AssertFatal( mIsActive == true, "LightManager::deactivate() - Already deactivated!" );
   AssertFatal( smActiveLM == this, "LightManager::activate() - This isn't the active light manager!" );

   if( Sim::getRootGroup() ) // To protect against shutdown.
      onLightManagerDeactivate_callback( getName() );

   mIsActive = false;
   mSceneManager = NULL;
   smActiveLM = NULL;

   // Just in case... make sure we're all clear.
   unregisterAllLights();
}

LightInfo* LightManager::getDefaultLight()
{
   // The sun is always our default light when
   // when its registered.
   if ( mSpecialLights[ LightManager::slSunLightType ] )
      return mSpecialLights[ LightManager::slSunLightType ];

   // Else return a dummy special light.
   if ( !mDefaultLight )
      mDefaultLight = createLightInfo();
   return mDefaultLight;
}

LightInfo* LightManager::getSpecialLight( LightManager::SpecialLightTypesEnum type, bool useDefault )
{
   if ( mSpecialLights[type] )
      return mSpecialLights[type];

   if ( useDefault )
      return getDefaultLight();

   return NULL;
}

void LightManager::setSpecialLight( LightManager::SpecialLightTypesEnum type, LightInfo *light )
{
   if ( light && type == slSunLightType )
   {
      // The sun must be specially positioned and ranged
      // so that it can be processed like a point light 
      // in the stock light shader used by Basic Lighting.
      
      light->setPosition( mCullPos - ( light->getDirection() * 10000.0f ) );
      light->setRange( 2000000.0f );
   }

   mSpecialLights[type] = light;
   registerGlobalLight( light, NULL );
}

void LightManager::registerGlobalLights( const Frustum *frustum, bool staticLighting )
{
   PROFILE_SCOPE( LightManager_RegisterGlobalLights );

   // TODO: We need to work this out...
   //
   // 1. Why do we register and unregister lights on every 
   //    render when they don't often change... shouldn't we
   //    just register once and keep them?
   // 
   // 2. If we do culling of lights should this happen as part
   //    of registration or somewhere else?
   //

   // Grab the lights to process.
   Vector<SceneObject*> activeLights;
   const U32 lightMask = LightObjectType;
   
   if ( staticLighting || !frustum )
   {
      // We're processing static lighting or want all the lights
      // in the container registerd...  so no culling.
      getSceneManager()->getContainer()->findObjectList( lightMask, &activeLights );
   }
   else
   {
      // Cull the lights using the frustum.
      getSceneManager()->getContainer()->findObjectList( *frustum, lightMask, &activeLights );

      for (U32 i = 0; i < activeLights.size(); ++i)
      {
         if (!getSceneManager()->mRenderedObjectsList.contains(activeLights[i]))
         {
            activeLights.erase(i);
            --i;
         }
      }

      // Store the culling position for sun placement
      // later... see setSpecialLight.
      mCullPos = frustum->getPosition();

      // HACK: Make sure the control object always gets 
      // processed as lights mounted to it don't change
      // the shape bounds and can often get culled.

      GameConnection *conn = GameConnection::getConnectionToServer();
      if ( conn->getControlObject() )
      {
         GameBase *conObject = conn->getControlObject();
         activeLights.push_back_unique( conObject );
      }
   }

   // Let the lights register themselves.
   for ( U32 i = 0; i < activeLights.size(); i++ )
   {
      ISceneLight *lightInterface = dynamic_cast<ISceneLight*>( activeLights[i] );
      if ( lightInterface )
         lightInterface->submitLights( this, staticLighting );
   }
}

void LightManager::registerGlobalLight( LightInfo *light, SimObject *obj )
{
   AssertFatal( !mRegisteredLights.contains( light ), 
      "LightManager::registerGlobalLight - This light is already registered!" );

   mRegisteredLights.push_back( light );
}

void LightManager::unregisterGlobalLight( LightInfo *light )
{
   mRegisteredLights.unregisterLight( light );

   // If this is the sun... clear the special light too.
   if ( light == mSpecialLights[slSunLightType] )
      dMemset( mSpecialLights, 0, sizeof( mSpecialLights ) );
}

void LightManager::registerLocalLight( LightInfo *light )
{
   // TODO: What should we do here?
}

void LightManager::unregisterLocalLight( LightInfo *light )
{
   // TODO: What should we do here?
}

void LightManager::unregisterAllLights()
{
   dMemset( mSpecialLights, 0, sizeof( mSpecialLights ) );
   mRegisteredLights.clear();
}

void LightManager::getAllUnsortedLights( Vector<LightInfo*> *list ) const
{
   list->merge( mRegisteredLights );
}

void LightManager::_update4LightConsts(   const SceneData &sgData,
                                          GFXShaderConstHandle *lightPositionSC,
                                          GFXShaderConstHandle *lightDiffuseSC,
                                          GFXShaderConstHandle *lightAmbientSC,
                                          GFXShaderConstHandle *lightInvRadiusSqSC,
                                          GFXShaderConstHandle *lightSpotDirSC,
                                          GFXShaderConstHandle *lightSpotAngleSC,
                                GFXShaderConstHandle *lightSpotFalloffSC,
                                          GFXShaderConstBuffer *shaderConsts )
{
   PROFILE_SCOPE( LightManager_Update4LightConsts );

   // Skip over gathering lights if we don't have to!
   if (  lightPositionSC->isValid() || 
         lightDiffuseSC->isValid() ||
         lightInvRadiusSqSC->isValid() ||
         lightSpotDirSC->isValid() ||
         lightSpotAngleSC->isValid() ||
       lightSpotFalloffSC->isValid() )
   {
      PROFILE_SCOPE( LightManager_Update4LightConsts_setLights );

         static AlignedArray<Point4F> lightPositions( 3, sizeof( Point4F ) );
         static AlignedArray<Point4F> lightSpotDirs( 3, sizeof( Point4F ) );
      static AlignedArray<Point4F> lightColors( 4, sizeof( Point4F ) );
      static Point4F lightInvRadiusSq;
      static Point4F lightSpotAngle;
     static Point4F lightSpotFalloff;
      F32 range;
      
      // Need to clear the buffers so that we don't leak
      // lights from previous passes or have NaNs.
      dMemset( lightPositions.getBuffer(), 0, lightPositions.getBufferSize() );
      dMemset( lightSpotDirs.getBuffer(), 0, lightSpotDirs.getBufferSize() );
      dMemset( lightColors.getBuffer(), 0, lightColors.getBufferSize() );
      lightInvRadiusSq = Point4F::Zero;
      lightSpotAngle.set( -1.0f, -1.0f, -1.0f, -1.0f );
      lightSpotFalloff.set( F32_MAX, F32_MAX, F32_MAX, F32_MAX );

      // Gather the data for the first 4 lights.
      const LightInfo *light;
      for ( U32 i=0; i < 4; i++ )
      {
         light = sgData.lights[i];
         if ( !light )            
            break;

            // The light positions and spot directions are 
            // in SoA order to make optimal use of the GPU.
            const Point3F &lightPos = light->getPosition();
            lightPositions[0][i] = lightPos.x;
            lightPositions[1][i] = lightPos.y;
            lightPositions[2][i] = lightPos.z;

            const VectorF &lightDir = light->getDirection();
            lightSpotDirs[0][i] = lightDir.x;
            lightSpotDirs[1][i] = lightDir.y;
            lightSpotDirs[2][i] = lightDir.z;
            
            if ( light->getType() == LightInfo::Spot )
         {
               lightSpotAngle[i] = mCos( mDegToRad( light->getOuterConeAngle() / 2.0f ) ); 
            lightSpotFalloff[i] = 1.0f / getMax( F32_MIN, mCos( mDegToRad( light->getInnerConeAngle() / 2.0f ) ) - lightSpotAngle[i] );
         }

         // Prescale the light color by the brightness to 
         // avoid doing this in the shader.
         lightColors[i] = Point4F(light->getColor()) * light->getBrightness();

         // We need 1 over range^2 here.
         range = light->getRange().x;
         lightInvRadiusSq[i] = 1.0f / ( range * range );
      }

      shaderConsts->setSafe( lightPositionSC, lightPositions );   
      shaderConsts->setSafe( lightDiffuseSC, lightColors );
      shaderConsts->setSafe( lightInvRadiusSqSC, lightInvRadiusSq );

         shaderConsts->setSafe( lightSpotDirSC, lightSpotDirs );
         shaderConsts->setSafe( lightSpotAngleSC, lightSpotAngle );
       shaderConsts->setSafe( lightSpotFalloffSC, lightSpotFalloff );

      
   }

   // Setup the ambient lighting from the first 
   // light which is the directional light if 
   // one exists at all in the scene.
   if ( lightAmbientSC->isValid() )
      shaderConsts->set( lightAmbientSC, sgData.ambientLightColor );
}

AvailableSLInterfaces* LightManager::getSceneLightingInterface()
{
   if ( !mAvailableSLInterfaces )
      mAvailableSLInterfaces = new AvailableSLInterfaces();

   return mAvailableSLInterfaces;
}

bool LightManager::lightScene( const char* callback, const char* param )
{
   BitSet32 flags = 0;

   if ( param )
   {
      if ( !dStricmp( param, "forceAlways" ) )
         flags.set( SceneLighting::ForceAlways );
      else if ( !dStricmp(param, "forceWritable" ) )
         flags.set( SceneLighting::ForceWritable );
      else if ( !dStricmp(param, "loadOnly" ) )
         flags.set( SceneLighting::LoadOnly );
   }

   // The SceneLighting object will delete itself 
   // once the lighting process is complete.   
   SceneLighting* sl = new SceneLighting( getSceneLightingInterface() );
   return sl->lightScene( callback, flags );
}

RenderPrePassMgr* LightManager::_findPrePassRenderBin()
{
   RenderPassManager* rpm = getSceneManager()->getDefaultRenderPass();
   for( U32 i = 0; i < rpm->getManagerCount(); i++ )
   {
      RenderBinManager *bin = rpm->getManager( i );
      if( bin->getRenderInstType() == RenderPrePassMgr::RIT_PrePass )
      {
         return ( RenderPrePassMgr* ) bin;
      }
   }

   return NULL;
}

DefineEngineFunction( setLightManager, bool, ( const char *name ),,
   "Finds and activates the named light manager.\n"
   "@return Returns true if the light manager is found and activated.\n"
   "@ingroup Lighting\n" )
{
   return gClientSceneGraph->setLightManager( name );
}

DefineEngineFunction( lightScene, bool, ( const char *completeCallbackFn, const char *mode ), ( nullAsType<const char*>(), nullAsType<const char*>() ),
   "Will generate static lighting for the scene if supported by the active light manager.\n\n"
   "If mode is \"forceAlways\", the lightmaps will be regenerated regardless of whether "
   "lighting cache files can be written to. If mode is \"forceWritable\", then the lightmaps "
   "will be regenerated only if the lighting cache files can be written.\n"
   "@param completeCallbackFn The name of the function to execute when the lighting is complete.\n"
   "@param mode One of \"forceAlways\",  \"forceWritable\" or \"loadOnly\".\n"
   "@return Returns true if the scene lighting process was started.\n"
   "@ingroup Lighting\n" )
{
   if ( !LIGHTMGR )
      return false;

   return LIGHTMGR->lightScene( completeCallbackFn, mode );
}

DefineEngineFunction( getLightManagerNames, String, (),,
   "Returns a tab seperated list of light manager names.\n"
   "@ingroup Lighting\n" )
{
   String names;
   LightManager::getLightManagerNames( &names );
   return names;
}

DefineEngineFunction( getActiveLightManager, const char*, (),,
   "Returns the active light manager name.\n"
   "@ingroup Lighting\n" )
{
   if ( !LIGHTMGR )
      return NULL;

   return LIGHTMGR->getName();
}

DefineEngineFunction( resetLightManager, void, (),,
   "@brief Deactivates and then activates the currently active light manager."
   "This causes most shaders to be regenerated and is often used when global "
   "rendering changes have occured.\n"
   "@ingroup Lighting\n" )
{
   LightManager *lm = LIGHTMGR;
   if ( !lm )
      return;

   SceneManager *sm = lm->getSceneManager();
   lm->deactivate();
   lm->activate( sm );
}
