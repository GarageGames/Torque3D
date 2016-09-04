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

#ifndef _TSSHAPEINSTANCE_H_
#define _TSSHAPEINSTANCE_H_

#ifndef __RESOURCE_H__
#include "core/resource.h"
#endif
#ifndef _TSSHAPE_H_
#include "ts/tsShape.h"
#endif
#ifndef _TSINTEGERSET_H_
#include "ts/tsIntegerSet.h"
#endif
#ifndef _CONSOLE_H_
#include "console/console.h"
#endif
#ifndef _GBITMAP_H_
#include "gfx/bitmap/gBitmap.h"
#endif
#ifndef _TSRENDERDATA_H_
#include "ts/tsRenderState.h"
#endif
#ifndef _TSMATERIALLIST_H_
#include "ts/tsMaterialList.h"
#endif

class RenderItem;
class TSThread;
class ConvexFeature;
class SceneRenderState;
class FeatureSet;


//-------------------------------------------------------------------------------------
// Instance versions of shape objects
//-------------------------------------------------------------------------------------

class TSCallback
{
public:
   virtual ~TSCallback() {}
   
   virtual void setNodeTransform(TSShapeInstance * si, S32 nodeIndex, MatrixF & localTransform) = 0;
};

/// An instance of a 3space shape.
///
/// @section TSShapeInstance_intro Introduction
///
/// A 3space model represents a significant amount of data. There are multiple meshes,
/// skeleton information, as well as animation data. Some of this, like the skeletal
/// transforms, are unique for each instance of the model (as different instances are
/// likely to be in different states of animation), while most of it, like texturing
/// information and vertex data, is the same amongst all instances of the shape.
///
/// To keep this data from being replicated for every instance of a 3shape object, Torque
/// uses the ResManager to instantiate and track TSShape objects. TSShape handles reading
/// and writing 3space models, as well as keeping track of static model data, as discussed
/// above. TSShapeInstance keeps track of all instance specific data, such as the currently
/// playing sequences or the active node transforms.
///
/// TSShapeInstance contains all the functionality for 3space models, while TSShape acts as
/// a repository for common data.
///
/// @section TSShapeInstance_functionality What Does TSShapeInstance Do?
///
/// TSShapeInstance handles several areas of functionality:
///      - Collision.
///      - Rendering.
///      - Animation.
///      - Updating skeletal transforms.
///      - Ballooning (see setShapeBalloon() and getShapeBalloon())
///
/// For an excellent example of how to render a TSShape in game, see TSStatic. For examples
/// of how to procedurally animate models, look at Player::updateLookAnimation().
class TSShapeInstance
{
   public:

   struct ObjectInstance;
   friend class TSThread;
   friend class TSLastDetail;
   friend class TSPartInstance;

   /// Base class for all renderable objects, including mesh objects and decal objects.
   ///
   /// An ObjectInstance points to the renderable items in the shape...
   struct ObjectInstance
   {
      virtual ~ObjectInstance() {}
   
      /// this needs to be set before using an objectInstance...tells us where to
      /// look for the transforms...gets set be shape instance 'setStatics' method
      const Vector<MatrixF> *mTransforms;

      S32 nodeIndex;
     
      /// Gets the transform of this object
      inline const MatrixF& getTransform() const
      {
         return nodeIndex < 0 ? MatrixF::Identity : (*mTransforms)[ nodeIndex ];
      }

     /// @name Render Functions
     /// @{

     /// Render!  This draws the base-textured object.
      virtual void render( S32 objectDetail, TSVertexBufferHandle &vb, TSMaterialList *, TSRenderState &rdata, F32 alpha, const char *meshName );

     /// Updates the vertex buffer data for this mesh (used for software skinning)
      virtual void updateVertexBuffer( S32 objectDetail, U8 *buffer );
      virtual bool bufferNeedsUpdate( S32 objectDetail );
     /// @}

     /// @name Collision Routines
     /// @{

      virtual bool buildPolyList( S32 objectDetail, AbstractPolyList *polyList, U32 &surfaceKey, TSMaterialList *materials );
      virtual bool getFeatures( S32 objectDetail, const MatrixF &mat, const Point3F &n, ConvexFeature *feature, U32 &surfaceKey );
      virtual void support( S32 od, const Point3F &v, F32 *currMaxDP, Point3F *currSupport );

