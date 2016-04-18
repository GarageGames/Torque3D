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
#include "gfx/gfxDevice.h"

#include "gfx/gfxInit.h"
#include "gfx/gfxCubemap.h"
#include "gfx/primBuilder.h"
#include "gfx/gfxDrawUtil.h"
#include "gfx/gfxFence.h"
#include "gfx/gfxFontRenderBatcher.h"
#include "gfx/gfxPrimitiveBuffer.h"
#include "gfx/gfxShader.h"
#include "gfx/gfxStateBlock.h"
#include "gfx/screenshot.h"
#include "gfx/gfxStringEnumTranslate.h"
#include "gfx/gfxTextureManager.h"

#include "core/frameAllocator.h"
#include "core/stream/fileStream.h"
#include "core/strings/unicode.h"
#include "core/util/journal/process.h"
#include "core/util/safeDelete.h"
#include "math/util/frustum.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"

GFXDevice * GFXDevice::smGFXDevice = NULL;
bool GFXDevice::smWireframe = false;
bool GFXDevice::smDisableVSync = true;
F32 GFXDevice::smForcedPixVersion = -1.0f;
bool GFXDevice::smDisableOcclusionQuery = false;
bool gDisassembleAllShaders = false;


void GFXDevice::initConsole()
{
   GFXStringEnumTranslate::init();

   Con::addVariable( "$gfx::wireframe", TypeBool, &smWireframe,
      "Used to toggle wireframe rendering at runtime.\n"
      "@ingroup GFX\n" );

   Con::addVariable( "$gfx::disassembleAllShaders", TypeBool, &gDisassembleAllShaders,
      "On supported devices this will dump shader disassembly to the "
      "procedural shader folder.\n"
      "@ingroup GFX\n" );

   Con::addVariable( "$gfx::disableOcclusionQuery", TypeBool, &smDisableOcclusionQuery,
      "Debug helper that disables all hardware occlusion queries causing "
      "them to return only the visibile state.\n"
      "@ingroup GFX\n" );

   Con::addVariable( "$pref::Video::disableVerticalSync", TypeBool, &smDisableVSync,
      "Disables vertical sync on the active device.\n"
      "@note The video mode must be reset for the change to take affect.\n"
      "@ingroup GFX\n" );

   Con::addVariable( "$pref::Video::forcedPixVersion", TypeF32, &smForcedPixVersion,
      "Will force the shader model if the value is positive and less than the "
      "shader model supported by the active device.  Use 0 for fixed function.\n"
      "@note The graphics device must be reset for the change to take affect.\n"
      "@ingroup GFX\n" );
}

GFXDevice::DeviceEventSignal& GFXDevice::getDeviceEventSignal()
{
   static DeviceEventSignal theSignal;
   return theSignal;
}

GFXDevice::GFXDevice() 
{    
   VECTOR_SET_ASSOCIATION( mVideoModes );
   VECTOR_SET_ASSOCIATION( mRTStack );

   mWorldMatrixDirty = false;
   mWorldStackSize = 0;
   mProjectionMatrixDirty = false;
   mViewMatrixDirty = false;
   mTextureMatrixCheckDirty = false;

   mViewMatrix.identity();
   mProjectionMatrix.identity();
   
   for( S32 i = 0; i < WORLD_STACK_MAX; i++ )
      mWorldMatrix[i].identity();
   
   AssertFatal(smGFXDevice == NULL, "Already a GFXDevice created! Bad!");
   smGFXDevice = this;
      
   // Vertex buffer cache
   mCurrVertexDecl = NULL;
   mVertexDeclDirty = false;
   for ( U32 i=0; i < VERTEX_STREAM_COUNT; i++ )
   {
      mVertexBufferDirty[i] = false;
      mVertexBufferFrequency[i] = 0;
      mVertexBufferFrequencyDirty[i] = false;
   }

   // Primitive buffer cache
   mPrimitiveBufferDirty = false;
   mTexturesDirty = false;
   
   // Use of TEXTURE_STAGE_COUNT in initialization is okay [7/2/2007 Pat]
   for(U32 i = 0; i < TEXTURE_STAGE_COUNT; i++)
   {
      mTextureDirty[i] = false;
      mCurrentTexture[i] = NULL;
      mNewTexture[i] = NULL;
      mCurrentCubemap[i] = NULL;
      mNewCubemap[i] = NULL;
      mTexType[i] = GFXTDT_Normal;

      mTextureMatrix[i].identity();
      mTextureMatrixDirty[i] = false;
   }

   mLightsDirty = false;
   for(U32 i = 0; i < LIGHT_STAGE_COUNT; i++)
   {
      mLightDirty[i] = false;
      mCurrentLightEnable[i] = false;
   }

   mGlobalAmbientColorDirty = false;
   mGlobalAmbientColor = ColorF(0.0f, 0.0f, 0.0f, 1.0f);

   mLightMaterialDirty = false;
   dMemset(&mCurrentLightMaterial, 0, sizeof(GFXLightMaterial));

   // State block 
   mStateBlockDirty = false;
   mCurrentStateBlock = NULL;
   mNewStateBlock = NULL;

   mCurrentShaderConstBuffer = NULL;

   // misc
   mAllowRender = true;
   mCurrentRenderStyle = RS_Standard;
   mCurrentProjectionOffset = Point2F::Zero;
   mCanCurrentlyRender = false;
   mInitialized = false;
   
   mRTDirty = false;
   mViewport = RectI::Zero;
   mViewportDirty = false;

   mCurrentFrontBufferIdx = 0;

   mDeviceSwizzle32 = NULL;
   mDeviceSwizzle24 = NULL;

   mResourceListHead = NULL;

   mCardProfiler = NULL;   

   // Initialize our drawing utility.
   mDrawer = NULL;
   mFrameTime = PlatformTimer::create();
   // Add a few system wide shader macros.
   GFXShader::addGlobalMacro( "TORQUE", "1" );
   GFXShader::addGlobalMacro( "TORQUE_VERSION", String::ToString(getVersionNumber()) );
   #if defined TORQUE_OS_WIN
      GFXShader::addGlobalMacro( "TORQUE_OS_WIN" );
   #elif defined TORQUE_OS_MAC
      GFXShader::addGlobalMacro( "TORQUE_OS_MAC" );
   #elif defined TORQUE_OS_LINUX
      GFXShader::addGlobalMacro( "TORQUE_OS_LINUX" );      
   #elif defined TORQUE_OS_XENON
      GFXShader::addGlobalMacro( "TORQUE_OS_XENON" );
   #elif defined TORQUE_OS_XBOX
      GFXShader::addGlobalMacro( "TORQUE_OS_XBOX" );      
   #elif defined TORQUE_OS_PS3
      GFXShader::addGlobalMacro( "TORQUE_OS_PS3" );            
   #endif

   mStereoTargets[0] = NULL;
   mStereoTargets[1] = NULL;
}

