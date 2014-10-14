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

#include "gfx/D3D9/pc/gfxPCD3D9Device.h"
#include "gfx/D3D9/pc/gfxPCD3D9Target.h"
#include "gfx/D3D9/gfxD3D9CardProfiler.h"
#include "gfx/D3D9/gfxD3D9EnumTranslate.h"
#include "platformWin32/platformWin32.h"
#include "windowManager/win32/win32Window.h"
#include "gfx/screenshot.h"
#include "gfx/D3D9/screenshotD3D9.h"
#include "gfx/D3D9/videoCaptureD3D9.h"
#include "core/util/journal/process.h"


bool GFXPCD3D9Device::mEnableNVPerfHUD = false;

GFXAdapter::CreateDeviceInstanceDelegate GFXPCD3D9Device::mCreateDeviceInstance(GFXPCD3D9Device::createInstance);

GFXPCD3D9Device::~GFXPCD3D9Device()
{
   if( mVideoFrameGrabber )
   {
      if( ManagedSingleton< VideoCapture >::instanceOrNull() )
         VIDCAP->setFrameGrabber( NULL );
      delete mVideoFrameGrabber;
   }
}

void GFXPCD3D9Device::createDirect3D9(LPDIRECT3D9 &d3d9, LPDIRECT3D9EX &d3d9ex)
{
   d3d9 = NULL;
   d3d9ex = NULL;

   if ( !Con::getBoolVariable( "$pref::Video::preferDirect3D9Ex", false ) )
   {
      d3d9 = Direct3DCreate9(D3D_SDK_VERSION);
      return;
   }

   HMODULE hD3D = LoadLibrary(TEXT("d3d9.dll"));

   if (hD3D)
   {

      HRESULT (WINAPI *pfnCreate9Ex)(UINT SDKVersion, IDirect3D9Ex**) = (HRESULT ( WINAPI *)(UINT SDKVersion, IDirect3D9Ex**)) GetProcAddress(hD3D, "Direct3DCreate9Ex");
      
      if (pfnCreate9Ex)
      {
         if (!FAILED(pfnCreate9Ex(D3D_SDK_VERSION, &d3d9ex)) && d3d9ex)
            d3d9ex->QueryInterface(__uuidof(IDirect3D9), reinterpret_cast<void **>(&d3d9));
      }

      if (!pfnCreate9Ex || !d3d9)
      {
         if (d3d9ex)
         {
            SAFE_RELEASE(d3d9ex)
            d3d9ex = NULL;
         }

         d3d9 = Direct3DCreate9(D3D_SDK_VERSION);
      }

      FreeLibrary(hD3D);
   }
}

GFXDevice *GFXPCD3D9Device::createInstance( U32 adapterIndex )
{
   LPDIRECT3D9 d3d9;
   LPDIRECT3D9EX d3d9ex;
   
   createDirect3D9(d3d9, d3d9ex);
   
   GFXPCD3D9Device* dev = new GFXPCD3D9Device( d3d9, adapterIndex );
   dev->mD3DEx = d3d9ex;
   return dev;
}

//-----------------------------------------------------------------------------

GFXFormat GFXPCD3D9Device::selectSupportedFormat(GFXTextureProfile *profile,
		const Vector<GFXFormat> &formats, bool texture, bool mustblend, bool mustfilter)
{
	DWORD usage = 0;

	if(profile->isDynamic())
		usage |= D3DUSAGE_DYNAMIC;

	if(profile->isRenderTarget())
		usage |= D3DUSAGE_RENDERTARGET;

	if(profile->isZTarget())
		usage |= D3DUSAGE_DEPTHSTENCIL;

	if(mustblend)
		usage |= D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING;

   if(mustfilter)
		usage |= D3DUSAGE_QUERY_FILTER;

	D3DDISPLAYMODE mode;
	D3D9Assert(mD3D->GetAdapterDisplayMode(mAdapterIndex, &mode), "Unable to get adapter mode.");

	D3DRESOURCETYPE type;
	if(texture)
		type = D3DRTYPE_TEXTURE;
	else
		type = D3DRTYPE_SURFACE;

	for(U32 i=0; i<formats.size(); i++)
	{
		if(mD3D->CheckDeviceFormat(mAdapterIndex, D3DDEVTYPE_HAL, mode.Format,
			usage, type, GFXD3D9TextureFormat[formats[i]]) == D3D_OK)
			return formats[i];
	}

	return GFXFormatR8G8B8A8;
}

HRESULT GFXPCD3D9Device::createDevice(U32 adapter, D3DDEVTYPE deviceType, HWND hFocusWindow, DWORD behaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters)
{
   HRESULT hres = E_FAIL;

   if (mD3DEx)
   {
      hres = mD3DEx->CreateDeviceEx( adapter, deviceType, 
         hFocusWindow,
         behaviorFlags, 
         pPresentationParameters, NULL, &mD3DDeviceEx ); 

      if (!FAILED(hres) && mD3DDeviceEx)
         hres = mD3DDeviceEx->QueryInterface(__uuidof(IDirect3DDevice9), reinterpret_cast<void**>(&mD3DDevice));  
   }
   else
   {
      hres = mD3D->CreateDevice( adapter, deviceType, 
         hFocusWindow,
         behaviorFlags, 
         pPresentationParameters, &mD3DDevice ); 
   }   

   return hres;

}

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Setup D3D present parameters - init helper function
//-----------------------------------------------------------------------------
D3DPRESENT_PARAMETERS GFXPCD3D9Device::setupPresentParams( const GFXVideoMode &mode, const HWND &hwnd ) const
{
   // Create D3D Presentation params
   D3DPRESENT_PARAMETERS d3dpp; 
   dMemset( &d3dpp, 0, sizeof( d3dpp ) );

   D3DFORMAT fmt = D3DFMT_X8R8G8B8; // 32 bit

   if( mode.bitDepth == 16 )
      fmt = D3DFMT_R5G6B5;

   D3DMULTISAMPLE_TYPE aatype;
   DWORD aalevel;

   // Setup the AA flags...  If we've been ask to 
   // disable  hardware AA then do that now.
   if ( mode.antialiasLevel == 0 || Con::getBoolVariable( "$pref::Video::disableHardwareAA", false ) )
   {
      aatype = D3DMULTISAMPLE_NONE;
      aalevel = 0;
   } 
   else 
   {
      aatype = D3DMULTISAMPLE_NONMASKABLE;
      aalevel = mode.antialiasLevel-1;
   }
  
   _validateMultisampleParams(fmt, aatype, aalevel);
   
   d3dpp.BackBufferWidth  = mode.resolution.x;
   d3dpp.BackBufferHeight = mode.resolution.y;
   d3dpp.BackBufferFormat = fmt;
   d3dpp.BackBufferCount  = 1;
   d3dpp.MultiSampleType  = aatype;
   d3dpp.MultiSampleQuality = aalevel;
   d3dpp.SwapEffect       = D3DSWAPEFFECT_DISCARD;
   d3dpp.hDeviceWindow    = hwnd;
   d3dpp.Windowed         = !mode.fullScreen;
   d3dpp.EnableAutoDepthStencil = TRUE;
   d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
   d3dpp.Flags            = 0;
   d3dpp.FullScreen_RefreshRateInHz = (mode.refreshRate == 0 || !mode.fullScreen) ? 
                                       D3DPRESENT_RATE_DEFAULT : mode.refreshRate;
   d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

   if ( smDisableVSync )
      d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;	// This does NOT wait for vsync

   return d3dpp;
}

