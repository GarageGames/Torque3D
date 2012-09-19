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
#include "lighting/shadowManager.h"

#include "scene/sceneManager.h"
#include "materials/materialManager.h"

const String ShadowManager::ManagerTypeName("ShadowManager");

//------------------------------------------------------------------------------

bool ShadowManager::canActivate()
{
   return true;
}

//------------------------------------------------------------------------------

void ShadowManager::activate()
{
   mSceneManager = gClientSceneGraph; //;getWorld()->findWorldManager<SceneManager>();
}

//------------------------------------------------------------------------------

SceneManager* ShadowManager::getSceneManager()
{
   return mSceneManager;   
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

// Runtime switching of shadow systems.  Requires correct world to be pushed at console.
ConsoleFunction( setShadowManager, bool, 1, 3, "string sShadowSystemName" )
{
   /*
   // Make sure this new one exists
   ShadowManager * newSM = dynamic_cast<ShadowManager*>(ConsoleObject::create(argv[1]));
   if (!newSM)
      return false;

   // Cleanup current
   ShadowManager * currentSM = world->findWorldManager<ShadowManager>();
   if (currentSM)
   {
      currentSM->deactivate();
      world->removeWorldManager(currentSM);
      delete currentSM;
   }

   // Add to world and init.
   world->addWorldManager(newSM);   
   newSM->activate();      
   MaterialManager::get()->reInitInstances();
   */
   return true;
}