GFXDrawUtil* GFXDevice::getDrawUtil()
{
   if (!mDrawer)
   {
      mDrawer = new GFXDrawUtil(this);
   }
   return mDrawer;
}

void GFXDevice::deviceInited()
{
   getDeviceEventSignal().trigger(deInit);
   mDeviceStatistics.setPrefix("$GFXDeviceStatistics::");

   // Initialize the static helper textures.
   GBitmap temp( 2, 2, false, GFXFormatR8G8B8A8 );
   temp.fill( ColorI::ONE );
   GFXTexHandle::ONE.set( &temp, &GFXDefaultStaticDiffuseProfile, false, "GFXTexHandle::ONE" ); 
   temp.fill( ColorI::ZERO );
   GFXTexHandle::ZERO.set( &temp, &GFXDefaultStaticDiffuseProfile, false, "GFXTexHandle::ZERO" ); 
   temp.fill( ColorI( 128, 128, 255 ) );
   GFXTexHandle::ZUP.set( &temp, &GFXDefaultStaticNormalMapProfile, false, "GFXTexHandle::ZUP" ); 
}

bool GFXDevice::destroy()
{
   // Cleanup the static helper textures.
   GFXTexHandle::ONE.free();
   GFXTexHandle::ZERO.free();
   GFXTexHandle::ZUP.free();

   // Make this release its buffer.
   PrimBuild::shutdown();

   // Let people know we are shutting down
   getDeviceEventSignal().trigger(deDestroy);

   if(smGFXDevice)
      smGFXDevice->preDestroy();
   SAFE_DELETE(smGFXDevice);

   return true;
}

void GFXDevice::preDestroy()
{
   // Delete draw util
   SAFE_DELETE( mDrawer );
}

GFXDevice::~GFXDevice()
{ 
   smGFXDevice = NULL;

   // Clean up our current buffers.
   mCurrentPrimitiveBuffer = NULL;
   for ( U32 i=0; i < VERTEX_STREAM_COUNT; i++ )
      mCurrentVertexBuffer[i] = NULL;

   // Clear out our current texture references
   for (U32 i = 0; i < TEXTURE_STAGE_COUNT; i++)
   {
      mCurrentTexture[i] = NULL;
      mNewTexture[i] = NULL;
      mCurrentCubemap[i] = NULL;
      mNewCubemap[i] = NULL;
   }

   mCurrentRT = NULL;

   // Release all the unreferenced textures in the cache.
   mTextureManager->cleanupCache();

   // Check for resource leaks
#ifdef TORQUE_DEBUG
   AssertFatal( GFXTextureObject::dumpActiveTOs() == 0, "There is a texture object leak, check the log for more details." );
   GFXPrimitiveBuffer::dumpActivePBs();
#endif

   SAFE_DELETE( mTextureManager );
   SAFE_DELETE( mFrameTime );

   // Clear out our state block references
   mCurrentStateBlocks.clear();
   mNewStateBlock = NULL;
   mCurrentStateBlock = NULL;

   mCurrentShaderConstBuffer = NULL;
   /// End Block above BTR

   // -- Clear out resource list
   // Note: our derived class destructor will have already released resources.
   // Clearing this list saves us from having our resources (which are not deleted
   // just released) turn around and try to remove themselves from this list.
   while (mResourceListHead)
   {
      GFXResource * head = mResourceListHead;
      mResourceListHead = head->mNextResource;
      
      head->mPrevResource = NULL;
      head->mNextResource = NULL;
      head->mOwningDevice = NULL;
   }
}

GFXStateBlockRef GFXDevice::createStateBlock(const GFXStateBlockDesc& desc)
{
   PROFILE_SCOPE( GFXDevice_CreateStateBlock );

   U32 hashValue = desc.getHashValue();
   if (mCurrentStateBlocks[hashValue])
      return mCurrentStateBlocks[hashValue];

   GFXStateBlockRef result = createStateBlockInternal(desc);
   result->registerResourceWithDevice(this);   
   mCurrentStateBlocks[hashValue] = result;
   return result;
}

