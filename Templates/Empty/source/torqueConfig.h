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

#ifndef _TORQUECONFIG_H_
#define _TORQUECONFIG_H_

//-----------------------------------------------------------------------------
//Hi, and welcome to the Torque Config file.
//
//This file is a central reference for the various configuration flags that
//you'll be using when controlling what sort of a Torque build you have. In
//general, the information here is global for your entire codebase, applying
//not only to your game proper, but also to all of your tools.

/// What's the name of your application? Used in a variety of places.
#define TORQUE_APP_NAME            "Empty"

/// What version of the application specific source code is this?
///
/// Version number is major * 1000 + minor * 100 + revision * 10.
#define TORQUE_APP_VERSION         1000

/// Human readable application version string.
#define TORQUE_APP_VERSION_STRING  "1.0"

/// Define me if you want to enable multithreading support.
#ifndef TORQUE_MULTITHREAD
#define TORQUE_MULTITHREAD
#endif

/// Define me if you want to disable Torque memory manager.
#ifndef TORQUE_DISABLE_MEMORY_MANAGER
#define TORQUE_DISABLE_MEMORY_MANAGER
#endif

/// The improved SimDictionary uses C++11 and is designed for games where
/// there are over 10000 simobjects active normally. To enable the new
/// SimDictionary just uncomment the line below.
//#define USE_NEW_SIMDICTIONARY

/// Define me if you want to disable the virtual mount system.
//#define TORQUE_DISABLE_VIRTUAL_MOUNT_SYSTEM

/// Define me if you want to disable looking for the root of a given path
/// within a zip file.  This means that the zip file name itself must be
/// the root of the path.  Requires the virtual mount system to be active.
//#define TORQUE_DISABLE_FIND_ROOT_WITHIN_ZIP

//Uncomment this define if you want to use the alternative zip support where you can 
//define your directories and files inside the zip just like you would on disk
//instead of the default zip support that treats the zip as an extra directory.
//#define TORQUE_ZIP_DISK_LAYOUT

/// Define me if you don't want Torque to compile dso's
#define TORQUE_NO_DSO_GENERATION

// Define me if this build is a tools build

#ifndef TORQUE_PLAYER
#  define TORQUE_TOOLS
#else
#  undef TORQUE_TOOLS
#endif

/// Define me if you want to enable the profiler.
///    See also the TORQUE_SHIPPING block below
//#define TORQUE_ENABLE_PROFILER

/// Define me to enable debug mode; enables a great number of additional
/// sanity checks, as well as making AssertFatal and AssertWarn do something.
/// This is usually defined by the build target.
//#define TORQUE_DEBUG

/// Define me if this is a shipping build; if defined I will instruct Torque
/// to batten down some hatches and generally be more "final game" oriented.
/// Notably this disables a liberal resource manager file searching, and
/// console help strings.
//#define TORQUE_SHIPPING

/// Define me to enable a variety of network debugging aids.
///
///  - NetConnection packet logging.
///  - DebugChecksum guards to detect mismatched pack/unpacks.
///  - Detection of invalid destination ghosts.
///
//#define TORQUE_DEBUG_NET

/// Define me to enable detailed console logging of net moves.
//#define TORQUE_DEBUG_NET_MOVES

/// Enable this define to change the default Net::MaxPacketDataSize
/// Do this at your own risk since it has the potential to cause packets
/// to be split up by old routers and Torque does not have a mechanism to
/// stitch split packets back together. Using this define can be very useful
/// in controlled network hardware environments (like a LAN) or for singleplayer
/// games (like BArricade and its large paths)
//#define MAXPACKETSIZE 1500

/// Modify me to enable metric gathering code in the renderers.
///
/// 0 does nothing; higher numbers enable higher levels of metric gathering.
//#define TORQUE_GATHER_METRICS 0

/// Define me if you want to enable debug guards in the memory manager.
///
/// Debug guards are known values placed before and after every block of
/// allocated memory. They are checked periodically by Memory::validate(),
/// and if they are modified (indicating an access to memory the app doesn't
/// "own"), an error is flagged (ie, you'll see a crash in the memory
/// manager's validate code). Using this and a debugger, you can track down
/// memory corruption issues quickly.
//#define TORQUE_DEBUG_GUARD

/// Define me if you want to enable instanced-static behavior
//#define TORQUE_ENABLE_THREAD_STATICS

/// Define me if you want to gather static-usage metrics
//#define TORQUE_ENABLE_THREAD_STATIC_METRICS

/// Define me if you want to enable debug guards on the FrameAllocator.
/// 
/// This is similar to the above memory manager guards, but applies only to the
/// fast FrameAllocator temporary pool memory allocations. The guards are only
/// checked when the FrameAllocator frees memory (when it's water mark changes).
/// This is most useful for detecting buffer overruns when using FrameTemp<> .
/// A buffer overrun in the FrameAllocator is unlikely to cause a crash, but may
/// still result in unexpected behavior, if other FrameTemp's are stomped.
//#define FRAMEALLOCATOR_DEBUG_GUARD

/// This #define is used by the FrameAllocator to set the size of the frame.
///
/// It was previously set to 3MB but I've increased it to 32MB due to the
/// FrameAllocator being used as temporary storage for bitmaps in the D3D9
/// texture manager.
#define TORQUE_FRAME_SIZE     32 << 20

// Finally, we define some dependent #defines. This enables some subsidiary
// functionality to get automatically turned on in certain configurations.

#ifdef TORQUE_DEBUG

   #define TORQUE_GATHER_METRICS 0
   #define TORQUE_ENABLE_PROFILE_PATH
   #ifndef TORQUE_DEBUG_GUARD
      #define TORQUE_DEBUG_GUARD
   #endif
   #ifndef TORQUE_NET_STATS
      #define TORQUE_NET_STATS
   #endif

   // Enables the C++ assert macros AssertFatal, AssertWarn, etc.
   #define TORQUE_ENABLE_ASSERTS

#endif

#ifdef TORQUE_RELEASE
  // If it's not DEBUG, it's a RELEASE build, put appropriate things here.
#endif

#ifdef TORQUE_SHIPPING

    // TORQUE_SHIPPING flags here.

#else

   // Enable the profiler by default, if we're not doing a shipping build.
   #define TORQUE_ENABLE_PROFILER

   // Enable the TorqueScript assert() instruction if not shipping.
   #define TORQUE_ENABLE_SCRIPTASSERTS

   // We also enable GFX debug events for use in Pix and other graphics
   // debugging tools.
   #define TORQUE_ENABLE_GFXDEBUGEVENTS

#endif

#ifdef TORQUE_TOOLS
#  define TORQUE_INSTANCE_EXCLUSION   "TorqueToolsTest"
#else
#  define TORQUE_INSTANCE_EXCLUSION   "TorqueTest"
#endif

// Someday, it might make sense to do some pragma magic here so we error
// on inconsistent flags.

// The Xbox360 has it's own profiling tools, the Torque Profiler should not be used
#ifdef TORQUE_OS_XENON
#  ifdef TORQUE_ENABLE_PROFILER
#     undef TORQUE_ENABLE_PROFILER
#  endif
#
#  ifdef TORQUE_ENABLE_PROFILE_PATH
#     undef TORQUE_ENABLE_PROFILE_PATH
#endif
#endif


#endif // _TORQUECONFIG_H_

