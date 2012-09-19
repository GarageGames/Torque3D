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
#ifndef _MATERIAL_MGR_H_
#define _MATERIAL_MGR_H_

#ifndef _MATERIALDEFINITION_H_
#include "materials/materialDefinition.h"
#endif
#ifndef _FEATURESET_H_
#include "shaderGen/featureSet.h"
#endif
#ifndef _GFXDEVICE_H_
#include "gfx/gfxDevice.h"
#endif
#ifndef _TSINGLETON_H_
#include "core/util/tSingleton.h"
#endif

class SimSet;
class MatInstance;

class MaterialManager : public ManagedSingleton<MaterialManager>
{
public:
   MaterialManager();
   ~MaterialManager();

   // ManagedSingleton
   static const char* getSingletonName() { return "MaterialManager"; }

   Material * allocateAndRegister(const String &objectName, const String &mapToName = String());
   Material * getMaterialDefinitionByName(const String &matName);
   SimSet * getMaterialSet();   

   // map textures to materials
   void mapMaterial(const String & textureName, const String & materialName);
   String getMapEntry(const String & textureName) const;

   // Return instance of named material caller is responsible for memory
   BaseMatInstance * createMatInstance( const String &matName );

   // Create a BaseMatInstance with the default feature flags. 
   BaseMatInstance * createMatInstance( const String &matName, const GFXVertexFormat *vertexFormat );
   BaseMatInstance * createMatInstance( const String &matName, const FeatureSet &features, const GFXVertexFormat *vertexFormat );

   /// The default feature set for materials.
   const FeatureSet& getDefaultFeatures() const { return mDefaultFeatures; }

   /// The feature exclusion list for disabling features.
   const FeatureSet& getExclusionFeatures() const { return mExclusionFeatures; }

   void recalcFeaturesFromPrefs();

   /// Get the default texture anisotropy.
   U32 getDefaultAnisotropy() const { return mDefaultAnisotropy; }

   /// Allocate and return an instance of special materials.  Caller is responsible for the memory.
   BaseMatInstance * createWarningMatInstance();

   /// Gets the global warning material instance, callers should not free this copy
   BaseMatInstance * getWarningMatInstance();

   /// Set the prepass enabled state.
   void setPrePassEnabled( bool enabled ) { mUsingPrePass = enabled; }

   /// Get the prepass enabled state.
   bool getPrePassEnabled() const { return mUsingPrePass; }

#ifndef TORQUE_SHIPPING

   // Allocate and return an instance of mesh debugging materials.  Caller is responsible for the memory.
   BaseMatInstance * createMeshDebugMatInstance(const ColorF &meshColor);

   // Gets the global material instance for a given color, callers should not free this copy
   BaseMatInstance * getMeshDebugMatInstance(const ColorF &meshColor);

#endif

   void dumpMaterialInstances( BaseMaterialDefinition *target = NULL ) const;

   void updateTime();
   F32 getTotalTime() const { return mAccumTime; }
   F32 getDeltaTime() const { return mDt; }
   U32 getLastUpdateTime() const { return mLastTime; }

   /// Signal used to notify systems that 
   /// procedural shaders have been flushed.
   typedef Signal<void()> FlushSignal;

   /// Returns the signal used to notify systems that the 
   /// procedural shaders have been flushed.
   FlushSignal& getFlushSignal() { return mFlushSignal; }

   /// Flushes all the procedural shaders and re-initializes all
   /// the active materials instances immediately.
   void flushAndReInitInstances();

   // Flush the instance
   void flushInstance( BaseMaterialDefinition *target );

   /// Re-initializes the material instances for a specific target material.   
   void reInitInstance( BaseMaterialDefinition *target );

protected:

   // MatInstance tracks it's instances here
   friend class MatInstance;
   void _track(MatInstance*);
   void _untrack(MatInstance*);

   /// @see LightManager::smActivateSignal
   void _onLMActivate( const char *lm, bool activate );

   bool _handleGFXEvent(GFXDevice::GFXDeviceEventType event);

   SimSet* mMaterialSet;
   Vector<BaseMatInstance*> mMatInstanceList;

   /// The default material features.
   FeatureSet mDefaultFeatures;

   /// The feature exclusion set.
   FeatureSet mExclusionFeatures;

   /// Signal used to notify systems that 
   /// procedural shaders have been flushed.
   FlushSignal mFlushSignal;

   /// If set we flush and reinitialize all materials at the
   /// start of the next rendered frame.
   bool mFlushAndReInit;

   // material map
   typedef Map<String, String> MaterialMap;
   MaterialMap mMaterialMap;

   bool mUsingPrePass;

   // time tracking
   F32 mDt;
   F32 mAccumTime;
   U32 mLastTime;

   BaseMatInstance* mWarningInst;

   /// The default max anisotropy used in texture filtering.
   S32 mDefaultAnisotropy;

   /// Called when $pref::Video::defaultAnisotropy is changed.
   void _updateDefaultAnisotropy();

   /// Called when one of the feature disabling $pref::s are changed.
   void _onDisableMaterialFeature() { mFlushAndReInit = true; }

#ifndef TORQUE_SHIPPING
   typedef Map<U32, BaseMatInstance *>  DebugMaterialMap;
   DebugMaterialMap  mMeshDebugMaterialInsts;
#endif

};

/// Helper for accessing MaterialManager singleton.
#define MATMGR MaterialManager::instance()

#endif // _MATERIAL_MGR_H_
