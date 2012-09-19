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
#ifndef _BASEMATINSTANCE_H_
#define _BASEMATINSTANCE_H_

#ifndef _TSIGNAL_H_
#include "core/util/tSignal.h"
#endif
#ifndef _BASEMATERIALDEFINITION_H_
#include "materials/baseMaterialDefinition.h"
#endif
#ifndef _MATERIALPARAMETERS_H_
#include "materials/materialParameters.h"
#endif
#ifndef _MMATRIX_H_
#include "math/mMatrix.h"
#endif
#ifndef _GFXENUMS_H_
#include "gfx/gfxEnums.h"
#endif
#ifndef _GFXSHADER_H_
#include "gfx/gfxShader.h"
#endif
#ifndef _MATERIALFEATUREDATA_H_
#include "materials/materialFeatureData.h"
#endif
#ifndef _MATINSTANCEHOOK_H_
#include "materials/matInstanceHook.h"
#endif
#ifndef _MATSTATEHINT_H_
#include "materials/matStateHint.h"
#endif

struct RenderPassData;
class GFXVertexBufferHandleBase;
class GFXPrimitiveBufferHandle;
struct SceneData;
class SceneRenderState;
struct GFXStateBlockDesc;
class GFXVertexFormat;
class MatrixSet;
class ProcessedMaterial;


///
class BaseMatInstance
{
protected:

   /// The array of active material hooks indexed 
   /// by a MatInstanceHookType.
   Vector<MatInstanceHook*> mHooks;

   ///
   MatFeaturesDelegate mFeaturesDelegate;

   /// Should be true if init has been called and it succeeded.
   /// It is up to the derived class to set this variable appropriately.
   bool mIsValid;

   /// This is set by initialization and used by the prepass.
   bool mHasNormalMaps;

public:

   virtual ~BaseMatInstance();

   /// @param features The features you want to allow for this material.  
   ///
   /// @param vertexFormat The vertex format on which this material will be rendered.
   ///
   /// @see GFXVertexFormat
   /// @see FeatureSet
   virtual bool init(   const FeatureSet &features, 
                        const GFXVertexFormat *vertexFormat ) = 0;

   /// Reinitializes the material using the previous
   /// initialization parameters.
   /// @see init
   virtual bool reInit() = 0;

   /// Returns true if init has been successfully called.
   /// It is up to the derived class to set this value properly.
   bool isValid() { return mIsValid; }

   /// Adds this stateblock to the base state block 
   /// used during initialization.
   /// @see init
   virtual void addStateBlockDesc(const GFXStateBlockDesc& desc) = 0;

   /// Updates the state blocks for this material.
   virtual void updateStateBlocks() = 0;

   /// Adds a shader macro which will be passed to the shader
   /// during initialization.
   /// @see init
   virtual void addShaderMacro( const String &name, const String &value ) = 0;

   /// Get a MaterialParameters block for this BaseMatInstance, 
   /// caller is responsible for freeing it.
   virtual MaterialParameters* allocMaterialParameters() = 0;

   /// Set the current parameters for this BaseMatInstance
   virtual void setMaterialParameters(MaterialParameters* param) = 0;

   /// Get the current parameters for this BaseMatInstance (BaseMatInstances are created with a default active
   /// MaterialParameters which is managed by BaseMatInstance.
   virtual MaterialParameters* getMaterialParameters() = 0;

   /// Returns a MaterialParameterHandle for name.
   virtual MaterialParameterHandle* getMaterialParameterHandle(const String& name) = 0;

   /// Sets up the next rendering pass for this material.  It is
   /// typically called like so...
   ///
   ///@code
   ///   while( mat->setupPass( state, sgData ) )
   ///   {
   ///      mat->setTransforms(...);
   ///      mat->setSceneInfo(...);
   ///      ...
   ///      GFX->drawPrimitive();
   ///   }
   ///@endcode
   ///
   virtual bool setupPass( SceneRenderState *state, const SceneData &sgData ) = 0;
   
   /// This initializes the material transforms and should be 
   /// called after setupPass() within the pass loop.
   /// @see setupPass
   virtual void setTransforms( const MatrixSet &matrixSet, SceneRenderState *state ) = 0;

   /// This initializes various material scene state settings and
   /// should be called after setupPass() within the pass loop.
   /// @see setupPass
   virtual void setSceneInfo( SceneRenderState *state, const SceneData &sgData ) = 0;

   /// This is normally called from within setupPass() automatically, so its
   /// unnecessary to do so manually unless a texture stage has changed.  If
   /// so it should be called after setupPass() within the pass loop.
   /// @see setupPass
   virtual void setTextureStages(SceneRenderState *, const SceneData &sgData ) = 0;

   /// Sets the vertex and primitive buffers as well as the instancing 
   /// stream buffer for the current material if the material is instanced.
   virtual void setBuffers( GFXVertexBufferHandleBase *vertBuffer, GFXPrimitiveBufferHandle *primBuffer ) = 0;

   /// Returns true if this material is instanced.
   virtual bool isInstanced() const = 0;

   /// Used to increment the instance buffer for this material.
   virtual bool stepInstance() = 0;

   /// Returns true if the material is forward lit and requires
   /// a list of lights which affect it when rendering.
   virtual bool isForwardLit() const = 0;

   /// Sets a SimObject which will passed into ShaderFeature::createConstHandles.
   /// Normal features do not make use of this, it is for special class specific
   /// or user designed features.
   virtual void setUserObject( SimObject *userObject ) = 0;
   virtual SimObject* getUserObject() const = 0;

   /// Returns the material this instance is based on.
   virtual BaseMaterialDefinition* getMaterial() = 0;

   // BTRTODO: This stuff below should probably not be in BaseMatInstance
   virtual bool hasGlow() = 0;
   
   virtual U32 getCurPass() = 0;

   virtual U32 getCurStageNum() = 0;

   virtual RenderPassData *getPass(U32 pass) = 0;

   /// Returns the state hint which can be used for 
   /// sorting and fast comparisions of the equality 
   /// of a material instance.
   virtual const MatStateHint& getStateHint() const = 0;

   /// Returns the active features in use by this material.
   /// @see getRequestedFeatures
   virtual const FeatureSet& getFeatures() const = 0;

   /// Returns the features that were requested at material
   /// creation time which may differ from the active features.
   /// @see getFeatures
   virtual const FeatureSet& getRequestedFeatures() const = 0;

   virtual const GFXVertexFormat* getVertexFormat() const = 0;

   virtual void dumpShaderInfo() const = 0;

   /// Fast test for use of normal maps in this material.
   bool hasNormalMap() const { return mHasNormalMaps; }

   ///
   MatFeaturesDelegate& getFeaturesDelegate() { return mFeaturesDelegate; }

   /// Returns true if this MatInstance is built from a CustomMaterial.
   virtual bool isCustomMaterial() const = 0;

   /// @name Material Hook functions
   /// @{

   ///
   void addHook( MatInstanceHook *hook );

   /// Helper function for getting a hook.
   /// @see getHook
   template <class HOOK>
   inline HOOK* getHook() { return (HOOK*)getHook( HOOK::Type ); }

   ///
   MatInstanceHook* getHook( const MatInstanceHookType &type ) const;

   ///
   void deleteHook( const MatInstanceHookType &type );

   ///
   U32 deleteAllHooks();

   /// @}

   virtual const GFXStateBlockDesc &getUserStateBlock() const = 0;

};

#endif /// _BASEMATINSTANCE_H_






