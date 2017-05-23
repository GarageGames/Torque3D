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

#ifndef _GFXDEVICE_H_
#define _GFXDEVICE_H_

#ifndef _GFXADAPTER_H_
#include "gfx/gfxAdapter.h"
#endif
#ifndef _GFXTARGET_H_
#include "gfx/gfxTarget.h"
#endif
#ifndef _GFXVERTEXBUFFER_H_
#include "gfx/gfxVertexBuffer.h"
#endif
#ifndef _GFXSTATEBLOCK_H_
#include "gfx/gfxStateBlock.h"
#endif
#ifndef _GFXSHADER_H_
#include "gfx/gfxShader.h"
#endif
#ifndef _GFXCUBEMAP_H_
#include "gfx/gfxCubemap.h"
#endif
#ifndef _TDICTIONARY_H_
#include "core/util/tDictionary.h"
#endif
#ifndef _TSIGNAL_H_
#include "core/util/tSignal.h"
#endif
#ifndef _GFXDEVICESTATISTICS_H_
#include "gfx/gfxDeviceStatistics.h"
#endif
#ifndef _MATHUTIL_FRUSTUM_H_
#include "math/util/frustum.h"
#endif

#ifndef _PLATFORM_PLATFORMTIMER_H_
#include "platform/platformTimer.h"
#endif

class FontRenderBatcher;
class GFont;
class GFXCardProfiler;
class GFXDrawUtil;
class GFXFence;
class GFXOcclusionQuery;
class GFXPrimitiveBuffer;
class GFXShader;
class GFXStateBlock;
class GFXShaderConstBuffer;
class GFXTextureManager;

// Global macro
#define GFX GFXDevice::get()

#define MAX_MRT_TARGETS 4 

//-----------------------------------------------------------------------------

/// GFXDevice is the TSE graphics interface layer. This allows the TSE to
/// do many things, such as use multiple render devices for multi-head systems,
/// and allow a game to render in DirectX 9, OpenGL or any other API which has
/// a GFX implementation seamlessly. There are many concepts in GFX device which
/// may not be familiar to you, especially if you have not used DirectX.
/// @n
/// <b>Buffers</b>
/// There are three types of buffers in GFX: vertex, index and primitive. Please
/// note that index buffers are not accessable outside the GFX layer, they are wrapped
/// by primitive buffers. Primitive buffers will be explained in detail later.
/// Buffers are allocated and deallocated using their associated allocXBuffer and
/// freeXBuffer methods on the device. When a buffer is allocated you pass in a
/// pointer to, depending on the buffer, a vertex type pointer or a U16 pointer. 
/// During allocation, this pointer is set to the address of where you should
/// copy in the information for this buffer. You must the tell the GFXDevice
/// that the information is in, and it should prepare the buffer for use by calling
/// the prepare method on it. Dynamic vertex buffer example:
/// @code
/// GFXVertexP *verts;        // Making a buffer containing verticies with only position
///
/// // Allocate a dynamic vertex buffer to hold 3 vertices and use *verts as the location to copy information into
/// GFXVertexBufferHandle vb = GFX->allocVertexBuffer( 3, &verts, true ); 
///
/// // Now set the information, we're making a triangle
/// verts[0].point = Point3F( 200.f, 200.f, 0.f );
/// verts[1].point = Point3F( 200.f, 400.f, 0.f );
/// verts[2].point = Point3F( 400.f, 200.f, 0.f );
///
/// // Tell GFX that the information is in and it should be made ready for use
/// // Note that nothing is done with verts, this should not and MUST NOT be deleted
/// // stored, or otherwise used after prepare is called.
/// GFX->prepare( vb );
///
/// // Because this is a dynamic vertex buffer, it is only assured to be valid until someone 
/// // else allocates a dynamic vertex buffer, so we will render it now
/// GFX->setVertexBuffer( vb );
/// GFX->drawPrimitive( GFXTriangleStrip, 0, 1 );
///
/// // Now because this is a dynamic vertex buffer it MUST NOT BE FREED you are only
/// // given a handle to a vertex buffer which belongs to the device
/// @endcode
/// 
/// To use a static vertex buffer, it is very similar, this is an example using a
/// static primitive buffer:
/// @n
/// This takes place inside a constructor for a class which has a member variable
/// called mPB which is the primitive buffer for the class instance.
/// @code
/// U16 *idx;                          // This is going to be where to write indices
/// GFXPrimitiveInfo *primitiveInfo;   // This will be where to write primitive information
///
/// // Allocate a primitive buffer with 4 indices, and 1 primitive described for use
/// mPB = GFX->allocPrimitiveBuffer( 4, &idx, 1, &primitiveInfo );
///
/// // Write the index information, this is going to be for the outline of a triangle using
/// // a line strip
/// idx[0] = 0;
/// idx[1] = 1;
/// idx[2] = 2;
/// idx[3] = 0;
///
/// // Write the information for the primitive
/// primitiveInfo->indexStart = 0;            // Starting with index 0
/// primitiveInfo->minVertex = 0;             // The minimum vertex index is 0
/// primitiveInfo->maxVertex = 3;             // The maximum vertex index is 3
/// primitiveInfo->primitiveCount = 3;        // There are 3 lines we are drawing
/// primitiveInfo->type = GFXLineStrip;       // This primitive info describes a line strip
/// @endcode
/// The following code takes place in the destructor for the same class
/// @code
/// // Because this is a static buffer it's our responsibility to free it when we are done
/// GFX->freePrimitiveBuffer( mPB );
/// @endcode
/// This last bit takes place in the rendering function for the class
/// @code
/// // You need to set a vertex buffer as well, primitive buffers contain indexing
/// // information, not vertex information. This is so you could have, say, a static
/// // vertex buffer, and a dynamic primitive buffer.
///
/// // This sets the primitive buffer to the static buffer we allocated in the constructor
/// GFX->setPrimitiveBuffer( mPB );
/// 
/// // Draw the first primitive contained in the set primitive buffer, our primitive buffer
/// // has only one primitive, so we could also technically call GFX->drawPrimitives(); and
/// // get the same result. 
/// GFX->drawPrimitive( 0 );
/// @endcode
/// If you need any more examples on how to use these buffers please see the rest of the engine.
/// @n
/// <b>Primitive Buffers</b>
/// @n
/// Primitive buffers wrap and extend the concept of index buffers. The purpose of a primitive
/// buffer is to let objects store all information they have to render their primitives in
/// a central place. Say that a shape is made up of triangle strips and triangle fans, it would
/// still have only one primitive buffer which contained primitive information for each strip
/// and fan. It could then draw itself with one call.
///
/// TO BE FINISHED LATER
class GFXDevice
{
private:
   friend class GFXInit;
   friend class GFXPrimitiveBufferHandle;
   friend class GFXVertexBufferHandleBase;
   friend class GFXTextureObject;
   friend class GFXTexHandle;
   friend class GFXVertexFormat;
   friend class GFXTestFullscreenToggle;
   friend class TestGFXTextureCube;
   friend class TestGFXRenderTargetCube;
   friend class TestGFXRenderTargetStack;
   friend class GFXResource;
   friend class LightMatInstance; // For stencil interface