      virtual bool buildPolyListOpcode( S32 objectDetail, AbstractPolyList *polyList, U32 &surfaceKey, TSMaterialList *materials );
      virtual bool castRayOpcode( S32 objectDetail, const Point3F &start, const Point3F &end, RayInfo *info, TSMaterialList *materials );
      virtual bool buildConvexOpcode( const MatrixF &mat, S32 objectDetail, const Box3F &bounds, Convex *c, Convex *list );

      /// Ray cast for collision detection
     virtual bool castRay( S32 objectDetail, const Point3F &start, const Point3F &end, RayInfo *info, TSMaterialList* materials ) = 0;
     /// @}
   };

   /// These are set up by default based on shape data
   struct MeshObjectInstance : ObjectInstance
   {
      TSMesh * const * meshList; ///< one mesh per detail level... Null entries allowed.
      const TSObject * object;
      S32 frame;
      S32 matFrame;
      F32 visible;
      
      /// If true this mesh is forced to be hidden
      /// regardless of the animation state.
      bool forceHidden;
      
      /// The time at which this mesh 
      /// was last rendered.
      U32 mLastTime;

      Vector<MatrixF> mActiveTransforms;

      MeshObjectInstance();
      virtual ~MeshObjectInstance() {}

      void render( S32 objectDetail, TSVertexBufferHandle &vb, TSMaterialList *, TSRenderState &rdata, F32 alpha, const char *meshName );

      void updateVertexBuffer( S32 objectDetail, U8 *buffer );

      bool bufferNeedsUpdate(S32 objectDetail);

      /// Gets the mesh with specified detail level
      TSMesh * getMesh(S32 num) const { return num<object->numMeshes ? *(meshList+num) : NULL; }

     /// @name Collision Routines
     /// @{

      bool buildPolyList( S32 objectDetail, AbstractPolyList *polyList, U32 &surfaceKey, TSMaterialList *materials );
      bool getFeatures( S32 objectDetail, const MatrixF &mat, const Point3F &n, ConvexFeature *feature, U32 &surfaceKey );
      void support( S32 od, const Point3F &v, F32 *currMaxDP, Point3F *currSupport );
      bool castRay( S32 objectDetail, const Point3F &start, const Point3F &end, RayInfo *info, TSMaterialList *materials );
      bool castRayRendered( S32 objectDetail, const Point3F &start, const Point3F &end, RayInfo *info, TSMaterialList *materials );

      bool buildPolyListOpcode( S32 objectDetail, AbstractPolyList *polyList, const Box3F &box, TSMaterialList* materials );
      bool castRayOpcode( S32 objectDetail, const Point3F &start, const Point3F &end, RayInfo *info, TSMaterialList *materials );
      bool buildConvexOpcode( const MatrixF &mat, S32 objectDetail, const Box3F &bounds, Convex *c, Convex *list );

     /// @}
   };

   protected:

   struct TSCallbackRecord
   {
      TSCallback * callback;
      S32 nodeIndex;
   };

//-------------------------------------------------------------------------------------
// Lists used for storage of transforms, nodes, objects, etc...
//-------------------------------------------------------------------------------------

   public:

   Vector<MeshObjectInstance> mMeshObjects;

   /// storage space for node transforms
   Vector<MatrixF> mNodeTransforms;

   /// @name Reference Transform Vectors
   /// unused until first transition
   /// @{
   Vector<Quat16>         mNodeReferenceRotations;
   Vector<Point3F>        mNodeReferenceTranslations;
   Vector<F32>            mNodeReferenceUniformScales;
   Vector<Point3F>        mNodeReferenceScaleFactors;
   Vector<Quat16>         mNodeReferenceArbitraryScaleRots;
   /// @}

   /// @name Workspace for Node Transforms
   /// @{
   static Vector<QuatF>   smNodeCurrentRotations;
   static Vector<Point3F> smNodeCurrentTranslations;
   static Vector<F32>     smNodeCurrentUniformScales;
   static Vector<Point3F> smNodeCurrentAlignedScales;
   static Vector<TSScale> smNodeCurrentArbitraryScales;
   static Vector<MatrixF> smNodeLocalTransforms;
   static TSIntegerSet    smNodeLocalTransformDirty;
   /// @}

   /// @name Threads
   /// keep track of who controls what on currently animating shape
   /// @{
   static Vector<TSThread*> smRotationThreads;
   static Vector<TSThread*> smTranslationThreads;
   static Vector<TSThread*> smScaleThreads;
   /// @}
	
