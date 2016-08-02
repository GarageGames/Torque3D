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

#include "gfx/D3D9/gfxD3D9Device.h"

#include "console/console.h"
#include "core/stream/fileStream.h"
#include "core/strings/unicode.h"
#include "gfx/D3D9/gfxD3D9CardProfiler.h"
#include "gfx/D3D9/gfxD3D9VertexBuffer.h"
#include "gfx/D3D9/screenShotD3D9.h"
#include "gfx/D3D9/gfxD3D9EnumTranslate.h"
#include "gfx/D3D9/gfxD3D9QueryFence.h"
#include "gfx/D3D9/gfxD3D9OcclusionQuery.h"
#include "gfx/D3D9/gfxD3D9Shader.h"
#include "windowManager/platformWindow.h"
#include "materials/shaderData.h"
#ifndef TORQUE_OS_XENON
#  include "windowManager/win32/win32Window.h"
#endif

D3DXFNTable GFXD3D9Device::smD3DX;

GFXD3D9Device::GFXD3D9Device( LPDIRECT3D9 d3d, U32 index ) 
{
   mDeviceSwizzle32 = &Swizzles::bgra;
   GFXVertexColor::setSwizzle( mDeviceSwizzle32 );

   mDeviceSwizzle24 = &Swizzles::bgr;

   mD3D = d3d;
   mAdapterIndex = index;
   mD3DDevice = NULL;
   mVolatileVB = NULL;

   mCurrentPB = NULL;
   mDynamicPB = NULL;

   mLastVertShader = NULL;
   mLastPixShader = NULL;

   mCanCurrentlyRender = false;
   mTextureManager = NULL;
   mCurrentStateBlock = NULL;
   mResourceListHead = NULL;

#ifdef TORQUE_DEBUG
   mVBListHead = NULL;
   mNumAllocatedVertexBuffers = 0;
#endif

   mPixVersion = 0.0;
   mNumSamplers = 0;
   mNumRenderTargets = 0;

   mCardProfiler = NULL;

   mDeviceDepthStencil = NULL;
   mDeviceBackbuffer = NULL;
   mDeviceColor = NULL;

   mCreateFenceType = -1; // Unknown, test on first allocate

   mCurrentConstBuffer = NULL;

   mOcclusionQuerySupported = false;

   for (U32 i = 0; i < GS_COUNT; ++i) 
      mModelViewProjSC[i] = NULL;

   // Set up the Enum translation tables
   GFXD3D9EnumTranslate::init();

#if !defined(TORQUE_OS_XENON)
   mD3DEx = NULL;
   mD3DDeviceEx = NULL;
#endif
}

//-----------------------------------------------------------------------------

GFXD3D9Device::~GFXD3D9Device() 
{
   // Release our refcount on the current stateblock object
   mCurrentStateBlock = NULL;

   releaseDefaultPoolResources();

   // Free the vertex declarations.
   VertexDeclMap::Iterator iter = mVertexDecls.begin();
   for ( ; iter != mVertexDecls.end(); iter++ )
      delete iter->value;

   // Check up on things
   Con::printf("Cur. D3DDevice ref count=%d", mD3DDevice->AddRef() - 1);
   mD3DDevice->Release();

   // Forcibly clean up the pools
   mVolatileVBList.setSize(0);
   mDynamicPB = NULL;

   // And release our D3D resources.
   SAFE_RELEASE( mDeviceDepthStencil );
   SAFE_RELEASE( mDeviceBackbuffer )
   SAFE_RELEASE( mDeviceColor );
   SAFE_RELEASE( mD3D );
   SAFE_RELEASE( mD3DDevice );

#ifdef TORQUE_DEBUG
   logVertexBuffers();
#endif

   if( mCardProfiler )
   {
      delete mCardProfiler;
      mCardProfiler = NULL;
   }

   if( gScreenShot )
   {
      delete gScreenShot;
      gScreenShot = NULL;
   }
}