//-----------------------------------------------------------------------------
// Enumerate D3D adapters
//-----------------------------------------------------------------------------
void GFXPCD3D9Device::enumerateAdapters( Vector<GFXAdapter*> &adapterList )
{
   // Grab a D3D9 handle here to first get the D3D9 devices
   LPDIRECT3D9 d3d9;
   LPDIRECT3D9EX d3d9ex;
   createDirect3D9( d3d9, d3d9ex); 

   // If we could not create the d3d9 object then either the system
   // is corrupt or they need to update the directx runtime.
   if ( !d3d9 )
   {
      Con::errorf( "Unsupported DirectX version!" );
      Platform::messageBox(   Con::getVariable( "$appName" ),
                              "DirectX could not be started!\r\n"
                              "Please be sure you have the latest version of DirectX installed.",
                              MBOk, MIStop );
      Platform::forceShutdown( -1 );
   }

   for( U32 adapterIndex = 0; adapterIndex < d3d9->GetAdapterCount(); adapterIndex++ ) 
   {
      GFXAdapter *toAdd = new GFXAdapter;
      toAdd->mType  = Direct3D9;
      toAdd->mIndex = adapterIndex;
      toAdd->mCreateDeviceInstanceDelegate = mCreateDeviceInstance;

      // Grab the shader model.
      D3DCAPS9 caps;
      d3d9->GetDeviceCaps(adapterIndex, D3DDEVTYPE_HAL, &caps);
      U8 *pxPtr = (U8*) &caps.PixelShaderVersion;
      toAdd->mShaderModel = pxPtr[1] + pxPtr[0] * 0.1;

      // Get the device description string.
      D3DADAPTER_IDENTIFIER9 temp;
      d3d9->GetAdapterIdentifier( adapterIndex, NULL, &temp ); // The NULL is the flags which deal with WHQL

      dStrncpy(toAdd->mName, temp.Description, GFXAdapter::MaxAdapterNameLen);
      dStrncat(toAdd->mName, " (D3D9)", GFXAdapter::MaxAdapterNameLen);

      // And the output display device name
      dStrncpy(toAdd->mOutputName, temp.DeviceName, GFXAdapter::MaxAdapterNameLen);

      // Video mode enumeration.
      Vector<D3DFORMAT> formats( __FILE__, __LINE__ );
      formats.push_back( D3DFMT_R5G6B5 );    // D3DFMT_R5G6B5 - 16bit format
      formats.push_back( D3DFMT_X8R8G8B8 );  // D3DFMT_X8R8G8B8 - 32bit format

      for( S32 i = 0; i < formats.size(); i++ ) 
      {
         DWORD MaxSampleQualities;
         d3d9->CheckDeviceMultiSampleType(adapterIndex, D3DDEVTYPE_HAL, formats[i], FALSE, D3DMULTISAMPLE_NONMASKABLE, &MaxSampleQualities);

         for( U32 j = 0; j < d3d9->GetAdapterModeCount( adapterIndex, formats[i] ); j++ ) 
         {
            D3DDISPLAYMODE mode;
            d3d9->EnumAdapterModes( adapterIndex, formats[i], j, &mode );

            GFXVideoMode vmAdd;

            vmAdd.bitDepth    = ( i == 0 ? 16 : 32 ); // This will need to be changed later
            vmAdd.fullScreen  = true;
            vmAdd.refreshRate = mode.RefreshRate;
            vmAdd.resolution  = Point2I( mode.Width, mode.Height );
            vmAdd.antialiasLevel = MaxSampleQualities;

            toAdd->mAvailableModes.push_back( vmAdd );
         }
      }

      adapterList.push_back( toAdd );
   }

   d3d9->Release();
}

void GFXPCD3D9Device::enumerateVideoModes() 
{
   Vector<D3DFORMAT> formats( __FILE__, __LINE__ );
   formats.push_back( D3DFMT_R5G6B5 );    // D3DFMT_R5G6B5 - 16bit format
   formats.push_back( D3DFMT_X8R8G8B8 );  // D3DFMT_X8R8G8B8 - 32bit format

   for( S32 i = 0; i < formats.size(); i++ ) 
   {
      for( U32 j = 0; j < mD3D->GetAdapterModeCount( mAdapterIndex, formats[i] ); j++ ) 
      {
         D3DDISPLAYMODE mode;
         mD3D->EnumAdapterModes( mAdapterIndex, formats[i], j, &mode );

         GFXVideoMode toAdd;

         toAdd.bitDepth = ( i == 0 ? 16 : 32 ); // This will need to be changed later
         toAdd.fullScreen = false;
         toAdd.refreshRate = mode.RefreshRate;
         toAdd.resolution = Point2I( mode.Width, mode.Height );

         mVideoModes.push_back( toAdd );
      }
   }
}

