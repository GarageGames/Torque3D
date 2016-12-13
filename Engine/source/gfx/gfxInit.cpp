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
#include "gfx/gfxInit.h"

#include "gfx/gfxTextureManager.h"
#include "gfx/gfxAPI.h"
#include "console/console.h"
#include "windowManager/platformWindowMgr.h"
#include "core/module.h"


Vector<GFXAdapter*> GFXInit::smAdapters( __FILE__, __LINE__ );
GFXInit::RegisterDeviceSignal* GFXInit::smRegisterDeviceSignal;


MODULE_BEGIN( GFX )

   MODULE_INIT
   {
      GFXInit::init();
      if( engineAPI::gUseConsoleInterop )
         GFXDevice::initConsole();
      GFXTextureManager::init();
   }
   
   MODULE_SHUTDOWN
   {
      GFXDevice::destroy();
      GFXInit::cleanup();
   }

MODULE_END;

IMPLEMENT_STATIC_CLASS( GFXInit, GFXAPI,
   "Functions for tracking GFX adapters and initializing them into devices."
);

ConsoleDoc(
   "@class GFXInit\n"
   "@ingroup GFX\n"
   "@brief Functions for tracking GFX adapters and initializing them into devices.\n"
);

inline static void _GFXInitReportAdapters(Vector<GFXAdapter*> &adapters)
{
   for (U32 i = 0; i < adapters.size(); i++)
   {
      switch (adapters[i]->mType)
      {
      case Direct3D9:
         Con::printf("   Direct 3D (version 9.x) device found");
         break;
      case OpenGL:
         Con::printf("   OpenGL device found");
         break;
      case NullDevice:
         Con::printf("   Null device found");
         break;
      case Direct3D11:
         Con::printf("   Direct 3D (version 11.x) device found");
         break;
      default :
         Con::printf("   Unknown device found");
         break;
      }
   }
}

inline static void _GFXInitGetInitialRes(GFXVideoMode &vm, const Point2I &initialSize)
{
   const U32 kDefaultWindowSizeX = 800;
   const U32 kDefaultWindowSizeY = 600;
   const bool kDefaultFullscreen = false;
   const U32 kDefaultBitDepth = 32;
   const U32 kDefaultRefreshRate = 60;

   // cache the desktop size of the main screen
   GFXVideoMode desktopVm = GFXInit::getDesktopResolution();

   // load pref variables, properly choose windowed / fullscreen  
   const String resString = Con::getVariable("$pref::Video::mode");

   // Set defaults into the video mode, then have it parse the user string.
   vm.resolution.x = kDefaultWindowSizeX;
   vm.resolution.y = kDefaultWindowSizeY;
   vm.fullScreen   = kDefaultFullscreen;
   vm.bitDepth     = kDefaultBitDepth;
   vm.refreshRate  = kDefaultRefreshRate;
   vm.wideScreen = false;

   vm.parseFromString(resString);
}

GFXInit::RegisterDeviceSignal& GFXInit::getRegisterDeviceSignal()
{
   if (smRegisterDeviceSignal)
      return *smRegisterDeviceSignal;
   smRegisterDeviceSignal = new RegisterDeviceSignal();
   return *smRegisterDeviceSignal;
}

void GFXInit::init()
{
   // init only once.
   static bool doneOnce = false;
   if(doneOnce)
      return;
   doneOnce = true;
   
   Con::printf( "GFX Init:" );

   //find our adapters
   Vector<GFXAdapter*> adapters( __FILE__, __LINE__ );
   GFXInit::enumerateAdapters();
   GFXInit::getAdapters(&adapters);

   if(!adapters.size())
       Con::errorf("Could not find a display adapter");

   //loop through and tell the user what kind of adapters we found
   _GFXInitReportAdapters(adapters);
   Con::printf( "" );
}