//------------------------------------------------------------------------------
// setupGenericShaders 
//------------------------------------------------------------------------------
inline void GFXD3D9Device::setupGenericShaders( GenericShaderType type /* = GSColor */ )
{
   AssertFatal(type != GSTargetRestore, ""); //not used

   if (mGenericShader[GSColor] == NULL)
   {
      ShaderData *shaderData;

      shaderData = new ShaderData();
      shaderData->setField("DXVertexShaderFile", "shaders/common/fixedFunction/colorV.hlsl");
      shaderData->setField("DXPixelShaderFile", "shaders/common/fixedFunction/colorP.hlsl");
      shaderData->setField("pixVersion", "3.0");
      shaderData->registerObject();
      mGenericShader[GSColor] = shaderData->getShader();
      mGenericShaderBuffer[GSColor] = mGenericShader[GSColor]->allocConstBuffer();
      mModelViewProjSC[GSColor] = mGenericShader[GSColor]->getShaderConstHandle("$modelView");
      Sim::getRootGroup()->addObject(shaderData);

      shaderData = new ShaderData();
      shaderData->setField("DXVertexShaderFile", "shaders/common/fixedFunction/modColorTextureV.hlsl");
      shaderData->setField("DXPixelShaderFile", "shaders/common/fixedFunction/modColorTextureP.hlsl");
      shaderData->setSamplerName("$diffuseMap", 0);
      shaderData->setField("pixVersion", "3.0");
      shaderData->registerObject();
      mGenericShader[GSModColorTexture] = shaderData->getShader();
      mGenericShaderBuffer[GSModColorTexture] = mGenericShader[GSModColorTexture]->allocConstBuffer();
      mModelViewProjSC[GSModColorTexture] = mGenericShader[GSModColorTexture]->getShaderConstHandle("$modelView");
      Sim::getRootGroup()->addObject(shaderData);

      shaderData = new ShaderData();
      shaderData->setField("DXVertexShaderFile", "shaders/common/fixedFunction/addColorTextureV.hlsl");
      shaderData->setField("DXPixelShaderFile", "shaders/common/fixedFunction/addColorTextureP.hlsl");
      shaderData->setSamplerName("$diffuseMap", 0);
      shaderData->setField("pixVersion", "3.0");
      shaderData->registerObject();
      mGenericShader[GSAddColorTexture] = shaderData->getShader();
      mGenericShaderBuffer[GSAddColorTexture] = mGenericShader[GSAddColorTexture]->allocConstBuffer();
      mModelViewProjSC[GSAddColorTexture] = mGenericShader[GSAddColorTexture]->getShaderConstHandle("$modelView");
      Sim::getRootGroup()->addObject(shaderData);

      shaderData = new ShaderData();
      shaderData->setField("DXVertexShaderFile", "shaders/common/fixedFunction/textureV.hlsl");
      shaderData->setField("DXPixelShaderFile", "shaders/common/fixedFunction/textureP.hlsl");
      shaderData->setSamplerName("$diffuseMap", 0);
      shaderData->setField("pixVersion", "3.0");
      shaderData->registerObject();
      mGenericShader[GSTexture] = shaderData->getShader();
      mGenericShaderBuffer[GSTexture] = mGenericShader[GSTexture]->allocConstBuffer();
      mModelViewProjSC[GSTexture] = mGenericShader[GSTexture]->getShaderConstHandle("$modelView");
      Sim::getRootGroup()->addObject(shaderData);

      //Force an update
      mViewportDirty = true;
   }

   MatrixF tempMatrix = mProjectionMatrix * mViewMatrix * mWorldMatrix[mWorldStackSize];
   mGenericShaderBuffer[type]->setSafe(mModelViewProjSC[type], tempMatrix);

   setShader(mGenericShader[type]);
   setShaderConstBuffer(mGenericShaderBuffer[type]);
}

//-----------------------------------------------------------------------------
/// Creates a state block object based on the desc passed in.  This object
/// represents an immutable state.
GFXStateBlockRef GFXD3D9Device::createStateBlockInternal(const GFXStateBlockDesc& desc)
{
   return GFXStateBlockRef(new GFXD3D9StateBlock(desc, mD3DDevice));
}

/// Activates a stateblock
void GFXD3D9Device::setStateBlockInternal(GFXStateBlock* block, bool force)
{
   AssertFatal(dynamic_cast<GFXD3D9StateBlock*>(block), "Incorrect stateblock type for this device!");
   GFXD3D9StateBlock* d3dBlock = static_cast<GFXD3D9StateBlock*>(block);
   GFXD3D9StateBlock* d3dCurrent = static_cast<GFXD3D9StateBlock*>(mCurrentStateBlock.getPointer());
   if (force)
      d3dCurrent = NULL;
   d3dBlock->activate(d3dCurrent);   
}

/// Called by base GFXDevice to actually set a const buffer
void GFXD3D9Device::setShaderConstBufferInternal(GFXShaderConstBuffer* buffer)
{
   if (buffer)
   {
      PROFILE_SCOPE(GFXD3D9Device_setShaderConstBufferInternal);
      AssertFatal(dynamic_cast<GFXD3D9ShaderConstBuffer*>(buffer), "Incorrect shader const buffer type for this device!");
      GFXD3D9ShaderConstBuffer* d3dBuffer = static_cast<GFXD3D9ShaderConstBuffer*>(buffer);

      d3dBuffer->activate(mCurrentConstBuffer);
      mCurrentConstBuffer = d3dBuffer;
   } else {
      mCurrentConstBuffer = NULL;
   }
}

//-----------------------------------------------------------------------------

void GFXD3D9Device::clear( U32 flags, ColorI color, F32 z, U32 stencil ) 
{
   // Make sure we have flushed our render target state.
   _updateRenderTargets();

   // Kind of a bummer we have to do this, there should be a better way made
   DWORD realflags = 0;

   if( flags & GFXClearTarget )
      realflags |= D3DCLEAR_TARGET;

   if( flags & GFXClearZBuffer )
      realflags |= D3DCLEAR_ZBUFFER;

   if( flags & GFXClearStencil )
      realflags |= D3DCLEAR_STENCIL;

   mD3DDevice->Clear( 0, NULL, realflags, 
      D3DCOLOR_ARGB( color.alpha, color.red, color.green, color.blue ), 
      z, stencil );
}

//-----------------------------------------------------------------------------

bool GFXD3D9Device::beginSceneInternal() 
{
   HRESULT hr = mD3DDevice->BeginScene();
   D3D9Assert(hr, "GFXD3D9Device::beginSceneInternal - failed to BeginScene");
   mCanCurrentlyRender = SUCCEEDED(hr);
   return mCanCurrentlyRender;      
}

//-----------------------------------------------------------------------------

void GFXD3D9Device::endSceneInternal() 
{
   mD3DDevice->EndScene();
   mCanCurrentlyRender = false;
}