void GFXDevice::setStateBlock(GFXStateBlock* block)
{
   AssertFatal(block, "NULL state block!");
   AssertFatal(block->getOwningDevice() == this, "This state doesn't apply to this device!");

   if (block != mCurrentStateBlock)
   {
      mStateDirty = true;
      mStateBlockDirty = true;
      mNewStateBlock = block;
   } else {
      mStateBlockDirty = false;
      mNewStateBlock = mCurrentStateBlock;
   }
}

void GFXDevice::setStateBlockByDesc( const GFXStateBlockDesc &desc )
{
   PROFILE_SCOPE( GFXDevice_SetStateBlockByDesc );
   GFXStateBlock *block = createStateBlock( desc );
   setStateBlock( block );
}

void GFXDevice::setShaderConstBuffer(GFXShaderConstBuffer* buffer)
{
   mCurrentShaderConstBuffer = buffer;
}

void GFXDevice::updateStates(bool forceSetAll /*=false*/)
{
   PROFILE_SCOPE(GFXDevice_updateStates);

   if(forceSetAll)
   {
      bool rememberToEndScene = false;
      if(!canCurrentlyRender())
      {
         if (!beginScene())
         {
            AssertFatal(false, "GFXDevice::updateStates:  Unable to beginScene!");
         }
         rememberToEndScene = true;
      }

      setMatrix( GFXMatrixProjection, mProjectionMatrix );
      setMatrix( GFXMatrixWorld, mWorldMatrix[mWorldStackSize] );
      setMatrix( GFXMatrixView, mViewMatrix );

      setVertexDecl( mCurrVertexDecl );

      for ( U32 i=0; i < VERTEX_STREAM_COUNT; i++ )
      {
         setVertexStream( i, mCurrentVertexBuffer[i] );
         setVertexStreamFrequency( i, mVertexBufferFrequency[i] );
      }

      if( mCurrentPrimitiveBuffer.isValid() ) // This could be NULL when the device is initalizing
         mCurrentPrimitiveBuffer->prepare();

      /// Stateblocks
      if ( mNewStateBlock )
         setStateBlockInternal(mNewStateBlock, true);
      mCurrentStateBlock = mNewStateBlock;

      for(U32 i = 0; i < getNumSamplers(); i++)
      {
         switch (mTexType[i])
         {
            case GFXTDT_Normal :
               {
                  mCurrentTexture[i] = mNewTexture[i];
                  setTextureInternal(i, mCurrentTexture[i]);
               }  
               break;
            case GFXTDT_Cube :
               {
                  mCurrentCubemap[i] = mNewCubemap[i];
                  if (mCurrentCubemap[i])
                     mCurrentCubemap[i]->setToTexUnit(i);
                  else
                     setTextureInternal(i, NULL);
               }
               break;
            default:
               AssertFatal(false, "Unknown texture type!");
               break;
         }
      }

      // Set our material
      setLightMaterialInternal(mCurrentLightMaterial);

      // Set our lights
      for(U32 i = 0; i < LIGHT_STAGE_COUNT; i++)
      {
         setLightInternal(i, mCurrentLight[i], mCurrentLightEnable[i]);
      }

       _updateRenderTargets();

      if(rememberToEndScene)
         endScene();

      return;
   }

   if (!mStateDirty)
      return;

   // Normal update logic begins here.
   mStateDirty = false;

   // Update Projection Matrix
   if( mProjectionMatrixDirty )
   {
      setMatrix( GFXMatrixProjection, mProjectionMatrix );
      mProjectionMatrixDirty = false;
   }
   
   // Update World Matrix
   if( mWorldMatrixDirty )
   {
      setMatrix( GFXMatrixWorld, mWorldMatrix[mWorldStackSize] );
      mWorldMatrixDirty = false;
   }
   
   // Update View Matrix
   if( mViewMatrixDirty )
   {
      setMatrix( GFXMatrixView, mViewMatrix );
      mViewMatrixDirty = false;
   }


   if( mTextureMatrixCheckDirty )
   {
      for( S32 i = 0; i < getNumSamplers(); i++ )
      {
         if( mTextureMatrixDirty[i] )
         {
            mTextureMatrixDirty[i] = false;
            setMatrix( (GFXMatrixType)(GFXMatrixTexture + i), mTextureMatrix[i] );
         }
      }

      mTextureMatrixCheckDirty = false;
   }

   // Update the vertex declaration.
   if ( mVertexDeclDirty )
   {
      setVertexDecl( mCurrVertexDecl );
      mVertexDeclDirty = false;
   }

   // Update the vertex buffers.
   for ( U32 i=0; i < VERTEX_STREAM_COUNT; i++ )
   {
      if ( mVertexBufferDirty[i] )
      {
         setVertexStream( i, mCurrentVertexBuffer[i] );
         mVertexBufferDirty[i] = false;
      }

      if ( mVertexBufferFrequencyDirty[i] )
      {
         setVertexStreamFrequency( i, mVertexBufferFrequency[i] );
         mVertexBufferFrequencyDirty[i] = false;
      }
   }

   // Update primitive buffer
   //
   // NOTE: It is very important to set the primitive buffer AFTER the vertex buffer
   // because in order to draw indexed primitives in DX8, the call to SetIndicies
   // needs to include the base vertex offset, and the DX8 GFXDevice relies on
   // having mCurrentVB properly assigned before the call to setIndices -patw
   if( mPrimitiveBufferDirty )
   {
      if( mCurrentPrimitiveBuffer.isValid() ) // This could be NULL when the device is initalizing
         mCurrentPrimitiveBuffer->prepare();
      mPrimitiveBufferDirty = false;
   }

   // NOTE: With state blocks, it's now important to update state before setting textures
   // some devices (e.g. OpenGL) set states on the texture and we need that information before
   // the texture is activated.
   if (mStateBlockDirty)
   {
      setStateBlockInternal(mNewStateBlock, false);
      mCurrentStateBlock = mNewStateBlock;
      mStateBlockDirty = false;
   }

   _updateRenderTargets();

   if( mTexturesDirty )
   {
      mTexturesDirty = false;
      for(U32 i = 0; i < getNumSamplers(); i++)
      {
         if(!mTextureDirty[i])
            continue;
         mTextureDirty[i] = false;

         switch (mTexType[i])
         {
         case GFXTDT_Normal :
            {
               mCurrentTexture[i] = mNewTexture[i];
               setTextureInternal(i, mCurrentTexture[i]);
            }  
            break;
         case GFXTDT_Cube :
            {
               mCurrentCubemap[i] = mNewCubemap[i];
               if (mCurrentCubemap[i])
                  mCurrentCubemap[i]->setToTexUnit(i);
               else
                  setTextureInternal(i, NULL);
            }
            break;
         default:
            AssertFatal(false, "Unknown texture type!");
            break;
         }
      }
   }
   
   // Set light material
   if(mLightMaterialDirty)
   {
      setLightMaterialInternal(mCurrentLightMaterial);
      mLightMaterialDirty = false;
   }

   // Set our lights
   if(mLightsDirty)
   {
      mLightsDirty = false;
      for(U32 i = 0; i < LIGHT_STAGE_COUNT; i++)
      {
         if(!mLightDirty[i])
            continue;

         mLightDirty[i] = false;
         setLightInternal(i, mCurrentLight[i], mCurrentLightEnable[i]);
      }
   }

   _updateRenderTargets();

#ifdef TORQUE_DEBUG_RENDER
   doParanoidStateCheck();
#endif
}