void GFXInit::cleanup()
{
   while( smAdapters.size() )
   {
      GFXAdapter* adapter = smAdapters.last();
      smAdapters.decrement();
      delete adapter;
   }

   if( smRegisterDeviceSignal )
      SAFE_DELETE( smRegisterDeviceSignal );
}

bool GFXInit::compareAdapterOutputDevice(const GFXAdapter* adapter, const char* outputDevice)
{
   // If the adapter doesn't have an output display device, then it supports all of them
   if(!adapter->mOutputName[0])
      return true;

   // Try and match the first part of the output device display name.  For example,
   // an adapter->mOutputName of "\\.\DISPLAY1" might correspond to a display name
   // of "\\.\DISPLAY1\Monitor0".  If two monitors are set up in duplicate mode then
   // they will have the same 'display' part in their display name.
   return (dStrstr(outputDevice, adapter->mOutputName) == outputDevice);
}

GFXAdapter* GFXInit::getAdapterOfType( GFXAdapterType type, const char* outputDevice )
{
   bool testOutputDevice = false;
   if(outputDevice && outputDevice[0])
      testOutputDevice = true;

   for( U32 i = 0; i < smAdapters.size(); i++ )
   {
      if( smAdapters[i]->mType == type )
      {
         if(testOutputDevice)
         {
            // Check if the output display device also matches
            if(compareAdapterOutputDevice(smAdapters[i], outputDevice))
            {
               return smAdapters[i];
            }
         }
         else
         {
            // No need to also test the output display device, so return
            return smAdapters[i];
         }
      }
   }

   return NULL;
}

GFXAdapter* GFXInit::getAdapterOfType(GFXAdapterType type, S32 outputDeviceIndex)
{
   for (U32 i = 0; i < smAdapters.size(); i++)
   {
      if (smAdapters[i]->mType == type)
      {
         if (smAdapters[i]->mIndex == outputDeviceIndex)
         {
            return smAdapters[i];
         }
      }
   }

   return NULL;
}

GFXAdapter* GFXInit::chooseAdapter( GFXAdapterType type, const char* outputDevice)
{
   GFXAdapter* adapter = GFXInit::getAdapterOfType(type, outputDevice);
   
   if(!adapter && type != OpenGL)
   {
      Con::errorf("The requested renderer, %s, doesn't seem to be available."
                  " Trying the default, OpenGL.", getAdapterNameFromType(type));
      adapter = GFXInit::getAdapterOfType(OpenGL, outputDevice);
   }
   
   if(!adapter)
   {
      Con::errorf("The OpenGL renderer doesn't seem to be available. Trying the GFXNulDevice.");
      adapter = GFXInit::getAdapterOfType(NullDevice, "");
   }
   
   AssertFatal( adapter, "There is no rendering device available whatsoever.");
   return adapter;
}

GFXAdapter* GFXInit::chooseAdapter(GFXAdapterType type, S32 outputDeviceIndex)
{
   GFXAdapter* adapter = GFXInit::getAdapterOfType(type, outputDeviceIndex);

   if (!adapter && type != OpenGL)
   {
      Con::errorf("The requested renderer, %s, doesn't seem to be available."
         " Trying the default, OpenGL.", getAdapterNameFromType(type));
      adapter = GFXInit::getAdapterOfType(OpenGL, outputDeviceIndex);
   }

   if (!adapter)
   {
      Con::errorf("The OpenGL renderer doesn't seem to be available. Trying the GFXNulDevice.");
      adapter = GFXInit::getAdapterOfType(NullDevice, 0);
   }

   AssertFatal(adapter, "There is no rendering device available whatsoever.");
   return adapter;
}

const char* GFXInit::getAdapterNameFromType(GFXAdapterType type)
{
   // must match GFXAdapterType order
   static const char* _names[] = { "OpenGL", "D3D11", "D3D9", "NullDevice", "Xenon" };
   
   if( type < 0 || type >= GFXAdapterType_Count )
   {
      Con::errorf( "GFXInit::getAdapterNameFromType - Invalid renderer type, defaulting to OpenGL" );
      return _names[OpenGL];
   }
      
   return _names[type];
}