void GFXD3D9Device::_updateRenderTargets()
{
   if ( mRTDirty || ( mCurrentRT && mCurrentRT->isPendingState() ) )
   {
      if ( mRTDeactivate )
      {
         mRTDeactivate->deactivate();
         mRTDeactivate = NULL;   
      }

      // NOTE: The render target changes are not really accurate
      // as the GFXTextureTarget supports MRT internally.  So when
      // we activate a GFXTarget it could result in multiple calls
      // to SetRenderTarget on the actual device.
      mDeviceStatistics.mRenderTargetChanges++;

      mCurrentRT->activate();

      mRTDirty = false;
   }  

   if ( mViewportDirty )
   {
      D3DVIEWPORT9 viewport;
      viewport.X       = mViewport.point.x;
      viewport.Y       = mViewport.point.y;
      viewport.Width   = mViewport.extent.x;
      viewport.Height  = mViewport.extent.y;
      viewport.MinZ    = 0.0;
      viewport.MaxZ    = 1.0;

      D3D9Assert( mD3DDevice->SetViewport( &viewport ), 
         "GFXD3D9Device::_updateRenderTargets() - Error setting viewport!" );

      mViewportDirty = false;
   }
}


#ifdef TORQUE_DEBUG

void GFXD3D9Device::logVertexBuffers() 
{

   // NOTE: This function should be called on the destructor of this class and ONLY then
   // otherwise it'll produce the wrong output
   if( mNumAllocatedVertexBuffers == 0 )
      return;

   FileStream fs;

   fs.open( "vertexbuffer.log", Torque::FS::File::Write );

   char buff[256];

   fs.writeLine( (U8 *)avar("-- Vertex buffer memory leak report -- time = %d", Platform::getRealMilliseconds()) );
   dSprintf( (char *)&buff, sizeof( buff ), "%d un-freed vertex buffers", mNumAllocatedVertexBuffers );
   fs.writeLine( (U8 *)buff );

   GFXD3D9VertexBuffer *walk = mVBListHead;

   while( walk != NULL ) 
   {
      dSprintf( (char *)&buff, sizeof( buff ), "[Name: %s] Size: %d", walk->name, walk->mNumVerts );
      fs.writeLine( (U8 *)buff );

      walk = walk->next;
   }

   fs.writeLine( (U8 *)"-- End report --" );

   fs.close();
}

//-----------------------------------------------------------------------------

void GFXD3D9Device::addVertexBuffer( GFXD3D9VertexBuffer *buffer ) 
{
   mNumAllocatedVertexBuffers++;

   if( mVBListHead == NULL ) 
   {
      mVBListHead = buffer;
   }
   else 
   {
      GFXD3D9VertexBuffer *walk = mVBListHead;

      while( walk->next != NULL ) 
      {
         walk = walk->next;
      }

      walk->next = buffer;
   }

   buffer->next = NULL;
}

//-----------------------------------------------------------------------------

void GFXD3D9Device::removeVertexBuffer( GFXD3D9VertexBuffer *buffer ) 
{
   mNumAllocatedVertexBuffers--;

   // Quick check to see if this is head of list
   if( mVBListHead == buffer ) 
   {
      mVBListHead = mVBListHead->next;
      return;
   }

   GFXD3D9VertexBuffer *walk = mVBListHead;

   while( walk->next != NULL ) 
   {
      if( walk->next == buffer ) 
      {
         walk->next = walk->next->next;
         return;
      }

      walk = walk->next;
   }

   AssertFatal( false, "Vertex buffer not found in list." );
}

#endif

//-----------------------------------------------------------------------------

void GFXD3D9Device::releaseDefaultPoolResources() 
{
   // Release all the dynamic vertex buffer arrays
   // Forcibly clean up the pools
   for( U32 i=0; i<mVolatileVBList.size(); i++ )
   {
      // Con::printf("Trying to release volatile vb with COM refcount of %d and internal refcount of %d", mVolatileVBList[i]->vb->AddRef() - 1, mVolatileVBList[i]->mRefCount);  
      // mVolatileVBList[i]->vb->Release();

      mVolatileVBList[i]->vb->Release();
      mVolatileVBList[i]->vb = NULL;
      mVolatileVBList[i] = NULL;
   }
   mVolatileVBList.setSize(0);

   // We gotta clear the current const buffer else the next
   // activate may erroneously think the device is still holding
   // this state and fail to set it.   
   mCurrentConstBuffer = NULL;

   // Set current VB to NULL and set state dirty
   for ( U32 i=0; i < VERTEX_STREAM_COUNT; i++ )
   {
      mCurrentVertexBuffer[i] = NULL;
      mVertexBufferDirty[i] = true;
      mVertexBufferFrequency[i] = 0;
      mVertexBufferFrequencyDirty[i] = true;
   }

   // Release dynamic index buffer
   if( mDynamicPB != NULL )
   {
      SAFE_RELEASE( mDynamicPB->ib );
   }

   // Set current PB/IB to NULL and set state dirty
   mCurrentPrimitiveBuffer = NULL;
   mCurrentPB = NULL;
   mPrimitiveBufferDirty = true;

   // Zombify texture manager (for D3D this only modifies default pool textures)
   if( mTextureManager ) 
      mTextureManager->zombify();

   // Kill off other potentially dangling references...
   SAFE_RELEASE( mDeviceDepthStencil );
   SAFE_RELEASE( mDeviceBackbuffer );
   mD3DDevice->SetDepthStencilSurface(NULL);

   // Set global dirty state so the IB/PB and VB get reset
   mStateDirty = true;

   // Walk the resource list and zombify everything.
   GFXResource *walk = mResourceListHead;
   while(walk)
   {
      walk->zombify();
      walk = walk->getNextResource();
   }
}