//-----------------------------------------------------------------------------
// Initialize - create window, device, etc
//-----------------------------------------------------------------------------
void GFXPCD3D9Device::init( const GFXVideoMode &mode, PlatformWindow *window /* = NULL */ )
{
   AssertFatal(window, "GFXPCD3D9Device::init - must specify a window!");

   initD3DXFnTable();

   Win32Window *win = dynamic_cast<Win32Window*>( window );
   AssertISV( win, "GFXD3D9Device::init - got a non Win32Window window passed in! Did DX go crossplatform?" );

   HWND winHwnd = win->getHWND();

   // Create D3D Presentation params
   D3DPRESENT_PARAMETERS d3dpp = setupPresentParams( mode, winHwnd );
   mMultisampleType = d3dpp.MultiSampleType;
   mMultisampleLevel = d3dpp.MultiSampleQuality;
      
#ifndef TORQUE_SHIPPING
   bool usePerfHud = GFXPCD3D9Device::mEnableNVPerfHUD || Con::getBoolVariable("$Video::useNVPerfHud", false);   
#else
   bool usePerfHud = false;
#endif

   HRESULT hres = E_FAIL;
   if ( usePerfHud )
   {  
      hres = createDevice(  mD3D->GetAdapterCount() - 1, D3DDEVTYPE_REF, winHwnd, D3DCREATE_MIXED_VERTEXPROCESSING, &d3dpp);
   }
   else 
   {
      // Vertex processing was changed from MIXED to HARDWARE because of the switch to a pure D3D device.

      // Set up device flags from our compile flags.
      U32 deviceFlags = 0;
      deviceFlags = D3DCREATE_HARDWARE_VERTEXPROCESSING;

      // Currently, offscreen rendering is only used by WPF apps and we need to create with D3DCREATE_MULTITHREAD for it
      // In all other cases, you can do better by locking/creating resources in the primary thread
      // and passing them to worker threads.
      if (window->getOffscreenRender())
      {
         deviceFlags |= D3DCREATE_MULTITHREADED;
         d3dpp.Windowed = TRUE;
         d3dpp.BackBufferHeight = 1;
         d3dpp.BackBufferWidth = 1;
         d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
         d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;
         d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
      }

      // DirectX will switch the floating poing control word to single precision
      // and disable exceptions by default.  There are a few issues with this...
      //
      // 1. It can cause rendering problems when running in WPF.
      // 2. Firefox embedding issues.
      // 3. Physics engines depend on the higher precision.
      //
      // Interestingly enough... DirectX 10 and 11 do not modifiy the floating point
      // settings and are always in full precision.
      //
      // The down side is we supposedly loose some performance, but so far i've not
      // seen a measurable impact.
      // 
      deviceFlags |= D3DCREATE_FPU_PRESERVE;

      // Try to do pure, unless we're doing debug (and thus doing more paranoid checking).
#ifndef TORQUE_DEBUG_RENDER
      deviceFlags |= D3DCREATE_PUREDEVICE;
#endif

      hres = createDevice( mAdapterIndex, D3DDEVTYPE_HAL, winHwnd, deviceFlags, &d3dpp);

      if (FAILED(hres) && hres != D3DERR_OUTOFVIDEOMEMORY)
      {
         Con::errorf("   Failed to create hardware device, trying mixed device");
         // turn off pure
         deviceFlags &= (~D3DCREATE_PUREDEVICE);

         // try mixed mode
         deviceFlags &= (~D3DCREATE_HARDWARE_VERTEXPROCESSING);
         deviceFlags |= D3DCREATE_MIXED_VERTEXPROCESSING;
         hres = createDevice( mAdapterIndex, D3DDEVTYPE_HAL, 
            winHwnd, deviceFlags, 
            &d3dpp);

         // try software 
         if (FAILED(hres) && hres != D3DERR_OUTOFVIDEOMEMORY)
         {
            Con::errorf("   Failed to create mixed mode device, trying software device");
            deviceFlags &= (~D3DCREATE_MIXED_VERTEXPROCESSING);
            deviceFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
            hres = createDevice( mAdapterIndex, D3DDEVTYPE_HAL, 
               winHwnd, deviceFlags,
               &d3dpp);

            if (FAILED(hres) && hres != D3DERR_OUTOFVIDEOMEMORY)
               Con::errorf("   Failed to create software device, giving up");
            D3D9Assert(hres, "GFXPCD3D9Device::init - CreateDevice failed!");
         }
      }
   }

   // Gracefully die if they can't give us a device.
   if(!mD3DDevice)
   {
      if (hres == D3DERR_OUTOFVIDEOMEMORY)
      {
         char errorMsg[4096];
         dSprintf(errorMsg, sizeof(errorMsg),
            "Out of video memory. Close other windows, reboot, and/or upgrade your video card drivers. Your video card is: %s", getAdapter().getName());
         Platform::AlertOK("DirectX Error", errorMsg);
      }
      else
      {
         Platform::AlertOK("DirectX Error!", "Failed to initialize Direct3D! Make sure you have DirectX 9 installed, and "
            "are running a graphics card that supports Pixel Shader 1.1.");
      }
      Platform::forceShutdown(1);
   }

   // Check up on things
   Con::printf("   Cur. D3DDevice ref count=%d", mD3DDevice->AddRef() - 1);
   mD3DDevice->Release();
   
   mTextureManager = new GFXD3D9TextureManager( mD3DDevice, mAdapterIndex );

   // Now reacquire all the resources we trashed earlier
   reacquireDefaultPoolResources();
      
   // Setup default states
   initStates();

   //-------- Output init info ---------   
   D3DCAPS9 caps;
   mD3DDevice->GetDeviceCaps( &caps );

   U8 *pxPtr = (U8*) &caps.PixelShaderVersion;
   mPixVersion = pxPtr[1] + pxPtr[0] * 0.1;
   if (mPixVersion >= 2.0f && mPixVersion < 3.0f && caps.PS20Caps.NumTemps >= 32)
      mPixVersion += 0.2f;
   else if (mPixVersion >= 2.0f && mPixVersion < 3.0f && caps.PS20Caps.NumTemps >= 22)
      mPixVersion += 0.1f;
   Con::printf( "   Pix version detected: %f", mPixVersion );

   if ( smForcedPixVersion >= 0.0f && smForcedPixVersion < mPixVersion )
   {
      mPixVersion = smForcedPixVersion;
      Con::errorf( "   Forced pix version: %f", mPixVersion );
   }

   U8 *vertPtr = (U8*) &caps.VertexShaderVersion;
   F32 vertVersion = vertPtr[1] + vertPtr[0] * 0.1;
   Con::printf( "   Vert version detected: %f", vertVersion );

   // The sampler count is based on the shader model and
   // not found in the caps.
   //
   // MaxSimultaneousTextures is only valid for fixed
   // function rendering.
   //
   if ( mPixVersion >= 2.0f )
      mNumSamplers = 16;
   else if ( mPixVersion >= 1.4f )
      mNumSamplers = 6;
   else if ( mPixVersion > 0.0f )
      mNumSamplers = 4;
   else
      mNumSamplers = caps.MaxSimultaneousTextures;      

   // This shouldn't happen until SM5 or some other
   // radical change in GPU hardware occurs.
   AssertFatal( mNumSamplers <= TEXTURE_STAGE_COUNT, 
      "GFXPCD3D9Device::init - Sampler count greater than TEXTURE_STAGE_COUNT!" );
            
   Con::printf( "   Maximum number of simultaneous samplers: %d", mNumSamplers );

   // detect max number of simultaneous render targets
   mNumRenderTargets = caps.NumSimultaneousRTs;
   Con::printf( "   Number of simultaneous render targets: %d", mNumRenderTargets );
   
   // detect occlusion query support
   if (SUCCEEDED(mD3DDevice->CreateQuery( D3DQUERYTYPE_OCCLUSION, NULL )))
	   mOcclusionQuerySupported = true;
      
   Con::printf( "   Hardware occlusion query detected: %s", mOcclusionQuerySupported ? "Yes" : "No" );      

   Con::printf( "   Using Direct3D9Ex: %s", isD3D9Ex() ? "Yes" : "No" );
   
   mCardProfiler = new GFXD3D9CardProfiler(mAdapterIndex);
   mCardProfiler->init();

   gScreenShot = new ScreenShotD3D;

   // Set the video capture frame grabber.
   mVideoFrameGrabber = new VideoFrameGrabberD3D9();
   VIDCAP->setFrameGrabber( mVideoFrameGrabber );

   // Grab the depth-stencil...
   SAFE_RELEASE(mDeviceDepthStencil);
   D3D9Assert(mD3DDevice->GetDepthStencilSurface(&mDeviceDepthStencil), "GFXD3D9Device::init - couldn't grab reference to device's depth-stencil surface.");  

   mInitialized = true;

   deviceInited();

   // Uncomment to dump out code needed in initStates, you may also need to enable the reference device (get rid of code in initStates first as well)
   // regenStates();
}

//------------------------------------------------------------------------------
void GFXPCD3D9Device::enterDebugEvent(ColorI color, const char *name)
{
   // BJGFIX
   WCHAR  eventName[260];
   MultiByteToWideChar( CP_ACP, 0, name, -1, eventName, 260 );

   D3DPERF_BeginEvent(D3DCOLOR_ARGB(color.alpha, color.red, color.green, color.blue),
      (LPCWSTR)&eventName);
}

//------------------------------------------------------------------------------
void GFXPCD3D9Device::leaveDebugEvent()
{
   D3DPERF_EndEvent();
}