GFXAdapterType GFXInit::getAdapterTypeFromName(const char* name)
{
   S32 ret = -1;
   for(S32 i = 0; i < GFXAdapterType_Count; i++)
   {
      if( !dStricmp( getAdapterNameFromType((GFXAdapterType)i), name ) )
         ret = i;
   }
   
   if( ret == -1 )
   {
      Con::errorf( "GFXInit::getAdapterTypeFromName - Invalid renderer name, defaulting to D3D9" );
      ret = Direct3D9;
   }
   
   return (GFXAdapterType)ret;
}

GFXAdapter *GFXInit::getBestAdapterChoice()
{
   // Get the user's preference for device...
   const String   renderer   = Con::getVariable("$pref::Video::displayDevice");
   const String   outputDevice = Con::getVariable("$pref::Video::displayOutputDevice");
   const String   adapterDevice = Con::getVariable("$Video::forceDisplayAdapter");

   GFXAdapterType adapterType = getAdapterTypeFromName(renderer.c_str());;
   GFXAdapter     *adapter = NULL;

   if (adapterDevice.isEmpty())
   {
      adapter = chooseAdapter(adapterType, outputDevice.c_str());
   }
   else
   {
     S32 adapterIdx = dAtoi(adapterDevice.c_str());
     if (adapterIdx == -1)
        adapter = chooseAdapter(adapterType, outputDevice.c_str());
     else
        adapter = chooseAdapter(adapterType, adapterIdx);
   }

   // Did they have one? Return it.
   if(adapter)
      return adapter;

   // Didn't have one. So make it up. Find the highest SM available. Prefer
   // D3D to GL because if we have a D3D device at all we're on windows,
   // and in an unknown situation on Windows D3D is probably the safest bet.
   //
   // If D3D is unavailable, we're not on windows, so GL is de facto the
   // best choice!
   F32 highestSMDX = 0.f, highestSMGL = 0.f;
   GFXAdapter  *foundAdapter9 = NULL, *foundAdapterGL = NULL, *foundAdapter11 = NULL;

   for (S32 i = 0; i<smAdapters.size(); i++)
   {
      GFXAdapter *currAdapter = smAdapters[i];
      switch (currAdapter->mType)
      {
      case Direct3D11:
         if (currAdapter->mShaderModel > highestSMDX)
         {
            highestSMDX = currAdapter->mShaderModel;
            foundAdapter11 = currAdapter;
         }
         break;

      case Direct3D9:
         if (currAdapter->mShaderModel > highestSMDX)
         {
            highestSMDX = currAdapter->mShaderModel;
            foundAdapter9 = currAdapter;
         }
         break;

      case OpenGL:
         if (currAdapter->mShaderModel > highestSMGL)
         {
            highestSMGL = currAdapter->mShaderModel;
            foundAdapterGL = currAdapter;
         }
         break;

      default:
         break;
      }
   }

   // Return best found in order DX11,DX9, GL
   if (foundAdapter11)
      return foundAdapter11;

   if (foundAdapter9)
      return foundAdapter9;

   if (foundAdapterGL)
      return foundAdapterGL;

   // Uh oh - we didn't find anything. Grab whatever we can that's not Null...
   for(S32 i=0; i<smAdapters.size(); i++)
      if(smAdapters[i]->mType != NullDevice)
         return smAdapters[i];

   // Dare we return a null device? No. Just return NULL.
   return NULL;
}

GFXVideoMode GFXInit::getInitialVideoMode()
{
   GFXVideoMode vm;
   _GFXInitGetInitialRes(vm, Point2I(800,600));
   return vm;
}

S32 GFXInit::getAdapterCount()
{
   return smAdapters.size();
}

