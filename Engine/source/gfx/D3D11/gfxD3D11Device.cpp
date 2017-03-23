//-----------------------------------------------------------------------------
// Copyright (c) 2015 GarageGames, LLC
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

#include "console/console.h"
#include "core/stream/fileStream.h"
#include "core/strings/unicode.h"
#include "core/util/journal/process.h"
#include "gfx/D3D11/gfxD3D11Device.h"
#include "gfx/D3D11/gfxD3D11CardProfiler.h"
#include "gfx/D3D11/gfxD3D11VertexBuffer.h"
#include "gfx/D3D11/gfxD3D11EnumTranslate.h"
#include "gfx/D3D11/gfxD3D11QueryFence.h"
#include "gfx/D3D11/gfxD3D11OcclusionQuery.h"
#include "gfx/D3D11/gfxD3D11Shader.h"
#include "gfx/D3D11/gfxD3D11Target.h"
#include "platformWin32/platformWin32.h"
#include "windowManager/win32/win32Window.h"
#include "windowManager/platformWindow.h"
#include "gfx/D3D11/screenshotD3D11.h"
#include "materials/shaderData.h"
#include <d3d9.h> //ok now stressing out folks, this is just for debug events(D3DPER) :)

#ifdef TORQUE_DEBUG
#include "d3d11sdklayers.h"
#endif

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")

GFXAdapter::CreateDeviceInstanceDelegate GFXD3D11Device::mCreateDeviceInstance(GFXD3D11Device::createInstance);

GFXDevice *GFXD3D11Device::createInstance(U32 adapterIndex)
{
   GFXD3D11Device* dev = new GFXD3D11Device(adapterIndex);
   return dev;
}

class GFXPCD3D11RegisterDevice
{
public:
   GFXPCD3D11RegisterDevice()
   {
      GFXInit::getRegisterDeviceSignal().notify(&GFXD3D11Device::enumerateAdapters);
   }
};

static GFXPCD3D11RegisterDevice pPCD3D11RegisterDevice;

//-----------------------------------------------------------------------------
/// Parse command line arguments for window creation
//-----------------------------------------------------------------------------
static void sgPCD3D11DeviceHandleCommandLine(S32 argc, const char **argv)
{
   // useful to pass parameters by command line for d3d (e.g. -dx9 -dx11)
   for (U32 i = 1; i < argc; i++)
   {
      argv[i];
   }
}

// Register the command line parsing hook
static ProcessRegisterCommandLine sgCommandLine(sgPCD3D11DeviceHandleCommandLine);

GFXD3D11Device::GFXD3D11Device(U32 index)
{
   mDeviceSwizzle32 = &Swizzles::bgra;
   GFXVertexColor::setSwizzle(mDeviceSwizzle32);

   mDeviceSwizzle24 = &Swizzles::bgr;

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

   mPixVersion = 0.0;

   mVertexShaderTarget = String::EmptyString;
   mPixelShaderTarget = String::EmptyString;
   mShaderModel = String::EmptyString;

   mDrawInstancesCount = 0;

   mCardProfiler = NULL;

   mDeviceDepthStencil = NULL;
   mDeviceBackbuffer = NULL;
   mDeviceBackBufferView = NULL;
   mDeviceDepthStencilView = NULL;

   mCreateFenceType = -1; // Unknown, test on first allocate

   mCurrentConstBuffer = NULL;

   mOcclusionQuerySupported = false;

   mDebugLayers = false;

   for (U32 i = 0; i < GS_COUNT; ++i)
      mModelViewProjSC[i] = NULL;

   // Set up the Enum translation tables
   GFXD3D11EnumTranslate::init();
}

GFXD3D11Device::~GFXD3D11Device()
{
   // Release our refcount on the current stateblock object
   mCurrentStateBlock = NULL;

   releaseDefaultPoolResources();

   mD3DDeviceContext->ClearState();
   mD3DDeviceContext->Flush();

   // Free the sampler states
   SamplerMap::Iterator sampIter = mSamplersMap.begin();
   for (; sampIter != mSamplersMap.end(); ++sampIter)
      SAFE_RELEASE(sampIter->value);

   // Free the vertex declarations.
   VertexDeclMap::Iterator iter = mVertexDecls.begin();
   for (; iter != mVertexDecls.end(); iter++)
      delete iter->value;

   // Forcibly clean up the pools
   mVolatileVBList.setSize(0);
   mDynamicPB = NULL;

   // And release our D3D resources.
   SAFE_RELEASE(mDeviceDepthStencilView);
   SAFE_RELEASE(mDeviceBackBufferView);
   SAFE_RELEASE(mDeviceDepthStencil);
   SAFE_RELEASE(mDeviceBackbuffer);
   SAFE_RELEASE(mD3DDeviceContext);

   SAFE_DELETE(mCardProfiler);
   SAFE_DELETE(gScreenShot);

#ifdef TORQUE_DEBUG
   if (mDebugLayers)
   {
      ID3D11Debug *pDebug = NULL;
      mD3DDevice->QueryInterface(IID_PPV_ARGS(&pDebug));
      AssertFatal(pDebug, "~GFXD3D11Device- Failed to get debug layer");
      pDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
      SAFE_RELEASE(pDebug);
   }
#endif

   SAFE_RELEASE(mSwapChain);
   SAFE_RELEASE(mD3DDevice);
}

GFXFormat GFXD3D11Device::selectSupportedFormat(GFXTextureProfile *profile, const Vector<GFXFormat> &formats, bool texture, bool mustblend, bool mustfilter)
{
   U32 features = 0;
   if(texture)
       features |= D3D11_FORMAT_SUPPORT_TEXTURE2D;
   if(mustblend)
       features |= D3D11_FORMAT_SUPPORT_BLENDABLE;
   if(mustfilter)
       features |= D3D11_FORMAT_SUPPORT_SHADER_SAMPLE;
   
   for(U32 i = 0; i < formats.size(); i++)
   {
      if(GFXD3D11TextureFormat[formats[i]] == DXGI_FORMAT_UNKNOWN)
         continue;

      U32 supportFlag = 0;
      mD3DDevice->CheckFormatSupport(GFXD3D11TextureFormat[formats[i]],&supportFlag);
      if(supportFlag & features)
         return formats[i];
   }
   
   return GFXFormatR8G8B8A8;
}

DXGI_SWAP_CHAIN_DESC GFXD3D11Device::setupPresentParams(const GFXVideoMode &mode, const HWND &hwnd)
{
   DXGI_SWAP_CHAIN_DESC d3dpp;
   ZeroMemory(&d3dpp, sizeof(d3dpp));

   DXGI_SAMPLE_DESC sampleDesc;
   sampleDesc.Count = 1;
   sampleDesc.Quality = 0;

   mMultisampleDesc = sampleDesc;

   d3dpp.BufferCount = !smDisableVSync ? 2 : 1; // triple buffering when vsync is on.
   d3dpp.BufferDesc.Width = mode.resolution.x;
   d3dpp.BufferDesc.Height = mode.resolution.y;
   d3dpp.BufferDesc.Format = GFXD3D11TextureFormat[GFXFormatR8G8B8A8];
   d3dpp.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
   d3dpp.OutputWindow = hwnd;
   d3dpp.SampleDesc = sampleDesc;
   d3dpp.Windowed = !mode.fullScreen;
   d3dpp.BufferDesc.RefreshRate.Numerator = mode.refreshRate;
   d3dpp.BufferDesc.RefreshRate.Denominator = 1;
   d3dpp.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

   if (mode.fullScreen)
   {
      d3dpp.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
      d3dpp.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
      d3dpp.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
   }

   return d3dpp;
}