	TSMaterialList* mMaterialList;    ///< by default, points to hShape material list
//-------------------------------------------------------------------------------------
// Misc.
//-------------------------------------------------------------------------------------

protected:

   /// @name Ground Transform Data
   /// @{
   MatrixF mGroundTransform;
   TSThread * mGroundThread;
   /// @}

   bool mScaleCurrentlyAnimated;

   S32 mCurrentDetailLevel;

   /// 0-1, how far along from current to next (higher) detail level...
   ///
   /// 0=at this dl, 1=at higher detail level, where higher means bigger size on screen
   /// for dl=0, we use twice detail level 0's size as the size of the "next" dl
   F32 mCurrentIntraDetailLevel;

   /// This is only valid when the instance was created from
   /// a resource.  Else it is null.
   Resource<TSShape> mShapeResource;

   /// This should always point to a valid shape and should
   /// equal mShapeResource if it was created from a resource.
   TSShape *mShape;

   /// Vertex buffer used for software skinning this instance
   TSVertexBufferHandle mSoftwareVertexBuffer;

   bool            mOwnMaterialList; ///< Does this own the material list pointer?
   bool            mUseOwnBuffer; ///< Force using our own copy of the vertex buffer

   bool           mAlphaAlways;
   F32            mAlphaAlwaysValue;

   bool          mUseOverrideTexture;

   U32 debrisRefCount;

   // the threads...
   Vector<TSThread*> mThreadList;
   Vector<TSThread*> mTransitionThreads;

   /// @name Transition nodes
   /// keep track of nodes that are involved in a transition
   ///
   /// @note this only tracks nodes we're transitioning from...
   /// nodes we're transitioning to are implicitly handled
   /// (i.e., we don't need to keep track of them)
   /// @{

   TSIntegerSet mTransitionRotationNodes;
   TSIntegerSet mTransitionTranslationNodes;
   TSIntegerSet mTransitionScaleNodes;
   /// @}

   /// keep track of nodes with animation restrictions put on them
   TSIntegerSet mMaskRotationNodes;
   TSIntegerSet mMaskPosXNodes;
   TSIntegerSet mMaskPosYNodes;
   TSIntegerSet mMaskPosZNodes;
   TSIntegerSet mDisableBlendNodes;
   TSIntegerSet mHandsOffNodes;        ///< Nodes that aren't animated through threads automatically
   TSIntegerSet mCallbackNodes;

   // node callbacks
   Vector<TSCallbackRecord> mNodeCallbacks;

   /// state variables
   U32 mTriggerStates;

   bool initGround();
   void addPath(TSThread * gt, F32 start, F32 end, MatrixF * mat = NULL);

   public:

   TSShape* getShape() const { return mShape; }

   TSMaterialList* getMaterialList() const { return mMaterialList; }
   
   /// Set the material list without taking ownership.
   /// @see cloneMaterialList
   void setMaterialList( TSMaterialList *matList );

   /// Call this to own the material list -- i.e., we'll make a copy of the 
   /// currently set material list and be responsible for deleting it.  You
   /// can pass an optional feature set for initializing the cloned materials.
   void cloneMaterialList( const FeatureSet *features = NULL ); 

   /// Initializes or re-initializes the material list with 
   /// an optional feature set.
   void initMaterialList(  const FeatureSet *features = NULL );

   void setUseOwnBuffer();
   bool ownMaterialList() const { return mOwnMaterialList; }

   /// Get the number of material targets in this shape instance
   S32 getTargetCount() const
   {
      if ( mOwnMaterialList )
         return getMaterialList()->size();
      else
         return getShape()->getTargetCount();
   }

   /// Get the indexed material target (may differ from the base TSShape material
   /// list if this instance has been reskinned).
   const String& getTargetName( S32 mapToNameIndex ) const
   {
      if ( mOwnMaterialList )
      {
         if ( mapToNameIndex < 0 || mapToNameIndex >= getMaterialList()->size() )
            return String::EmptyString;

         return getMaterialList()->getMaterialName( mapToNameIndex );
      }
      else
      {
         return getShape()->getTargetName( mapToNameIndex );
      }
   }

   void reSkin( String newBaseName, String oldBaseName = String::EmptyString );

