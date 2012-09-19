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

#ifndef _BASICSCENEOBJECTLIGHTINGPLUGIN_H_
#define _BASICSCENEOBJECTLIGHTINGPLUGIN_H_

#ifndef _SCENEOBJECTLIGHTINGPLUGIN_H_
#include "scene/sceneObjectLightingPlugin.h"
#endif

#ifndef _SCENEOBJECT_H_
#include "scene/sceneObject.h"
#endif

#ifndef _TSINGLETON_H_
#include "core/util/tSingleton.h"
#endif


class ShadowBase;

class BasicSceneObjectLightingPlugin : public SceneObjectLightingPlugin
{
private:

   ShadowBase* mShadow;
   SceneObject* mParentObject;

   static Vector<BasicSceneObjectLightingPlugin*> smPluginInstances;
   
public:
   BasicSceneObjectLightingPlugin(SceneObject* parent);
   ~BasicSceneObjectLightingPlugin();

   static Vector<BasicSceneObjectLightingPlugin*>* getPluginInstances() { return &smPluginInstances; }

   static void cleanupPluginInstances();
   static void resetAll();

   const F32 getScore() const;

   // Called from BasicLightManager
   virtual void updateShadow( SceneRenderState *state );
   virtual void renderShadow( SceneRenderState *state );

   // Called by statics
   virtual U32  packUpdate(SceneObject* obj, U32 checkMask, NetConnection *conn, U32 mask, BitStream *stream) { return 0; }
   virtual void unpackUpdate(SceneObject* obj, NetConnection *conn, BitStream *stream) { }

   virtual void reset();
};

class BasicSceneObjectPluginFactory : public ManagedSingleton< BasicSceneObjectPluginFactory >
{
protected:

   /// Called from the light manager on activation.
   /// @see LightManager::addActivateCallback
   void _onLMActivate( const char *lm, bool enable );   
   
   void _onDecalManagerClear();

   void removeLightPlugin(SceneObject* obj);
   void addLightPlugin(SceneObject* obj);
   void addToExistingObjects();

   bool mEnabled;

public:

   BasicSceneObjectPluginFactory();
   ~BasicSceneObjectPluginFactory();
   
   // For ManagedSingleton.
   static const char* getSingletonName() { return "BasicSceneObjectPluginFactory"; }
   
   void _setEnabled();
};

#endif // !_BASICSCENEOBJECTLIGHTINGPLUGIN_H_