   //--------------------------------------------------------------------------
   // Static GFX interface
   //--------------------------------------------------------------------------
public:

   enum GFXDeviceEventType
   {
      /// The device has been created, but not initialized
      deCreate,
      
      /// The device has been initialized
      deInit,
      
      /// The device is about to be destroyed.
      deDestroy,
      
      /// The device has started rendering a frame
      deStartOfFrame,
      
      /// The device is about to finish rendering a frame
      deEndOfFrame,

      /// The device has rendered a frame and ended the scene
      dePostFrame,

      /// The device has started rendering a frame's field (such as for side-by-side rendering)
      deStartOfField,

     /// left stereo frame has been rendered
     deLeftStereoFrameRendered,

     /// right stereo frame has been rendered
     deRightStereoFrameRendered,

      /// The device is about to finish rendering a frame's field
      deEndOfField,
   };

   typedef Signal <bool (GFXDeviceEventType)> DeviceEventSignal;
   static DeviceEventSignal& getDeviceEventSignal();
   
   static GFXDevice *get() { return smGFXDevice; }

   static void initConsole();
   static bool destroy();
   
   static bool devicePresent() { return (smGFXDevice && smGFXDevice->getAdapterType() != NullDevice); }

private:
   /// @name Device management variables
   /// @{
   static GFXDevice * smGFXDevice; ///< Global GFXDevice 
 
   /// @}

   //--------------------------------------------------------------------------
   // Core GFX interface
   //--------------------------------------------------------------------------
public:
   enum GFXDeviceRenderStyles
   {
      RS_Standard          = 0,
      RS_StereoSideBySide  = (1<<0),     // Render into current Render Target side-by-side
     RS_StereoSeparate    = (1<<1)      // Render in two separate passes (then combined by vr compositor)
   };

   enum GFXDeviceLimits
   {
      NumStereoPorts = 2
   };

private:

   /// Adapter for this device.
   GFXAdapter mAdapter;

protected:
   /// List of valid video modes for this device.
   Vector<GFXVideoMode> mVideoModes;

   /// The CardProfiler for this device.
   GFXCardProfiler *mCardProfiler;

   /// Head of the resource list.
   ///
   /// @see GFXResource
   GFXResource *mResourceListHead;

   /// Set once the device is active.
   bool mCanCurrentlyRender;

   /// Set if we're in a mode where we want rendering to occur.
   bool mAllowRender;

   /// The style of rendering that is to be performed, based on GFXDeviceRenderStyles
   U32 mCurrentRenderStyle;

   /// Current stereo target being rendered to
   S32 mCurrentStereoTarget;

   /// Eye offset used when using a stereo rendering style
   Point3F mStereoEyeOffset[NumStereoPorts];

   /// Center matrix for head
   MatrixF mStereoHeadTransform;

   /// Left and right matrix for eyes
   MatrixF mStereoEyeTransforms[NumStereoPorts];

   /// Inverse of mStereoEyeTransforms
   MatrixF mInverseStereoEyeTransforms[NumStereoPorts];

   /// Fov port settings
   FovPort mFovPorts[NumStereoPorts];

   /// Destination viewports for stereo rendering
   RectI mStereoViewports[NumStereoPorts];

   /// Destination targets for stereo rendering
   GFXTextureTarget* mStereoTargets[NumStereoPorts];

   /// This will allow querying to see if a device is initialized and ready to
   /// have operations performed on it.
   bool mInitialized;
   bool mReset;

   /// This is called before this, or any other device, is deleted in the global destroy()
   /// method. It allows the device to clean up anything while everything is still valid.
   virtual void preDestroy();

   /// Set the adapter that this device is using.  For use by GFXInit::createDevice only.
   virtual void setAdapter(const GFXAdapter& adapter) { mAdapter = adapter; }