void GFXD3D11Device::enumerateAdapters(Vector<GFXAdapter*> &adapterList)
{
   IDXGIAdapter1* EnumAdapter;
   IDXGIFactory1* DXGIFactory;

   CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&DXGIFactory));

   for(U32 adapterIndex = 0; DXGIFactory->EnumAdapters1(adapterIndex, &EnumAdapter) != DXGI_ERROR_NOT_FOUND; ++adapterIndex) 
   {
      GFXAdapter *toAdd = new GFXAdapter;
      toAdd->mType  = Direct3D11;
      toAdd->mIndex = adapterIndex;
      toAdd->mCreateDeviceInstanceDelegate = mCreateDeviceInstance;

      toAdd->mShaderModel = 5.0f;
      DXGI_ADAPTER_DESC1 desc;
      EnumAdapter->GetDesc1(&desc);

      // LUID identifies adapter for oculus rift
      dMemcpy(&toAdd->mLUID, &desc.AdapterLuid, sizeof(toAdd->mLUID));

      size_t size=wcslen(desc.Description);
      char *str = new char[size+1];

      wcstombs(str, desc.Description,size);
      str[size]='\0';
      String Description=str;
      SAFE_DELETE_ARRAY(str);

      dStrncpy(toAdd->mName, Description.c_str(), GFXAdapter::MaxAdapterNameLen);
      dStrncat(toAdd->mName, " (D3D11)", GFXAdapter::MaxAdapterNameLen);

      IDXGIOutput* pOutput = NULL; 
      HRESULT hr;

      hr = EnumAdapter->EnumOutputs(adapterIndex, &pOutput);

      if(hr == DXGI_ERROR_NOT_FOUND)
      {
         SAFE_RELEASE(EnumAdapter);
         break;
      }

      if(FAILED(hr))
         AssertFatal(false, "GFXD3D11Device::enumerateAdapters -> EnumOutputs call failure");

      UINT numModes = 0;
      DXGI_MODE_DESC* displayModes = NULL;
      DXGI_FORMAT format = DXGI_FORMAT_B8G8R8A8_UNORM;

      // Get the number of elements
      hr = pOutput->GetDisplayModeList(format, 0, &numModes, NULL);

      if(FAILED(hr))
         AssertFatal(false, "GFXD3D11Device::enumerateAdapters -> GetDisplayModeList call failure");

      displayModes = new DXGI_MODE_DESC[numModes]; 

      // Get the list
      hr = pOutput->GetDisplayModeList(format, 0, &numModes, displayModes);

      if(FAILED(hr))
         AssertFatal(false, "GFXD3D11Device::enumerateAdapters -> GetDisplayModeList call failure");

      for(U32 numMode = 0; numMode < numModes; ++numMode)
      {
         GFXVideoMode vmAdd;

         vmAdd.fullScreen = true;
         vmAdd.bitDepth = 32;
         vmAdd.refreshRate = displayModes[numMode].RefreshRate.Numerator / displayModes[numMode].RefreshRate.Denominator;
         vmAdd.resolution.x = displayModes[numMode].Width;
         vmAdd.resolution.y = displayModes[numMode].Height;
         toAdd->mAvailableModes.push_back(vmAdd);
      }

      //Check adapater can handle feature level 10
      D3D_FEATURE_LEVEL deviceFeature;
      ID3D11Device *pTmpDevice = nullptr;
      // Create temp Direct3D11 device.
      bool suitable = true;
      UINT createDeviceFlags = D3D11_CREATE_DEVICE_SINGLETHREADED | D3D11_CREATE_DEVICE_BGRA_SUPPORT;
      hr = D3D11CreateDevice(EnumAdapter, D3D_DRIVER_TYPE_UNKNOWN, NULL, createDeviceFlags, NULL, 0, D3D11_SDK_VERSION, &pTmpDevice, &deviceFeature, NULL);

      if (FAILED(hr))
         suitable = false;

      if (deviceFeature < D3D_FEATURE_LEVEL_10_0)
         suitable = false;

      //double check we support required bgra format for LEVEL_10_0 & LEVEL_10_1
      if (deviceFeature == D3D_FEATURE_LEVEL_10_0 || deviceFeature == D3D_FEATURE_LEVEL_10_1)
      {
         U32 formatSupported = 0;
         pTmpDevice->CheckFormatSupport(DXGI_FORMAT_B8G8R8A8_UNORM, &formatSupported);
         U32 flagsRequired = D3D11_FORMAT_SUPPORT_RENDER_TARGET | D3D11_FORMAT_SUPPORT_DISPLAY;
         if (!(formatSupported && flagsRequired))
         {
            Con::printf("DXGI adapter: %s does not support BGRA", Description.c_str());
            suitable = false;
         }
      }

      delete[] displayModes;
      SAFE_RELEASE(pTmpDevice);
      SAFE_RELEASE(pOutput);
      SAFE_RELEASE(EnumAdapter);

      if (suitable)
      {
         adapterList.push_back(toAdd);
      }
      else
      {
         Con::printf("DXGI adapter: %s does not support D3D11 feature level 10 or better", Description.c_str());
         delete toAdd;
      }
   }

   SAFE_RELEASE(DXGIFactory);
}

void GFXD3D11Device::enumerateVideoModes() 
{
   mVideoModes.clear();

   IDXGIAdapter1* EnumAdapter;
   IDXGIFactory1* DXGIFactory;
   HRESULT hr;

   hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&DXGIFactory));

   if (FAILED(hr))
      AssertFatal(false, "GFXD3D11Device::enumerateVideoModes -> CreateDXGIFactory1 call failure");

   for(U32 adapterIndex = 0; DXGIFactory->EnumAdapters1(adapterIndex, &EnumAdapter) != DXGI_ERROR_NOT_FOUND; ++adapterIndex) 
   {
      IDXGIOutput* pOutput = NULL;      

      hr = EnumAdapter->EnumOutputs(adapterIndex, &pOutput);

      if(hr == DXGI_ERROR_NOT_FOUND)
      {
         SAFE_RELEASE(EnumAdapter);
         break;
      }

      if(FAILED(hr))
         AssertFatal(false, "GFXD3D11Device::enumerateVideoModes -> EnumOutputs call failure");

      UINT numModes = 0;
      DXGI_MODE_DESC* displayModes = NULL;
      DXGI_FORMAT format = GFXD3D11TextureFormat[GFXFormatR8G8B8A8];

      // Get the number of elements
      hr = pOutput->GetDisplayModeList(format, 0, &numModes, NULL);

      if(FAILED(hr))
         AssertFatal(false, "GFXD3D11Device::enumerateVideoModes -> GetDisplayModeList call failure");

      displayModes = new DXGI_MODE_DESC[numModes]; 

      // Get the list
      hr = pOutput->GetDisplayModeList(format, 0, &numModes, displayModes);

      if(FAILED(hr))
         AssertFatal(false, "GFXD3D11Device::enumerateVideoModes -> GetDisplayModeList call failure");

      for(U32 numMode = 0; numMode < numModes; ++numMode)
      {
         GFXVideoMode toAdd;

         toAdd.fullScreen = false;
         toAdd.bitDepth = 32;
         toAdd.refreshRate = displayModes[numMode].RefreshRate.Numerator / displayModes[numMode].RefreshRate.Denominator;
         toAdd.resolution.x = displayModes[numMode].Width;
         toAdd.resolution.y = displayModes[numMode].Height;
         mVideoModes.push_back(toAdd);
      }

      delete[] displayModes;
      SAFE_RELEASE(pOutput);
      SAFE_RELEASE(EnumAdapter);
   }

   SAFE_RELEASE(DXGIFactory);
}