   enum
   {
      MaskNodeRotation       = 0x01,
      MaskNodePosX           = 0x02,
      MaskNodePosY           = 0x04,
      MaskNodePosZ           = 0x08,
      MaskNodeBlend          = 0x10,
      MaskNodeAll            = MaskNodeRotation|MaskNodePosX|MaskNodePosY|MaskNodePosZ|MaskNodeBlend,
      MaskNodeAllButBlend    = MaskNodeRotation|MaskNodePosX|MaskNodePosY|MaskNodePosZ,
      MaskNodeAllButRotation = MaskNodePosX|MaskNodePosY|MaskNodePosZ|MaskNodeBlend,
      MaskNodeAllButPosX     = MaskNodeRotation|MaskNodePosY|MaskNodePosZ|MaskNodeBlend,
      MaskNodeAllButPosY     = MaskNodeRotation|MaskNodePosX|MaskNodePosZ|MaskNodeBlend,
      MaskNodeAllButPosZ     = MaskNodeRotation|MaskNodePosX|MaskNodePosY|MaskNodeBlend,
      MaskNodeHandsOff       = 0x20, ///< meaning, don't even set to default, programmer controls it (blend still applies)
      MaskNodeCallback       = 0x40  ///< meaning, get local transform via callback function (see setCallback)
                                     ///< callback data2 is node index, callback return value is pointer to local transform
                                     ///< Note: won't get this callback everytime you animate...application responsibility
                                     ///< to make sure matrix pointer continues to point to valid and updated local transform
   };
   /// @name Node Masking
   /// set node masking...
   /// @{
   void setNodeAnimationState(S32 nodeIndex, U32 animationState, TSCallback * callback = NULL);
   U32  getNodeAnimationState(S32 nodeIndex);
   /// @}

   /// @name Trigger states
   /// check trigger value
   /// @{
   bool getTriggerState(U32 stateNum, bool clearState = true);
   void setTriggerState(U32 stateNum, bool on);
   void setTriggerStateBit(U32 stateBit, bool on);
   /// @}

   /// @name Debris Management
   /// @{
   void incDebrisRefCount() { ++debrisRefCount; }
   void decDebrisRefCount() { debrisRefCount > 0 ? --debrisRefCount : 0; }
   U32 getDebrisRefCount() const { return debrisRefCount; }
   /// @}

   /// @name AlphaAlways
   /// AlphaAlways allows the entire model to become translucent at the same value
   /// @{
   void setAlphaAlways(F32 value) { mAlphaAlways = (value<0.99f); mAlphaAlwaysValue = value; }
   F32 getAlphaAlwaysValue() const { return mAlphaAlways ? mAlphaAlwaysValue : 1.0f; }
   bool getAlphaAlways() const { return mAlphaAlways; }
   /// @}

//-------------------------------------------------------------------------------------
// private methods for setting up and affecting animation
//-------------------------------------------------------------------------------------

   private:

   /// @name Private animation methods
   /// These are private methods for setting up and affecting animation
   /// @{
   void sortThreads();

   void updateTransitions();
   void handleDefaultScale(S32 a, S32 b, TSIntegerSet & scaleBeenSet);
   void updateTransitionNodeTransforms(TSIntegerSet& transitionNodes);
   void handleTransitionNodes(S32 a, S32 b);
   void handleNodeScale(S32 a, S32 b);
   void handleAnimatedScale(TSThread *, S32 a, S32 b, TSIntegerSet &);
   void handleMaskedPositionNode(TSThread *, S32 nodeIndex, S32 offset);
   void handleBlendSequence(TSThread *, S32 a, S32 b);
   void checkScaleCurrentlyAnimated();
   /// @}

//-------------------------------------------------------------------------------------
// animate, render, & detail control
//-------------------------------------------------------------------------------------

   public:

   struct RenderData
   {
      MeshObjectInstance* currentObjectInstance;

      S32 detailLevel;
      S32 materialIndex;
      const Point3F * objectScale;
   };

   /// Scale pixel size by this amount when selecting
   /// detail levels.
   static F32 smDetailAdjust;

   /// If this is set to a positive pixel value shapes
   /// with a smaller pixel size than this will skip 
   /// rendering entirely.
   static F32 smSmallestVisiblePixelSize;

   /// never choose detail level number below this value (except if
   /// only way to get a visible detail)
   static S32 smNumSkipRenderDetails;

   /// For debugging / metrics.
   static F32 smLastScreenErrorTolerance;
   static F32 smLastScaledDistance;
   static F32 smLastPixelSize;

   /// Debugging
   /// @{