//-----------------------------------------------------------------------------

void GFXD3D9Device::reacquireDefaultPoolResources() 
{
   // Now do the dynamic index buffers
   if( mDynamicPB == NULL )
      mDynamicPB = new GFXD3D9PrimitiveBuffer(this, 0, 0, GFXBufferTypeDynamic);

   D3D9Assert( mD3DDevice->CreateIndexBuffer( sizeof( U16 ) * MAX_DYNAMIC_INDICES, 
#ifdef TORQUE_OS_XENON
      D3DUSAGE_WRITEONLY,
#else
      D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC,
#endif
      GFXD3D9IndexFormat[GFXIndexFormat16], D3DPOOL_DEFAULT, &mDynamicPB->ib, NULL ), "Failed to allocate dynamic IB" );

   // Grab the depth-stencil...
   SAFE_RELEASE(mDeviceDepthStencil);   
   D3D9Assert(mD3DDevice->GetDepthStencilSurface(&mDeviceDepthStencil), 
      "GFXD3D9Device::reacquireDefaultPoolResources - couldn't grab reference to device's depth-stencil surface.");  

   SAFE_RELEASE(mDeviceBackbuffer);
   mD3DDevice->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &mDeviceBackbuffer );

   // Store for query later.
   mD3DDevice->GetDisplayMode( 0, &mDisplayMode );

   // Walk the resource list and zombify everything.
   GFXResource *walk = mResourceListHead;
   while(walk)
   {
      walk->resurrect();
      walk = walk->getNextResource();
   }

   if(mTextureManager)
      mTextureManager->resurrect();
}

GFXD3D9VertexBuffer* GFXD3D9Device::findVBPool( const GFXVertexFormat *vertexFormat, U32 vertsNeeded )
{
   PROFILE_SCOPE( GFXD3D9Device_findVBPool );

   // Verts needed is ignored on the base device, 360 is different
   for( U32 i=0; i<mVolatileVBList.size(); i++ )
      if( mVolatileVBList[i]->mVertexFormat.isEqual( *vertexFormat ) )
         return mVolatileVBList[i];

   return NULL;
}

GFXD3D9VertexBuffer * GFXD3D9Device::createVBPool( const GFXVertexFormat *vertexFormat, U32 vertSize )
{
   PROFILE_SCOPE( GFXD3D9Device_createVBPool );

   // this is a bit funky, but it will avoid problems with (lack of) copy constructors
   //    with a push_back() situation
   mVolatileVBList.increment();
   StrongRefPtr<GFXD3D9VertexBuffer> newBuff;
   mVolatileVBList.last() = new GFXD3D9VertexBuffer();
   newBuff = mVolatileVBList.last();

   newBuff->mNumVerts   = 0;
   newBuff->mBufferType = GFXBufferTypeVolatile;
   newBuff->mVertexFormat.copy( *vertexFormat );
   newBuff->mVertexSize = vertSize;
   newBuff->mDevice = this;

   // Requesting it will allocate it.
   vertexFormat->getDecl();

   //   Con::printf("Created buff with type %x", vertFlags);

   D3D9Assert( mD3DDevice->CreateVertexBuffer( vertSize * MAX_DYNAMIC_VERTS, 
#ifdef TORQUE_OS_XENON
      D3DUSAGE_WRITEONLY,
#else
      D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC,
#endif
      0, 
      D3DPOOL_DEFAULT, 
      &newBuff->vb, 
      NULL ), 
      "Failed to allocate dynamic VB" );
   return newBuff;
}

//-----------------------------------------------------------------------------

void GFXD3D9Device::setClipRect( const RectI &inRect ) 
{
	// We transform the incoming rect by the view 
   // matrix first, so that it can be used to pan
   // and scale the clip rect.
   //
   // This is currently used to take tiled screenshots.
	Point3F pos( inRect.point.x, inRect.point.y, 0.0f );
   Point3F extent( inRect.extent.x, inRect.extent.y, 0.0f );
   getViewMatrix().mulP( pos );
   getViewMatrix().mulV( extent );  
   RectI rect( pos.x, pos.y, extent.x, extent.y );

   // Clip the rect against the renderable size.
   Point2I size = mCurrentRT->getSize();

   RectI maxRect(Point2I(0,0), size);
   rect.intersect(maxRect);

   mClipRect = rect;

   F32 l = F32( mClipRect.point.x );
   F32 r = F32( mClipRect.point.x + mClipRect.extent.x );
   F32 b = F32( mClipRect.point.y + mClipRect.extent.y );
   F32 t = F32( mClipRect.point.y );

   // Set up projection matrix, 
   static Point4F pt;   
   pt.set(2.0f / (r - l), 0.0f, 0.0f, 0.0f);
   mTempMatrix.setColumn(0, pt);

   pt.set(0.0f, 2.0f/(t - b), 0.0f, 0.0f);
   mTempMatrix.setColumn(1, pt);

   pt.set(0.0f, 0.0f, 1.0f, 0.0f);
   mTempMatrix.setColumn(2, pt);

   pt.set((l+r)/(l-r), (t+b)/(b-t), 1.0f, 1.0f);
   mTempMatrix.setColumn(3, pt);

   setProjectionMatrix( mTempMatrix );

   // Set up world/view matrix
   mTempMatrix.identity();   
   setWorldMatrix( mTempMatrix );

   setViewport( mClipRect );
}