void GFXDevice::setPrimitiveBuffer( GFXPrimitiveBuffer *buffer )
{
   if( buffer == mCurrentPrimitiveBuffer )
      return;
   
   mCurrentPrimitiveBuffer = buffer;
   mPrimitiveBufferDirty = true;
   mStateDirty = true;
}

void GFXDevice::drawPrimitive( U32 primitiveIndex )
{
   AssertFatal( mCurrentPrimitiveBuffer.isValid(), "Trying to call drawPrimitive with no current primitive buffer, call setPrimitiveBuffer()" );
   AssertFatal( primitiveIndex < mCurrentPrimitiveBuffer->mPrimitiveCount, "Out of range primitive index.");
   drawPrimitive( mCurrentPrimitiveBuffer->mPrimitiveArray[primitiveIndex] );
}

void GFXDevice::drawPrimitive( const GFXPrimitive &prim )
{
   // Do NOT add index buffer offset to this call, it will be added by drawIndexedPrimitive
   drawIndexedPrimitive(   prim.type, 
                           prim.startVertex,
                           prim.minIndex, 
                           prim.numVertices, 
                           prim.startIndex, 
                           prim.numPrimitives );
}

void GFXDevice::drawPrimitives()
{
   AssertFatal( mCurrentPrimitiveBuffer.isValid(), "Trying to call drawPrimitive with no current primitive buffer, call setPrimitiveBuffer()" );

   GFXPrimitive *info = NULL;
   
   for( U32 i = 0; i < mCurrentPrimitiveBuffer->mPrimitiveCount; i++ ) {
      info = &mCurrentPrimitiveBuffer->mPrimitiveArray[i];

      // Do NOT add index buffer offset to this call, it will be added by drawIndexedPrimitive
      drawIndexedPrimitive(   info->type, 
                              info->startVertex,
                              info->minIndex, 
                              info->numVertices, 
                              info->startIndex, 
                              info->numPrimitives );
   }
}

DefineEngineFunction( getDisplayDeviceList, String, (),,
   "Returns a tab-seperated string of the detected devices across all adapters.\n"
   "@ingroup GFX\n" )
{
   Vector<GFXAdapter*> adapters;
   GFXInit::getAdapters(&adapters);

   StringBuilder str;
   for (S32 i=0; i<adapters.size(); i++)
   {
      if (i)
         str.append( '\t' );
      str.append(adapters[i]->mName);
   }

   return str.end();
}

void GFXDevice::setFrustum(   F32 left, 
                              F32 right, 
                              F32 bottom, 
                              F32 top, 
                              F32 nearPlane, 
                              F32 farPlane,
                              bool bRotate )
{
   // store values
   mFrustum.set(false, left, right, top, bottom, nearPlane, farPlane);
   
   // compute matrix
   MatrixF projection;
   mFrustum.getProjectionMatrix(&projection, bRotate);
   setProjectionMatrix( projection );
}

void GFXDevice::setFrustum( const Frustum& frust, bool bRotate )
{
   // store values
   mFrustum = frust;
   
   // compute matrix
   MatrixF projection;
   mFrustum.getProjectionMatrix(&projection, bRotate);
   setProjectionMatrix( projection );
}


void GFXDevice::getFrustum( F32 *left, F32 *right, F32 *bottom, F32 *top, F32 *nearPlane, F32 *farPlane, bool *isOrtho ) const
{   
   if ( left )       *left       = mFrustum.getNearLeft();
   if ( right )      *right      = mFrustum.getNearRight();
   if ( bottom )     *bottom     = mFrustum.getNearBottom();
   if ( top )        *top        = mFrustum.getNearTop();
   if ( nearPlane )  *nearPlane  = mFrustum.getNearDist();
   if ( farPlane )   *farPlane   = mFrustum.getFarDist();
   if ( isOrtho )    *isOrtho    = mFrustum.isOrtho();
}

