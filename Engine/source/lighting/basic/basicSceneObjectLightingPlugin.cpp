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
#include "lighting/basic/basicSceneObjectLightingPlugin.h"

#include "lighting/lightManager.h"
#include "lighting/shadowMap/shadowMapPass.h"
#include "lighting/shadowMap/shadowMapManager.h"
#include "lighting/common/projectedShadow.h"
#include "T3D/shapeBase.h"
#include "gfx/gfxDevice.h"
#include "gfx/gfxTransformSaver.h"
#include "ts/tsRenderState.h"
#include "gfx/sim/cubemapData.h"
#include "scene/reflector.h"
#include "T3D/decal/decalManager.h"
#include "core/module.h"


MODULE_BEGIN( BasicSceneObjectLightingPlugin )

   MODULE_INIT
   {
      BasicSceneObjectPluginFactory::createSingleton();
   }
   
   MODULE_SHUTDOWN
   {
      BasicSceneObjectPluginFactory::deleteSingleton();
   }
   
MODULE_END;


static const U32 shadowObjectTypeMask = PlayerObjectType | CorpseObjectType | ItemObjectType | VehicleObjectType;
Vector<BasicSceneObjectLightingPlugin*> BasicSceneObjectLightingPlugin::smPluginInstances( __FILE__, __LINE__ );

BasicSceneObjectLightingPlugin::BasicSceneObjectLightingPlugin(SceneObject* parent)
   : mParentObject( parent )
{
   mShadow = NULL;

   // Stick us on the list.
   smPluginInstances.push_back( this );
}

BasicSceneObjectLightingPlugin::~BasicSceneObjectLightingPlugin()
{
   SAFE_DELETE( mShadow );
   
   // Delete us from the list.
   smPluginInstances.remove( this );
}

void BasicSceneObjectLightingPlugin::reset()
{
   SAFE_DELETE( mShadow );
}

void BasicSceneObjectLightingPlugin::cleanupPluginInstances()
{
   for (U32 i = 0; i < smPluginInstances.size(); i++)
   {
      BasicSceneObjectLightingPlugin *plug = smPluginInstances[i];
      smPluginInstances.remove( plug );
      delete plug;
      i--;
   }
   
   smPluginInstances.clear();
}

void BasicSceneObjectLightingPlugin::resetAll()
{
   for( U32 i = 0, num = smPluginInstances.size(); i < num; ++ i )
      smPluginInstances[ i ]->reset();
}

const F32 BasicSceneObjectLightingPlugin::getScore() const
{ 
   return mShadow ? mShadow->getScore() : 0.0f; 
}

void BasicSceneObjectLightingPlugin::updateShadow( SceneRenderState *state )
{
   if ( !mShadow )
      mShadow = new ProjectedShadow( mParentObject );

   mShadow->update( state );
}

void BasicSceneObjectLightingPlugin::renderShadow( SceneRenderState *state )
{
   // hack until new scenegraph in place
   GFXTransformSaver ts;
   
   TSRenderState rstate;
   rstate.setSceneState(state);

   F32 camDist = (state->getCameraPosition() - mParentObject->getRenderPosition()).len();
   
   // Make sure the shadow wants to be rendered
   if( mShadow->shouldRender( state ) )
   {
      // Render! (and note the time)      
      mShadow->render( camDist, rstate );
   }
}

BasicSceneObjectPluginFactory::BasicSceneObjectPluginFactory()
 : mEnabled( false )
{
   LightManager::smActivateSignal.notify( this, &BasicSceneObjectPluginFactory::_onLMActivate );

   ShadowMapManager::smShadowDeactivateSignal.notify( this, &BasicSceneObjectPluginFactory::_setEnabled );
}

BasicSceneObjectPluginFactory::~BasicSceneObjectPluginFactory()
{
   LightManager::smActivateSignal.remove( this, &BasicSceneObjectPluginFactory::_onLMActivate );

   ShadowMapManager::smShadowDeactivateSignal.remove( this, &BasicSceneObjectPluginFactory::_setEnabled );
}

void BasicSceneObjectPluginFactory::_onLMActivate( const char *lm, bool enable )
{
   _setEnabled();
}

void BasicSceneObjectPluginFactory::_setEnabled()
{
   bool enable = false;

   // Enabled if using basic lighting.
   LightManager *lm = LightManager::getActiveLM();
   if ( lm && dStricmp( lm->getName(), "Basic Lighting" ) == 0 )   
      enable = true;
   
   // Disabled if all shadows are explictly disabled.
   if ( ShadowMapPass::smDisableShadows )
      enable = false;

   // Already at the desired state.
   if ( enable == mEnabled )
      return;
   
   if ( enable )
   {
      SceneObject::smSceneObjectAdd.notify(this, &BasicSceneObjectPluginFactory::addLightPlugin);
      SceneObject::smSceneObjectRemove.notify(this, &BasicSceneObjectPluginFactory::removeLightPlugin);
      
      if( gDecalManager )
         gDecalManager->getClearDataSignal().notify( this, &BasicSceneObjectPluginFactory::_onDecalManagerClear );
         
      addToExistingObjects();
   } 
   else 
   {
      SceneObject::smSceneObjectAdd.remove(this, &BasicSceneObjectPluginFactory::addLightPlugin);
      SceneObject::smSceneObjectRemove.remove(this, &BasicSceneObjectPluginFactory::removeLightPlugin);
      
      if( gDecalManager )
         gDecalManager->getClearDataSignal().remove( this, &BasicSceneObjectPluginFactory::_onDecalManagerClear );
         
      BasicSceneObjectLightingPlugin::cleanupPluginInstances();
   }

   mEnabled = enable;
}

void BasicSceneObjectPluginFactory::_onDecalManagerClear()
{
   BasicSceneObjectLightingPlugin::resetAll();
}

void BasicSceneObjectPluginFactory::removeLightPlugin( SceneObject *obj )
{
   // Grab the plugin instance.
   SceneObjectLightingPlugin *lightPlugin = obj->getLightingPlugin();

   // Delete it, which will also remove it
   // from the static list of plugin instances.
   if ( lightPlugin )
   {
      delete lightPlugin;
      obj->setLightingPlugin( NULL );
   }
}

void BasicSceneObjectPluginFactory::addLightPlugin(SceneObject* obj)
{
   bool serverObj = obj->isServerObject();
   bool isShadowType = (obj->getTypeMask() & shadowObjectTypeMask);

   if ( !isShadowType || serverObj )
      return;

   obj->setLightingPlugin(new BasicSceneObjectLightingPlugin(obj));
}

// Some objects may not get cleaned up during mission load/free, so add our
// plugin to existing scene objects
void BasicSceneObjectPluginFactory::addToExistingObjects()
{
   SimpleQueryList sql;  
   gClientContainer.findObjects( shadowObjectTypeMask, SimpleQueryList::insertionCallback, &sql);
   for (SceneObject** i = sql.mList.begin(); i != sql.mList.end(); i++)
      addLightPlugin(*i);
}
                                                                                                         
