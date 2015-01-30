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
#include "materials/materialManager.h"

#include "materials/matInstance.h"
#include "materials/materialFeatureTypes.h"
#include "lighting/lightManager.h"
#include "core/util/safeDelete.h"
#include "shaderGen/shaderGen.h"
#include "core/module.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"


MODULE_BEGIN( MaterialManager )

   MODULE_INIT_BEFORE( GFX )
   MODULE_SHUTDOWN_BEFORE( GFX )
   
   MODULE_INIT
   {
      MaterialManager::createSingleton();
   }
   
   MODULE_SHUTDOWN
   {
      MaterialManager::deleteSingleton();
   }

MODULE_END;


MaterialManager::MaterialManager()
{
   VECTOR_SET_ASSOCIATION( mMatInstanceList );

   mDt = 0.0f; 
   mAccumTime = 0.0f; 
   mLastTime = 0; 
   mWarningInst = NULL;
   
   GFXDevice::getDeviceEventSignal().notify( this, &MaterialManager::_handleGFXEvent );

   // Make sure we get activation signals
   // and that we're the last to get them.
   LightManager::smActivateSignal.notify( this, &MaterialManager::_onLMActivate, 9999 );

   mMaterialSet = NULL;

   mUsingPrePass = false;

   mFlushAndReInit = false;

   mDefaultAnisotropy = 1;
   Con::addVariable( "$pref::Video::defaultAnisotropy", TypeS32, &mDefaultAnisotropy, 
      "@brief Global variable defining the default anisotropy value.\n\n"
      "Controls the default anisotropic texture filtering level for all materials, including the terrain. "
      "This value can be changed at runtime to see its affect without reloading.\n\n "
	   "@ingroup Materials");
   Con::NotifyDelegate callabck( this, &MaterialManager::_updateDefaultAnisotropy );
   Con::addVariableNotify( "$pref::Video::defaultAnisotropy", callabck );

   Con::NotifyDelegate callabck2( this, &MaterialManager::_onDisableMaterialFeature );
   Con::setVariable( "$pref::Video::disableNormalMapping", "false" );
   Con::addVariableNotify( "$pref::Video::disableNormalMapping", callabck2 );
   Con::setVariable( "$pref::Video::disablePixSpecular", "false" );
   Con::addVariableNotify( "$pref::Video::disablePixSpecular", callabck2 );
   Con::setVariable( "$pref::Video::disableCubemapping", "false" );
   Con::addVariableNotify( "$pref::Video::disableCubemapping", callabck2 );
   Con::setVariable( "$pref::Video::disableParallaxMapping", "false" );
   Con::addVariableNotify( "$pref::Video::disableParallaxMapping", callabck2 );
}

MaterialManager::~MaterialManager()
{
   GFXDevice::getDeviceEventSignal().remove( this, &MaterialManager::_handleGFXEvent );  
   LightManager::smActivateSignal.remove( this, &MaterialManager::_onLMActivate );

   SAFE_DELETE( mWarningInst );

#ifndef TORQUE_SHIPPING
   DebugMaterialMap::Iterator itr = mMeshDebugMaterialInsts.begin();

   for ( ; itr != mMeshDebugMaterialInsts.end(); itr++ )
      delete itr->value;
#endif
}

void MaterialManager::_onLMActivate( const char *lm, bool activate )
{
   if ( !activate )
      return;

   // Since the light manager usually swaps shadergen features
   // and changes system wide shader defines we need to completely
   // flush and rebuild all the material instances.

   mFlushAndReInit = true;
}

void MaterialManager::_updateDefaultAnisotropy()
{
   // Update all the materials.
   Vector<BaseMatInstance*>::iterator iter = mMatInstanceList.begin();
   for ( ; iter != mMatInstanceList.end(); iter++ )
      (*iter)->updateStateBlocks();
}

Material * MaterialManager::allocateAndRegister(const String &objectName, const String &mapToName)
{
   Material *newMat = new Material();

   if ( mapToName.isNotEmpty() )
      newMat->mMapTo = mapToName;

   bool registered = newMat->registerObject(objectName );
   AssertFatal( registered, "Unable to register material" );

   if (registered)
      Sim::getRootGroup()->addObject( newMat );
   else
   {
      delete newMat;
      newMat = NULL;
   }

   return newMat;
}

Material * MaterialManager::getMaterialDefinitionByName(const String &matName)
{
   // Get the material
   Material * foundMat;

   if(!Sim::findObject(matName, foundMat))
   {
      Con::errorf("MaterialManager: Unable to find material '%s'", matName.c_str());
      return NULL;
   }

   return foundMat;
}

BaseMatInstance* MaterialManager::createMatInstance(const String &matName)
{
   BaseMaterialDefinition* mat = NULL;
   if (Sim::findObject(matName, mat))
      return mat->createMatInstance();

   return NULL;
}

BaseMatInstance* MaterialManager::createMatInstance(  const String &matName, 
                                                      const GFXVertexFormat *vertexFormat )
{
   return createMatInstance( matName, getDefaultFeatures(), vertexFormat );
}

