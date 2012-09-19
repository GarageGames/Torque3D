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

/// This is our global version number for the engine source code that
/// we are using. See <game>/source/torqueConfig.h for the game's source
/// code version, the game name, and which type of game it is (TGB, TGE, TGEA, etc.).
///
/// Version number is major * 1000 + minor * 100 + revision * 10.
#define TORQUE_GAME_ENGINE          1100

/// Human readable engine version string.
#define TORQUE_GAME_ENGINE_VERSION_STRING  "2011"

/// Gets the specified version number.  The version number is specified as a global in version.cc
U32 getVersionNumber();
/// Gets the version number in string form
const char* getVersionString();
/// Gets the engine product name in string form
const char* getEngineProductString();
/// Gets the compile date and time
const char* getCompileTimeString();

#endif