void GFXD3D9Device::setVertexStream( U32 stream, GFXVertexBuffer *buffer )
{
   GFXD3D9VertexBuffer *d3dBuffer = static_cast<GFXD3D9VertexBuffer*>( buffer );

   if ( stream == 0 )
   {
      // Set the volatile buffer which is used to 
      // offset the start index when doing draw calls.
      if ( d3dBuffer && d3dBuffer->mVolatileStart > 0 )
         mVolatileVB = d3dBuffer;
      else
         mVolatileVB = NULL;
   }

   // NOTE: We do not use the stream offset here for stream 0
   // as that feature is *supposedly* not as well supported as 
   // using the start index in drawPrimitive.
   //
   // If we can verify that this is not the case then we should
   // start using this method exclusively for all streams.
   
   D3D9Assert( mD3DDevice->SetStreamSource(  stream, 
                                             d3dBuffer ? d3dBuffer->vb : NULL,
                                             d3dBuffer && stream != 0 ? d3dBuffer->mVolatileStart * d3dBuffer->mVertexSize : 0, 
                                             d3dBuffer ? d3dBuffer->mVertexSize : 0 ),
                                             "GFXD3D9Device::setVertexStream - Failed to set stream source." );
}

void GFXD3D9Device::setVertexStreamFrequency( U32 stream, U32 frequency )
{
   if ( frequency == 0 )
      frequency = 1;
   else
   {
      if ( stream == 0 )
         frequency = D3DSTREAMSOURCE_INDEXEDDATA | frequency;
      else
         frequency = D3DSTREAMSOURCE_INSTANCEDATA | frequency;
   }

   D3D9Assert( mD3DDevice->SetStreamSourceFreq( stream, frequency ),
      "GFXD3D9Device::setVertexStreamFrequency - Failed to set stream frequency." );
}

void GFXD3D9Device::_setPrimitiveBuffer( GFXPrimitiveBuffer *buffer ) 
{
   mCurrentPB = static_cast<GFXD3D9PrimitiveBuffer *>( buffer );

   D3D9Assert( mD3DDevice->SetIndices( mCurrentPB->ib ), "Failed to set indices" );
}

void GFXD3D9Device::drawPrimitive( GFXPrimitiveType primType, U32 vertexStart, U32 primitiveCount ) 
{
   // This is done to avoid the function call overhead if possible
   if( mStateDirty )
      updateStates();
   if (mCurrentShaderConstBuffer)
      setShaderConstBufferInternal(mCurrentShaderConstBuffer);

   if ( mVolatileVB )
      vertexStart += mVolatileVB->mVolatileStart;

   D3D9Assert( mD3DDevice->DrawPrimitive( GFXD3D9PrimType[primType], vertexStart, primitiveCount ), "Failed to draw primitives" );  

   mDeviceStatistics.mDrawCalls++;
   if ( mVertexBufferFrequency[0] > 1 )
      mDeviceStatistics.mPolyCount += primitiveCount * mVertexBufferFrequency[0];
   else
      mDeviceStatistics.mPolyCount += primitiveCount;
}

//-----------------------------------------------------------------------------

void GFXD3D9Device::drawIndexedPrimitive( GFXPrimitiveType primType, 
                                          U32 startVertex, 
                                          U32 minIndex, 
                                          U32 numVerts, 
                                          U32 startIndex, 
                                          U32 primitiveCount ) 
{
   // This is done to avoid the function call overhead if possible
   if( mStateDirty )
      updateStates();
   if (mCurrentShaderConstBuffer)
      setShaderConstBufferInternal(mCurrentShaderConstBuffer);

   AssertFatal( mCurrentPB != NULL, "Trying to call drawIndexedPrimitive with no current index buffer, call setIndexBuffer()" );

   if ( mVolatileVB )
      startVertex += mVolatileVB->mVolatileStart;

   D3D9Assert( mD3DDevice->DrawIndexedPrimitive(   GFXD3D9PrimType[primType], 
      startVertex, 
      /* mCurrentPB->mVolatileStart + */ minIndex,
      numVerts, 
      mCurrentPB->mVolatileStart + startIndex, 
      primitiveCount ), "Failed to draw indexed primitive" );

   mDeviceStatistics.mDrawCalls++;
   if ( mVertexBufferFrequency[0] > 1 )
      mDeviceStatistics.mPolyCount += primitiveCount * mVertexBufferFrequency[0];
   else
      mDeviceStatistics.mPolyCount += primitiveCount;
}

GFXShader* GFXD3D9Device::createShader()
{
   GFXD3D9Shader* shader = new GFXD3D9Shader();
   shader->registerResourceWithDevice( this );
   return shader;
}

void GFXD3D9Device::disableShaders(bool force)
{
   setShader( NULL, force );
   setShaderConstBuffer( NULL );
}

//-----------------------------------------------------------------------------
// Set shader - this function exists to make sure this is done in one place,
//              and to make sure redundant shader states are not being
//              sent to the card.
//-----------------------------------------------------------------------------
void GFXD3D9Device::setShader( GFXShader *shader, bool force )
{
   GFXD3D9Shader *d3dShader = static_cast<GFXD3D9Shader*>( shader );

   IDirect3DPixelShader9 *pixShader = ( d3dShader != NULL ? d3dShader->mPixShader : NULL );
   IDirect3DVertexShader9 *vertShader = ( d3dShader ? d3dShader->mVertShader : NULL );

   if( pixShader != mLastPixShader || force )
   {
      mD3DDevice->SetPixelShader( pixShader );
      mLastPixShader = pixShader;
   }

   if( vertShader != mLastVertShader || force )
   {
      mD3DDevice->SetVertexShader( vertShader );
      mLastVertShader = vertShader;
   }
}