   /// Notify GFXDevice that we are initialized
   virtual void deviceInited();
public:
   GFXDevice();
   virtual ~GFXDevice();

   /// Initialize this GFXDevice, optionally specifying a platform window to
   /// bind to.
   virtual void init( const GFXVideoMode &mode, PlatformWindow *window = NULL ) = 0;

   /// Returns true if the scene has begun and its
   /// safe to make rendering calls.
   /// @see beginScene
   /// @see endScene
   bool canCurrentlyRender() const { return mCanCurrentlyRender; }

   bool recentlyReset(){ return mReset; }
   void beginReset(){ mReset = true; }
   void finalizeReset(){ mReset = false; }

   void setAllowRender( bool render ) { mAllowRender = render; }

   inline bool allowRender() const { return mAllowRender; }
   
   /// Retrieve the current rendering style based on GFXDeviceRenderStyles
   U32 getCurrentRenderStyle() const { return mCurrentRenderStyle; }

   /// Retrieve the current stereo target being rendered to
   S32 getCurrentStereoTarget() const { return mCurrentStereoTarget; }

   /// Set the current rendering style, based on GFXDeviceRenderStyles
   void setCurrentRenderStyle(U32 style) { mCurrentRenderStyle = style; }

   /// Set the current stereo target being rendered to (in case we're doing anything with postfx)
   void setCurrentStereoTarget(const F32 targetId) { mCurrentStereoTarget = targetId; }

   /// Get the current eye offset used during stereo rendering
   const Point3F* getStereoEyeOffsets() { return mStereoEyeOffset; }

   const MatrixF& getStereoHeadTransform() { return mStereoHeadTransform;  }
   const MatrixF* getStereoEyeTransforms() { return mStereoEyeTransforms; }
   const MatrixF* getInverseStereoEyeTransforms() { return mInverseStereoEyeTransforms; }

   /// Sets the head matrix for stereo rendering
   void setStereoHeadTransform(const MatrixF &mat) { mStereoHeadTransform = mat; }

   /// Set the current eye offset used during stereo rendering
   void setStereoEyeOffsets(Point3F *offsets) { dMemcpy(mStereoEyeOffset, offsets, sizeof(Point3F) * NumStereoPorts); }

   void setStereoEyeTransforms(MatrixF *transforms) { dMemcpy(mStereoEyeTransforms, transforms, sizeof(mStereoEyeTransforms)); dMemcpy(mInverseStereoEyeTransforms, transforms, sizeof(mInverseStereoEyeTransforms)); mInverseStereoEyeTransforms[0].inverse(); mInverseStereoEyeTransforms[1].inverse();  }

   /// Set the current eye offset used during stereo rendering. Assumes NumStereoPorts are available.
   void setStereoFovPort(const FovPort *ports) { dMemcpy(mFovPorts, ports, sizeof(mFovPorts)); }

   /// Get the current eye offset used during stereo rendering
   const FovPort* getStereoFovPort() { return mFovPorts; }

   /// Sets stereo viewports
   void setSteroViewports(const RectI *ports) { dMemcpy(mStereoViewports, ports, sizeof(RectI) * NumStereoPorts); }

   /// Sets stereo render targets
   void setStereoTargets(GFXTextureTarget **targets) { mStereoTargets[0] = targets[0]; mStereoTargets[1] = targets[1]; }

   RectI* getStereoViewports() { return mStereoViewports; }

   /// Activates a stereo render target, setting the correct viewport to render eye contents.
   /// If eyeId is -1, set a viewport encompassing the entire size of the render targets.
   void activateStereoTarget(S32 eyeId)
   {
      if (eyeId == -1)
      {
         if (mStereoTargets[0])
         {
            setActiveRenderTarget(mStereoTargets[0], true);
         }
      }
      else
      {
         if (mStereoTargets[eyeId])
         {
            setActiveRenderTarget(mStereoTargets[eyeId], false);
         }
         setViewport(mStereoViewports[eyeId]);
      }

      mCurrentStereoTarget = eyeId;
   }

   GFXCardProfiler* getCardProfiler() const { return mCardProfiler; }

   /// Returns active graphics adapter type.
   virtual GFXAdapterType getAdapterType()=0;

   /// Returns the Adapter that was used to create this device
   virtual const GFXAdapter& getAdapter() { return mAdapter; }

   /// @}

   /// @name Debug Methods
   /// @{

   virtual void enterDebugEvent(ColorI color, const char *name) = 0;
   virtual void leaveDebugEvent() = 0;
   virtual void setDebugMarker(ColorI color, const char *name) = 0;

   /// @}

   /// @name Resource debug methods
   /// @{
   
   /// Lists how many of each GFX resource (e.g. textures, texture targets, shaders, etc.) GFX is aware of
   /// @param unflaggedOnly   If true, this method only counts unflagged resources
   virtual void listResources(bool unflaggedOnly);

   /// Flags all resources GFX is currently aware of
   virtual void flagCurrentResources();

   /// Clears the flag on all resources GFX is currently aware of
   virtual void clearResourceFlags();

   /// Dumps a description of the specified resource types to the console
   /// @param resNames     A string of space separated class names (e.g. "GFXTextureObject GFXTextureTarget GFXShader")
   ///                     to describe to the console
   /// @param file         A path to the file to write the descriptions to.  If it is NULL or "", descriptions are
   ///                     written to the console.
   /// @param unflaggedOnly If true, this method only counts unflagged resources
   /// @note resNames is case sensitive because there is no dStristr function.
   virtual void describeResources(const char* resName, const char* file, bool unflaggedOnly);

