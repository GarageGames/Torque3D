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

#ifndef _GFXINIT_H_
#define _GFXINIT_H_

#ifndef _GFXDEVICE_H_
   #include "gfx/gfxDevice.h"
#endif
#ifndef _ENGINEOBJECT_H_
   #include "console/engineObject.h"
#endif


/// Interface for tracking GFX adapters and initializing them into devices.
/// @note Implement this class per platform.
/// @note This is just a class so it can be friends with GFXDevice)
class GFXInit 
{
   DECLARE_STATIC_CLASS( GFXInit );
   
public:
   /// Allows device to register themselves as available
   typedef Signal<void (Vector<GFXAdapter*>&)> RegisterDeviceSignal;
   static RegisterDeviceSignal& getRegisterDeviceSignal();

   /// Prepares the adapter list.
   static void init();

   /// Cleans out the adapter list.
   static void cleanup();
      
   /// Creates a GFXDevice based on an adapter from the
   /// enumerateAdapters method.
   ///
   /// @param   adapter   Graphics adapter to create device
   static GFXDevice *createDevice( GFXAdapter *adapter );

   /// Enumerate all the graphics adapters on the system
   static void enumerateAdapters();

   /// Get the enumerated adapters.  Should only call this after
   /// a call to enumerateAdapters.
   static void getAdapters( Vector<GFXAdapter*> *adapters );

   /// Get the number of available adapters.
   static S32 getAdapterCount();
   
   /// Compares the adapter's output display device with the given output display device
   static bool compareAdapterOutputDevice(const GFXAdapter* adapter, const char* outputDevice);

   /// Chooses a suitable GFXAdapter, based on type, preferences, and fallbacks.
   /// If the requested type is omitted, we use the prefs value.
   /// If the requested type isn't found, we use fallbacks: OpenGL, NullDevice
   /// This method never returns NULL.
   static GFXAdapter *chooseAdapter( GFXAdapterType type, const char* outputDevice);

   /// Override which chooses an adapter based on an index instead
   static GFXAdapter *chooseAdapter( GFXAdapterType type, S32 outputDeviceIndex );

   /// Gets the first adapter of the requested type (and on the requested output device)
   /// from the list of enumerated adapters. Should only call this after a call to
   /// enumerateAdapters.
   static GFXAdapter *getAdapterOfType( GFXAdapterType type, const char* outputDevice );

   /// Override which gets an adapter based on an index instead
   static GFXAdapter *getAdapterOfType( GFXAdapterType type, S32 outputDeviceIndex );
      
   /// Converts a GFXAdapterType to a string name. Useful for writing out prefs
   static const char *getAdapterNameFromType( GFXAdapterType type );
   
   /// Converts a string to a GFXAdapterType. Useful for reading in prefs.
   static GFXAdapterType getAdapterTypeFromName( const char* name );
   
   /// Returns a GFXVideoMode that describes the current state of the main monitor.
   /// This should probably move to the abstract window manager
   static GFXVideoMode getDesktopResolution();

   /// Based on user preferences (or in the absence of a valid user selection,
   /// a heuristic), return a "best" adapter.
   static GFXAdapter *getBestAdapterChoice();

   /// Get the initial video mode based on user preferences (or a heuristic).
   static GFXVideoMode getInitialVideoMode();
private:
   /// List of known adapters.
   static Vector<GFXAdapter*> smAdapters;

   static RegisterDeviceSignal* smRegisterDeviceSignal;
};

#endif
