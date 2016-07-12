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

#ifndef _GFXD3D9DEVICE_H_
#define _GFXD3D9DEVICE_H_

#include "platform/tmm_off.h"

#ifdef TORQUE_OS_XENON
#  include "platformXbox/platformXbox.h"
#else
#  include <d3dx9.h>
#  include "platformWin32/platformWin32.h"
#endif
#ifndef _GFXD3D9STATEBLOCK_H_
#include "gfx/D3D9/gfxD3D9StateBlock.h"
#endif
#ifndef _GFXD3DTEXTUREMANAGER_H_
#include "gfx/D3D9/gfxD3D9TextureManager.h"
#endif
#ifndef _GFXD3D9CUBEMAP_H_
#include "gfx/D3D9/gfxD3D9Cubemap.h"
#endif
#ifndef _GFXD3D9PRIMITIVEBUFFER_H_
#include "gfx/D3D9/gfxD3D9PrimitiveBuffer.h"
#endif
#ifndef _GFXINIT_H_
#include "gfx/gfxInit.h"
#endif
#ifndef _PLATFORMDLIBRARY_H
#include "platform/platformDlibrary.h"
#endif

#ifndef TORQUE_OS_XENON
#include <DxErr.h>
#else
#include <dxerr9.h>
#define DXGetErrorStringA DXGetErrorString9A
#define DXGetErrorDescriptionA DXGetErrorDescription9A
#endif

#include "platform/tmm_on.h"

inline void D3D9Assert( HRESULT hr, const char *info ) 
{
#if defined( TORQUE_DEBUG )
   if( FAILED( hr ) ) 
   {
      char buf[256];
      dSprintf( buf, 256, "%s\n%s\n%s", DXGetErrorStringA( hr ), DXGetErrorDescriptionA( hr ), info );
      AssertFatal( false, buf ); 
      //      DXTrace( __FILE__, __LINE__, hr, info, true );
   }
#endif
}