//-----------------------------------------------------------------------------
// allocPrimitiveBuffer
//-----------------------------------------------------------------------------
GFXPrimitiveBuffer * GFXD3D9Device::allocPrimitiveBuffer(   U32 numIndices, 
                                                            U32 numPrimitives, 
                                                            GFXBufferType bufferType,
                                                            void* data )
{
   // Allocate a buffer to return
   GFXD3D9PrimitiveBuffer * res = new GFXD3D9PrimitiveBuffer(this, numIndices, numPrimitives, bufferType);

   // Determine usage flags
   U32 usage = 0;
   D3DPOOL pool = D3DPOOL_DEFAULT;

   // Assumptions:
   //    - static buffers are write rarely, use many
   //    - dynamic buffers are write many, use many
   //    - volatile buffers are write once, use once
   // You may never read from a buffer.
   switch(bufferType)
   {
   case GFXBufferTypeImmutable:
   case GFXBufferTypeStatic:
      pool = isD3D9Ex() ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED;
      break;

   case GFXBufferTypeDynamic:
   case GFXBufferTypeVolatile:
#ifndef TORQUE_OS_XENON
      usage |= D3DUSAGE_DYNAMIC;
#endif
      break;
   }

   // Register resource
   res->registerResourceWithDevice(this);

   // We never allow reading from a primitive buffer.
   usage |= D3DUSAGE_WRITEONLY;

   // Create d3d index buffer
   if(bufferType == GFXBufferTypeVolatile)
   {
      // Get it from the pool if it's a volatile...
      AssertFatal( numIndices < MAX_DYNAMIC_INDICES, "Cannot allocate that many indices in a volatile buffer, increase MAX_DYNAMIC_INDICES." );

      res->ib              = mDynamicPB->ib;
      // mDynamicPB->ib->AddRef();
      res->mVolatileBuffer = mDynamicPB;
   }
   else
   {
      // Otherwise, get it as a seperate buffer...
      D3D9Assert(mD3DDevice->CreateIndexBuffer( sizeof(U16) * numIndices , usage, GFXD3D9IndexFormat[GFXIndexFormat16], pool, &res->ib, 0),
         "Failed to allocate an index buffer.");
   }
   
   if(data)
   {
      void* dest;
      res->lock(0, numIndices, &dest);
      dMemcpy(dest, data, sizeof(U16) * numIndices);
      res->unlock();
   }

   return res;
}

//-----------------------------------------------------------------------------
// allocVertexBuffer
//-----------------------------------------------------------------------------
GFXVertexBuffer * GFXD3D9Device::allocVertexBuffer(   U32 numVerts, 
                                                      const GFXVertexFormat *vertexFormat, 
                                                      U32 vertSize, 
                                                      GFXBufferType bufferType, 
                                                      void* data)
{
   PROFILE_SCOPE( GFXD3D9Device_allocVertexBuffer );

   GFXD3D9VertexBuffer *res = new GFXD3D9VertexBuffer(   this, 
                                                         numVerts, 
                                                         vertexFormat, 
                                                         vertSize, 
                                                         bufferType );

   // Determine usage flags
   U32 usage = 0;
   D3DPOOL pool = D3DPOOL_DEFAULT;

   res->mNumVerts = 0;

   // Assumptions:
   //    - static buffers are write rarely, use many
   //    - dynamic buffers are write many, use many
   //    - volatile buffers are write once, use once
   // You may never read from a buffer.

   switch(bufferType)
   {
   case GFXBufferTypeImmutable:
   case GFXBufferTypeStatic:
      pool = isD3D9Ex() ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED;
      break;

   case GFXBufferTypeDynamic:
   case GFXBufferTypeVolatile:
      pool = D3DPOOL_DEFAULT;
      usage |= D3DUSAGE_WRITEONLY;
#ifndef TORQUE_OS_XENON
      usage |= D3DUSAGE_DYNAMIC;
#endif
      break;
   }

   res->registerResourceWithDevice(this);

   // Create vertex buffer
   if( bufferType == GFXBufferTypeVolatile )
   {
      // NOTE: Volatile VBs are pooled and will be allocated at lock time.

      AssertFatal( numVerts <= MAX_DYNAMIC_VERTS, 
         "GFXD3D9Device::allocVertexBuffer - Volatile vertex buffer is too big... see MAX_DYNAMIC_VERTS!" );
   }
   else
   {
      // Requesting it will allocate it.
      vertexFormat->getDecl();

      // Get a new buffer...
      D3D9Assert( mD3DDevice->CreateVertexBuffer( vertSize * numVerts, usage, 0, pool, &res->vb, NULL ), 
         "Failed to allocate VB" );
   }

   res->mNumVerts = numVerts;
   
   if(data)
   {
      void* dest;
      res->lock(0, numVerts, &dest);
      dMemcpy(dest, data, vertSize * numVerts);
      res->unlock();
   }
   
   return res;
}

//-----------------------------------------------------------------------------
// deallocate vertex buffer
//-----------------------------------------------------------------------------
void GFXD3D9Device::deallocVertexBuffer( GFXD3D9VertexBuffer *vertBuff )
{
   SAFE_RELEASE(vertBuff->vb);
}