   /// Renders the vertex normals assuming the GFX state
   /// is setup for rendering in model space.
   void renderDebugNormals( F32 normalScalar, S32 dl );

   /// Render all node transforms as small axis gizmos.  It is recommended
   /// that prior to calling this, shapeInstance::animate is called so that 
   /// nodes are in object space and that the GFX state is setup for
   /// rendering in model space.
   void renderDebugNodes();

   /// Print mesh data to the console, valid String parameters
   /// are Visible, Hidden, or All.
   void listMeshes( const String &state ) const;

   /// @}

   void render( const TSRenderState &rdata );
   void render( const TSRenderState &rdata, S32 dl, F32 intraDL = 0.0f );

   bool bufferNeedsUpdate(S32 objectDetail, S32 start, S32 end);

   void animate() { animate( mCurrentDetailLevel ); }
   void animate(S32 dl);
   void animateNodes(S32 ss);
   void animateVisibility(S32 ss);
   void animateFrame(S32 ss);
   void animateMatFrame(S32 ss);
   void animateSubtrees(bool forceFull = true);
   void animateNodeSubtrees(bool forceFull = true);

   /// Sets the 'forceHidden' state on the named mesh.
   /// @see MeshObjectInstance::forceHidden
   void setMeshForceHidden( const char *meshName, bool hidden );
   
   /// Sets the 'forceHidden' state on a mesh.
   /// @see MeshObjectInstance::forceHidden
   void setMeshForceHidden( S32 meshIndex, bool hidden );

   /// @name Animation Scale
   /// Query about animated scale
   /// @{
   bool animatesScale() { return (mShape->mFlags & TSShape::AnyScale) != 0; }
   bool animatesUniformScale() { return (mShape->mFlags & TSShape::UniformScale) != 0; }
   bool animatesAlignedScale() { return (mShape->mFlags & TSShape::AlignedScale) != 0; }
   bool animatesArbitraryScale() { return (mShape->mFlags & TSShape::ArbitraryScale) != 0; }
   bool scaleCurrentlyAnimated() { return mScaleCurrentlyAnimated; }
   /// @}

   //
   bool inTransition() { return !mTransitionThreads.empty(); }

   /// @name Ground Transforms
   /// The animator of a model can make the bounding box
   /// animate along with the object.  Doing so will move the object with the bounding box.
   /// The ground transform turns the world bounding box into the post-animation bounding box
   /// when such a technique is used.  However, few models actually use this technique.
   /// @{

   void animateGround(); ///< clears previous ground transform
   MatrixF & getGroundTransform() { return mGroundTransform; }
   void deltaGround(TSThread *, F32 start, F32 end, MatrixF * mat = NULL);
   void deltaGround1(TSThread *, F32 start, F32 end, MatrixF& mat);
   /// @}

   U32 getNumDetails() const { return mShape ? mShape->details.size() : 0; }

   S32 getCurrentDetail() const { return mCurrentDetailLevel; }

   F32 getCurrentIntraDetail() const { return mCurrentIntraDetailLevel; }

   void setCurrentDetail( S32 dl, F32 intraDL = 1.0f );

   /// Helper function which internally calls setDetailFromDistance.
   S32 setDetailFromPosAndScale( const SceneRenderState *state, 
                                 const Point3F &pos, 
                                 const Point3F &scale );

   /// Selects the current detail level using the scaled
   /// distance between your object and the camera.
   ///
   /// @see TSShape::Detail.
   S32 setDetailFromDistance( const SceneRenderState *state, F32 scaledDist );

   /// Sets the current detail level using the legacy screen error metric.
   S32 setDetailFromScreenError( F32 errorTOL );

   enum
   {
      TransformDirty =  BIT(0),
      VisDirty =        BIT(1),
      FrameDirty =      BIT(2),
      MatFrameDirty =   BIT(3),
      ThreadDirty =     BIT(4),
      AllDirtyMask = TransformDirty | VisDirty | FrameDirty | MatFrameDirty | ThreadDirty
   };
   U32 * mDirtyFlags;
   void setDirty(U32 dirty);
   void clearDirty(U32 dirty);

//-------------------------------------------------------------------------------------
// collision interface routines
//-------------------------------------------------------------------------------------

   public:

   bool buildPolyList(AbstractPolyList *, S32 dl);
   bool getFeatures(const MatrixF& mat, const Point3F& n, ConvexFeature*, S32 dl);
   bool castRay(const Point3F & start, const Point3F & end, RayInfo *,S32 dl);
   bool castRayRendered(const Point3F & start, const Point3F & end, RayInfo *,S32 dl);
   bool quickLOS(const Point3F & start, const Point3F & end, S32 dl) { return castRay(start,end,NULL,dl); }
   Point3F support(const Point3F & v, S32 dl);
   void computeBounds(S32 dl, Box3F & bounds); ///< uses current transforms to compute bounding box around a detail level
                                               ///< see like named method on shape if you want to use default transforms

   bool buildPolyListOpcode( S32 dl, AbstractPolyList *polyList, const Box3F &box );
   bool castRayOpcode( S32 objectDetail, const Point3F & start, const Point3F & end, RayInfo *);
   bool buildConvexOpcode( const MatrixF &objMat, const Point3F &objScale, S32 objectDetail, const Box3F &bounds, Convex *c, Convex *list );

//-------------------------------------------------------------------------------------
// Thread Control
//-------------------------------------------------------------------------------------

   /// @name Thread Control
   /// Threads!  In order to animate an object, first you need to have an animation in the object.
   /// Then, you need to get the TSShape of the object:
   /// @code
   /// TSShape* shape = mShapeInstance->getShape());
   /// @endcode
   /// Next, get the sequence and store::
   /// @code
   /// S32 seq = shape->findSequence("foo"));
   /// @endcode
   /// Create a new thread (if needed):
   /// @code
   /// TSThread* thread = mShapeInstance->addThread();
   /// @endcode
   /// Finally, set the position in the sequence:
   /// @code
   /// mShapeInstance->setSequence(thread, seq, 0)
   /// @endcode
   /// @{

   public:

   TSThread * addThread();                        ///< Create a new thread
   TSThread * getThread(S32 threadNumber);        ///< @note  threads can change order, best to hold
                                                  ///<        onto a thread from the start
   void destroyThread(TSThread * thread);         ///< Destroy a thread!
   U32 threadCount();                             ///< How many threads are there?

   void setSequence(TSThread *, S32 seq, F32 pos);///< Get the thread a sequence
   /// Transition to a sequence
   void transitionToSequence(TSThread *, S32 seq, F32 pos, F32 duration, bool continuePlay);
   void clearTransition(TSThread *);              ///< Stop transitions
   U32  getSequence(TSThread *);                  ///< Get the sequence of the thread

   void setBlendEnabled(TSThread *, bool blendOn);///< Set whether or not the thread will blend
   bool getBlendEnabled(TSThread *);              ///< Does this thread blend?

   void setPriority(TSThread *, F32 priority);    ///< Set thread priority
   F32 getPriority(TSThread * thread);             ///< Get thread priority

   F32 getTime(TSThread * thread);                ///< Get how long the thread has been playing
   F32 getPos(TSThread * thread);                 ///< Get the position in the thread

   void setTime(TSThread * thread, F32 time);     ///< Set how long into the thread to use
   void setPos(TSThread * thread, F32 pos);       ///< Set the position of the thread

   bool isInTransition(TSThread * thread);        ///< Is this thread in transition?
   F32 getTimeScale(TSThread * thread);           ///< Get the time scale of the thread
   void setTimeScale(TSThread * thread, F32);     ///< Set the time scale of the thread

   F32 getDuration(TSThread * thread);            ///< Get the duration of the thread
   F32 getScaledDuration(TSThread * thread);      ///< Get the duration of the thread with the scale factored in

   S32 getKeyframeCount(TSThread * thread);       ///< Get the number of keyframes
   S32 getKeyframeNumber(TSThread * thread);      ///< Get which keyframe the thread is on
   /// Set which keyframe the thread is on
   void setKeyframeNumber(TSThread * thread, S32 kf);

   void advanceTime(F32 delta, TSThread *); ///< advance time on a particular thread
   void advanceTime(F32 delta);             ///< advance time on all threads
   void advancePos(F32 delta, TSThread *);  ///< advance pos  on a particular thread
   void advancePos(F32 delta);              ///< advance pos  on all threads
   /// @}

//-------------------------------------------------------------------------------------
// constructors, destructors, initialization, io
//-------------------------------------------------------------------------------------

   TSShapeInstance( const Resource<TSShape> & shape, bool loadMaterials = true);
   TSShapeInstance( TSShape * pShape, bool loadMaterials = true);
   ~TSShapeInstance();