   /// Returns the current GFXDeviceStatistics, stats are cleared every ::beginScene call.
   GFXDeviceStatistics* getDeviceStatistics() { return &mDeviceStatistics; }
protected:
   GFXDeviceStatistics mDeviceStatistics;

   /// This is a helper method for describeResourcesToFile.  It walks through the
   /// GFXResource list and sorts it by item type, putting the resources into the proper vector.
   /// @see describeResources
   virtual void fillResourceVectors(const char* resNames, bool unflaggedOnly, Vector<GFXResource*> &textureObjects,
      Vector<GFXResource*> &textureTargets, Vector<GFXResource*> &windowTargets, Vector<GFXResource*> &vertexBuffers, 
      Vector<GFXResource*> &primitiveBuffers, Vector<GFXResource*> &fences, Vector<GFXResource*> &cubemaps, 
      Vector<GFXResource*> &shaders, Vector<GFXResource*> &stateblocks);
public:

   /// @}

   /// @name Video Mode Functions
   /// @{
   /// Enumerates the supported video modes of the device
   virtual void enumerateVideoModes() = 0;

   /// Returns the video mode list.
   /// @see GFXVideoMode
   const Vector<GFXVideoMode>* const getVideoModeList() const { return &mVideoModes; }

   /// Returns the first format from the list which meets all 
   /// the criteria of the texture profile and query options.      
   virtual GFXFormat selectSupportedFormat(GFXTextureProfile *profile,
      const Vector<GFXFormat> &formats, bool texture, bool mustblend, bool mustfilter) = 0;

   /// @}

   //-----------------------------------------------------------------------------
protected:

   /// @name State tracking variables
   /// @{

   /// Set if ANY state is dirty, including matrices or primitive buffers.
   bool mStateDirty;     
   
   enum TexDirtyType
   {
      GFXTDT_Normal,
      GFXTDT_Cube
   };
   
   GFXTexHandle mCurrentTexture[TEXTURE_STAGE_COUNT];
   GFXTexHandle mNewTexture[TEXTURE_STAGE_COUNT];
   GFXCubemapHandle mCurrentCubemap[TEXTURE_STAGE_COUNT];
   GFXCubemapHandle mNewCubemap[TEXTURE_STAGE_COUNT];

   TexDirtyType   mTexType[TEXTURE_STAGE_COUNT];
   bool           mTextureDirty[TEXTURE_STAGE_COUNT];
   bool           mTexturesDirty;

   // This maps a GFXStateBlockDesc hash value to a GFXStateBlockRef
   typedef Map<U32, GFXStateBlockRef> StateBlockMap;
   StateBlockMap mCurrentStateBlocks;

   // This tracks whether or not our state block is dirty.
   bool  mStateBlockDirty;
   GFXStateBlockRef mCurrentStateBlock;
   GFXStateBlockRef mNewStateBlock;

   GFXShaderConstBuffer *mCurrentShaderConstBuffer;

   /// A global forced wireframe mode.
   static bool smWireframe;

   /// The global vsync state.
   static bool smDisableVSync;

   /// The forced shader model version if non-zero.
   static F32 smForcedPixVersion;

   /// Disable all hardware occlusion queries causing 
   /// them to return only the visibile state.
   static bool smDisableOcclusionQuery;

   /// @}

   /// @name Light Tracking
   /// @{

   GFXLightInfo  mCurrentLight[LIGHT_STAGE_COUNT]; 
   bool          mCurrentLightEnable[LIGHT_STAGE_COUNT];
   bool          mLightDirty[LIGHT_STAGE_COUNT];
   bool          mLightsDirty;

   ColorF        mGlobalAmbientColor;
   bool          mGlobalAmbientColorDirty;

   /// @}

   /// @name Fixed function material tracking
   /// @{

   GFXLightMaterial mCurrentLightMaterial;
   bool mLightMaterialDirty;

   /// @}

   /// @name Bitmap modulation and color stack
   /// @{

   ///

   /// @}

   /// @see getDeviceSwizzle32
   Swizzle<U8, 4> *mDeviceSwizzle32;

   /// @see getDeviceSwizzle24
   Swizzle<U8, 3> *mDeviceSwizzle24;


   //-----------------------------------------------------------------------------

   /// @name Matrix managing variables
   /// @{

   ///
   MatrixF mWorldMatrix[WORLD_STACK_MAX];
   bool    mWorldMatrixDirty;
   S32     mWorldStackSize;

   MatrixF mProjectionMatrix;
   bool    mProjectionMatrixDirty;

   MatrixF mViewMatrix;
   bool    mViewMatrixDirty;

   MatrixF mTextureMatrix[TEXTURE_STAGE_COUNT];
   bool    mTextureMatrixDirty[TEXTURE_STAGE_COUNT];
   bool    mTextureMatrixCheckDirty;
   /// @}

   /// @name Current frustum planes
   /// @{

   ///
   Frustum mFrustum;

   //-----------------------------------------------------------------------------

   /// @name Stateblock functions
   /// @{

   /// Called by GFXDevice to create a device specific stateblock
   virtual GFXStateBlockRef createStateBlockInternal(const GFXStateBlockDesc& desc) = 0;
   /// Called by GFXDevice to actually set a stateblock.
   /// @param force If true, set all states 
   virtual void setStateBlockInternal(GFXStateBlock* block, bool force) = 0;
   /// @}

