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

#ifndef _PHYSX3_H_
#define _PHYSX3_H_

//-------------------------------------------------------------------------
//defines to keep PhysX happy and compiling
#if defined(TORQUE_OS_MAC) && !defined(__APPLE__)
   #define __APPLE__
#elif defined(TORQUE_OS_LINUX) && !defined(LINUX)
   #define LINUX
#elif defined(TORQUE_OS_WIN) && !defined(WIN32)
   #define WIN32
#endif

// macOS _DEBUG & NDEBUG
#if defined(TORQUE_OS_MAC) && defined(TORQUE_DEBUG) && !defined(_DEBUG)
   #define _DEBUG
#elif defined(TORQUE_OS_MAC) && defined(TORQUE_RELEASE) && !defined(NDEBUG)
   #define NDEBUG
#endif

// Linux _DEBUG & NDEBUG
#if defined(TORQUE_OS_LINUX) && defined(TORQUE_DEBUG) && !defined(_DEBUG)
#define _DEBUG
#elif defined(TORQUE_OS_LINUX) && defined(TORQUE_RELEASE) && !defined(NDEBUG)
#define NDEBUG
#endif

//-------------------------------------------------------------------------

//safe release template
template <class T> void SafeReleasePhysx(T* a)
{
   if (a)
   {
      a->release();
      a = NULL;
   }
}

#include <PxPhysicsAPI.h>
#include <extensions/PxExtensionsAPI.h>
#include <extensions/PxDefaultErrorCallback.h>
#include <extensions/PxDefaultAllocator.h>
#include <extensions/PxDefaultSimulationFilterShader.h>
#include <extensions/PxDefaultCpuDispatcher.h>
#include <extensions/PxShapeExt.h>
#include <extensions/PxSimpleFactory.h>
#include <foundation/PxFoundation.h>
#include <characterkinematic/PxController.h>
#include <pvd/PxPvd.h>


extern physx::PxPhysics* gPhysics3SDK;

#endif // _PHYSX3_
