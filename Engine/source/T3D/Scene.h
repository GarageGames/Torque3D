#pragma once

#include "console/engineAPI.h"

#ifndef _NETOBJECT_H_
#include "sim/netObject.h"
#endif

#ifndef _ITICKABLE_H_
#include "core/iTickable.h"
#endif

#include "scene/sceneObject.h"

/// Scene
/// This object is effectively a smart container to hold and manage any relevent scene objects and data
/// used to run things.
class Scene : public NetObject, public virtual ITickable
{
   typedef NetObject Parent;

   bool mIsSubScene;

   Scene* mParentScene;

   Vector<Scene*> mSubScenes;

   Vector<SceneObject*> mPermanentObjects;

   Vector<SceneObject*> mDynamicObjects;

   S32 mSceneId;

   bool mIsEditing;

   bool mIsDirty;

protected:
   static Scene * smRootScene;

   DECLARE_CONOBJECT(Scene);

public:
   Scene();
   ~Scene();

   static void initPersistFields();

   virtual bool onAdd();
   virtual void onRemove();

   virtual void interpolateTick(F32 delta);
   virtual void processTick();
   virtual void advanceTime(F32 timeDelta);

   virtual void addObject(SimObject* object);
   virtual void removeObject(SimObject* object);

   void addDynamicObject(SceneObject* object);
   void removeDynamicObject(SceneObject* object);

   //
   //Networking
   U32 packUpdate(NetConnection *conn, U32 mask, BitStream *stream);
   void unpackUpdate(NetConnection *conn, BitStream *stream);

   //
   Vector<SceneObject*> getObjectsByClass(String className);

   static Scene *getRootScene() 
   { 
      if (Scene::smSceneList.empty())
         return nullptr;

      return Scene::smSceneList[0];
   }

   static Vector<Scene*> smSceneList;
};