void GFXInit::getAdapters(Vector<GFXAdapter*> *adapters)
{
   adapters->clear();
   for (U32 k = 0; k < smAdapters.size(); k++)
      adapters->push_back(smAdapters[k]);
}

GFXVideoMode GFXInit::getDesktopResolution()
{
   GFXVideoMode resVm;

   // Retrieve Resolution Information.
   resVm.bitDepth    = WindowManager->getDesktopBitDepth();
   resVm.resolution  = WindowManager->getDesktopResolution();
   resVm.fullScreen  = false;
   resVm.refreshRate = 60;

   // Return results
   return resVm;
}

void GFXInit::enumerateAdapters() 
{
   // Call each device class and have it report any adapters it supports.
   if(smAdapters.size())
   {
      // CodeReview Seems like this is ok to just ignore? [bjg, 5/19/07]
      //Con::warnf("GFXInit::enumerateAdapters - already have a populated adapter list, aborting re-analysis.");
      return;
   }

   getRegisterDeviceSignal().trigger(GFXInit::smAdapters);     
}

GFXDevice *GFXInit::createDevice( GFXAdapter *adapter ) 
{
   Con::printf("Attempting to create GFX device: %s [%s]", adapter->getName(), adapter->getOutputName());

   GFXDevice* temp = adapter->mCreateDeviceInstanceDelegate(adapter->mIndex);
   if (temp)
   {
      Con::printf("Device created, setting adapter and enumerating modes");
      temp->setAdapter(*adapter);
      temp->enumerateVideoModes();
      temp->getVideoModeList();
   }
   else
      Con::errorf("Failed to create GFX device");

   GFXDevice::getDeviceEventSignal().trigger(GFXDevice::deCreate);

   return temp;
}


DefineEngineFunction( getDesktopResolution, Point3F, (),,
   "Returns the width, height, and bitdepth of the screen/desktop.\n\n@ingroup GFX" )
{
   GFXVideoMode res = GFXInit::getDesktopResolution();
   return Point3F( res.resolution.x, res.resolution.y, res.bitDepth );
}

DefineEngineStaticMethod( GFXInit, getAdapterCount, S32, (),,
   "Return the number of graphics adapters available. @ingroup GFX")
{
   return GFXInit::getAdapterCount();
}

DefineEngineStaticMethod( GFXInit, getAdapterName, String, ( S32 index ),,
   "Returns the name of the graphics adapter.\n"
   "@param index The index of the adapter." )
{
   Vector<GFXAdapter*> adapters( __FILE__, __LINE__ );
   GFXInit::getAdapters(&adapters);

   if(index >= 0 && index < adapters.size())
      return adapters[index]->mName;

   Con::errorf( "GFXInit::getAdapterName - Out of range adapter index." );
   return String::EmptyString;
}

DefineEngineStaticMethod( GFXInit, getAdapterOutputName, String, ( S32 index ),,
   "Returns the name of the graphics adapter's output display device.\n"
   "@param index The index of the adapter." )
{
   Vector<GFXAdapter*> adapters( __FILE__, __LINE__ );
   GFXInit::getAdapters(&adapters);

   if(index >= 0 && index < adapters.size())
      return adapters[index]->mOutputName;

   Con::errorf( "GFXInit::getAdapterOutputName - Out of range adapter index." );
   return String::EmptyString;
}

DefineEngineStaticMethod( GFXInit, getAdapterType, GFXAdapterType, ( S32 index ),,
   "Returns the type (D3D9, D3D8, GL, Null) of a graphics adapter.\n"
   "@param index The index of the adapter." )
{
   Vector<GFXAdapter*> adapters( __FILE__, __LINE__ );
   GFXInit::getAdapters(&adapters);

   if( index >= 0 && index < adapters.size())
      return adapters[index]->mType;

   Con::errorf( "GFXInit::getAdapterType - Out of range adapter index." );
   return GFXAdapterType_Count;
}

