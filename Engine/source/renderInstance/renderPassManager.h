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
#ifndef _RENDERPASSMANAGER_H_
#define _RENDERPASSMANAGER_H_

#ifndef _GFXDEVICE_H_
#include "gfx/gfxDevice.h"
#endif
#ifndef _SCENEOBJECT_H_
#include "scene/sceneObject.h"
#endif
#ifndef _SIMOBJECT_H_
#include "console/simObject.h"
#endif
#ifndef _DATACHUNKER_H_
#include "core/dataChunker.h"
#endif
#ifndef _SCENEMANAGER_H_
#include "scene/sceneManager.h"
#endif

class SceneRenderState;
class ISceneObject;
class BaseMatInstance;
struct SceneData;
class ShaderData;
class RenderBinManager;
class LightInfo;
struct RenderInst;
class MatrixSet;
class GFXPrimitiveBufferHandle;

/// A RenderInstType hash value.
typedef U32 RenderInstTypeHash;

/// A a tiny wrapper around String that exposes a U32 operator so
/// that we can assign the RIT to RenderInst::type field.
class RenderInstType
{
   /// For direct access to mName.
   friend class RenderBinManager;

protected:
      
   String mName;

public:  

   RenderInstType()
      :  mName( Invalid.mName )
   {
   }

   RenderInstType( const RenderInstType &type )
      :  mName( type.mName )
   {
   }

   RenderInstType( const String &name )
      :  mName( name )
   {
   }

   ~RenderInstType() {}

   operator RenderInstTypeHash() const { return (RenderInstTypeHash)mName.getHashCaseInsensitive(); }

   const String& getName() const { return mName; }

   bool isValid() const { return (RenderInstTypeHash)*this != (RenderInstTypeHash)Invalid; }

   static const RenderInstType Invalid;
};


///
class RenderPassManager : public SimObject
{
   typedef SimObject Parent;

public:   

   // Default bin types.  Not necessarily the only bin types in the system.
   // RIT = "R"ender "I"nstance "T"ype
   static const RenderInstType RIT_Mesh;
   static const RenderInstType RIT_Shadow;
   static const RenderInstType RIT_Sky;
   static const RenderInstType RIT_Terrain;
   static const RenderInstType RIT_Object;   // objects that do their own rendering
   static const RenderInstType RIT_ObjectTranslucent;// self rendering; but sorted with static const RenderInstType RIT_Translucent
   static const RenderInstType RIT_Decal;
   static const RenderInstType RIT_DecalRoad;
   static const RenderInstType RIT_Water;
   static const RenderInstType RIT_Foliage;
   static const RenderInstType RIT_VolumetricFog;
   static const RenderInstType RIT_Translucent;
   static const RenderInstType RIT_Begin;
   static const RenderInstType RIT_Custom;
   static const RenderInstType RIT_Particle;
   static const RenderInstType RIT_Occluder;
   static const RenderInstType RIT_Editor;

public:

   RenderPassManager();
   virtual ~RenderPassManager();

   /// @name Allocation interface
   /// @{

   /// Allocate a render instance, use like so:  MyRenderInstType* t = gRenderInstMgr->allocInst<MyRenderInstType>();
   /// Valid until ::clear called.
   template <typename T>
   T* allocInst()
   {
      T* inst = mChunker.alloc<T>();
      inst->clear();
      return inst;
   }

   /// Allocate a matrix, valid until ::clear called.
   MatrixF* allocUniqueXform(const MatrixF& data) 
   { 
      MatrixF *r = mChunker.alloc<MatrixF>(); 
      *r = data; 
      return r; 
   }

   enum SharedTransformType
   {
      View,
      Projection,
   };

   const MatrixF* allocSharedXform(SharedTransformType stt);

   void assignSharedXform(SharedTransformType stt, const MatrixF &xfm);

   MatrixSet &getMatrixSet() { return *mMatrixSet; }

   /// Allocate a GFXPrimitive object which will remain valid 
   /// until the pass manager is cleared.
   GFXPrimitive* allocPrim() { return mChunker.alloc<GFXPrimitive>(); }
   /// @}

   /// Add a RenderInstance to the list
   virtual void addInst( RenderInst *inst );
   
   /// Sorts the list of RenderInst's per bin. (Normally, one should just call renderPass)
   void sort();