// Typedefs
#define D3DX_FUNCTION(fn_name, fn_return, fn_args) \
   typedef fn_return (WINAPI *D3DXFNPTR##fn_name)fn_args;
#include "gfx/D3D9/d3dx9Functions.h"
#undef D3DX_FUNCTION

// Function table
struct D3DXFNTable
{
   D3DXFNTable() : isLoaded( false ){};
   bool isLoaded;
   DLibraryRef dllRef;
   DLibraryRef compilerDllRef;
#define D3DX_FUNCTION(fn_name, fn_return, fn_args) \
   D3DXFNPTR##fn_name fn_name;
#include "gfx/D3D9/d3dx9Functions.h"
#undef D3DX_FUNCTION
};

#define GFXD3DX static_cast<GFXD3D9Device *>(GFX)->smD3DX 

class GFXResource;
class GFXD3D9ShaderConstBuffer;

//------------------------------------------------------------------------------

class GFXD3D9Device : public GFXDevice
{
   friend class GFXResource;
   friend class GFXD3D9PrimitiveBuffer;
   friend class GFXD3D9VertexBuffer;
   friend class GFXD3D9TextureObject;
   friend class GFXPCD3D9TextureTarget;
   friend class GFXPCD3D9WindowTarget;

   typedef GFXDevice Parent;

protected:

   MatrixF mTempMatrix;    ///< Temporary matrix, no assurances on value at all
   RectI mClipRect;

   typedef StrongRefPtr<GFXD3D9VertexBuffer> RPGDVB;
   Vector<RPGDVB> mVolatileVBList;

   class D3D9VertexDecl : public GFXVertexDecl
   {
   public:
      virtual ~D3D9VertexDecl()
      {
         SAFE_RELEASE( decl );
      }

      IDirect3DVertexDeclaration9 *decl;
   };

   /// Used to lookup a vertex declaration for the vertex format.
   /// @see allocVertexDecl
   typedef Map<String,D3D9VertexDecl*> VertexDeclMap;
   VertexDeclMap mVertexDecls;

   IDirect3DSurface9 *mDeviceBackbuffer;
   IDirect3DSurface9 *mDeviceDepthStencil;
   IDirect3DSurface9 *mDeviceColor;

   /// The stream 0 vertex buffer used for volatile VB offseting.
   GFXD3D9VertexBuffer *mVolatileVB;

   static void initD3DXFnTable();
   //-----------------------------------------------------------------------
   StrongRefPtr<GFXD3D9PrimitiveBuffer> mDynamicPB;                       ///< Dynamic index buffer
   GFXD3D9PrimitiveBuffer *mCurrentPB;

   IDirect3DVertexShader9 *mLastVertShader;
   IDirect3DPixelShader9 *mLastPixShader;

   GFXShaderRef mGenericShader[GS_COUNT];
   GFXShaderConstBufferRef mGenericShaderBuffer[GS_COUNT];
   GFXShaderConstHandle *mModelViewProjSC[GS_COUNT];

   S32 mCreateFenceType;

   LPDIRECT3D9       mD3D;        ///< D3D Handle
   LPDIRECT3DDEVICE9 mD3DDevice;  ///< Handle for D3DDevice

#if !defined(TORQUE_OS_XENON)
   LPDIRECT3D9EX       mD3DEx;             ///< D3D9Ex Handle
   LPDIRECT3DDEVICE9EX mD3DDeviceEx; ///< Handle for D3DDevice9Ex
#endif

   U32  mAdapterIndex;            ///< Adapter index because D3D supports multiple adapters

   F32 mPixVersion;
   U32 mNumSamplers;               ///< Profiled (via caps)
   U32 mNumRenderTargets;          ///< Profiled (via caps)

   D3DMULTISAMPLE_TYPE mMultisampleType;
   DWORD mMultisampleLevel;

   bool mOcclusionQuerySupported;

   /// The current adapter display mode.
   D3DDISPLAYMODE mDisplayMode;

   /// To manage creating and re-creating of these when device is aquired
   void reacquireDefaultPoolResources();

   /// To release all resources we control from D3DPOOL_DEFAULT
   void releaseDefaultPoolResources();

   /// This you will probably never, ever use, but it is used to generate the code for
   /// the initStates() function
   void regenStates();

   virtual GFXD3D9VertexBuffer* findVBPool( const GFXVertexFormat *vertexFormat, U32 numVertsNeeded );
   virtual GFXD3D9VertexBuffer* createVBPool( const GFXVertexFormat *vertexFormat, U32 vertSize );

#ifdef TORQUE_DEBUG
   /// @name Debug Vertex Buffer information/management
   /// @{

   ///
   U32 mNumAllocatedVertexBuffers; ///< To keep track of how many are allocated and freed
   GFXD3D9VertexBuffer *mVBListHead;
   void addVertexBuffer( GFXD3D9VertexBuffer *buffer );
   void removeVertexBuffer( GFXD3D9VertexBuffer *buffer );
   void logVertexBuffers();
   /// @}
#endif

   // State overrides
   // {

   ///
   virtual void setTextureInternal(U32 textureUnit, const GFXTextureObject* texture);

   /// Called by GFXDevice to create a device specific stateblock
   virtual GFXStateBlockRef createStateBlockInternal(const GFXStateBlockDesc& desc);
   /// Called by GFXDevice to actually set a stateblock.
   virtual void setStateBlockInternal(GFXStateBlock* block, bool force);

   /// Track the last const buffer we've used.  Used to notify new constant buffers that
   /// they should send all of their constants up
   StrongRefPtr<GFXD3D9ShaderConstBuffer> mCurrentConstBuffer;
   /// Called by base GFXDevice to actually set a const buffer
   virtual void setShaderConstBufferInternal(GFXShaderConstBuffer* buffer);

   // CodeReview - How exactly do we want to deal with this on the Xenon?
   // Right now it's just in an #ifndef in gfxD3D9Device.cpp - AlexS 4/11/07
   virtual void setLightInternal(U32 lightStage, const GFXLightInfo light, bool lightEnable);
   virtual void setLightMaterialInternal(const GFXLightMaterial mat);
   virtual void setGlobalAmbientInternal(ColorF color);

   virtual void initStates()=0;
   // }

   // Index buffer management
   // {
   virtual void _setPrimitiveBuffer( GFXPrimitiveBuffer *buffer );
   virtual void drawIndexedPrimitive(  GFXPrimitiveType primType, 
                                       U32 startVertex, 
                                       U32 minIndex, 
                                       U32 numVerts, 
                                       U32 startIndex, 
                                       U32 primitiveCount );
   // }

   virtual GFXShader* createShader();
   void disableShaders(bool force = false);

   /// Device helper function
   virtual D3DPRESENT_PARAMETERS setupPresentParams( const GFXVideoMode &mode, const HWND &hwnd ) const = 0;
   
public:
   static D3DXFNTable smD3DX;

   static GFXDevice *createInstance( U32 adapterIndex );

   GFXTextureObject* createRenderSurface( U32 width, U32 height, GFXFormat format, U32 mipLevel );

   const D3DDISPLAYMODE& getDisplayMode() const { return mDisplayMode; }

   /// Constructor
   /// @param   d3d   Direct3D object to instantiate this device with
   /// @param   index   Adapter index since D3D can use multiple graphics adapters
   GFXD3D9Device( LPDIRECT3D9 d3d, U32 index );
   virtual ~GFXD3D9Device();

   // Activate/deactivate
   // {
   virtual void init( const GFXVideoMode &mode, PlatformWindow *window = NULL ) = 0;

   virtual void preDestroy() { Parent::preDestroy(); if(mTextureManager) mTextureManager->kill(); }

   GFXAdapterType getAdapterType(){ return Direct3D9; }

   U32 getAdaterIndex() const { return mAdapterIndex; }

   virtual GFXCubemap *createCubemap();

   virtual F32  getPixelShaderVersion() const { return mPixVersion; }
   virtual void setPixelShaderVersion( F32 version ){ mPixVersion = version; }
   virtual void setShader( GFXShader *shader, bool force = false );
   virtual U32  getNumSamplers() const { return mNumSamplers; }
   virtual U32  getNumRenderTargets() const { return mNumRenderTargets; }
   // }

   // Misc rendering control
   // {
   virtual void clear( U32 flags, ColorI color, F32 z, U32 stencil );
   virtual bool beginSceneInternal();
   virtual void endSceneInternal();

   virtual void setClipRect( const RectI &rect );
   virtual const RectI& getClipRect() const { return mClipRect; }

   // }

   /// @name Render Targets
   /// @{
   virtual void _updateRenderTargets();
   /// @}

   // Vertex/Index buffer management
   // {
   virtual GFXVertexBuffer* allocVertexBuffer(  U32 numVerts, 
                                                const GFXVertexFormat *vertexFormat,
                                                U32 vertSize,
                                                GFXBufferType bufferType,
                                                void* data = NULL );
   virtual GFXPrimitiveBuffer *allocPrimitiveBuffer(  U32 numIndices, 
                                                      U32 numPrimitives, 
                                                      GFXBufferType bufferType,
                                                      void* data = NULL );
   virtual void deallocVertexBuffer( GFXD3D9VertexBuffer *vertBuff );
   virtual GFXVertexDecl* allocVertexDecl( const GFXVertexFormat *vertexFormat );
   virtual void setVertexDecl( const GFXVertexDecl *decl );

   virtual void setVertexStream( U32 stream, GFXVertexBuffer *buffer );
   virtual void setVertexStreamFrequency( U32 stream, U32 frequency );
   // }

   virtual U32 getMaxDynamicVerts() { return MAX_DYNAMIC_VERTS; }
   virtual U32 getMaxDynamicIndices() { return MAX_DYNAMIC_INDICES; }

   // Rendering
   // {
   virtual void drawPrimitive( GFXPrimitiveType primType, U32 vertexStart, U32 primitiveCount );
   // }

   virtual LPDIRECT3DDEVICE9 getDevice(){ return mD3DDevice; }
   virtual LPDIRECT3D9 getD3D() { return mD3D; }

   /// Reset
   virtual void reset( D3DPRESENT_PARAMETERS &d3dpp ) = 0;

   virtual void setupGenericShaders( GenericShaderType type  = GSColor );

   // Function only really used on the, however a centralized function for
   // destroying resources is probably a good thing -patw
   virtual void destroyD3DResource( IDirect3DResource9 *d3dResource ) { SAFE_RELEASE( d3dResource ); }; 

   inline virtual F32 getFillConventionOffset() const { return 0.5f; }
   virtual void doParanoidStateCheck();

   GFXFence *createFence();

   GFXOcclusionQuery* createOcclusionQuery();   

   // Default multisample parameters
   D3DMULTISAMPLE_TYPE getMultisampleType() const { return mMultisampleType; }
   DWORD getMultisampleLevel() const { return mMultisampleLevel; } 

   // Whether or not the Direct3D device was created with Direct3D9Ex support
#if !defined(TORQUE_OS_XENON)
   virtual bool isD3D9Ex() { return mD3DEx != NULL; }
#else
   virtual bool isD3D9Ex() { return false; }
#endif

   // Get the backbuffer, currently only access for WPF support
   virtual IDirect3DSurface9* getBackBuffer() { return mDeviceBackbuffer; }

};


#endif // _GFXD3D9DEVICE_H_