DefineEngineStaticMethod( GFXInit, getAdapterShaderModel, F32, ( S32 index ),,
   "Returns the supported shader model of the graphics adapter or -1 if the index is bad.\n"
   "@param index The index of the adapter." )
{
   Vector<GFXAdapter*> adapters( __FILE__, __LINE__ );
   GFXInit::getAdapters(&adapters);

   if(index < 0 || index >= adapters.size())
   {
      Con::errorf("GFXInit::getAdapterShaderModel - Out of range adapter index.");
      return -1.0f;
   }

   return adapters[index]->mShaderModel;
}

DefineEngineStaticMethod( GFXInit, getDefaultAdapterIndex, S32, (),,
   "Returns the index of the default graphics adapter.  This is the graphics device "
   "which will be used to initialize the engine." )
{
   GFXAdapter *a = GFXInit::getBestAdapterChoice();

   // We have to find the index of the adapter in the list to
   // return an index that makes sense to the script.
   Vector<GFXAdapter*> adapters( __FILE__, __LINE__ );
   GFXInit::getAdapters(&adapters);
   for(S32 i=0; i<adapters.size(); i++)
      if (  adapters[i]->mIndex == a->mIndex && 
            adapters[i]->mType == a->mType )
         return i;

   Con::warnf( "GFXInit::getDefaultAdapterIndex - Didn't find the adapter in the list!" );
   return -1;
}

DefineEngineStaticMethod( GFXInit, getAdapterModeCount, S32, ( S32 index ),,
   "Gets the number of modes available on the specified adapter.\n\n"
   "@param index Index of the adapter to get modes from.\n"
   "@return The number of video modes supported by the adapter or -1 if the given adapter was not found." )
{
   Vector<GFXAdapter*> adapters( __FILE__, __LINE__ );
   GFXInit::getAdapters(&adapters);

   if( index < 0 || index >= adapters.size())
   {
      Con::warnf( "GFXInit::getAdapterModeCount - The index was out of range." );
      return -1;
   }

   return adapters[index]->mAvailableModes.size();
}

DefineConsoleStaticMethod( GFXInit, getAdapterMode, String, ( S32 index, S32 modeIndex ),,
   "Gets the details of the specified adapter mode.\n\n"
   "@param index Index of the adapter to query.\n"
   "@param modeIndex Index of the mode to get data from.\n"
   "@return A video mode string in the format 'width height fullscreen bitDepth refreshRate aaLevel'.\n"
   "@see GuiCanvas::getVideoMode()" )
{
   Vector<GFXAdapter*> adapters( __FILE__, __LINE__ );
   GFXInit::getAdapters(&adapters);

   if ( index < 0 || index >= adapters.size() )
   {
      Con::warnf( "GFXInit::getAdapterMode - The adapter index was out of range." );
      return String::EmptyString;
   }

   if ( modeIndex < 0 || modeIndex >= adapters[index]->mAvailableModes.size() )
   {
      Con::warnf( "GFXInit::getAdapterMode - The mode index was out of range." );
      return String::EmptyString;
   }

   const GFXVideoMode &vm = adapters[index]->mAvailableModes[modeIndex];
   return vm.toString();
}

DefineEngineStaticMethod( GFXInit, createNullDevice, void, (),,
   "Create the NULL graphics device used for testing or headless operation." )
{
   // Enumerate things for GFX before we have an active device.
   GFXInit::enumerateAdapters();
 
   // Create a device.
   GFXAdapter *a = GFXInit::chooseAdapter(NullDevice, "");
 
   GFXDevice *newDevice = GFX;
 
   // Do we have a global device already? (This is the site if you want
   // to start rendering to multiple devices simultaneously)
   if(newDevice == NULL)
      newDevice = GFXInit::createDevice(a);
 
   newDevice->setAllowRender( false );
}