   /// Called by base GFXDevice to actually set a const buffer
   virtual void setShaderConstBufferInternal(GFXShaderConstBuffer* buffer) = 0;

   virtual void setTextureInternal(U32 textureUnit, const GFXTextureObject*texture) = 0;

   virtual void setLightInternal(U32 lightStage, const GFXLightInfo light, bool lightEnable) = 0;
   virtual void setGlobalAmbientInternal(ColorF color) = 0;
   virtual void setLightMaterialInternal(const GFXLightMaterial mat) = 0;

   virtual bool beginSceneInternal() = 0;
   virtual void endSceneInternal() = 0;

   /// @name State Initialization.
   /// @{

   /// State initialization. This MUST BE CALLED in setVideoMode after the device
   /// is created.
   virtual void initStates() = 0;
   /// @}

   //-----------------------------------------------------------------------------

   /// This function must be implemented differently per
   /// API and it should set ONLY the current matrix.
   /// For example, in OpenGL, there should be NO matrix stack
   /// activity, all the stack stuff is managed in the GFX layer.
   ///
   /// OpenGL does not have separate world and
   /// view matrices. It has ModelView which is world * view.
   /// You must take this into consideration.
   ///
   /// @param   mtype   Which matrix to set, world/view/projection
   /// @param   mat   Matrix to assign
   virtual void setMatrix( GFXMatrixType mtype, const MatrixF &mat ) = 0;

   //-----------------------------------------------------------------------------
protected:


   /// @name Buffer Allocation 
   /// These methods are implemented per-device and are called by the GFX layer
   /// when a user calls an alloc
   ///
   /// @note Primitive Buffers are NOT implemented per device, they wrap index buffers
   /// @{

   /// This allocates a vertex buffer and returns a pointer to the allocated buffer.
   /// This function should not be called directly - rather it should be used by
   /// the GFXVertexBufferHandle class.
   virtual GFXVertexBuffer *allocVertexBuffer(  U32 numVerts, 
                                                const GFXVertexFormat *vertexFormat, 
                                                U32 vertSize, 
                                                GFXBufferType bufferType,
                                                void* data = NULL ) = 0;

   /// Called from GFXVertexFormat to allocate the hardware 
   /// specific vertex declaration for rendering.
   virtual GFXVertexDecl* allocVertexDecl( const GFXVertexFormat *vertexFormat ) = 0;

   /// Sets the current vertex declaration on the device.
   virtual void setVertexDecl( const GFXVertexDecl *decl ) = 0;

   /// Sets the vertex buffer on the device.
   virtual void setVertexStream( U32 stream, GFXVertexBuffer *buffer ) = 0;

   /// Set the vertex stream frequency on the device.
   virtual void setVertexStreamFrequency( U32 stream, U32 frequency ) = 0;

   /// The maximum number of supported vertex streams which
   /// may be more than the device supports.
   static const U32 VERTEX_STREAM_COUNT = 4;

   StrongRefPtr<GFXVertexBuffer> mCurrentVertexBuffer[VERTEX_STREAM_COUNT];
   bool mVertexBufferDirty[VERTEX_STREAM_COUNT];
   U32 mVertexBufferFrequency[VERTEX_STREAM_COUNT];
   bool mVertexBufferFrequencyDirty[VERTEX_STREAM_COUNT];

   const GFXVertexDecl *mCurrVertexDecl;
   bool mVertexDeclDirty;

   StrongRefPtr<GFXPrimitiveBuffer> mCurrentPrimitiveBuffer;
   bool mPrimitiveBufferDirty;

   /// This allocates a primitive buffer and returns a pointer to the allocated buffer.
   /// A primitive buffer's type argument refers to the index data - the primitive data will
   /// always be preserved from call to call.
   ///
   /// @note All index buffers use unsigned 16-bit indices.
   virtual GFXPrimitiveBuffer *allocPrimitiveBuffer(  U32 numIndices, 
                                                      U32 numPrimitives, 
                                                      GFXBufferType bufferType,
                                                      void* data = NULL ) = 0;

   /// @}

   //---------------------------------------
   // SFX buffer
   //---------------------------------------
protected:

   GFXTexHandle mFrontBuffer[2];
   U32 mCurrentFrontBufferIdx;

   //---------------------------------------
   // Render target related
   //---------------------------------------
   
   /// A stack of previously active render targets.
   Vector<GFXTargetRef> mRTStack;

   /// The current render target which may or may not 
   /// not be yet activated.
   /// @see mRTDirty
   GFXTargetRef mCurrentRT;
   
   /// This tracks a previously activated render target
   /// which need to be deactivated. 
   GFXTargetRef mRTDeactivate;
   
   /// This is set when the current and/or deactivate render
   /// targets have changed and the device need to update
   /// its state on the next draw/clear.
   bool mRTDirty;

   /// Updates the render targets and viewport in a device
   /// specific manner when they are dirty.
   virtual void _updateRenderTargets() = 0;

   /// The current viewport rect.
   RectI mViewport;

   /// If true the viewport has been changed and
   /// it must be updated on the next draw/clear.
   bool mViewportDirty;

public:

   /// @name Texture functions
   /// @{
protected:
   GFXTextureManager * mTextureManager;

public:   
   virtual GFXCubemap * createCubemap() = 0;

   inline GFXTextureManager *getTextureManager()
   {
      return mTextureManager;
   }

   ///@}

   /// Swizzle to convert 32bpp bitmaps from RGBA to the native device format.
   const Swizzle<U8, 4> *getDeviceSwizzle32() const 
   { 
      return mDeviceSwizzle32;
   }