   /// Renders the list of RenderInsts (Normally, one should just call renderPass)
   void render( SceneRenderState *state );

   /// Resets our allocated RenderInstances and Matrices. (Normally, one should just call renderPass)
   void clear();

   // Calls sort, render, and clear
   void renderPass( SceneRenderState *state );

   /// Returns the active depth buffer for this pass (NOTE: This value may be GFXTextureTarget::sDefaultDepthStencil)
   GFXTextureObject *getDepthTargetTexture();

   /// Assigns the value for the above method
   void setDepthTargetTexture(GFXTextureObject *zTarget);

   /// @name RenderBinManager interface
   /// @{

   /// Add a render bin manager to the list of render bin manager, this SceneRenderPassManager now owns the render bin manager and will free it when needed.
   /// @param mgr Render manager to add
   /// @param processAddOrder Where to add the manager in the addInst list, set to NO_PROCESSADD to skip processing
   ///        this is in place for RenderManagers that will bypass the main ::addInst interface and doesn't want to process
   ///        them.
   /// @param renderOrder Where to add the manager in the render list.
   void addManager(RenderBinManager* mgr);
   
   /// Removes a manager from render and process add lists
   /// @param mgr Render bin manager to remove, the caller is now responsible for freeing the mgr.
   void removeManager(RenderBinManager* mgr);

   /// How many render bin managers do we have?
   U32 getManagerCount() const { return mRenderBins.size(); }

   /// Get the render manager at i
   RenderBinManager* getManager( S32 i ) const;

   /// @}

   /// Get scene manager which this render pass belongs to.
   SceneManager* getSceneManager()
   {
      if ( !mSceneManager )
         mSceneManager = gClientSceneGraph;

      return mSceneManager;
   }

   /// This signal is triggered when a render bin is about to be rendered.
   ///
   /// @param bin    The render bin we're signaling.
   /// @param state  The current scene state.
   /// @params preRender   If true it is before the bin is rendered, else its 
   ///                     after being rendered.
   ///
   typedef Signal <void (  RenderBinManager *bin, 
                           const SceneRenderState *state, 
                           bool preRender )> RenderBinEventSignal;

   /// @see RenderBinEventSignal
   static RenderBinEventSignal& getRenderBinSignal();


   typedef Signal<void(RenderInst *inst)> AddInstSignal;

   AddInstSignal& getAddSignal( RenderInstTypeHash type )
   {
      return mAddInstSignals.findOrInsert( type )->value; 
   }

   // ConsoleObject interface
   static void initPersistFields();
   DECLARE_CONOBJECT(RenderPassManager);

protected:

   MultiTypedChunker mChunker;
      
   Vector< RenderBinManager* > mRenderBins;


   typedef HashTable<RenderInstTypeHash,AddInstSignal> AddInstTable;

   AddInstTable mAddInstSignals;

   SceneManager * mSceneManager;
   GFXTexHandle mDepthBuff;
   MatrixSet *mMatrixSet;

   /// Do a sorted insert into a vector, renderOrder bool controls which test we run for insertion.
   void _insertSort(Vector<RenderBinManager*>& list, RenderBinManager* mgr, bool renderOrder);
};

//**************************************************************************
// Render Instance
//**************************************************************************
struct RenderInst
{
   /// The type of render instance this is.
   RenderInstTypeHash type;

   /// This should be true if the object needs to be sorted 
   /// back to front with other translucent instances.
   /// @see sortDistSq
   bool translucentSort;  

   /// The reference squared distance from the camera used for
   /// back to front sorting of the instances.
   /// @see translucentSort
   F32 sortDistSq;

   /// The default key used by render managers for
   /// internal sorting.
   U32 defaultKey;

   /// The secondary key used by render managers for
   /// internal sorting.
   U32 defaultKey2;

   /// Does a memset to clear the render instance.
   void clear();
};

struct ObjectRenderInst : public RenderInst
{
   /// This is a delegate specific index which is usually
   /// used to define a mounted object.
   S32 objectIndex;

   /// Extra data to be used within the render callback.
   /// ObjectRenderInst does not own or cleanup this data.
   void *userData;

