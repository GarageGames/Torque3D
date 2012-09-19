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

#ifndef _SHADOWMANAGER_H_
#define _SHADOWMANAGER_H_

#ifndef _TSIGNAL_H_
#include "core/util/tSignal.h"
#endif
#ifndef _TORQUE_STRING_H_
#include "core/util/str.h"
#endif

class SceneManager;

class ShadowManager
{
public:
   ShadowManager() : mSceneManager(NULL) {}
   virtual ~ShadowManager() { }

   // Called when the shadow manager should become active
   virtual void activate();

   // Called when we don't want the shadow manager active (should clean up)
   virtual void deactivate() { }

   // Return an "id" that other systems can use to load different versions of assets (custom shaders, etc.)
   // Should be short and contain no spaces and safe for filename use.
   //virtual const char* getId() const = 0;

   // SceneManager manager   
   virtual SceneManager* getSceneManager();

   // Called to find out if it is valid to activate this shadow system.  If not, we should print out
   // a console warning explaining why.
   virtual bool canActivate();

   // SimWorldManager
   static const String ManagerTypeName;
   const String & getManagerTypeName() const { return ManagerTypeName; }

private:
   SceneManager* mSceneManager;
};

#endif // _SHADOWMANAGER_H_