//------------------------------------------------------------------------------
void GFXPCD3D9Device::setDebugMarker(ColorI color, const char *name)
{
   // BJGFIX
   WCHAR  eventName[260];
   MultiByteToWideChar( CP_ACP, 0, name, -1, eventName, 260 );

   D3DPERF_SetMarker(D3DCOLOR_ARGB(color.alpha, color.red, color.green, color.blue), 
      (LPCWSTR)&eventName);
}

//-----------------------------------------------------------------------------

void GFXPCD3D9Device::setMatrix( GFXMatrixType mtype, const MatrixF &mat ) 
{
   mat.transposeTo( mTempMatrix );

   mD3DDevice->SetTransform( (_D3DTRANSFORMSTATETYPE)mtype, (D3DMATRIX *)&mTempMatrix );
}

//-----------------------------------------------------------------------------

void GFXPCD3D9Device::_setTextureStageState( U32 stage, U32 state, U32 value ) 
{
   switch( state )
   {
      case GFXTSSColorOp:
      case GFXTSSAlphaOp:
         mD3DDevice->SetTextureStageState( stage, GFXD3D9TextureStageState[state], GFXD3D9TextureOp[value] );
         break;

      default:
         mD3DDevice->SetTextureStageState( stage, GFXD3D9TextureStageState[state], value );
         break;
   }
}

//------------------------------------------------------------------------------

