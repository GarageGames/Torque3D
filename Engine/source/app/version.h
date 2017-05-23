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

#ifndef _VERSION_H_
#define _VERSION_H_

/// Since we can build different engine "products" out of the same
/// base engine source we need a way to differentiate which product
/// this particular game is using.
///
/// TGE       0001
/// TGEA      0002
/// TGB       0003
/// TGEA 360  0004
/// TGE WII   0005
/// Torque 3D 0006
/// Torque 3D MIT 0007
#define TORQUE_ENGINE_PRODUCT      0007

/// This is our global version number for the engine source code that
/// we are using. See <game>/source/torqueConfig.h for the game's source
/// code version, the game name, and which type of game it is (TGB, TGE, TGEA, etc.).
///
/// Version number is major * 1000 + minor * 100 + revision * 10.
#define TORQUE_GAME_ENGINE          3900

/// Human readable engine version string.
#define TORQUE_GAME_ENGINE_VERSION_STRING  "3.9.0"

/// Gets the engine version number.  The version number is specified as a global in version.cc
U32 getVersionNumber();
/// Gets the engine version number in a human readable form
const char* getVersionString();
/// Gets the engine product name in string form
const char* getEngineProductString();
/// Gets the compile date and time
const char* getCompileTimeString();

/// Gets the application version number.  The version number is specified as a global in torqueConfig.h
U32 getAppVersionNumber();
/// Gets the human readable application version string.
const char* getAppVersionString();

#endif
