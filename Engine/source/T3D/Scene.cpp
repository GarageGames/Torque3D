#include "Scene.h"

Scene * Scene::smRootScene = nullptr;
Vector<Scene*> Scene::smSceneList;

IMPLEMENT_CO_NETOBJECT_V1(Scene);

Scene::Scene() : 
   mIsSubScene(false),
   mParentScene(nullptr),
   mSceneId(-1),
   mIsEditing(false),
   mIsDirty(false)
{

}

Scene::~Scene()
{

}

void Scene::initPersistFields()
{
   Parent::initPersistFields();

   addGroup("Internal");
   addField("isSubscene", TypeBool, Offset(mIsSubScene, Scene), "", AbstractClassRep::FIELD_HideInInspectors);
   addField("isEditing", TypeBool, Offset(mIsEditing, Scene), "", AbstractClassRep::FIELD_HideInInspectors);
   addField("isDirty", TypeBool, Offset(mIsDirty, Scene), "", AbstractClassRep::FIELD_HideInInspectors);
   endGroup("Internal");
}

bool Scene::onAdd()
{
   if (!Parent::onAdd())
      return false;

   smSceneList.push_back(this);
   mSceneId = smSceneList.size() - 1;

   /*if (smRootScene == nullptr)
   {
      //we're the first scene, so we're the root. woo!
      smRootScene = this;
   }
   else
   {
      mIsSubScene = true;
      smRootScene->mSubScenes.push_back(this);
   }*/

   return true;
}

void Scene::onRemove()
{
   Parent::onRemove();

   smSceneList.remove(this);
   mSceneId = -1;

   /*if (smRootScene == this)
   {
      for (U32 i = 0; i < mSubScenes.size(); i++)
      {
         mSubScenes[i]->deleteObject();
      }
   }
   else if (smRootScene != nullptr)
   {
      for (U32 i = 0; i < mSubScenes.size(); i++)
      {
         if(mSubScenes[i]->getId() == getId())
            smRootScene->mSubScenes.erase(i);
      }
   }*/
}

void Scene::addObject(SimObject* object)
{
   //Child scene
   Scene* scene = dynamic_cast<Scene*>(object);
   if (scene)
   {
      //We'll keep these principly separate so they don't get saved into each other
      mSubScenes.push_back(scene);
      return;
   }

   SceneObject* sceneObj = dynamic_cast<SceneObject*>(object);
   if (sceneObj)
   {
      //We'll operate on the presumption that if it's being added via regular parantage means, it's considered permanent
      mPermanentObjects.push_back(sceneObj);
      Parent::addObject(object);

      return;
   }

   //Do it like regular, though we should probably bail if we're trying to add non-scene objects to the scene?
   Parent::addObject(object);
}

void Scene::removeObject(SimObject* object)
{
   //Child scene
   Scene* scene = dynamic_cast<Scene*>(object);
   if (scene)
   {
      //We'll keep these principly separate so they don't get saved into each other
      mSubScenes.remove(scene);
      return;
   }

   SceneObject* sceneObj = dynamic_cast<SceneObject*>(object);
   if (sceneObj)
   {
      //We'll operate on the presumption that if it's being added via regular parantage means, it's considered permanent

      mPermanentObjects.remove(sceneObj);
      Parent::removeObject(object);

      return;
   }

   Parent::removeObject(object);
}

void Scene::addDynamicObject(SceneObject* object)
{
   mDynamicObjects.push_back(object);

   //Do it like regular, though we should probably bail if we're trying to add non-scene objects to the scene?
   Parent::addObject(object);
}

void Scene::removeDynamicObject(SceneObject* object)
{
   mDynamicObjects.remove(object);

   //Do it like regular, though we should probably bail if we're trying to add non-scene objects to the scene?
   Parent::removeObject(object);
}

void Scene::interpolateTick(F32 delta)
{

}

void Scene::processTick()
{

}

void Scene::advanceTime(F32 timeDelta)
{

}

U32 Scene::packUpdate(NetConnection *conn, U32 mask, BitStream *stream)
{
   bool ret = Parent::packUpdate(conn, mask, stream);

   return ret;
}

void Scene::unpackUpdate(NetConnection *conn, BitStream *stream)
{

}

//
Vector<SceneObject*> Scene::getObjectsByClass(String className)
{
   return Vector<SceneObject*>();
}

DefineEngineFunction(getScene, Scene*, (U32 sceneId), (0),
   "Get the root Scene object that is loaded.\n"
   "@return The id of the Root Scene. Will be 0 if no root scene is loaded")
{
   if (Scene::smSceneList.empty() || sceneId >= Scene::smSceneList.size())
      return nullptr;

   return Scene::smSceneList[sceneId];
}

DefineEngineFunction(getRootScene, S32, (), ,
   "Get the root Scene object that is loaded.\n"
   "@return The id of the Root Scene. Will be 0 if no root scene is loaded")
{
   Scene* root = Scene::getRootScene();

   if (root)
      return root->getId();

   return 0;
}

DefineEngineMethod(Scene, getRootScene, S32, (),,
   "Get the root Scene object that is loaded.\n"
   "@return The id of the Root Scene. Will be 0 if no root scene is loaded")
{
   Scene* root = Scene::getRootScene();

   if (root)
      return root->getId();

   return 0;
}

DefineEngineMethod(Scene, addDynamicObject, void, (SceneObject* sceneObj), (nullAsType<SceneObject*>()),
   "Get the root Scene object that is loaded.\n"
   "@return The id of the Root Scene. Will be 0 if no root scene is loaded")
{
   object->addDynamicObject(sceneObj);
}

DefineEngineMethod(Scene, removeDynamicObject, void, (SceneObject* sceneObj), (nullAsType<SceneObject*>()),
   "Get the root Scene object that is loaded.\n"
   "@return The id of the Root Scene. Will be 0 if no root scene is loaded")
{
   object->removeDynamicObject(sceneObj);
}

DefineEngineMethod(Scene, getObjectsByClass, String, (String className), (""),
   "Get the root Scene object that is loaded.\n"
   "@return The id of the Root Scene. Will be 0 if no root scene is loaded")
{
   if (className == String::EmptyString)
      return "";

   //return object->getObjectsByClass(className);
   return "";
}