void GFXPCD3D9Device::initStates() 
{
   //-------------------------------------
   // Auto-generated default states, see regenStates() for details
   //

   // Render states
   mD3DDevice->SetRenderState( GFXD3D9RenderState[0], 1 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[1], 3 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[2], 2 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[3], 1 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[4], 0 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[5], 1 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[6], 2 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[7], 1 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[8], 3 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[9], 4 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[10], 0 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[11], 8 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[12], 0 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[13], 0 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[14], 0 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[15], 0 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[16], 0 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[17], 0 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[18], 0 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[19], 1065353216 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[20], 1065353216 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[21], 0 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[22], 0 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[23], 1 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[24], 1 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[25], 1 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[26], 8 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[27], 0 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[28], -1 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[29], -1 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[30], -1 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[31], 0 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[32], 0 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[33], 0 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[34], 0 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[35], 0 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[36], 0 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[37], 0 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[38], 0 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[39], 1 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[40], 1 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[41], 0 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[42], 0 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[43], 1 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[44], 1 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[45], 0 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[46], 1 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[47], 2 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[48], 0 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[49], 0 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[50], 0 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[51], 0 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[52], 1065353216 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[53], 1065353216 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[54], 0 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[55], 0 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[56], 1065353216 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[57], 0 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[58], 0 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[59], 1 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[60], -1 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[61], 0 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[62], 0 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[63], 1115684864 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[64], 0 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[65], 15 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[66], 0 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[67], 1 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[68], 3 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[69], 1 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[70], 0 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[71], 0 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[72], 0 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[73], 1065353216 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[74], 1065353216 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[75], 0 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[76], 0 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[77], 1065353216 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[78], 0 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[79], 0 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[80], 0 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[81], 1 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[82], 1 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[83], 1 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[84], 8 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[85], 15 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[86], 15 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[87], 15 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[88], -1 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[89], 0 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[90], 0 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[91], 0 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[92], 0 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[93], 0 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[94], 0 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[95], 0 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[96], 0 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[97], 0 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[98], 0 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[99], 0 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[100], 2 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[101], 1 );
   mD3DDevice->SetRenderState( GFXD3D9RenderState[102], 1 );

   // Texture Stage states
   mD3DDevice->SetTextureStageState( 0, GFXD3D9TextureStageState[0], 4 );
   mD3DDevice->SetTextureStageState( 0, GFXD3D9TextureStageState[1], 2 );
   mD3DDevice->SetTextureStageState( 0, GFXD3D9TextureStageState[2], 1 );
   mD3DDevice->SetTextureStageState( 0, GFXD3D9TextureStageState[3], 2 );
   mD3DDevice->SetTextureStageState( 0, GFXD3D9TextureStageState[4], 2 );
   mD3DDevice->SetTextureStageState( 0, GFXD3D9TextureStageState[5], 1 );
   mD3DDevice->SetTextureStageState( 0, GFXD3D9TextureStageState[6], 0 );
   mD3DDevice->SetTextureStageState( 0, GFXD3D9TextureStageState[7], 0 );
   mD3DDevice->SetTextureStageState( 0, GFXD3D9TextureStageState[8], 0 );
   mD3DDevice->SetTextureStageState( 0, GFXD3D9TextureStageState[9], 0 );
   mD3DDevice->SetTextureStageState( 0, GFXD3D9TextureStageState[10], 0 );
   mD3DDevice->SetTextureStageState( 0, GFXD3D9TextureStageState[11], 0 );
   mD3DDevice->SetTextureStageState( 0, GFXD3D9TextureStageState[12], 0 );
   mD3DDevice->SetTextureStageState( 0, GFXD3D9TextureStageState[13], 0 );
   mD3DDevice->SetTextureStageState( 0, GFXD3D9TextureStageState[14], 1 );
   mD3DDevice->SetTextureStageState( 0, GFXD3D9TextureStageState[15], 1 );
   mD3DDevice->SetTextureStageState( 0, GFXD3D9TextureStageState[16], 1 );
   mD3DDevice->SetTextureStageState( 0, GFXD3D9TextureStageState[17], 0 );
   mD3DDevice->SetTextureStageState( 1, GFXD3D9TextureStageState[0], 1 );
   mD3DDevice->SetTextureStageState( 1, GFXD3D9TextureStageState[1], 2 );
   mD3DDevice->SetTextureStageState( 1, GFXD3D9TextureStageState[2], 1 );
   mD3DDevice->SetTextureStageState( 1, GFXD3D9TextureStageState[3], 1 );
   mD3DDevice->SetTextureStageState( 1, GFXD3D9TextureStageState[4], 2 );
   mD3DDevice->SetTextureStageState( 1, GFXD3D9TextureStageState[5], 1 );
   mD3DDevice->SetTextureStageState( 1, GFXD3D9TextureStageState[6], 0 );
   mD3DDevice->SetTextureStageState( 1, GFXD3D9TextureStageState[7], 0 );
   mD3DDevice->SetTextureStageState( 1, GFXD3D9TextureStageState[8], 0 );
   mD3DDevice->SetTextureStageState( 1, GFXD3D9TextureStageState[9], 0 );
   mD3DDevice->SetTextureStageState( 1, GFXD3D9TextureStageState[10], 1 );
   mD3DDevice->SetTextureStageState( 1, GFXD3D9TextureStageState[11], 0 );
   mD3DDevice->SetTextureStageState( 1, GFXD3D9TextureStageState[12], 0 );
   mD3DDevice->SetTextureStageState( 1, GFXD3D9TextureStageState[13], 0 );
   mD3DDevice->SetTextureStageState( 1, GFXD3D9TextureStageState[14], 1 );
   mD3DDevice->SetTextureStageState( 1, GFXD3D9TextureStageState[15], 1 );
   mD3DDevice->SetTextureStageState( 1, GFXD3D9TextureStageState[16], 1 );
   mD3DDevice->SetTextureStageState( 1, GFXD3D9TextureStageState[17], 0 );
   mD3DDevice->SetTextureStageState( 2, GFXD3D9TextureStageState[0], 1 );
   mD3DDevice->SetTextureStageState( 2, GFXD3D9TextureStageState[1], 2 );
   mD3DDevice->SetTextureStageState( 2, GFXD3D9TextureStageState[2], 1 );
   mD3DDevice->SetTextureStageState( 2, GFXD3D9TextureStageState[3], 1 );
   mD3DDevice->SetTextureStageState( 2, GFXD3D9TextureStageState[4], 2 );
   mD3DDevice->SetTextureStageState( 2, GFXD3D9TextureStageState[5], 1 );
   mD3DDevice->SetTextureStageState( 2, GFXD3D9TextureStageState[6], 0 );
   mD3DDevice->SetTextureStageState( 2, GFXD3D9TextureStageState[7], 0 );
   mD3DDevice->SetTextureStageState( 2, GFXD3D9TextureStageState[8], 0 );
   mD3DDevice->SetTextureStageState( 2, GFXD3D9TextureStageState[9], 0 );
   mD3DDevice->SetTextureStageState( 2, GFXD3D9TextureStageState[10], 2 );
   mD3DDevice->SetTextureStageState( 2, GFXD3D9TextureStageState[11], 0 );
   mD3DDevice->SetTextureStageState( 2, GFXD3D9TextureStageState[12], 0 );
   mD3DDevice->SetTextureStageState( 2, GFXD3D9TextureStageState[13], 0 );
   mD3DDevice->SetTextureStageState( 2, GFXD3D9TextureStageState[14], 1 );
   mD3DDevice->SetTextureStageState( 2, GFXD3D9TextureStageState[15], 1 );
   mD3DDevice->SetTextureStageState( 2, GFXD3D9TextureStageState[16], 1 );
   mD3DDevice->SetTextureStageState( 2, GFXD3D9TextureStageState[17], 0 );
   mD3DDevice->SetTextureStageState( 3, GFXD3D9TextureStageState[0], 1 );
   mD3DDevice->SetTextureStageState( 3, GFXD3D9TextureStageState[1], 2 );
   mD3DDevice->SetTextureStageState( 3, GFXD3D9TextureStageState[2], 1 );
   mD3DDevice->SetTextureStageState( 3, GFXD3D9TextureStageState[3], 1 );
   mD3DDevice->SetTextureStageState( 3, GFXD3D9TextureStageState[4], 2 );
   mD3DDevice->SetTextureStageState( 3, GFXD3D9TextureStageState[5], 1 );
   mD3DDevice->SetTextureStageState( 3, GFXD3D9TextureStageState[6], 0 );
   mD3DDevice->SetTextureStageState( 3, GFXD3D9TextureStageState[7], 0 );
   mD3DDevice->SetTextureStageState( 3, GFXD3D9TextureStageState[8], 0 );
   mD3DDevice->SetTextureStageState( 3, GFXD3D9TextureStageState[9], 0 );
   mD3DDevice->SetTextureStageState( 3, GFXD3D9TextureStageState[10], 3 );
   mD3DDevice->SetTextureStageState( 3, GFXD3D9TextureStageState[11], 0 );
   mD3DDevice->SetTextureStageState( 3, GFXD3D9TextureStageState[12], 0 );
   mD3DDevice->SetTextureStageState( 3, GFXD3D9TextureStageState[13], 0 );
   mD3DDevice->SetTextureStageState( 3, GFXD3D9TextureStageState[14], 1 );
   mD3DDevice->SetTextureStageState( 3, GFXD3D9TextureStageState[15], 1 );
   mD3DDevice->SetTextureStageState( 3, GFXD3D9TextureStageState[16], 1 );
   mD3DDevice->SetTextureStageState( 3, GFXD3D9TextureStageState[17], 0 );
   mD3DDevice->SetTextureStageState( 4, GFXD3D9TextureStageState[0], 1 );
   mD3DDevice->SetTextureStageState( 4, GFXD3D9TextureStageState[1], 2 );
   mD3DDevice->SetTextureStageState( 4, GFXD3D9TextureStageState[2], 1 );
   mD3DDevice->SetTextureStageState( 4, GFXD3D9TextureStageState[3], 1 );
   mD3DDevice->SetTextureStageState( 4, GFXD3D9TextureStageState[4], 2 );
   mD3DDevice->SetTextureStageState( 4, GFXD3D9TextureStageState[5], 1 );
   mD3DDevice->SetTextureStageState( 4, GFXD3D9TextureStageState[6], 0 );
   mD3DDevice->SetTextureStageState( 4, GFXD3D9TextureStageState[7], 0 );
   mD3DDevice->SetTextureStageState( 4, GFXD3D9TextureStageState[8], 0 );
   mD3DDevice->SetTextureStageState( 4, GFXD3D9TextureStageState[9], 0 );
   mD3DDevice->SetTextureStageState( 4, GFXD3D9TextureStageState[10], 4 );
   mD3DDevice->SetTextureStageState( 4, GFXD3D9TextureStageState[11], 0 );
   mD3DDevice->SetTextureStageState( 4, GFXD3D9TextureStageState[12], 0 );
   mD3DDevice->SetTextureStageState( 4, GFXD3D9TextureStageState[13], 0 );
   mD3DDevice->SetTextureStageState( 4, GFXD3D9TextureStageState[14], 1 );
   mD3DDevice->SetTextureStageState( 4, GFXD3D9TextureStageState[15], 1 );
   mD3DDevice->SetTextureStageState( 4, GFXD3D9TextureStageState[16], 1 );
   mD3DDevice->SetTextureStageState( 4, GFXD3D9TextureStageState[17], 0 );
   mD3DDevice->SetTextureStageState( 5, GFXD3D9TextureStageState[0], 1 );
   mD3DDevice->SetTextureStageState( 5, GFXD3D9TextureStageState[1], 2 );
   mD3DDevice->SetTextureStageState( 5, GFXD3D9TextureStageState[2], 1 );
   mD3DDevice->SetTextureStageState( 5, GFXD3D9TextureStageState[3], 1 );
   mD3DDevice->SetTextureStageState( 5, GFXD3D9TextureStageState[4], 2 );
   mD3DDevice->SetTextureStageState( 5, GFXD3D9TextureStageState[5], 1 );
   mD3DDevice->SetTextureStageState( 5, GFXD3D9TextureStageState[6], 0 );
   mD3DDevice->SetTextureStageState( 5, GFXD3D9TextureStageState[7], 0 );
   mD3DDevice->SetTextureStageState( 5, GFXD3D9TextureStageState[8], 0 );
   mD3DDevice->SetTextureStageState( 5, GFXD3D9TextureStageState[9], 0 );
   mD3DDevice->SetTextureStageState( 5, GFXD3D9TextureStageState[10], 5 );
   mD3DDevice->SetTextureStageState( 5, GFXD3D9TextureStageState[11], 0 );
   mD3DDevice->SetTextureStageState( 5, GFXD3D9TextureStageState[12], 0 );
   mD3DDevice->SetTextureStageState( 5, GFXD3D9TextureStageState[13], 0 );
   mD3DDevice->SetTextureStageState( 5, GFXD3D9TextureStageState[14], 1 );
   mD3DDevice->SetTextureStageState( 5, GFXD3D9TextureStageState[15], 1 );
   mD3DDevice->SetTextureStageState( 5, GFXD3D9TextureStageState[16], 1 );
   mD3DDevice->SetTextureStageState( 5, GFXD3D9TextureStageState[17], 0 );
   mD3DDevice->SetTextureStageState( 6, GFXD3D9TextureStageState[0], 1 );
   mD3DDevice->SetTextureStageState( 6, GFXD3D9TextureStageState[1], 2 );
   mD3DDevice->SetTextureStageState( 6, GFXD3D9TextureStageState[2], 1 );
   mD3DDevice->SetTextureStageState( 6, GFXD3D9TextureStageState[3], 1 );
   mD3DDevice->SetTextureStageState( 6, GFXD3D9TextureStageState[4], 2 );
   mD3DDevice->SetTextureStageState( 6, GFXD3D9TextureStageState[5], 1 );
   mD3DDevice->SetTextureStageState( 6, GFXD3D9TextureStageState[6], 0 );
   mD3DDevice->SetTextureStageState( 6, GFXD3D9TextureStageState[7], 0 );
   mD3DDevice->SetTextureStageState( 6, GFXD3D9TextureStageState[8], 0 );
   mD3DDevice->SetTextureStageState( 6, GFXD3D9TextureStageState[9], 0 );
   mD3DDevice->SetTextureStageState( 6, GFXD3D9TextureStageState[10], 6 );
   mD3DDevice->SetTextureStageState( 6, GFXD3D9TextureStageState[11], 0 );
   mD3DDevice->SetTextureStageState( 6, GFXD3D9TextureStageState[12], 0 );
   mD3DDevice->SetTextureStageState( 6, GFXD3D9TextureStageState[13], 0 );
   mD3DDevice->SetTextureStageState( 6, GFXD3D9TextureStageState[14], 1 );
   mD3DDevice->SetTextureStageState( 6, GFXD3D9TextureStageState[15], 1 );
   mD3DDevice->SetTextureStageState( 6, GFXD3D9TextureStageState[16], 1 );
   mD3DDevice->SetTextureStageState( 6, GFXD3D9TextureStageState[17], 0 );
   mD3DDevice->SetTextureStageState( 7, GFXD3D9TextureStageState[0], 1 );
   mD3DDevice->SetTextureStageState( 7, GFXD3D9TextureStageState[1], 2 );
   mD3DDevice->SetTextureStageState( 7, GFXD3D9TextureStageState[2], 1 );
   mD3DDevice->SetTextureStageState( 7, GFXD3D9TextureStageState[3], 1 );
   mD3DDevice->SetTextureStageState( 7, GFXD3D9TextureStageState[4], 2 );
   mD3DDevice->SetTextureStageState( 7, GFXD3D9TextureStageState[5], 1 );
   mD3DDevice->SetTextureStageState( 7, GFXD3D9TextureStageState[6], 0 );
   mD3DDevice->SetTextureStageState( 7, GFXD3D9TextureStageState[7], 0 );
   mD3DDevice->SetTextureStageState( 7, GFXD3D9TextureStageState[8], 0 );
   mD3DDevice->SetTextureStageState( 7, GFXD3D9TextureStageState[9], 0 );
   mD3DDevice->SetTextureStageState( 7, GFXD3D9TextureStageState[10], 7 );
   mD3DDevice->SetTextureStageState( 7, GFXD3D9TextureStageState[11], 0 );
   mD3DDevice->SetTextureStageState( 7, GFXD3D9TextureStageState[12], 0 );
   mD3DDevice->SetTextureStageState( 7, GFXD3D9TextureStageState[13], 0 );
   mD3DDevice->SetTextureStageState( 7, GFXD3D9TextureStageState[14], 1 );
   mD3DDevice->SetTextureStageState( 7, GFXD3D9TextureStageState[15], 1 );
   mD3DDevice->SetTextureStageState( 7, GFXD3D9TextureStageState[16], 1 );
   mD3DDevice->SetTextureStageState( 7, GFXD3D9TextureStageState[17], 0 );

   // Sampler states
   mD3DDevice->SetSamplerState( 0, GFXD3D9SamplerState[0], 1 );
   mD3DDevice->SetSamplerState( 0, GFXD3D9SamplerState[1], 1 );
   mD3DDevice->SetSamplerState( 0, GFXD3D9SamplerState[2], 1 );
   mD3DDevice->SetSamplerState( 0, GFXD3D9SamplerState[3], 0 );
   mD3DDevice->SetSamplerState( 0, GFXD3D9SamplerState[4], 1 );
   mD3DDevice->SetSamplerState( 0, GFXD3D9SamplerState[5], 1 );
   mD3DDevice->SetSamplerState( 0, GFXD3D9SamplerState[6], 0 );
   mD3DDevice->SetSamplerState( 0, GFXD3D9SamplerState[7], 0 );
   mD3DDevice->SetSamplerState( 0, GFXD3D9SamplerState[8], 0 );
   mD3DDevice->SetSamplerState( 0, GFXD3D9SamplerState[9], 1 );
   mD3DDevice->SetSamplerState( 0, GFXD3D9SamplerState[10], 0 );
   mD3DDevice->SetSamplerState( 0, GFXD3D9SamplerState[11], 0 );
   mD3DDevice->SetSamplerState( 0, GFXD3D9SamplerState[12], 0 );
   mD3DDevice->SetSamplerState( 1, GFXD3D9SamplerState[0], 1 );
   mD3DDevice->SetSamplerState( 1, GFXD3D9SamplerState[1], 1 );
   mD3DDevice->SetSamplerState( 1, GFXD3D9SamplerState[2], 1 );
   mD3DDevice->SetSamplerState( 1, GFXD3D9SamplerState[3], 0 );
   mD3DDevice->SetSamplerState( 1, GFXD3D9SamplerState[4], 1 );
   mD3DDevice->SetSamplerState( 1, GFXD3D9SamplerState[5], 1 );
   mD3DDevice->SetSamplerState( 1, GFXD3D9SamplerState[6], 0 );
   mD3DDevice->SetSamplerState( 1, GFXD3D9SamplerState[7], 0 );
   mD3DDevice->SetSamplerState( 1, GFXD3D9SamplerState[8], 0 );
   mD3DDevice->SetSamplerState( 1, GFXD3D9SamplerState[9], 1 );
   mD3DDevice->SetSamplerState( 1, GFXD3D9SamplerState[10], 0 );
   mD3DDevice->SetSamplerState( 1, GFXD3D9SamplerState[11], 0 );
   mD3DDevice->SetSamplerState( 1, GFXD3D9SamplerState[12], 0 );
   mD3DDevice->SetSamplerState( 2, GFXD3D9SamplerState[0], 1 );
   mD3DDevice->SetSamplerState( 2, GFXD3D9SamplerState[1], 1 );
   mD3DDevice->SetSamplerState( 2, GFXD3D9SamplerState[2], 1 );
   mD3DDevice->SetSamplerState( 2, GFXD3D9SamplerState[3], 0 );
   mD3DDevice->SetSamplerState( 2, GFXD3D9SamplerState[4], 1 );
   mD3DDevice->SetSamplerState( 2, GFXD3D9SamplerState[5], 1 );
   mD3DDevice->SetSamplerState( 2, GFXD3D9SamplerState[6], 0 );
   mD3DDevice->SetSamplerState( 2, GFXD3D9SamplerState[7], 0 );
   mD3DDevice->SetSamplerState( 2, GFXD3D9SamplerState[8], 0 );
   mD3DDevice->SetSamplerState( 2, GFXD3D9SamplerState[9], 1 );
   mD3DDevice->SetSamplerState( 2, GFXD3D9SamplerState[10], 0 );
   mD3DDevice->SetSamplerState( 2, GFXD3D9SamplerState[11], 0 );
   mD3DDevice->SetSamplerState( 2, GFXD3D9SamplerState[12], 0 );
   mD3DDevice->SetSamplerState( 3, GFXD3D9SamplerState[0], 1 );
   mD3DDevice->SetSamplerState( 3, GFXD3D9SamplerState[1], 1 );
   mD3DDevice->SetSamplerState( 3, GFXD3D9SamplerState[2], 1 );
   mD3DDevice->SetSamplerState( 3, GFXD3D9SamplerState[3], 0 );
   mD3DDevice->SetSamplerState( 3, GFXD3D9SamplerState[4], 1 );
   mD3DDevice->SetSamplerState( 3, GFXD3D9SamplerState[5], 1 );
   mD3DDevice->SetSamplerState( 3, GFXD3D9SamplerState[6], 0 );
   mD3DDevice->SetSamplerState( 3, GFXD3D9SamplerState[7], 0 );
   mD3DDevice->SetSamplerState( 3, GFXD3D9SamplerState[8], 0 );
   mD3DDevice->SetSamplerState( 3, GFXD3D9SamplerState[9], 1 );
   mD3DDevice->SetSamplerState( 3, GFXD3D9SamplerState[10], 0 );
   mD3DDevice->SetSamplerState( 3, GFXD3D9SamplerState[11], 0 );
   mD3DDevice->SetSamplerState( 3, GFXD3D9SamplerState[12], 0 );
   mD3DDevice->SetSamplerState( 4, GFXD3D9SamplerState[0], 1 );
   mD3DDevice->SetSamplerState( 4, GFXD3D9SamplerState[1], 1 );
   mD3DDevice->SetSamplerState( 4, GFXD3D9SamplerState[2], 1 );
   mD3DDevice->SetSamplerState( 4, GFXD3D9SamplerState[3], 0 );
   mD3DDevice->SetSamplerState( 4, GFXD3D9SamplerState[4], 1 );
   mD3DDevice->SetSamplerState( 4, GFXD3D9SamplerState[5], 1 );
   mD3DDevice->SetSamplerState( 4, GFXD3D9SamplerState[6], 0 );
   mD3DDevice->SetSamplerState( 4, GFXD3D9SamplerState[7], 0 );
   mD3DDevice->SetSamplerState( 4, GFXD3D9SamplerState[8], 0 );
   mD3DDevice->SetSamplerState( 4, GFXD3D9SamplerState[9], 1 );
   mD3DDevice->SetSamplerState( 4, GFXD3D9SamplerState[10], 0 );
   mD3DDevice->SetSamplerState( 4, GFXD3D9SamplerState[11], 0 );
   mD3DDevice->SetSamplerState( 4, GFXD3D9SamplerState[12], 0 );
   mD3DDevice->SetSamplerState( 5, GFXD3D9SamplerState[0], 1 );
   mD3DDevice->SetSamplerState( 5, GFXD3D9SamplerState[1], 1 );
   mD3DDevice->SetSamplerState( 5, GFXD3D9SamplerState[2], 1 );
   mD3DDevice->SetSamplerState( 5, GFXD3D9SamplerState[3], 0 );
   mD3DDevice->SetSamplerState( 5, GFXD3D9SamplerState[4], 1 );
   mD3DDevice->SetSamplerState( 5, GFXD3D9SamplerState[5], 1 );
   mD3DDevice->SetSamplerState( 5, GFXD3D9SamplerState[6], 0 );
   mD3DDevice->SetSamplerState( 5, GFXD3D9SamplerState[7], 0 );
   mD3DDevice->SetSamplerState( 5, GFXD3D9SamplerState[8], 0 );
   mD3DDevice->SetSamplerState( 5, GFXD3D9SamplerState[9], 1 );
   mD3DDevice->SetSamplerState( 5, GFXD3D9SamplerState[10], 0 );
   mD3DDevice->SetSamplerState( 5, GFXD3D9SamplerState[11], 0 );
   mD3DDevice->SetSamplerState( 5, GFXD3D9SamplerState[12], 0 );
   mD3DDevice->SetSamplerState( 6, GFXD3D9SamplerState[0], 1 );
   mD3DDevice->SetSamplerState( 6, GFXD3D9SamplerState[1], 1 );
   mD3DDevice->SetSamplerState( 6, GFXD3D9SamplerState[2], 1 );
   mD3DDevice->SetSamplerState( 6, GFXD3D9SamplerState[3], 0 );
   mD3DDevice->SetSamplerState( 6, GFXD3D9SamplerState[4], 1 );
   mD3DDevice->SetSamplerState( 6, GFXD3D9SamplerState[5], 1 );
   mD3DDevice->SetSamplerState( 6, GFXD3D9SamplerState[6], 0 );
   mD3DDevice->SetSamplerState( 6, GFXD3D9SamplerState[7], 0 );
   mD3DDevice->SetSamplerState( 6, GFXD3D9SamplerState[8], 0 );
   mD3DDevice->SetSamplerState( 6, GFXD3D9SamplerState[9], 1 );
   mD3DDevice->SetSamplerState( 6, GFXD3D9SamplerState[10], 0 );
   mD3DDevice->SetSamplerState( 6, GFXD3D9SamplerState[11], 0 );
   mD3DDevice->SetSamplerState( 6, GFXD3D9SamplerState[12], 0 );
   mD3DDevice->SetSamplerState( 7, GFXD3D9SamplerState[0], 1 );
   mD3DDevice->SetSamplerState( 7, GFXD3D9SamplerState[1], 1 );
   mD3DDevice->SetSamplerState( 7, GFXD3D9SamplerState[2], 1 );
   mD3DDevice->SetSamplerState( 7, GFXD3D9SamplerState[3], 0 );
   mD3DDevice->SetSamplerState( 7, GFXD3D9SamplerState[4], 1 );
   mD3DDevice->SetSamplerState( 7, GFXD3D9SamplerState[5], 1 );
   mD3DDevice->SetSamplerState( 7, GFXD3D9SamplerState[6], 0 );
   mD3DDevice->SetSamplerState( 7, GFXD3D9SamplerState[7], 0 );
   mD3DDevice->SetSamplerState( 7, GFXD3D9SamplerState[8], 0 );
   mD3DDevice->SetSamplerState( 7, GFXD3D9SamplerState[9], 1 );
   mD3DDevice->SetSamplerState( 7, GFXD3D9SamplerState[10], 0 );
   mD3DDevice->SetSamplerState( 7, GFXD3D9SamplerState[11], 0 );
   mD3DDevice->SetSamplerState( 7, GFXD3D9SamplerState[12], 0 );
}