void GFXDevice::setOrtho(  F32 left, 
                           F32 right, 
                           F32 bottom, 
                           F32 top, 
                           F32 nearPlane, 
                           F32 farPlane,
                           bool doRotate )
{
   // store values
   mFrustum.set(true, left, right, top, bottom, nearPlane, farPlane);

   // compute matrix
   MatrixF projection;
   mFrustum.getProjectionMatrix(&projection, doRotate);  

   setProjectionMatrix( projection );
}

Point2F GFXDevice::getWorldToScreenScale() const
{
   Point2F scale;

   const RectI &viewport = getViewport();

   if ( mFrustum.isOrtho() )
      scale.set(  viewport.extent.x / mFrustum.getWidth(),
                  viewport.extent.y / mFrustum.getHeight() );
   else
      scale.set(  ( mFrustum.getNearDist() * viewport.extent.x ) / mFrustum.getWidth(),
                  ( mFrustum.getNearDist() * viewport.extent.y ) / mFrustum.getHeight() );

   return scale;
}

//-----------------------------------------------------------------------------
// Set Light
//-----------------------------------------------------------------------------
void GFXDevice::setLight(U32 stage, GFXLightInfo* light)
{
   AssertFatal(stage < LIGHT_STAGE_COUNT, "GFXDevice::setLight - out of range stage!");

   if(!mLightDirty[stage])
   {
      mStateDirty = true;
      mLightsDirty = true;
      mLightDirty[stage] = true;
   }
   mCurrentLightEnable[stage] = (light != NULL);
   if(mCurrentLightEnable[stage])
      mCurrentLight[stage] = *light;
}

//-----------------------------------------------------------------------------
// Set Light Material
//-----------------------------------------------------------------------------
void GFXDevice::setLightMaterial(const GFXLightMaterial& mat)
{
   mCurrentLightMaterial = mat;
   mLightMaterialDirty = true;
   mStateDirty = true;
}

void GFXDevice::setGlobalAmbientColor(const ColorF& color)
{
   if(mGlobalAmbientColor != color)
   {
      mGlobalAmbientColor = color;
      mGlobalAmbientColorDirty = true;
   }
}

//-----------------------------------------------------------------------------
// Set texture
//-----------------------------------------------------------------------------
void GFXDevice::setTexture( U32 stage, GFXTextureObject *texture )
{
   AssertFatal(stage < getNumSamplers(), "GFXDevice::setTexture - out of range stage!");

   if (  mTexType[stage] == GFXTDT_Normal &&
         (  ( mTextureDirty[stage] && mNewTexture[stage].getPointer() == texture ) ||
            ( !mTextureDirty[stage] && mCurrentTexture[stage].getPointer() == texture ) ) )
      return;

   mStateDirty = true;
   mTexturesDirty = true;
   mTextureDirty[stage] = true;

   mNewTexture[stage] = texture;
   mTexType[stage] = GFXTDT_Normal;

   // Clear out the cubemaps
   mNewCubemap[stage] = NULL;
   mCurrentCubemap[stage] = NULL;
}

//-----------------------------------------------------------------------------
// Set cube texture
//-----------------------------------------------------------------------------
void GFXDevice::setCubeTexture( U32 stage, GFXCubemap *texture )
{
   AssertFatal(stage < getNumSamplers(), "GFXDevice::setTexture - out of range stage!");

   if (  mTexType[stage] == GFXTDT_Cube &&
         (  ( mTextureDirty[stage] && mNewCubemap[stage].getPointer() == texture ) ||
            ( !mTextureDirty[stage] && mCurrentCubemap[stage].getPointer() == texture ) ) )
      return;

   mStateDirty = true;
   mTexturesDirty = true;
   mTextureDirty[stage] = true;

   mNewCubemap[stage] = texture;
   mTexType[stage] = GFXTDT_Cube;

   // Clear out the normal textures
   mNewTexture[stage] = NULL;
   mCurrentTexture[stage] = NULL;
}

//------------------------------------------------------------------------------

inline bool GFXDevice::beginScene()
{
   AssertFatal( mCanCurrentlyRender == false, "GFXDevice::beginScene() - The scene has already begun!" );

   mDeviceStatistics.clear();

   // Send the start of frame signal.
   getDeviceEventSignal().trigger( GFXDevice::deStartOfFrame );
   mFrameTime->reset();
   return beginSceneInternal();
}

inline void GFXDevice::endScene()
{
   AssertFatal( mCanCurrentlyRender == true, "GFXDevice::endScene() - The scene has already ended!" );
   
   // End frame signal
   getDeviceEventSignal().trigger( GFXDevice::deEndOfFrame );

   endSceneInternal();
   mDeviceStatistics.exportToConsole();
}

inline void GFXDevice::beginField()
{
   AssertFatal( mCanCurrentlyRender == true, "GFXDevice::beginField() - The scene has not yet begun!" );

   // Send the start of field signal.
   getDeviceEventSignal().trigger( GFXDevice::deStartOfField );
}

inline void GFXDevice::endField()
{
   AssertFatal( mCanCurrentlyRender == true, "GFXDevice::endField() - The scene has not yet begun!" );

   // Send the end of field signal.
   getDeviceEventSignal().trigger( GFXDevice::deEndOfField );
}