   /// Swizzle to convert 24bpp bitmaps from RGB to the native device format.
   const Swizzle<U8, 3> *getDeviceSwizzle24() const 
   { 
      return mDeviceSwizzle24;
   }

   /// @name Render Target functions
   /// @{

   /// Allocate a target for doing render to texture operations, with no
   /// depth/stencil buffer.
   virtual GFXTextureTarget *allocRenderToTextureTarget()=0;

   /// Allocate a target for a given window.
   virtual GFXWindowTarget *allocWindowTarget(PlatformWindow *window)=0;

   /// Store the current render target to restore later.
   void pushActiveRenderTarget();

   /// Restore the previous render target.
   void popActiveRenderTarget();

   /// Assign a new active render target.
   void setActiveRenderTarget( GFXTarget *target, bool updateViewport=true );

   /// Returns the current active render target.
   inline GFXTarget* getActiveRenderTarget() { return mCurrentRT; }

   ///@}

   /// @name Shader functions
   /// @{
   virtual F32   getPixelShaderVersion() const = 0;
   virtual void  setPixelShaderVersion( F32 version ) = 0;

   /// Returns the number of texture samplers that can be used in a shader rendering pass
   virtual U32 getNumSamplers() const = 0;

   /// Returns the number of simultaneous render targets supported by the device.
   virtual U32 getNumRenderTargets() const = 0;

   virtual void setShader( GFXShader *shader, bool force = false ) {}
   virtual void disableShaders( bool force = false ) {} // TODO Remove when T3D 4.0

   /// Set the buffer! (Actual set happens on the next draw call, just like textures, state blocks, etc)
   void setShaderConstBuffer(GFXShaderConstBuffer* buffer);
   
   /// Creates a new empty shader which must be initialized 
   /// and deleted by the caller.
   /// @see GFXShader::init
   virtual GFXShader* createShader() = 0;
   
   /// @}
 
   //-----------------------------------------------------------------------------

   /// @name Rendering methods
   /// @{

   ///
   virtual void clear( U32 flags, ColorI color, F32 z, U32 stencil ) = 0;
   virtual bool beginScene();
   virtual void endScene();
   virtual void beginField();
   virtual void endField();
   PlatformTimer *mFrameTime;

   virtual GFXTexHandle & getFrontBuffer(){ return mFrontBuffer[mCurrentFrontBufferIdx]; }

   void setPrimitiveBuffer( GFXPrimitiveBuffer *buffer );

   /// Sets the vertex buffer.
   ///
   /// When setting the stream 0 vertex buffer it will automatically
   /// set its associated vertex format as the active format.
   ///
   /// @param buffer    The vertex buffer or NULL to clear the buffer.
   /// @param stream    The stream index of the vertex source stream to place the buffer.
   /// @param frequency The stream frequency of the vertex buffer.
   void setVertexBuffer( GFXVertexBuffer *buffer, U32 stream = 0, U32 frequency = 0 );

   /// Sets the current vertex format.
   ///
   /// This should only be used if the vertex format of the stream 0 vertex 
   /// buffer is different from the one associated to it.  Typically this
   /// is used when rendering from multiple vertex streams.
   ///
   void setVertexFormat( const GFXVertexFormat *vertexFormat );

   virtual void drawPrimitive( GFXPrimitiveType primType, U32 vertexStart, U32 primitiveCount ) = 0;

   /// The parameters to drawIndexedPrimitive are somewhat complicated. From a raw-data stand point
   /// they evaluate to something like the following:
   /// @code
   /// U16 indicies[] = { 0, 1, 2, 1, 0, 0, 2 }; 
   /// Point3F verts[] = { Point3F( 0.0f, 0.0f, 0.0f ), Point3F( 0.0f, 1.0f, 0.0f ), Point3F( 0.0f, 0.0f, 1.0f ) };
   /// 
   /// GFX->drawIndexedPrimitive( GFXLineList, // Drawing a list of lines, each line is two verts
   ///                            0, // vertex 0 will be referenced so minIndex = 0
   ///                            3, // 3 verticies will be used for this draw call
   ///                            1, // We want index 1 to be the first index used, so indicies 1-6 will be used
   ///                            3  // Drawing 3 LineList primitives, meaning 6 verts will be drawn
   ///                             );
   ///
   /// U16 *idxPtr = &indicies[1];  // 1 = startIndex, so the pointer is offset such that:
   ///                              //    idxPtr[0] is the same as indicies[1]
   ///
   /// U32 numVertsToDrawFromBuffer = primitiveCount * 2; // 2 verts define a line in the GFXLineList primitive type (6)
   /// @endcode
   ///
   /// @param  primType    Type of primitive to draw
   ///
   /// @param  startVertex This defines index zero.  Its the offset from the start of
   ///                     the vertex buffer to the first vertex.
   ///
   /// @param  minIndex    The smallest index into the vertex stream which will be used for this draw call.
   ///                     This is a zero based index relative to startVertex.  It is strictly a performance
   ///                     hint for implementations. No vertex below minIndex will be referenced by this draw
   ///                     call. For device implementors, this should _not_ be used to offset the vertex buffer,
   ///                     or index buffer.
   ///
   /// @param  numVerts    The number of verticies which will be referenced in this draw call. This is not
   ///                     the number of verticies which will be drawn. That is a function of 'primType' and 
   ///                     'primitiveCount'.
   ///
   /// @param  startIndex  An offset from the start of the index buffer to specify where to start. If
   ///                     'idxBuffer' is a pointer to an array of integers, this could be written as
   ///                     int *offsetIdx = idxBuffer + startIndex;
   ///
   /// @param  primitiveCount The number of primitives of type 'primType' to draw.
   ///
   virtual void drawIndexedPrimitive(  GFXPrimitiveType primType, 
                                       U32 startVertex, 
                                       U32 minIndex, 
                                       U32 numVerts, 
                                       U32 startIndex, 
                                       U32 primitiveCount ) = 0;