void GFXPCD3D9Device::_validateMultisampleParams(D3DFORMAT format, D3DMULTISAMPLE_TYPE & aatype, DWORD & aalevel) const
{
   if (aatype != D3DMULTISAMPLE_NONE)
   {
      DWORD MaxSampleQualities;      
      mD3D->CheckDeviceMultiSampleType(mAdapterIndex, D3DDEVTYPE_HAL, format, FALSE, D3DMULTISAMPLE_NONMASKABLE, &MaxSampleQualities);
      aatype = D3DMULTISAMPLE_NONMASKABLE;
      aalevel = getMin((U32)aalevel, (U32)MaxSampleQualities-1);
   }
}

bool GFXPCD3D9Device::beginSceneInternal() 
{
   // Make sure we have a device
   HRESULT res = mD3DDevice->TestCooperativeLevel();

   S32 attempts = 0;
   const S32 MaxAttempts = 40;
   const S32 SleepMsPerAttempt = 50;
   while(res == D3DERR_DEVICELOST && attempts < MaxAttempts)
   {
      // Lost device! Just keep querying
      res = mD3DDevice->TestCooperativeLevel();

      Con::warnf("GFXPCD3D9Device::beginScene - Device needs to be reset, waiting on device...");

      Sleep(SleepMsPerAttempt);
      attempts++;
   }

   if (attempts >= MaxAttempts && res == D3DERR_DEVICELOST)
   {
      Con::errorf("GFXPCD3D9Device::beginScene - Device lost and reset wait time exceeded, skipping reset (will retry later)");
      mCanCurrentlyRender = false;
      return false;
   }

   // Trigger a reset if we can't get a good result from TestCooperativeLevel.
   if(res == D3DERR_DEVICENOTRESET)
   {
      Con::warnf("GFXPCD3D9Device::beginScene - Device needs to be reset, resetting device...");

      // Reset the device!
      GFXResource *walk = mResourceListHead;
      while(walk)
      {
         // Find the window target with implicit flag set and reset the device with its presentation params.
         if(GFXPCD3D9WindowTarget *gdwt = dynamic_cast<GFXPCD3D9WindowTarget*>(walk))
         {
            if(gdwt->mImplicit)
            {
               reset(gdwt->mPresentationParams);
               break;
            }
         }

         walk = walk->getNextResource();
      }
   }

   // Call parent
   return Parent::beginSceneInternal();
}