GFXVertexDecl* GFXD3D9Device::allocVertexDecl( const GFXVertexFormat *vertexFormat )
{
   PROFILE_SCOPE( GFXD3D9Device_allocVertexDecl );

   // First check the map... you shouldn't allocate VBs very often
   // if you want performance.  The map lookup should never become
   // a performance bottleneck.
   D3D9VertexDecl *decl = mVertexDecls[vertexFormat->getDescription()];
   if ( decl )
      return decl;

   // Setup the declaration struct.
   U32 elemCount = vertexFormat->getElementCount();
   U32 offsets[4] = { 0 };
   U32 stream;
   D3DVERTEXELEMENT9 *vd = new D3DVERTEXELEMENT9[ elemCount + 1 ];
   for ( U32 i=0; i < elemCount; i++ )
   {
      const GFXVertexElement &element = vertexFormat->getElement( i );
      
      stream = element.getStreamIndex();

      vd[i].Stream = stream;
      vd[i].Offset = offsets[stream];
      vd[i].Type = GFXD3D9DeclType[element.getType()];
      vd[i].Method = D3DDECLMETHOD_DEFAULT;

      // We force the usage index of 0 for everything but 
      // texture coords for now... this may change later.
      vd[i].UsageIndex = 0;

      if ( element.isSemantic( GFXSemantic::POSITION ) )
         vd[i].Usage = D3DDECLUSAGE_POSITION;
      else if ( element.isSemantic( GFXSemantic::NORMAL ) )
         vd[i].Usage = D3DDECLUSAGE_NORMAL;
      else if ( element.isSemantic( GFXSemantic::COLOR ) )
         vd[i].Usage = D3DDECLUSAGE_COLOR;
      else if ( element.isSemantic( GFXSemantic::TANGENT ) )
         vd[i].Usage = D3DDECLUSAGE_TANGENT;
      else if ( element.isSemantic( GFXSemantic::BINORMAL ) )
         vd[i].Usage = D3DDECLUSAGE_BINORMAL;
      else
      {
         // Anything that falls thru to here will be a texture coord.
         vd[i].Usage = D3DDECLUSAGE_TEXCOORD;
         vd[i].UsageIndex = element.getSemanticIndex();
      }

      offsets[stream] += element.getSizeInBytes();
   }

   D3DVERTEXELEMENT9 declEnd = D3DDECL_END();
   vd[elemCount] = declEnd;

   decl = new D3D9VertexDecl();
   D3D9Assert( mD3DDevice->CreateVertexDeclaration( vd, &decl->decl ), 
      "GFXD3D9Device::allocVertexDecl - Failed to create vertex declaration!" );

   delete [] vd;

   // Store it in the cache.
   mVertexDecls[vertexFormat->getDescription()] = decl;

   return decl;
}

void GFXD3D9Device::setVertexDecl( const GFXVertexDecl *decl )
{
   IDirect3DVertexDeclaration9 *dx9Decl = NULL;
   if ( decl )
      dx9Decl = static_cast<const D3D9VertexDecl*>( decl )->decl;
   D3D9Assert( mD3DDevice->SetVertexDeclaration( dx9Decl ), "GFXD3D9Device::setVertexDecl - Failed to set vertex declaration." );
}

//-----------------------------------------------------------------------------
// This function should ONLY be called from GFXDevice::updateStates() !!!
//-----------------------------------------------------------------------------
void GFXD3D9Device::setTextureInternal( U32 textureUnit, const GFXTextureObject *texture)
{
   if( texture == NULL )
   {
      D3D9Assert(mD3DDevice->SetTexture( textureUnit, NULL ), "Failed to set texture to null!");
      return;
   }

   GFXD3D9TextureObject *tex = (GFXD3D9TextureObject *) texture;
   D3D9Assert(mD3DDevice->SetTexture( textureUnit, tex->getTex()), "Failed to set texture to valid value!");
}

//-----------------------------------------------------------------------------
// This function should ONLY be called from GFXDevice::updateStates() !!!
//-----------------------------------------------------------------------------
void GFXD3D9Device::setLightInternal(U32 lightStage, const GFXLightInfo light, bool lightEnable)
{
#ifndef TORQUE_OS_XENON
   if(!lightEnable)
   {
      mD3DDevice->LightEnable(lightStage, false);
      return;
   }
   D3DLIGHT9 d3dLight;
   switch (light.mType)
   {
   case GFXLightInfo::Ambient:
      AssertFatal(false, "Instead of setting an ambient light you should set the global ambient color.");
      return;
   case GFXLightInfo::Vector:
      d3dLight.Type = D3DLIGHT_DIRECTIONAL;
      break;

   case GFXLightInfo::Point:
      d3dLight.Type = D3DLIGHT_POINT;
      break;

   case GFXLightInfo::Spot:      
      d3dLight.Type = D3DLIGHT_SPOT;
      break;

   default :
      AssertFatal(false, "Unknown light type!");
   };

   dMemcpy(&d3dLight.Diffuse, &light.mColor, sizeof(light.mColor));
   dMemcpy(&d3dLight.Ambient, &light.mAmbient, sizeof(light.mAmbient));
   dMemcpy(&d3dLight.Specular, &light.mColor, sizeof(light.mColor));
   dMemcpy(&d3dLight.Position, &light.mPos, sizeof(light.mPos));
   dMemcpy(&d3dLight.Direction, &light.mDirection, sizeof(light.mDirection));

   d3dLight.Range = light.mRadius;

   d3dLight.Falloff = 1.0;

   d3dLight.Attenuation0 = 1.0f;
   d3dLight.Attenuation1 = 0.1f;
   d3dLight.Attenuation2 = 0.0f;

   d3dLight.Theta = light.mInnerConeAngle;
   d3dLight.Phi = light.mOuterConeAngle;

   mD3DDevice->SetLight(lightStage, &d3dLight);
   mD3DDevice->LightEnable(lightStage, true); 
#endif
}