   void drawPrimitive( const GFXPrimitive &prim );
   void drawPrimitive( U32 primitiveIndex );
   void drawPrimitives();
   void drawPrimitiveBuffer( GFXPrimitiveBuffer *buffer );
   /// @}

   //-----------------------------------------------------------------------------

   /// Allocate a fence. The API specific implementation of GFXDevice is responsible
   /// to make sure that the proper type is used. GFXGeneralFence should work in
   /// all cases. 
   virtual GFXFence *createFence() = 0;

   /// Returns a hardware occlusion query object or NULL
   /// if this device does not support them.   
   virtual GFXOcclusionQuery* createOcclusionQuery() { return NULL; }
   
   /// @name Light Settings
   /// NONE of these should be overridden by API implementations
   /// because of the state caching stuff.
   /// @{
   void setLight(U32 stage, GFXLightInfo* light);
   void setLightMaterial(const GFXLightMaterial& mat);
   void setGlobalAmbientColor(const ColorF& color);

   /// @}
   
   /// @name Texture State Settings
   /// NONE of these should be overridden by API implementations
   /// because of the state caching stuff.
   /// @{

   ///
   void setTexture(U32 stage, GFXTextureObject*texture);
   void setCubeTexture( U32 stage, GFXCubemap *cubemap );
   inline GFXTextureObject* getCurrentTexture( U32 stage ) { return mCurrentTexture[stage]; }

   /// @}

   /// @name State Block Interface
   /// @{

   /// Creates a state block object based on the desc passed in.  This object
   /// represents an immutable state.
   virtual GFXStateBlockRef createStateBlock( const GFXStateBlockDesc &desc );

   /// Sets the current stateblock (actually activated in ::updateStates)
   virtual void setStateBlock( GFXStateBlock *block );

   GFXStateBlock* getStateBlock() { return mNewStateBlock; }

   /// This sets a stateblock directly from the description
   /// structure.  Its acceptable to use this for debug rendering
   /// and other low frequency rendering tasks.
   virtual void setStateBlockByDesc( const GFXStateBlockDesc &desc );

   /// @}

   /// @name General state interface
   /// @{
   /// Sets the dirty Render/Texture/Sampler states from the caching system
   void updateStates(bool forceSetAll = false);

   /// Returns the forced global wireframe state.
   static bool getWireframe() { return smWireframe; }

   /// Returns true if the occlusion query is disabled.
   static bool getDisableOcclusionQuery() { return smDisableOcclusionQuery; }
   /// @}

   //-----------------------------------------------------------------------------

   /// @name Matrix interface
   /// @{

   /// Sets the top of the world matrix stack
   /// @param   newWorld   New world matrix to set
   void setWorldMatrix( const MatrixF &newWorld );

   /// Gets the matrix on the top of the world matrix stack
   inline const MatrixF &getWorldMatrix() const { return mWorldMatrix[mWorldStackSize]; }

   /// Pushes the world matrix stack and copies the current top
   /// matrix to the new top of the stack
   void pushWorldMatrix();

   /// Pops the world matrix stack
   void popWorldMatrix();

   /// Sets the projection matrix
   /// @param   newProj   New projection matrix to set
   void setProjectionMatrix( const MatrixF &newProj );

   /// Gets the projection matrix
   inline const MatrixF &getProjectionMatrix() const { return mProjectionMatrix; }

   /// Sets the view matrix
   /// @param   newView   New view matrix to set
   void setViewMatrix( const MatrixF &newView );

   /// Gets the view matrix
   inline const MatrixF &getViewMatrix() const { return mViewMatrix; }

   /// Multiplies the matrix at the top of the world matrix stack by a matrix
   /// and replaces the top of the matrix stack with the result
   /// @param   mat   Matrix to multiply
   void multWorld( const MatrixF &mat );

   /// Set texture matrix for a sampler
   void setTextureMatrix( const U32 stage, const MatrixF &texMat );

   /// Set an area of the target to render to.
   void setViewport( const RectI &rect );

   /// Get the current area of the target we will render to.   
   const RectI &getViewport() const { return mViewport; }

   virtual void setClipRect( const RectI &rect ) = 0;
   virtual const RectI &getClipRect() const = 0;

   /// Set the projection frustum.
   ///
   /// @see MathUtils::makeFrustum()
   virtual void setFrustum( F32 left, F32 right,
                            F32 bottom, F32 top,
                            F32 nearPlane, F32 farPlane,
                            bool bRotate = true);

   ///
   virtual void setFrustum( const Frustum& frust,
                            bool bRotate = true );

   /// Get the projection frustum.
   void getFrustum(  F32 *left, 
                     F32 *right, 
                     F32 *bottom, 
                     F32 *top, 
                     F32 *nearPlane, 
                     F32 *farPlane, 
                     bool *isOrtho ) const;

   /// Get the projection frustum.
   const Frustum& getFrustum() const { return mFrustum; }