void GFXD3D11Device::init(const GFXVideoMode &mode, PlatformWindow *window)
{
   AssertFatal(window, "GFXD3D11Device::init - must specify a window!");
   HWND hwnd = (HWND)window->getSystemWindow(PlatformWindow::WindowSystem_Windows);
   SetFocus(hwnd);//ensure window has focus

   UINT createDeviceFlags = D3D11_CREATE_DEVICE_SINGLETHREADED | D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef TORQUE_DEBUG
   createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
   mDebugLayers = true;
#endif

   D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_HARDWARE;// use D3D_DRIVER_TYPE_REFERENCE for reference device
   // create a device & device context
   HRESULT hres = D3D11CreateDevice(NULL,
                                    driverType,
                                    NULL,
                                    createDeviceFlags,
                                    NULL,
                                    0,
                                    D3D11_SDK_VERSION,
                                    &mD3DDevice,
                                    &mFeatureLevel,
                                    &mD3DDeviceContext);

   if(FAILED(hres))
   {
      #ifdef TORQUE_DEBUG
         //try again without debug device layer enabled
         createDeviceFlags &= ~D3D11_CREATE_DEVICE_DEBUG;
         hres = D3D11CreateDevice(NULL,
                                  driverType,
                                  NULL,
                                  createDeviceFlags,
                                  NULL,
                                  0,
                                  D3D11_SDK_VERSION,
                                  &mD3DDevice,
                                  &mFeatureLevel,
                                  &mD3DDeviceContext);
         //if we failed again than we definitely have a problem
         if (FAILED(hres))
            AssertFatal(false, "GFXD3D11Device::init - D3D11CreateDeviceAndSwapChain failed!");

         Con::warnf("GFXD3D11Device::init - Debug layers not detected!");
         mDebugLayers = false;
      #else
         AssertFatal(false, "GFXD3D11Device::init - D3D11CreateDeviceAndSwapChain failed!");
      #endif
   }

#ifdef TORQUE_DEBUG
   _suppressDebugMessages();
#endif

   mTextureManager = new GFXD3D11TextureManager();

   // Now reacquire all the resources we trashed earlier
   reacquireDefaultPoolResources();
   //set vert/pixel shader targets
   switch (mFeatureLevel)
   {
   case D3D_FEATURE_LEVEL_11_0:
      mVertexShaderTarget = "vs_5_0";
      mPixelShaderTarget = "ps_5_0";
      mPixVersion = 5.0f;
      mShaderModel = "50";
      break;
   case D3D_FEATURE_LEVEL_10_1:
      mVertexShaderTarget = "vs_4_1";
      mPixelShaderTarget = "ps_4_1";
      mPixVersion = 4.1f;
      mShaderModel = "41";
      break;
   case D3D_FEATURE_LEVEL_10_0:
      mVertexShaderTarget = "vs_4_0";
      mPixelShaderTarget = "ps_4_0";
      mPixVersion = 4.0f;
      mShaderModel = "40";
      break;
   default:
      AssertFatal(false, "GFXD3D11Device::init - We don't support this feature level");
   }

   D3D11_QUERY_DESC queryDesc;
   queryDesc.Query = D3D11_QUERY_OCCLUSION;
   queryDesc.MiscFlags = 0;

   ID3D11Query *testQuery = NULL;

   // detect occlusion query support
   if (SUCCEEDED(mD3DDevice->CreateQuery(&queryDesc, &testQuery))) mOcclusionQuerySupported = true;

   SAFE_RELEASE(testQuery);

   Con::printf("Hardware occlusion query detected: %s", mOcclusionQuerySupported ? "Yes" : "No");
   
   mCardProfiler = new GFXD3D11CardProfiler();
   mCardProfiler->init();

   gScreenShot = new ScreenShotD3D11;

   mInitialized = true;
   deviceInited();
}

// Supress any debug layer messages we don't want to see
void GFXD3D11Device::_suppressDebugMessages()
{
   if (mDebugLayers)
   {
      ID3D11Debug *pDebug = NULL;
      if (SUCCEEDED(mD3DDevice->QueryInterface(__uuidof(ID3D11Debug), (void**)&pDebug)))
      {
         ID3D11InfoQueue *pInfoQueue = NULL;
         if (SUCCEEDED(pDebug->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&pInfoQueue)))
         {
            //Disable breaking on error or corruption, this can be handy when using VS graphics debugging
            pInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, false);
            pInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, false);

            D3D11_MESSAGE_ID hide[] =
            {
               //this is harmless and no need to spam the console
               D3D11_MESSAGE_ID_QUERY_BEGIN_ABANDONING_PREVIOUS_RESULTS
            };

            D3D11_INFO_QUEUE_FILTER filter;
            memset(&filter, 0, sizeof(filter));
            filter.DenyList.NumIDs = _countof(hide);
            filter.DenyList.pIDList = hide;
            pInfoQueue->AddStorageFilterEntries(&filter);
            SAFE_RELEASE(pInfoQueue);
         }
         SAFE_RELEASE(pDebug);
      }
   }
}

bool GFXD3D11Device::beginSceneInternal() 
{
   mCanCurrentlyRender = true;
   return mCanCurrentlyRender;  
}

GFXWindowTarget * GFXD3D11Device::allocWindowTarget(PlatformWindow *window)
{
   AssertFatal(window,"GFXD3D11Device::allocWindowTarget - no window provided!");

   // Set up a new window target...
   GFXD3D11WindowTarget *gdwt = new GFXD3D11WindowTarget();
   gdwt->mWindow = window;
   gdwt->mSize = window->getClientExtent();
   
   if (!mInitialized)
   {
      gdwt->mSecondaryWindow = false;
      // Allocate the device.
      init(window->getVideoMode(), window);
      gdwt->initPresentationParams();
      gdwt->createSwapChain();
      gdwt->createBuffersAndViews();

      mSwapChain = gdwt->getSwapChain();
      mDeviceBackbuffer = gdwt->getBackBuffer();
      mDeviceDepthStencil = gdwt->getDepthStencil();
      mDeviceBackBufferView = gdwt->getBackBufferView();
      mDeviceDepthStencilView = gdwt->getDepthStencilView();

   }
   else //additional window/s
   {
      gdwt->mSecondaryWindow = true;
      gdwt->initPresentationParams();
      gdwt->createSwapChain();
      gdwt->createBuffersAndViews();
   }
   
   gdwt->registerResourceWithDevice(this);

   return gdwt;
}