GFXWindowTarget * GFXPCD3D9Device::allocWindowTarget( PlatformWindow *window )
{
   AssertFatal(window,"GFXD3D9Device::allocWindowTarget - no window provided!");
#ifndef TORQUE_OS_XENON
   AssertFatal(dynamic_cast<Win32Window*>(window), 
      "GFXD3D9Device::allocWindowTarget - only works with Win32Windows!");
#endif

   // Set up a new window target...
   GFXPCD3D9WindowTarget *gdwt = new GFXPCD3D9WindowTarget();
   gdwt->mWindow = window;
   gdwt->mSize = window->getClientExtent();
   gdwt->mDevice = this;
   gdwt->initPresentationParams();

   // Now, we have to init & bind our device... we have basically two scenarios
   // of which the first is:
   if(mD3DDevice == NULL)
   {
      // Allocate the device.
      init(window->getVideoMode(), window);

      // Cool, we have the device, grab back the depthstencil buffer as well
      // as the swap chain.
      gdwt->mImplicit = true;
      gdwt->setImplicitSwapChain();
   }
   else
   {
      // And the second case:
      // Initialized device, create an additional swap chain.
      gdwt->mImplicit = false;
      gdwt->createAdditionalSwapChain();         
   }

   gdwt->registerResourceWithDevice(this);

   return gdwt;
}