void GFXD3D9Device::setLightMaterialInternal(const GFXLightMaterial mat)
{
#ifndef TORQUE_OS_XENON
   D3DMATERIAL9 d3dmat;
   dMemset(&d3dmat, 0, sizeof(D3DMATERIAL9));
   D3DCOLORVALUE col;

   col.r = mat.ambient.red;
   col.g = mat.ambient.green;
   col.b = mat.ambient.blue;
   col.a = mat.ambient.alpha;
   d3dmat.Ambient = col;

   col.r = mat.diffuse.red;
   col.g = mat.diffuse.green;
   col.b = mat.diffuse.blue;
   col.a = mat.diffuse.alpha;
   d3dmat.Diffuse = col;

   col.r = mat.specular.red;
   col.g = mat.specular.green;
   col.b = mat.specular.blue;
   col.a = mat.specular.alpha;
   d3dmat.Specular = col;

   col.r = mat.emissive.red;
   col.g = mat.emissive.green;
   col.b = mat.emissive.blue;
   col.a = mat.emissive.alpha;
   d3dmat.Emissive = col;

   d3dmat.Power = mat.shininess;
   mD3DDevice->SetMaterial(&d3dmat);
#endif
}

void GFXD3D9Device::setGlobalAmbientInternal(ColorF color)
{
#ifndef TORQUE_OS_XENON
   mD3DDevice->SetRenderState(D3DRS_AMBIENT,
      D3DCOLOR_COLORVALUE(color.red, color.green, color.blue, color.alpha));
#endif
}

//------------------------------------------------------------------------------
// Check for texture mis-match between GFX internal state and what is on the card
// This function is expensive because of the readbacks from DX, and additionally
// won't work unless it's a non-pure device.
//
// This function can crash or give false positives when the game
// is shutting down or returning to the main menu as some of the textures
// present in the mCurrentTexture array will have been freed.
//
// This function is best used as a quick check for mismatched state when it is
// suspected.
//------------------------------------------------------------------------------
void GFXD3D9Device::doParanoidStateCheck()
{
#ifdef TORQUE_DEBUG
   // Read back all states and make sure they match what we think they should be.

   // For now just do texture binds.
   for(U32 i = 0; i < getNumSamplers(); i++)
   {
      IDirect3DBaseTexture9 *b=NULL;
      getDevice()->GetTexture(i, &b);
      if ((mCurrentTexture[i].isNull()) && (mCurrentCubemap[i].isNull()))
      {
         AssertFatal(b == NULL, "GFXD3D9Device::doParanoidStateCheck - got non-null texture in expected NULL slot!");
         getDevice()->SetTexture(i, NULL);
      }
      else
      {
         AssertFatal(mCurrentTexture[i] || mCurrentCubemap[i], "GFXD3D9Device::doParanoidStateCheck - got null texture in expected non-null slot!");
         if (mCurrentCubemap[i])         
         {
            IDirect3DCubeTexture9 *cur= static_cast<GFXD3D9Cubemap*>(mCurrentCubemap[i].getPointer())->mCubeTex;
            AssertFatal(cur == b, "GFXD3D9Device::doParanoidStateCheck - mismatched cubemap!");
         }
         else
         {
            IDirect3DBaseTexture9 *cur= static_cast<GFXD3D9TextureObject*>(mCurrentTexture[i].getPointer())->getTex();
            AssertFatal(cur == b, "GFXD3D9Device::doParanoidStateCheck - mismatched 2d texture!");
         }
      }

      SAFE_RELEASE(b);
   }
#endif
}

GFXFence *GFXD3D9Device::createFence()
{
   // Figure out what fence type we should be making if we don't know
   if( mCreateFenceType == -1 )
   {
      IDirect3DQuery9 *testQuery = NULL;
      mCreateFenceType = ( mD3DDevice->CreateQuery( D3DQUERYTYPE_EVENT, &testQuery ) == D3DERR_NOTAVAILABLE );
      SAFE_RELEASE( testQuery );
   }

   // Cool, use queries
   if( !mCreateFenceType )
   {
      GFXFence* fence = new GFXD3D9QueryFence( this );
      fence->registerResourceWithDevice(this);
      return fence;
   }

   // CodeReview: At some point I would like a specialized D3D9 implementation of
   // the method used by the general fence, only without the overhead incurred 
   // by using the GFX constructs. Primarily the lock() method on texture handles
   // will do a data copy, and this method doesn't require a copy, just a lock
   // [5/10/2007 Pat]
   GFXFence* fence = new GFXGeneralFence( this );
   fence->registerResourceWithDevice(this);
   return fence;
}

GFXOcclusionQuery* GFXD3D9Device::createOcclusionQuery()
{  
   GFXOcclusionQuery *query;
   if (mOcclusionQuerySupported)
      query = new GFXD3D9OcclusionQuery( this );
   else
      return NULL;      

   query->registerResourceWithDevice(this);
   return query;
}

GFXCubemap * GFXD3D9Device::createCubemap()
{
   GFXD3D9Cubemap* cube = new GFXD3D9Cubemap();
   cube->registerResourceWithDevice(this);
   return cube;
}