GFXTextureTarget* GFXD3D11Device::allocRenderToTextureTarget()
{
   GFXD3D11TextureTarget *targ = new GFXD3D11TextureTarget();
   targ->registerResourceWithDevice(this);

   return targ;
}

void GFXD3D11Device::beginReset()
{
   if (!mD3DDevice)
      return;

   mInitialized = false;

   releaseDefaultPoolResources();

   // Clean up some commonly dangling state. This helps prevents issues with
   // items that are destroyed by the texture manager callbacks and recreated
   // later, but still left bound.
   setVertexBuffer(NULL);
   setPrimitiveBuffer(NULL);
   for (S32 i = 0; i<getNumSamplers(); i++)
      setTexture(i, NULL);

   mD3DDeviceContext->ClearState();

   //release old buffers and views
   SAFE_RELEASE(mDeviceDepthStencilView);
   SAFE_RELEASE(mDeviceBackBufferView);
   SAFE_RELEASE(mDeviceDepthStencil);
   SAFE_RELEASE(mDeviceBackbuffer);
}

void GFXD3D11Device::endReset(GFXD3D11WindowTarget *windowTarget)
{
   //grab new references
   mDeviceBackbuffer = windowTarget->getBackBuffer();
   mDeviceDepthStencil = windowTarget->getDepthStencil();
   mDeviceBackBufferView = windowTarget->getBackBufferView();
   mDeviceDepthStencilView = windowTarget->getDepthStencilView();

   mD3DDeviceContext->OMSetRenderTargets(1, &mDeviceBackBufferView, mDeviceDepthStencilView);

   // Now reacquire all the resources we trashed earlier
   reacquireDefaultPoolResources();
   mInitialized = true;
   // Mark everything dirty and flush to card, for sanity.
   updateStates(true);
}

void GFXD3D11Device::setupGenericShaders(GenericShaderType type)
{
   AssertFatal(type != GSTargetRestore, ""); //not used

   if(mGenericShader[GSColor] == NULL)
   {
      ShaderData *shaderData;
      //shader model 4.0 is enough for the generic shaders
      const char* shaderModel = "4.0";
      shaderData = new ShaderData();
      shaderData->setField("DXVertexShaderFile", "shaders/common/fixedFunction/colorV.hlsl");
      shaderData->setField("DXPixelShaderFile", "shaders/common/fixedFunction/colorP.hlsl");
      shaderData->setField("pixVersion", shaderModel);
      shaderData->registerObject();
      mGenericShader[GSColor] =  shaderData->getShader();
      mGenericShaderBuffer[GSColor] = mGenericShader[GSColor]->allocConstBuffer();
      mModelViewProjSC[GSColor] = mGenericShader[GSColor]->getShaderConstHandle("$modelView");
      Sim::getRootGroup()->addObject(shaderData);

      shaderData = new ShaderData();
      shaderData->setField("DXVertexShaderFile", "shaders/common/fixedFunction/modColorTextureV.hlsl");
      shaderData->setField("DXPixelShaderFile", "shaders/common/fixedFunction/modColorTextureP.hlsl");
      shaderData->setField("pixVersion", shaderModel);
      shaderData->registerObject();
      mGenericShader[GSModColorTexture] = shaderData->getShader();
      mGenericShaderBuffer[GSModColorTexture] = mGenericShader[GSModColorTexture]->allocConstBuffer();
      mModelViewProjSC[GSModColorTexture] = mGenericShader[GSModColorTexture]->getShaderConstHandle("$modelView");
      Sim::getRootGroup()->addObject(shaderData);

      shaderData = new ShaderData();
      shaderData->setField("DXVertexShaderFile", "shaders/common/fixedFunction/addColorTextureV.hlsl");
      shaderData->setField("DXPixelShaderFile", "shaders/common/fixedFunction/addColorTextureP.hlsl");
      shaderData->setField("pixVersion", shaderModel);
      shaderData->registerObject();
      mGenericShader[GSAddColorTexture] = shaderData->getShader();
      mGenericShaderBuffer[GSAddColorTexture] = mGenericShader[GSAddColorTexture]->allocConstBuffer();
      mModelViewProjSC[GSAddColorTexture] = mGenericShader[GSAddColorTexture]->getShaderConstHandle("$modelView");
      Sim::getRootGroup()->addObject(shaderData);

      shaderData = new ShaderData();
      shaderData->setField("DXVertexShaderFile", "shaders/common/fixedFunction/textureV.hlsl");
      shaderData->setField("DXPixelShaderFile", "shaders/common/fixedFunction/textureP.hlsl");
      shaderData->setField("pixVersion", shaderModel);
      shaderData->registerObject();
      mGenericShader[GSTexture] = shaderData->getShader();
      mGenericShaderBuffer[GSTexture] = mGenericShader[GSTexture]->allocConstBuffer();
      mModelViewProjSC[GSTexture] = mGenericShader[GSTexture]->getShaderConstHandle("$modelView");
      Sim::getRootGroup()->addObject(shaderData);

      //Force an update
      mViewportDirty = true;
      _updateRenderTargets();
   }

   MatrixF tempMatrix =  mProjectionMatrix * mViewMatrix * mWorldMatrix[mWorldStackSize];  
   mGenericShaderBuffer[type]->setSafe(mModelViewProjSC[type], tempMatrix);

   setShader(mGenericShader[type]);
   setShaderConstBuffer(mGenericShaderBuffer[type]);
}

//-----------------------------------------------------------------------------
/// Creates a state block object based on the desc passed in.  This object
/// represents an immutable state.
GFXStateBlockRef GFXD3D11Device::createStateBlockInternal(const GFXStateBlockDesc& desc)
{
   return GFXStateBlockRef(new GFXD3D11StateBlock(desc));
}

/// Activates a stateblock
void GFXD3D11Device::setStateBlockInternal(GFXStateBlock* block, bool force)
{
   AssertFatal(static_cast<GFXD3D11StateBlock*>(block), "Incorrect stateblock type for this device!");
   GFXD3D11StateBlock* d3dBlock = static_cast<GFXD3D11StateBlock*>(block);
   GFXD3D11StateBlock* d3dCurrent = static_cast<GFXD3D11StateBlock*>(mCurrentStateBlock.getPointer());

   if (force)
      d3dCurrent = NULL;

   d3dBlock->activate(d3dCurrent);   
}

/// Called by base GFXDevice to actually set a const buffer
void GFXD3D11Device::setShaderConstBufferInternal(GFXShaderConstBuffer* buffer)
{
   if (buffer)
   {
      PROFILE_SCOPE(GFXD3D11Device_setShaderConstBufferInternal);
      AssertFatal(static_cast<GFXD3D11ShaderConstBuffer*>(buffer), "Incorrect shader const buffer type for this device!");
      GFXD3D11ShaderConstBuffer* d3dBuffer = static_cast<GFXD3D11ShaderConstBuffer*>(buffer);

      d3dBuffer->activate(mCurrentConstBuffer);
      mCurrentConstBuffer = d3dBuffer;
   }
   else
   {
      mCurrentConstBuffer = NULL;
   }
}

//-----------------------------------------------------------------------------