   void buildInstanceData(TSShape *, bool loadMaterials);
   void initNodeTransforms();
   void initMeshObjects();

   void dump(Stream &);
   void dumpNode(Stream &, S32 level, S32 nodeIndex, Vector<S32> & detailSizes);

   void *mData; ///< available for use by app...initialized to 0

   void prepCollision();

//-------------------------------------------------------------------------------------
// accumulation
//-------------------------------------------------------------------------------------

   bool hasAccumulation();
};


//-------------------------------------------------------------------------------------
// Thread class
//-------------------------------------------------------------------------------------

/// 3space animation thread.
///
/// An animation thread:  runtime data associated with a single sequence that is
/// running (or two sequences if in transition between them).
///
/// A shape instance can have multiple threads running. When multiple threads are running,
/// which thread/sequence controls which node or object is determined based
/// on priority of the sequence.
///
/// @note all thread data and methods are private (but TSShapeInstance is a friend).
///       Users should treat thread pointers like keys -- they are used to ID
///       the thread when interfacing with the shape, but are not manipulated
///       by anything but the TSShapeInstance.  See "Thread control" methods
///       for more info on controlling threads.
class TSThread
{
   friend class TSShapeInstance;

   S32 priority;

   TSShapeInstance * mShapeInstance;  ///< Instance of the shape that this thread animates

   S32 sequence;                      ///< Sequence this thread will perform
   F32 pos;

   F32 timeScale;                     ///< How fast to play through the sequence

   S32 keyNum1;                       ///< Keyframe at or before current position
   S32 keyNum2;                       ///< Keyframe at or after current position
   F32 keyPos;

   bool blendDisabled;                ///< Blend with other sequences?

   /// if in transition...
   struct TransitionData
   {
      bool inTransition;

      F32 duration;
      F32 pos;
      F32 direction;
      F32 targetScale; ///< time scale for sequence we are transitioning to (during transition only)
                       ///< this is either 1 or 0 (if 1 target sequence plays as we transition, if 0 it doesn't)
      TSIntegerSet oldRotationNodes;    ///< nodes controlled by this thread pre-transition
      TSIntegerSet oldTranslationNodes; ///< nodes controlled by this thread pre-transition
      TSIntegerSet oldScaleNodes;       ///< nodes controlled by this thread pre-transition
      U32 oldSequence; ///< sequence that was set before transition began
      F32 oldPos;      ///< position of sequence before transition began
   } transitionData;

   struct
   {
      F32 start;
      F32 end;
      S32 loop;
   } path;
   bool makePath;

   /// given a position on the thread, choose correct keyframes
   /// slight difference between one-shot and cyclic sequences -- see comments below for details
   void selectKeyframes(F32 pos, const TSSequence * seq, S32 * k1, S32 * k2, F32 * kpos);

   void getGround(F32 p, MatrixF * pMat);

   /// @name Triggers
   /// Triggers are used to do something once a certain animation point has been reached.
   ///
   /// For example, when the player's foot animation hits the ground, a foot puff and
   /// foot print are triggered from the thread.
   ///
   /// These are called by advancePos()
   /// @{
   void animateTriggers();
   void activateTriggers(F32 a, F32 b);
   /// @}

   TSThread(TSShapeInstance*);
   TSThread() {}

   void setSequence(S32 seq, F32 pos);
   void transitionToSequence(S32 seq, F32 pos, F32 duration, bool continuePlay);

   void advanceTime(F32 delta);
   void advancePos(F32 delta);

   F32 getTime();
   F32 getPos();

   void setTime(F32);
   void setPos(F32);

   bool isInTransition();
   F32 getTimeScale();
   void setTimeScale(F32);

   F32 getDuration();
   F32 getScaledDuration();

   S32 getKeyframeCount();
   S32 getKeyframeNumber();
   void setKeyframeNumber(S32 kf);

public:

   TSShapeInstance * getShapeInstance() { return mShapeInstance; }
   bool hasSequence() const { return sequence >= 0; }
   U32 getSeqIndex() const { return sequence; }
   const TSSequence* getSequence() const { return &(mShapeInstance->mShape->sequences[sequence]); }
   const String& getSequenceName() const { return mShapeInstance->mShape->getSequenceName(sequence); }
   S32 operator<(const TSThread &) const;
};

typedef TSShapeInstance::ObjectInstance TSObjectInstance;

#endif // _TSSHAPEINSTANCE_H_