   /// The delegate callback function to call to render
   /// this object instance.
   ///
   /// @param ri           The ObjectRenderInst that called the delegate.
   ///
   /// @param state        The scene state we're rendering.
   ///
   /// @param overrideMat  An alternative material to use during rendering... usually
   ///                     used for special renders like shadows.  If the object doesn't
   ///                     support override materials it shouldn't render at all.
   Delegate<void( ObjectRenderInst *ri, 
                  SceneRenderState *state, 
                  BaseMatInstance *overrideMat )> renderDelegate;

   // Clear this instance.
   void clear();
};

struct MeshRenderInst : public RenderInst
{
   ////
   GFXVertexBufferHandleBase *vertBuff;
   
   ////
   GFXPrimitiveBufferHandle *primBuff;

   /// If not NULL it is used to draw the primitive, else
   /// the primBuffIndex is used.
   /// @see primBuffIndex
   GFXPrimitive *prim;

   /// If prim is NULL then this index is used to draw the
   /// indexed primitive from the primitive buffer.
   /// @see prim
   U32 primBuffIndex;
   
   /// The material to setup when drawing this instance.
   BaseMatInstance *matInst;

   /// The object to world transform (world transform in most API's).
   const MatrixF *objectToWorld;       
   
   /// The worldToCamera (view transform in most API's).
   const MatrixF* worldToCamera;       
   
   /// The projection matrix.
   const MatrixF* projection;         

   // misc render states
   U8    transFlags;
   bool  reflective;
   F32   visibility;

   /// A generic hint value passed from the game
   /// code down to the material for use by shader 
   /// features.
   void *materialHint;

   /// The lights we pass to the material for this 
   /// mesh in order light importance.
   LightInfo* lights[8];

   // textures
   GFXTextureObject *lightmap;
   GFXTextureObject *fogTex;
   GFXTextureObject *backBuffTex;
   GFXTextureObject *reflectTex;
   GFXTextureObject *miscTex;
   GFXTextureObject *accuTex;
   GFXCubemap   *cubemap;

   /// @name Hardware Skinning
   /// {
   MatrixF *mNodeTransforms;
   U32 mNodeTransformCount;
   /// }

#ifdef TORQUE_ENABLE_GFXDEBUGEVENTS
   const char *meshName;
   const char *objectName;
#endif

   void clear();
};

enum ParticleSystemState
{
   PSS_AwaitingHighResDraw = 0, // Keep this as first element so that if the offscreen manager rejects a particle system it will get drawn high-res
   PSS_AwaitingOffscreenDraw,
   PSS_AwaitingCompositeDraw,
   PSS_AwaitingMixedResDraw,
   PSS_DrawComplete,
};

/// A special render instance for particles.
struct ParticleRenderInst : public RenderInst
{
   /// The vertex buffer.
   GFXVertexBufferHandleBase *vertBuff;
   
   /// The primitive buffer.
   GFXPrimitiveBufferHandle *primBuff;

   /// The total particle count to render.
   S32 count;

   bool glow;

   /// The combined model, camera, and projection transform.
   const MatrixF *modelViewProj;       
        
   /// Blend style for the particle system 
   enum BlendStyle {
      BlendUndefined = 0,
      BlendNormal,
      BlendAdditive,
      BlendSubtractive,
      BlendPremultAlpha,
      BlendGreyscale,
      BlendStyle_COUNT,
   };
   U8 blendStyle;

   /// For the offscreen particle manager
   U8 targetIndex;

   /// State for the particle system
   ParticleSystemState systemState;

   /// The soft particle fade distance in meters.
   F32 softnessDistance;

   /// Bounding box render transform
   const MatrixF *bbModelViewProj;

   /// The particle texture.
   GFXTextureObject *diffuseTex;

   void clear();
};

class GFXOcclusionQuery;
class SceneObject;

/// A special render instance for occlusion tests.
struct OccluderRenderInst : public RenderInst
{   
   Point3F scale;
   Point3F position;   
   const MatrixF *orientation;
   GFXOcclusionQuery *query; 
   
   // This optional query will have all pixels rendered.
   // Its purpose is to return to the user the full pixel count for comparison
   // with the other query.
   GFXOcclusionQuery *query2;

   /// Render a sphere or a box.
   bool isSphere;

   void clear();
};

#endif // _RENDERPASSMANAGER_H_