void GFXD3D11Device::clear(U32 flags, ColorI color, F32 z, U32 stencil)
{
   // Make sure we have flushed our render target state.
   _updateRenderTargets();

   UINT depthstencilFlag = 0;

   ID3D11RenderTargetView* rtView = NULL;
   ID3D11DepthStencilView* dsView = NULL;

   mD3DDeviceContext->OMGetRenderTargets(1, &rtView, &dsView);

   const FLOAT clearColor[4] = {
      static_cast<F32>(color.red) * (1.0f / 255.0f),
      static_cast<F32>(color.green) * (1.0f / 255.0f),
      static_cast<F32>(color.blue) * (1.0f / 255.0f),
      static_cast<F32>(color.alpha) * (1.0f / 255.0f)
   };

   if (flags & GFXClearTarget && rtView)
      mD3DDeviceContext->ClearRenderTargetView(rtView, clearColor);

   if (flags & GFXClearZBuffer)
      depthstencilFlag |= D3D11_CLEAR_DEPTH;

   if (flags & GFXClearStencil)
      depthstencilFlag |= D3D11_CLEAR_STENCIL;

   if (depthstencilFlag && dsView)
      mD3DDeviceContext->ClearDepthStencilView(dsView, depthstencilFlag, z, stencil);

   SAFE_RELEASE(rtView);
   SAFE_RELEASE(dsView);
}

void GFXD3D11Device::endSceneInternal() 
{
   mCanCurrentlyRender = false;
}