void GFXDevice::setViewport( const RectI &inRect ) 
{
   // Clip the rect against the renderable size.
   Point2I size = mCurrentRT->getSize();
   RectI maxRect(Point2I(0,0), size);
   RectI rect = inRect;
   rect.intersect(maxRect);

   if ( mViewport != rect )
   {
      mViewport = rect;
      mViewportDirty = true;
   }   
}

void GFXDevice::pushActiveRenderTarget()
{
   // Push the current target on to the stack.
   mRTStack.push_back( mCurrentRT );
}

void GFXDevice::popActiveRenderTarget()
{
   AssertFatal( mRTStack.size() > 0, "GFXDevice::popActiveRenderTarget() - stack is empty!" );

   // Restore the last item on the stack and pop.
   setActiveRenderTarget( mRTStack.last() );
   mRTStack.pop_back();
}

void GFXDevice::setActiveRenderTarget( GFXTarget *target, bool updateViewport )
{
   AssertFatal( target, 
      "GFXDevice::setActiveRenderTarget - must specify a render target!" );

   if ( target == mCurrentRT )
      return;
   
   // If we're not dirty then store the 
   // current RT for deactivation later.
   if ( !mRTDirty )
   {
      // Deactivate the target queued for deactivation
      if(mRTDeactivate)
         mRTDeactivate->deactivate();

      mRTDeactivate = mCurrentRT;
   }

   mRTDirty = true;
   mCurrentRT = target;

   // When a target changes we also change the viewport
   // to match it.  This causes problems when the viewport
   // has been modified for clipping to a GUI bounds.
   //
   // We should consider removing this and making it the
   // responsibility of the caller to set a proper viewport
   // when the target is changed.   
   if ( updateViewport )
   {
      setViewport( RectI( Point2I::Zero, mCurrentRT->getSize() ) );
   }
}

/// Helper class for GFXDevice::describeResources.
class DescriptionOutputter
{
   /// Are we writing to a file?
   bool mWriteToFile;

   /// File if we are writing to a file
   FileStream mFile;
public:
   DescriptionOutputter(const char* file)
   {
      mWriteToFile = false;
      // If we've been given what could be a valid file path, open it.
      if(file && file[0] != '\0')
      {
         mWriteToFile = mFile.open(file, Torque::FS::File::Write);

         // Note that it is safe to retry.  If this is hit, we'll just write to the console instead of to the file.
         AssertFatal(mWriteToFile, avar("DescriptionOutputter::DescriptionOutputter - could not open file %s", file));
      }
   }

   ~DescriptionOutputter()
   {
      // Close the file
      if(mWriteToFile)
         mFile.close();
   }

   /// Writes line to the file or to the console, depending on what we want.
   void write(const char* line)
   {
      if(mWriteToFile)
         mFile.writeLine((const U8*)line);
      else
         Con::printf(line);
   }
};

#ifndef TORQUE_SHIPPING
void GFXDevice::dumpStates( const char *fileName ) const
{
   DescriptionOutputter output(fileName);

   output.write("Current state");
   if (!mCurrentStateBlock.isNull())
      output.write(mCurrentStateBlock->getDesc().describeSelf().c_str());
   else
      output.write("No state!");

   output.write("\nAll states:\n");
   GFXResource* walk = mResourceListHead;
   while(walk)
   {
      const GFXStateBlock* sb = dynamic_cast<const GFXStateBlock*>(walk);
      if (sb)
      {
         output.write(sb->getDesc().describeSelf().c_str());
      }
      walk = walk->getNextResource();
   }
}
#endif

void GFXDevice::listResources(bool unflaggedOnly)
{
   U32 numTextures = 0, numShaders = 0, numRenderToTextureTargs = 0, numWindowTargs = 0;
   U32 numCubemaps = 0, numVertexBuffers = 0, numPrimitiveBuffers = 0, numFences = 0;
   U32 numStateBlocks = 0;

   GFXResource* walk = mResourceListHead;
   while(walk)
   {
      if(unflaggedOnly && walk->isFlagged())
      {
         walk = walk->getNextResource();
         continue;
      }

      if(dynamic_cast<GFXTextureObject*>(walk))
         numTextures++;
      else if(dynamic_cast<GFXShader*>(walk))
         numShaders++;
      else if(dynamic_cast<GFXTextureTarget*>(walk))
         numRenderToTextureTargs++;
      else if(dynamic_cast<GFXWindowTarget*>(walk))
         numWindowTargs++;
      else if(dynamic_cast<GFXCubemap*>(walk))
         numCubemaps++;
      else if(dynamic_cast<GFXVertexBuffer*>(walk))
         numVertexBuffers++;
      else if(dynamic_cast<GFXPrimitiveBuffer*>(walk))
         numPrimitiveBuffers++;
      else if(dynamic_cast<GFXFence*>(walk))
         numFences++;
      else if (dynamic_cast<GFXStateBlock*>(walk))
         numStateBlocks++;
      else
         Con::warnf("Unknown resource: %x", walk);

      walk = walk->getNextResource();
   }
   const char* flag = unflaggedOnly ? "unflagged" : "allocated";

   Con::printf("GFX currently has:");
   Con::printf("   %i %s textures", numTextures, flag);
   Con::printf("   %i %s shaders", numShaders, flag);
   Con::printf("   %i %s texture targets", numRenderToTextureTargs, flag);
   Con::printf("   %i %s window targets", numWindowTargs, flag);
   Con::printf("   %i %s cubemaps", numCubemaps, flag);
   Con::printf("   %i %s vertex buffers", numVertexBuffers, flag);
   Con::printf("   %i %s primitive buffers", numPrimitiveBuffers, flag);
   Con::printf("   %i %s fences", numFences, flag);
   Con::printf("   %i %s state blocks", numStateBlocks, flag);
}