   /// This will construct and apply an orthographic projection matrix with the provided parameters
   /// @param doRotate If set to true, the resulting matrix will be rotated PI/2 around the X axis
   //                  for support in tsShapeInstance. You probably want to leave this as 'false'.
   void setOrtho(F32 left, F32 right, F32 bottom, F32 top, F32 nearPlane, F32 farPlane, bool doRotate = false);
   
   /// Return true if the current frustum uses orthographic projection rather than perspective projection.
   bool isFrustumOrtho() const { return mFrustum.isOrtho(); }

   /// @}
   
   /// Returns the scale for converting world space 
   /// units to screen space units... aka pixels.
   ///
   /// This is the true scale which is best used for GUI
   /// drawing.  For doing lod calculations you should be
   /// using the functions in SceneState which is adjusted
   /// for special cases like shadows and reflections.
   ///
   /// @see SceneState::getWorldToScreenScale()
   /// @see SceneState::projectRadius()
   ///
   Point2F getWorldToScreenScale() const;

public:
   enum GenericShaderType
   {
      GSColor = 0,
      GSTexture,
      GSModColorTexture,
      GSAddColorTexture,
      GSTargetRestore,
      GS_COUNT
   };

   /// This is a helper function to set a default shader for rendering GUI elements
   /// on systems which do not support fixed-function operations as well as for
   /// things which need just generic position/texture/color shaders
   ///
   /// @param  type  Type of generic shader, add your own if you need
   virtual void setupGenericShaders( GenericShaderType type = GSColor ) {};

   /// Get the fill convention for this device
   virtual F32 getFillConventionOffset() const = 0;

   virtual U32 getMaxDynamicVerts() = 0;
   virtual U32 getMaxDynamicIndices() = 0;

   virtual void doParanoidStateCheck(){};

   /// Get access to this device's drawing utility class.
   GFXDrawUtil *getDrawUtil();

#ifndef TORQUE_SHIPPING
   /// This is a method designed for debugging. It will allow you to dump the states
   /// in the render manager out to a file so that it can be diffed and examined.
   void dumpStates( const char *fileName ) const;
#else
   void dumpStates( const char *fileName ) const {};
#endif
   protected:
      GFXDrawUtil *mDrawer;
}; 

//-----------------------------------------------------------------------------
// Matrix interface

inline void GFXDevice::setWorldMatrix( const MatrixF &newWorld )
{
   mWorldMatrixDirty = true;
   mStateDirty = true;
   mWorldMatrix[mWorldStackSize] = newWorld;
}

inline void GFXDevice::pushWorldMatrix()
{
   mWorldMatrixDirty = true;
   mStateDirty = true;
   mWorldStackSize++;
   AssertFatal( mWorldStackSize < WORLD_STACK_MAX, "GFX: Exceeded world matrix stack size" );
   mWorldMatrix[mWorldStackSize] = mWorldMatrix[mWorldStackSize - 1];
}

inline void GFXDevice::popWorldMatrix()
{
   mWorldMatrixDirty = true;
   mStateDirty = true;
   mWorldStackSize--;
   AssertFatal( mWorldStackSize >= 0, "GFX: Negative WorldStackSize!" );
}

inline void GFXDevice::multWorld( const MatrixF &mat )
{
   mWorldMatrixDirty = true;
   mStateDirty = true;
   mWorldMatrix[mWorldStackSize].mul(mat);
}

inline void GFXDevice::setProjectionMatrix( const MatrixF &newProj )
{
   mProjectionMatrixDirty = true;
   mStateDirty = true;
   mProjectionMatrix = newProj;
}

inline void GFXDevice::setViewMatrix( const MatrixF &newView )
{
   mStateDirty = true;
   mViewMatrixDirty = true;
   mViewMatrix = newView;
}

inline void GFXDevice::setTextureMatrix( const U32 stage, const MatrixF &texMat )
{
   AssertFatal( stage < TEXTURE_STAGE_COUNT, "Out of range texture sampler" );
   mStateDirty = true;
   mTextureMatrixDirty[stage] = true;
   mTextureMatrix[stage] = texMat;
   mTextureMatrixCheckDirty = true;
}

//-----------------------------------------------------------------------------
// Buffer management

inline void GFXDevice::setVertexBuffer( GFXVertexBuffer *buffer, U32 stream, U32 frequency )
{
   AssertFatal( stream < VERTEX_STREAM_COUNT, "GFXDevice::setVertexBuffer - Bad stream index!" );

   if ( buffer && stream == 0 )
      setVertexFormat( &buffer->mVertexFormat );

   if ( buffer != mCurrentVertexBuffer[stream] )
   {
      mCurrentVertexBuffer[stream] = buffer;
      mVertexBufferDirty[stream] = true;
      mStateDirty = true;
   }

   if ( mVertexBufferFrequency[stream] != frequency )
   {
      mVertexBufferFrequency[stream] = frequency;
      mVertexBufferFrequencyDirty[stream] = true;
      mStateDirty = true;
   }   
}

inline void GFXDevice::setVertexFormat( const GFXVertexFormat *vertexFormat )
{
   if ( vertexFormat->getDecl() == mCurrVertexDecl )
      return;

   mCurrVertexDecl = vertexFormat->getDecl();
   mVertexDeclDirty = true;
   mStateDirty = true;
}


#if defined(TORQUE_DEBUG) && defined(TORQUE_DEBUG_GFX)
#define GFXAssertFatal(x, error) AssertFatal(x, error)
#else
#define GFXAssertFatal(x, error)
#endif

#endif // _GFXDEVICE_H_