void GFXD3D11Device::_updateRenderTargets()
{
   if (mRTDirty || (mCurrentRT && mCurrentRT->isPendingState()))
   {
      if (mRTDeactivate)
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

   if (mViewportDirty)
   {
      D3D11_VIEWPORT viewport;

      viewport.TopLeftX = mViewport.point.x;
      viewport.TopLeftY = mViewport.point.y;
      viewport.Width = mViewport.extent.x;
      viewport.Height = mViewport.extent.y;
      viewport.MinDepth   = 0.0f;
      viewport.MaxDepth   = 1.0f;

      mD3DDeviceContext->RSSetViewports(1, &viewport);

      mViewportDirty = false;
   }
}

void GFXD3D11Device::releaseDefaultPoolResources() 
{
   // Release all the dynamic vertex buffer arrays
   // Forcibly clean up the pools
   for(U32 i=0; i<mVolatileVBList.size(); i++)
   {
      SAFE_RELEASE(mVolatileVBList[i]->vb);
      mVolatileVBList[i] = NULL;
   }
   mVolatileVBList.setSize(0);

   // We gotta clear the current const buffer else the next
   // activate may erroneously think the device is still holding
   // this state and fail to set it.   
   mCurrentConstBuffer = NULL;

   // Set current VB to NULL and set state dirty
   for (U32 i=0; i < VERTEX_STREAM_COUNT; i++)
   {
      mCurrentVertexBuffer[i] = NULL;
      mVertexBufferDirty[i] = true;
      mVertexBufferFrequency[i] = 0;
      mVertexBufferFrequencyDirty[i] = true;
   }

   // Release dynamic index buffer
   if(mDynamicPB != NULL)
   {
      SAFE_RELEASE(mDynamicPB->ib);
   }

   // Set current PB/IB to NULL and set state dirty
   mCurrentPrimitiveBuffer = NULL;
   mCurrentPB = NULL;
   mPrimitiveBufferDirty = true;

   // Zombify texture manager (for D3D this only modifies default pool textures)
   if( mTextureManager ) 
      mTextureManager->zombify();

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

void GFXD3D11Device::reacquireDefaultPoolResources() 
{
   // Now do the dynamic index buffers
   if( mDynamicPB == NULL )
      mDynamicPB = new GFXD3D11PrimitiveBuffer(this, 0, 0, GFXBufferTypeDynamic);

   D3D11_BUFFER_DESC desc;
   desc.ByteWidth = sizeof(U16) * MAX_DYNAMIC_INDICES;
   desc.Usage = D3D11_USAGE_DYNAMIC;
   desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
   desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
   desc.MiscFlags = 0;
   desc.StructureByteStride = 0;

   HRESULT hr = D3D11DEVICE->CreateBuffer(&desc, NULL, &mDynamicPB->ib);

   if(FAILED(hr)) 
   {
      AssertFatal(false, "Failed to allocate dynamic IB");
   }

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

GFXD3D11VertexBuffer* GFXD3D11Device::findVBPool( const GFXVertexFormat *vertexFormat, U32 vertsNeeded )
{
   PROFILE_SCOPE( GFXD3D11Device_findVBPool );

   for( U32 i=0; i<mVolatileVBList.size(); i++ )
      if( mVolatileVBList[i]->mVertexFormat.isEqual( *vertexFormat ) )
         return mVolatileVBList[i];

   return NULL;
}

GFXD3D11VertexBuffer * GFXD3D11Device::createVBPool( const GFXVertexFormat *vertexFormat, U32 vertSize )
{
   PROFILE_SCOPE( GFXD3D11Device_createVBPool );

   // this is a bit funky, but it will avoid problems with (lack of) copy constructors
   //    with a push_back() situation
   mVolatileVBList.increment();
   StrongRefPtr<GFXD3D11VertexBuffer> newBuff;
   mVolatileVBList.last() = new GFXD3D11VertexBuffer();
   newBuff = mVolatileVBList.last();

   newBuff->mNumVerts   = 0;
   newBuff->mBufferType = GFXBufferTypeVolatile;
   newBuff->mVertexFormat.copy( *vertexFormat );
   newBuff->mVertexSize = vertSize;
   newBuff->mDevice = this;

   // Requesting it will allocate it.
   vertexFormat->getDecl(); 

   D3D11_BUFFER_DESC desc;
   desc.ByteWidth = vertSize * MAX_DYNAMIC_VERTS;
   desc.Usage = D3D11_USAGE_DYNAMIC;
   desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
   desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
   desc.MiscFlags = 0;
   desc.StructureByteStride = 0;

   HRESULT hr = D3D11DEVICE->CreateBuffer(&desc, NULL, &newBuff->vb);

   if(FAILED(hr)) 
   {
      AssertFatal(false, "Failed to allocate dynamic VB");
   }

   return newBuff;
}

//-----------------------------------------------------------------------------

void GFXD3D11Device::setClipRect( const RectI &inRect ) 
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

void GFXD3D11Device::setVertexStream( U32 stream, GFXVertexBuffer *buffer )
{
   GFXD3D11VertexBuffer *d3dBuffer = static_cast<GFXD3D11VertexBuffer*>( buffer );

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

   U32 strides[1] = { d3dBuffer ? d3dBuffer->mVertexSize : 0 };
   U32 offset = d3dBuffer && stream != 0 ? d3dBuffer->mVolatileStart * d3dBuffer->mVertexSize : 0;
   ID3D11Buffer* buff = d3dBuffer ? d3dBuffer->vb : NULL;

   getDeviceContext()->IASetVertexBuffers(stream, 1, &buff, strides, &offset);
}

void GFXD3D11Device::setVertexStreamFrequency( U32 stream, U32 frequency )
{
   if (stream == 0)
      mDrawInstancesCount = frequency; // instances count
}

void GFXD3D11Device::_setPrimitiveBuffer( GFXPrimitiveBuffer *buffer ) 
{
   mCurrentPB = static_cast<GFXD3D11PrimitiveBuffer *>( buffer );

   mD3DDeviceContext->IASetIndexBuffer(mCurrentPB->ib, DXGI_FORMAT_R16_UINT, 0);
}

U32 GFXD3D11Device::primCountToIndexCount(GFXPrimitiveType primType, U32 primitiveCount)
{
   switch (primType)
   {
   case GFXPointList:
      return primitiveCount;
      break;
   case GFXLineList:
      return primitiveCount * 2;
      break;
   case GFXLineStrip:
      return primitiveCount + 1;
      break;
   case GFXTriangleList:
      return primitiveCount * 3;
      break;
   case GFXTriangleStrip:
      return 2 + primitiveCount;
      break;
   default:
      AssertFatal(false, "GFXGLDevice::primCountToIndexCount - unrecognized prim type");
      break;

   }
   return 0;
}


void GFXD3D11Device::drawPrimitive( GFXPrimitiveType primType, U32 vertexStart, U32 primitiveCount ) 
{
   // This is done to avoid the function call overhead if possible
   if( mStateDirty )
      updateStates();
   if (mCurrentShaderConstBuffer)
      setShaderConstBufferInternal(mCurrentShaderConstBuffer);

   if ( mVolatileVB )
       vertexStart += mVolatileVB->mVolatileStart;

   mD3DDeviceContext->IASetPrimitiveTopology(GFXD3D11PrimType[primType]);
   
   if ( mDrawInstancesCount )
      mD3DDeviceContext->DrawInstanced(primCountToIndexCount(primType, primitiveCount), mDrawInstancesCount, vertexStart, 0);
   else
      mD3DDeviceContext->Draw(primCountToIndexCount(primType, primitiveCount), vertexStart);
  
   mDeviceStatistics.mDrawCalls++;
   if ( mVertexBufferFrequency[0] > 1 )
      mDeviceStatistics.mPolyCount += primitiveCount * mVertexBufferFrequency[0];
   else
      mDeviceStatistics.mPolyCount += primitiveCount;
}

void GFXD3D11Device::drawIndexedPrimitive( GFXPrimitiveType primType, 
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

   mD3DDeviceContext->IASetPrimitiveTopology(GFXD3D11PrimType[primType]);
  
   if ( mDrawInstancesCount )
      mD3DDeviceContext->DrawIndexedInstanced(primCountToIndexCount(primType, primitiveCount), mDrawInstancesCount, mCurrentPB->mVolatileStart + startIndex, startVertex, 0);
   else
      mD3DDeviceContext->DrawIndexed(primCountToIndexCount(primType,primitiveCount), mCurrentPB->mVolatileStart + startIndex, startVertex);   

   mDeviceStatistics.mDrawCalls++;
   if ( mVertexBufferFrequency[0] > 1 )
      mDeviceStatistics.mPolyCount += primitiveCount * mVertexBufferFrequency[0];
   else
      mDeviceStatistics.mPolyCount += primitiveCount;
}

GFXShader* GFXD3D11Device::createShader()
{
   GFXD3D11Shader* shader = new GFXD3D11Shader();
   shader->registerResourceWithDevice( this );
   return shader;
}

//-----------------------------------------------------------------------------
// Set shader - this function exists to make sure this is done in one place,
//              and to make sure redundant shader states are not being
//              sent to the card.
//-----------------------------------------------------------------------------
void GFXD3D11Device::setShader(GFXShader *shader, bool force)
{
   if(shader)
   {
      GFXD3D11Shader *d3dShader = static_cast<GFXD3D11Shader*>(shader);

      if (d3dShader->mPixShader != mLastPixShader || force)
      {
        mD3DDeviceContext->PSSetShader( d3dShader->mPixShader, NULL, 0);
        mLastPixShader = d3dShader->mPixShader;
      }

      if (d3dShader->mVertShader != mLastVertShader || force)
      {
        mD3DDeviceContext->VSSetShader( d3dShader->mVertShader, NULL, 0);
        mLastVertShader = d3dShader->mVertShader;
      }     
   }
   else
   {
      setupGenericShaders();
   }
}

GFXPrimitiveBuffer * GFXD3D11Device::allocPrimitiveBuffer(U32 numIndices, U32 numPrimitives, GFXBufferType bufferType, void *data )
{
   // Allocate a buffer to return
   GFXD3D11PrimitiveBuffer * res = new GFXD3D11PrimitiveBuffer(this, numIndices, numPrimitives, bufferType);

   // Determine usage flags
   D3D11_USAGE usage = D3D11_USAGE_DEFAULT;

   // Assumptions:
   //    - static buffers are write once, use many
   //    - dynamic buffers are write many, use many
   //    - volatile buffers are write once, use once
   // You may never read from a buffer.
   //TODO: enable proper support for D3D11_USAGE_IMMUTABLE
   switch(bufferType)
   {
   case GFXBufferTypeImmutable:
   case GFXBufferTypeStatic:
      usage = D3D11_USAGE_DEFAULT; //D3D11_USAGE_IMMUTABLE;
      break;

   case GFXBufferTypeDynamic:
   case GFXBufferTypeVolatile:
     usage = D3D11_USAGE_DYNAMIC;
      break;
   }

   // Register resource
   res->registerResourceWithDevice(this);

   // Create d3d index buffer
   if(bufferType == GFXBufferTypeVolatile)
   {
        // Get it from the pool if it's a volatile...
        AssertFatal(numIndices < MAX_DYNAMIC_INDICES, "Cannot allocate that many indices in a volatile buffer, increase MAX_DYNAMIC_INDICES.");

        res->ib = mDynamicPB->ib;
        res->mVolatileBuffer = mDynamicPB;
   }
   else
   {
      // Otherwise, get it as a seperate buffer...
      D3D11_BUFFER_DESC desc;
      desc.ByteWidth = sizeof(U16) * numIndices;
      desc.Usage = usage;
      if(bufferType == GFXBufferTypeDynamic)
         desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; // We never allow reading from a primitive buffer.
      else
         desc.CPUAccessFlags = 0;
      desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
      desc.MiscFlags = 0;
      desc.StructureByteStride = 0;

      HRESULT hr = D3D11DEVICE->CreateBuffer(&desc, NULL, &res->ib);

      if(FAILED(hr)) 
      {
         AssertFatal(false, "Failed to allocate an index buffer.");
      }
   }

   if (data)
   {
      void* dest;
      res->lock(0, numIndices, &dest);
      dMemcpy(dest, data, sizeof(U16) * numIndices);
      res->unlock();
   }

   return res;
}

GFXVertexBuffer * GFXD3D11Device::allocVertexBuffer(U32 numVerts, const GFXVertexFormat *vertexFormat, U32 vertSize, GFXBufferType bufferType, void *data)
{
   PROFILE_SCOPE( GFXD3D11Device_allocVertexBuffer );

   GFXD3D11VertexBuffer *res = new GFXD3D11VertexBuffer(   this, 
                                                         numVerts, 
                                                         vertexFormat, 
                                                         vertSize, 
                                                         bufferType );
   
   // Determine usage flags
   D3D11_USAGE usage = D3D11_USAGE_DEFAULT;

   res->mNumVerts = 0;

   // Assumptions:
   //    - static buffers are write once, use many
   //    - dynamic buffers are write many, use many
   //    - volatile buffers are write once, use once
   // You may never read from a buffer.
   //TODO: enable proper support for D3D11_USAGE_IMMUTABLE
   switch(bufferType)
   {
   case GFXBufferTypeImmutable:
   case GFXBufferTypeStatic:
      usage = D3D11_USAGE_DEFAULT;
      break;

   case GFXBufferTypeDynamic:
   case GFXBufferTypeVolatile:
     usage = D3D11_USAGE_DYNAMIC;
      break;
   }

   // Register resource
   res->registerResourceWithDevice(this);

   // Create vertex buffer
   if(bufferType == GFXBufferTypeVolatile)
   {
        // NOTE: Volatile VBs are pooled and will be allocated at lock time.
        AssertFatal(numVerts <= MAX_DYNAMIC_VERTS, "GFXD3D11Device::allocVertexBuffer - Volatile vertex buffer is too big... see MAX_DYNAMIC_VERTS!");
   }
   else
   {
      // Requesting it will allocate it.
      vertexFormat->getDecl(); //-ALEX disabled to postpone until after shader is actually set...

      // Get a new buffer...
      D3D11_BUFFER_DESC desc;
      desc.ByteWidth = vertSize * numVerts;
      desc.Usage = usage;
      desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
      if(bufferType == GFXBufferTypeDynamic)
         desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; // We never allow reading from a vertex buffer.
      else
         desc.CPUAccessFlags = 0;
      desc.MiscFlags = 0;
      desc.StructureByteStride = 0;

      HRESULT hr = D3D11DEVICE->CreateBuffer(&desc, NULL, &res->vb);

      if(FAILED(hr)) 
      {
         AssertFatal(false, "Failed to allocate VB");
      }
   }

   res->mNumVerts = numVerts;

   if (data)
   {
      void* dest;
      res->lock(0, numVerts, &dest);
      dMemcpy(dest, data, vertSize * numVerts);
      res->unlock();
   }

   return res;
}

String GFXD3D11Device::_createTempShaderInternal(const GFXVertexFormat *vertexFormat)
{
   U32 elemCount = vertexFormat->getElementCount();
   //Input data
   StringBuilder inputData;
   inputData.append("struct VertIn {");
   //Output data
   StringBuilder outputData;
   outputData.append("struct VertOut {");
   // Shader main body data
   StringBuilder mainBodyData;
   //make shader
   mainBodyData.append("VertOut main(VertIn IN){VertOut OUT;");

   bool addedPadding = false;
   for (U32 i = 0; i < elemCount; i++)
   {
      const GFXVertexElement &element = vertexFormat->getElement(i);
      String semantic = element.getSemantic();
      String semanticOut = semantic;
      String type;

      AssertFatal(!(addedPadding && !element.isSemantic(GFXSemantic::PADDING)), "Padding added before data");

      if (element.isSemantic(GFXSemantic::POSITION))
      {
         semantic = "POSITION";
         semanticOut = "SV_Position";
      }
      else if (element.isSemantic(GFXSemantic::NORMAL))
      {
         semantic = "NORMAL";
         semanticOut = semantic;
      }
      else if (element.isSemantic(GFXSemantic::COLOR))
      {
         semantic = "COLOR";
         semanticOut = semantic;
      }
      else if (element.isSemantic(GFXSemantic::TANGENT))
      {
         semantic = "TANGENT";
         semanticOut = semantic;
      }
      else if (element.isSemantic(GFXSemantic::BINORMAL))
      {
         semantic = "BINORMAL";
         semanticOut = semantic;
      }
      else if (element.isSemantic(GFXSemantic::BLENDINDICES))
      {
         semantic = String::ToString("BLENDINDICES%d", element.getSemanticIndex());
         semanticOut = semantic;
      }
      else if (element.isSemantic(GFXSemantic::BLENDWEIGHT))
      {
         semantic = String::ToString("BLENDWEIGHT%d", element.getSemanticIndex());
         semanticOut = semantic;
      }
      else if (element.isSemantic(GFXSemantic::PADDING))
      {
         addedPadding = true;
         continue;
      }
      else
      {
         //Anything that falls thru to here will be a texture coord.
         semantic = String::ToString("TEXCOORD%d", element.getSemanticIndex());
         semanticOut = semantic;
      }

      switch (GFXD3D11DeclType[element.getType()])
      {
      case DXGI_FORMAT_R32_FLOAT:
         type = "float";
         break;
      case DXGI_FORMAT_R32G32_FLOAT:
         type = "float2";
         break;
      case DXGI_FORMAT_R32G32B32_FLOAT:
         type = "float3";
         break;
      case DXGI_FORMAT_R32G32B32A32_FLOAT:
      case DXGI_FORMAT_B8G8R8A8_UNORM:
      case DXGI_FORMAT_R8G8B8A8_UNORM:
         type = "float4";
         break;
      case DXGI_FORMAT_R8G8B8A8_UINT:
         type = "uint4";
         break;
      }

      StringBuilder in;
      in.format("%s %s%d : %s;", type.c_str(), "var", i, semantic.c_str());
      inputData.append(in.data());

      //SV_Position must be float4
      if (semanticOut == String("SV_Position"))
      {
         StringBuilder out;
         out.format("float4 %s%d : %s;", "var", i, semanticOut.c_str());
         outputData.append(out.data());
         StringBuilder body;
         body.format("OUT.%s%d = float4(IN.%s%d.xyz,1);", "var", i, "var", i);
         mainBodyData.append(body.data());
      }
      else
      {
         StringBuilder out;
         out.format("%s %s%d : %s;", type.c_str(), "var", i, semanticOut.c_str());
         outputData.append(out.data());
         StringBuilder body;
         body.format("OUT.%s%d = IN.%s%d;", "var", i, "var", i);
         mainBodyData.append(body.data());
      }
   }

   inputData.append("};");
   outputData.append("};");
   mainBodyData.append("return OUT;}");

   //final data
   StringBuilder finalData;
   finalData.append(inputData.data());
   finalData.append(outputData.data());
   finalData.append(mainBodyData.data());

   return String(finalData.data());
}

GFXVertexDecl* GFXD3D11Device::allocVertexDecl( const GFXVertexFormat *vertexFormat )
{
   PROFILE_SCOPE( GFXD3D11Device_allocVertexDecl );

   // First check the map... you shouldn't allocate VBs very often
   // if you want performance.  The map lookup should never become
   // a performance bottleneck.
   D3D11VertexDecl *decl = mVertexDecls[vertexFormat->getDescription()];
   if ( decl )
      return decl;

   U32 elemCount = vertexFormat->getElementCount();
 
   ID3DBlob* code = NULL;
  
   // We have to generate a temporary shader here for now since the input layout creation
   // expects a shader to be already compiled to verify the vertex layout structure. The problem
   // is that most of the time the regular shaders are compiled AFTER allocVertexDecl is called.
   if(!decl)
   {
      //TODO: Perhaps save/cache the ID3DBlob for later use on identical vertex formats,save creating/compiling the temp shader everytime
      String shaderData = _createTempShaderInternal(vertexFormat);     

#ifdef TORQUE_DEBUG
      U32 flags = D3DCOMPILE_DEBUG | D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_WARNINGS_ARE_ERRORS;
#else
      U32 flags = D3DCOMPILE_ENABLE_STRICTNESS;
#endif

      ID3DBlob *errorBlob = NULL;
      HRESULT hr = D3DCompile(shaderData.c_str(), shaderData.length(), NULL, NULL, NULL, "main", "vs_5_0", flags, 0, &code, &errorBlob);
      StringBuilder error;

      if(errorBlob)
      {
         error.append((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize());
         AssertFatal(hr, error.data());
      }

      SAFE_RELEASE(errorBlob);
   }
   
   AssertFatal(code, "D3D11Device::allocVertexDecl - compiled vert shader code missing!");

   // Setup the declaration struct.
   
   U32 stream;
   D3D11_INPUT_ELEMENT_DESC *vd = new D3D11_INPUT_ELEMENT_DESC[ elemCount];

   S32 elemIndex = 0;
   for (S32 i = 0; i < elemCount; i++, elemIndex++)
   {
      const GFXVertexElement &element = vertexFormat->getElement(elemIndex);

      stream = element.getStreamIndex();

      vd[i].InputSlot = stream;

      vd[i].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
      vd[i].Format = GFXD3D11DeclType[element.getType()];
      // If instancing is enabled, the per instance data is only used on stream 1.
      if (vertexFormat->hasInstancing() && stream == 1)
      {
         vd[i].InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
         vd[i].InstanceDataStepRate = 1;
      }
      else
      {
         vd[i].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
         vd[i].InstanceDataStepRate = 0;
      }
      // We force the usage index of 0 for everything but 
      // texture coords for now... this may change later.
      vd[i].SemanticIndex = 0;

      if (element.isSemantic(GFXSemantic::POSITION))
         vd[i].SemanticName = "POSITION";
      else if (element.isSemantic(GFXSemantic::NORMAL))
         vd[i].SemanticName = "NORMAL";
      else if (element.isSemantic(GFXSemantic::COLOR))
         vd[i].SemanticName = "COLOR";
      else if (element.isSemantic(GFXSemantic::TANGENT))
         vd[i].SemanticName = "TANGENT";
      else if (element.isSemantic(GFXSemantic::BINORMAL))
         vd[i].SemanticName = "BINORMAL";
      else if (element.isSemantic(GFXSemantic::BLENDWEIGHT))
      {
         vd[i].SemanticName = "BLENDWEIGHT";
         vd[i].SemanticIndex = element.getSemanticIndex();
      }
      else if (element.isSemantic(GFXSemantic::BLENDINDICES))
      {
         vd[i].SemanticName = "BLENDINDICES";
         vd[i].SemanticIndex = element.getSemanticIndex();
      }
      else if (element.isSemantic(GFXSemantic::PADDING))
      {
         i--;
         elemCount--;
         continue;
      }
      else
      {
          //Anything that falls thru to here will be a texture coord.
         vd[i].SemanticName = "TEXCOORD";
         vd[i].SemanticIndex = element.getSemanticIndex();
      }

   }

   decl = new D3D11VertexDecl();
   HRESULT hr = mD3DDevice->CreateInputLayout(vd, elemCount,code->GetBufferPointer(), code->GetBufferSize(), &decl->decl);
   
   if (FAILED(hr))
   {
      AssertFatal(false, "GFXD3D11Device::allocVertexDecl - Failed to create vertex input layout!");
   }

   delete [] vd;
   SAFE_RELEASE(code);

   // Store it in the cache.
   mVertexDecls[vertexFormat->getDescription()] = decl;

   return decl;
}

void GFXD3D11Device::setVertexDecl( const GFXVertexDecl *decl )
{
   ID3D11InputLayout *dx11Decl = NULL;
   if (decl)
      dx11Decl = static_cast<const D3D11VertexDecl*>(decl)->decl;
   
   mD3DDeviceContext->IASetInputLayout(dx11Decl);
}

//-----------------------------------------------------------------------------
// This function should ONLY be called from GFXDevice::updateStates() !!!
//-----------------------------------------------------------------------------
void GFXD3D11Device::setTextureInternal( U32 textureUnit, const GFXTextureObject *texture)
{
   if( texture == NULL )
   {
      ID3D11ShaderResourceView *pView = NULL;
      mD3DDeviceContext->PSSetShaderResources(textureUnit, 1, &pView);
      return;
   }

   GFXD3D11TextureObject  *tex = (GFXD3D11TextureObject*)(texture);
   mD3DDeviceContext->PSSetShaderResources(textureUnit, 1, tex->getSRViewPtr());
}

GFXFence *GFXD3D11Device::createFence()
{
   // Figure out what fence type we should be making if we don't know
   if( mCreateFenceType == -1 )
   {
     D3D11_QUERY_DESC desc;
     desc.MiscFlags = 0;
     desc.Query = D3D11_QUERY_EVENT;

     ID3D11Query *testQuery = NULL;

     HRESULT hRes = mD3DDevice->CreateQuery(&desc, &testQuery);

     if(FAILED(hRes))
     {
        mCreateFenceType = true;
     }

     else
     {
        mCreateFenceType = false;
     }

      SAFE_RELEASE(testQuery);
   }

   // Cool, use queries
   if(!mCreateFenceType)
   {
      GFXFence* fence = new GFXD3D11QueryFence( this );
      fence->registerResourceWithDevice(this);
      return fence;
   }

   // CodeReview: At some point I would like a specialized implementation of
   // the method used by the general fence, only without the overhead incurred 
   // by using the GFX constructs. Primarily the lock() method on texture handles
   // will do a data copy, and this method doesn't require a copy, just a lock
   // [5/10/2007 Pat]
   GFXFence* fence = new GFXGeneralFence( this );
   fence->registerResourceWithDevice(this);
   return fence;
}

GFXOcclusionQuery* GFXD3D11Device::createOcclusionQuery()
{  
   GFXOcclusionQuery *query;
   if (mOcclusionQuerySupported)
      query = new GFXD3D11OcclusionQuery( this );
   else
      return NULL;      

   query->registerResourceWithDevice(this);
   return query;
}

GFXCubemap * GFXD3D11Device::createCubemap()
{
   GFXD3D11Cubemap* cube = new GFXD3D11Cubemap();
   cube->registerResourceWithDevice(this);
   return cube;
}

//------------------------------------------------------------------------------
void GFXD3D11Device::enterDebugEvent(ColorI color, const char *name)
{
   // BJGFIX
   WCHAR  eventName[260];
   MultiByteToWideChar(CP_ACP, 0, name, -1, eventName, 260);

   D3DPERF_BeginEvent(D3DCOLOR_ARGB(color.alpha, color.red, color.green, color.blue),
      (LPCWSTR)&eventName);
}

//------------------------------------------------------------------------------
void GFXD3D11Device::leaveDebugEvent()
{
   D3DPERF_EndEvent();
}

//------------------------------------------------------------------------------
void GFXD3D11Device::setDebugMarker(ColorI color, const char *name)
{
   // BJGFIX
   WCHAR  eventName[260];
   MultiByteToWideChar(CP_ACP, 0, name, -1, eventName, 260);

   D3DPERF_SetMarker(D3DCOLOR_ARGB(color.alpha, color.red, color.green, color.blue),
      (LPCWSTR)&eventName);
}