void GFXDevice::fillResourceVectors(const char* resNames, bool unflaggedOnly, Vector<GFXResource*> &textureObjects,
                                 Vector<GFXResource*> &textureTargets, Vector<GFXResource*> &windowTargets, Vector<GFXResource*> &vertexBuffers, 
                                 Vector<GFXResource*> &primitiveBuffers, Vector<GFXResource*> &fences, Vector<GFXResource*> &cubemaps, 
                                 Vector<GFXResource*> &shaders, Vector<GFXResource*> &stateblocks)
{
   bool describeTexture = true, describeTextureTarget = true, describeWindowTarget = true, describeVertexBuffer = true, 
      describePrimitiveBuffer = true, describeFence = true, describeCubemap = true, describeShader = true,
      describeStateBlock = true;

   // If we didn't specify a string of names, we'll print all of them
   if(resNames && resNames[0] != '\0')
   {
      // If we did specify a string of names, determine which names
      describeTexture =          (dStrstr(resNames, "GFXTextureObject")    != NULL);
      describeTextureTarget =    (dStrstr(resNames, "GFXTextureTarget")    != NULL);
      describeWindowTarget =     (dStrstr(resNames, "GFXWindowTarget")     != NULL);
      describeVertexBuffer =     (dStrstr(resNames, "GFXVertexBuffer")     != NULL);
      describePrimitiveBuffer =  (dStrstr(resNames, "GFXPrimitiveBuffer")   != NULL);
      describeFence =            (dStrstr(resNames, "GFXFence")            != NULL);
      describeCubemap =          (dStrstr(resNames, "GFXCubemap")          != NULL);
      describeShader =           (dStrstr(resNames, "GFXShader")           != NULL);
      describeStateBlock =       (dStrstr(resNames, "GFXStateBlock")           != NULL);
   }

   // Start going through the list
   GFXResource* walk = mResourceListHead;
   while(walk)
   {
      // If we only want unflagged resources, skip all flagged resources
      if(unflaggedOnly && walk->isFlagged())
      {
         walk = walk->getNextResource();
         continue;
      }

      // All of the following checks go through the same logic.
      // if(describingThisResource) 
      // {
      //    ResourceType* type = dynamic_cast<ResourceType*>(walk)
      //    if(type)
      //    {
      //       typeVector.push_back(type);
      //       walk = walk->getNextResource();
      //       continue;
      //    }
      // }

      if(describeTexture)
      {
         GFXTextureObject* tex = dynamic_cast<GFXTextureObject*>(walk);
         {
            if(tex)
            {
               textureObjects.push_back(tex);
               walk = walk->getNextResource();
               continue;
            }
         }
      }
      if(describeShader)
      {
         GFXShader* shd = dynamic_cast<GFXShader*>(walk);
         if(shd)
         {
            shaders.push_back(shd);
            walk = walk->getNextResource();
            continue;
         }
      }
      if(describeVertexBuffer)
      {
         GFXVertexBuffer* buf = dynamic_cast<GFXVertexBuffer*>(walk);
         if(buf)
         {
            vertexBuffers.push_back(buf);
            walk = walk->getNextResource();
            continue;
         }
      }
      if(describePrimitiveBuffer)
      {
         GFXPrimitiveBuffer* buf = dynamic_cast<GFXPrimitiveBuffer*>(walk);
         if(buf)
         {
            primitiveBuffers.push_back(buf);
            walk = walk->getNextResource();
            continue;
         }
      }
      if(describeTextureTarget)
      {
         GFXTextureTarget* targ = dynamic_cast<GFXTextureTarget*>(walk);
         if(targ)
         {
            textureTargets.push_back(targ);
            walk = walk->getNextResource();
            continue;
         }
      }
      if(describeWindowTarget)
      {
         GFXWindowTarget* targ = dynamic_cast<GFXWindowTarget*>(walk);
         if(targ)
         {
            windowTargets.push_back(targ);
            walk = walk->getNextResource();
            continue;
         }
      }
      if(describeCubemap)
      {
         GFXCubemap* cube = dynamic_cast<GFXCubemap*>(walk);
         if(cube)
         {
            cubemaps.push_back(cube);
            walk = walk->getNextResource();
            continue;
         }
      }
      if(describeFence)
      {
         GFXFence* fence = dynamic_cast<GFXFence*>(walk);
         if(fence)
         {
            fences.push_back(fence);
            walk = walk->getNextResource();
            continue;
         }
      }
      if (describeStateBlock)
      {
         GFXStateBlock* sb = dynamic_cast<GFXStateBlock*>(walk);
         if (sb)
         {
            stateblocks.push_back(sb);
            walk = walk->getNextResource();
            continue;
         }
      }
      // Wasn't something we were looking for
      walk = walk->getNextResource();
   }
}

void GFXDevice::describeResources(const char* resNames, const char* filePath, bool unflaggedOnly)
{
   const U32 numResourceTypes = 9;
   Vector<GFXResource*> resVectors[numResourceTypes];
   const char* reslabels[numResourceTypes] = { "texture", "texture target", "window target", "vertex buffers", "primitive buffers", "fences", "cubemaps", "shaders", "stateblocks" };   

   // Fill the vectors with the right resources
   fillResourceVectors(resNames, unflaggedOnly, resVectors[0], resVectors[1], resVectors[2], resVectors[3], 
      resVectors[4], resVectors[5], resVectors[6], resVectors[7], resVectors[8]);

   // Helper object
   DescriptionOutputter output(filePath);

   // Print the info to the file
   // Note that we check if we have any objects of that type.
   for (U32 i = 0; i < numResourceTypes; i++)
   {
      if (resVectors[i].size())
      {
         // Header
         String header = String::ToString("--------Dumping GFX %s descriptions...----------", reslabels[i]);
         output.write(header);
         // Data
         for (U32 j = 0; j < resVectors[i].size(); j++)
         {
            GFXResource* resource = resVectors[i][j];
            String dataline = String::ToString("Addr: %x %s", resource, resource->describeSelf().c_str());
            output.write(dataline.c_str());
         }
         // Footer
         output.write("--------------------Done---------------------");
         output.write("");
      }
   }
}