GFXTextureTarget * GFXPCD3D9Device::allocRenderToTextureTarget()
{
   GFXPCD3D9TextureTarget *targ = new GFXPCD3D9TextureTarget();
   targ->mDevice = this;

   targ->registerResourceWithDevice(this);

   return targ;
}

//-----------------------------------------------------------------------------
// Reset D3D device
//-----------------------------------------------------------------------------
void GFXPCD3D9Device::reset( D3DPRESENT_PARAMETERS &d3dpp )
{
   if(!mD3DDevice)
      return;

   mInitialized = false;

   mMultisampleType = d3dpp.MultiSampleType;
   mMultisampleLevel = d3dpp.MultiSampleQuality;
   _validateMultisampleParams(d3dpp.BackBufferFormat, mMultisampleType, mMultisampleLevel);

   // Clean up some commonly dangling state. This helps prevents issues with
   // items that are destroyed by the texture manager callbacks and recreated
   // later, but still left bound.
   setVertexBuffer(NULL);
   setPrimitiveBuffer(NULL);
   for(S32 i=0; i<getNumSamplers(); i++)
      setTexture(i, NULL);

   // Deal with the depth/stencil buffer.
   if(mDeviceDepthStencil)
   {
      Con::printf("GFXPCD3D9Device::reset - depthstencil %x has %d ref's", mDeviceDepthStencil, mDeviceDepthStencil->AddRef()-1);
      mDeviceDepthStencil->Release();
   }

   // First release all the stuff we allocated from D3DPOOL_DEFAULT
   releaseDefaultPoolResources();

   // reset device
   Con::printf( "--- Resetting D3D Device ---" );
   HRESULT hres = S_OK;
   hres = mD3DDevice->Reset( &d3dpp );

   if( FAILED( hres ) )
   {
      while( mD3DDevice->TestCooperativeLevel() == D3DERR_DEVICELOST )
      {
         Sleep( 100 );
      }

      hres = mD3DDevice->Reset( &d3dpp );
   }

   D3D9Assert( hres, "GFXD3D9Device::reset - Failed to create D3D Device!" );
   mInitialized = true;

   // Setup default states
   initStates();

   // Now re aquire all the resources we trashed earlier
   reacquireDefaultPoolResources();

   // Mark everything dirty and flush to card, for sanity.
   updateStates(true);
}

//
// Register this device with GFXInit
//
class GFXPCD3D9RegisterDevice
{
public:
   GFXPCD3D9RegisterDevice()
   {
      GFXInit::getRegisterDeviceSignal().notify(&GFXPCD3D9Device::enumerateAdapters);
   }
};

static GFXPCD3D9RegisterDevice pPCD3D9RegisterDevice;

//-----------------------------------------------------------------------------
/// Parse command line arguments for window creation
//-----------------------------------------------------------------------------
static void sgPCD3D9DeviceHandleCommandLine( S32 argc, const char **argv )
{
   for (U32 i = 1; i < argc; i++)
   {
      String s(argv[i]);
      if (s.equal("-nvperfhud", String::NoCase))
      {
         GFXPCD3D9Device::mEnableNVPerfHUD = true;
         break;
      }
   }   
}

// Register the command line parsing hook
static ProcessRegisterCommandLine sgCommandLine( sgPCD3D9DeviceHandleCommandLine );

extern "C" HRESULT WINAPI D3D_GetBackBufferNoRef(IDirect3DSurface9 **ppSurface)
{
    HRESULT hr = S_OK;
  
    GFXD3D9Device *dev = static_cast<GFXD3D9Device *>(GFX);

    if (!dev)
    {
       *ppSurface = NULL;
       return S_OK;
    }

    *ppSurface = dev->getBackBuffer();

    return hr;
}