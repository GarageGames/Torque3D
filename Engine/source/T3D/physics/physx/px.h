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

//
// This PhysX implementation for Torque was originally based on
// the "PhysX in TGEA" resource written by Shannon Scarvaci.
//
// http://www.garagegames.com/index.php?sec=mg&mod=resource&page=view&qid=12711
//

#ifndef _PHYSX_H_
#define _PHYSX_H_

/*
#ifndef _TORQUE_TYPES_H_
#	include "platform/types.h"
#endif
*/

#include "platform/tmm_off.h"

#ifdef TORQUE_DEBUG
#include <crtdbg.h>
#endif

#if defined(TORQUE_OS_MAC) && !defined(__APPLE__)
   #define __APPLE__
#elif defined(TORQUE_OS_LINUX) && !defined(LINUX)
   #define LINUX
#elif defined(TORQUE_OS_WIN32) && !defined(WIN32)
   #define WIN32
#endif

#ifndef NX_PHYSICS_NXPHYSICS
#include <NxPhysics.h>
#endif
#ifndef NX_FOUNDATION_NXSTREAM
#include <NxStream.h>
#endif
#ifndef NX_COOKING_H
#include <NxCooking.h>
#endif
#ifndef NX_FOUNDATION_NXUSEROUTPUTSTREAM
#include <NxUserOutputStream.h>
#endif
#ifndef NX_PHYSICS_NXBIG
#include "NxExtended.h"
#endif
#include <NxUserAllocatorDefault.h>
#include <CCTAllocator.h>
#include <NxControllerManager.h>
#include <CharacterControllerManager.h>
#include <NxController.h>
#include <NxCapsuleController.h>

/// The single global physx sdk object for this process.
extern NxPhysicsSDK *gPhysicsSDK;

#include "platform/tmm_on.h"

#endif // _PHYSX_H_