BaseMatInstance* MaterialManager::createMatInstance(  const String &matName, 
                                                      const FeatureSet& features, 
                                                      const GFXVertexFormat *vertexFormat )
{
   BaseMatInstance* mat = createMatInstance(matName);
   if (mat)
   {
      mat->init( features, vertexFormat );
      return mat;
   }

   return NULL;
}

BaseMatInstance  * MaterialManager::createWarningMatInstance()
{
   Material *warnMat = static_cast<Material*>(Sim::findObject("WarningMaterial"));

   BaseMatInstance   *warnMatInstance = NULL;

   if( warnMat != NULL )
   {
      warnMatInstance = warnMat->createMatInstance();

      GFXStateBlockDesc desc;
      desc.setCullMode(GFXCullNone);
      warnMatInstance->addStateBlockDesc(desc);

      warnMatInstance->init(  getDefaultFeatures(), 
                              getGFXVertexFormat<GFXVertexPNTTB>() );
   }

   return warnMatInstance;
}

// Gets the global warning material instance, callers should not free this copy
BaseMatInstance * MaterialManager::getWarningMatInstance()
{
   if (!mWarningInst)
      mWarningInst = createWarningMatInstance();

   return mWarningInst;
}

#ifndef TORQUE_SHIPPING
BaseMatInstance * MaterialManager::createMeshDebugMatInstance(const ColorF &meshColor)
{
   String  meshDebugStr = String::ToString( "Torque_MeshDebug_%d", meshColor.getRGBAPack() );

   Material *debugMat;
   if (!Sim::findObject(meshDebugStr,debugMat))
   {
      debugMat = allocateAndRegister( meshDebugStr );

      debugMat->mDiffuse[0] = meshColor;
      debugMat->mEmissive[0] = true;
   }

   BaseMatInstance   *debugMatInstance = NULL;

   if( debugMat != NULL )
   {
      debugMatInstance = debugMat->createMatInstance();

      GFXStateBlockDesc desc;
      desc.setCullMode(GFXCullNone);
      desc.fillMode = GFXFillWireframe;
      debugMatInstance->addStateBlockDesc(desc);

      // Disable fog and other stuff.
      FeatureSet debugFeatures;
      debugFeatures.addFeature( MFT_DiffuseColor );
      debugMatInstance->init( debugFeatures, getGFXVertexFormat<GFXVertexPCN>() );
   }

   return debugMatInstance;
}

// Gets the global material instance for a given color, callers should not free this copy
BaseMatInstance *MaterialManager::getMeshDebugMatInstance(const ColorF &meshColor)
{
   DebugMaterialMap::Iterator itr = mMeshDebugMaterialInsts.find( meshColor.getRGBAPack() );

   BaseMatInstance   *inst  = NULL;

   if ( itr == mMeshDebugMaterialInsts.end() )
      inst = createMeshDebugMatInstance( meshColor );
   else
      inst = itr->value;

   mMeshDebugMaterialInsts.insert( meshColor.getRGBAPack(), inst );

   return inst;
}
#endif

void MaterialManager::mapMaterial(const String & textureName, const String & materialName)
{
   if (getMapEntry(textureName).isNotEmpty())
   {
      if (!textureName.equal("unmapped_mat", String::NoCase))
         Con::warnf(ConsoleLogEntry::General, "Warning, overwriting material for: %s", textureName.c_str());
   }

   mMaterialMap[String::ToLower(textureName)] = materialName;
}

String MaterialManager::getMapEntry(const String & textureName) const
{
   MaterialMap::ConstIterator iter = mMaterialMap.find(String::ToLower(textureName));
   if ( iter == mMaterialMap.end() )
      return String();
   return iter->value;
}

void MaterialManager::flushAndReInitInstances()
{
   // Clear the flag if its set.
   mFlushAndReInit = false;   

   // Check to see if any shader preferences have changed.
   recalcFeaturesFromPrefs();

   // First we flush all the shader gen shaders which will
   // invalidate all GFXShader* to them.
   SHADERGEN->flushProceduralShaders();   
   mFlushSignal.trigger();

   // First do a pass deleting all hooks as they can contain
   // materials themselves.  This means we have to restart the
   // loop every time we delete any hooks... lame.
   Vector<BaseMatInstance*>::iterator iter = mMatInstanceList.begin();
   while ( iter != mMatInstanceList.end() )
   {
      if ( (*iter)->deleteAllHooks() != 0 )
      {
         // Restart the loop.
         iter = mMatInstanceList.begin();
         continue;
      }

      iter++;
   }

   // Now do a pass re-initializing materials.
   iter = mMatInstanceList.begin();
   for ( ; iter != mMatInstanceList.end(); iter++ )
      (*iter)->reInit();
}

// Used in the materialEditor. This flushes the material preview object so it can be reloaded easily.
void MaterialManager::flushInstance( BaseMaterialDefinition *target )
{
   Vector<BaseMatInstance*>::iterator iter = mMatInstanceList.begin();
   while ( iter != mMatInstanceList.end() )
   {
      if ( (*iter)->getMaterial() == target )
      {
		  (*iter)->deleteAllHooks();
		  return;
      }
	  iter++;
   }
}