void GFXDevice::flagCurrentResources()
{
   GFXResource* walk = mResourceListHead;
   while(walk)
   {
      walk->setFlag();
      walk = walk->getNextResource();
   }
}

void GFXDevice::clearResourceFlags()
{
   GFXResource* walk = mResourceListHead;
   while(walk)
   {
      walk->clearFlag();
      walk = walk->getNextResource();
   }
}

DefineEngineFunction( listGFXResources, void, ( bool unflaggedOnly ), ( false ),
   "Returns a list of the unflagged GFX resources. See flagCurrentGFXResources for usage details.\n"
   "@ingroup GFX\n"
   "@see flagCurrentGFXResources, clearGFXResourceFlags, describeGFXResources" )
{
   GFX->listResources(unflaggedOnly);
}

DefineEngineFunction( flagCurrentGFXResources, void, (),,
   "@brief Flags all currently allocated GFX resources.\n"
   "Used for resource allocation and leak tracking by flagging "
   "current resources then dumping a list of unflagged resources "
   "at some later point in execution.\n"
   "@ingroup GFX\n"
   "@see listGFXResources, clearGFXResourceFlags, describeGFXResources" )
{
   GFX->flagCurrentResources();
}

DefineEngineFunction( clearGFXResourceFlags, void, (),,
   "Clears the flagged state on all allocated GFX resources. "
   "See flagCurrentGFXResources for usage details.\n"
   "@ingroup GFX\n"
   "@see flagCurrentGFXResources, listGFXResources, describeGFXResources" )
{
   GFX->clearResourceFlags();
}

DefineEngineFunction( describeGFXResources, void, ( const char *resourceTypes, const char *filePath, bool unflaggedOnly ), ( false ),
   "@brief Dumps a description of GFX resources to a file or the console.\n"
   "@param resourceTypes A space seperated list of resource types or an empty string for all resources.\n"
   "@param filePath A file to dump the list to or an empty string to write to the console.\n"
   "@param unflaggedOnly If true only unflagged resources are dumped. See flagCurrentGFXResources.\n"
   "@note The resource types can be one or more of the following:\n\n"
   "  - texture\n"
   "  - texture target\n"
   "  - window target\n"
   "  - vertex buffers\n"
   "  - primitive buffers\n"
   "  - fences\n"
   "  - cubemaps\n"
   "  - shaders\n"
   "  - stateblocks\n\n"
   "@ingroup GFX\n" )
{
   GFX->describeResources( resourceTypes, filePath, unflaggedOnly );
}

DefineEngineFunction( describeGFXStateBlocks, void, ( const char *filePath ),,
   "Dumps a description of all state blocks.\n"     
   "@param filePath A file to dump the state blocks to or an empty string to write to the console.\n"
   "@ingroup GFX\n" )
{
   GFX->dumpStates( filePath );   
}

DefineEngineFunction( getPixelShaderVersion, F32, (),,
   "Returns the pixel shader version for the active device.\n"
   "@ingroup GFX\n" )
{
   return GFX->getPixelShaderVersion();
}   

DefineEngineFunction( setPixelShaderVersion, void, ( F32 version ),,
   "@brief Sets the pixel shader version for the active device.\n"
   "This can be used to force a lower pixel shader version than is supported by "
   "the device for testing or performance optimization.\n"
   "@param version The floating point shader version number.\n"
   "@note This will only affect shaders/materials created after the call "
   "and should be used before the game begins.\n"
   "@see $pref::Video::forcedPixVersion\n"
   "@ingroup GFX\n" )
{
   GFX->setPixelShaderVersion( version );
}

DefineEngineFunction( getDisplayDeviceInformation, const char*, (),,
   "Get the string describing the active GFX device.\n"
   "@ingroup GFX\n" )
{
   if (!GFXDevice::devicePresent())
      return "(no device)";

   const GFXAdapter& adapter = GFX->getAdapter();
   return adapter.getName();
}

DefineEngineFunction( getBestHDRFormat, GFXFormat, (),,
   "Returns the best texture format for storage of HDR data for the active device.\n"
   "@ingroup GFX\n" )
{
   // TODO: Maybe expose GFX::selectSupportedFormat() so that this
   // specialized method can be moved to script.

   // Figure out the best HDR format.  This is the smallest
   // format which supports blending and filtering.
   Vector<GFXFormat> formats;
   formats.push_back( GFXFormatR10G10B10A2 );
   formats.push_back( GFXFormatR16G16B16A16F );
   formats.push_back( GFXFormatR16G16B16A16 );    
   GFXFormat format = GFX->selectSupportedFormat(  &GFXDefaultRenderTargetProfile,
                                                   formats, 
                                                   true,
                                                   true,
                                                   true );

   return format;
}

DefineConsoleFunction(ResetGFX, void, (), , "forces the gbuffer to be reinitialized in cases of improper/lack of buffer clears.")
{
   GFX->beginReset();
}