void MaterialManager::reInitInstance( BaseMaterialDefinition *target )
{
   Vector<BaseMatInstance*>::iterator iter = mMatInstanceList.begin();
   for ( ; iter != mMatInstanceList.end(); iter++ )
   {
      if ( (*iter)->getMaterial() == target )
         (*iter)->reInit();
   }
}

void MaterialManager::updateTime()
{
   U32 curTime = Sim::getCurrentTime();
   if(curTime > mLastTime)
   {
      mDt =  (curTime - mLastTime) / 1000.0f;
      mLastTime = curTime;
      mAccumTime += mDt;
   }
   else
      mDt = 0.0f;
}

SimSet * MaterialManager::getMaterialSet()
{
   if(!mMaterialSet)
      mMaterialSet = static_cast<SimSet*>( Sim::findObject( "MaterialSet" ) );

   AssertFatal( mMaterialSet, "MaterialSet not found" );
   return mMaterialSet;
}

void MaterialManager::dumpMaterialInstances( BaseMaterialDefinition *target ) const
{
   if ( !mMatInstanceList.size() )
      return;

   if ( target )
      Con::printf( "--------------------- %s MatInstances ---------------------", target->getName() );
   else
      Con::printf( "--------------------- MatInstances %d ---------------------", mMatInstanceList.size() );

   for( U32 i=0; i<mMatInstanceList.size(); i++ )
   {
      BaseMatInstance *inst = mMatInstanceList[i];
      
      if ( target && inst->getMaterial() != target )
         continue;

      inst->dumpShaderInfo();

      Con::printf( "" );
   }

   Con::printf( "---------------------- Dump complete ----------------------");
}

void MaterialManager::_track( MatInstance *matInstance )
{
   mMatInstanceList.push_back( matInstance );
}

void MaterialManager::_untrack( MatInstance *matInstance )
{
   mMatInstanceList.remove( matInstance );
}

void MaterialManager::recalcFeaturesFromPrefs()
{
   mDefaultFeatures.clear();
   FeatureType::addDefaultTypes( &mDefaultFeatures );

   mExclusionFeatures.setFeature(   MFT_NormalMap, 
                                    Con::getBoolVariable( "$pref::Video::disableNormalMapping", false ) );

   mExclusionFeatures.setFeature(   MFT_SpecularMap,
                                    Con::getBoolVariable( "$pref::Video::disablePixSpecular", false ) );

   mExclusionFeatures.setFeature(   MFT_PixSpecular,
                                    Con::getBoolVariable( "$pref::Video::disablePixSpecular", false ) );

   mExclusionFeatures.setFeature(   MFT_CubeMap, 
                                    Con::getBoolVariable( "$pref::Video::disableCubemapping", false ) );

   mExclusionFeatures.setFeature(   MFT_Parallax, 
                                    Con::getBoolVariable( "$pref::Video::disableParallaxMapping", false ) );
}

bool MaterialManager::_handleGFXEvent( GFXDevice::GFXDeviceEventType event_ )
{
   switch ( event_ )
   {
      case GFXDevice::deInit:
         recalcFeaturesFromPrefs();
         break;

      case GFXDevice::deDestroy :
         SAFE_DELETE( mWarningInst );
         break;

      case GFXDevice::deStartOfFrame:
         if ( mFlushAndReInit )
            flushAndReInitInstances();
         break;

      default:
         break;
   }

   return true;
}

DefineConsoleFunction( reInitMaterials, void, (),,
   "@brief Flushes all procedural shaders and re-initializes all active material instances.\n\n" 
   "@ingroup Materials")
{
   MATMGR->flushAndReInitInstances();
}

DefineConsoleFunction( addMaterialMapping, void, (const char * texName, const char * matName), , "(string texName, string matName)\n"
   "@brief Maps the given texture to the given material.\n\n"
   "Generates a console warning before overwriting.\n\n"
   "Material maps are used by terrain and interiors for triggering "
   "effects when an object moves onto a terrain "
   "block or interior surface using the associated texture.\n\n"
   "@ingroup Materials")
{
   MATMGR->mapMaterial(texName, matName);
}

DefineConsoleFunction( getMaterialMapping, const char*, (const char * texName), , "(string texName)\n"
   "@brief Returns the name of the material mapped to this texture.\n\n"
   "If no materials are found, an empty string is returned.\n\n"
   "@param texName Name of the texture\n\n"
   "@ingroup Materials")
{
   return MATMGR->getMapEntry(texName).c_str();
}

DefineConsoleFunction( dumpMaterialInstances, void, (), ,
   "@brief Dumps a formatted list of currently allocated material instances to the console.\n\n"
   "@ingroup Materials")
{
   MATMGR->dumpMaterialInstances();
}

DefineConsoleFunction( getMapEntry, const char*, (const char * texName), ,
   "@hide")
{
	return MATMGR->getMapEntry( String(texName) );
}
